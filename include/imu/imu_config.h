/**
 * @file    imu_config.h
 * @brief   Buddy 4 configuration: hardware constants, default tuning values
 *          and the run-time parameter structure.
 *
 * !!! IMPORTANT !!!
 * Every threshold, filter cut-off and timing value marked "UNVALIDATED" is an
 * initial DEVELOPMENT DEFAULT chosen from first-principles estimates for a
 * small (~150 mm wheelbase) two-wheel robot car travelling at 0.1-0.5 m/s.
 * None of these values has been validated on the team's vehicle. They must be
 * replaced with values derived from the recorded measurements described in
 * docs/buddy4/README.md (Part 7) before they are relied upon.
 * (Design Review Sections 8.2 / 8.3: "Deferred / Needs validation".)
 *
 * The values can be overridden either at compile time (-D on the compiler
 * command line, because every macro is wrapped in #ifndef) or at run time
 * through imu_params_t.
 */

#ifndef IMU_CONFIG_H
#define IMU_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_types.h"
#include "imu/lsm303dlhc_regs.h"

/* ===================================================================== */
/*  Hardware allocation (Design Review Section 2) - CONFIRMED BY DOCUMENT */
/* ===================================================================== */

/** Robo Pico Grove 4: GP16 = I2C0 SDA, GP17 = I2C0 SCL (RP2040 GPIO mux). */
#ifndef IMU_I2C_SDA_GPIO
#define IMU_I2C_SDA_GPIO                (16U)
#endif
#ifndef IMU_I2C_SCL_GPIO
#define IMU_I2C_SCL_GPIO                (17U)
#endif
/** 0 = i2c0, 1 = i2c1. GP16/GP17 can only be I2C0 on the RP2040. */
#ifndef IMU_I2C_INSTANCE
#define IMU_I2C_INSTANCE                (0U)
#endif
/**
 * Enable the RP2040 internal pull-ups (~50-80 kOhm). Most LSM303DLHC breakout
 * boards already carry 4.7-10 kOhm pull-ups; the internal ones are only a
 * weak backup. Verify the breakout with a multimeter.
 */
#ifndef IMU_I2C_INTERNAL_PULLUPS
#define IMU_I2C_INTERNAL_PULLUPS        (1)
#endif

/* ===================================================================== */
/*  Static sizing (compile time only)                                     */
/* ===================================================================== */
#define IMU_HEADING_HIST_LEN            (16U)  /**< heading history slots  */
#define IMU_EVENT_CRITICAL_PENDING      (4U)   /**< critical retry slots   */
#ifndef IMU_STATE_TRACE_LEN
#define IMU_STATE_TRACE_LEN             (16U)  /**< state transition trace */
#endif
#define IMU_ACCEL_BATCH_MAX             (LSM303_FIFO_DEPTH)
#define IMU_MAX_SAMPLE_PERIOD_MS        (100U)
#define IMU_MIN_SAMPLE_PERIOD_MS        (5U)

/* ===================================================================== */
/*  Default (UNVALIDATED) tuning values                                   */
/* ===================================================================== */

/* --- Sampling ----------------------------------------------------------
 * 20 ms (50 Hz): two ticks of the 10 ms uT-Kernel system timer used by the
 * TRON RP2040 port. Must be an integer multiple of the kernel tick. */
#ifndef IMU_CFG_SAMPLE_PERIOD_MS
#define IMU_CFG_SAMPLE_PERIOD_MS        (20U)
#endif

/* --- I2C ---------------------------------------------------------------- */
#ifndef IMU_CFG_I2C_BAUD_HZ
#define IMU_CFG_I2C_BAUD_HZ             (400000U)  /* LSM303DLHC fast-mode */
#endif
#ifndef IMU_CFG_I2C_TIMEOUT_US
#define IMU_CFG_I2C_TIMEOUT_US          (2000U)    /* per transfer bound   */
#endif
#ifndef IMU_CFG_I2C_MAX_RETRIES
#define IMU_CFG_I2C_MAX_RETRIES         (2U)
#endif

/* --- Fault handling (UNVALIDATED) ---------------------------------------- */
#define IMU_CFG_FAULT_CONSEC_CYCLES     (5U)     /* 100 ms of failures     */
#define IMU_CFG_FROZEN_CYCLES           (50U)    /* 1 s identical samples  */
#define IMU_CFG_STALE_DATA_MS           (200U)
#define IMU_CFG_ACCEL_PLAUS_MIN_G       (0.20f)
#define IMU_CFG_INIT_RETRY_MS           (200U)
#define IMU_CFG_INIT_MAX_ATTEMPTS       (5U)
#define IMU_CFG_RECOVERY_INTERVAL_MS    (200U)
#define IMU_CFG_RECOVERY_INTERVAL_MAX_MS (2000U)
#define IMU_CFG_RECOVERY_MAX_ATTEMPTS   (10U)
#define IMU_CFG_RECOVERY_VERIFY_SAMPLES (10U)
#define IMU_CFG_COLLISION_SETTLE_MS     (500U)

