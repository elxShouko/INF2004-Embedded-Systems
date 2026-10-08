#include "obstacle/obstacle.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#include "obstacle/obstacle_config.h"
#include "obstacle/servo.h"
#include "obstacle/ultrasonic.h"

#include "pico/stdlib.h"

#define DEG_TO_RAD \
    (3.14159265358979323846f / 180.0f)

static obstacle_state_t current_state =
    OBSTACLE_STATE_MONITOR;

static obstacle_fault_t current_fault =
    OBSTACLE_FAULT_NONE;

static obstacle_callbacks_t system_callbacks;

static obstacle_profile_t current_profile;

static obstacle_scan_sample_t
    scan_samples[OBSTACLE_SCAN_SAMPLE_COUNT];

static uint32_t last_forward_distance_cm = 0U;

static uint32_t consecutive_invalid_readings = 0U;

static bool stop_request_sent = false;

static bool vehicle_stopped_received = false;
static bool bypass_complete_received = false;
static bool line_found_received = false;

static uint64_t state_entry_time_ms = 0ULL;

static uint64_t get_time_ms(void)
{
    return time_us_64() / 1000ULL;
}

static void set_state(
    obstacle_state_t new_state)
{
    current_state = new_state;
    state_entry_time_ms = get_time_ms();
}

static bool state_timeout_reached(
    uint32_t timeout_ms)
{
    return
        (get_time_ms() -
         state_entry_time_ms) >=
        (uint64_t)timeout_ms;
}

static obstacle_fault_t map_sensor_fault(
    ultrasonic_status_t status)
{
    obstacle_fault_t fault;

    if ((status ==
         ULTRASONIC_STATUS_TIMEOUT_RISE) ||
        (status ==
         ULTRASONIC_STATUS_TIMEOUT_FALL))
    {
        fault =
            OBSTACLE_FAULT_SENSOR_TIMEOUT;
    }
    else
    {
        fault =
            OBSTACLE_FAULT_REPEATED_INVALID_READINGS;
    }

    return fault;
}

static void enter_fault(
    obstacle_fault_t fault)
{
    current_fault = fault;

    set_state(
        OBSTACLE_STATE_FAULT);

    if (system_callbacks.request_fault != NULL)
    {
        system_callbacks.request_fault(fault);
    }
}

static obstacle_fault_t perform_scan(void)
{
    uint32_t valid_samples;
    uint32_t distance_cm;
    uint8_t angle_deg;
    ultrasonic_status_t status;

    valid_samples = 0U;

    for (uint32_t index = 0U;
         index < OBSTACLE_SCAN_SAMPLE_COUNT;
         index++)
    {
        angle_deg =
            (uint8_t)(
                OBSTACLE_SCAN_START_DEG +
                (index *
                 OBSTACLE_SCAN_STEP_DEG));

        scan_samples[index].angle_deg =
            angle_deg;

        scan_samples[index].distance_cm =
            0U;

        scan_samples[index].valid =
            false;

        if (!servo_set_angle(angle_deg))
        {
            return OBSTACLE_FAULT_SERVO;
        }

        sleep_ms(
            OBSTACLE_SERVO_SETTLE_MS);

        status =
            ultrasonic_get_validated_distance_cm(
                &distance_cm);

        if (status ==
            ULTRASONIC_STATUS_OK)
        {
            scan_samples[index].distance_cm =
                distance_cm;

            scan_samples[index].valid =
                true;

            valid_samples++;
        }
    }

    if (!servo_set_angle(
            OBSTACLE_SCAN_CENTER_DEG))
    {
        return OBSTACLE_FAULT_SERVO;
    }

    if (valid_samples <
        OBSTACLE_MIN_VALID_SCAN_SAMPLES)
    {
        return OBSTACLE_FAULT_SCAN_INVALID;
    }

    return OBSTACLE_FAULT_NONE;
}

