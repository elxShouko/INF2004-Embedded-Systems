/** @file member3_minbits_test.c
 * @brief Exhaustive action packing and real compact transport regressions.
 */

#define S3_HOST_CODEC_QUEUE 1
#define main retained_owner_main
#include "member3_rtos_test.c"
#undef main

static uint8_t g_receive_bytes[S3_INPUT_WIRE_MAX];
static INT g_receive_size = E_TMOUT;

INT tk_rcv_mbf (ID id, void * p_message, TMO timeout)
{
    (void)id;
    (void)timeout;
    if (g_receive_size > 0)
    {
        memcpy(p_message, g_receive_bytes, (uint32_t)g_receive_size);
    }
    return g_receive_size;
}

static void test_actions (void)
{
    uint8_t packed[NAV_ACTION_BYTES] = {0};
    for (uint32_t slot = 0U; slot < NAV_ALPHABET_SIZE; ++slot)
    {
        for (uint32_t action = 0U; action <= NAV_COMMAND_U_TURN; ++action)
        {
            uint8_t before[NAV_ACTION_BYTES];
            memcpy(before, packed, sizeof(before));
            assert(nav_action_set(packed, slot, (nav_command_t)action));
            assert(action == (uint32_t)nav_action_get(packed, slot));
            for (uint32_t other = 0U; other < NAV_ALPHABET_SIZE; ++other)
            {
                if (slot != other)
                {
                    assert(nav_action_get(before, other) ==
                           nav_action_get(packed, other));
                }
            }
        }
    }
    nav_update_t update = {0};
    update.selected = 1U;
    update.actions[0] = 7U;
    assert(!nav_update_valid(&update));
    update.actions[0] = NAV_COMMAND_LEFT;
    update.selected = UINT32_MAX;
    assert(!nav_update_valid(&update));
    uint8_t before[NAV_ACTION_BYTES];
    memcpy(before, packed, sizeof(before));
    assert(!nav_action_set(packed, 26U, NAV_COMMAND_LEFT));
    assert(!nav_action_set(packed, 0U, (nav_command_t)5U));
    assert(0 == memcmp(before, packed, sizeof(before)));
    assert(0U == (packed[9] & 0xc0U));
    puts("S3-U26 PASS: every action/slot; neighbours and padding preserved");
}

static void check_input (const s3_input_t * p_input, uint32_t expected)
{
    uint8_t bytes[S3_INPUT_WIRE_MAX];
    uint8_t encoded[S3_INPUT_WIRE_MAX];
    s3_input_t output = {0};
    assert(expected == s3_input_encode(p_input, bytes));
    assert(s3_input_decode(bytes, expected, &output));
    assert(output.kind == p_input->kind);
    assert(output.request_id == p_input->request_id);
    assert(expected == s3_input_encode(&output, encoded));
    assert(0 == memcmp(bytes, encoded, expected));
    unsigned char before[sizeof(output)];
    memcpy(before, &output, sizeof(before));
    for (uint32_t truncated = 0U; truncated < expected; ++truncated)
    {
        assert(!s3_input_decode(bytes, truncated, &output));
        assert(0 == memcmp(before, &output, sizeof(before)));
    }
    assert(!s3_input_decode(bytes, expected + 1U, &output));
    bytes[expected - 1U] |= 0x80U;
    assert(!s3_input_decode(bytes, expected, &output));
    assert(0 == memcmp(before, &output, sizeof(before)));
}

