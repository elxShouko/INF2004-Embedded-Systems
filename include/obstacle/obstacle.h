#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    OBSTACLE_STATE_MONITOR = 0,
    OBSTACLE_STATE_VALIDATE,
    OBSTACLE_STATE_REQUEST_STOP,
    OBSTACLE_STATE_SCANNING,
    OBSTACLE_STATE_BUILD_PROFILE,
    OBSTACLE_STATE_PLAN_BYPASS,
    OBSTACLE_STATE_BYPASS,
    OBSTACLE_STATE_LINE_RECOVERY,
    OBSTACLE_STATE_FAULT
} obstacle_state_t;

typedef enum
{
    BYPASS_NONE = 0,
    BYPASS_LEFT,
    BYPASS_RIGHT,
    BYPASS_NO_SAFE_ROUTE
} bypass_direction_t;

typedef enum
{
    OBSTACLE_FAULT_NONE = 0,
    OBSTACLE_FAULT_SENSOR_TIMEOUT,
    OBSTACLE_FAULT_REPEATED_INVALID_READINGS,
    OBSTACLE_FAULT_SERVO,
    OBSTACLE_FAULT_SCAN_INVALID,
    OBSTACLE_FAULT_NO_SAFE_BYPASS,
    OBSTACLE_FAULT_STOP_TIMEOUT,
    OBSTACLE_FAULT_BYPASS_TIMEOUT,
    OBSTACLE_FAULT_LINE_RECOVERY_TIMEOUT
} obstacle_fault_t;

typedef struct
{
    uint8_t angle_deg;
    uint32_t distance_cm;
    bool valid;
} obstacle_scan_sample_t;

typedef struct
{
    bool valid;

    uint32_t closest_distance_cm;
    uint8_t closest_angle_deg;

    int32_t closest_lateral_offset_cm;
    uint32_t closest_forward_distance_cm;

    uint32_t estimated_width_cm;

    uint32_t left_clearance_cm;
    uint32_t right_clearance_cm;

    uint8_t left_valid_samples;
    uint8_t right_valid_samples;

    bypass_direction_t bypass_direction;
} obstacle_profile_t;

/*
 * These callbacks are the interface from Buddy 5
 * to the Mission Controller.
 *
 * Buddy 5 never directly controls the motors.
 */
typedef struct
{
    void (*request_stop)(void);

    void (*request_bypass)(
        bypass_direction_t direction,
        const obstacle_profile_t *profile);

    void (*request_line_recovery)(void);

    void (*request_fault)(
        obstacle_fault_t fault);
} obstacle_callbacks_t;

bool obstacle_system_init(
    const obstacle_callbacks_t *callbacks);

/*
 * Intended for the high-priority periodic
 * Obstacle Monitor Task.
 */
void obstacle_monitor_step(void);

/*
 * Intended for the medium-high priority
 * event-driven Obstacle Handling Task.
 */
void obstacle_handling_step(void);

void obstacle_reset(void);

/*
 * Notifications received from other subsystems
 * through the Mission Controller.
 */
void obstacle_notify_vehicle_stopped(void);

void obstacle_notify_bypass_complete(void);

void obstacle_notify_line_found(void);

/*
 * Telemetry / status getters.
 */
obstacle_state_t obstacle_get_state(void);

obstacle_fault_t obstacle_get_fault(void);

const obstacle_profile_t *
obstacle_get_profile(void);

const obstacle_scan_sample_t *
obstacle_get_scan_samples(
    uint32_t *sample_count);

uint32_t obstacle_get_forward_distance_cm(void);

const char *obstacle_state_name(
    obstacle_state_t state);

const char *obstacle_fault_name(
    obstacle_fault_t fault);

const char *obstacle_bypass_name(
    bypass_direction_t direction);

#endif