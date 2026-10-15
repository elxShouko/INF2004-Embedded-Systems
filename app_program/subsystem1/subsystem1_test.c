/**
 * @file subsystem1_test.c
 * @brief On-target tests for subsystem 1. Built only with: make TEST=1
 *
 * Each TEST_CHECK prints a PASS or FAIL line on the console. Save the console
 * output to testing/subsystem1/logs/ and record the result in
 * testing/subsystem1/TEST_LOG.md. Test IDs: S1-U01, S1-U02, ...
 *
 * NOTE: Run these in the default build (no NET, no MQTT). With NET=lwip
 * MQTT=1 the Wi-Fi task also takes frames from the buffer, which would make
 * the exact-count checks unreliable.
 */

#include "../test_report.h"
#include "subsystem1.h"

#if APP_TEST

#include "../../lib/liblwip/sysdepend/pico_rp2040/net_backoff.h"
#include "s1_sim.h"

#define TEXT_CAPACITY S1_CODEC_TELEMETRY_JSON_CAPACITY
#define WORST_CASE_MAX 255u
#define COMMAND_DEPTH_PROBE 20u
#define FRAME_COUNT 3u
#define MARKER 0xA5A50000u
#define FORWARD_LIMIT_MS 50u
#define SIM_WINDOW_MS 1050
#define SIM_FRAMES_MIN 9u
#define SIM_FRAMES_MAX 11u
#define KEY_COUNT 9u

static bool
contains (const char * p_text, const char * p_part)
{
    const char * p_start = p_text;
    bool         b_found = false;
    uint32_t     offset  = 0u;

    while (('\0' != *p_start) && (!b_found))
    {
        offset = 0u;
        while (('\0' != p_part[offset]) && (p_part[offset] == p_start[offset]))
        {
            offset++;
        }
        b_found = ('\0' == p_part[offset]);
        p_start++;
    }
    return b_found;
}

static void
drain_telemetry (void)
{
    char text[TEXT_CAPACITY] = {0};

    while (s1_net_hook_pull_telemetry(text, sizeof(text)) > 0u)
    {
        /* Discard every frame that is waiting. */
    }
}

static void
drain_commands (void)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};

    while (E_OK == s1_net_command_receive(&command, TMO_POL))
    {
        /* Discard every command that is waiting. */
    }
}

static bool
keys_present (const char * p_json)
{
    static const char * const p_keys[KEY_COUNT] = {
        "\"timestamp\":",          "\"robot_state\":",
        "\"left_motor_speed\":",   "\"right_motor_speed\":",
        "\"left_encoder_count\":", "\"right_encoder_count\":",
        "\"line_status\":",        "\"hump_peak\":",
        "\"obstacle_distance\":"};
    bool     b_all = true;
    uint32_t key   = 0u;

    for (key = 0u; key < KEY_COUNT; key++)
    {
        b_all = b_all && contains(p_json, p_keys[key]);
    }
    return b_all;
}

static void
test_codec (void)
{
    char             text[TEXT_CAPACITY] = {0};
    s1_codec_frame_t frame  = {1234u, 5u, 6u, 12, 455, 120, -120, 2u, 2u};
    s1_codec_frame_t worst  = {0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
                               -2000000000, -2000000000, -32768,
                               -32768,      255u,        255u};
    uint32_t         length = 0u;
    TEST_INIT();

    /* S1-U01: FR17 -- every required field is in the JSON, with its name. */
    length = s1_codec_encode_telemetry(&frame, 1u, text, sizeof(text));
    TEST_CHECK("S1-U01", (length > 0u) && keys_present(text),
               "telemetry JSON has all FR17 fields");

    /* S1-U02: NFR10 -- the longest possible frame stays under 256 bytes. */
    length = s1_codec_encode_telemetry(&worst, 0xFFFFFFFFu, text, sizeof(text));
    TEST_CHECK("S1-U02", (length > 0u) && (length <= WORST_CASE_MAX),
               "worst-case JSON is under 256 bytes");

    /* S1-U05: FR20 -- heartbeat format. */
    length = s1_codec_encode_heartbeat(74829u, text, sizeof(text));
    TEST_CHECK("S1-U05",
               (length > 0u) &&
                   contains(text, "{\"status\":\"ALIVE\",\"uptime_ms\":74829}"),
               "heartbeat is {status ALIVE, uptime_ms}");
    TEST_SUMMARY("subsystem1 codec");
}

