/** @file runtime_main.c
 * Each case runs in a fresh process (static lifecycle).
 */
#include "runtime_fake.h"

#include <stdio.h>
#include <stdlib.h>

static bool run_case (uint32_t number);

int main (int argc, char *argv[])
{
    uint32_t number = 0u;
    bool b_pass = false;

    if (2 != argc)
    {
        return 2;
    }
    /* Test runner supplies 149..161 only; no untrusted data in this harness. */
    number = (uint32_t)strtoul(argv[1], NULL, 10);
    b_pass = run_case(number) && (!g_fake.b_violation) &&
             (0u == g_fake.lock_depth);
    (void)printf("[TEST] S4-U%u %s runtime fault injection\n", number,
                 b_pass ? "PASS" : "FAIL");
    return b_pass ? 0 : 1;
}

static bool run_case (uint32_t number)
{
    imu_runtime_status_t status = {0};
    imu_core_status_t event = {0};

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
               (E_TMOUT == imu_runtime_set_speed(1.0f, 0u, true));
    }
    if (161u == number)
    {
        return (E_PAR == imu_runtime_get_status(NULL)) &&
               (E_PAR == imu_runtime_get_event(NULL)) &&
               (E_PAR == imu_runtime_set_speed(-1.0f, 0u, true)) &&
               (E_OK == imu_runtime_set_speed(100.0f, 0u, true)) &&
               (E_OK == imu_runtime_set_speed(0.0f, 0u, false));
    }
    if (E_OK != imu_runtime_start())
    {
        return false;
    }
    runtime_fake_run();
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

/* End of file. */

