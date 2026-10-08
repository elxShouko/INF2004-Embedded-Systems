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

/*
 * Current Buddy 5 state and fault.
 */
static obstacle_state_t current_state =
    OBSTACLE_STATE_MONITOR;

static obstacle_fault_t current_fault =
    OBSTACLE_FAULT_NONE;

/*
 * Interface to Mission Controller.
 */
static obstacle_callbacks_t system_callbacks;

/*
 * Latest obstacle information.
 */
static obstacle_profile_t current_profile;

static obstacle_action_plan_t current_action_plan;

/*
 * Coarse scan:
 * Used to locate the general obstacle region.
 */
static obstacle_scan_sample_t
    coarse_samples[OBSTACLE_COARSE_SAMPLE_COUNT];

/*
 * Fine scan:
 * Used to build a more accurate obstacle profile.
 */
static obstacle_scan_sample_t
    scan_samples[OBSTACLE_FINE_SAMPLE_COUNT];

static uint32_t current_scan_sample_count = 0U;

/*
 * Monitoring information.
 */
static uint32_t last_forward_distance_cm = 0U;

static uint32_t consecutive_invalid_readings = 0U;

/*
 * Coordination flags.
 */
static bool stop_request_sent = false;

static bool vehicle_stopped_received = false;
static bool bypass_complete_received = false;
static bool line_found_received = false;

static uint64_t state_entry_time_ms = 0ULL;


/*
 * Get current system time in milliseconds.
 */
static uint64_t get_time_ms(void)
{
    return time_us_64() / 1000ULL;
}


/*
 * Change Buddy 5 state.
 */
static void set_state(
    obstacle_state_t new_state)
{
    current_state = new_state;

    state_entry_time_ms =
        get_time_ms();
}


/*
 * Check whether the current state has timed out.
 */
static bool state_timeout_reached(
    uint32_t timeout_ms)
{
    return
        (get_time_ms() - state_entry_time_ms) >=
        (uint64_t)timeout_ms;
}


/*
 * Reset the generated profile and action plan.
 */
static void clear_profile_and_plan(void)
{
    memset(
        &current_profile,
        0,
        sizeof(current_profile));

    memset(
        &current_action_plan,
        0,
        sizeof(current_action_plan));

    current_profile.bypass_direction =
        BYPASS_NONE;

    current_action_plan.bypass_direction =
        BYPASS_NONE;
}


/*
 * Convert ultrasonic errors to Buddy 5 faults.
 */
static obstacle_fault_t map_sensor_fault(
    ultrasonic_status_t status)
{
    if ((status ==
         ULTRASONIC_STATUS_TIMEOUT_RISE) ||
        (status ==
         ULTRASONIC_STATUS_TIMEOUT_FALL))
    {
        return
            OBSTACLE_FAULT_SENSOR_TIMEOUT;
    }

    return
        OBSTACLE_FAULT_REPEATED_INVALID_READINGS;
}


/*
 * Enter FAULT and report it to
 * the Mission Controller.
 */
static void enter_fault(
    obstacle_fault_t fault)
{
    current_fault = fault;

    set_state(
        OBSTACLE_STATE_FAULT);

    if (system_callbacks.request_fault != NULL)
    {
        system_callbacks.request_fault(
            fault);
    }
}


/*
 * Take one validated ultrasonic measurement
 * at a requested servo angle.
 */
static bool scan_at_angle(
    uint8_t angle_deg,
    obstacle_scan_sample_t *sample)
{
    uint32_t distance_cm;
    ultrasonic_status_t status;

    if (sample == NULL)
    {
        return false;
    }

    sample->angle_deg = angle_deg;
    sample->distance_cm = 0U;
    sample->valid = false;

    if (!servo_set_angle(angle_deg))
    {
        return false;
    }

    sleep_ms(
        OBSTACLE_SERVO_SETTLE_MS);

    status =
        ultrasonic_get_validated_distance_cm(
            &distance_cm);

    /*
     * Invalid ultrasonic readings do not
     * immediately abort the full scan.
     *
     * The number of valid scan samples is
     * checked after scanning finishes.
     */
    if (status != ULTRASONIC_STATUS_OK)
    {
        return true;
    }

    sample->distance_cm =
        distance_cm;

    sample->valid =
        true;

    return true;
}


