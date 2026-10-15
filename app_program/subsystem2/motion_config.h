/**
 * @file motion_config.h
 * @brief Pure Buddy 2 calibration configuration for Milestone 1A.
 *
 * Internal candidate types only. No RTOS, GPIO, command transport or motor
 * control is provided. Runtime configuration management and synchronisation
 * are not implemented here.
 */
#ifndef MOTION_CONFIG_H
#define MOTION_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "motion_types.h"

/* Week 6 design assignments, cross-checked against the Robo Pico datasheet.
 * These constants do NOT establish the robot's actual wiring or polarity.
 * GP0/1: incompatible with UART console/crash reporting.
 * GP28: check the VBAT jumper and ADC pin setup.
 * GP8/9: conflict with the currently disabled BSP I2C0 pin configuration.
 */
#define MOTION_CONFIG_LEFT_MOTOR_A_GPIO     (8U)
#define MOTION_CONFIG_LEFT_MOTOR_B_GPIO     (9U)
#define MOTION_CONFIG_RIGHT_MOTOR_A_GPIO    (10U)
#define MOTION_CONFIG_RIGHT_MOTOR_B_GPIO    (11U)
#define MOTION_CONFIG_LEFT_ENCODER_A_GPIO   (7U)
#define MOTION_CONFIG_LEFT_ENCODER_B_GPIO   (28U)
#define MOTION_CONFIG_RIGHT_ENCODER_A_GPIO  (0U)
#define MOTION_CONFIG_RIGHT_ENCODER_B_GPIO  (1U)

typedef enum
{
    MOTION_CONFIG_CALIBRATION_UNCONFIGURED = 0,
    MOTION_CONFIG_CALIBRATION_UNVERIFIED,
    MOTION_CONFIG_CALIBRATION_CONFIRMED
} motion_config_calibration_t;

typedef struct
{
    /* Decoded counts per WHEEL revolution, including the selected decoding
     * multiplier and any gearing exactly once. Zero means unknown.
     */
    uint32_t counts_per_wheel_rev;
    /* Effective loaded-wheel travel per revolution, in micrometres.
     * Store circumference rather than redundant diameter/gearing fields.
     */
    uint32_t wheel_circumference_um;
    motion_types_polarity_t motor_polarity;
    motion_types_polarity_t encoder_polarity;
} motion_config_wheel_t;

typedef struct
{
    motion_config_wheel_t left;
    motion_config_wheel_t right;
    /* Effective lateral track width for differential turns, NOT wheelbase. */
    uint32_t track_width_um;
    /* Positive magnitude limiting both signed wheel-speed requests, mm/s. */
    motion_types_speed_mm_s_t max_wheel_speed_mm_s;
    /* CONFIRMED is a caller assertion backed by physical evidence, not a
     * conclusion the validator can derive, and is never independently
     * sufficient to authorise motor actuation. Future motion code must use
     * a validated, immutable configuration snapshot throughout an active run.
     * Do not edit that active snapshot. Any configuration update must return
     * the candidate to UNVERIFIED and require revalidation plus explicit
     * calibration reconfirmation before use in a subsequent run.
     * This lifecycle is a caller requirement; M1A does not enforce it.
     */
    motion_config_calibration_t calibration;
} motion_config_t;

typedef enum
{
    MOTION_CONFIG_VALID = 0,
    MOTION_CONFIG_NULL,
    MOTION_CONFIG_UNCONFIGURED,
    MOTION_CONFIG_BAD_CALIBRATION,
    MOTION_CONFIG_BAD_LEFT_WHEEL,
    MOTION_CONFIG_BAD_RIGHT_WHEEL,
    MOTION_CONFIG_BAD_TRACK_WIDTH,
    MOTION_CONFIG_BAD_SPEED_LIMIT,
    MOTION_CONFIG_UNVERIFIED
} motion_config_result_t;

/* Reset every field to its safe, unknown state. False for NULL, else true.
 * No measured values, timing or PID defaults are supplied.
 */
bool motion_config_init (motion_config_t * p_config);

/* Read-only, deterministic first-error validation: calibration state, left
 * wheel, right wheel, track width, speed limit, then confirmation evidence.
 * UNCONFIGURED short-circuits; use UNVERIFIED while preparing a candidate.
 * VALID checks only the M1A subset. It does NOT authorise/enable motion or
 * certify physical safety: future PID, timing, stop/fault and hardware
 * checks remain mandatory. Positive values alone do not prove plausibility.
 */
motion_config_result_t motion_config_validate (
    motion_config_t const * p_config);

#endif /* MOTION_CONFIG_H */

/*** end of file ***/

