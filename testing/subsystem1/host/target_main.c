/* Runs the on-target test entry point on the PC, after creating the objects.
 * The process exit status is the verdict: 0 only if every [TEST] check
 * passed, at least S1_HOST_MIN_CHECKS ran, and the run completed. */
#include "../../../app_program/subsystem1/s1_net.h"
#include "../../../app_program/subsystem1/s1_sim.h"

extern void subsystem1_test (void);
extern int  host_test_pass_count (void);
extern int  host_test_fail_count (void);
extern int  host_has_threads (void);

/* A run that skips tests must not pass: update when tests are added. */
#define S1_HOST_MIN_CHECKS 24

int
main (void)
{
    int passed = 0;
    int failed = 0;
    int status = 0;

    if ((s1_net_init() < 0) || (s1_sim_init() < 0))
    {
        tm_printf((UB *) "HOST-RUN: kernel object creation failed\n");
        status = 2;
    }
    else
    {
        subsystem1_test();
        passed = host_test_pass_count();
        failed = host_test_fail_count();
        tm_printf((UB *) "HOST-RUN: %d passed, %d failed, threads=%d\n", passed,
                 failed, host_has_threads());
        if ((failed > 0) || (passed < S1_HOST_MIN_CHECKS))
        {
            tm_printf((UB *) "HOST-RUN: FAILED (need 0 failed and >= %d passed)\n",
                     S1_HOST_MIN_CHECKS);
            status = 1;
        }
    }
    return status;
}
