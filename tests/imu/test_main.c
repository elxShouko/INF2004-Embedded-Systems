/**
 * @file    test_main.c
 * @brief   HOST-ONLY test runner for the Buddy 4 IMU / Terrain subsystem.
 *
 * Usage: ./buddy4_tests [-v] [-f substring]
 *        -v prints every emitted event, -f runs only matching tests.
 * Exit code: 0 if all tests passed, 1 otherwise.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "test_util.h"


/** Informative only: host execution time of one sampling period. */
static void perf_report(void)
{
    clock_t  c0;
    clock_t  c1;
    uint32_t n = 0U;

    test_section("PERFORMANCE (host, informative - NOT a Pico W figure)");
    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    scn_add_segment(&g_h.scn, 3.0, 100.0, 0.25, 0.25, 0.0);
    scn_add_hump(&g_h.scn, 1.0, 0.02, 0.10);
    (void)harness_start();
    (void)harness_until_monitoring(5000U);
    c0 = clock();
    while (n < 5000U)
    {
        harness_run_ms(20U);
        n++;
    }
    c1 = clock();
    printf("  %u cycles, %.2f us per cycle incl. simulator (host CPU)\n",
           (unsigned)n, 1e6 * (double)(c1 - c0) / CLOCKS_PER_SEC / n);
}

int main(int argc, char ** argv)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "-v"))
        {
            g_verbose = true;
        }
        else if ((0 == strcmp(argv[i], "-f")) && ((i + 1) < argc))
        {
            g_filter = argv[i + 1];
            i++;
        }
        else
        {
            printf("usage: %s [-v] [-f name-substring]\n", argv[0]);
            return (2);
        }
    }

    suite_unit_driver();
    suite_unit_math_cal();
    suite_unit_processing();
    suite_unit_hump();
    suite_unit_events();
    suite_blackbox();
    suite_robustness();
    suite_rtos();
    if (NULL == g_filter)
    {
        perf_report();
    }

    printf("\n==================================================\n");
    printf("  %u tests, %u assertions, %u FAILED\n", (unsigned)g_tests_run,
           (unsigned)g_asserts, (unsigned)g_tests_failed);
    printf("==================================================\n");

    return ((0U == g_tests_failed) ? 0 : 1);
}
