/**
 * @file    imu_params.c
 * @brief   Buddy 4 run-time parameter defaults and validation.
 */

#include <math.h>
#include <stddef.h>

#include "imu/imu_config.h"

/* Upper bounds used only for sanity checking (not tuning values). */
#define PARAM_MAX_ANGLE_DEG        (90.0f)
#define PARAM_MAX_G                (16.0f)
#define PARAM_MAX_TIME_MS          (60000U)
#define PARAM_MAX_LPF_HZ           (100.0f)
#define PARAM_MAX_WHEELBASE_MM     (2000.0f)
#define PARAM_MAX_SAMPLES          (2000U)

static bool param_finite_pos(float value)
{
    return ((0 != isfinite(value)) && (value > 0.0f));
}

static bool param_finite_range(float value, float lo, float hi)
{
    return ((0 != isfinite(value)) && (value >= lo) && (value <= hi));
}

static bool param_time_ok(uint32_t value)
{
    return ((value > 0U) && (value <= PARAM_MAX_TIME_MS));
}

uint32_t imu_params_accel_odr_hz(uint8_t odr_code)
{
    uint32_t hz;

    switch (odr_code)
    {
        case LSM303_ACCEL_ODR_1HZ:   hz = 1U;   break;
        case LSM303_ACCEL_ODR_10HZ:  hz = 10U;  break;
        case LSM303_ACCEL_ODR_25HZ:  hz = 25U;  break;
        case LSM303_ACCEL_ODR_50HZ:  hz = 50U;  break;
        case LSM303_ACCEL_ODR_100HZ: hz = 100U; break;
        case LSM303_ACCEL_ODR_200HZ: hz = 200U; break;
        case LSM303_ACCEL_ODR_400HZ: hz = 400U; break;
        default:                     hz = 0U;   break;
    }

    return (hz);
}