static bool build_profile(void)
{
    uint32_t closest_distance;
    uint32_t left_minimum;
    uint32_t right_minimum;

    uint32_t left_count;
    uint32_t right_count;

    uint32_t first_obstacle_index;
    uint32_t last_obstacle_index;

    bool obstacle_index_found;

    float relative_angle_rad;
    float half_span_rad;
    float estimated_width;

    closest_distance = UINT32_MAX;

    left_minimum = UINT32_MAX;
    right_minimum = UINT32_MAX;

    left_count = 0U;
    right_count = 0U;

    /*
     * Find closest point and conservative
     * left/right clearances.
     */
    for (uint32_t index = 0U;
         index < OBSTACLE_SCAN_SAMPLE_COUNT;
         index++)
    {
        if (!scan_samples[index].valid)
        {
            continue;
        }

        if (scan_samples[index].distance_cm <
            closest_distance)
        {
            closest_distance =
                scan_samples[index].distance_cm;

            current_profile.closest_angle_deg =
                scan_samples[index].angle_deg;
        }

        if (scan_samples[index].angle_deg <
            OBSTACLE_SCAN_CENTER_DEG)
        {
            left_count++;

            if (scan_samples[index].distance_cm <
                left_minimum)
            {
                left_minimum =
                    scan_samples[index].distance_cm;
            }
        }
        else if (scan_samples[index].angle_deg >
                 OBSTACLE_SCAN_CENTER_DEG)
        {
            right_count++;

            if (scan_samples[index].distance_cm <
                right_minimum)
            {
                right_minimum =
                    scan_samples[index].distance_cm;
            }
        }
    }

    if (closest_distance == UINT32_MAX)
    {
        return false;
    }

    current_profile.closest_distance_cm =
        closest_distance;

    current_profile.left_valid_samples =
        (uint8_t)left_count;

    current_profile.right_valid_samples =
        (uint8_t)right_count;

    if (left_count >=
        OBSTACLE_MIN_VALID_SIDE_SAMPLES)
    {
        current_profile.left_clearance_cm =
            left_minimum;
    }
    else
    {
        current_profile.left_clearance_cm =
            0U;
    }

    if (right_count >=
        OBSTACLE_MIN_VALID_SIDE_SAMPLES)
    {
        current_profile.right_clearance_cm =
            right_minimum;
    }
    else
    {
        current_profile.right_clearance_cm =
            0U;
    }

    /*
     * Estimate obstacle position relative to
     * the centre of the robot.
     */
    relative_angle_rad =
        ((float)current_profile.closest_angle_deg -
         (float)OBSTACLE_SCAN_CENTER_DEG) *
        DEG_TO_RAD;

    current_profile.closest_lateral_offset_cm =
        (int32_t)lroundf(
            (float)closest_distance *
            sinf(relative_angle_rad));

    current_profile.closest_forward_distance_cm =
        (uint32_t)lroundf(
            (float)closest_distance *
            cosf(relative_angle_rad));

    /*
     * Approximate obstacle width.
     *
     * Samples whose distance is near the closest
     * detected surface are treated as belonging to
     * the obstacle.
     */
    first_obstacle_index = 0U;
    last_obstacle_index = 0U;
    obstacle_index_found = false;

    for (uint32_t index = 0U;
         index < OBSTACLE_SCAN_SAMPLE_COUNT;
         index++)
    {
        if (!scan_samples[index].valid)
        {
            continue;
        }

        if (scan_samples[index].distance_cm <=
            (closest_distance +
             OBSTACLE_PROFILE_EDGE_MARGIN_CM))
        {
            if (!obstacle_index_found)
            {
                first_obstacle_index = index;
                obstacle_index_found = true;
            }

            last_obstacle_index = index;
        }
    }

    current_profile.estimated_width_cm = 0U;

    if (obstacle_index_found &&
        (last_obstacle_index >
         first_obstacle_index))
    {
        uint32_t angular_span_deg;

        angular_span_deg =
            (last_obstacle_index -
             first_obstacle_index) *
            OBSTACLE_SCAN_STEP_DEG;

        half_span_rad =
            ((float)angular_span_deg *
             0.5f) *
            DEG_TO_RAD;

        estimated_width =
            2.0f *
            (float)closest_distance *
            tanf(half_span_rad);

        if (estimated_width > 0.0f)
        {
            current_profile.estimated_width_cm =
                (uint32_t)lroundf(
                    estimated_width);
        }
    }

    current_profile.valid = true;

    return true;
}

