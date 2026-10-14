/**
 * @file motion_pid.h
 * @brief Pure signed wheel-speed PID; normalized effort, never raw PWM.
 */
#ifndef MOTION_PID_H
#define MOTION_PID_H

#include "motion_config.h"
#include <float.h>

/* Explicit binary32 contract; do not silently change control precision. */
#if (FLT_RADIX != 2) || (FLT_MANT_DIG != 24) || (FLT_MAX_EXP != 128)
#error "motion_pid requires binary32 float"
#endif
#if defined(__FAST_MATH__) || (__FINITE_MATH_ONLY__ > 0)
#error "motion_pid requires nonfinite checks; disable fast/finite-only math"
#endif

/* Barr-C 5.4 deviation: retain a module-prefixed float alias because C99
 * supplies no standard float32_t. The guards above establish binary32.
 */
typedef float motion_pid_value_t;

typedef enum
{
    MOTION_PID_OK = 0,
    MOTION_PID_IDLE,
    MOTION_PID_NULL,
    MOTION_PID_UNINITIALISED,
    MOTION_PID_BAD_CONFIG,
    MOTION_PID_BAD_TIME,
    MOTION_PID_ARITHMETIC
} motion_pid_result_t;

typedef struct
{
    /* Gain units: effort/(mm/s), effort/((mm/s)*s), effort*s/(mm/s). */
    motion_pid_value_t proportional_gain;
    motion_pid_value_t integral_gain;
    motion_pid_value_t derivative_gain;
    /* Finite normalized effort bounds inside [-1,+1], containing zero. */
    motion_pid_value_t output_min;
    motion_pid_value_t output_max;
    /* Finite integral-effort bounds containing zero; equal zero disables I. */
    motion_pid_value_t integral_min;
    motion_pid_value_t integral_max;
    uint32_t max_elapsed_us;
    motion_config_calibration_t calibration;
} motion_pid_config_t;

/* Owner-only state; configuration is copied at init and must not be edited
 * while active. Re-init after calibration changes. Zero state is disabled.
 */
typedef struct
{
    motion_pid_config_t config;
    motion_pid_value_t integral;
    motion_types_speed_mm_s_t previous_measurement_mm_s;
    motion_types_speed_mm_s_t previous_target_mm_s;
    bool b_have_measurement;
    bool b_initialised;
} motion_pid_t;

/* Requires explicit CONFIRMED, nonnegative finite gains (Kd=0 supported),
 * valid bounds and nonzero max_elapsed_us. All-zero gains are safe but do
 * not regulate motion. Confirmation is not a stability or actuation claim.
 */
motion_pid_result_t motion_pid_validate (motion_pid_config_t const * p_config);
motion_pid_result_t motion_pid_init (motion_pid_t * p_controller,
                                   motion_pid_config_t const * p_config);
motion_pid_result_t motion_pid_reset (motion_pid_t * p_controller);

/* P + I - Kd*d(measurement)/dt, dt = elapsed_us / 1e6 seconds.
 * Itrial = clamp(Iold + Ki*error*dt, integral_min, integral_max).
 * A rising saturation-driving Itrial is clipped to max(Iold,
 * min(Itrial, output_max - (P + D))); falling I uses the symmetric minimum
 * and output_min. This clips the increment, allows crossing/recovery and
 * never injects an opposite integral solely to cancel saturated P/D.
 * Target reversal clears I, retaining D's
 * measurement history. First active update uses D=0.
 * Inactive or zero target clears I/history and returns IDLE with zero effort;
 * this is NOT a physical stop/brake implementation.
 * Invalid timing clears dynamics, returning zero; a later valid call may
 * restart. Nonfinite arithmetic/config disables the instance until init.
 * No floating-point effort is usable unless result == OK (IDLE is zero).
 * Separate instances are required for each wheel. No allocations or RTOS.
 * p_effort must not alias instance state or configuration.
 * Caller must supply a valid, fresh measured speed, not a stale/unavailable
 * estimator value. Uses nontrapping IEEE arithmetic; target execution cost
 * and floating-point runtime behaviour still need firmware verification.
 */
motion_pid_result_t motion_pid_update (motion_pid_t * p_controller,
    motion_types_speed_mm_s_t target_mm_s,
    motion_types_speed_mm_s_t measured_mm_s, uint32_t elapsed_us,
    bool b_active, motion_pid_value_t * p_effort);

#endif /* MOTION_PID_H */

/*** end of file ***/

