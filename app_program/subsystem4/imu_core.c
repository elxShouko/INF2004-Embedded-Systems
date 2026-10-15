/** @file imu_core.c
 * Requirements: Write-up pp. 2-3, 8; Week 6 FR7-FR9, NFR5, section 6.4.
 * Height is an OPTIONAL path integral, not double-integrated acceleration.
 * It needs calibrated distance from the motion owner. See README caveats.
 */
#include "imu_core.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#define IMU_DEG_PER_RAD 57.2957795131f
#define IMU_AXIS_COUNT 3u

static bool sample_valid (const imu_core_sample_t *p_sample);
static void calibrate (imu_core_t *p_core,
                       const imu_core_sample_t *p_sample);
static void heading_update (imu_core_t *p_core,
                            const imu_core_sample_t *p_sample,
                            float32_t pitch, float32_t roll);
static void hump_update (imu_core_t *p_core,
                         const imu_core_sample_t *p_sample,
                         float32_t sine, bool b_tilt_valid);
static void classify_motion (imu_core_t *p_core,
                             const imu_core_sample_t *p_sample,
                             float32_t magnitude);
static void record_peak (imu_core_t *p_core);

imu_core_config_t imu_core_defaults (void)
{
    const imu_core_config_t config =
    {
        .ascend_deg = 6.0f,
        .level_deg = 2.0f,
        .impact_g = 1.8f,
        .turn_dps = 30.0f,
        .mag_scale = {1.0f, 1.0f, 1.0f},
        .calibration_samples = 100u,
        .maximum_gap_ms = 100u,
        .maximum_hump_ms = 10000u,
        .b_mag_calibrated = false
    };

    return config;
}

bool imu_core_init (imu_core_t *p_core,
                    const imu_core_config_t *p_config)
{
    bool b_valid = false;

    if ((NULL == p_core) || (NULL == p_config))
    {
        return false;
    }
    memset(p_core, 0, sizeof(*p_core));
    p_core->status.phase = IMU_FAULT;
    b_valid = isfinite(p_config->ascend_deg) &&
              isfinite(p_config->level_deg) &&
              isfinite(p_config->impact_g) &&
              isfinite(p_config->turn_dps) &&
              (p_config->level_deg > 0.0f) &&
              (p_config->ascend_deg > p_config->level_deg) &&
              (p_config->ascend_deg < 45.0f) &&
              (p_config->impact_g > 1.2f) &&
              (p_config->impact_g < 2.0f) &&
              (p_config->turn_dps > 0.0f) &&
              (p_config->calibration_samples >= 2u) &&
              (p_config->calibration_samples <= 10000u) &&
              (p_config->maximum_gap_ms >= 20u) &&
              (p_config->maximum_gap_ms <= 1000u) &&
              (p_config->maximum_hump_ms > p_config->maximum_gap_ms) &&
              (p_config->maximum_hump_ms <= 60000u);
    for (uint32_t axis = 0u; axis < IMU_AXIS_COUNT; ++axis)
    {
        b_valid = b_valid && isfinite(p_config->mag_offset[axis]) &&
                  isfinite(p_config->mag_scale[axis]) &&
                  (fabsf(p_config->mag_offset[axis]) <= 8.0f) &&
                  (p_config->mag_scale[axis] >= 0.1f) &&
                  (p_config->mag_scale[axis] <= 10.0f);
    }
    if (b_valid)
    {
        p_core->config = *p_config;
        p_core->status.phase = IMU_CALIBRATING;
    }
    return b_valid;
}

void imu_core_fault (imu_core_t *p_core)
{
    if (NULL != p_core)
    {
        p_core->status.phase = IMU_FAULT;
        p_core->status.events |= IMU_EVENT_FAULT;
        p_core->status.b_height_valid = false;
        p_core->status.b_heading_valid = false;
        p_core->b_hump_height_valid = false;
    }
}

