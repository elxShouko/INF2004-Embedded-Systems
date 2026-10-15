/** @file buddy4_cases.h Shared host/TEST=1 cases, no real sensor access. */
#ifndef BUDDY4_CASES_H
#define BUDDY4_CASES_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*buddy4_cases_report_t) (const char *p_id, bool b_pass,
                                      const char *p_description);

uint32_t buddy4_cases_run (buddy4_cases_report_t p_report);

#endif /* BUDDY4_CASES_H */