/* --- Calibration (UNVALIDATED) -------------------------------------------- */
#define IMU_CFG_CAL_SAMPLES             (100U)   /* 2 s at 50 Hz           */
#define IMU_CFG_CAL_MAX_NOISE_G         (0.02f)  /* per-axis std dev       */
#define IMU_CFG_CAL_G_MIN               (0.85f)
#define IMU_CFG_CAL_G_MAX               (1.15f)
#define IMU_CFG_CAL_MAX_MOUNT_TILT_DEG  (30.0f)
#define IMU_CFG_CAL_MAX_ATTEMPTS        (5U)
#define IMU_CFG_CAL_RETRY_MS            (500U)
#define IMU_CFG_MAG_CAL_MIN_SPAN_GAUSS  (0.30f)
#define IMU_CFG_MAG_CAL_MIN_SAMPLES     (50U)

/* --- Filtering (UNVALIDATED) ---------------------------------------------- */
#define IMU_CFG_ATTITUDE_LPF_HZ         (8.0f)
#define IMU_CFG_ATTITUDE_VALID_TOL_G    (0.15f)
#define IMU_CFG_HEADING_LPF_HZ          (3.0f)
#define IMU_CFG_MAG_FIELD_TOL_FRAC      (0.25f)
#define IMU_CFG_HEADING_MAX_JUMP_DEG    (45.0f)
#define IMU_CFG_HEADING_OUTLIER_LIMIT   (3U)
#define IMU_CFG_IMPACT_LPF_HZ           (2.0f)
#define IMU_CFG_VIB_LPF_HZ              (1.0f)

/* --- Tilt (UNVALIDATED) --------------------------------------------------- */
#define IMU_CFG_TILT_THRESHOLD_DEG      (15.0f)
#define IMU_CFG_TILT_HYSTERESIS_DEG     (3.0f)
#define IMU_CFG_TILT_MIN_DURATION_MS    (300U)
#define IMU_CFG_TILT_CRITICAL_DEG       (35.0f)

/* --- Turning (UNVALIDATED) ------------------------------------------------ */
#define IMU_CFG_TURN_RATE_WINDOW_MS     (200U)
#define IMU_CFG_TURN_ENTER_DPS          (25.0f)
#define IMU_CFG_TURN_EXIT_DPS           (12.0f)
#define IMU_CFG_TURN_MIN_DURATION_MS    (200U)
#define IMU_CFG_TURN_EXIT_DURATION_MS   (200U)
#define IMU_CFG_HEADING_CHANGE_DEG      (20.0f)
#define IMU_CFG_TURN_ALLOW_UNCAL_MAG    (true)

/* --- Hump detection (UNVALIDATED) ----------------------------------------- */
#define IMU_CFG_HUMP_ASCENT_ENTER_DEG   (3.0f)
#define IMU_CFG_HUMP_ASCENT_MIN_MS      (60U)
#define IMU_CFG_HUMP_PEAK_DROP_DEG      (1.0f)
#define IMU_CFG_HUMP_DESCENT_ENTER_DEG  (2.5f)   /* magnitude, nose-down */
#define IMU_CFG_HUMP_DESCENT_MIN_MS     (40U)
#define IMU_CFG_HUMP_LEVEL_BAND_DEG     (1.5f)
#define IMU_CFG_HUMP_LEVEL_MIN_MS       (100U)
#define IMU_CFG_HUMP_MIN_PEAK_DEG       (3.5f)
#define IMU_CFG_HUMP_MIN_DESCENT_DEG    (2.0f)
#define IMU_CFG_HUMP_ASCENT_MAX_MS      (2000U)
#define IMU_CFG_HUMP_MIN_EVENT_MS       (150U)
/* Max time between the crest and the start of the nose-down phase: front
 * wheels land, rear wheels must then reach the hump. ~1.3 * L / v_min
 * (L = 0.14 m, v_min = 0.12 m/s). Distance-based (1.5 L) with a hint. */