uint32_t imu_core_update (imu_core_t *p_core,
                          const imu_core_sample_t *p_sample)
{
    float32_t accel[IMU_AXIS_COUNT] = {0.0f};
    float32_t magnitude = 0.0f;
    float32_t pitch = 0.0f;
    float32_t roll = 0.0f;
    uint32_t  elapsed = 0u;

    if (NULL == p_core)
    {
        return IMU_EVENT_FAULT;
    }
    p_core->status.events = 0u;
    if ((IMU_FAULT == p_core->status.phase) || (!sample_valid(p_sample)))
    {
        imu_core_fault(p_core);
        return p_core->status.events;
    }
    elapsed = p_sample->time_ms - p_core->status.time_ms;
    if (p_core->b_time_valid &&
        ((0u == elapsed) || (elapsed > p_core->config.maximum_gap_ms)))
    {
        imu_core_fault(p_core);
        return p_core->status.events;
    }
    p_core->b_time_valid = true;
    p_core->status.time_ms = p_sample->time_ms;
    for (uint32_t axis = 0u; axis < IMU_AXIS_COUNT; ++axis)
    {
        accel[axis] = p_sample->accel_g[axis] - p_core->bias[axis];
        magnitude += accel[axis] * accel[axis];
    }
    magnitude = sqrtf(magnitude);
    if ((magnitude >= p_core->config.impact_g) || (magnitude < 0.2f))
    {
        imu_core_fault(p_core);
        p_core->status.events |= IMU_EVENT_IMPACT;
        p_core->status.motion = IMU_IMPACT;
        return p_core->status.events;
    }
    if (IMU_CALIBRATING == p_core->status.phase)
    {
        calibrate(p_core, p_sample);
        return p_core->status.events;
    }
    pitch = atan2f(-accel[0], hypotf(accel[1], accel[2]));
    roll = atan2f(accel[1], accel[2]);
    p_core->status.pitch_deg = pitch * IMU_DEG_PER_RAD;
    p_core->status.roll_deg = roll * IMU_DEG_PER_RAD;
    heading_update(p_core, p_sample, pitch, roll);
    /* Dynamic acceleration corrupts gravity-based tilt. */
    hump_update(p_core, p_sample, sinf(pitch),
                 (fabsf(magnitude - 1.0f) <= 0.15f) &&
                 (fabsf(roll) <= 0.35f));
    classify_motion(p_core, p_sample, magnitude);
    return p_core->status.events;
}

static bool sample_valid (const imu_core_sample_t *p_sample)
{
    bool b_valid = (NULL != p_sample);

    if (!b_valid)
    {
        return false;
    }
    for (uint32_t axis = 0u; axis < IMU_AXIS_COUNT; ++axis)
    {
        b_valid = b_valid && isfinite(p_sample->accel_g[axis]) &&
                  (fabsf(p_sample->accel_g[axis]) < 2.0f);
        if (p_sample->b_mag_valid)
        {
            b_valid = b_valid && isfinite(p_sample->mag_gauss[axis]) &&
                      (fabsf(p_sample->mag_gauss[axis]) < 1.3f);
        }
    }
    if (p_sample->b_distance_valid)
    {
        b_valid = b_valid && isfinite(p_sample->distance_mm) &&
                  (p_sample->distance_mm >= 0.0f) &&
                  (p_sample->distance_mm <= 100.0f);
    }
    return b_valid;
}

