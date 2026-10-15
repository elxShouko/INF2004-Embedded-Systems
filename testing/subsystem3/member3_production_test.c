/** @file member3_production_test.c
 * @brief Production-tick host simulation of the unchanged S3 owner code.
 * Synthetic timings below are fixtures, never hardware calibration values.
 */

#include <setjmp.h>

#define CNF_TIMER_PERIOD 10
#define S3_HOST_PRODUCTION 1
#define main logical_suite_main
#include "logs/member3_rtos_test.c"
#undef main

static jmp_buf g_task_exit;
static uint64_t g_clock_us;
static uint32_t g_cycles;
static uint32_t g_reads;
static uint32_t g_received;
static uint32_t g_limit;
static bool g_b_flood_replies;
static s3_config_t g_config;

/** @brief Construct a deliberately slow, resolvable synthetic configuration. */
static s3_config_t production_config (uint32_t period_ms)
{
    s3_config_t config = {0};
    config.calibration = (ir_calibration_t){7U, 7U};
    config.period_ms = period_ms;
    config.line_stable_us = 20000U;
    config.line_lost_us = 40000U;
    config.line_found_us = 40000U;
    config.recovery_timeout_us = 500000U;
    config.barcode_quiet_us = 250000U;
    config.barcode_timeout_us = 10000000U;
    config.max_sample_gap_us = period_ms * 1000U + 10000U;
    config.odometry_stale_us = 100000U;
    config.barcode_rearm_us = 500000U;
    return config;
}

/** @brief Enter one calibrated production context without invoking 1 ms tests. */
static void production_init (s3_context_t * p_context, uint32_t period_ms)
{
    memset(p_context, 0, sizeof(*p_context));
    p_context->status.mode = LINE_MODE_PAUSED;
    nav_map_init(&p_context->map);
    s3_config_t config = production_config(period_ms);
    assert(configure(p_context, &config));
    s3_input_t request = {0};
    request.kind = S3_INPUT_MODE;
    request.value.mode = LINE_MODE_TRACKING;
    g_full = false;
    g_count = 0U;
    input(p_context, &request, 0U);
    assert(g_outputs[0].accepted);
    g_count = 0U;
}

/** @brief Deliver a sample with an explicit acquisition interval. */
static void sample_after (s3_context_t * p_context, uint32_t elapsed_us,
                          uint8_t mask)
{
    ir_sample_t sample = {
        p_context->status.raw.timestamp_us + elapsed_us, mask};
    process(p_context, &sample);
}

/** @brief Verify every permitted multiple and representative invalid periods. */
static void test_periods (void)
{
    for (uint32_t period_ms = 10U; period_ms <= 100U; period_ms += 10U)
    {
        s3_context_t context = {0};
        context.status.mode = LINE_MODE_PAUSED;
        s3_config_t config = production_config(period_ms);
        assert(configure(&context, &config));
        config.max_sample_gap_us = period_ms * 1000U - 1U;
        assert(!configure(&context, &config));
    }
    const uint32_t invalid[] = {0U, 1U, 5U, 9U, 11U, 15U, 99U, 101U,
                                UINT32_MAX};
    for (uint32_t index = 0U; index < sizeof(invalid) / sizeof(invalid[0]);
         ++index)
    {
        s3_context_t context = {0};
        context.status.mode = LINE_MODE_PAUSED;
        s3_config_t config = production_config(10U);
        config.period_ms = invalid[index];
        assert(!configure(&context, &config));
        assert(!context.status.calibrated);
    }
    puts("S3-U10 PASS: 10..100 ms multiples; unsupported periods rejected");
}

/** @brief Generate physical edge times independently of the polling clock. */
static uint32_t frame_edges (uint64_t * p_edges, uint32_t narrow_us,
                             uint32_t phase_us, bool b_reverse)
{
    const char * p_symbols[] = {"nWnnWnWnn", "WnnnnWnnW", "nWnnWnWnn"};
    uint32_t widths[BARCODE_RUN_COUNT] = {0};
    uint32_t count = 0U;
    for (uint32_t symbol = 0U; symbol < 3U; ++symbol)
    {
        for (uint32_t element = 0U; element < 9U; ++element)
        {
            widths[count++] = narrow_us *
                (('W' == p_symbols[symbol][element]) ? 3U : 1U);
        }
        if (symbol < 2U)
        {
            widths[count++] = narrow_us;
        }
    }
    assert(BARCODE_RUN_COUNT == count);
    p_edges[0] = 300000U + phase_us;
    for (uint32_t index = 0U; index < count; ++index)
    {
        const uint32_t source = b_reverse ? count - 1U - index : index;
        p_edges[index + 1U] = p_edges[index] + widths[source];
    }
    return count + 1U;
}

