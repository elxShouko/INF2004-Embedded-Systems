/**
 * @file    test_util.h
 * @brief   HOST-ONLY minimal test framework + Buddy 4 test harness helpers.
 */

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "imu/imu_events.h"
#include "imu/imu_terrain.h"
#include "scenario.h"
#include "sim_lsm303.h"

/* ------------------------------ framework ---------------------------- */
extern uint32_t g_tests_run;
extern uint32_t g_tests_failed;
extern uint32_t g_asserts;
extern bool     g_cur_failed;
extern char const * g_cur_test;
extern bool     g_verbose;
extern char const * g_filter;

#define TEST_ASSERT(cond)                                                    \
    do {                                                                     \
        g_asserts++;                                                         \
        if (!(cond)) {                                                       \
            printf("    FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
            g_cur_failed = true;                                             \
        }                                                                    \
    } while (0)

#define TEST_ASSERT_MSG(cond, ...)                                           \
    do {                                                                     \
        g_asserts++;                                                         \
        if (!(cond)) {                                                       \
            printf("    FAIL %s:%d: %s -- ", __FILE__, __LINE__, #cond);     \
            printf(__VA_ARGS__);                                             \
            printf("\n");                                                    \
            g_cur_failed = true;                                             \
        }                                                                    \
    } while (0)

#define TEST_ASSERT_EQ(a, b)                                                 \
    TEST_ASSERT_MSG((long long)(a) == (long long)(b), "%lld != %lld",        \
                    (long long)(a), (long long)(b))

#define TEST_ASSERT_NEAR(a, b, tol)                                          \
    TEST_ASSERT_MSG(fabs((double)(a) - (double)(b)) <= (double)(tol),        \
                    "%.4f vs %.4f (tol %.4f)", (double)(a), (double)(b),     \
                    (double)(tol))

#define RUN_TEST(fn)  test_run(#fn, fn)

void test_run(char const * p_name, void (*p_fn)(void));
void test_section(char const * p_title);

/* --------------------------- event capture --------------------------- */
#define CAP_MAX 512U

typedef struct
{
    imu_event_t ev[CAP_MAX];
    uint32_t    count;        /* total accepted */
    uint32_t    capacity;     /* emulated queue capacity (<= CAP_MAX)   */
    uint32_t    level;        /* current queue level (drained by test)  */
    uint32_t    notified;
    imu_event_t last_notified;
    bool        b_error;      /* sink returns ERROR                      */
} capture_t;

extern capture_t g_cap;

void      cap_reset(uint32_t capacity);
imu_event_sink_t cap_sink(void);
uint32_t  cap_count_type(imu_event_type_t type);
imu_event_t const * cap_last_of(imu_event_type_t type);
void      cap_drain(void);

/* ------------------------------ harness ------------------------------ */
typedef struct
{
    imu_terrain_t sys;
    imu_params_t  params;
    scenario_t    scn;
    uint32_t      now_ms;
    bool          b_hint;      /* emulate Buddy 2 encoder hint     */
    bool          b_no_drain;  /* MC does not read its queue       */
} harness_t;

extern harness_t g_h;

/** Reset sim + capture, default params (callers may edit g_h.params
 *  before harness_start). Scenario defaults: stationary, level. */
void harness_setup(void);

/** Initialise imu_terrain with g_h.params and attach scenario stimulus. */
imu_err_t harness_start(void);

/** Run for @p ms of simulated time (steps at the sample period). */
void harness_run_ms(uint32_t ms);

/** Run until the subsystem reaches MONITORING (returns false on timeout). */
bool harness_until_monitoring(uint32_t max_ms);

/** Run until @p state (returns false on timeout). */
bool harness_until_state(imu_state_t state, uint32_t max_ms);

imu_status_t harness_status(void);

/** Did the trace ever contain a transition into @p state? */
bool harness_trace_has(imu_state_t state);

/* ------------------------------ suites -------------------------------- */
void suite_unit_driver(void);
void suite_unit_math_cal(void);
void suite_unit_processing(void);
void suite_unit_hump(void);
void suite_unit_events(void);
void suite_blackbox(void);
void suite_robustness(void);
void suite_rtos(void);

#endif /* TEST_UTIL_H */