static void
test_commands_and_backoff (void)
{
    uint32_t delay_ms = NET_BACKOFF_INITIAL_MS;
    bool     b_ok     = true;
    uint32_t step     = 0u;
    TEST_INIT();

    /* S1-U03: FR18 -- the command words are recognised, junk is not. */
    TEST_CHECK(
        "S1-U03",
        (S1_CMD_EMERGENCY_STOP ==
         s1_codec_parse_command("EMERGENCY_STOP", 14u)) &&
            (S1_CMD_START_RUN == s1_codec_parse_command(" start_run\n", 11u)) &&
            (S1_CMD_PAUSE == s1_codec_parse_command("PAUSE", 5u)) &&
            (S1_CMD_RESUME == s1_codec_parse_command("RESUME", 6u)) &&
            (S1_CMD_NONE == s1_codec_parse_command("SELF_DESTRUCT", 13u)),
        "command parser accepts the four commands, rejects junk");

    /* S1-U04: FR19 -- the delays are 1, 2, 4, 8, 8 seconds. */
    for (step = 0u; step < 4u; step++)
    {
        delay_ms = net_backoff_next_ms(delay_ms);
        b_ok =
            b_ok && (delay_ms == (((step < 3u) ? (2u << step) : 8u) * 1000u));
    }
    TEST_CHECK("S1-U04", b_ok && (NET_BACKOFF_MAX_MS == delay_ms),
               "backoff 1,2,4,8 then capped at 8 s");
    TEST_SUMMARY("subsystem1 commands and backoff");
}

static void
test_telemetry_buffer (void)
{
    char             text[TEXT_CAPACITY] = {0};
    s1_codec_frame_t frame               = {0};
    uint32_t         index_sent          = 0u;
    uint32_t         index_got           = 0u;
    uint32_t         sent_ok             = 0u;
    bool             b_in_order          = true;
    ER               result              = E_OK;
    TEST_INIT();

    s1_sim_set_enabled(false);
    drain_telemetry();

    /* S1-U06: frames come out in the order they went in. */
    for (index_sent = 0u; index_sent < FRAME_COUNT; index_sent++)
    {
        frame.timestamp_ms = MARKER + index_sent;
        s1_net_telemetry_send(&frame);
    }
    for (index_got = 0u; index_got < FRAME_COUNT; index_got++)
    {
        b_in_order =
            b_in_order &&
            (s1_net_hook_pull_telemetry(text, sizeof(text)) > 0u) &&
            contains(text, (0u == index_got)   ? "\"timestamp\":2779054080,"
                           : (1u == index_got) ? "\"timestamp\":2779054081,"
                                               : "\"timestamp\":2779054082,");
    }
    TEST_CHECK("S1-U06", b_in_order, "telemetry frames keep their order");

    /* S1-U07: a full buffer refuses with E_TMOUT and never blocks. */
    for (index_sent = 0u; index_sent < (COMMAND_DEPTH_PROBE * 2u); index_sent++)
    {
        result = s1_net_telemetry_send(&frame);
        if (E_OK == result)
        {
            sent_ok++;
        }
    }
    TEST_CHECK("S1-U07",
               (E_TMOUT == result) && (sent_ok >= 1u) &&
                   (sent_ok < (COMMAND_DEPTH_PROBE * 2u)),
               "full telemetry buffer returns E_TMOUT, does not block");
    drain_telemetry();
    TEST_CHECK("S1-U07b", E_OK == s1_net_telemetry_send(&frame),
               "buffer accepts frames again after it is drained");
    drain_telemetry();
    TEST_SUMMARY("subsystem1 telemetry buffer");
}