void imu_params_default(imu_params_t * p_params)
{
    if (NULL == p_params)
    {
        return;
    }

    p_params->sample_period_ms   = IMU_CFG_SAMPLE_PERIOD_MS;
    p_params->i2c_baud_hz        = IMU_CFG_I2C_BAUD_HZ;
    p_params->i2c_timeout_us     = IMU_CFG_I2C_TIMEOUT_US;
    p_params->i2c_max_retries    = (uint8_t)IMU_CFG_I2C_MAX_RETRIES;
    p_params->accel_odr_code     = LSM303_ACCEL_ODR_200HZ;
    p_params->accel_fs_code      = LSM303_ACCEL_FS_4G;
    p_params->b_accel_fifo       = true;
    p_params->mag_odr_code       = LSM303_MAG_ODR_75HZ;
    p_params->mag_gain_code      = LSM303_MAG_GAIN_1_9;
    p_params->b_mag_id_check     = true;

    /* Default: sensor X forward, Y left, Z up. CHANGE AFTER AXIS TEST. */
    p_params->axis_map[0]  = (uint8_t)IMU_AXIS_X;
    p_params->axis_map[1]  = (uint8_t)IMU_AXIS_Y;
    p_params->axis_map[2]  = (uint8_t)IMU_AXIS_Z;
    p_params->axis_sign[0] = 1;
    p_params->axis_sign[1] = 1;
    p_params->axis_sign[2] = 1;

    p_params->fault_consec_cycles      = IMU_CFG_FAULT_CONSEC_CYCLES;
    p_params->frozen_cycles            = IMU_CFG_FROZEN_CYCLES;
    p_params->stale_data_ms            = IMU_CFG_STALE_DATA_MS;
    p_params->accel_plaus_min_g        = IMU_CFG_ACCEL_PLAUS_MIN_G;
    p_params->init_retry_ms            = IMU_CFG_INIT_RETRY_MS;
    p_params->init_max_attempts        = IMU_CFG_INIT_MAX_ATTEMPTS;
    p_params->recovery_interval_ms     = IMU_CFG_RECOVERY_INTERVAL_MS;
    p_params->recovery_interval_max_ms = IMU_CFG_RECOVERY_INTERVAL_MAX_MS;
    p_params->recovery_max_attempts    = IMU_CFG_RECOVERY_MAX_ATTEMPTS;
    p_params->recovery_verify_samples  = IMU_CFG_RECOVERY_VERIFY_SAMPLES;
    p_params->collision_settle_ms      = IMU_CFG_COLLISION_SETTLE_MS;

    p_params->cal_samples            = IMU_CFG_CAL_SAMPLES;
    p_params->cal_max_noise_g        = IMU_CFG_CAL_MAX_NOISE_G;
    p_params->cal_g_min              = IMU_CFG_CAL_G_MIN;
    p_params->cal_g_max              = IMU_CFG_CAL_G_MAX;
    p_params->cal_max_mount_tilt_deg = IMU_CFG_CAL_MAX_MOUNT_TILT_DEG;
    p_params->cal_max_attempts       = IMU_CFG_CAL_MAX_ATTEMPTS;
    p_params->cal_retry_ms           = IMU_CFG_CAL_RETRY_MS;
    p_params->mag_cal_min_span_gauss = IMU_CFG_MAG_CAL_MIN_SPAN_GAUSS;
    p_params->mag_cal_min_samples    = IMU_CFG_MAG_CAL_MIN_SAMPLES;

    p_params->b_mag_cal_valid       = false;
    p_params->mag_hard_iron_gauss.x = 0.0f;
    p_params->mag_hard_iron_gauss.y = 0.0f;
    p_params->mag_hard_iron_gauss.z = 0.0f;
    p_params->mag_soft_iron_scale.x = 1.0f;
    p_params->mag_soft_iron_scale.y = 1.0f;
    p_params->mag_soft_iron_scale.z = 1.0f;
    p_params->mag_ref_field_gauss   = 0.0f;

    p_params->accel_bias_g.x = 0.0f;
    p_params->accel_bias_g.y = 0.0f;
    p_params->accel_bias_g.z = 0.0f;

    p_params->attitude_lpf_hz       = IMU_CFG_ATTITUDE_LPF_HZ;
    p_params->attitude_valid_tol_g  = IMU_CFG_ATTITUDE_VALID_TOL_G;
    p_params->heading_lpf_hz        = IMU_CFG_HEADING_LPF_HZ;
    p_params->mag_field_tol_frac    = IMU_CFG_MAG_FIELD_TOL_FRAC;
    p_params->heading_max_jump_deg  = IMU_CFG_HEADING_MAX_JUMP_DEG;
    p_params->heading_outlier_limit = IMU_CFG_HEADING_OUTLIER_LIMIT;
    p_params->impact_lpf_hz         = IMU_CFG_IMPACT_LPF_HZ;
    p_params->vib_lpf_hz            = IMU_CFG_VIB_LPF_HZ;

    p_params->tilt_threshold_deg   = IMU_CFG_TILT_THRESHOLD_DEG;
    p_params->tilt_hysteresis_deg  = IMU_CFG_TILT_HYSTERESIS_DEG;
    p_params->tilt_min_duration_ms = IMU_CFG_TILT_MIN_DURATION_MS;
    p_params->tilt_critical_deg    = IMU_CFG_TILT_CRITICAL_DEG;

    p_params->turn_rate_window_ms    = IMU_CFG_TURN_RATE_WINDOW_MS;
    p_params->turn_enter_dps         = IMU_CFG_TURN_ENTER_DPS;
    p_params->turn_exit_dps          = IMU_CFG_TURN_EXIT_DPS;
    p_params->turn_min_duration_ms   = IMU_CFG_TURN_MIN_DURATION_MS;
    p_params->turn_exit_duration_ms  = IMU_CFG_TURN_EXIT_DURATION_MS;
    p_params->heading_change_deg     = IMU_CFG_HEADING_CHANGE_DEG;
    p_params->b_turn_allow_uncal_mag = IMU_CFG_TURN_ALLOW_UNCAL_MAG;

    p_params->hump_ascent_enter_deg    = IMU_CFG_HUMP_ASCENT_ENTER_DEG;
    p_params->hump_ascent_min_ms       = IMU_CFG_HUMP_ASCENT_MIN_MS;
    p_params->hump_peak_drop_deg       = IMU_CFG_HUMP_PEAK_DROP_DEG;
    p_params->hump_descent_enter_deg   = IMU_CFG_HUMP_DESCENT_ENTER_DEG;
    p_params->hump_descent_min_ms      = IMU_CFG_HUMP_DESCENT_MIN_MS;
    p_params->hump_level_band_deg      = IMU_CFG_HUMP_LEVEL_BAND_DEG;
    p_params->hump_level_min_ms        = IMU_CFG_HUMP_LEVEL_MIN_MS;
    p_params->hump_min_peak_deg        = IMU_CFG_HUMP_MIN_PEAK_DEG;
    p_params->hump_min_descent_deg     = IMU_CFG_HUMP_MIN_DESCENT_DEG;
    p_params->hump_ascent_max_ms       = IMU_CFG_HUMP_ASCENT_MAX_MS;
    p_params->hump_min_event_ms        = IMU_CFG_HUMP_MIN_EVENT_MS;
    p_params->hump_crest_max_ms        = IMU_CFG_HUMP_CREST_MAX_MS;
    p_params->hump_max_event_ms        = IMU_CFG_HUMP_MAX_EVENT_MS;
    p_params->hump_rearm_level_ms      = IMU_CFG_HUMP_REARM_LEVEL_MS;
    p_params->hump_min_gap_ms          = IMU_CFG_HUMP_MIN_GAP_MS;
    p_params->hump_impact_thr_scale    = IMU_CFG_HUMP_IMPACT_THR_SCALE;
    p_params->b_hump_require_moving    = IMU_CFG_HUMP_REQUIRE_MOVING;
    p_params->hump_max_hint_accel_g    = IMU_CFG_HUMP_MAX_HINT_ACCEL_G;
    p_params->wheelbase_mm             = IMU_CFG_WHEELBASE_MM;
    p_params->hump_height_gain         = IMU_CFG_HUMP_HEIGHT_GAIN;
    p_params->hump_height_offset_mm    = IMU_CFG_HUMP_HEIGHT_OFFSET_MM;
    p_params->imu_pos_frac             = IMU_CFG_IMU_POS_FRAC;
    p_params->b_hump_height_calibrated = false;
    p_params->height_method            = IMU_HEIGHT_METHOD_GEOMETRIC;
    p_params->rank_metric              = IMU_RANK_BY_PEAK_PITCH;

    p_params->impact_minor_g              = IMU_CFG_IMPACT_MINOR_G;
    p_params->impact_critical_g           = IMU_CFG_IMPACT_CRITICAL_G;
    p_params->impact_critical_any_g       = IMU_CFG_IMPACT_CRITICAL_ANY_G;
    p_params->impact_horiz_ratio          = IMU_CFG_IMPACT_HORIZ_RATIO;
    p_params->impact_max_duration_ms      = IMU_CFG_IMPACT_MAX_DURATION_MS;
    p_params->impact_cooldown_ms          = IMU_CFG_IMPACT_COOLDOWN_MS;
    p_params->impact_hold_ms              = IMU_CFG_IMPACT_HOLD_MS;
    p_params->impact_warmup_ms            = IMU_CFG_IMPACT_WARMUP_MS;
    p_params->b_impact_minor_enters_fault = IMU_CFG_IMPACT_MINOR_FAULT;

    p_params->accel_enter_g          = IMU_CFG_ACCEL_ENTER_G;
    p_params->accel_exit_g           = IMU_CFG_ACCEL_EXIT_G;
    p_params->class_hold_ms          = IMU_CFG_CLASS_HOLD_MS;
    p_params->stationary_speed_mm_s  = IMU_CFG_STATIONARY_SPEED_MM_S;
    p_params->stationary_vib_g       = IMU_CFG_STATIONARY_VIB_G;
    p_params->hint_max_age_ms        = IMU_CFG_HINT_MAX_AGE_MS;

    p_params->motion_evt_min_ms = IMU_CFG_MOTION_EVT_MIN_MS;
    p_params->tilt_evt_min_ms   = IMU_CFG_TILT_EVT_MIN_MS;
    p_params->turn_evt_min_ms   = IMU_CFG_TURN_EVT_MIN_MS;
}

