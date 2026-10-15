/**
 * @file    imu_types.h
 * @brief   Buddy 4 (IMU / Terrain) - common types shared by all IMU modules.
 *
 * This header is RTOS-independent and hardware-independent so that the
 * algorithmic modules can be compiled and tested on a host PC.
 *
 * Units used throughout the Buddy 4 subsystem:
 *   - acceleration (specific force): g   (1 g = 9.80665 m/s^2)
 *   - magnetic field:                gauss
 *   - angles:                        degrees
 *   - angular rate:                  degrees per second
 *   - time:                          milliseconds since boot (uint32_t,
 *                                    wraps after ~49.7 days; all interval
 *                                    arithmetic uses unsigned subtraction)
 *
 * Vehicle frame (after axis mapping and mounting correction):
 *   +X = forward, +Y = left, +Z = up   (right-handed)
 *   pitch > 0 : nose up
 *   roll  > 0 : left side up (vehicle rolled towards its right)
 *   heading   : compass heading of +X, 0..360 deg, clockwise seen from
 *               above, so a LEFT turn DECREASES heading.
 */

#ifndef IMU_TYPES_H
#define IMU_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/** Error codes returned by Buddy 4 functions (0 = success). */
typedef enum
{
    IMU_OK                =   0,
    IMU_ERR_PARAM         =  -1,  /**< NULL pointer / out-of-range input  */
    IMU_ERR_I2C           =  -2,  /**< I2C NACK or bus error              */
    IMU_ERR_TIMEOUT       =  -3,  /**< I2C transfer timed out             */
    IMU_ERR_ID            =  -4,  /**< Device identity check failed       */
    IMU_ERR_NOT_READY     =  -5,  /**< Module not initialised             */
    IMU_ERR_INVALID_DATA  =  -6,  /**< Implausible sensor data            */
    IMU_ERR_CAL_MOTION    =  -7,  /**< Calibration: vehicle not still     */
    IMU_ERR_CAL_RANGE     =  -8,  /**< Calibration: result out of range   */
    IMU_ERR_CAL_SAMPLES   =  -9,  /**< Calibration: too few valid samples */
    IMU_ERR_BUSY          = -10,  /**< Shared data temporarily locked     */
    IMU_ERR_STATE         = -11,  /**< Request not allowed in this state  */
    IMU_ERR_QUEUE_FULL    = -12,  /**< Bounded queue full                 */
    IMU_ERR_NO_DATA       = -13,  /**< No new sample available            */
    IMU_ERR_CAL_COVERAGE  = -14,  /**< Mag calibration: rotation too small*/
    IMU_ERR_RTOS          = -15,  /**< RTOS object creation / call failed */
    IMU_ERR_VERIFY        = -16   /**< Register read-back mismatch       */
} imu_err_t;

/** Floating-point 3-axis vector. */
typedef struct
{
    float x;
    float y;
    float z;
} imu_vec3_t;

/** Raw signed 3-axis sensor sample (sensor frame, sensor counts). */
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} imu_raw3_t;

/** Confidence attached to measurements that are partly inferred. */
typedef enum
{
    IMU_CONF_NONE = 0,   /**< Not available                             */
    IMU_CONF_LOW,        /**< Inferred / uncalibrated / degraded        */
    IMU_CONF_MEDIUM,     /**< Measured, but with documented limitations */
    IMU_CONF_HIGH        /**< Measured and cross-checked                */
} imu_confidence_t;

/** Severity attached to events. */
typedef enum
{
    IMU_SEV_INFO = 0,
    IMU_SEV_MINOR,
    IMU_SEV_CRITICAL
} imu_severity_t;

/**
 * Motion classes. STATIONARY / STEADY_MOTION / ACCELERATING / TURNING_* /
 * CLIMBING_HUMP / DESCENDING_HUMP / SUDDEN_IMPACT come from the briefing.
 * Added (justified in docs Part 3):
 *   UNKNOWN       - no valid data (start-up, fault)
 *   STEADY_MOTION - no significant acceleration and vibration present:
 *                   an accelerometer cannot tell constant velocity from
 *                   standing still with motors running, so this class is
 *                   used instead of guessing STATIONARY.
 *   DECELERATING  - braking is physically distinct and must not be shown
 *                   as ACCELERATING or confused with an impact.
 */
