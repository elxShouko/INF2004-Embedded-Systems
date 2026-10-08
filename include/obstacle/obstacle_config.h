#ifndef OBSTACLE_CONFIG_H
#define OBSTACLE_CONFIG_H

#include <stdint.h>

/*
 * Confirmed HC-SR04 connections.
 */
#define OBSTACLE_TRIG_GPIO                 26U
#define OBSTACLE_ECHO_GPIO                 6U

/*
 * IMPORTANT:
 * Servo GPIO has not yet been confirmed.
 *
 * 255 prevents accidental GPIO configuration.
 * Replace this value only after the physical servo signal
 * connection has been confirmed.
 */
#define OBSTACLE_SERVO_GPIO                255U

/*
 * Ultrasonic measurement settings.
 * These values should be calibrated during hardware testing.
 */
#define OBSTACLE_MIN_DISTANCE_CM           2U
#define OBSTACLE_MAX_DISTANCE_CM           400U
#define OBSTACLE_ECHO_TIMEOUT_US           30000ULL

#define OBSTACLE_VALIDATION_SAMPLES        3U
#define OBSTACLE_VALIDATION_TOLERANCE_CM   5U
#define OBSTACLE_MAX_INVALID_READINGS      3U

/*
 * Initial obstacle thresholds.
 * Final values must be validated experimentally.
 */
#define OBSTACLE_DETECTION_THRESHOLD_CM    20U
#define OBSTACLE_SAFE_CLEARANCE_CM         25U

/*
 * High-resolution servo scan.
 *
 * Monitor normally faces forward at 90 degrees.
 * Once an obstacle is confirmed, scan from 30 to 150 degrees
 * in 10-degree increments.
 */
#define OBSTACLE_SCAN_START_DEG            30U
#define OBSTACLE_SCAN_END_DEG              150U
#define OBSTACLE_SCAN_STEP_DEG             10U
#define OBSTACLE_SCAN_CENTER_DEG           90U

#define OBSTACLE_SCAN_SAMPLE_COUNT \
    (((OBSTACLE_SCAN_END_DEG - OBSTACLE_SCAN_START_DEG) / \
      OBSTACLE_SCAN_STEP_DEG) + 1U)

#define OBSTACLE_MIN_VALID_SCAN_SAMPLES    7U
#define OBSTACLE_MIN_VALID_SIDE_SAMPLES    3U

/*
 * Used to estimate where the obstacle edges are.
 */
#define OBSTACLE_PROFILE_EDGE_MARGIN_CM    15U

/*
 * Servo timings.
 * Conservative starting values; hardware calibration required.
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
 * Temporary standalone main-loop period.
 * Later µT-Kernel tasks will provide their own scheduling.
 */
#define OBSTACLE_TEST_LOOP_DELAY_MS         50U

#endif