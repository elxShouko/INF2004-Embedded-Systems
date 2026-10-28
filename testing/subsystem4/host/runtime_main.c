/** @file runtime_main.c
 * Each case runs in a fresh process (static lifecycle).
 */
#include "runtime_fake.h"
#include "../../../app_program/subsystem4/imu_link.h"
#include "../../../app_program/subsystem4/drv_lsm.h"

#include <stdio.h>
#include <stdlib.h>

static bool run_case (uint32_t number);
static bool run_bridge_case (uint32_t number);

int main (int argc, char *argv[])
{
    uint32_t number = 0u;
    bool b_pass = false;

    if (2 != argc)
    {
        return 2;
    }
    /* Runner supplies registered numeric IDs; no untrusted harness input. */
    number = (uint32_t)strtoul(argv[1], NULL, 10);
    b_pass = run_case(number) && (!g_fake.b_violation) &&
             (0u == g_fake.lock_depth);
    (void)printf("[TEST] S4-U%u %s runtime/bridge check\n", number,
                 b_pass ? "PASS" : "FAIL");
    return b_pass ? 0 : 1;
}

static bool run_case (uint32_t number)
{
    imu_runtime_status_t status = {0};
    imu_core_status_t event = {0};

    if ((number >= 190u) && (number <= 198u))
    {
        return run_bridge_case(number);
    }

    if ((number >= 149u) && (number <= 152u))
    {
        g_fake.fail_create = (int32_t)(number - 148u);
        return (E_SYS == imu_runtime_init()) &&
               (E_OBJ == imu_runtime_start()) && (!g_fake.b_started);
    }
    if (E_OK != imu_runtime_init())
    {
        return false;
    }
#if defined(APP_ROBOT) && APP_ROBOT
    if ((210u == number) || (211u == number))
    {
        imu_core_sample_t raw = {0};
        uint32_t result = 123u;
        if (210u == number)
        {
            return ((E_PAR == imu_runtime_raw(NULL, &result)) &&
                (E_PAR == imu_runtime_raw(&raw, NULL)) &&
                (E_OK == imu_runtime_raw(&raw, &result)) &&
                (UINT32_MAX == result) && (0u == g_fake.samples));
        }
        raw.time_ms = 777u;
        g_fake.b_lock_failure = true;
        return ((E_TMOUT == imu_runtime_raw(&raw, &result)) &&
            (777u == raw.time_ms) && (123u == result) &&
            (0u == g_fake.samples));
    }
#endif
    if (153u == number)
    {
        return (!g_fake.b_started) && (0u == g_fake.samples) &&
               (E_OK == imu_runtime_start());
    }
    if (154u == number)
    {
        g_fake.b_start_failure = true;
        return E_SYS == imu_runtime_start();
    }
    g_fake.b_open_failure = (155u == number);
    g_fake.b_full_queue = (156u == number);
    g_fake.b_stale = (157u == number);
    g_fake.b_delay_failure = (158u == number);
    g_fake.b_overrun = (159u == number);
    if (160u == number)
    {
        g_fake.b_lock_failure = true;
        return (E_TMOUT == imu_runtime_get_status(&status)) &&
               (E_TMOUT == imu_runtime_set_motion_trust(true));
    }
    if (161u == number)
    {
        return (E_PAR == imu_runtime_get_status(NULL)) &&
               (E_PAR == imu_runtime_get_event(NULL)) &&
               (E_PAR == imu_runtime_set_speed(-1.0f, 0u, true)) &&
               (E_PAR == imu_runtime_set_speed(100.0f, 0u, true)) &&
               (E_OK == imu_runtime_set_motion_trust(true)) &&
               (E_OK == imu_runtime_set_speed(0.0f, 0u, false));
    }
    if (199u == number)
    {
        return (E_OK == imu_runtime_set_motion_trust(true)) &&
               (E_OK == imu_runtime_set_motion_trust(false)) &&
               (E_PAR == imu_runtime_set_speed(500.0f, 0u, true)) &&
               (E_OK == imu_runtime_set_speed(0.0f, 0u, false));
    }
    if (E_OK != imu_runtime_start())
    {
        return false;
    }
    runtime_fake_run();
#if defined(APP_ROBOT) && APP_ROBOT
    if (212u == number)
    {
        imu_core_sample_t raw = {0};
        uint32_t result = 123u;
        uint32_t reads = g_fake.samples;
        return ((reads > 0u) && (E_OK == imu_runtime_raw(&raw, &result)) &&
            (DRV_LSM_IO_ERROR == result) && (raw.accel_g[2] == 1.0f) &&
            (raw.time_ms < g_fake.time_ms) && (reads == g_fake.samples));
    }
#endif
    if ((E_OK != imu_runtime_wait_fault(TMO_POL)) ||
        (E_OK != imu_runtime_wait_fault(TMO_POL)) ||
        (E_OK != imu_runtime_get_status(&status)))
    {
        return false;
    }
    if (156u == number)
    {
        return (status.dropped_events > 0u) &&
               (IMU_FAULT == status.sensing.phase) &&
               (E_TMOUT == imu_runtime_get_event(&event));
    }
    return (IMU_FAULT == status.sensing.phase) && (1u == g_fake.exits) &&
           (E_OK == imu_runtime_get_event(&event)) &&
           (0u != (event.events & IMU_EVENT_FAULT));
}

