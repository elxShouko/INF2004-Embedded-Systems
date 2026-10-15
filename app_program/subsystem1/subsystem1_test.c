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
#define BACKOFF_FIRST_MS 1000u
#define BACKOFF_SECOND_MS 2000u
#define STATUS_ROUNDS 20000u
#define STATUS_VALUE_STEP 10u
#define STATUS_READ_LIMIT 4000000u
#define STATUS_WAIT_MS 20000
#define STORM_ROUNDS 5000u
#define TOGGLE_PERIOD 2u
#define SETTLE_MS 300
#define SIM_PROBE_MS 350
#define SIM_PROBE_MIN 2u
#define FLAG_WRITER_DONE 0x01u
#define FLAG_READER_DONE 0x02u
#define FLAG_STORM_DONE 0x04u
#define FLAG_READER_READY 0x08u

/* On the Pico both default to the real two-core test. The PC runner sets
 * S1_TEST_SERIAL_HOST=1 and S1_TEST_REQUIRE_OVERLAP=0 when it has no threads.
 */
#ifndef S1_TEST_SERIAL_HOST
#define S1_TEST_SERIAL_HOST 0
#endif
#ifndef S1_TEST_REQUIRE_OVERLAP
#define S1_TEST_REQUIRE_OVERLAP 1
#endif
#define READER_START_WAIT_MS 5000
#define STATUS_READS_MIN 100u

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
test_backoff_notification (void)
{
    s1_net_stats_t before = {0};
    s1_net_stats_t after  = {0};
    UINT           flags  = 0u;
    ER             result = E_OK;
    TEST_INIT();

    /* S1-U13: entering BACKOFF from a connected state is a change of the
     * connected state, so it raises LINK_CHANGED exactly once. */
    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    result = s1_net_events_wait(S1_NET_FLG_LINK_CHANGED, &flags, TMO_POL);
    TEST_CHECK("S1-U13a",
               (E_OK == result) && s1_net_is_connected() &&
                   (E_TMOUT == s1_net_events_wait(S1_NET_FLG_LINK_CHANGED,
                                                  &flags, TMO_POL)),
               "start connected; initial notification consumed");

    s1_net_get_stats(&before);
    s1_net_hook_on_event(S1_NET_EVENT_BACKOFF, BACKOFF_FIRST_MS);
    s1_net_get_stats(&after);
    result = s1_net_events_wait(S1_NET_FLG_LINK_CHANGED, &flags, TMO_POL);
    TEST_CHECK("S1-U13b",
               (!after.b_connected) && (!s1_net_is_connected()) &&
                   (E_OK == result) &&
                   (0u != (flags & S1_NET_FLG_LINK_CHANGED)),
               "BACKOFF clears connected and raises link-changed");
    TEST_CHECK("S1-U13c",
               (after.backoff_count == (before.backoff_count + 1u)) &&
                   (BACKOFF_FIRST_MS == after.backoff_last_ms) &&
                   (after.disconnects == (before.disconnects + 1u)),
               "BACKOFF is counted and the disconnect is counted once");

    s1_net_hook_on_event(S1_NET_EVENT_BACKOFF, BACKOFF_SECOND_MS);
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U13d",
               (E_TMOUT ==
                s1_net_events_wait(S1_NET_FLG_LINK_CHANGED, &flags, TMO_POL)) &&
                   (BACKOFF_SECOND_MS == after.backoff_last_ms) &&
                   (after.disconnects == (before.disconnects + 1u)),
               "repeat BACKOFF / DISCONNECTED while down: no new notification");

    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    s1_net_events_wait(S1_NET_FLG_LINK_CHANGED, &flags, TMO_POL);
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);
    result = s1_net_events_wait(S1_NET_FLG_LINK_CHANGED, &flags, TMO_POL);
    TEST_CHECK("S1-U13e", (E_OK == result) && !s1_net_is_connected(),
               "DISCONNECTED (without BACKOFF first) still notifies once");
    TEST_SUMMARY("subsystem1 backoff notification");
}

/* ---- Concurrent status access (S1-U14, S1-U15) ----------------------- */

typedef struct
{
    ID       flag_id;
    uint32_t reads;
    uint32_t violations;
    uint32_t seen_connected;
    uint32_t seen_disconnected;
    bool     b_reader_gave_up;
} status_test_t;

static status_test_t  g_status_test = {0};
static s1_net_stats_t g_status_base = {0};