static void test_inputs (void)
{
    s3_input_t input_message = {0};
    input_message.request_id = UINT32_MAX;
    input_message.kind = S3_INPUT_CONFIG;
    input_message.value.config.calibration = (ir_calibration_t){7U, 7U};
    input_message.value.config.period_ms = 100U;
    input_message.value.config.barcode_timeout_us = UINT32_MAX;
    input_message.value.config.junction.max_span_um = UINT32_MAX;
    check_input(&input_message, 79U);
    input_message.kind = S3_INPUT_MODE;
    for (uint32_t mode = 0U; mode <= LINE_MODE_RECOVERY; ++mode)
    {
        input_message.value.mode = (line_mode_t)mode;
        check_input(&input_message, 5U);
    }
    input_message.request_id = 0x12345678U;
    input_message.value.mode = LINE_MODE_RECOVERY;
    uint8_t golden[S3_INPUT_WIRE_MAX];
    const uint8_t expected[] = {0xc1U, 0xb3U, 0xa2U, 0x91U, 0x18U};
    assert(5U == s3_input_encode(&input_message, golden));
    assert(0 == memcmp(expected, golden, sizeof(expected)));
    input_message.kind = S3_INPUT_MAPPING;
    assert(nav_update_parse(&input_message.value.mapping, "MAP A=LEFT Z=UTURN B=NONE"));
    check_input(&input_message, 18U);
    input_message.kind = S3_INPUT_ODOMETRY;
    input_message.value.odometry.timestamp_us = UINT64_MAX;
    input_message.value.odometry.forward_distance_um = UINT64_MAX;
    input_message.value.odometry.tracking_forward = true;
    check_input(&input_message, 21U);
    input_message.kind = S3_INPUT_MANOEUVRE_DONE;
    input_message.value.manoeuvre_event_id = UINT32_MAX;
    check_input(&input_message, 9U);
    assert(0U == s3_input_encode(NULL, g_receive_bytes));
    assert(!s3_input_decode(NULL, 5U, &input_message));
    puts("S3-U27 PASS: input lengths, round trips, truncation and padding");
}

static void test_outputs (void)
{
    static const uint32_t sizes[] = {19U, 18U, 13U};
    for (uint32_t kind = 0U; kind < 3U; ++kind)
    {
        for (uint32_t letter = 0U; letter <= 26U; ++letter)
        {
            s3_output_t message = {0};
            message.kind = (uint8_t)(kind & 3U);
            message.event_id = UINT32_MAX;
            message.map_revision = UINT32_MAX;
            message.request_id = UINT32_MAX;
            message.accepted = true;
            message.fault = 31U;
            message.line = LINE_OBSERVATION_ABSENT;
            message.navigation.timestamp_us = UINT64_MAX;
            message.navigation.black_mask = 7U;
            message.navigation.kind = NAV_EVENT_BARCODE_VALID;
            message.navigation.command = NAV_COMMAND_U_TURN;
            message.navigation.state = LINE_STATE_AWAITING_MANOEUVRE;
            message.navigation.barcode_character =
                (26U == letter) ? '\0' : (char)('A' + letter);
            uint8_t bytes[S3_OUTPUT_WIRE_MAX + 1U];
            uint8_t encoded[S3_OUTPUT_WIRE_MAX];
            s3_output_t decoded = {0};
            assert(sizes[kind] == s3_output_encode(&message, bytes));
            assert(s3_output_decode(bytes, sizes[kind], &decoded));
            assert(sizes[kind] == s3_output_encode(&decoded, encoded));
            assert(0 == memcmp(bytes, encoded, sizes[kind]));
            assert(UINT32_MAX == decoded.event_id);
            assert(UINT32_MAX == decoded.map_revision);
            assert(31U == decoded.fault);
            if (S3_OUTPUT_NAVIGATION == kind)
            {
                assert(UINT64_MAX == decoded.navigation.timestamp_us);
                assert(message.navigation.barcode_character ==
                       decoded.navigation.barcode_character);
            }
            unsigned char before[sizeof(decoded)];
            memcpy(before, &decoded, sizeof(before));
            for (uint32_t length = 0U; length < sizes[kind]; ++length)
            {
                assert(!s3_output_decode(bytes, length, &decoded));
                assert(0 == memcmp(before, &decoded, sizeof(before)));
            }
            assert(!s3_output_decode(bytes, sizes[kind] + 1U, &decoded));
            if (S3_OUTPUT_LINE == kind)
            {
                bytes[sizes[kind] - 1U] |= 0x80U;
                assert(!s3_output_decode(bytes, sizes[kind], &decoded));
                assert(0 == memcmp(before, &decoded, sizeof(before)));
            }
        }
    }
    s3_output_t bad = {0};
    bad.navigation.command = (nav_command_t)5U;
    assert(0U == s3_output_encode(&bad, g_receive_bytes));
    bad.navigation.command = NAV_COMMAND_NONE;
    bad.navigation.black_mask = 8U;
    assert(0U == s3_output_encode(&bad, g_receive_bytes));
    bad.navigation.black_mask = 0U;
    bad.navigation.barcode_character = '*';
    assert(0U == s3_output_encode(&bad, g_receive_bytes));
    puts("S3-U28 PASS: output kinds/letters; numeric limits; malformed frames");
}