/** @brief Sample an independent waveform, including transitions between polls. */
static void scan_frame (s3_context_t * p_context, uint32_t narrow_us,
                        uint32_t phase_us, bool b_reverse, bool b_jitter)
{
    uint64_t edges[BARCODE_RUN_COUNT + 1U] = {0};
    const uint32_t count = frame_edges(edges, narrow_us, phase_us, b_reverse);
    uint32_t edge = 0U;
    uint32_t sample_index = 0U;
    bool b_black = false;
    const uint64_t end_us = edges[count - 1U] + 300000U;
    while (p_context->status.raw.timestamp_us < end_us)
    {
        /* 10/12/11 ms acquisition gaps model dispatch/work variation. */
        const uint32_t interval = p_context->config.period_ms * 1000U +
            (b_jitter ? (sample_index % 3U) * 1000U : 0U);
        const uint64_t now = p_context->status.raw.timestamp_us + interval;
        while ((edge < count) && (now >= edges[edge]))
        {
            b_black = !b_black;
            ++edge;
        }
        sample_after(p_context, interval, b_black ? 7U : 3U);
        ++sample_index;
    }
}

/** @brief Check both scan directions and phases at nominal and jittered rates. */
static void test_capture (void)
{
    const uint32_t periods[] = {10U, 20U};
    for (uint32_t index = 0U; index < 2U; ++index)
    {
        for (uint32_t direction = 0U; direction < 2U; ++direction)
        {
            for (uint32_t phase = 0U; phase < 10000U; phase += 1000U)
            {
                s3_context_t context;
                production_init(&context, periods[index]);
                scan_frame(&context, 60000U, phase, direction != 0U, false);
                assert(1U == events(NAV_EVENT_BARCODE_VALID));
                assert(S3_FAULT_NONE == context.status.fault);
            }
        }
    }
    for (uint32_t direction = 0U; direction < 2U; ++direction)
    {
        s3_context_t context;
        production_init(&context, 10U);
        scan_frame(&context, 60000U, 5000U, direction != 0U, true);
        assert(1U == events(NAV_EVENT_BARCODE_VALID));
    }
    puts("S3-U11 PASS: sampled *A*, forward/reverse, phase sweep, 10/20 ms");
}

/** @brief Check jitter boundaries, excessive gaps and reversed sample time. */
static void test_sample_gaps (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    sample_after(&context, 10000U, 3U);
    sample_after(&context, 20000U, 3U);
    assert(S3_FAULT_NONE == context.status.fault);
    sample_after(&context, 20001U, 3U);
    assert(S3_FAULT_SAMPLE_GAP == context.status.fault);
    assert(LINE_MODE_PAUSED == context.status.mode);
    production_init(&context, 10U);
    sample_after(&context, 10000U, 3U);
    const ir_sample_t backwards = {9999U, 3U};
    process(&context, &backwards);
    assert(S3_FAULT_SAMPLE_GAP == context.status.fault);
    puts("S3-U12 PASS: allowed jitter, excessive gap and reversed clock");
}

/** @brief Demonstrate that subperiod pulses can disappear without a gap fault. */
static void test_aliasing (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    scan_frame(&context, 2000U, 1000U, false, false);
    assert(0U == events(NAV_EVENT_BARCODE_VALID));
    assert(S3_FAULT_NONE == context.status.fault);
    /* A black pulse at 11..13 ms lies wholly between the 10/20 ms samples. */
    barcode_capture_t capture;
    char letter = '\0';
    assert(barcode_capture_init(&capture, 10000U, 100000U));
    assert(BARCODE_RESULT_PENDING ==
           barcode_capture_update(&capture, 0U, false, &letter));
    assert(BARCODE_RESULT_PENDING ==
           barcode_capture_update(&capture, 10000U, false, &letter));
    assert(BARCODE_RESULT_PENDING ==
           barcode_capture_update(&capture, 20000U, false, &letter));
    assert(!capture.capturing);
    assert(0U == capture.count);
    puts("S3-U13 PASS: undersampled frame rejected; hidden pulse unobservable");
}

