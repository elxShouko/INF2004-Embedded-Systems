/** @file imu_config.h
 * Board enable is deliberately opt-in until wiring/pull-ups are verified.
 * All sources build by default; the enabled profile retains hardware paths.
 */
#ifndef IMU_CONFIG_H
#define IMU_CONFIG_H

#ifndef S4_IMU_ENABLE
#define S4_IMU_ENABLE 0
#endif

#define S4_IMU_SDA 16u
#define S4_IMU_SCL 17u
#define S4_IMU_BAUD_HZ 100000u
#define S4_IMU_BUS_TIMEOUT_US 2000u
#define S4_IMU_PERIOD_MS 20u
#define S4_IMU_CAL_TIMEOUT_MS 10000u
#define S4_IMU_PRIORITY 19
#define S4_IMU_EVENT_DEPTH 8u

/* Replace only with measured hard-iron/axis-scale calibration. The sensor
 * must be mounted X forward, Y left, Z up; otherwise add a verified map.
 * Keeping false prevents uncalibrated turn estimates being marked valid. */
#define S4_MAG_CALIBRATED false
#define S4_MAG_OFFSET_X 0.0f
#define S4_MAG_OFFSET_Y 0.0f
#define S4_MAG_OFFSET_Z 0.0f
#define S4_MAG_SCALE_X 1.0f
#define S4_MAG_SCALE_Y 1.0f
#define S4_MAG_SCALE_Z 1.0f

/* Set only from recorded hardware calibration. Disabled by default. */
#define S4_HEIGHT_CALIBRATED false
#define S4_HEIGHT_MAX_ACCEL_MM_S2 0.0f
#define S4_HEIGHT_SPEED_BAND_MM_S 0.0f
#define S4_HEIGHT_WHEEL_DELTA_MM_S 0.0f
#define S4_HEIGHT_MIN_SPEED_MM_S 0.0f
#define S4_HEIGHT_SETTLE_MS 0u
#define S4_HEIGHT_ASCENT_MS 0u
#define S4_HEIGHT_LEVEL_MS 0u

#endif /* IMU_CONFIG_H */

