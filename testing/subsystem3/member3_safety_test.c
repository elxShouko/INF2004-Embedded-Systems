/** @file member3_safety_test.c
 * @brief Pacing failures, identifier limits and source-evidence lifetime.
 */

#include <stdint.h>
static int32_t g_delay_result;
static uint32_t g_delay_calls;
#define S3_HOST_DELAY_FAILURE 1
#define S3_HOST_EXIT_TEST 1
#define S3_FEEDBACK_MAIN retained_feedback_main
#include "member3_feedback_test.c"

static uint32_t g_exit_calls;

/** @brief Emulate the non-returning kernel exit without ending the host suite. */
void tk_ext_tsk (void)
{
    ++g_exit_calls;
    longjmp(g_task_exit, 1);
}

static void delay_failure (ER error, bool b_full, bool b_busy)
{
    g_config = production_config(10U);
    g_clock_us = 10000U;
    g_cycles = 0U;
    g_reads = 0U;
    g_received = 0U;
    g_limit = 1000U;
    g_count = 0U;
    g_full = b_full;
    g_b_flood_replies = false;
    g_delay_result = error;
    g_delay_calls = 0U;
    g_exit_calls = 0U;
    g_lock_result = b_busy ? E_TMOUT : E_OK;
    g_unlock_result = E_OK;
    g_status = (s3_status_t){0};
    g_status.raw.timestamp_us = 123U;
    if (0 == setjmp(g_task_exit))
    {
        s3_navigation_task(0, NULL);
        assert(false);
    }
    assert(1U == g_delay_calls);
    assert(1U == g_exit_calls);
    assert(1U == g_reads);
    assert(0U == g_cycles);
    if (b_busy)
    {
        assert(123U == g_status.raw.timestamp_us);
    }
    else
    {
        assert(g_clock_us == g_status.raw.timestamp_us);
        assert((g_status.fault & S3_FAULT_KERNEL) != 0U);
        assert(LINE_MODE_PAUSED == g_status.mode);
        assert(LINE_STATE_FAULT_REPORTED == g_status.state);
        if (b_full)
        {
            assert((g_status.fault & S3_FAULT_OUTPUT_FULL) != 0U);
        }
    }
    g_delay_result = E_OK;
    g_full = false;
    g_lock_result = E_OK;
}

static void test_pacing (void)
{
    delay_failure(E_OBJ, false, false);
    delay_failure(E_TMOUT, false, false);
    delay_failure(E_PAR, true, false);
    delay_failure(E_OBJ, true, true);
    puts("S3-U32 PASS: first failed delay faults/exits; no retry or busy loop");
}

static void test_identifiers (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    g_count = 0U;
    context.event_id = UINT32_MAX - 2U;
    context.status.raw.timestamp_us = UINT64_C(0x100000123);
    context.status.pending_manoeuvre_event_id = 777U;
    s3_output_t output = {0};
    output.kind = S3_OUTPUT_REPLY;
    emit(&context, &output);
    assert(UINT32_MAX - 1U == g_outputs[0].event_id);
    emit(&context, &output);
    assert(UINT32_MAX == g_outputs[1].event_id);
    assert(NAV_EVENT_FAULT == g_outputs[1].navigation.kind);
    assert(NAV_COMMAND_NONE == g_outputs[1].navigation.command);
    assert((context.status.fault & S3_FAULT_KERNEL) != 0U);
    assert(777U == context.status.pending_manoeuvre_event_id);
    const uint32_t count = g_count;
    for (uint32_t repeat = 0U; repeat < 4U; ++repeat)
    {
        emit(&context, &output);
    }
    assert(count == g_count);
    assert(UINT32_MAX == context.event_id);
    s3_config_t config = production_config(10U);
    assert(!configure(&context, &config));
    s3_input_t done = {0};
    done.kind = S3_INPUT_MANOEUVRE_DONE;
    done.value.manoeuvre_event_id = 777U;
    input(&context, &done, 0U);
    assert(777U == context.status.pending_manoeuvre_event_id);
    context.status.output_drops = UINT32_MAX;
    emit(&context, &output);
    assert(UINT32_MAX == context.status.output_drops);
    production_init(&context, 10U);
    context.status.output_drops = UINT32_MAX;
    g_full = true;
    emit(&context, &output);
    assert(UINT32_MAX == context.status.output_drops);
    g_full = false;
    production_init(&context, 10U);
    context.map.revision = UINT32_MAX;
    nav_update_t update;
    assert(nav_update_parse(&update, "MAP A=LEFT"));
    const nav_map_t original = context.map;
    assert(!nav_map_commit(&context.map, &update));
    assert(0 == memcmp(&original, &context.map, sizeof(original)));
    puts("S3-U33 PASS: final ID is a fault; no wrap/reuse; drops saturate");
}

