#include <stdio.h>

#include "pico/stdlib.h"

#include "obstacle/obstacle.h"
#include "obstacle/obstacle_config.h"

static void test_request_stop(void)
{
    printf(
        "[Buddy 5 -> Mission Controller] "
        "STOP requested\n");

    /*
     * Temporary standalone test response.
     *
     * During team integration, the Mission Controller
     * should call this only after Buddy 2 confirms
     * that the vehicle has stopped.
     */
    obstacle_notify_vehicle_stopped();
}

static void test_request_bypass(
    bypass_direction_t direction,
    const obstacle_profile_t *profile)
{
    printf(
        "[Buddy 5 -> Mission Controller] "
        "Bypass requested: %s\n",
        obstacle_bypass_name(direction));

    if (profile != NULL)
    {
        printf(
            "Closest: %lu cm\n",
            (unsigned long)
                profile->closest_distance_cm);

        printf(
            "Estimated width: %lu cm\n",
            (unsigned long)
                profile->estimated_width_cm);

        printf(
            "Left clearance: %lu cm\n",
            (unsigned long)
                profile->left_clearance_cm);

        printf(
            "Right clearance: %lu cm\n",
            (unsigned long)
                profile->right_clearance_cm);
    }

    /*
     * Temporary standalone test response.
     *
     * During integration, Buddy 2 / Mission Controller
     * will notify Buddy 5 when the bypass is complete.
     */
    obstacle_notify_bypass_complete();
}

static void test_request_line_recovery(void)
{
    printf(
        "[Buddy 5 -> Mission Controller] "
        "Line recovery requested\n");

    /*
     * Temporary standalone test response.
     *
     * During integration, Buddy 3 sends LINE_FOUND.
     */
    obstacle_notify_line_found();
}

static void test_request_fault(
    obstacle_fault_t fault)
{
    printf(
        "[Buddy 5 -> Mission Controller] "
        "FAULT requested: %s\n",
        obstacle_fault_name(fault));
}

int main(void)
{
    obstacle_callbacks_t callbacks;

    stdio_init_all();

    sleep_ms(2000);

    callbacks.request_stop =
        test_request_stop;

    callbacks.request_bypass =
        test_request_bypass;

    callbacks.request_line_recovery =
        test_request_line_recovery;

    callbacks.request_fault =
        test_request_fault;

    printf(
        "Buddy 5 obstacle subsystem starting...\n");

    if (!obstacle_system_init(&callbacks))
    {
        printf(
            "Buddy 5 initialization failed.\n");

        printf(
            "Current fault: %s\n",
            obstacle_fault_name(
                obstacle_get_fault()));

        printf(
            "NOTE: Confirm the servo GPIO in "
            "obstacle_config.h.\n");
    }

    while (true)
    {
        /*
         * These two functions correspond to the two
         * RTOS tasks in the Design Review:
         *
         * 1. Obstacle Monitor Task
         * 2. Obstacle Handling Task
         */
        obstacle_monitor_step();

        obstacle_handling_step();

        sleep_ms(
            OBSTACLE_TEST_LOOP_DELAY_MS);
    }

    return 0;
}