static void plan_bypass(void)
{
    bool left_safe;
    bool right_safe;

    left_safe =
        (current_profile.left_valid_samples >=
         OBSTACLE_MIN_VALID_SIDE_SAMPLES) &&
        (current_profile.left_clearance_cm >=
         OBSTACLE_SAFE_CLEARANCE_CM);

    right_safe =
        (current_profile.right_valid_samples >=
         OBSTACLE_MIN_VALID_SIDE_SAMPLES) &&
        (current_profile.right_clearance_cm >=
         OBSTACLE_SAFE_CLEARANCE_CM);

    if (left_safe && right_safe)
    {
        if (current_profile.left_clearance_cm >=
            current_profile.right_clearance_cm)
        {
            current_profile.bypass_direction =
                BYPASS_LEFT;
        }
        else
        {
            current_profile.bypass_direction =
                BYPASS_RIGHT;
        }
    }
    else if (left_safe)
    {
        current_profile.bypass_direction =
            BYPASS_LEFT;
    }
    else if (right_safe)
    {
        current_profile.bypass_direction =
            BYPASS_RIGHT;
    }
    else
    {
        current_profile.bypass_direction =
            BYPASS_NO_SAFE_ROUTE;
    }
}

bool obstacle_system_init(
    const obstacle_callbacks_t *callbacks)
{
    memset(
        &system_callbacks,
        0,
        sizeof(system_callbacks));

    memset(
        &current_profile,
        0,
        sizeof(current_profile));

    memset(
        scan_samples,
        0,
        sizeof(scan_samples));

    if (callbacks != NULL)
    {
        system_callbacks = *callbacks;
    }

    ultrasonic_init();

    current_fault =
        OBSTACLE_FAULT_NONE;

    consecutive_invalid_readings = 0U;

    vehicle_stopped_received = false;
    bypass_complete_received = false;
    line_found_received = false;

    stop_request_sent = false;

    last_forward_distance_cm = 0U;

    set_state(
        OBSTACLE_STATE_MONITOR);

    if (!servo_init(
            OBSTACLE_SERVO_GPIO))
    {
        enter_fault(
            OBSTACLE_FAULT_SERVO);

        return false;
    }

    return true;
}

void obstacle_monitor_step(void)
{
    uint32_t distance_cm;
    ultrasonic_status_t status;

    if (current_state !=
        OBSTACLE_STATE_MONITOR)
    {
        return;
    }

    status =
        ultrasonic_read_distance_cm(
            &distance_cm);

    if (status ==
        ULTRASONIC_STATUS_OK)
    {
        consecutive_invalid_readings = 0U;

        last_forward_distance_cm =
            distance_cm;

        if (distance_cm <=
            OBSTACLE_DETECTION_THRESHOLD_CM)
        {
            set_state(
                OBSTACLE_STATE_VALIDATE);
        }
    }
    else
    {
        consecutive_invalid_readings++;

        if (consecutive_invalid_readings >=
            OBSTACLE_MAX_INVALID_READINGS)
        {
            enter_fault(
                map_sensor_fault(status));
        }
    }
}