static void test_odometry (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    const uint64_t now = UINT64_C(0x100000123);
    s3_odometry_t odometry = {now, 100U, true};
    assert(accept_odometry(&context, &odometry, now));
    assert(accept_odometry(&context, &odometry,
                           now + context.config.odometry_stale_us));
    assert(now == context.odometry_us);
    assert(!accept_odometry(&context, &odometry,
                            now + context.config.odometry_stale_us + 1U));
    assert(!context.b_odometry_usable);
    assert(context.b_have_odometry);
    assert(now == context.odometry_us);
    odometry.forward_distance_um = 101U;
    assert(!accept_odometry(&context, &odometry, now));
    assert(100U == context.distance_um);
    odometry.timestamp_us = now - 1U;
    assert(!accept_odometry(&context, &odometry, now));
    odometry.timestamp_us = now + 1U;
    assert(!accept_odometry(&context, &odometry, now));
    assert(accept_odometry(&context, &odometry, now + 1U));
    assert(context.b_odometry_usable);
    odometry.forward_distance_um = 100U;
    ++odometry.timestamp_us;
    assert(!accept_odometry(&context, &odometry, now + 2U));
    odometry.timestamp_us = UINT64_MAX - 10U;
    odometry.forward_distance_um = UINT64_MAX;
    assert(accept_odometry(&context, &odometry, UINT64_MAX));
    assert(!accept_odometry(&context, &odometry, 0U));
    puts("S3-U34 PASS: age boundary, no re-stamp, duplicates and 64-bit limits");
}

static void test_observation_time (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    const uint64_t start = UINT64_C(0xfffff000);
    ir_sample_t sample = {start, 3U};
    process(&context, &sample);
    sample.timestamp_us += 10000U;
    process(&context, &sample);
    assert(!context.status.line_valid);
    sample.timestamp_us += 10000U;
    process(&context, &sample);
    assert(context.status.line_valid);
    assert(sample.timestamp_us > UINT32_MAX);
    assert(sample.timestamp_us == context.status.raw.timestamp_us);
    assert(sample.timestamp_us ==
           g_outputs[g_count - 1U].navigation.timestamp_us);
    sample.levels = 1U;
    sample.timestamp_us += 10000U;
    const uint32_t before = g_count;
    process(&context, &sample);
    assert(before == g_count);
    assert(LINE_OBSERVATION_CENTRED == context.status.line);
    assert(1U == context.status.black_mask);
    sample.timestamp_us += 20000U;
    process(&context, &sample);
    assert(before + 1U == g_count);
    assert(LINE_OBSERVATION_CORRECT_LEFT == context.status.line);
    assert(sample.timestamp_us ==
           g_outputs[g_count - 1U].navigation.timestamp_us);
    puts("S3-U35 PASS: 64-bit time and same-update confirmation event");
}

int main (void)
{
    mtx_s3_status = 1;
    mbf_s3_input = 2;
    mbf_s3_output = 3;
    test_pacing();
    test_identifiers();
    test_odometry();
    test_observation_time();
    puts("Host simulation only; target latency remains unmeasured.");
    return 0;
}

/*** End of file. ***/

