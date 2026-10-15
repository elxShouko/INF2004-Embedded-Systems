/** @file member3_feedback_test.c
 * @brief Bounded copied-status and navigation handshake host regressions.
 */

static void check_publication (void);
#define S3_HOST_MUTEX 1
#define S3_HOST_PUBLICATION_CHECK check_publication
/* Include existing fixtures; keep this suite's entry point independent. */
#define S3_PRODUCTION_MAIN production_suite_main
#include "member3_production_test.c"

static ER g_lock_result = E_OK;
static ER g_unlock_result = E_OK;
static uint32_t g_lock_calls;
static uint32_t g_unlock_calls;

/** @brief Simulate contention and assert every production acquisition polls. */
ER tk_loc_mtx (ID id, TMO timeout)
{
    assert(id == mtx_s3_status);
    assert(TMO_POL == timeout);
    ++g_lock_calls;
    return g_lock_result;
}

/** @brief Inject release failure without modelling real kernel ownership. */
ER tk_unl_mtx (ID id)
{
    assert(id == mtx_s3_status);
    ++g_unlock_calls;
    return g_unlock_result;
}

/** @brief Check publication freshness or retained acquisition time on busy. */
static void check_publication (void)
{
    if (E_TMOUT == g_lock_result)
    {
        assert(123U == g_status.raw.timestamp_us);
    }
    else
    {
        assert(g_status.raw.timestamp_us == g_clock_us);
    }
}

/** @brief Check polling contention, all error exits and timestamp preservation. */
static void test_status_poll (void)
{
    s3_status_t output;
    memset(&output, 0xa5, sizeof(output));
    unsigned char unchanged[sizeof(output)];
    memcpy(unchanged, &output, sizeof(output));
    mtx_s3_status = 1;
    g_lock_calls = 0U;
    g_unlock_calls = 0U;
    assert(E_PAR == s3_get_status(NULL));
    mtx_s3_status = 0;
    assert(E_OBJ == s3_get_status(&output));
    assert(0U == g_lock_calls);
    assert(0 == memcmp(unchanged, &output, sizeof(output)));
    mtx_s3_status = 1;
    g_lock_result = E_TMOUT;
    assert(E_TMOUT == s3_get_status(&output));
    assert(1U == g_lock_calls);
    assert(0U == g_unlock_calls);
    assert(0 == memcmp(unchanged, &output, sizeof(output)));
    g_lock_result = E_OBJ;
    assert(E_OBJ == s3_get_status(&output));
    assert(0 == memcmp(unchanged, &output, sizeof(output)));
    g_lock_result = E_OK;
    g_unlock_result = E_OBJ;
    assert(E_OBJ == s3_get_status(&output));
    assert(1U == g_unlock_calls);
    assert(0 == memcmp(unchanged, &output, sizeof(output)));
    g_unlock_result = E_OK;
    g_status = (s3_status_t){0};
    g_status.raw = (ir_sample_t){123U, 1U};
    g_status.black_mask = 1U;
    g_status.calibrated = true;
    g_status.line_valid = true;
    g_status.line = LINE_OBSERVATION_CENTRED;
    g_status.mode = LINE_MODE_TRACKING;
    g_status.state = LINE_STATE_AWAITING_MANOEUVRE;
    g_status.fault = S3_FAULT_SAMPLE_GAP;
    g_status.pending_manoeuvre_event_id = 42U;
    g_status.map_revision = 7U;
    g_status.mapping[0] = NAV_COMMAND_LEFT;
    assert(E_OK == s3_get_status(&output));
    assert(output.calibrated && output.line_valid);
    assert(123U == output.raw.timestamp_us);
    assert(1U == output.raw.levels && 1U == output.black_mask);
    assert(LINE_OBSERVATION_CENTRED == output.line);
    assert(LINE_MODE_TRACKING == output.mode);
    assert(LINE_STATE_AWAITING_MANOEUVRE == output.state);
    assert(S3_FAULT_SAMPLE_GAP == output.fault);
    assert(42U == output.pending_manoeuvre_event_id);
    assert(7U == output.map_revision);
    assert(NAV_COMMAND_LEFT == output.mapping[0]);
    g_clock_us = 1000000U;
    assert(E_OK == s3_get_status(&output));
    assert(123U == output.raw.timestamp_us);
    assert(123U == g_status.raw.timestamp_us);
    puts("S3-U20 PASS: polling timeout/errors, coherent fields, read != sample");
}