typedef enum
{
    IMU_MOTION_UNKNOWN = 0,
    IMU_MOTION_STATIONARY,
    IMU_MOTION_STEADY_MOTION,
    IMU_MOTION_ACCELERATING,
    IMU_MOTION_DECELERATING,
    IMU_MOTION_TURNING_LEFT,
    IMU_MOTION_TURNING_RIGHT,
    IMU_MOTION_CLIMBING_HUMP,
    IMU_MOTION_DESCENDING_HUMP,
    IMU_MOTION_SUDDEN_IMPACT,
    IMU_MOTION_COUNT
} imu_motion_class_t;

/** Heading quality. */
typedef enum
{
    IMU_HEADING_INVALID = 0,     /**< no usable magnetometer data           */
    IMU_HEADING_UNCALIBRATED,    /**< raw field, hard-iron not removed      */
    IMU_HEADING_DISTURBED,       /**< |B| differs from calibrated reference */
    IMU_HEADING_GOOD
} imu_heading_quality_t;

/** Turning direction. */
typedef enum
{
    IMU_TURN_NONE = 0,
    IMU_TURN_LEFT,
    IMU_TURN_RIGHT
} imu_turn_dir_t;

/** Hump-processing phase (Design Review 6.4 HUMP PROCESSING region). */
typedef enum
{
    IMU_HUMP_MONITORING = 0,
    IMU_HUMP_ASCENT,
    IMU_HUMP_PEAK,
    IMU_HUMP_DESCENT
} imu_hump_phase_t;

/** Buddy 4 subsystem states (Design Review Section 6.4). */
typedef enum
{
    IMU_STATE_INITIALISATION = 0,
    IMU_STATE_CALIBRATION,
    IMU_STATE_MONITORING,
    IMU_STATE_HUMP_ASCENT,
    IMU_STATE_HUMP_PEAK,
    IMU_STATE_HUMP_DESCENT,
    IMU_STATE_TILT_EVENT,       /**< transient (run-to-completion)   */
    IMU_STATE_TURN_EVENT,       /**< transient (run-to-completion)   */
    IMU_STATE_IMPACT_EVENT,     /**< transient: classify impact      */
    IMU_STATE_FAULT_HANDLING,   /**< transient: report collision     */
    IMU_STATE_SENSOR_FAULT,     /**< transient: handle IMU error     */
    IMU_STATE_FAULT_SAFE,
    IMU_STATE_RECOVERY,
    IMU_STATE_STOPPED,
    IMU_STATE_COUNT
} imu_state_t;

/** Reason for being in a fault state. */
typedef enum
{
    IMU_FAULT_NONE = 0,
    IMU_FAULT_INIT_FAILED,
    IMU_FAULT_CAL_FAILED,
    IMU_FAULT_I2C,
    IMU_FAULT_STALE_DATA,
    IMU_FAULT_INVALID_DATA,
    IMU_FAULT_FROZEN_DATA,
    IMU_FAULT_COLLISION
} imu_fault_t;

/**
 * Optional motion hint supplied by Buddy 2 (encoders) through the Mission
 * Controller. Buddy 4 never reads Buddy 2's hardware; it only consumes this
 * structure if somebody provides it.
 */
typedef struct
{
    bool     b_valid;            /**< false = no encoder information      */
    bool     b_accel_valid;      /**< accel_mm_s2 field is meaningful     */
    int32_t  speed_mm_s;         /**< signed forward speed (+ = forward)  */
    int32_t  accel_mm_s2;        /**< signed forward acceleration         */
    uint32_t timestamp_ms;       /**< time the hint was produced          */
} imu_motion_hint_t;

#endif /* IMU_TYPES_H */
