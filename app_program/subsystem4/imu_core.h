/** @file imu_core.h
 * Buddy 4 pure logic. See testing/subsystem4/TEST_LOG.md for test scope.
 * Contexts have one owner; callers provide synchronization when shared.
 */
#ifndef IMU_CORE_H
#define IMU_CORE_H

#include <stdbool.h>
#include <stdint.h>

typedef float float32_t;

typedef enum
{
    IMU_CALIBRATING,
    IMU_MONITORING,
    IMU_ASCENDING,
    IMU_PEAK,
    IMU_DESCENDING,
    IMU_FAULT
} imu_core_phase_t;

typedef enum
{
    IMU_MOTION_UNKNOWN,
    IMU_STATIONARY,
    IMU_MOVING,
    IMU_ACCELERATING,
    IMU_TURNING,
    IMU_CLIMBING,
    IMU_DESCENDING_MOTION,
    IMU_IMPACT
} imu_core_motion_t;

#define IMU_EVENT_CALIBRATED 0x01u
#define IMU_EVENT_ASCENT     0x02u
#define IMU_EVENT_PEAK       0x04u
#define IMU_EVENT_DESCENT    0x08u
#define IMU_EVENT_COMPLETE   0x10u
#define IMU_EVENT_TURN       0x20u
#define IMU_EVENT_FAULT      0x40u
#define IMU_EVENT_IMPACT     0x80u
#define IMU_EVENT_HEIGHT_INVALID 0x100u

typedef struct
{
    float32_t accel_g[3];       /**< Body axes: X forward, Y left, Z up. */
    float32_t mag_gauss[3];
    uint32_t  time_ms;          /**< Monotonic modulo 2^32 milliseconds. */
    float32_t distance_mm;      /**< Forward path since previous sample. */
    bool      b_distance_valid;
    bool      b_mag_valid;
    float32_t speed_mm_s;       /**< Signed mean wheel speed; never abs(). */
    float32_t wheel_delta_mm_s; /**< Absolute left/right speed difference. */
    uint32_t  motion_time_ms;   /**< Source clock; used only for differences. */
    uint32_t  motion_age_ms;    /**< Age checked in the source clock. */
    uint32_t  motion_epoch;     /**< Changes on explicit trust revocation. */
    bool      b_motion_valid;
    bool      b_straight;
    bool      b_traction_trusted;
} imu_core_sample_t;

typedef struct
{
    float32_t ascend_deg;
    float32_t level_deg;
    float32_t impact_g;
    float32_t turn_dps;
    float32_t mag_offset[3];
    float32_t mag_scale[3];
    uint32_t  calibration_samples;
    uint32_t  maximum_gap_ms;
    uint32_t  maximum_hump_ms;
    bool      b_mag_calibrated;
    /* Measured operating envelope. Zero/default means height is disabled. */
    float32_t maximum_accel_mm_s2;
    float32_t speed_band_mm_s;
    float32_t wheel_delta_mm_s;
    float32_t minimum_speed_mm_s;
    uint32_t  settle_ms;
    uint32_t  ascent_ms;
    uint32_t  level_ms;
    bool      b_height_calibrated;
} imu_core_config_t;

typedef struct
{
    imu_core_phase_t  phase;
    imu_core_motion_t motion;
    uint32_t          time_ms;
    uint32_t          events;  /**< Transitions from this update only. */
    float32_t         pitch_deg;
    float32_t         roll_deg;
    float32_t         heading_deg;
    float32_t         turn_dps;
    float32_t         hump_peak_mm;
    float32_t         highest_hump_mm;
    bool              b_heading_valid;
    bool              b_height_valid;
    bool              b_highest_valid;
    uint32_t          height_reasons;
    bool              b_peak_provisional;
} imu_core_status_t;

typedef struct
{
    imu_core_config_t config;
    imu_core_status_t status;
    float32_t         bias[3];
    float32_t         sum[3];
    float32_t         previous_accel[3];
    float32_t         height_mm;
    float32_t         previous_sine;
    float32_t         previous_heading;
    uint32_t          count;
    uint32_t          hump_start_ms;
    uint32_t          heading_time_ms;
    bool              b_time_valid;
    bool              b_previous_heading;
    bool              b_hump_height_valid;
    float32_t         previous_speed;
    float32_t         stable_speed;
    float32_t         saved_highest_mm;
    uint32_t          previous_motion_ms;
    uint32_t          previous_motion_epoch;
    uint32_t          stable_since_ms;
    uint32_t          level_since_ms;
    bool              b_motion_baseline;
    bool              b_motion_stable;
    bool              b_level_pending;
    bool              b_saved_highest_valid;
    bool              b_peak_recorded;
} imu_core_t;

#define IMU_HEIGHT_UNCALIBRATED 0x001u
#define IMU_HEIGHT_MOTION       0x002u
#define IMU_HEIGHT_REVERSE      0x004u
#define IMU_HEIGHT_TURN         0x008u
#define IMU_HEIGHT_TRACTION     0x010u
#define IMU_HEIGHT_DYNAMIC      0x020u
#define IMU_HEIGHT_PERSISTENCE  0x040u
#define IMU_HEIGHT_REVOKED      0x080u
#define IMU_MOTION_MAX_AGE_MS    40u

/** Development settings, NOT measured calibration or accepted thresholds. */
imu_core_config_t imu_core_defaults (void);
bool imu_core_init (imu_core_t *p_core,
                    const imu_core_config_t *p_config);
/** Requires stationary, level startup; faults stay latched until re-init. */
uint32_t imu_core_update (imu_core_t *p_core,
                          const imu_core_sample_t *p_sample);
void imu_core_fault (imu_core_t *p_core);

#endif /* IMU_CORE_H */

