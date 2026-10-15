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

#include "test_util.h"


int main(int argc, char ** argv)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "-v"))
        {
            g_b_verbose = true;
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
    if (0U == g_tests_run)
    {
        (void)fprintf(stderr, "No tests matched the filter.\n");
        return (2);
    }

    printf("\n==================================================\n");
    printf("  %u tests, %u assertions, %u FAILED\n", (unsigned)g_tests_run,
           (unsigned)g_asserts, (unsigned)g_tests_failed);
    printf("==================================================\n");

    return ((0U == g_tests_failed) ? 0 : 1);
}
