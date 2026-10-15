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
extern bool     g_b_cur_failed;
extern char const * g_cur_test;
extern bool     g_b_verbose;
extern char const * g_filter;

/* These wrappers capture the call-site location. Each operand is evaluated
 * exactly once; the checking and reporting are ordinary functions. */
#define TEST_ASSERT(cond) \
    (test_check((cond), __FILE__, __LINE__, #cond))
#define TEST_ASSERT_MSG(cond, ...) \
    (test_message((cond), __FILE__, __LINE__, __VA_ARGS__))
#define TEST_ASSERT_EQ(a, b) \
    (test_equal((int64_t)(a), (int64_t)(b), __FILE__, __LINE__))
#define TEST_ASSERT_NEAR(a, b, tol) \
    (test_near((double)(a), (double)(b), (double)(tol), __FILE__, __LINE__))

void test_check(bool b_pass, char const * p_file, int line,
                char const * p_expression);
void test_message(bool b_pass, char const * p_file, int line,
                  char const * p_format, ...);
void test_equal(int64_t actual, int64_t expected, char const * p_file, int line);
void test_near(double actual, double expected, double tolerance,
              char const * p_file, int line);

void test_run(char const * p_id, char const * p_name, void (*p_fn)(void));
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