static void test_rejection (void)
{
    s3_context_t context;
    init_context(&context);
    s3_input_t request = {0};
    s3_input_t decoded;
    request.kind = S3_INPUT_MODE;
    request.request_id = 123U;
    request.value.mode = (line_mode_t)255U;
    uint32_t size = s3_input_encode(&request, g_receive_bytes);
    assert(s3_input_decode(g_receive_bytes, size, &decoded));
    input(&context, &decoded, 0U);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(123U == g_outputs[g_count - 1U].request_id);
    request.kind = S3_INPUT_MAPPING;
    request.value.mapping = (nav_update_t){0};
    assert(!nav_update_parse(&request.value.mapping, "MAP A=LEFT Z=INVALID"));
    const nav_map_t original = context.map;
    size = s3_input_encode(&request, g_receive_bytes);
    assert(s3_input_decode(g_receive_bytes, size, &decoded));
    input(&context, &decoded, 0U);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(0 == memcmp(&original, &context.map, sizeof(original)));
    request.kind = S3_INPUT_CONFIG;
    request.value.config = context.config;
    request.value.config.period_ms = UINT32_MAX;
    size = s3_input_encode(&request, g_receive_bytes);
    assert(s3_input_decode(g_receive_bytes, size, &decoded));
    assert(127U == decoded.value.config.period_ms);
    context.status.mode = LINE_MODE_PAUSED;
    assert(!configure(&context, &decoded.value.config));
    s3_output_t response = {0};
    response.kind = S3_OUTPUT_REPLY;
    response.accepted = true;
    g_receive_size = (INT)s3_output_encode(&response, g_receive_bytes);
    assert(E_OK == s3_receive(&response, TMO_POL));
    assert(response.accepted);
    unsigned char before[sizeof(response)];
    memcpy(before, &response, sizeof(before));
    --g_receive_size;
    assert(E_PAR == s3_receive(&response, TMO_POL));
    assert(0 == memcmp(before, &response, sizeof(before)));
    g_receive_size = E_TMOUT;
    assert(E_TMOUT == s3_receive(&response, TMO_POL));
    puts("S3-U29 PASS: rejection replies, transactionality, receive preservation");
}

/** @brief Follow the public packed input/output path through the S3 owner. */
static s3_output_t map_reply (s3_context_t * p_context,
                              const s3_input_t * p_request)
{
    s3_input_t decoded = {0};
    s3_output_t reply = {0};
    const unsigned int before = g_count;
    assert(E_OK == s3_submit(p_request, TMO_POL));
    assert(s3_input_decode(g_submitted_input, g_submitted_size, &decoded));
    assert(p_request->request_id == decoded.request_id);
    input(p_context, &decoded, 0U);
    assert(before + 1U == g_count);
    g_receive_size = (INT)s3_output_encode(&g_outputs[before],
                                           g_receive_bytes);
    assert(E_OK == s3_receive(&reply, TMO_POL));
    assert(S3_OUTPUT_REPLY == reply.kind);
    assert(p_request->request_id == reply.request_id);
    assert(p_request->request_id != reply.event_id);
    assert(p_context->map.revision == reply.map_revision);
    return reply;
}

/** @brief Preserve Buddy 1 map correlation on acceptance and rejection. */
static void test_gateway_reply (void)
{
    static const uint32_t sequences[] = {0U, 1U, 0x123456U, 0x7fffffffU};
    for (uint32_t index = 0U;
         index < sizeof(sequences) / sizeof(sequences[0]); ++index)
    {
        s3_context_t context;
        init_context(&context);
        s3_input_t request = {0};
        request.kind = S3_INPUT_MAPPING;
        request.request_id = 0x80000000U | sequences[index];
        assert(nav_update_parse(&request.value.mapping,
            "MAP W=FORWARD X=UTURN Y=LEFT Z=RIGHT"));
        const uint32_t revision = context.map.revision;
        s3_output_t reply = map_reply(&context, &request);
        assert(reply.accepted);
        assert(revision + 1U == reply.map_revision);
        const nav_map_t installed = context.map;

        assert(nav_update_parse(&request.value.mapping, "MAP A=LEFT"));
        reply = map_reply(&context, &request);
        assert(!reply.accepted);
        assert(0 == memcmp(&installed, &context.map, sizeof(installed)));

        assert(nav_update_parse(&request.value.mapping,
            "MAP A=LEFT B=RIGHT C=FORWARD D=UTURN"));
        context.pending_letter = 'W';
        context.pending_command = NAV_COMMAND_STRAIGHT;
        reply = map_reply(&context, &request);
        assert(!reply.accepted);
        assert(0 == memcmp(&installed, &context.map, sizeof(installed)));
    }
    puts("S3-U40 PASS: full gateway IDs echoed in accepted/rejected replies");
}