/*
 * Stage 1:
 *
 * Perform a wide coarse scan to identify
 * approximately where the obstacle is.
 */
static obstacle_fault_t perform_coarse_scan(
    uint8_t *closest_angle_deg)
{
    uint32_t valid_samples;
    uint32_t closest_distance;
    uint8_t angle_deg;

    if (closest_angle_deg == NULL)
    {
        return
            OBSTACLE_FAULT_SCAN_INVALID;
    }

    valid_samples = 0U;
    closest_distance = UINT32_MAX;

    memset(
        coarse_samples,
        0,
        sizeof(coarse_samples));

    for (uint32_t index = 0U;
         index < OBSTACLE_COARSE_SAMPLE_COUNT;
         index++)
    {
        angle_deg =
            (uint8_t)(
                OBSTACLE_COARSE_START_DEG +
                (index *
                 OBSTACLE_COARSE_STEP_DEG));

        if (!scan_at_angle(
                angle_deg,
                &coarse_samples[index]))
        {
            return
                OBSTACLE_FAULT_SERVO;
        }

        if (!coarse_samples[index].valid)
        {
            continue;
        }

        valid_samples++;

        if (coarse_samples[index].distance_cm <
            closest_distance)
        {
            closest_distance =
                coarse_samples[index].distance_cm;

            *closest_angle_deg =
                coarse_samples[index].angle_deg;
        }
    }

    if ((valid_samples <
         OBSTACLE_MIN_VALID_SCAN_SAMPLES) ||
        (closest_distance == UINT32_MAX))
    {
        return
            OBSTACLE_FAULT_SCAN_INVALID;
    }

    return
        OBSTACLE_FAULT_NONE;
}


/*
 * Stage 2:
 *
 * Perform a finer scan around the obstacle
 * direction discovered during the coarse scan.
 */
static obstacle_fault_t perform_fine_scan(
    uint8_t obstacle_angle_deg)
{
    int32_t start_angle;
    int32_t end_angle;
    uint32_t valid_samples;

    start_angle =
        (int32_t)obstacle_angle_deg -
        (int32_t)OBSTACLE_FINE_RANGE_DEG;

    end_angle =
        (int32_t)obstacle_angle_deg +
        (int32_t)OBSTACLE_FINE_RANGE_DEG;

    /*
     * Keep scan angles inside the configured
     * safe servo range.
     */
    if (start_angle <
        (int32_t)OBSTACLE_COARSE_START_DEG)
    {
        start_angle =
            (int32_t)OBSTACLE_COARSE_START_DEG;
    }

    if (end_angle >
        (int32_t)OBSTACLE_COARSE_END_DEG)
    {
        end_angle =
            (int32_t)OBSTACLE_COARSE_END_DEG;
    }

    memset(
        scan_samples,
        0,
        sizeof(scan_samples));

    current_scan_sample_count = 0U;

    valid_samples = 0U;

    for (int32_t angle = start_angle;
         angle <= end_angle;
         angle +=
             (int32_t)OBSTACLE_FINE_STEP_DEG)
    {
        if (current_scan_sample_count >=
            OBSTACLE_FINE_SAMPLE_COUNT)
        {
            break;
        }

        if (!scan_at_angle(
                (uint8_t)angle,
                &scan_samples[
                    current_scan_sample_count]))
        {
            return
                OBSTACLE_FAULT_SERVO;
        }

        if (scan_samples[
                current_scan_sample_count].valid)
        {
            valid_samples++;
        }

        current_scan_sample_count++;
    }

    /*
     * Return the ultrasonic sensor to face
     * forwards after scanning.
     */
    if (!servo_set_angle(
            OBSTACLE_SCAN_CENTER_DEG))
    {
        return
            OBSTACLE_FAULT_SERVO;
    }

    if (valid_samples <
        OBSTACLE_MIN_VALID_SCAN_SAMPLES)
    {
        return
            OBSTACLE_FAULT_SCAN_INVALID;
    }

    return
        OBSTACLE_FAULT_NONE;
}


