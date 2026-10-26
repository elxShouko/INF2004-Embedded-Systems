/* Requirement regressions: real S3 owner, sampled signals, no hardware. */
#define S3_PRODUCTION_MAIN retained_production_main
#include "member3_production_test.c"
#include "../../app_program/subsystem1/s1_codec.h"

static void install_pc_map(s3_context_t *context, const char *text)
{
    s1_codec_command_t command = {0};
    assert(s1_codec_parse_mapping(text, (uint32_t)strlen(text), &command));
    s3_input_t request = {0};
    request.kind = S3_INPUT_MAPPING;
    for (uint32_t slot = 0U; slot < 4U; ++slot)
    {
        const uint32_t index = (uint32_t)(command.mapping_letters[slot] - 'A');
        request.value.mapping.selected |= 1UL << index;
        assert(nav_action_set(request.value.mapping.actions, index,
            (nav_command_t)(slot + 1U)));
    }
    input(context, &request, context->status.raw.timestamp_us);
    assert(g_outputs[g_count - 1U].accepted);
}

static void test_empty_and_replacement(void)
{
    s3_context_t context;
    production_init(&context, 10U);
    nav_map_init(&context.map); /* Remove only the explicit test fixture map. */
    scan_frame(&context, 60000U, 5000U, false, false);
    assert(events(NAV_EVENT_BARCODE_VALID) == 1U);
    assert(context.pending_letter == '\0');
    assert(context.status.pending_manoeuvre_event_id == 0U);
    install_pc_map(&context, "MAP A=LEFT B=RIGHT C=FORWARD D=UTURN");
    install_pc_map(&context, "MAP W=FORWARD X=UTURN Y=LEFT Z=RIGHT");
    assert(nav_map_lookup(&context.map, 'A') == NAV_COMMAND_NONE);
    assert(nav_map_lookup(&context.map, 'Y') == NAV_COMMAND_LEFT);
    const nav_map_t before = context.map;
    s3_input_t bad = {0};
    bad.kind = S3_INPUT_MAPPING;
    assert(nav_update_parse(&bad.value.mapping, "MAP A=LEFT"));
    input(&context, &bad, context.status.raw.timestamp_us);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(memcmp(&before, &context.map, sizeof(before)) == 0);
    puts("S3-U38 PASS: no startup actions; complete maps replace old letters");
}

static void test_junction_sequence(void)
{
    const char *maps[4] = {
        "MAP A=LEFT B=RIGHT C=FORWARD D=UTURN",
        "MAP A=RIGHT B=LEFT C=FORWARD D=UTURN",
        "MAP A=FORWARD B=LEFT C=RIGHT D=UTURN",
        "MAP A=UTURN B=LEFT C=RIGHT D=FORWARD"};
    for (uint32_t action = 0U; action < 4U; ++action)
    {
        for (uint32_t reverse = 0U; reverse < 2U; ++reverse)
        {
            s3_context_t context;
            production_init(&context, 10U);
            install_pc_map(&context, maps[action]);
            for (uint32_t run = 0U; run < 2U; ++run)
            {
                g_count = 0U;
                scan_frame(&context, 60000U, 5000U, reverse != 0U, false);
                assert(context.pending_letter == 'A');
                assert(context.pending_command == (nav_command_t)(action + 1U));
                assert(context.status.pending_manoeuvre_event_id == 0U);
                assert(context.status.state == LINE_STATE_TRACKING);
                for (uint32_t step = 0U; step < 10U; ++step)
                {
                    sample_after(&context, 10000U, 3U);
                }
                assert(events(NAV_EVENT_JUNCTION) == 0U);
                s3_input_t request = {0};
                request.kind = S3_INPUT_MAPPING;
                assert(nav_update_parse(&request.value.mapping, maps[0]));
                input(&context, &request, context.status.raw.timestamp_us);
                assert(!g_outputs[g_count - 1U].accepted);
                host_crossing(&context);
                assert(events(NAV_EVENT_JUNCTION) == 1U);
                assert(context.pending_letter == '\0');
                assert(context.status.state == LINE_STATE_AWAITING_MANOEUVRE);
                const uint32_t pending = context.status.pending_manoeuvre_event_id;
                assert(pending != 0U);
                bool found = false;
                for (uint32_t index = 0U; index < g_count; ++index)
                {
                    const s3_output_t *event = &g_outputs[index];
                    if (event->event_id == pending)
                    {
                        assert(event->navigation.kind == NAV_EVENT_JUNCTION);
                        assert(event->navigation.barcode_character == 'A');
                        assert(event->navigation.command == (nav_command_t)(action + 1U));
                        found = true;
                    }
                }
                assert(found);
                request.kind = S3_INPUT_MANOEUVRE_DONE;
                request.value.manoeuvre_event_id = pending;
                input(&context, &request, context.status.raw.timestamp_us);
                assert(g_outputs[g_count - 1U].accepted);
                for (uint32_t step = 0U; step < 8U; ++step)
                {
                    sample_after(&context, 10000U, 3U);
                }
                assert(context.status.mode == LINE_MODE_PAUSED);
                request.kind = S3_INPUT_MODE;
                request.value.mode = LINE_MODE_TRACKING;
                input(&context, &request, context.status.raw.timestamp_us);
                assert(g_outputs[g_count - 1U].accepted);
            }
        }
    }
    puts("S3-U39 PASS: both directions, four actions, wait for junction, repeat");
}

int main(void)
{
    test_empty_and_replacement();
    test_junction_sequence();
    return 0;
}