void obstacle_handling_step(void)
{
    uint32_t distance_cm;
    ultrasonic_status_t status;
    obstacle_fault_t scan_fault;

    switch (current_state)
    {
        case OBSTACLE_STATE_MONITOR:
            break;

        case OBSTACLE_STATE_VALIDATE:

            status =
                ultrasonic_get_validated_distance_cm(
                    &distance_cm);

            if (status ==
                ULTRASONIC_STATUS_OK)
            {
                consecutive_invalid_readings = 0U;

                last_forward_distance_cm =
                    distance_cm;

                if (distance_cm <=
                    OBSTACLE_DETECTION_THRESHOLD_CM)
                {
                    stop_request_sent = false;
                    vehicle_stopped_received = false;

                    set_state(
                        OBSTACLE_STATE_REQUEST_STOP);
                }
                else
                {
                    set_state(
                        OBSTACLE_STATE_MONITOR);
                }
            }
            else
            {
                consecutive_invalid_readings++;

                if (consecutive_invalid_readings >=
                    OBSTACLE_MAX_INVALID_READINGS)
                {
                    enter_fault(
                        map_sensor_fault(status));
                }
                else
                {
                    set_state(
                        OBSTACLE_STATE_MONITOR);
                }
            }

            break;

        case OBSTACLE_STATE_REQUEST_STOP:

            if (!stop_request_sent)
            {
                stop_request_sent = true;

                if (system_callbacks.request_stop !=
                    NULL)
                {
                    system_callbacks.request_stop();
                }
            }

            if (vehicle_stopped_received)
            {
                vehicle_stopped_received = false;

                set_state(
                    OBSTACLE_STATE_SCANNING);
            }
            else if (state_timeout_reached(
                         OBSTACLE_STOP_TIMEOUT_MS))
            {
                enter_fault(
                    OBSTACLE_FAULT_STOP_TIMEOUT);
            }

            break;

        case OBSTACLE_STATE_SCANNING:

            scan_fault = perform_scan();

            if (scan_fault ==
                OBSTACLE_FAULT_NONE)
            {
                set_state(
                    OBSTACLE_STATE_BUILD_PROFILE);
            }
            else
            {
                enter_fault(scan_fault);
            }

            break;

        case OBSTACLE_STATE_BUILD_PROFILE:

            if (build_profile())
            {
                set_state(
                    OBSTACLE_STATE_PLAN_BYPASS);
            }
            else
            {
                enter_fault(
                    OBSTACLE_FAULT_SCAN_INVALID);
            }

            break;

        case OBSTACLE_STATE_PLAN_BYPASS:

            plan_bypass();

            if (current_profile.bypass_direction ==
                BYPASS_NO_SAFE_ROUTE)
            {
                enter_fault(
                    OBSTACLE_FAULT_NO_SAFE_BYPASS);
            }
            else
            {
                bypass_complete_received = false;

                set_state(
                    OBSTACLE_STATE_BYPASS);

                if (system_callbacks.request_bypass !=
                    NULL)
                {
                    system_callbacks.request_bypass(
                        current_profile.bypass_direction,
                        &current_profile);
                }
            }

            break;

        case OBSTACLE_STATE_BYPASS:

            if (bypass_complete_received)
            {
                bypass_complete_received = false;
                line_found_received = false;

                set_state(
                    OBSTACLE_STATE_LINE_RECOVERY);

                if (system_callbacks.request_line_recovery !=
                    NULL)
                {
                    system_callbacks.request_line_recovery();
                }
            }
            else if (state_timeout_reached(
                         OBSTACLE_BYPASS_TIMEOUT_MS))
            {
                enter_fault(
                    OBSTACLE_FAULT_BYPASS_TIMEOUT);
            }

            break;

        case OBSTACLE_STATE_LINE_RECOVERY:

            if (line_found_received)
            {
                line_found_received = false;

                stop_request_sent = false;

                set_state(
                    OBSTACLE_STATE_MONITOR);
            }
            else if (state_timeout_reached(
                         OBSTACLE_LINE_RECOVERY_TIMEOUT_MS))
            {
                enter_fault(
                    OBSTACLE_FAULT_LINE_RECOVERY_TIMEOUT);
            }

            break;

        case OBSTACLE_STATE_FAULT:
        default:
            break;
    }
}

void obstacle_reset(void)
{
    consecutive_invalid_readings = 0U;

    vehicle_stopped_received = false;
    bypass_complete_received = false;
    line_found_received = false;

    stop_request_sent = false;

    current_fault =
        OBSTACLE_FAULT_NONE;

    current_profile.valid = false;
    current_profile.bypass_direction =
        BYPASS_NONE;

    if (!servo_is_ready())
    {
        enter_fault(
            OBSTACLE_FAULT_SERVO);

        return;
    }

    (void)servo_set_angle(
        OBSTACLE_SCAN_CENTER_DEG);

    set_state(
        OBSTACLE_STATE_MONITOR);
}