/** @brief Supply a persistent input flood to the real owner-task loop. */
INT tk_rcv_mbf (ID id, void * p_message, TMO timeout)
{
    (void)id;
    assert(TMO_POL == timeout);
    s3_input_t message = {0};
    message.kind = S3_INPUT_ODOMETRY;
    message.value.odometry.timestamp_us = g_clock_us;
    message.value.odometry.forward_distance_um = g_clock_us;
    message.value.odometry.tracking_forward = true;
    if ((0U == g_cycles) && (0U == g_received))
    {
        message.kind = S3_INPUT_CONFIG;
        message.value.config = g_config;
    }
    else if ((0U == g_cycles) && (1U == g_received))
    {
        message.kind = S3_INPUT_MODE;
        message.value.mode = LINE_MODE_TRACKING;
    }
    else if (g_b_flood_replies)
    {
        message.kind = S3_INPUT_MAPPING;
        strcpy(message.value.mapping, "MAP A=RIGHT");
    }
    memcpy(p_message, &message, sizeof(message));
    ++g_received;
    return (INT)sizeof(message);
}

/** @brief Fake a timestamped GPIO read while counting actual task sampling. */
void s3_ir_read (ir_sample_t * p_sample)
{
    *p_sample = (ir_sample_t){g_clock_us, 3U};
    ++g_reads;
}

/** @brief Initialize the fake driver without accessing hardware. */
ER s3_ir_init (void)
{
    return E_OK;
}

/** @brief Return the same boot-relative clock used in synthetic samples. */
uint64_t s3_clock_us (void)
{
    return g_clock_us;
}

/** @brief Bound the real infinite loop and emulate output capacity/consumer. */
ER tk_dly_tsk (uint32_t delay)
{
    assert(g_config.period_ms == delay);
    assert(S3_INPUTS_PER_CYCLE == g_received);
    assert(g_reads == g_cycles + 1U);
#ifdef S3_HOST_PUBLICATION_CHECK
    S3_HOST_PUBLICATION_CHECK();
#else
    assert(g_status.raw.timestamp_us == g_clock_us);
#endif
    if (g_b_flood_replies)
    {
        assert(g_count <= S3_OUTPUT_CAPACITY);
        g_full = (g_count + S3_INPUTS_PER_CYCLE > S3_OUTPUT_CAPACITY);
    }
    else
    {
        g_count = 0U; /* Prompt consumer. */
        assert(S3_FAULT_NONE == g_status.fault);
    }
    ++g_cycles;
    if (g_cycles == g_limit)
    {
        longjmp(g_task_exit, 1);
    }
    g_received = 0U;
    g_clock_us += (uint64_t)delay * 1000U + 1000U;
    return E_OK;
}

/** @brief Verify bounded input work and continued reads with output overload. */
static void test_queue_load (bool b_flood_replies)
{
    g_config = production_config(10U);
    g_b_flood_replies = b_flood_replies;
    g_clock_us = 10000U;
    g_cycles = 0U;
    g_reads = 0U;
    g_received = 0U;
    g_limit = 50U;
    g_full = false;
    g_count = 0U;
    if (0 == setjmp(g_task_exit))
    {
        s3_navigation_task(0, NULL);
    }
    assert(g_limit == g_reads);
    if (b_flood_replies)
    {
        assert(S3_FAULT_OUTPUT_FULL == g_status.fault);
        assert(LINE_MODE_PAUSED == g_status.mode);
        assert(g_status.output_drops > 0U);
    }
    else
    {
        assert(g_status.line_valid);
        assert(LINE_OBSERVATION_CENTRED == g_status.line);
    }
    g_full = false;
    puts(b_flood_replies ? "S3-U15 PASS: output overload latches; reads continue"
                        : "S3-U14 PASS: four inputs/cycle; reads continue");
}

