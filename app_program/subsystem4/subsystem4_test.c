/*
 *	subsystem4_test.c
 *	On-target tests for subsystem 4.  Built only with: make TEST=1
 *	Record S4 results in testing/subsystem4/TEST_LOG.md and the
 *	INT- result in testing/integration/TEST_LOG.md.
 */

#include "subsystem4.h"
#include "../test_report.h"
#include "../../testing/subsystem4/host/buddy4_cases.h"

#if APP_TEST

static void report_imu_case (const char *p_id, bool b_pass,
                             const char *p_text)
{
    tm_printf((UB *)"[TEST] %s %s %s\n", p_id,
              b_pass ? "PASS" : "FAIL", p_text);
}

EXPORT void subsystem4_test(void)
{
	T_RMBF	rmbf;
	UW	before;

	TEST_INIT();

	/* S4-U01: the message buffer accepts the documented line length */
	TEST_CHECK("S4-U01",
		   tk_ref_mbf(mbf_summary, &rmbf) == E_OK &&
		   rmbf.maxmsz == S4_SUMMARY_MAX,
		   "message buffer max line = S4_SUMMARY_MAX");

	/* INT-02 (integration, IF-03): summary lines from subsystem 3 reach
	   the monitor.  One line per 5 samples = one per 1.5 s. */
	before = s4_lines_received;
	tk_dly_tsk(3500);
	TEST_CHECK("INT-02", s4_lines_received >= before + 2,
		   "summary lines flow from subsystem 3 to subsystem 4");

	TEST_SUMMARY("subsystem4");
    /* Pure cases use local contexts and a fake bus, not the live IMU. */
    {
        const uint32_t failures = buddy4_cases_run(report_imu_case);
        tm_printf((UB *)"[TEST] buddy4 SUMMARY %u failures\n", failures);
    }
}

#endif /* APP_TEST */
