/* Runs the on-target test entry point on the PC, after creating the objects.
 * The process exit status is the verdict: 0 only if every [TEST] check
 * passed, at least S1_HOST_MIN_CHECKS ran, and the run completed. */
#include "../../../app_program/subsystem1/s1_net.h"
#include "../../../app_program/subsystem1/s1_sim.h"

extern void subsystem1_test(void);
extern int host_test_pass_count(void);
extern int host_test_fail_count(void);
extern int host_has_threads(void);

/* A run that skips tests must not pass: update when tests are added. */
#if S1_WIRE_JSON
#define S1_HOST_MIN_CHECKS_THREADS 45
#define S1_HOST_MIN_CHECKS_SERIAL 42
#else /* Binary build: 17 more checks (S1-U23 to S1-U26). */
#define S1_HOST_MIN_CHECKS_THREADS 62
#define S1_HOST_MIN_CHECKS_SERIAL 59
#endif

int main(void)
{
    int passed = 0;
    int failed = 0;
    int status = 0;
    int min_checks = host_has_threads() ? S1_HOST_MIN_CHECKS_THREADS
                                        : S1_HOST_MIN_CHECKS_SERIAL;

    if ((s1_net_init() < 0) || (s1_sim_init() < 0))
    {
        tm_printf((UB *)"HOST-RUN: kernel object creation failed\n");
        status = 2;
    }
    else
    {
        /* TWF_ANDW is zero, not an independently settable mode bit. */
        T_CFLG flag = {0};
        UINT pattern = 0u;
        ID id = tk_cre_flg(&flag);

        if ((id <= 0) || (E_OK != tk_set_flg(id, 1u)) ||
            (E_TMOUT != tk_wai_flg(id, 3u, TWF_ANDW, &pattern, TMO_POL)) ||
            (E_OK != tk_wai_flg(id, 3u, TWF_ORW, &pattern, TMO_POL)) ||
            (E_OK != tk_set_flg(id, 2u)) ||
            (E_OK != tk_wai_flg(id, 3u, TWF_ANDW, &pattern, TMO_POL)))
        {
            tm_printf((UB *)"HOST-RUN: event flag mode regression failed\n");
            return 2;
        }
        subsystem1_test();
        passed = host_test_pass_count();
        failed = host_test_fail_count();
        tm_printf((UB *)"HOST-RUN: %d passed, %d failed, threads=%d\n", passed,
                  failed, host_has_threads());
        if ((failed > 0) || (passed < min_checks))
        {
            tm_printf((UB *)"HOST-RUN: FAILED (need 0 failed and >= %d passed)\n",
                      min_checks);
            status = 1;
        }
    }
    return status;
}