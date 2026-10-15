/** @file imu_core.h
 * Buddy 4 pure sensing logic. See testing/buddy4/README.md for assumptions.
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

typedef struct
{
    float32_t accel_g[3];       /**< Body axes: X forward, Y left, Z up. */
    float32_t mag_gauss[3];
    uint32_t  time_ms;          /**< Monotonic modulo 2^32 milliseconds. */
    float32_t distance_mm;      /**< Forward path since previous sample. */
    bool      b_distance_valid;
    bool      b_mag_valid;
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
} imu_core_t;

/** Development settings, NOT measured calibration or accepted thresholds. */
imu_core_config_t imu_core_defaults (void);
bool imu_core_init (imu_core_t *p_core,
                    const imu_core_config_t *p_config);
/** Requires stationary, level startup; faults stay latched until re-init. */
uint32_t imu_core_update (imu_core_t *p_core,
                          const imu_core_sample_t *p_sample);
void imu_core_fault (imu_core_t *p_core);

#endif /* IMU_CORE_H */