/** @brief Verify correlated completion and explicit recovery/resume policy. */
static void test_handoffs (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    scan_frame(&context, 60000U, 5000U, false, false);
    const uint32_t pending = context.status.pending_manoeuvre_event_id;
    assert(0U != pending);
    s3_input_t request = {0};
    request.kind = S3_INPUT_MANOEUVRE_DONE;
    request.value.manoeuvre_event_id = pending + 1U;
    input(&context, &request, context.status.raw.timestamp_us);
    assert(!g_outputs[g_count - 1U].accepted);
    request.value.manoeuvre_event_id = pending;
    input(&context, &request, context.status.raw.timestamp_us);
    assert(g_outputs[g_count - 1U].accepted);
    input(&context, &request, context.status.raw.timestamp_us);
    assert(!g_outputs[g_count - 1U].accepted);
    for (uint32_t sample = 0U; sample < 8U; ++sample)
    {
        sample_after(&context, 10000U, 3U);
    }
    assert(1U == events(NAV_EVENT_LINE_FOUND));
    assert(LINE_MODE_PAUSED == context.status.mode);
    request.kind = S3_INPUT_MODE;
    request.value.mode = LINE_MODE_TRACKING;
    input(&context, &request, context.status.raw.timestamp_us);
    assert(g_outputs[g_count - 1U].accepted);
    assert(!context.status.line_valid);
    sample_after(&context, 10000U, 3U);
    sample_after(&context, 20000U, 3U);
    assert(context.status.line_valid);
    const uint32_t outputs = g_count;
    sample_after(&context, 10000U, 3U);
    assert(outputs == g_count); /* Unchanged line needs controller refresh. */
    assert(context.status.raw.timestamp_us >
           g_outputs[outputs - 1U].navigation.timestamp_us);
    puts("S3-U16 PASS: completion correlation, stable found, explicit resume");
}

/** @brief Verify bypass suppression, recovery timeout and invalid sample fault. */
static void test_failures (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    s3_input_t request = {0};
    request.kind = S3_INPUT_MODE;
    request.value.mode = LINE_MODE_BYPASS;
    input(&context, &request, 0U);
    for (uint32_t sample = 0U; sample < 60U; ++sample)
    {
        sample_after(&context, 10000U, 0U);
    }
    assert(0U == events(NAV_EVENT_LINE_LOST));
    assert(S3_FAULT_NONE == context.status.fault);
    request.value.mode = LINE_MODE_RECOVERY;
    input(&context, &request, context.status.raw.timestamp_us);
    for (uint32_t sample = 0U; sample < 50U; ++sample)
    {
        sample_after(&context, 10000U, 0U);
    }
    assert(S3_FAULT_RECOVERY_TIMEOUT == context.status.fault);
    production_init(&context, 10U);
    sample_after(&context, 10000U, 8U);
    assert(S3_FAULT_DRIVER == context.status.fault);
    assert(LINE_MODE_PAUSED == context.status.mode);
    puts("S3-U17 PASS: bypass, recovery timeout, invalid sensor payload");
}

/** @brief Require current centred sensing throughout recovery confirmation. */
static void test_found_interruption (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    s3_input_t request = {0};
    request.kind = S3_INPUT_MODE;
    request.value.mode = LINE_MODE_RECOVERY;
    input(&context, &request, 0U);
    for (uint32_t sample = 0U; sample < 6U; ++sample)
    {
        sample_after(&context, 10000U, 3U);
    }
    assert(0U == events(NAV_EVENT_LINE_FOUND));
    /* At the confirmation deadline the filter still retains centred. */
    sample_after(&context, 10000U, 0U);
    assert(0U == events(NAV_EVENT_LINE_FOUND));
    assert(LINE_MODE_RECOVERY == context.status.mode);
    for (uint32_t sample = 0U; sample < 4U; ++sample)
    {
        sample_after(&context, 10000U, 3U);
    }
    assert(0U == events(NAV_EVENT_LINE_FOUND));
    sample_after(&context, 10000U, 3U);
    assert(1U == events(NAV_EVENT_LINE_FOUND));
    assert(LINE_MODE_PAUSED == context.status.mode);
    puts("S3-U18 PASS: raw interruption restarts stable found interval");
}

/** @brief Run production-timing simulations separately from the 1 ms suite. */
#ifndef S3_PRODUCTION_MAIN
#define S3_PRODUCTION_MAIN main
#endif
int S3_PRODUCTION_MAIN (void)
{
    test_periods();
    test_capture();
    test_sample_gaps();
    test_aliasing();
    test_queue_load(false);
    test_queue_load(true);
    test_handoffs();
    test_failures();
    test_found_interruption();
    puts("Host simulation only: no RTOS scheduling or physical speed proof.");
    return 0;
}

/*** End of file. ***/