#define IMU_CFG_HUMP_CREST_MAX_MS       (1500U)
#define IMU_CFG_HUMP_MAX_EVENT_MS       (4000U)
#define IMU_CFG_HUMP_REARM_LEVEL_MS     (150U)
#define IMU_CFG_HUMP_MIN_GAP_MS         (300U)
#define IMU_CFG_HUMP_IMPACT_THR_SCALE   (1.5f)
#define IMU_CFG_HUMP_REQUIRE_MOVING     (true)   /* only used with a hint */
#define IMU_CFG_HUMP_MAX_HINT_ACCEL_G   (0.10f)  /* only used with a hint */
/** Wheelbase (axle-to-axle / wheel-to-caster contact distance).
 *  PLACEHOLDER - MEASURE THE REAL VEHICLE. */
#define IMU_CFG_WHEELBASE_MM            (140.0f)
#define IMU_CFG_HUMP_HEIGHT_GAIN        (1.0f)
#define IMU_CFG_HUMP_HEIGHT_OFFSET_MM   (0.0f)
#define IMU_CFG_IMU_POS_FRAC            (0.5f)   /* IMU dist from rear / L */

/* --- Impact (UNVALIDATED) ------------------------------------------------- */
#define IMU_CFG_IMPACT_MINOR_G          (1.2f)
#define IMU_CFG_IMPACT_CRITICAL_G       (2.0f)
#define IMU_CFG_IMPACT_CRITICAL_ANY_G   (3.0f)
#define IMU_CFG_IMPACT_HORIZ_RATIO      (0.6f)
#define IMU_CFG_IMPACT_MAX_DURATION_MS  (60U)
#define IMU_CFG_IMPACT_COOLDOWN_MS      (1000U)
#define IMU_CFG_IMPACT_HOLD_MS          (500U)
#define IMU_CFG_IMPACT_WARMUP_MS        (500U)
#define IMU_CFG_IMPACT_MINOR_FAULT      (false)

/* --- Motion classification (UNVALIDATED) ---------------------------------- */
#define IMU_CFG_ACCEL_ENTER_G           (0.06f)
#define IMU_CFG_ACCEL_EXIT_G            (0.03f)
#define IMU_CFG_CLASS_HOLD_MS           (150U)
#define IMU_CFG_STATIONARY_SPEED_MM_S   (10)
#define IMU_CFG_STATIONARY_VIB_G        (0.015f)
#define IMU_CFG_HINT_MAX_AGE_MS         (200U)
#define IMU_CFG_HINT_MAX_SPEED_MM_S     (3000)   /* plausibility bound */
#define IMU_CFG_HINT_MAX_ACCEL_MM_S2    (20000)  /* plausibility bound */

/* --- Event rate limiting -------------------------------------------------- */
#define IMU_CFG_MOTION_EVT_MIN_MS       (250U)
#define IMU_CFG_TILT_EVT_MIN_MS         (250U)
#define IMU_CFG_TURN_EVT_MIN_MS         (100U)

/* ===================================================================== */
/*  Enumerations used by the parameter structure                          */
/* ===================================================================== */

/** Vehicle-to-sensor axis selector. */
typedef enum
{
    IMU_AXIS_X = 0,
    IMU_AXIS_Y = 1,
    IMU_AXIS_Z = 2
} imu_axis_t;

/** Physical height estimation method (see docs Part 2 for the physics). */
typedef enum
{
    IMU_HEIGHT_METHOD_NONE = 0,   /**< report only the IMU peak metric     */
    IMU_HEIGHT_METHOD_GEOMETRIC,  /**< h = L * sin(peak pitch)             */
    IMU_HEIGHT_METHOD_INTEGRATED  /**< h = max integral(v sin(pitch)) / k  */
} imu_height_method_t;

/** Metric used to rank humps and pick the mission maximum. */
typedef enum
{
    IMU_RANK_BY_PEAK_PITCH = 0,   /**< directly measured (default)         */
    IMU_RANK_BY_HEIGHT_EST        /**< estimated height (needs calibration)*/
} imu_rank_metric_t;

/* ===================================================================== */
/*  Run-time parameter structure                                          */
/* ===================================================================== */

/** All tunables in one structure so that tests and the Mission Controller
 *  can change them without recompiling. Populate with imu_params_default()
 *  and validate with imu_params_validate(). */