/** @brief Exercise the public queue codec and owner at a supplied raw time. */
static void deliver_input (s3_context_t * p_context,
                            const s3_input_t * p_request, uint64_t now)
{
    s3_input_t decoded = {0};
    assert(E_OK == s3_submit(p_request, TMO_POL));
    assert(s3_input_decode(g_submitted_input, g_submitted_size, &decoded));
    input(p_context, &decoded, now);
}

/** @brief Conflicting duplicates cannot change acquisition identity or age. */
static void test_odometry_identity (void)
{
    s3_context_t context;
    init_context(&context);
    const uint64_t timestamp = UINT64_C(0x100000123);
    const uint64_t distance = UINT64_C(0x200000789);
    s3_input_t request = {0};
    request.kind = S3_INPUT_ODOMETRY;
    request.value.odometry = (s3_odometry_t){timestamp, distance, true};
    deliver_input(&context, &request, timestamp);
    assert(context.b_odometry_usable);
    assert(timestamp == context.odometry_us);
    assert(distance == context.distance_um);
    const unsigned int before = g_count;
    const uint64_t boundary = timestamp + context.config.odometry_stale_us;
    deliver_input(&context, &request, boundary);
    assert(context.b_odometry_usable);
    assert(timestamp == context.odometry_us);

    request.value.odometry.tracking_forward = false;
    deliver_input(&context, &request, boundary);
    assert(!context.b_odometry_usable);
    assert(context.b_odometry_forward);
    assert(distance == context.distance_um);
    assert(timestamp == context.odometry_us);
    request.value.odometry.tracking_forward = true;
    ++request.value.odometry.forward_distance_um;
    deliver_input(&context, &request, boundary);
    assert(!context.b_odometry_usable);
    assert(distance == context.distance_um);
    request.value.odometry.forward_distance_um = distance;
    deliver_input(&context, &request, boundary);
    assert(context.b_odometry_usable);
    deliver_input(&context, &request, boundary + 1U);
    assert(!context.b_odometry_usable);
    assert(timestamp == context.odometry_us);
    assert(distance == context.distance_um);

    /* A genuinely newer sample may change tracking state, with no movement. */
    request.value.odometry.timestamp_us = boundary + 1U;
    request.value.odometry.tracking_forward = false;
    deliver_input(&context, &request, boundary + 1U);
    assert(context.b_odometry_usable);
    assert(!context.b_odometry_forward);
    assert(boundary + 1U == context.odometry_us);
    assert(distance == context.distance_um);
    assert(before == g_count); /* Odometry does not generate reply events. */
    puts("S3-U41 PASS: packed odometry identity, full width and source age");
}

/** @brief Only the navigation payload releases S3's pending event. */
static void test_completion_identity (void)
{
    s3_context_t context;
    init_context(&context);
    const uint32_t navigation_id = 71U;
    const uint32_t motion_id = 9001U;
    context.status.state = LINE_STATE_AWAITING_MANOEUVRE;
    context.status.pending_manoeuvre_event_id = navigation_id;
    s3_input_t request = {0};
    request.kind = S3_INPUT_MANOEUVRE_DONE;
    request.request_id = navigation_id;
    request.value.manoeuvre_event_id = motion_id;
    deliver_input(&context, &request, 100U);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(navigation_id == context.status.pending_manoeuvre_event_id);
    assert(LINE_STATE_AWAITING_MANOEUVRE == context.status.state);

    /* Model the controller having confirmed S2 success and standstill.
     * S3 receives only its correlated event ID, never the S2 terminal result.
     * The reply token is independent of the navigation payload.
     */
    request.request_id = motion_id;
    request.value.manoeuvre_event_id = navigation_id;
    deliver_input(&context, &request, 200U);
    assert(g_outputs[g_count - 1U].accepted);
    assert(motion_id == g_outputs[g_count - 1U].request_id);
    assert(0U == context.status.pending_manoeuvre_event_id);
    assert(LINE_STATE_RECOVERY == context.status.state);
    assert(LINE_MODE_RECOVERY == context.status.mode);
    assert(200U == context.recovery_since_us);
    deliver_input(&context, &request, 300U);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(200U == context.recovery_since_us);
    puts("S3-U42 PASS: motion, navigation and reply IDs stay independent");
}