/** @brief Observe filter lag and changing raw time without repeat events. */
static void test_snapshot_semantics (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    for (uint32_t sample = 0U; sample < 4U; ++sample)
    {
        sample_after(&context, 10000U, 3U);
    }
    const uint32_t count = g_count;
    const uint64_t timestamp = context.status.raw.timestamp_us;
    sample_after(&context, 10000U, 3U);
    assert(count == g_count);
    assert(timestamp + 10000U == context.status.raw.timestamp_us);
    sample_after(&context, 10000U, 1U);
    g_status = context.status;
    s3_status_t output = {0};
    assert(E_OK == s3_get_status(&output));
    assert(output.line_valid);
    assert(1U == output.black_mask);
    assert(LINE_OBSERVATION_CENTRED == output.line);
    line_observation_t current = LINE_OBSERVATION_ABSENT;
    assert(line_classify(output.black_mask, &current));
    assert(current != output.line);
    sample_after(&context, 20001U, 1U);
    g_status = context.status;
    assert(E_OK == s3_get_status(&output));
    assert(S3_FAULT_SAMPLE_GAP == output.fault);
    assert(LINE_MODE_PAUSED == output.mode);
    puts("S3-U21 PASS: unchanged-line freshness, raw/filter lag, sample gap");
}

/** @brief Ensure contention skips publication without stopping owner samples. */
static void test_busy_publication (void)
{
    g_status = (s3_status_t){0};
    g_status.raw.timestamp_us = 123U;
    g_status.line_valid = true;
    g_status.line = LINE_OBSERVATION_CENTRED;
    g_lock_result = E_TMOUT;
    g_lock_calls = 0U;
    g_unlock_calls = 0U;
    test_queue_load(false);
    assert(50U == g_reads);
    assert(50U == g_lock_calls);
    assert(0U == g_unlock_calls);
    assert(123U == g_status.raw.timestamp_us);
    g_lock_result = E_OK;
    test_queue_load(false);
    assert(g_status.raw.timestamp_us > 123U);
    puts("S3-U22 PASS: contended publication skips; sampling continues");
}

/** @brief Preserve pending ownership across paused/cancelled manoeuvres. */
static void test_cancelled_completion (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    scan_frame(&context, 60000U, 5000U, false, false);
    const uint32_t pending = context.status.pending_manoeuvre_event_id;
    assert(0U != pending);
    /* Queued/failed motion is not completion: withholding DONE keeps waiting. */
    for (uint32_t sample = 0U; sample < 8U; ++sample)
    {
        sample_after(&context, 10000U, 3U);
    }
    assert(LINE_STATE_AWAITING_MANOEUVRE == context.status.state);
    assert(pending == context.status.pending_manoeuvre_event_id);
    assert(0U == events(NAV_EVENT_LINE_FOUND));
    s3_input_t request = {0};
    request.kind = S3_INPUT_MODE;
    request.value.mode = LINE_MODE_PAUSED;
    input(&context, &request, context.status.raw.timestamp_us);
    assert(g_outputs[g_count - 1U].accepted);
    request.kind = S3_INPUT_MANOEUVRE_DONE;
    request.value.manoeuvre_event_id = pending;
    input(&context, &request, context.status.raw.timestamp_us);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(pending == context.status.pending_manoeuvre_event_id);
    assert(LINE_MODE_PAUSED == context.status.mode);
    for (uint32_t sample = 0U; sample < 8U; ++sample)
    {
        sample_after(&context, 10000U, 3U);
    }
    assert(0U == events(NAV_EVENT_LINE_FOUND));
    request.kind = S3_INPUT_MODE;
    request.value.mode = LINE_MODE_TRACKING;
    input(&context, &request, context.status.raw.timestamp_us);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(pending == context.status.pending_manoeuvre_event_id);
    puts("S3-U23 PASS: paused completion cannot release pending manoeuvre");
}