static void calibrate (imu_core_t *p_core,
                       const imu_core_sample_t *p_sample)
{
    bool b_still = (fabsf(p_sample->accel_g[0]) < 0.1f) &&
                   (fabsf(p_sample->accel_g[1]) < 0.1f) &&
                   (fabsf(p_sample->accel_g[2] - 1.0f) < 0.1f);

    for (uint32_t axis = 0u; axis < IMU_AXIS_COUNT; ++axis)
    {
        if (0u != p_core->count)
        {
            b_still = b_still &&
                (fabsf(p_sample->accel_g[axis] -
                       p_core->previous_accel[axis]) < 0.03f);
        }
        p_core->previous_accel[axis] = p_sample->accel_g[axis];
    }
    if (!b_still)
    {
        p_core->count = 0u;
        memset(p_core->sum, 0, sizeof(p_core->sum));
        return;
    }
    ++p_core->count;
    for (uint32_t axis = 0u; axis < IMU_AXIS_COUNT; ++axis)
    {
        p_core->sum[axis] += p_sample->accel_g[axis];
    }
    if (p_core->count >= p_core->config.calibration_samples)
    {
        for (uint32_t axis = 0u; axis < IMU_AXIS_COUNT; ++axis)
        {
            /* Count <= 10000: exactly representable by float32_t. */
            p_core->bias[axis] = p_core->sum[axis] /
                                 (float32_t)p_core->count;
        }
        p_core->bias[2] -= 1.0f;
        p_core->status.phase = IMU_MONITORING;
        p_core->status.events = IMU_EVENT_CALIBRATED;
    }
}

static void heading_update (imu_core_t *p_core,
                            const imu_core_sample_t *p_sample,
                            float32_t pitch, float32_t roll)
{
    float32_t mag[IMU_AXIS_COUNT] = {0.0f};
    float32_t horizontal_x = 0.0f;
    float32_t horizontal_y = 0.0f;
    float32_t heading = 0.0f;
    float32_t delta = 0.0f;
    uint32_t  elapsed = p_sample->time_ms - p_core->heading_time_ms;

    p_core->status.b_heading_valid = false;
    p_core->status.turn_dps = 0.0f;
    if ((!p_sample->b_mag_valid) || (!p_core->config.b_mag_calibrated))
    {
        p_core->b_previous_heading = false;
        return;
    }
    for (uint32_t axis = 0u; axis < IMU_AXIS_COUNT; ++axis)
    {
        mag[axis] = (p_sample->mag_gauss[axis] -
                     p_core->config.mag_offset[axis]) *
                     p_core->config.mag_scale[axis];
    }
    horizontal_x = (mag[0] * cosf(pitch)) + (mag[2] * sinf(pitch));
    horizontal_y = (mag[0] * sinf(roll) * sinf(pitch)) +
                   (mag[1] * cosf(roll)) -
                   (mag[2] * sinf(roll) * cosf(pitch));
    if (hypotf(horizontal_x, horizontal_y) < 0.05f)
    {
        p_core->b_previous_heading = false;
        return;
    }
    heading = atan2f(-horizontal_y, horizontal_x) * IMU_DEG_PER_RAD;
    p_core->status.heading_deg = heading;
    p_core->status.b_heading_valid = true;
    if (p_core->b_previous_heading && (0u != elapsed) &&
        (elapsed <= p_core->config.maximum_gap_ms))
    {
        delta = heading - p_core->previous_heading;
        if (delta > 180.0f)
        {
            delta -= 360.0f;
        }
        else if (delta < -180.0f)
        {
            delta += 360.0f;
        }
        else
        {
            /* Already the shortest signed heading difference. */
        }
        /* Elapsed is bounded above by 1000 ms and nonzero. */
        p_core->status.turn_dps = delta * 1000.0f / (float32_t)elapsed;
        if (fabsf(p_core->status.turn_dps) >= p_core->config.turn_dps)
        {
            p_core->status.events |= IMU_EVENT_TURN;
        }
    }
    p_core->previous_heading = heading;
    p_core->heading_time_ms = p_sample->time_ms;
    p_core->b_previous_heading = true;
}