/*
 * Complete adaptive scan:
 *
 * COARSE SCAN
 *      ↓
 * Find approximate obstacle direction
 *      ↓
 * FINE SCAN around obstacle
 */
static obstacle_fault_t perform_adaptive_scan(void)
{
    uint8_t obstacle_angle_deg;
    obstacle_fault_t fault;

    obstacle_angle_deg =
        OBSTACLE_SCAN_CENTER_DEG;

    fault =
        perform_coarse_scan(
            &obstacle_angle_deg);

    if (fault != OBSTACLE_FAULT_NONE)
    {
        return fault;
    }

    fault =
        perform_fine_scan(
            obstacle_angle_deg);

    return fault;
}


/*
 * Build the obstacle profile.
 *
 * Calculates:
 *
 * - closest distance
 * - closest angle
 * - approximate lateral position
 * - approximate forward distance
 * - estimated obstacle width
 * - left clearance
 * - right clearance
 */
static bool build_profile(void)
{
    uint32_t closest_distance;

    uint32_t left_minimum;
    uint32_t right_minimum;

    uint32_t left_count;
    uint32_t right_count;

    bool obstacle_edge_found;

    uint8_t first_obstacle_angle;
    uint8_t last_obstacle_angle;

    float relative_angle_rad;
    float half_span_rad;
    float estimated_width;

    closest_distance = UINT32_MAX;

    left_minimum = UINT32_MAX;
    right_minimum = UINT32_MAX;

    left_count = 0U;
    right_count = 0U;

    /*
     * Find closest point from the detailed
     * fine scan.
     */
    for (uint32_t index = 0U;
         index < current_scan_sample_count;
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
    }

    if (closest_distance == UINT32_MAX)
    {
        return false;
    }

    current_profile.closest_distance_cm =
        closest_distance;

    /*
     * Use the wider coarse scan to estimate
     * clearance on both sides.
     */
    for (uint32_t index = 0U;
         index < OBSTACLE_COARSE_SAMPLE_COUNT;
         index++)
    {
        if (!coarse_samples[index].valid)
        {
            continue;
        }

        if (coarse_samples[index].angle_deg <
            OBSTACLE_SCAN_CENTER_DEG)
        {
            left_count++;

            if (coarse_samples[index].distance_cm <
                left_minimum)
            {
                left_minimum =
                    coarse_samples[index].distance_cm;
            }
        }
        else if (coarse_samples[index].angle_deg >
                 OBSTACLE_SCAN_CENTER_DEG)
        {
            right_count++;

            if (coarse_samples[index].distance_cm <
                right_minimum)
            {
                right_minimum =
                    coarse_samples[index].distance_cm;
            }
        }
    }

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
     * Estimate obstacle location relative
     * to the centre of the robot.
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
     * Estimate obstacle width.
     *
     * Fine-scan measurements close to the
     * nearest surface are treated as belonging
     * to the same obstacle.
     */
    obstacle_edge_found = false;

    first_obstacle_angle = 0U;
    last_obstacle_angle = 0U;

    for (uint32_t index = 0U;
         index < current_scan_sample_count;
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
            if (!obstacle_edge_found)
            {
                first_obstacle_angle =
                    scan_samples[index].angle_deg;

                obstacle_edge_found = true;
            }

            last_obstacle_angle =
                scan_samples[index].angle_deg;
        }
    }

    current_profile.estimated_width_cm =
        0U;

    if (obstacle_edge_found &&
        (last_obstacle_angle >
         first_obstacle_angle))
    {
        uint32_t angular_span_deg;

        angular_span_deg =
            (uint32_t)(
                last_obstacle_angle -
                first_obstacle_angle);

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

    current_profile.valid =
        true;

    return true;
}


/*
 * Determine whether left or right provides
 * enough clearance for a bypass.
 */
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


/*
 * Add one action to the high-level
 * avoidance plan.
 */