void obstacle_notify_vehicle_stopped(void)
{
    vehicle_stopped_received = true;
}

void obstacle_notify_bypass_complete(void)
{
    bypass_complete_received = true;
}

void obstacle_notify_line_found(void)
{
    line_found_received = true;
}

obstacle_state_t obstacle_get_state(void)
{
    return current_state;
}

obstacle_fault_t obstacle_get_fault(void)
{
    return current_fault;
}

const obstacle_profile_t *
obstacle_get_profile(void)
{
    return &current_profile;
}

const obstacle_scan_sample_t *
obstacle_get_scan_samples(
    uint32_t *sample_count)
{
    if (sample_count != NULL)
    {
        *sample_count =
            OBSTACLE_SCAN_SAMPLE_COUNT;
    }

    return scan_samples;
}

uint32_t obstacle_get_forward_distance_cm(void)
{
    return last_forward_distance_cm;
}

const char *obstacle_state_name(
    obstacle_state_t state)
{
    const char *name;

    switch (state)
    {
        case OBSTACLE_STATE_MONITOR:
            name = "MONITOR";
            break;

        case OBSTACLE_STATE_VALIDATE:
            name = "VALIDATE";
            break;

        case OBSTACLE_STATE_REQUEST_STOP:
            name = "REQUEST_STOP";
            break;

        case OBSTACLE_STATE_SCANNING:
            name = "SCANNING";
            break;

        case OBSTACLE_STATE_BUILD_PROFILE:
            name = "BUILD_PROFILE";
            break;

        case OBSTACLE_STATE_PLAN_BYPASS:
            name = "PLAN_BYPASS";
            break;

        case OBSTACLE_STATE_BYPASS:
            name = "BYPASS";
            break;

        case OBSTACLE_STATE_LINE_RECOVERY:
            name = "LINE_RECOVERY";
            break;

        case OBSTACLE_STATE_FAULT:
            name = "FAULT";
            break;

        default:
            name = "UNKNOWN";
            break;
    }

    return name;
}

const char *obstacle_fault_name(
    obstacle_fault_t fault)
{
    const char *name;

    switch (fault)
    {
        case OBSTACLE_FAULT_NONE:
            name = "NONE";
            break;

        case OBSTACLE_FAULT_SENSOR_TIMEOUT:
            name = "SENSOR_TIMEOUT";
            break;

        case OBSTACLE_FAULT_REPEATED_INVALID_READINGS:
            name = "REPEATED_INVALID_READINGS";
            break;

        case OBSTACLE_FAULT_SERVO:
            name = "SERVO_FAILURE";
            break;

        case OBSTACLE_FAULT_SCAN_INVALID:
            name = "SCAN_INVALID";
            break;

        case OBSTACLE_FAULT_NO_SAFE_BYPASS:
            name = "NO_SAFE_BYPASS";
            break;

        case OBSTACLE_FAULT_STOP_TIMEOUT:
            name = "STOP_TIMEOUT";
            break;

        case OBSTACLE_FAULT_BYPASS_TIMEOUT:
            name = "BYPASS_TIMEOUT";
            break;

        case OBSTACLE_FAULT_LINE_RECOVERY_TIMEOUT:
            name = "LINE_RECOVERY_TIMEOUT";
            break;

        default:
            name = "UNKNOWN";
            break;
    }

    return name;
}

const char *obstacle_bypass_name(
    bypass_direction_t direction)
{
    const char *name;

    switch (direction)
    {
        case BYPASS_NONE:
            name = "NONE";
            break;

        case BYPASS_LEFT:
            name = "LEFT";
            break;

        case BYPASS_RIGHT:
            name = "RIGHT";
            break;

        case BYPASS_NO_SAFE_ROUTE:
            name = "NO_SAFE_ROUTE";
            break;

        default:
            name = "UNKNOWN";
            break;
    }

    return name;
}