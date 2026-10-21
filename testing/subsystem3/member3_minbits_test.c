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
    printf("sizes: map=%zu status=%zu output=%zu input_adapter=%zu\n",
           sizeof(nav_map_t), sizeof(s3_status_t), sizeof(s3_output_t),
           sizeof(s3_input_t));
    puts("Host simulation only; no physical or SMP timing claim.");
    return 0;
}
