#ifndef OBSTACLE_CONFIG_H
#define OBSTACLE_CONFIG_H

#include <stdint.h>

/*
 * Confirmed HC-SR04 connections.
 */
#define OBSTACLE_TRIG_GPIO                 26U
#define OBSTACLE_ECHO_GPIO                 6U

/*
 * Servo GPIO is not yet confirmed.
 * 255 prevents accidental use of the wrong GPIO.
 */
#define OBSTACLE_SERVO_GPIO                255U

/*
 * Ultrasonic configuration.
 */
#define OBSTACLE_MIN_DISTANCE_CM           2U
#define OBSTACLE_MAX_DISTANCE_CM           400U
#define OBSTACLE_ECHO_TIMEOUT_US           30000ULL

/*
 * Measurement validation.
 */
#define OBSTACLE_VALIDATION_SAMPLES        3U
#define OBSTACLE_VALIDATION_TOLERANCE_CM   5U
#define OBSTACLE_MAX_INVALID_READINGS      3U

/*
 * Obstacle thresholds.
 * Final values require physical calibration.
 */
#define OBSTACLE_DETECTION_THRESHOLD_CM    20U
#define OBSTACLE_SAFE_CLEARANCE_CM         25U

/*
 * If the obstacle is extremely close when profiled,
 * request a reverse action before turning.
 *
 * Final value requires physical testing.
 */
#define OBSTACLE_REVERSE_THRESHOLD_CM      10U

/*
 * Adaptive coarse scan.
 *
 * 30, 60, 90, 120, 150 degrees.
 */
#define OBSTACLE_COARSE_START_DEG          30U
#define OBSTACLE_COARSE_END_DEG            150U
#define OBSTACLE_COARSE_STEP_DEG           30U

#define OBSTACLE_COARSE_SAMPLE_COUNT \
    (((OBSTACLE_COARSE_END_DEG - \
       OBSTACLE_COARSE_START_DEG) / \
      OBSTACLE_COARSE_STEP_DEG) + 1U)

/*
 * Fine scan around the obstacle found
 * during the coarse scan.
 */
#define OBSTACLE_FINE_RANGE_DEG            20U
#define OBSTACLE_FINE_STEP_DEG             5U

#define OBSTACLE_FINE_SAMPLE_COUNT \
    (((OBSTACLE_FINE_RANGE_DEG * 2U) / \
      OBSTACLE_FINE_STEP_DEG) + 1U)

/*
 * Servo centre.
 */
#define OBSTACLE_SCAN_CENTER_DEG           90U

/*
 * Compatibility names used by other code.
 */
#define OBSTACLE_SCAN_START_DEG \
    OBSTACLE_COARSE_START_DEG

#define OBSTACLE_SCAN_END_DEG \
    OBSTACLE_COARSE_END_DEG

#define OBSTACLE_SCAN_STEP_DEG \
    OBSTACLE_COARSE_STEP_DEG

#define OBSTACLE_SCAN_SAMPLE_COUNT \
    OBSTACLE_COARSE_SAMPLE_COUNT

/*
 * Scan validation.
 */
#define OBSTACLE_MIN_VALID_SCAN_SAMPLES    3U
#define OBSTACLE_MIN_VALID_SIDE_SAMPLES    2U

/*
 * Obstacle profiling.
 */
#define OBSTACLE_PROFILE_EDGE_MARGIN_CM    15U

/*
 * Servo PWM configuration.
 * Requires physical calibration later.
 */
#define OBSTACLE_SERVO_MIN_PULSE_US        1000U
#define OBSTACLE_SERVO_MAX_PULSE_US        2000U
#define OBSTACLE_SERVO_PERIOD_US           20000U
#define OBSTACLE_SERVO_FREQUENCY_HZ        50U
#define OBSTACLE_SERVO_SETTLE_MS           150U

/*
 * Safety timeouts.
 */
#define OBSTACLE_STOP_TIMEOUT_MS           2000U
#define OBSTACLE_BYPASS_TIMEOUT_MS         10000U
#define OBSTACLE_LINE_RECOVERY_TIMEOUT_MS  8000U

/*
 * Standalone development loop.
 */
#define OBSTACLE_TEST_LOOP_DELAY_MS         50U

#endif