static void
test_command_path (void)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};
    s1_net_stats_t     before  = {0};
    s1_net_stats_t     after   = {0};
    UINT               flags   = 0u;
    ER                 result  = E_OK;
    uint32_t           index   = 0u;
    SYSTIM             time    = {0};
    TEST_INIT();

    drain_commands();
    s1_net_events_wait(S1_NET_FLG_COMMAND | S1_NET_FLG_ESTOP, &flags, TMO_POL);

    /* S1-U08: FR18, NFR7 -- an E-STOP is queued, flagged, and fast. */
    tk_get_otm(&time);
    s1_net_hook_on_command("EMERGENCY_STOP", 14u, time.lo);
    result = s1_net_events_wait(S1_NET_FLG_ESTOP, &flags, TMO_POL);
    TEST_CHECK("S1-U08a",
               (E_OK == result) && (0u != (flags & S1_NET_FLG_ESTOP)),
               "EMERGENCY_STOP sets the E-STOP event flag");
    result = s1_net_command_receive(&command, TMO_POL);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U08b",
               (E_OK == result) &&
                   (S1_CMD_EMERGENCY_STOP == command.command_id) &&
                   (after.command_forward_max_ms < FORWARD_LIMIT_MS),
               "EMERGENCY_STOP delivered, forwarded in under 50 ms");

    /* S1-U09: unknown text is counted and not queued. */
    s1_net_get_stats(&before);
    s1_net_hook_on_command("SELF_DESTRUCT", 13u, time.lo);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U09",
               (E_TMOUT == s1_net_command_receive(&command, TMO_POL)) &&
                   (after.commands_rejected == (before.commands_rejected + 1u)),
               "unknown command is rejected and not queued");

    /* S1-U10: when the buffer is full, E-STOP still gets through. */
    drain_commands();
    for (index = 0u; index < COMMAND_DEPTH_PROBE; index++)
    {
        s1_net_hook_on_command("PAUSE", 5u, time.lo);
    }
    s1_net_get_stats(&before);
    s1_net_hook_on_command("EMERGENCY_STOP", 14u, time.lo);
    s1_net_get_stats(&after);
    index = 0u;
    while (E_OK == s1_net_command_receive(&command, TMO_POL))
    {
        index++;
    }
    TEST_CHECK("S1-U10",
               (S1_CMD_EMERGENCY_STOP == command.command_id) && (index >= 1u) &&
                   (after.commands_dropped > before.commands_dropped),
               "EMERGENCY_STOP evicts the oldest command when full");
    s1_net_events_wait(S1_NET_FLG_COMMAND | S1_NET_FLG_ESTOP, &flags, TMO_POL);
    TEST_SUMMARY("subsystem1 command path");
}

static void
test_link_events (void)
{
    s1_codec_frame_t frame               = {0};
    char             text[TEXT_CAPACITY] = {0};
    UINT             flags               = 0u;
    TEST_INIT();

    /* S1-U11: a new connection drops stale frames and tells the tasks. */
    s1_net_telemetry_send(&frame);
    s1_net_telemetry_send(&frame);
    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    TEST_CHECK("S1-U11a",
               s1_net_is_connected() &&
                   (0u == s1_net_hook_pull_telemetry(text, sizeof(text))),
               "CONNECTED flushes stale telemetry");
    s1_net_events_wait(S1_NET_FLG_LINK_CHANGED, &flags, TMO_POL);
    TEST_CHECK("S1-U11b", 0u != (flags & S1_NET_FLG_LINK_CHANGED),
               "CONNECTED raises the link-changed flag");
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);
    TEST_CHECK("S1-U11c", !s1_net_is_connected(),
               "DISCONNECTED clears the connected state");
    s1_net_events_wait(S1_NET_FLG_LINK_CHANGED, &flags, TMO_POL);
    TEST_SUMMARY("subsystem1 link events");
}

static void
test_simulator (void)
{
    char     text[TEXT_CAPACITY] = {0};
    uint32_t frames              = 0u;
    TEST_INIT();

    if (0u != S1_SIMULATE_TELEMETRY)
    {
        /* S1-U12: FR16 -- the producer runs at 10 Hz (100 ms). */
        drain_telemetry();
        s1_sim_set_enabled(true);
        tk_dly_tsk(SIM_WINDOW_MS);
        s1_sim_set_enabled(false);
        while (s1_net_hook_pull_telemetry(text, sizeof(text)) > 0u)
        {
            frames++;
        }
        TEST_CHECK("S1-U12",
                   (frames >= SIM_FRAMES_MIN) && (frames <= SIM_FRAMES_MAX),
                   "simulator produces about 10 frames per second");
        drain_telemetry();
        s1_sim_set_enabled(true);
    }
    TEST_SUMMARY("subsystem1 simulator");
}

/* Entry point called by usermain() in a TEST=1 build. */
void
subsystem1_test (void)
{
    test_codec();
    test_commands_and_backoff();
    test_telemetry_buffer();
    test_command_path();
    test_link_events();
    test_simulator();
}

#endif /* APP_TEST */

/*** end of file ***/