/** @brief Preserve every command/state and controller mode in packed data. */
static void test_navigation_meanings (void)
{
    uint8_t bytes[S3_OUTPUT_WIRE_MAX];
    for (uint32_t state = LINE_STATE_CALIBRATING;
         state <= LINE_STATE_FAULT_REPORTED; ++state)
    {
        for (uint32_t command = NAV_COMMAND_NONE;
             command <= NAV_COMMAND_U_TURN; ++command)
        {
            s3_output_t event = {0};
            event.kind = S3_OUTPUT_NAVIGATION;
            event.event_id = 123U;
            event.navigation.kind = NAV_EVENT_JUNCTION;
            event.navigation.state = (line_state_t)state;
            event.navigation.command = (nav_command_t)command;
            event.navigation.timestamp_us = UINT64_C(0x100000123);
            event.navigation.black_mask = 7U;
            event.navigation.barcode_character = 'Z';
            g_receive_size = (INT)s3_output_encode(&event, g_receive_bytes);
            assert(19 == g_receive_size);
            s3_output_t received = {0};
            assert(E_OK == s3_receive(&received, TMO_POL));
            assert(event.navigation.state == received.navigation.state);
            assert(event.navigation.command == received.navigation.command);
            assert(event.navigation.kind == received.navigation.kind);
            assert(event.event_id == received.event_id);
            assert(event.navigation.timestamp_us ==
                   received.navigation.timestamp_us);
            assert(event.navigation.black_mask ==
                   received.navigation.black_mask);
            assert('Z' == received.navigation.barcode_character);
        }
    }
    s3_context_t context;
    init_context(&context);
    for (uint32_t mode = LINE_MODE_PAUSED; mode <= LINE_MODE_RECOVERY; ++mode)
    {
        s3_input_t request = {0};
        request.kind = S3_INPUT_MODE;
        request.value.mode = (line_mode_t)mode;
        deliver_input(&context, &request, 100U + mode);
        assert(g_outputs[g_count - 1U].accepted);
        assert(mode == context.status.mode);
        assert(((LINE_MODE_RECOVERY == mode) ? LINE_STATE_RECOVERY :
                LINE_STATE_TRACKING) == context.status.state);
    }
    /* Reserved three-bit states cannot be published as navigation events. */
    s3_output_t event = {0};
    event.kind = S3_OUTPUT_NAVIGATION;
    event.navigation.state = (line_state_t)6U;
    assert(0U == s3_output_encode(&event, bytes));
    puts("S3-U43 PASS: packed commands/states and all controller modes");
}

int main (void)
{
    assert(E_OK == s3_navigation_init());
    assert(336U == g_mbf_allocations[0]);
    assert(384U == g_mbf_allocations[1]);
    s3_input_t submitted = {0};
    submitted.kind = S3_INPUT_MODE;
    submitted.value.mode = LINE_MODE_PAUSED;
    assert(E_OK == s3_submit(&submitted, TMO_POL));
    assert(5U == g_submitted_size);
    assert(s3_input_decode(g_submitted_input, g_submitted_size, &submitted));
    test_actions();
    test_inputs();
    test_outputs();
    test_rejection();
    test_gateway_reply();
    test_odometry_identity();
    test_completion_identity();
    test_navigation_meanings();
    printf("sizes: map=%zu status=%zu output=%zu input_adapter=%zu\n",
           sizeof(nav_map_t), sizeof(s3_status_t), sizeof(s3_output_t),
           sizeof(s3_input_t));
    puts("Host simulation only; no physical or SMP timing claim.");
    return 0;
}