/** @brief Reject a short noisy barcode burst without losing front feedback. */
static void test_barcode_noise (void)
{
    s3_context_t context;
    production_init(&context, 10U);
    for (uint32_t sample = 0U; sample < 30U; ++sample)
    {
        sample_after(&context, 10000U, 3U);
    }
    for (uint32_t sample = 0U; sample < 5U; ++sample)
    {
        sample_after(&context, 10000U, (sample % 2U) ? 3U : 7U);
    }
    for (uint32_t sample = 0U; sample < 30U; ++sample)
    {
        sample_after(&context, 10000U, 3U);
    }
    assert(0U == events(NAV_EVENT_BARCODE_VALID));
    assert(1U == events(NAV_EVENT_BARCODE_INVALID));
    assert(0U == context.status.pending_manoeuvre_event_id);
    assert(context.status.line_valid);
    assert(LINE_OBSERVATION_CENTRED == context.status.line);
    puts("S3-U24 PASS: barcode noise rejected; front sensing continues");
}

/** @brief Exercise configuration/map reply correlation and atomic rejection. */
static void test_config_delivery (void)
{
    s3_context_t context = {0};
    context.status.mode = LINE_MODE_PAUSED;
    nav_map_init(&context.map);
    g_count = 0U;
    s3_input_t request = {0};
    request.kind = S3_INPUT_CONFIG;
    request.request_id = 101U;
    request.value.config = production_config(10U);
    input(&context, &request, 0U);
    assert(g_outputs[g_count - 1U].accepted);
    assert(101U == g_outputs[g_count - 1U].request_id);
    assert(context.status.calibrated);
    assert(!context.status.line_valid);
    request.kind = S3_INPUT_MAPPING;
    request.request_id = 102U;
    strcpy(request.value.mapping, "MAP A=LEFT B=RIGHT C=FORWARD D=UTURN");
    input(&context, &request, 0U);
    assert(g_outputs[g_count - 1U].accepted);
    assert(102U == g_outputs[g_count - 1U].request_id);
    const uint32_t revision = context.map.revision;
    assert(revision == g_outputs[g_count - 1U].map_revision);
    strcpy(request.value.mapping, "MAP A=INVALID");
    input(&context, &request, 0U);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(revision == context.map.revision);
    assert(NAV_COMMAND_LEFT == nav_map_lookup(&context.map, 'A'));
    request.kind = S3_INPUT_MODE;
    request.request_id = 103U;
    request.value.mode = LINE_MODE_TRACKING;
    input(&context, &request, 0U);
    assert(g_outputs[g_count - 1U].accepted);
    assert(103U == g_outputs[g_count - 1U].request_id);
    request.kind = S3_INPUT_CONFIG;
    request.value.config = production_config(20U);
    input(&context, &request, 0U);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(10U == context.config.period_ms);
    puts("S3-U25 PASS: config/map reply IDs and rejected updates preserved");
}

/** @brief Exercise provider feedback without a controller or physical motors. */
int main (void)
{
    test_status_poll();
    test_snapshot_semantics();
    test_busy_publication();
    test_cancelled_completion();
    test_barcode_noise();
    test_config_delivery();
    test_handoffs();
    test_found_interruption();
    puts("Host fakes only; no real mutex latency/SMP/hardware claim.");
    return 0;
}

/*** End of file. ***/