typedef struct
{
    /* Sampling / bus */
    uint32_t sample_period_ms;
    uint32_t i2c_baud_hz;
    uint32_t i2c_timeout_us;
    uint8_t  i2c_max_retries;
    uint8_t  accel_odr_code;
    uint8_t  accel_fs_code;
    bool     b_accel_fifo;
    uint8_t  mag_odr_code;
    uint8_t  mag_gain_code;
    bool     b_mag_id_check;

    /* Sensor-to-vehicle axis map: vehicle axis k = sign[k] * sensor[axis[k]]
     * k: 0 = forward, 1 = left, 2 = up. Must form a right-handed frame. */
    uint8_t  axis_map[3];
    int8_t   axis_sign[3];

    /* Faults */
    uint32_t fault_consec_cycles;
    uint32_t frozen_cycles;
    uint32_t stale_data_ms;
    float    accel_plaus_min_g;
    uint32_t init_retry_ms;
    uint32_t init_max_attempts;
    uint32_t recovery_interval_ms;
    uint32_t recovery_interval_max_ms;
    uint32_t recovery_max_attempts;
    uint32_t recovery_verify_samples;
    uint32_t collision_settle_ms;

    /* Calibration */
    uint32_t cal_samples;
    float    cal_max_noise_g;
    float    cal_g_min;
    float    cal_g_max;
    float    cal_max_mount_tilt_deg;
    uint32_t cal_max_attempts;
    uint32_t cal_retry_ms;
    float    mag_cal_min_span_gauss;
    uint32_t mag_cal_min_samples;

    /* Stored magnetometer calibration (from the offline procedure) */
    bool       b_mag_cal_valid;
    imu_vec3_t mag_hard_iron_gauss;
    imu_vec3_t mag_soft_iron_scale;
    float      mag_ref_field_gauss;     /* 0 = take from calibration */

    /* Optional stored accelerometer bias from a 6-position calibration */
    imu_vec3_t accel_bias_g;

    /* Filtering */
    float    attitude_lpf_hz;
    float    attitude_valid_tol_g;
    float    heading_lpf_hz;
    float    mag_field_tol_frac;
    float    heading_max_jump_deg;
    uint32_t heading_outlier_limit;
    float    impact_lpf_hz;
    float    vib_lpf_hz;

    /* Tilt */
    float    tilt_threshold_deg;
    float    tilt_hysteresis_deg;
    uint32_t tilt_min_duration_ms;
    float    tilt_critical_deg;

    /* Turning */
    uint32_t turn_rate_window_ms;
    float    turn_enter_dps;
    float    turn_exit_dps;
    uint32_t turn_min_duration_ms;
    uint32_t turn_exit_duration_ms;
    float    heading_change_deg;
    bool     b_turn_allow_uncal_mag;

    /* Hump */
    float    hump_ascent_enter_deg;
    uint32_t hump_ascent_min_ms;
    float    hump_peak_drop_deg;
    float    hump_descent_enter_deg;
    uint32_t hump_descent_min_ms;
    float    hump_level_band_deg;
    uint32_t hump_level_min_ms;
    float    hump_min_peak_deg;
    float    hump_min_descent_deg;
    uint32_t hump_ascent_max_ms;
    uint32_t hump_min_event_ms;
    uint32_t hump_crest_max_ms;
    uint32_t hump_max_event_ms;
    uint32_t hump_rearm_level_ms;
    uint32_t hump_min_gap_ms;
    float    hump_impact_thr_scale;
    bool     b_hump_require_moving;
    float    hump_max_hint_accel_g;
    float    wheelbase_mm;
    float    hump_height_gain;
    float    hump_height_offset_mm;
    float    imu_pos_frac;
    bool     b_hump_height_calibrated;
    imu_height_method_t height_method;
    imu_rank_metric_t   rank_metric;

    /* Impact */
    float    impact_minor_g;
    float    impact_critical_g;
    float    impact_critical_any_g;
    float    impact_horiz_ratio;
    uint32_t impact_max_duration_ms;
    uint32_t impact_cooldown_ms;
    uint32_t impact_hold_ms;
    uint32_t impact_warmup_ms;
    bool     b_impact_minor_enters_fault;

    /* Motion classification */
    float    accel_enter_g;
    float    accel_exit_g;
    uint32_t class_hold_ms;
    int32_t  stationary_speed_mm_s;
    float    stationary_vib_g;
    uint32_t hint_max_age_ms;

    /* Event rate limiting */
    uint32_t motion_evt_min_ms;
    uint32_t tilt_evt_min_ms;
    uint32_t turn_evt_min_ms;
} imu_params_t;

/**
 * @brief  Fill @p p_params with the compile-time defaults above.
 * @param  p_params  destination (must not be NULL)
 */
void imu_params_default(imu_params_t * p_params);

/**
 * @brief  Range-check a parameter set (axis map handedness, ordering of
 *         hysteresis thresholds, non-zero periods, finite values...).
 * @return IMU_OK or IMU_ERR_PARAM.
 */
imu_err_t imu_params_validate(imu_params_t const * p_params);

/** @return accelerometer ODR in Hz for an LSM303 ODR code (0 if unknown). */
uint32_t imu_params_accel_odr_hz(uint8_t odr_code);

#endif /* IMU_CONFIG_H */
