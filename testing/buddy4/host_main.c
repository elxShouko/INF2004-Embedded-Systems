/** @file host_main.c Host entry point; test data never touches hardware. */
#include "buddy4_cases.h"

#include <stdio.h>

static void report_case (const char *p_id, bool b_pass, const char *p_text);

int main (void)
{
    const uint32_t failures = buddy4_cases_run(report_case);

    (void)printf("[TEST] buddy4 SUMMARY %u failures\n", failures);
    return (0u == failures) ? 0 : 1;
}

static void report_case (const char *p_id, bool b_pass, const char *p_text)
{
    (void)printf("[TEST] %s %s %s\n", p_id, b_pass ? "PASS" : "FAIL", p_text);
}

/* End of file. */