/* One snapshot must be internally consistent: every CONNECTED is followed
 * by a BACKOFF that the writer reports with value = 10 x its own count. */
static bool
snapshot_is_coherent (const s1_net_stats_t * p_snap)
{
    uint32_t connects    = p_snap->connects - g_status_base.connects;
    uint32_t disconnects = p_snap->disconnects - g_status_base.disconnects;
    uint32_t backoffs    = p_snap->backoff_count - g_status_base.backoff_count;
    uint32_t expected    = (connects - disconnects);
    bool     b_ok        = (backoffs == disconnects) &&
                (expected == (p_snap->b_connected ? 1u : 0u));

    if (backoffs > 0u)
    {
        b_ok =
            b_ok && (p_snap->backoff_last_ms == (backoffs * STATUS_VALUE_STEP));
    }
    return b_ok;
}

/* Writer: stands in for the Wi-Fi service task (pinned to the other core). */
static void
status_writer_task (INT stacd, void * p_exinf)
{
    uint32_t round   = 0u;
    UINT     pattern = 0u;

    (void) stacd;   /* Cast is safe: discards an unused task parameter. */
    (void) p_exinf; /* Cast is safe: discards an unused task parameter. */

    /* Start only once the reader is running, so the two really overlap. */
    tk_wai_flg(g_status_test.flag_id, FLAG_READER_READY, TWF_ORW, &pattern,
               READER_START_WAIT_MS);
    for (round = 1u; round <= STATUS_ROUNDS; round++)
    {
        s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
        s1_net_hook_on_event(S1_NET_EVENT_BACKOFF, round * STATUS_VALUE_STEP);
    }
    tk_set_flg(g_status_test.flag_id, FLAG_WRITER_DONE);
    tk_ext_tsk();
}

/* Reader: an application task reading the status as fast as it can. */
static void
status_reader_task (INT stacd, void * p_exinf)
{
    s1_net_stats_t snap    = {0};
    UINT           pattern = 0u;
    uint32_t       count   = 0u;
    bool           b_done  = false;

    (void) stacd;   /* Cast is safe: discards an unused task parameter. */
    (void) p_exinf; /* Cast is safe: discards an unused task parameter. */

    tk_set_flg(g_status_test.flag_id, FLAG_READER_READY);
    while ((count < STATUS_READS_MIN) ||
           ((!b_done) && (count < STATUS_READ_LIMIT)))
    {
        s1_net_get_stats(&snap);
        g_status_test.reads++;
        if (!snapshot_is_coherent(&snap))
        {
            g_status_test.violations++;
        }
        if (snap.b_connected)
        {
            g_status_test.seen_connected++;
        }
        else
        {
            g_status_test.seen_disconnected++;
        }
        (void) s1_net_is_connected();
        b_done = (E_OK == tk_wai_flg(g_status_test.flag_id, FLAG_WRITER_DONE,
                                     TWF_ORW, &pattern, TMO_POL));
        count++;
    }
    g_status_test.b_reader_gave_up = !b_done;
    tk_set_flg(g_status_test.flag_id, FLAG_READER_DONE);
    tk_ext_tsk();
}

static void
test_concurrent_status (void)
{
    T_CFLG         flag_info = {0};
    s1_net_stats_t final     = {0};
    UINT           pattern   = 0u;
    ER             result    = E_OK;
    TEST_INIT();

    /* S1-U14: a writer on one core and a reader on the other, thousands of
     * times. Every snapshot must be coherent (no half-updated block). */
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);
    s1_net_get_stats(&g_status_base);
    flag_info.flgatr      = TA_TFIFO | TA_WMUL;
    g_status_test         = (status_test_t){0};
    g_status_test.flag_id = tk_cre_flg(&flag_info);

#if S1_TEST_SERIAL_HOST
    /* No threads on this PC: the writer must finish before the reader runs. */
    app_start_task(status_writer_task, S1_PRI_SIM, TP_PRC2, "s1_t_writer");
    app_start_task(status_reader_task, S1_PRI_SIM, TP_PRC1, "s1_t_reader");
#else
    app_start_task(status_reader_task, S1_PRI_SIM, TP_PRC1, "s1_t_reader");
    app_start_task(status_writer_task, S1_PRI_SIM, TP_PRC2, "s1_t_writer");
