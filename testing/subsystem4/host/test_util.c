/**
 * @file    test_util.c
 * @brief   HOST-ONLY test framework and harness (see test_util.h).
 */

#include <string.h>
#include <inttypes.h>
#include <stdarg.h>

#include "test_util.h"

uint32_t     g_tests_run    = 0U;
uint32_t     g_tests_failed = 0U;
uint32_t     g_asserts      = 0U;
bool         g_b_cur_failed   = false;
char const * g_cur_test     = "";
bool         g_b_verbose      = false;
char const * g_filter       = NULL;

capture_t g_cap;
harness_t g_h;

void test_check(bool b_pass, char const * p_file, int line,
                char const * p_expression)
{
    test_message(b_pass, p_file, line, "%s", p_expression);
}

void test_message(bool b_pass, char const * p_file, int line,
                  char const * p_format, ...)
{
    g_asserts++;
    if (!b_pass)
    {
        va_list arguments;
        va_start(arguments, p_format);
        (void)printf("    FAIL %s:%d: ", p_file, line);
        (void)vprintf(p_format, arguments);
        (void)printf("\n");
        va_end(arguments);
        g_b_cur_failed = true;
    }
}

void test_equal(int64_t actual, int64_t expected, char const * p_file, int line)
{
    test_message(actual == expected, p_file, line,
                 "actual=%" PRId64 " expected=%" PRId64, actual, expected);
}

void test_near(double actual, double expected, double tolerance,
              char const * p_file, int line)
{
    bool b_pass = isfinite(actual) && isfinite(expected) && isfinite(tolerance)
                  && (tolerance >= 0.0)
                  && (fabs(actual - expected) <= tolerance);
    test_message(b_pass, p_file, line, "actual=%.4f expected=%.4f tolerance=%.4f",
                 actual, expected, tolerance);
}

void test_run(char const * p_id, char const * p_name, void (*p_fn)(void))
{
    if ((NULL != g_filter) && (NULL == strstr(p_name, g_filter))
        && (NULL == strstr(p_id, g_filter)))
    {
        return;
    }
    g_cur_test   = p_name;
    g_b_cur_failed = false;
    g_tests_run++;
    p_fn();
    if (g_b_cur_failed)
    {
        g_tests_failed++;
        printf("[TEST] %s FAIL %s\n", p_id, p_name);
    }
    else
    {
        printf("[TEST] %s PASS %s\n", p_id, p_name);
    }
}

void test_section(char const * p_title)
{
    if (NULL == g_filter)
    {
        printf("\n== %s ==\n", p_title);
    }
}

/* ----------------------------- capture -------------------------------- */

static imu_sink_result_t cap_send(void * p_ctx, imu_event_t const * p_evt)
{
    (void)p_ctx;

    if (g_cap.b_error)
    {
        return (IMU_SINK_ERROR);
    }
    if (g_cap.level >= g_cap.capacity)
    {
        return (IMU_SINK_FULL);
    }
    if (g_cap.count < CAP_MAX)
    {
        g_cap.ev[g_cap.count] = *p_evt;
    }
    g_cap.count++;
    g_cap.level++;
    if (g_b_verbose)
    {
        printf("      evt t=%u seq=%u %s sev=%u state=%s\n",
               (unsigned)p_evt->timestamp_ms, (unsigned)p_evt->seq,
               imu_event_type_name((imu_event_type_t)p_evt->type),
               (unsigned)p_evt->severity,
               imu_state_name((imu_state_t)p_evt->state));
    }
    return (IMU_SINK_OK);
}

static void cap_notify(void * p_ctx, imu_event_t const * p_evt)
{
    (void)p_ctx;
    g_cap.notified++;
    g_cap.last_notified = *p_evt;
}

void cap_reset(uint32_t capacity)
{
    (void)memset(&g_cap, 0, sizeof(g_cap));
    g_cap.capacity = (capacity > CAP_MAX) ? CAP_MAX : capacity;
}

imu_event_sink_t cap_sink(void)
{
    imu_event_sink_t s;

    s.p_send   = cap_send;
    s.p_notify = cap_notify;
    s.p_ctx    = NULL;
    return (s);
}

uint32_t cap_count_type(imu_event_type_t type)
{
    uint32_t i;
    uint32_t n = 0U;
    uint32_t lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;

    for (i = 0U; i < lim; i++)
    {
        if (g_cap.ev[i].type == (uint8_t)type)
        {
            n++;
        }
    }
    return (n);
}

imu_event_t const * cap_last_of(imu_event_type_t type)
{
    uint32_t lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    uint32_t i;

    for (i = lim; i > 0U; i--)
    {
        if (g_cap.ev[i - 1U].type == (uint8_t)type)
        {
            return (&g_cap.ev[i - 1U]);
        }
    }
    return (NULL);
}

void cap_drain(void)
{
    g_cap.level = 0U;
}

/* ----------------------------- harness -------------------------------- */

void harness_setup(void)
{
    sim_reset();
    cap_reset(CAP_MAX);
    (void)memset(&g_h, 0, sizeof(g_h));
    imu_params_default(&g_h.params);
    scn_init(&g_h.scn);
}

imu_err_t harness_start(void)
{
    imu_event_sink_t sink = cap_sink();

    sim_cfg()->p_fn  = scn_stimulus;
    sim_cfg()->p_ctx = &g_h.scn;
    g_h.now_ms = sim_time_ms();

    return (imu_terrain_init(&g_h.sys, &g_h.params, &sink));
}

void harness_run_ms(uint32_t ms)
{
    uint32_t period = g_h.params.sample_period_ms;
    uint32_t t;

    for (t = 0U; t < ms; t += period)
    {
        sim_advance_ms(period);
        g_h.now_ms = sim_time_ms();
        if (g_h.b_hint)
        {
            imu_motion_hint_t h;
            double            v;
            double            a;

            scn_speed_at(&g_h.scn, (double)g_h.now_ms * 0.001, &v, &a);
            h.b_valid       = true;
            h.b_accel_valid = true;
            h.speed_mm_s    = (int32_t)lround(v * 1000.0);
            h.accel_mm_s2   = (int32_t)lround(a * 1000.0);
            h.timestamp_ms  = g_h.now_ms;
            (void)imu_terrain_set_hint(&g_h.sys, &h);
        }
        imu_terrain_step(&g_h.sys, g_h.now_ms);
        if (!g_h.b_no_drain)
        {
            cap_drain();      /* the "Mission Controller" drains its queue */
        }
    }
}

bool harness_until_state(imu_state_t state, uint32_t max_ms)
{
    uint32_t t;
    uint32_t period = g_h.params.sample_period_ms;

    for (t = 0U; t < max_ms; t += period)
    {
        if (g_h.sys.state == state)
        {
            return (true);
        }
        harness_run_ms(period);
    }
    return (g_h.sys.state == state);
}

bool harness_until_monitoring(uint32_t max_ms)
{
    return (harness_until_state(IMU_STATE_MONITORING, max_ms));
}

imu_status_t harness_status(void)
{
    imu_status_t st;

    (void)imu_terrain_get_status(&g_h.sys, &st);
    return (st);
}

bool harness_trace_has(imu_state_t state)
{
    imu_trace_entry_t tr[IMU_STATE_TRACE_LEN];
    uint8_t           n = imu_terrain_get_trace(&g_h.sys, tr,
                                                (uint8_t)IMU_STATE_TRACE_LEN);
    uint8_t           i;

    for (i = 0U; i < n; i++)
    {
        if (tr[i].to == (uint8_t)state)
        {
            return (true);
        }
    }
    return (false);
}
