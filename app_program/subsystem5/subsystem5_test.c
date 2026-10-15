/*
 *	subsystem5_test.c
 *	On-target tests for subsystem 5.  Built only with: make TEST=1
 *	Record results in testing/subsystem5/TEST_LOG.md.  IDs: S5-U01, ...
 */

#include "subsystem5.h"
#include "../test_report.h"

#if APP_TEST

EXPORT void subsystem5_test(void)
{
	TEST_INIT();

	TEST_CHECK("S5-U01", tk_get_tid() > 0, "test hook runs in task context");

	TEST_SUMMARY("subsystem5");
}

#endif /* APP_TEST */