/** Check that the axis map is a permutation and right-handed (det = +1). */
static bool params_axis_map_ok(imu_params_t const * p_params)
{
    float   m[3][3] = { { 0.0f } };
    float   det;
    uint8_t used = 0U;
    uint8_t k;

    for (k = 0U; k < 3U; k++)
    {
        uint8_t axis = p_params->axis_map[k];
        int8_t  sign = p_params->axis_sign[k];

        if ((axis > (uint8_t)IMU_AXIS_Z) || ((1 != sign) && (-1 != sign)))
        {
            return (false);
        }
        used |= (uint8_t)(1U << axis);
        m[k][axis] = (float)sign;
    }

    if (0x07U != used)
    {
        return (false);   /* not a permutation */
    }

    det = (m[0][0] * ((m[1][1] * m[2][2]) - (m[1][2] * m[2][1])))
        - (m[0][1] * ((m[1][0] * m[2][2]) - (m[1][2] * m[2][0])))
        + (m[0][2] * ((m[1][0] * m[2][1]) - (m[1][1] * m[2][0])));

    /* det = -1 would mirror the world: left/right turns would swap. */
    return (det > 0.0f);
}

imu_err_t imu_params_validate(imu_params_t const * p_params)
{
    bool b_ok = true;

    if (NULL == p_params)
    {
        return (IMU_ERR_PARAM);
    }

    b_ok = b_ok && (p_params->sample_period_ms >= IMU_MIN_SAMPLE_PERIOD_MS);
    b_ok = b_ok && (p_params->sample_period_ms <= IMU_MAX_SAMPLE_PERIOD_MS);
    b_ok = b_ok && (p_params->i2c_baud_hz >= 10000U);
    b_ok = b_ok && (p_params->i2c_baud_hz <= 400000U);
    b_ok = b_ok && (p_params->i2c_timeout_us >= 100U);
    b_ok = b_ok && (p_params->i2c_timeout_us <= 20000U);
    b_ok = b_ok && (p_params->i2c_max_retries <= 5U);
    b_ok = b_ok && (0U != imu_params_accel_odr_hz(p_params->accel_odr_code));
    b_ok = b_ok && (p_params->accel_fs_code <= LSM303_ACCEL_FS_16G);
    b_ok = b_ok && (p_params->mag_odr_code <= LSM303_MAG_ODR_220HZ);
    b_ok = b_ok && (p_params->mag_gain_code >= LSM303_MAG_GAIN_1_3);
    b_ok = b_ok && (p_params->mag_gain_code <= LSM303_MAG_GAIN_8_1);
    b_ok = b_ok && params_axis_map_ok(p_params);

    b_ok = b_ok && (p_params->fault_consec_cycles > 0U);
    b_ok = b_ok && (p_params->frozen_cycles > 1U);
    b_ok = b_ok && param_time_ok(p_params->stale_data_ms);
    b_ok = b_ok && param_finite_range(p_params->accel_plaus_min_g,
                                      0.0f, 0.9f);
    b_ok = b_ok && param_time_ok(p_params->init_retry_ms);
    b_ok = b_ok && (p_params->init_max_attempts > 0U);
    b_ok = b_ok && param_time_ok(p_params->recovery_interval_ms);
    b_ok = b_ok && (p_params->recovery_interval_max_ms
                    >= p_params->recovery_interval_ms);
    b_ok = b_ok && param_time_ok(p_params->recovery_interval_max_ms);
    b_ok = b_ok && (p_params->recovery_max_attempts > 0U);
    b_ok = b_ok && (p_params->recovery_verify_samples > 0U);
    b_ok = b_ok && param_time_ok(p_params->collision_settle_ms);

    b_ok = b_ok && (p_params->cal_samples >= 10U);
    b_ok = b_ok && (p_params->cal_samples <= PARAM_MAX_SAMPLES);
    b_ok = b_ok && param_finite_pos(p_params->cal_max_noise_g);
    b_ok = b_ok && param_finite_range(p_params->cal_g_min, 0.1f, 1.0f);
    b_ok = b_ok && param_finite_range(p_params->cal_g_max, 1.0f, 2.0f);
    b_ok = b_ok && param_finite_range(p_params->cal_max_mount_tilt_deg,
                                      0.0f, PARAM_MAX_ANGLE_DEG);
    b_ok = b_ok && (p_params->cal_max_attempts > 0U);
    b_ok = b_ok && param_time_ok(p_params->cal_retry_ms);
    b_ok = b_ok && param_finite_pos(p_params->mag_cal_min_span_gauss);
    b_ok = b_ok && (p_params->mag_cal_min_samples >= 10U);

    b_ok = b_ok && param_finite_range(p_params->mag_soft_iron_scale.x,
                                      0.2f, 5.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_soft_iron_scale.y,
                                      0.2f, 5.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_soft_iron_scale.z,
                                      0.2f, 5.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_hard_iron_gauss.x,
                                      -8.0f, 8.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_hard_iron_gauss.y,
                                      -8.0f, 8.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_hard_iron_gauss.z,
                                      -8.0f, 8.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_ref_field_gauss,
                                      0.0f, 8.0f);
    b_ok = b_ok && param_finite_range(p_params->accel_bias_g.x, -0.5f, 0.5f);
    b_ok = b_ok && param_finite_range(p_params->accel_bias_g.y, -0.5f, 0.5f);
    b_ok = b_ok && param_finite_range(p_params->accel_bias_g.z, -0.5f, 0.5f);

    b_ok = b_ok && param_finite_range(p_params->attitude_lpf_hz,
                                      0.05f, PARAM_MAX_LPF_HZ);
    b_ok = b_ok && param_finite_range(p_params->attitude_valid_tol_g,
                                      0.01f, 1.0f);
    b_ok = b_ok && param_finite_range(p_params->heading_lpf_hz,
                                      0.05f, PARAM_MAX_LPF_HZ);
    b_ok = b_ok && param_finite_range(p_params->mag_field_tol_frac,
                                      0.01f, 1.0f);
    b_ok = b_ok && param_finite_range(p_params->heading_max_jump_deg,
                                      1.0f, 180.0f);
    b_ok = b_ok && param_finite_range(p_params->impact_lpf_hz,
                                      0.05f, PARAM_MAX_LPF_HZ);
    b_ok = b_ok && param_finite_range(p_params->vib_lpf_hz,
                                      0.05f, PARAM_MAX_LPF_HZ);

    b_ok = b_ok && param_finite_range(p_params->tilt_threshold_deg,
                                      1.0f, PARAM_MAX_ANGLE_DEG);
    b_ok = b_ok && param_finite_range(p_params->tilt_hysteresis_deg, 0.0f,
                                      p_params->tilt_threshold_deg);
    b_ok = b_ok && param_time_ok(p_params->tilt_min_duration_ms);
    b_ok = b_ok && param_finite_range(p_params->tilt_critical_deg,
                                      p_params->tilt_threshold_deg,
                                      PARAM_MAX_ANGLE_DEG);

    b_ok = b_ok && param_time_ok(p_params->turn_rate_window_ms);
    b_ok = b_ok && (p_params->turn_rate_window_ms
                    >= (2U * p_params->sample_period_ms));
    b_ok = b_ok && (p_params->turn_rate_window_ms
                    < ((IMU_HEADING_HIST_LEN - 1U)
                       * p_params->sample_period_ms));
    b_ok = b_ok && param_finite_pos(p_params->turn_enter_dps);
    b_ok = b_ok && param_finite_range(p_params->turn_exit_dps, 0.0f,
                                      p_params->turn_enter_dps);
    b_ok = b_ok && param_time_ok(p_params->turn_min_duration_ms);
    b_ok = b_ok && param_time_ok(p_params->turn_exit_duration_ms);
    b_ok = b_ok && param_finite_range(p_params->heading_change_deg,
                                      1.0f, 180.0f);

    b_ok = b_ok && param_finite_range(p_params->hump_level_band_deg,
                                      0.1f, 20.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_ascent_enter_deg,
                                      p_params->hump_level_band_deg, 45.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_descent_enter_deg,
                                      p_params->hump_level_band_deg, 45.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_peak_drop_deg,
                                      0.1f, 20.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_min_peak_deg,
                                      p_params->hump_ascent_enter_deg, 45.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_min_descent_deg,
                                      0.0f, 45.0f);
    b_ok = b_ok && param_time_ok(p_params->hump_ascent_min_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_descent_min_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_level_min_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_ascent_max_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_min_event_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_crest_max_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_max_event_ms);
    b_ok = b_ok && (p_params->hump_max_event_ms > p_params->hump_min_event_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_rearm_level_ms);
    b_ok = b_ok && (p_params->hump_min_gap_ms <= PARAM_MAX_TIME_MS);
    b_ok = b_ok && param_finite_range(p_params->hump_impact_thr_scale,
                                      1.0f, 10.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_max_hint_accel_g,
                                      0.0f, 2.0f);
    b_ok = b_ok && param_finite_range(p_params->wheelbase_mm,
                                      10.0f, PARAM_MAX_WHEELBASE_MM);
    b_ok = b_ok && param_finite_range(p_params->hump_height_gain,
                                      0.1f, 10.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_height_offset_mm,
                                      -100.0f, 100.0f);
    b_ok = b_ok && param_finite_range(p_params->imu_pos_frac, 0.05f, 1.5f);
    b_ok = b_ok && (p_params->height_method <= IMU_HEIGHT_METHOD_INTEGRATED);
    b_ok = b_ok && (p_params->rank_metric <= IMU_RANK_BY_HEIGHT_EST);

    b_ok = b_ok && param_finite_range(p_params->impact_minor_g,
                                      0.1f, PARAM_MAX_G);
    b_ok = b_ok && param_finite_range(p_params->impact_critical_g,
                                      p_params->impact_minor_g, PARAM_MAX_G);
    b_ok = b_ok && param_finite_range(p_params->impact_critical_any_g,
                                      p_params->impact_critical_g,
                                      PARAM_MAX_G);
    b_ok = b_ok && param_finite_range(p_params->impact_horiz_ratio,
                                      0.0f, 1.0f);
    b_ok = b_ok && param_time_ok(p_params->impact_max_duration_ms);
    b_ok = b_ok && param_time_ok(p_params->impact_cooldown_ms);
    b_ok = b_ok && param_time_ok(p_params->impact_hold_ms);
    b_ok = b_ok && (p_params->impact_warmup_ms <= PARAM_MAX_TIME_MS);

    b_ok = b_ok && param_finite_range(p_params->accel_enter_g, 0.001f, 2.0f);
    b_ok = b_ok && param_finite_range(p_params->accel_exit_g, 0.0f,
                                      p_params->accel_enter_g);
    b_ok = b_ok && param_time_ok(p_params->class_hold_ms);
    b_ok = b_ok && (p_params->stationary_speed_mm_s >= 0);
    b_ok = b_ok && param_finite_range(p_params->stationary_vib_g, 0.0f, 1.0f);
    b_ok = b_ok && param_time_ok(p_params->hint_max_age_ms);

    b_ok = b_ok && (p_params->motion_evt_min_ms <= PARAM_MAX_TIME_MS);
    b_ok = b_ok && (p_params->tilt_evt_min_ms <= PARAM_MAX_TIME_MS);
    b_ok = b_ok && (p_params->turn_evt_min_ms <= PARAM_MAX_TIME_MS);

    return (b_ok ? IMU_OK : IMU_ERR_PARAM);
}