static bool add_action_to_plan(
    obstacle_action_t action)
{
    if (current_action_plan.action_count >=
        OBSTACLE_MAX_ACTIONS)
    {
        return false;
    }

    current_action_plan.actions[
        current_action_plan.action_count] =
        action;

    current_action_plan.action_count++;

    return true;
}


/*
 * Build the high-level avoidance and
 * recovery action plan.
 *
 * Example normal obstacle:
 *
 * STOP
 * TURN_LEFT
 * BYPASS
 * SEARCH_LINE
 * RESUME_LINE
 *
 * Example very close obstacle:
 *
 * STOP
 * REVERSE
 * TURN_RIGHT
 * BYPASS
 * SEARCH_LINE
 * RESUME_LINE
 *
 * Buddy 5 only creates the plan.
 * Buddy 2 / Mission Controller executes
 * the actual motor movement.
 */

 static bool build_action_plan(void)
{
    memset(
        &current_action_plan,
        0,
        sizeof(current_action_plan));

    current_action_plan.bypass_direction =
        current_profile.bypass_direction;

    if (current_profile.bypass_direction ==
        BYPASS_NO_SAFE_ROUTE)
    {
        return false;
    }

    /*
     * Always stop before avoidance.
     */
    if (!add_action_to_plan(
            OBSTACLE_ACTION_STOP))
    {
        return false;
    }

    /*
     * Reverse first if the obstacle is
     * extremely close.
     */
    if (current_profile.closest_distance_cm <=
        OBSTACLE_REVERSE_THRESHOLD_CM)
    {
        current_action_plan.reverse_required =
            true;

        if (!add_action_to_plan(
                OBSTACLE_ACTION_REVERSE))
        {
            return false;
        }
    }

    /*
     * Turn toward the selected bypass side.
     */
    if (current_profile.bypass_direction ==
        BYPASS_LEFT)
    {
        if (!add_action_to_plan(
                OBSTACLE_ACTION_TURN_LEFT))
        {
            return false;
        }
    }
    else if (current_profile.bypass_direction ==
             BYPASS_RIGHT)
    {
        if (!add_action_to_plan(
                OBSTACLE_ACTION_TURN_RIGHT))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    /*
     * Travel around the obstacle.
     */
    if (!add_action_to_plan(
            OBSTACLE_ACTION_BYPASS))
    {
        return false;
    }

    /*
     * After passing the obstacle, steer back
     * toward the original line.
     *
     * Bypass left  -> recover toward right.
     * Bypass right -> recover toward left.
     *
     * Exact turn angle/distance will later be
     * handled by Buddy 2 and calibrated on the
     * real robot.
     */
    if (current_profile.bypass_direction ==
        BYPASS_LEFT)
    {
        if (!add_action_to_plan(
                OBSTACLE_ACTION_TURN_RIGHT))
        {
            return false;
        }
    }
    else
    {
        if (!add_action_to_plan(
                OBSTACLE_ACTION_TURN_LEFT))
        {
            return false;
        }
    }

    /*
     * Search for the original line using
     * Buddy 3's line sensors.
     */
    if (!add_action_to_plan(
            OBSTACLE_ACTION_SEARCH_LINE))
    {
        return false;
    }

    /*
     * Resume normal line following after
     * Buddy 3 reports LINE_FOUND.
     */
    if (!add_action_to_plan(
            OBSTACLE_ACTION_RESUME_LINE))
    {
        return false;
    }

    return true;
}

/*
 * Initialise Buddy 5.
 */
bool obstacle_system_init(
    const obstacle_callbacks_t *callbacks)
{
    memset(
        &system_callbacks,
        0,
        sizeof(system_callbacks));

    memset(
        coarse_samples,
        0,
        sizeof(coarse_samples));

    memset(
        scan_samples,
        0,
        sizeof(scan_samples));

    clear_profile_and_plan();

    if (callbacks != NULL)
    {
        system_callbacks =
            *callbacks;
    }

    ultrasonic_init();

    current_fault =
        OBSTACLE_FAULT_NONE;

    consecutive_invalid_readings =
        0U;

    vehicle_stopped_received =
        false;

    bypass_complete_received =
        false;

    line_found_received =
        false;

    stop_request_sent =
        false;

    last_forward_distance_cm =
        0U;

    current_scan_sample_count =
        0U;

    set_state(
        OBSTACLE_STATE_MONITOR);

    /*
     * This currently fails intentionally
     * while OBSTACLE_SERVO_GPIO is 255.
     *
     * Replace it with the actual servo GPIO
     * once the physical connection is known.
     */
    if (!servo_init(
            OBSTACLE_SERVO_GPIO))
    {
        enter_fault(
            OBSTACLE_FAULT_SERVO);

        return false;
    }

    return true;
}


/*
 * High-priority periodic monitoring logic.
 */
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
        consecutive_invalid_readings =
            0U;

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


/*
 * Medium-high priority obstacle handling logic.
 */
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
                consecutive_invalid_readings =
                    0U;

                last_forward_distance_cm =
                    distance_cm;

                if (distance_cm <=
                    OBSTACLE_DETECTION_THRESHOLD_CM)
                {
                    /*
                     * New obstacle:
                     * remove information from any
                     * previous obstacle.
                     */
                    clear_profile_and_plan();

                    stop_request_sent =
                        false;

                    vehicle_stopped_received =
                        false;

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
                stop_request_sent =
                    true;

                if (system_callbacks.request_stop !=
                    NULL)
                {
                    system_callbacks.request_stop();
                }
            }

            /*
             * Only scan after the vehicle has
             * actually stopped.
             */
            if (vehicle_stopped_received)
            {
                vehicle_stopped_received =
                    false;

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

            scan_fault =
                perform_adaptive_scan();

            if (scan_fault ==
                OBSTACLE_FAULT_NONE)
            {
                set_state(
                    OBSTACLE_STATE_BUILD_PROFILE);
            }
            else
            {
                enter_fault(
                    scan_fault);
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

            /*
             * Step 1:
             * Decide which side is safe.
             */
            plan_bypass();

            if (current_profile.bypass_direction ==
                BYPASS_NO_SAFE_ROUTE)
            {
                enter_fault(
                    OBSTACLE_FAULT_NO_SAFE_BYPASS);

                break;
            }

            /*
             * Step 2:
             * Build STOP / REVERSE / TURN /
             * BYPASS / RECOVERY plan.
             */
            if (!build_action_plan())
            {
                enter_fault(
                    OBSTACLE_FAULT_NO_SAFE_BYPASS);

                break;
            }

            bypass_complete_received =
                false;

            set_state(
                OBSTACLE_STATE_BYPASS);

            /*
             * Tell Mission Controller which
             * direction should be used.
             */
            if (system_callbacks.request_bypass !=
                NULL)
            {
                system_callbacks.request_bypass(
                    current_profile.bypass_direction,
                    &current_profile);
            }

            break;


        case OBSTACLE_STATE_BYPASS:

            /*
             * Wait for Buddy 2 / Mission Controller
             * to confirm that the avoidance movement
             * is complete.
             */
            if (bypass_complete_received)
            {
                bypass_complete_received =
                    false;

                line_found_received =
                    false;

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

            /*
             * Buddy 3 detects the actual line.
             */
            if (line_found_received)
            {
                line_found_received =
                    false;

                stop_request_sent =
                    false;

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


/*
 * Reset Buddy 5 after a fault
 * or mission restart.
 */
void obstacle_reset(void)
{
    consecutive_invalid_readings =
        0U;

    vehicle_stopped_received =
        false;

    bypass_complete_received =
        false;

    line_found_received =
        false;

    stop_request_sent =
        false;

    current_fault =
        OBSTACLE_FAULT_NONE;

    current_scan_sample_count =
        0U;

    clear_profile_and_plan();

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


/*
 * Notifications from Mission Controller
 * and other subsystems.
 */
void obstacle_notify_vehicle_stopped(void)
{
    vehicle_stopped_received =
        true;
}


void obstacle_notify_bypass_complete(void)
{
    bypass_complete_received =
        true;
}


void obstacle_notify_line_found(void)
{
    line_found_received =
        true;
}


/*
 * Status / telemetry getters.
 */
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


const obstacle_action_plan_t *
obstacle_get_action_plan(void)
{
    return &current_action_plan;
}


const obstacle_scan_sample_t *
obstacle_get_scan_samples(
    uint32_t *sample_count)
{
    if (sample_count != NULL)
    {
        *sample_count =
            current_scan_sample_count;
    }

    return scan_samples;
}


uint32_t obstacle_get_forward_distance_cm(void)
{
    return last_forward_distance_cm;
}


/*
 * Human-readable state names.
 */
const char *obstacle_state_name(
    obstacle_state_t state)
{
    switch (state)
    {
        case OBSTACLE_STATE_MONITOR:
            return "MONITOR";

        case OBSTACLE_STATE_VALIDATE:
            return "VALIDATE";

        case OBSTACLE_STATE_REQUEST_STOP:
            return "REQUEST_STOP";

        case OBSTACLE_STATE_SCANNING:
            return "SCANNING";

        case OBSTACLE_STATE_BUILD_PROFILE:
            return "BUILD_PROFILE";

        case OBSTACLE_STATE_PLAN_BYPASS:
            return "PLAN_BYPASS";

        case OBSTACLE_STATE_BYPASS:
            return "BYPASS";

        case OBSTACLE_STATE_LINE_RECOVERY:
            return "LINE_RECOVERY";

        case OBSTACLE_STATE_FAULT:
            return "FAULT";

        default:
            return "UNKNOWN";
    }
}


/*
 * Human-readable fault names.
 */
const char *obstacle_fault_name(
    obstacle_fault_t fault)
{
    switch (fault)
    {
        case OBSTACLE_FAULT_NONE:
            return "NONE";

        case OBSTACLE_FAULT_SENSOR_TIMEOUT:
            return "SENSOR_TIMEOUT";

        case OBSTACLE_FAULT_REPEATED_INVALID_READINGS:
            return "REPEATED_INVALID_READINGS";

        case OBSTACLE_FAULT_SERVO:
            return "SERVO_FAILURE";

        case OBSTACLE_FAULT_SCAN_INVALID:
            return "SCAN_INVALID";

        case OBSTACLE_FAULT_NO_SAFE_BYPASS:
            return "NO_SAFE_BYPASS";

        case OBSTACLE_FAULT_STOP_TIMEOUT:
            return "STOP_TIMEOUT";

        case OBSTACLE_FAULT_BYPASS_TIMEOUT:
            return "BYPASS_TIMEOUT";

        case OBSTACLE_FAULT_LINE_RECOVERY_TIMEOUT:
            return "LINE_RECOVERY_TIMEOUT";

        default:
            return "UNKNOWN";
    }
}


/*
 * Human-readable bypass names.
 */
const char *obstacle_bypass_name(
    bypass_direction_t direction)
{
    switch (direction)
    {
        case BYPASS_NONE:
            return "NONE";

        case BYPASS_LEFT:
            return "LEFT";

        case BYPASS_RIGHT:
            return "RIGHT";

        case BYPASS_NO_SAFE_ROUTE:
            return "NO_SAFE_ROUTE";

        default:
            return "UNKNOWN";
    }
}


/*
 * Human-readable action names.
 */
const char *obstacle_action_name(
    obstacle_action_t action)
{
    switch (action)
    {
        case OBSTACLE_ACTION_NONE:
            return "NONE";

        case OBSTACLE_ACTION_STOP:
            return "STOP";

        case OBSTACLE_ACTION_REVERSE:
            return "REVERSE";

        case OBSTACLE_ACTION_TURN_LEFT:
            return "TURN_LEFT";

        case OBSTACLE_ACTION_TURN_RIGHT:
            return "TURN_RIGHT";

        case OBSTACLE_ACTION_BYPASS:
            return "BYPASS";

        case OBSTACLE_ACTION_SEARCH_LINE:
            return "SEARCH_LINE";

        case OBSTACLE_ACTION_RESUME_LINE:
            return "RESUME_LINE";

        default:
            return "UNKNOWN";
    }
}