static bool run_bridge_case (uint32_t number)
{
    imu_core_sample_t sample = {0};

    g_fake.time_ms = 100u;
    g_fake.motion.state = MOTION_RUN_ACTIVE;
    g_fake.motion.fault_result = MOTION_RUN_NONE;
    g_fake.motion.active_kind = MOVE_PLAN_SPEED;
    g_fake.motion.timestamp_us = 100000u;
    g_fake.motion.measurement_timestamp_us = 100000u;
    g_fake.motion.measurement_max_age_us = 40000u;
    g_fake.motion.b_measurement_valid = true;
    g_fake.motion.speed_mm_s[0] = 500;
    g_fake.motion.speed_mm_s[1] = 500;
    g_fake.motion.target_mm_s[0] = 500;
    g_fake.motion.target_mm_s[1] = 500;
    sample.time_ms = 100u;
    if (191u == number)
    {
        g_fake.motion.speed_mm_s[0] = -500;
        g_fake.motion.speed_mm_s[1] = -500;
        g_fake.motion.target_mm_s[0] = -500;
        g_fake.motion.target_mm_s[1] = -500;
    }
    if (192u == number)
    {
        g_fake.motion.speed_mm_s[1] = -500;
        g_fake.motion.active_kind = MOVE_PLAN_LEFT;
    }
    if (193u == number)
    {
        g_fake.motion.measurement_timestamp_us = 50000u;
    }
    if (194u == number)
    {
        g_fake.b_motion_busy = true;
    }
    if (195u == number)
    {
        g_fake.b_motion_clock_fail = true;
    }
    if (196u == number)
    {
        g_fake.motion.measurement_timestamp_us = 110000u;
    }
    if (197u == number)
    {
        g_fake.motion.target_mm_s[1] = 200;
    }
    if (198u == number)
    {
        g_fake.motion.speed_mm_s[1] = 400;
    }
    imu_link_read(&sample, 80u, 5.0f);
    switch (number)
    {
        case 190u:
            return sample.b_motion_valid && sample.b_distance_valid &&
                   sample.b_straight && (10.0f == sample.distance_mm) &&
                   (100u == sample.motion_time_ms);
        case 191u:
            return sample.b_motion_valid && (!sample.b_distance_valid) &&
                   (!sample.b_straight) &&
                   (-500.0f == sample.speed_mm_s);
        case 192u:
            return sample.b_motion_valid && (!sample.b_straight) &&
                   (0.0f == sample.distance_mm);
        case 193u:
        case 194u:
        case 195u:
        case 196u:
            return (!sample.b_motion_valid) &&
                   (!sample.b_distance_valid);
        case 197u:
        case 198u:
            return sample.b_motion_valid && (!sample.b_straight);
        default:
            return false;
    }
}

/* End of file. */