#endif
    result =
        tk_wai_flg(g_status_test.flag_id, FLAG_WRITER_DONE | FLAG_READER_DONE,
                   TWF_ANDW, &pattern, STATUS_WAIT_MS);
    s1_net_get_stats(&final);

    TEST_CHECK("S1-U14a", E_OK == result,
               "writer and reader tasks finished (no deadlock)");
    TEST_CHECK("S1-U14b",
               (0u == g_status_test.violations) &&
                   (g_status_test.reads >= STATUS_READS_MIN) &&
                   !g_status_test.b_reader_gave_up,
               "every concurrent snapshot was coherent");
    /* Proof that the two tasks really overlapped: the reader saw the state
     * change while the writer was running. (The PC runner without pthreads
     * runs the tasks one after the other and switches this check off.) */
    TEST_CHECK("S1-U14c",
               (0u == S1_TEST_REQUIRE_OVERLAP) ||
                   ((g_status_test.seen_connected > 0u) &&
                    (g_status_test.seen_disconnected > 0u)),
               "reader saw both states while the writer ran");
    TEST_CHECK("S1-U14d",
               ((final.connects - g_status_base.connects) == STATUS_ROUNDS) &&
                   ((final.disconnects - g_status_base.disconnects) ==
                    STATUS_ROUNDS) &&
                   !final.b_connected,
               "no update was lost under contention");
    tm_printf(
        (UB *) "s1: S1-U14 reads=%d violations=%d seen_up=%d seen_down=%d\n",
        (int) g_status_test.reads, (int) g_status_test.violations,
        (int) g_status_test.seen_connected,
        (int) g_status_test.seen_disconnected);
    TEST_SUMMARY("subsystem1 concurrent status");
}

static void
sim_toggle_task (INT stacd, void * p_exinf)
{
    uint32_t round = 0u;

    (void) stacd;   /* Cast is safe: discards an unused task parameter. */
    (void) p_exinf; /* Cast is safe: discards an unused task parameter. */

    for (round = 0u; round < STORM_ROUNDS; round++)
    {
        s1_sim_set_enabled(0u == (round % TOGGLE_PERIOD));
    }
    s1_sim_set_enabled(false);
    tk_set_flg(g_status_test.flag_id, FLAG_STORM_DONE);
    tk_ext_tsk();
}

static uint32_t
count_frames (void)
{
    char     text[TEXT_CAPACITY] = {0};
    uint32_t frames              = 0u;

    while (s1_net_hook_pull_telemetry(text, sizeof(text)) > 0u)
    {
        frames++;
    }
    return frames;
}

static void
test_sim_switch (void)
{
    T_CFLG   flag_info  = {0};
    UINT     pattern    = 0u;
    ER       result     = E_OK;
    uint32_t frames_off = 0u;
    uint32_t frames_on  = 0u;
    TEST_INIT();

    if (0u != S1_SIMULATE_TELEMETRY)
    {
        /* S1-U15: the enable switch set on one core is seen by the
         * simulator task on the other, also after a storm of changes. */
        flag_info.flgatr      = TA_TFIFO | TA_WMUL;
        g_status_test.flag_id = tk_cre_flg(&flag_info);
        app_start_task(sim_toggle_task, S1_PRI_SIM, TP_PRC2, "s1_t_toggle");
        result = tk_wai_flg(g_status_test.flag_id, FLAG_STORM_DONE, TWF_ORW,
                            &pattern, STATUS_WAIT_MS);
        TEST_CHECK("S1-U15a", E_OK == result,
                   "5000 enable/disable changes finished (no deadlock)");

        tk_dly_tsk(SETTLE_MS);
        (void) count_frames();
        tk_dly_tsk(SIM_PROBE_MS);
        frames_off = count_frames();
        TEST_CHECK("S1-U15b", 0u == frames_off,
                   "simulator stays silent after the switch was turned off");

        s1_sim_set_enabled(true);
        tk_dly_tsk(SIM_PROBE_MS);
        frames_on = count_frames();
        TEST_CHECK("S1-U15c", frames_on >= SIM_PROBE_MIN,
                   "simulator resumes after the switch was turned on");
        s1_sim_set_enabled(true);
    }
    TEST_SUMMARY("subsystem1 simulator switch");
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
    test_backoff_notification();
    test_concurrent_status();
    test_simulator();
    test_sim_switch();
}

#endif /* APP_TEST */

/*** end of file ***/