static void hump_update (imu_core_t *p_core,
                         const imu_core_sample_t *p_sample,
                         float32_t sine, bool b_tilt_valid)
{
    imu_core_status_t *p_status = &p_core->status;

    if ((IMU_MONITORING == p_status->phase) &&
        (p_status->pitch_deg >= p_core->config.ascend_deg))
    {
        p_status->phase = IMU_ASCENDING;
        p_status->events |= IMU_EVENT_ASCENT;
        p_status->hump_peak_mm = 0.0f;
        p_status->b_height_valid = false;
        p_core->height_mm = 0.0f;
        p_core->hump_start_ms = p_sample->time_ms;
        p_core->b_hump_height_valid = p_sample->b_distance_valid;
    }
    if (IMU_MONITORING != p_status->phase)
    {
        p_core->b_hump_height_valid = p_core->b_hump_height_valid &&
                                      p_sample->b_distance_valid &&
                                      b_tilt_valid;
        if ((p_sample->time_ms - p_core->hump_start_ms) >
            p_core->config.maximum_hump_ms)
        {
            imu_core_fault(p_core);
            return;
        }
        if (p_core->b_hump_height_valid &&
            (IMU_DESCENDING != p_status->phase))
        {
            p_core->height_mm += p_sample->distance_mm *
                                 (sine + p_core->previous_sine) * 0.5f;
            p_status->hump_peak_mm = fmaxf(p_status->hump_peak_mm,
                                          p_core->height_mm);
        }
    }
    if ((IMU_ASCENDING == p_status->phase) &&
        (p_status->pitch_deg <= p_core->config.level_deg))
    {
        p_status->phase = IMU_PEAK;
        p_status->events |= IMU_EVENT_PEAK;
    }
    if (IMU_PEAK == p_status->phase)
    {
        /* Week 6 section 6.4: compare/update maximum at the peak. */
        record_peak(p_core);
    }
    if ((IMU_PEAK == p_status->phase) &&
        (p_status->pitch_deg <= -p_core->config.ascend_deg))
    {
        p_status->phase = IMU_DESCENDING;
        p_status->events |= IMU_EVENT_DESCENT;
    }
    if ((IMU_DESCENDING == p_status->phase) &&
        (fabsf(p_status->pitch_deg) <= p_core->config.level_deg))
    {
        p_status->phase = IMU_MONITORING;
        p_status->events |= IMU_EVENT_COMPLETE;
    }
    p_core->previous_sine = sine;
}

static void record_peak (imu_core_t *p_core)
{
    imu_core_status_t *p_status = &p_core->status;

    p_status->b_height_valid = p_core->b_hump_height_valid;
    if (p_status->b_height_valid)
    {
        p_status->highest_hump_mm = fmaxf(p_status->highest_hump_mm,
                                         p_status->hump_peak_mm);
        p_status->b_highest_valid = true;
    }
}

static void classify_motion (imu_core_t *p_core,
                             const imu_core_sample_t *p_sample,
                             float32_t magnitude)
{
    imu_core_status_t *p_status = &p_core->status;

    if (IMU_FAULT == p_status->phase)
    {
        p_status->motion = IMU_MOTION_UNKNOWN;
    }
    else if (0u != (p_status->events & IMU_EVENT_TURN))
    {
        p_status->motion = IMU_TURNING;
    }
    else if (IMU_ASCENDING == p_status->phase)
    {
        p_status->motion = IMU_CLIMBING;
    }
    else if (IMU_DESCENDING == p_status->phase)
    {
        p_status->motion = IMU_DESCENDING_MOTION;
    }
    else if (fabsf(magnitude - 1.0f) > 0.15f)
    {
        p_status->motion = IMU_ACCELERATING;
    }
    else if (p_sample->b_distance_valid)
    {
        p_status->motion = (p_sample->distance_mm < 0.1f) ?
                           IMU_STATIONARY : IMU_MOVING;
    }
    else
    {
        /* Acceleration alone cannot distinguish rest from constant speed. */
        p_status->motion = IMU_MOTION_UNKNOWN;
    }
}

/* End of file. */

