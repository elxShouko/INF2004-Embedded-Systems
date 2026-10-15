/** @file imu_guard.c
 * Reject dynamic/unsupported motion; do not pretend magnitude proves tilt.
 * Limits and dwell times require measured trials. Defaults disable height.
 */
#include "imu_guard.h"

#include <math.h>

static uint32_t motion_reason (const imu_core_sample_t * p_sample,
                               const imu_core_config_t * p_config);

/** Constrain calibration to the source's representable/time domains. */
bool imu_guard_config (const imu_core_config_t * p_config)
{
    return (isfinite(p_config->maximum_accel_mm_s2)) &&
           (p_config->maximum_accel_mm_s2 > 0.0f) &&
           (p_config->maximum_accel_mm_s2 <= 2000000.0f) &&
           (isfinite(p_config->speed_band_mm_s)) &&
           (p_config->speed_band_mm_s > 0.0f) &&
           (p_config->speed_band_mm_s <= 2000.0f) &&
           (isfinite(p_config->wheel_delta_mm_s)) &&
           (p_config->wheel_delta_mm_s >= 0.0f) &&
           (p_config->wheel_delta_mm_s <= 2000.0f) &&
           (isfinite(p_config->minimum_speed_mm_s)) &&
           (p_config->minimum_speed_mm_s > 0.0f) &&
           (p_config->minimum_speed_mm_s <= 1000.0f) &&
           (p_config->settle_ms >= 20u) &&
           (p_config->settle_ms <= p_config->maximum_hump_ms) &&
           (p_config->ascent_ms >= 20u) &&
           (p_config->ascent_ms <= p_config->maximum_hump_ms) &&
           (p_config->level_ms >= 20u) &&
           (p_config->level_ms <= p_config->maximum_hump_ms);
}

/** Require new speed evidence for derivatives and a stable level approach. */
uint32_t imu_guard_check (imu_core_t * p_core,
                          const imu_core_sample_t * p_sample,
                          bool b_gravity_like)
{
    const imu_core_config_t * p_config = &p_core->config;
    uint32_t reasons = motion_reason(p_sample, p_config);
    uint32_t elapsed = p_sample->motion_time_ms - p_core->previous_motion_ms;
    bool b_new = (0u != elapsed);

    if (!p_config->b_height_calibrated)
    {
        reasons |= IMU_HEIGHT_UNCALIBRATED;
    }
    if (!b_gravity_like)
    {
        reasons |= IMU_HEIGHT_DYNAMIC;
    }
    if (p_sample->motion_epoch != p_core->previous_motion_epoch)
    {
        reasons |= IMU_HEIGHT_REVOKED;
        p_core->b_motion_baseline = false;
    }
    if (p_core->b_motion_baseline && (0u == reasons))
    {
        if ((elapsed > 100u) ||
            ((!b_new) &&
             (fabsf(p_sample->speed_mm_s - p_core->previous_speed) > 0.001f)))
        {
            reasons |= IMU_HEIGHT_MOTION;
        }
        else if (b_new)
        {
            /* Elapsed is in 1..100 ms, exactly representable as float. */
            const float32_t acceleration =
                fabsf(p_sample->speed_mm_s - p_core->previous_speed) *
                1000.0f / (float32_t)elapsed;

            if (acceleration > p_config->maximum_accel_mm_s2)
            {
                reasons |= IMU_HEIGHT_DYNAMIC;
            }
        }
        if (fabsf(p_sample->speed_mm_s - p_core->stable_speed) >
            p_config->speed_band_mm_s)
        {
            reasons |= IMU_HEIGHT_DYNAMIC;
        }
    }
    if ((0u != reasons) || (!p_core->b_motion_baseline))
    {
        p_core->stable_since_ms = p_sample->time_ms;
        p_core->stable_speed = p_sample->speed_mm_s;
        p_core->b_motion_stable = false;
    }
    else if ((IMU_MONITORING == p_core->status.phase) &&
             (fabsf(p_core->status.pitch_deg) <= p_config->level_deg) &&
             ((p_sample->time_ms - p_core->stable_since_ms) >=
              p_config->settle_ms) && b_new)
    {
        p_core->b_motion_stable = true;
    }
    if ((IMU_MONITORING == p_core->status.phase) &&
        (fabsf(p_core->status.pitch_deg) > p_config->level_deg) &&
        (fabsf(p_core->status.pitch_deg) < p_config->ascend_deg))
    {
        /* A nonlevel pre-ascent sample cannot be the stable approach. */
        p_core->b_motion_stable = false;
        p_core->stable_since_ms = p_sample->time_ms;
    }
    p_core->b_motion_baseline = (0u == motion_reason(p_sample, p_config));
    p_core->previous_speed = p_sample->speed_mm_s;
    p_core->previous_motion_ms = p_sample->motion_time_ms;
    p_core->previous_motion_epoch = p_sample->motion_epoch;
    return reasons;
}

/** Fail closed for stale, reverse, turning, slipping or unavailable input. */
static uint32_t motion_reason (const imu_core_sample_t * p_sample,
                               const imu_core_config_t * p_config)
{
    uint32_t reasons = 0u;

    if ((!p_sample->b_motion_valid) || (!p_sample->b_distance_valid) ||
        (p_sample->motion_age_ms > IMU_MOTION_MAX_AGE_MS) ||
        (!isfinite(p_sample->speed_mm_s)) ||
        (fabsf(p_sample->speed_mm_s) > 1000.0f))
    {
        reasons |= IMU_HEIGHT_MOTION;
    }
    if (p_sample->speed_mm_s < p_config->minimum_speed_mm_s)
    {
        reasons |= IMU_HEIGHT_REVERSE;
    }
    if ((!p_sample->b_straight) ||
        (!isfinite(p_sample->wheel_delta_mm_s)) ||
        (p_sample->wheel_delta_mm_s < 0.0f) ||
        (p_sample->wheel_delta_mm_s > p_config->wheel_delta_mm_s))
    {
        reasons |= IMU_HEIGHT_TURN;
    }
    if (!p_sample->b_traction_trusted)
    {
        reasons |= IMU_HEIGHT_TRACTION;
    }
    return reasons;
}

/* End of file. */

