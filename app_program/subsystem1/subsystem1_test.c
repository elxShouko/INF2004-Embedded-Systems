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
#define STATUS_PAUSE_EVERY 64u
#define FLAG_HOLDING 0x10u
#define FLAG_RELEASE 0x20u
#define FLAG_HOLDER_DONE 0x40u
#define FLAG_CALLER_DONE 0x80u
#define HOLD_PROBE_MS 200
#define HOLD_PROBE_STEP_MS 5
#define PAUSE_FILL 20u
#define SHORT_MESSAGE_BYTES 4
#define BAD_COMMAND_ID 99
#define STALE_AGE_MS 2000u
#define VERY_STALE_AGE_MS 5000u
#define FAIL_ONCE 1
#define FAIL_ALWAYS 100

/* On the Pico both default to the real two-core test. The PC runner sets
 * S1_TEST_SERIAL_HOST=1 and S1_TEST_REQUIRE_OVERLAP=0 when it has no threads.
 */
#ifndef S1_TEST_HOST_FAULTS
#define S1_TEST_HOST_FAULTS 0
#endif
#if S1_TEST_HOST_FAULTS
extern void kernel_emulation_fail_set_flg(int count);
#endif
#ifndef S1_TEST_SERIAL_HOST
#define S1_TEST_SERIAL_HOST 0
#endif
#ifndef S1_TEST_REQUIRE_OVERLAP
#define S1_TEST_REQUIRE_OVERLAP 1
#endif
#define READER_START_WAIT_MS 5000
#define STATUS_READS_MIN 100u

static bool
contains(const char *p_text, const char *p_part)
{
    const char *p_start = p_text;
    bool b_found = false;
    uint32_t offset = 0u;

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

#if S1_WIRE_JSON
/* Value of the "timestamp" field in one telemetry JSON text, or 0. */
static uint32_t
timestamp_of(const char *p_text)
{
    static const char key[] = "\"timestamp\":";
    uint32_t start = 0u;
    uint32_t match = 0u;
    uint32_t value = 0u;

    for (start = 0u; '\0' != p_text[start]; start++)
    {
        match = 0u;
        while (('\0' != key[match]) && (p_text[start + match] == key[match]))
        {
            match++;
        }
        if ('\0' == key[match])
        {
            while (('0' <= p_text[start + match]) &&
                   ('9' >= p_text[start + match]))
            {
                value = (value * 10u) + (uint32_t)(p_text[start + match] - '0');
                match++;
            }
            break;
        }
    }
    return value;
}
#endif

/* Telemetry timestamp of one pulled message, in either wire format. */
static uint32_t
pulled_timestamp(const char *p_message, uint32_t length)
{
    uint32_t value = 0u;
#if S1_WIRE_JSON
    (void)length; /* Cast is safe: discards an unused parameter. */
    value = timestamp_of(p_message);
#else
    s1_codec_bin_telemetry_t decoded = {0};

    if (s1_codec_decode_telemetry_bin((const uint8_t *)p_message, length,
                                      &decoded))
    {
        value = decoded.timestamp_ms;
    }
#endif
    return value;
}

/* Deliver one command to the application hook, in either wire format. */
static void
hook_command(s1_codec_command_id_t command_id, uint32_t rx_ms)
{
#if S1_WIRE_JSON
    const char *p_name = s1_codec_command_name(command_id);
    uint32_t length = 0u;

    while ('\0' != p_name[length])
    {
        length++;
    }
    s1_net_hook_on_command(p_name, length, rx_ms);
#else
    uint8_t byte = (uint8_t)command_id;

    s1_net_hook_on_command((const char *)&byte, 1u, rx_ms);
#endif
}

/* A command the application must reject, in either wire format. */
static void
hook_bad_command(uint32_t rx_ms)
{
#if S1_WIRE_JSON
    s1_net_hook_on_command("SELF_DESTRUCT", 13u, rx_ms);
#else
    static const uint8_t bad = 0x07u; /* Id 7 is not a command. */

    s1_net_hook_on_command((const char *)&bad, 1u, rx_ms);
#endif
}

static void
drain_telemetry(void)
{
    char text[TEXT_CAPACITY] = {0};

    while (s1_net_hook_pull_telemetry(text, sizeof(text)) > 0u)
    {
        /* Discard every frame that is waiting. */
    }
}

static void
drain_commands(void)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};

    while (E_OK == s1_net_command_receive(&command, TMO_POL))
    {
        /* Discard every command that is waiting. */
    }
}

static bool
keys_present(const char *p_json)
{
    static const char *const p_keys[KEY_COUNT] = {
        "\"timestamp\":", "\"robot_state\":",
        "\"left_motor_speed\":", "\"right_motor_speed\":",
        "\"left_encoder_count\":", "\"right_encoder_count\":",
        "\"line_status\":", "\"hump_peak\":",
        "\"obstacle_distance\":"};
    bool b_all = true;
    uint32_t key = 0u;

    for (key = 0u; key < KEY_COUNT; key++)
    {
        b_all = b_all && contains(p_json, p_keys[key]);
    }
    return b_all;
}

static void
test_codec(void)
{
    char text[TEXT_CAPACITY] = {0};
    s1_codec_frame_t frame = {1234u, 5u, 6u, 12, 455, 120, -120, 2u, 2u, 0u, 0u, {0u, 0u, 0u}, 0u, 0u};
    s1_codec_frame_t worst = {0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, -2000000000, -2000000000, -32768, -32768, 255u, 255u, 0u, 0u, {0u, 0u, 0u}, 0u, 0u};
    uint32_t length = 0u;
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
test_commands_and_backoff(void)
{
    uint32_t delay_ms = NET_BACKOFF_INITIAL_MS;
    bool b_ok = true;
    uint32_t step = 0u;
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
test_telemetry_buffer(void)
{
    char text[TEXT_CAPACITY] = {0};
    s1_codec_frame_t frame = {0};
    uint32_t index_sent = 0u;
    uint32_t index_got = 0u;
    uint32_t length = 0u;
    uint32_t sent_ok = 0u;
    uint32_t first_timestamp = 0u;
    uint32_t last_timestamp = 0u;
    s1_net_stats_t before = {0};
    s1_net_stats_t after = {0};
    bool b_in_order = true;
    ER result = E_OK;
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
        length = s1_net_hook_pull_telemetry(text, sizeof(text));
        b_in_order = b_in_order && (length > 0u) &&
                     (pulled_timestamp(text, length) == (MARKER + index_got));
    }
    TEST_CHECK("S1-U06", b_in_order, "telemetry frames keep their order");

    /* S1-U07: a full buffer drops the OLDEST frame, never blocks and never
     * refuses the newest. The loss is counted. */
    index_got = 0u;
    sent_ok = 0u;
    s1_net_get_stats(&before);
    for (index_sent = 0u; index_sent < (COMMAND_DEPTH_PROBE * 2u); index_sent++)
    {
        frame.timestamp_ms = MARKER + 100u + index_sent;
        result = s1_net_telemetry_send(&frame);
        if (E_OK == result)
        {
            sent_ok++;
        }
    }
    while ((length = s1_net_hook_pull_telemetry(text, sizeof(text))) > 0u)
    {
        last_timestamp = pulled_timestamp(text, length);
        if (0u == index_got)
        {
            first_timestamp = last_timestamp;
        }
        index_got++;
    }
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U07a", sent_ok == (COMMAND_DEPTH_PROBE * 2u),
               "every send into a full buffer succeeds (drop-oldest)");
    TEST_CHECK("S1-U07",
               (index_got >= 1u) && (index_got < (COMMAND_DEPTH_PROBE * 2u)) &&
                   (first_timestamp > (MARKER + 100u)) &&
                   (last_timestamp ==
                    (MARKER + 100u + (COMMAND_DEPTH_PROBE * 2u) - 1u)),
               "oldest frames are dropped, the newest frame is kept");
    TEST_CHECK("S1-U07c",
               (after.telemetry_overwritten - before.telemetry_overwritten) ==
                   ((COMMAND_DEPTH_PROBE * 2u) - index_got),
               "each dropped frame is counted in telemetry_overwritten");
    drain_telemetry();
    TEST_CHECK("S1-U07b", E_OK == s1_net_telemetry_send(&frame),
               "buffer accepts frames again after it is drained");
    drain_telemetry();
    TEST_SUMMARY("subsystem1 telemetry buffer");
}

static void
test_command_path(void)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};
    s1_net_stats_t before = {0};
    s1_net_stats_t after = {0};
    UINT flags = 0u;
    ER result = E_OK;
    uint32_t index = 0u;
    s1_codec_command_id_t last_id = S1_CMD_NONE;
    SYSTIM time = {0};
    TEST_INIT();

    drain_commands();
    s1_net_events_wait(S1_NET_FLG_COMMAND | S1_NET_FLG_ESTOP, &flags, TMO_POL);

    /* S1-U08: FR18, NFR7 -- an E-STOP is queued, flagged, and fast. */
    tk_get_otm(&time);
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
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
    hook_bad_command(time.lo);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U09",
               (E_TMOUT == s1_net_command_receive(&command, TMO_POL)) &&
                   (after.commands_rejected == (before.commands_rejected + 1u)),
               "unknown command is rejected and not queued");

    /* S1-U10: when the buffer is full, E-STOP still gets through. */
    drain_commands();
    for (index = 0u; index < COMMAND_DEPTH_PROBE; index++)
    {
        hook_command(S1_CMD_PAUSE, time.lo);
    }
    s1_net_get_stats(&before);
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
    s1_net_get_stats(&after);
    index = 0u;
    while (E_OK == s1_net_command_receive(&command, TMO_POL))
    {
        index++;
        last_id = command.command_id;
    }
    TEST_CHECK("S1-U10",
               (S1_CMD_EMERGENCY_STOP == last_id) && (index >= 1u) &&
                   (after.commands_dropped > before.commands_dropped),
               "EMERGENCY_STOP evicts the oldest command when full");
    s1_net_events_wait(S1_NET_FLG_COMMAND | S1_NET_FLG_ESTOP, &flags, TMO_POL);
    TEST_SUMMARY("subsystem1 command path");
}

static void
test_link_events(void)
{
    s1_codec_frame_t frame = {0};
    char text[TEXT_CAPACITY] = {0};
    UINT flags = 0u;
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
test_backoff_notification(void)
{
    s1_net_stats_t before = {0};
    s1_net_stats_t after = {0};
    UINT flags = 0u;
    ER result = E_OK;
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
    ID flag_id;
    uint32_t reads;
    uint32_t violations;
    uint32_t busy;
    uint32_t seen_connected;
    uint32_t seen_disconnected;
    bool b_reader_gave_up;
} status_test_t;

static status_test_t g_status_test = {0};
static s1_net_stats_t g_status_base = {0};

/* One snapshot must be internally consistent: every CONNECTED is followed
 * by a BACKOFF that the writer reports with value = 10 x its own count. */
static bool
snapshot_is_coherent(const s1_net_stats_t *p_snap)
{
    uint32_t connects = p_snap->connects - g_status_base.connects;
    uint32_t disconnects = p_snap->disconnects - g_status_base.disconnects;
    uint32_t backoffs = p_snap->backoff_count - g_status_base.backoff_count;
    uint32_t expected = (connects - disconnects);
    bool b_ok = (backoffs == disconnects) &&
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
status_writer_task(INT stacd, void *p_exinf)
{
    uint32_t round = 0u;
    UINT pattern = 0u;

    (void)stacd;   /* Cast is safe: discards an unused task parameter. */
    (void)p_exinf; /* Cast is safe: discards an unused task parameter. */

    /* Start only once the reader is running, so the two really overlap. */
    tk_wai_flg(g_status_test.flag_id, FLAG_READER_READY, TWF_ORW, &pattern,
               READER_START_WAIT_MS);
    for (round = 1u; round <= STATUS_ROUNDS; round++)
    {
        s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
        if (0u == (round % STATUS_PAUSE_EVERY))
        {
            /* Stay connected for a moment so the reader can observe it. */
            tk_wai_flg(g_status_test.flag_id, FLAG_WRITER_DONE, TWF_ORW,
                       &pattern, 1);
        }
        s1_net_hook_on_event(S1_NET_EVENT_BACKOFF, round * STATUS_VALUE_STEP);
    }
    tk_set_flg(g_status_test.flag_id, FLAG_WRITER_DONE);
    tk_ext_tsk();
}

/* Reader: an application task reading the status as fast as it can. */
static void
status_reader_task(INT stacd, void *p_exinf)
{
    s1_net_stats_t snap = {0};
    UINT pattern = 0u;
    uint32_t count = 0u;
    bool b_done = false;

    (void)stacd;   /* Cast is safe: discards an unused task parameter. */
    (void)p_exinf; /* Cast is safe: discards an unused task parameter. */

    tk_set_flg(g_status_test.flag_id, FLAG_READER_READY);
    while ((g_status_test.reads < STATUS_READS_MIN) ||
           ((!b_done) && (count < STATUS_READ_LIMIT)))
    {
        if (E_OK == s1_net_get_stats(&snap))
        {
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
        }
        else
        {
            g_status_test.busy++; /* Bounded wait expired: allowed, counted. */
        }
        (void)s1_net_is_connected();
        b_done = (E_OK == tk_wai_flg(g_status_test.flag_id, FLAG_WRITER_DONE,
                                     TWF_ORW, &pattern,
                                     (0u == (count % STATUS_PAUSE_EVERY)) ? 1 : TMO_POL));
        count++;
    }
    g_status_test.b_reader_gave_up = !b_done;
    tk_set_flg(g_status_test.flag_id, FLAG_READER_DONE);
    tk_ext_tsk();
}

static void
test_concurrent_status(void)
{
    T_CFLG flag_info = {0};
    s1_net_stats_t final = {0};
    UINT pattern = 0u;
    ER result = E_OK;
    TEST_INIT();

    /* S1-U14: a writer on one core and a reader on the other, thousands of
     * times. Every snapshot must be coherent (no half-updated block). */
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);
    s1_net_get_stats(&g_status_base);
    flag_info.flgatr = TA_TFIFO | TA_WMUL;
    g_status_test = (status_test_t){0};
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
        (UB *)"s1: S1-U14 reads=%d violations=%d seen_up=%d seen_down=%d\n",
        (int)g_status_test.reads, (int)g_status_test.violations,
        (int)g_status_test.seen_connected,
        (int)g_status_test.seen_disconnected);
    TEST_SUMMARY("subsystem1 concurrent status");
}

static void
sim_toggle_task(INT stacd, void *p_exinf)
{
    uint32_t round = 0u;

    (void)stacd;   /* Cast is safe: discards an unused task parameter. */
    (void)p_exinf; /* Cast is safe: discards an unused task parameter. */

    for (round = 0u; round < STORM_ROUNDS; round++)
    {
        s1_sim_set_enabled(0u == (round % TOGGLE_PERIOD));
    }
    s1_sim_set_enabled(false);
    tk_set_flg(g_status_test.flag_id, FLAG_STORM_DONE);
    tk_ext_tsk();
}

static uint32_t
count_frames(void)
{
    char text[TEXT_CAPACITY] = {0};
    uint32_t frames = 0u;

    while (s1_net_hook_pull_telemetry(text, sizeof(text)) > 0u)
    {
        frames++;
    }
    return frames;
}

static void
test_sim_switch(void)
{
    T_CFLG flag_info = {0};
    UINT pattern = 0u;
    ER result = E_OK;
    uint32_t frames_off = 0u;
    uint32_t frames_on = 0u;
    TEST_INIT();

    if (0u != S1_SIMULATE_TELEMETRY)
    {
        /* S1-U15: the enable switch set on one core is seen by the
         * simulator task on the other, also after a storm of changes. */
        flag_info.flgatr = TA_TFIFO | TA_WMUL;
        g_status_test.flag_id = tk_cre_flg(&flag_info);
        app_start_task(sim_toggle_task, S1_PRI_SIM, TP_PRC2, "s1_t_toggle");
        result = tk_wai_flg(g_status_test.flag_id, FLAG_STORM_DONE, TWF_ORW,
                            &pattern, STATUS_WAIT_MS);
        TEST_CHECK("S1-U15a", E_OK == result,
                   "5000 enable/disable changes finished (no deadlock)");

        tk_dly_tsk(SETTLE_MS);
        (void)count_frames();
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

/* ---- Emergency path (S1-U16 .. S1-U20) ------------------------------- */

static void
estop_reset(void)
{
    UINT flags = 0u;

    drain_commands();
    s1_net_estop_clear();
    s1_net_events_wait(S1_NET_FLG_COMMAND | S1_NET_FLG_ESTOP, &flags, TMO_POL);
}

#if !S1_TEST_SERIAL_HOST
static ID g_ctl_flag = 0;

static void
ctl_flag_create(void)
{
    T_CFLG flag_info = {0};

    if (0 == g_ctl_flag)
    {
        flag_info.flgatr = TA_TFIFO | TA_WMUL;
        g_ctl_flag = tk_cre_flg(&flag_info);
    }
    tk_clr_flg(g_ctl_flag, 0u);
}

/* A diagnostic task that holds the status mutex until told to let go. */
static void
mutex_holder_task(INT stacd, void *p_exinf)
{
    UINT pattern = 0u;

    (void)stacd;   /* Cast is safe: discards an unused task parameter. */
    (void)p_exinf; /* Cast is safe: discards an unused task parameter. */

    tk_loc_mtx(s1_net_test_status_mutex(), TMO_FEVR);
    tk_set_flg(g_ctl_flag, FLAG_HOLDING);
    tk_wai_flg(g_ctl_flag, FLAG_RELEASE, TWF_ORW, &pattern, STATUS_WAIT_MS);
    tk_unl_mtx(s1_net_test_status_mutex());
    tk_set_flg(g_ctl_flag, FLAG_HOLDER_DONE);
    tk_ext_tsk();
}

/* Stands in for the Wi-Fi task delivering an EMERGENCY_STOP. */
static void
estop_caller_task(INT stacd, void *p_exinf)
{
    SYSTIM time = {0};

    (void)stacd;   /* Cast is safe: discards an unused task parameter. */
    (void)p_exinf; /* Cast is safe: discards an unused task parameter. */

    tk_get_otm(&time);
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
    tk_set_flg(g_ctl_flag, FLAG_CALLER_DONE);
    tk_ext_tsk();
}

#endif /* !S1_TEST_SERIAL_HOST */

static void
test_estop_with_held_mutex(void)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};
    s1_net_stats_t stats = {0};
    UINT pattern = 0u;
    UINT flags = 0u;
    bool b_early = false;
    bool b_pulse = false;
    bool b_wait = false;
    ER stat_err = E_OK;
    int waited = 0;
    TEST_INIT();

#if S1_TEST_SERIAL_HOST
    /* No threads here: nobody can hold the mutex while another task runs. */
    (void)command;  /* Cast is safe: marks the variable unused here. */
    (void)stats;    /* Cast is safe: marks the variable unused here. */
    (void)pattern;  /* Cast is safe: marks the variable unused here. */
    (void)flags;    /* Cast is safe: marks the variable unused here. */
    (void)b_early;  /* Cast is safe: marks the variable unused here. */
    (void)b_pulse;  /* Cast is safe: marks the variable unused here. */
    (void)b_wait;   /* Cast is safe: marks the variable unused here. */
    (void)stat_err; /* Cast is safe: marks the variable unused here. */
    (void)waited;   /* Cast is safe: marks the variable unused here. */
    TEST_CHECK("S1-U16a", true, "skipped on a PC without threads (Pico only)");
#else
    /* S1-U16: a diagnostic task holds the status mutex and the command queue
     * is full of PAUSE commands. EMERGENCY_STOP must still be published to
     * the safety path before the hook can reach any lock or the queue. */
    estop_reset();
    ctl_flag_create();
    {
        s1_codec_command_t pause = {S1_CMD_PAUSE, 0u};
        uint32_t index = 0u;

        for (index = 0u; index < PAUSE_FILL; index++)
        {
            tk_snd_mbf(s1_net_test_command_buffer(), &pause, sizeof(pause),
                       TMO_POL);
        }
    }
    app_start_task(mutex_holder_task, S1_PRI_SIM, TP_PRC2, "s1_t_holder");
    tk_wai_flg(g_ctl_flag, FLAG_HOLDING, TWF_ORW, &pattern, STATUS_WAIT_MS);
    app_start_task(estop_caller_task, S1_PRI_SIM, TP_PRC1, "s1_t_caller");

    for (waited = 0; (waited < HOLD_PROBE_MS) && !b_early;
         waited += HOLD_PROBE_STEP_MS)
    {
        b_early = s1_net_estop_latched();
        /* Wait on a flag that stays clear: a real delay on every runner. */
        tk_wai_flg(g_ctl_flag, FLAG_CALLER_DONE, TWF_ORW, &pattern,
                   HOLD_PROBE_STEP_MS);
    }
    b_pulse = (E_OK == s1_net_events_wait(S1_NET_FLG_ESTOP, &flags, TMO_POL));
    b_wait = (E_OK != tk_wai_flg(g_ctl_flag, FLAG_CALLER_DONE, TWF_ORW,
                                 &pattern, TMO_POL));
    TEST_CHECK("S1-U16a", b_early && b_pulse && b_wait,
               "latch and pulse published while the hook is still blocked on "
               "the held mutex");

    stat_err = s1_net_get_stats(&stats);
    TEST_CHECK("S1-U16b",
               (E_TMOUT == stat_err) && s1_net_estop_latched() &&
                   !s1_net_is_connected(),
               "readers stay bounded: get_stats times out, latch and link "
               "reads never block");

    tk_set_flg(g_ctl_flag, FLAG_RELEASE);
    tk_wai_flg(g_ctl_flag, FLAG_CALLER_DONE | FLAG_HOLDER_DONE, TWF_ANDW,
               &pattern, STATUS_WAIT_MS);
    TEST_CHECK("S1-U16c",
               (E_OK == s1_net_get_stats(&stats)) &&
                   (stats.estop_received >= 1u),
               "after the mutex is released the hook finishes and counts it");
    /* The command queue was full of PAUSE: the E-STOP is still delivered. */
    {
        bool b_found = false;

        while ((E_OK == s1_net_command_receive(&command, TMO_POL)) && !b_found)
        {
            b_found = (S1_CMD_EMERGENCY_STOP == command.command_id);
        }
        TEST_CHECK("S1-U16d", b_found && s1_net_estop_latched(),
                   "EMERGENCY_STOP also reaches the queue (full of PAUSE)");
    }
    estop_reset();
#endif
    TEST_SUMMARY("subsystem1 emergency with held mutex");
}

static void
test_estop_latch(void)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};
    s1_net_stats_t stats = {0};
    SYSTIM time = {0};
    UINT flags = 0u;
    uint32_t index = 0u;
    TEST_INIT();

    estop_reset();
    tk_get_otm(&time);

    /* S1-U17: the latch is set by EMERGENCY_STOP and by nothing else. */
    hook_command(S1_CMD_PAUSE, time.lo);
    TEST_CHECK("S1-U17a", !s1_net_estop_latched(),
               "an ordinary command does not latch an emergency");
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
    TEST_CHECK(
        "S1-U17b",
        s1_net_estop_latched() &&
            (E_OK == s1_net_events_wait(S1_NET_FLG_ESTOP, &flags, TMO_POL)),
        "EMERGENCY_STOP sets the latch and the pulse flag");

    /* S1-U18: nothing ordinary cancels it. */
    hook_command(S1_CMD_RESUME, time.lo);
    hook_command(S1_CMD_START_RUN, time.lo);
    hook_command(S1_CMD_PAUSE, time.lo);
    hook_bad_command(time.lo);
    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    s1_net_hook_on_event(S1_NET_EVENT_BACKOFF, BACKOFF_FIRST_MS);
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);
    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    s1_net_events_wait(S1_NET_FLG_COMMAND | S1_NET_FLG_ESTOP |
                           S1_NET_FLG_LINK_CHANGED,
                       &flags, TMO_POL);
    while (E_OK == s1_net_command_receive(&command, TMO_POL))
    {
        index++; /* Drain the whole queue, as a consumer would. */
    }
    TEST_CHECK("S1-U18a", (index >= 1u) && s1_net_estop_latched(),
               "RESUME, START_RUN, PAUSE, reconnects and draining the queue "
               "leave the latch set");
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);

    /* S1-U19: only a deliberate clear releases it, and a later ordinary
     * command does not set it again. */
    TEST_CHECK("S1-U19a",
               (E_OK == s1_net_estop_clear()) && !s1_net_estop_latched(),
               "s1_net_estop_clear() releases the latch");
    hook_command(S1_CMD_RESUME, time.lo);
    TEST_CHECK("S1-U19b", !s1_net_estop_latched(),
               "after the clear, RESUME does not latch again");
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
    s1_net_get_stats(&stats);
    TEST_CHECK("S1-U19c",
               s1_net_estop_latched() && (stats.estop_received >= 2u) &&
                   (0u == stats.estop_post_failures),
               "a new EMERGENCY_STOP latches again; no post failures counted");
    estop_reset();
    TEST_SUMMARY("subsystem1 emergency latch");
}

static void
test_estop_post_failure(void)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};
    s1_net_stats_t before = {0};
    s1_net_stats_t after = {0};
    SYSTIM time = {0};
    TEST_INIT();

#if S1_TEST_HOST_FAULTS
    estop_reset();
    tk_get_otm(&time);

    /* S1-U20: safety-posting failures are handled explicitly. One failed
     * kernel call is retried and the emergency still latches. */
    s1_net_get_stats(&before);
    kernel_emulation_fail_set_flg(FAIL_ONCE);
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U20a",
               s1_net_estop_latched() &&
                   (after.estop_post_failures == before.estop_post_failures),
               "one failed flag post is retried; the latch is set");
    kernel_emulation_fail_set_flg(0);
    estop_reset();

    /* If every post fails the failure is counted, and the command in the
     * queue is still the second path to the consumer. */
    s1_net_get_stats(&before);
    kernel_emulation_fail_set_flg(FAIL_ALWAYS);
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
    kernel_emulation_fail_set_flg(0);
    s1_net_get_stats(&after);
    TEST_CHECK(
        "S1-U20b",
        (after.estop_post_failures == (before.estop_post_failures + 1u)) &&
            (E_OK == s1_net_command_receive(&command, TMO_POL)) &&
            (S1_CMD_EMERGENCY_STOP == command.command_id),
        "a persistent post failure is counted and the queued command "
        "still delivers the emergency");
    estop_reset();
#else
    (void)command;
    (void)before;
    (void)after;
    (void)time;
    TEST_CHECK("S1-U20a", true, "fault injection runs on the PC runner only");
#endif
    TEST_SUMMARY("subsystem1 emergency post failure");
}

static void
test_command_validation(void)
{
    s1_codec_command_t command = {S1_CMD_EMERGENCY_STOP, 1u};
    s1_codec_command_t bad = {(s1_codec_command_id_t)BAD_COMMAND_ID, 0u};
    s1_codec_command_t fresh = {S1_CMD_PAUSE, 0u};
    s1_net_stats_t before = {0};
    s1_net_stats_t after = {0};
    SYSTIM time = {0};
    ER result = E_OK;
    TEST_INIT();

    estop_reset();
    tk_get_otm(&time);
    s1_net_get_stats(&before);

    /* S1-U21: a message of the wrong size is rejected, never half-filled. */
    /* A valid command id, but only the first bytes of the message. */
    tk_snd_mbf(s1_net_test_command_buffer(), &fresh, SHORT_MESSAGE_BYTES,
               TMO_POL);
    result = s1_net_command_receive(&command, TMO_POL);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U21a",
               (E_SYS == result) && (S1_CMD_NONE == command.command_id) &&
                   (0u == command.rx_timestamp_ms) &&
                   (after.commands_bad_size == (before.commands_bad_size + 1u)),
               "truncated message: E_SYS, output zeroed, counted");

    /* An unknown command id is rejected too. */
    tk_snd_mbf(s1_net_test_command_buffer(), &bad, sizeof(bad), TMO_POL);
    result = s1_net_command_receive(&command, TMO_POL);
    TEST_CHECK("S1-U21b",
               (E_SYS == result) && (S1_CMD_NONE == command.command_id),
               "unknown command id: E_SYS, output zeroed");

    /* The kernel's own errors are not disguised, and the queue is not stuck. */
    result = s1_net_command_receive(&command, TMO_POL);
    TEST_CHECK("S1-U21c", E_TMOUT == result,
               "empty queue returns the kernel's E_TMOUT unchanged");
    fresh.rx_timestamp_ms = time.lo;
    tk_snd_mbf(s1_net_test_command_buffer(), &fresh, sizeof(fresh), TMO_POL);
    result = s1_net_command_receive(&command, TMO_POL);
    TEST_CHECK("S1-U21d",
               (E_OK == result) && (S1_CMD_PAUSE == command.command_id),
               "a good message after a bad one is delivered");

    /* S1-U22: stale commands are dropped; an emergency never is. */
    s1_net_get_stats(&before);
    hook_command(S1_CMD_PAUSE, time.lo - STALE_AGE_MS);
    hook_command(S1_CMD_RESUME, time.lo);
    result = s1_net_command_receive(&command, TMO_POL);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U22a",
               (E_OK == result) && (S1_CMD_RESUME == command.command_id) &&
                   (after.commands_stale == (before.commands_stale + 1u)),
               "a 2 s old PAUSE is discarded, the fresh RESUME is delivered");
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo - VERY_STALE_AGE_MS);
    result = s1_net_command_receive(&command, TMO_POL);
    TEST_CHECK("S1-U22b",
               (E_OK == result) &&
                   (S1_CMD_EMERGENCY_STOP == command.command_id),
               "an old EMERGENCY_STOP is still delivered");
    hook_command(S1_CMD_PAUSE, time.lo - STALE_AGE_MS);
    result = s1_net_command_receive(&command, TMO_POL);
    TEST_CHECK("S1-U22c", E_TMOUT == result,
               "only a stale command queued: nothing is delivered");
    estop_reset();
    TEST_SUMMARY("subsystem1 command validation");
}

static void
test_simulator(void)
{
    char text[TEXT_CAPACITY] = {0};
    uint32_t frames = 0u;
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

#if !S1_WIRE_JSON
/* ---- Binary wire format and fault mask (S1-U23, S1-U24, S1-U25) ------- */

#define FAULT_ROUNDS 20000u
#define FAULT_HELD_BIT S1_FAULT_LINE_SENSOR
#define FAULT_TEST_BITS                                           \
    (S1_FAULT_MOTOR_STALL | S1_FAULT_IMU | S1_FAULT_LINE_SENSOR | \
     S1_FAULT_BARCODE)
#define BIN_BYTES S1_CODEC_TELEMETRY_BIN_BYTES

static ID g_fault_flag_id = 0;
static volatile uint32_t g_fault_violations_a = 0u;
static volatile uint32_t g_fault_violations_b = 0u;

/* A frame whose every group is in range. */
static s1_codec_frame_t
good_frame(uint32_t timestamp)
{
    s1_codec_frame_t frame = {0};

    frame.timestamp_ms = timestamp;
    frame.robot_state = S1_ROBOT_STATE_OPERATIONAL;
    frame.line_status = 3u;
    frame.motor_speed_left = 100;
    frame.motor_speed_right = -100;
    frame.encoder_generation = 1u;
    frame.encoder_timestamp_us = ((uint64_t)timestamp * 1000u) - 2000u;
    frame.hump_peak_x10 = 50;
    frame.obstacle_distance_x10 = 300;
    frame.sensor_age_us[0] = 5000u;
    frame.sensor_age_us[1] = 6000u;
    frame.sensor_age_us[2] = 7000u;
    frame.validity = S1_VALID_ALL;
    frame.schema_version = 2u;
    return frame;
}

/* Pull one message and decode it. */
static bool
pull_decoded(s1_codec_bin_telemetry_t *p_decoded)
{
    char text[TEXT_CAPACITY] = {0};
    uint32_t length = s1_net_hook_pull_telemetry(text, sizeof(text));

    return (BIN_BYTES == length) &&
           s1_codec_decode_telemetry_bin((const uint8_t *)text, length,
                                         p_decoded);
}

static void
test_binary_wire(void)
{
    char text[TEXT_CAPACITY] = {0};
    s1_codec_frame_t frame = {0};
    s1_codec_bin_telemetry_t decoded = {0};
    s1_codec_command_t command = {S1_CMD_NONE, 0u};
    s1_net_stats_t before = {0};
    s1_net_stats_t after = {0};
    SYSTIM time = {0};
    uint32_t length = 0u;
    uint32_t uptime = 0u;
    bool b_latch = false;
    bool b_ok = false;
    uint8_t bytes[2] = {0u, 0u};
    TEST_INIT();

    s1_sim_set_enabled(false);
    drain_telemetry();
    estop_reset();
    /* Earlier link tests left the "link was lost" bit set. Send one frame so
     * it is reported and cleared; the checks below then start clean. */
    frame = good_frame(MARKER - 1u);
    s1_net_telemetry_send(&frame);
    pull_decoded(&decoded);

    /* S1-U23a: a frame goes in as a struct and comes out as 29 packed bytes. */
    frame = good_frame(MARKER);
    b_ok = (E_OK == s1_net_telemetry_send(&frame)) && pull_decoded(&decoded);
    TEST_CHECK("S1-U23a",
               b_ok && (MARKER == decoded.timestamp_ms) &&
                   (100 == decoded.speed_left) &&
                   (-100 == decoded.speed_right) &&
                   (300u == decoded.obstacle_x10) &&
                   (S1_VALID_ALL == decoded.validity) &&
                   (0u == decoded.fault_mask),
               "telemetry is 29 packed bytes that decode to the same values");

    /* S1-U23b: an out-of-range value clears its validity bit, is counted,
     * and the rest of the frame is still sent. */
    s1_net_get_stats(&before);
    frame = good_frame(MARKER + 1u);
    frame.obstacle_distance_x10 = 5;
    b_ok = (E_OK == s1_net_telemetry_send(&frame)) && pull_decoded(&decoded);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U23b",
               b_ok && (0u == (decoded.validity & S1_VALID_OBSTACLE)) &&
                   (0u != (decoded.validity & S1_VALID_SPEED)) &&
                   (1u == (after.telemetry_out_of_range -
                           before.telemetry_out_of_range)),
               "out-of-range obstacle: validity bit cleared, counted, rest kept");

    /* S1-U23c: a robot state that needs more than 3 bits is refused. */
    frame = good_frame(MARKER + 2u);
    frame.robot_state = 9u;
    TEST_CHECK("S1-U23c",
               (E_PAR == s1_net_telemetry_send(&frame)) &&
                   (0u == s1_net_hook_pull_telemetry(text, sizeof(text))),
               "robot_state 9 is refused and never buffered");

    /* S1-U23d: commands are exactly one byte; text and bad bytes are
     * rejected; EMERGENCY_STOP still latches. */
    drain_commands();
    tk_get_otm(&time);
    s1_net_get_stats(&before);
    hook_command(S1_CMD_PAUSE, time.lo);
    b_ok = (E_OK == s1_net_command_receive(&command, TMO_POL)) &&
           (S1_CMD_PAUSE == command.command_id);
    s1_net_hook_on_command("EMERGENCY_STOP", 14u, time.lo); /* old text */
    bytes[0] = 0x01u;
    bytes[1] = 0x00u;
    s1_net_hook_on_command((const char *)bytes, 2u, time.lo); /* length 2 */
    bytes[0] = 0x81u;
    s1_net_hook_on_command((const char *)bytes, 1u, time.lo); /* reserved */
    hook_bad_command(time.lo);
    s1_net_get_stats(&after);
    TEST_CHECK("S1-U23d",
               b_ok && !s1_net_estop_latched() &&
                   (4u == (after.commands_rejected - before.commands_rejected)) &&
                   (E_TMOUT == s1_net_command_receive(&command, TMO_POL)),
               "1-byte command works; text, 2 bytes, reserved bits, id 7 rejected");
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
    TEST_CHECK("S1-U23e", s1_net_estop_latched(),
               "byte 0x01 latches the emergency stop");

    /* S1-U23f: the heartbeat is 8 bytes and shows the latch. */
    length = s1_net_hook_make_heartbeat(text, sizeof(text), 777u);
    b_ok = s1_codec_decode_heartbeat_bin((const uint8_t *)text, length,
                                         &uptime, &b_latch, NULL);
    TEST_CHECK("S1-U23f",
               b_ok && (8u == length) && (777u == uptime) && b_latch,
               "heartbeat is 8 bytes with uptime and the latch");
    estop_reset();
    TEST_SUMMARY("subsystem1 binary wire format");
}

static void
test_fault_mask(void)
{
    s1_codec_frame_t frame = {0};
    s1_codec_bin_telemetry_t decoded = {0};
    SYSTIM time = {0};
    bool b_ok = false;
    TEST_INIT();

    drain_telemetry();
    estop_reset();
    s1_net_fault_clear(S1_FAULT_ALL & ~S1_FAULT_OWNED_BY_S1);
    frame = good_frame(MARKER + 10u);

    /* S1-U24a: owners set their bits; the next frame carries them. */
    b_ok = (E_OK == s1_net_fault_set(S1_FAULT_MOTOR_STALL | S1_FAULT_IMU)) &&
           (E_OK == s1_net_telemetry_send(&frame)) && pull_decoded(&decoded);
    TEST_CHECK("S1-U24a",
               b_ok && (0x05u == s1_net_fault_get()) &&
                   (0x05u == decoded.fault_mask),
               "two bits set: mask 0x05, carried in the frame");

    /* S1-U24b: clearing one bit leaves the other; a zero mask does nothing. */
    TEST_CHECK("S1-U24b",
               (E_OK == s1_net_fault_clear(S1_FAULT_MOTOR_STALL)) &&
                   (E_OK == s1_net_fault_set(0u)) &&
                   (E_OK == s1_net_fault_clear(0u)) &&
                   (S1_FAULT_IMU == s1_net_fault_get()),
               "clearing one bit keeps the other");

    /* S1-U24c: other subsystems may not touch the Buddy 1 bits. */
    TEST_CHECK("S1-U24c",
               (E_PAR == s1_net_fault_set(S1_FAULT_WIFI_LOSS)) &&
                   (E_PAR == s1_net_fault_set(S1_FAULT_ESTOP_LATCHED |
                                              S1_FAULT_BARCODE)) &&
                   (E_PAR == s1_net_fault_clear(S1_FAULT_ESTOP_LATCHED)) &&
                   (S1_FAULT_IMU == s1_net_fault_get()),
               "setting or clearing a Buddy 1 bit is refused, nothing changes");

    /* S1-U24d: bit 5 is the emergency latch itself. */
    tk_get_otm(&time);
    hook_command(S1_CMD_EMERGENCY_STOP, time.lo);
    b_ok = (0u != (s1_net_fault_get() & S1_FAULT_ESTOP_LATCHED));
    s1_net_telemetry_send(&frame);
    b_ok = b_ok && pull_decoded(&decoded) &&
           ((S1_FAULT_IMU | S1_FAULT_ESTOP_LATCHED) == decoded.fault_mask);
    estop_reset();
    TEST_CHECK("S1-U24d",
               b_ok && (0u == (s1_net_fault_get() & S1_FAULT_ESTOP_LATCHED)),
               "bit 5 follows the emergency latch, set and cleared");

    /* S1-U24e: a lost link is reported once, in the first frame after it
     * returns, and other owners' bits are not disturbed. */
    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    b_ok = (0u == (s1_net_fault_get() & S1_FAULT_WIFI_LOSS));
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);
    b_ok = b_ok && (0u != (s1_net_fault_get() & S1_FAULT_WIFI_LOSS));
    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    s1_net_telemetry_send(&frame);
    b_ok = b_ok && pull_decoded(&decoded) &&
           (0u != (decoded.fault_mask & S1_FAULT_WIFI_LOSS)) &&
           (0u != (decoded.fault_mask & S1_FAULT_IMU));
    s1_net_telemetry_send(&frame);
    b_ok = b_ok && pull_decoded(&decoded) &&
           (0u == (decoded.fault_mask & S1_FAULT_WIFI_LOSS)) &&
           (0u != (decoded.fault_mask & S1_FAULT_IMU));
    TEST_CHECK("S1-U24e", b_ok,
               "Wi-Fi loss bit: reported once after the outage, IMU bit kept");
    s1_net_fault_clear(S1_FAULT_ALL & ~S1_FAULT_OWNED_BY_S1);
    TEST_SUMMARY("subsystem1 fault mask");
}

/* Two tasks on different cores each toggle their own bit thousands of times
 * while checking that a third bit, set once, is never seen cleared. A
 * read-modify-write implementation can lose that third bit. */
static void
fault_task_a(INT stacd, void *p_exinf)
{
    uint32_t round = 0u;

    (void)stacd;   /* Cast is safe: discards an unused task parameter. */
    (void)p_exinf; /* Cast is safe: discards an unused task parameter. */
    for (round = 0u; round < FAULT_ROUNDS; round++)
    {
        s1_net_fault_set(S1_FAULT_MOTOR_STALL);
        if (0u == (s1_net_fault_get() & FAULT_HELD_BIT))
        {
            g_fault_violations_a++;
        }
        s1_net_fault_clear(S1_FAULT_MOTOR_STALL);
        if (0u == (s1_net_fault_get() & FAULT_HELD_BIT))
        {
            g_fault_violations_a++;
        }
    }
    s1_net_fault_set(S1_FAULT_MOTOR_STALL);
    tk_set_flg(g_fault_flag_id, FLAG_WRITER_DONE);
    tk_ext_tsk();
}

static void
fault_task_b(INT stacd, void *p_exinf)
{
    uint32_t round = 0u;

    (void)stacd;   /* Cast is safe: discards an unused task parameter. */
    (void)p_exinf; /* Cast is safe: discards an unused task parameter. */
    for (round = 0u; round < FAULT_ROUNDS; round++)
    {
        s1_net_fault_set(S1_FAULT_IMU);
        if (0u == (s1_net_fault_get() & FAULT_HELD_BIT))
        {
            g_fault_violations_b++;
        }
        s1_net_fault_clear(S1_FAULT_IMU);
        if (0u == (s1_net_fault_get() & FAULT_HELD_BIT))
        {
            g_fault_violations_b++;
        }
    }
    s1_net_fault_set(S1_FAULT_IMU);
    tk_set_flg(g_fault_flag_id, FLAG_READER_DONE);
    tk_ext_tsk();
}

static void
test_fault_two_core(void)
{
    T_CFLG flag_info = {0};
    UINT pattern = 0u;
    ER result = E_OK;
    TEST_INIT();

    s1_net_fault_clear(S1_FAULT_ALL & ~S1_FAULT_OWNED_BY_S1);
    s1_net_fault_set(FAULT_HELD_BIT);
    g_fault_violations_a = 0u;
    g_fault_violations_b = 0u;
    flag_info.flgatr = TA_TFIFO | TA_WMUL;
    g_fault_flag_id = tk_cre_flg(&flag_info);

    app_start_task(fault_task_a, S1_PRI_SIM, TP_PRC2, "s1_t_fault_a");
    app_start_task(fault_task_b, S1_PRI_SIM, TP_PRC1, "s1_t_fault_b");
    result = tk_wai_flg(g_fault_flag_id, FLAG_WRITER_DONE | FLAG_READER_DONE,
                        TWF_ANDW, &pattern, STATUS_WAIT_MS);

    TEST_CHECK("S1-U25a", E_OK == result,
               "both fault tasks finished (no deadlock)");
    TEST_CHECK("S1-U25b",
               (0u == g_fault_violations_a) && (0u == g_fault_violations_b) &&
                   ((FAULT_HELD_BIT | S1_FAULT_MOTOR_STALL | S1_FAULT_IMU) ==
                    s1_net_fault_get()),
               "two cores toggling their own bits: the third bit is never lost");
    tm_printf((UB *)"s1: S1-U25 violations a=%d b=%d final=0x%x\n",
              (int)g_fault_violations_a, (int)g_fault_violations_b,
              (unsigned)s1_net_fault_get());
    s1_net_fault_clear(S1_FAULT_ALL & ~S1_FAULT_OWNED_BY_S1);
    TEST_SUMMARY("subsystem1 fault mask, two cores");
}

/* ---- Mapping update path (S1-U26) ---------------------------------------- */

static void
map_hook(const char *text)
{
    uint32_t length = 0u;

    while ('\0' != text[length])
    {
        length++;
    }
    s1_net_hook_on_command(text, length, 0u);
}

static s1_codec_map_ack_t
map_heartbeat_ack(void)
{
    char text[S1_CODEC_HEARTBEAT_JSON_CAPACITY] = {0};
    s1_codec_map_ack_t ack = {99u, 99u, 99u};
    uint32_t uptime = 0u;
    bool b_latch = false;
    uint32_t length = s1_net_hook_make_heartbeat(text, sizeof(text), 1u);

    if (!s1_codec_decode_heartbeat_bin((const uint8_t *)text, length, &uptime,
                                       &b_latch, &ack))
    {
        ack.result = 99u;
    }
    return ack;
}

static void
test_map_path(void)
{
    s1_net_map_request_t request = {{0u, {0u}}, 0u, 0u};
    s1_net_stats_t before = {0};
    s1_net_stats_t after = {0};
    s1_codec_map_ack_t ack = {0u, 0u, 0u};
    bool b_ok = false;
    TEST_INIT();

    /* S1-U26a: text is parsed by nav_update_parse() and queued as a binary update. */
    map_hook("MAP A=FORWARD B=LEFT");
    b_ok = (E_OK == s1_net_map_receive(&request, TMO_POL)) &&
           (3u == request.update.selected) &&
           (NAV_COMMAND_STRAIGHT ==
            nav_action_get(request.update.actions, 0u)) &&
           (NAV_COMMAND_LEFT == nav_action_get(request.update.actions, 1u));
    TEST_CHECK("S1-U26a", b_ok,
               "MAP text becomes a binary update read with nav_action_get()");

    /* S1-U26b: bad text is rejected before Buddy 3 and reported. */
    s1_net_get_stats(&before);
    map_hook("MAP A=SIDEWAYS");
    s1_net_get_stats(&after);
    ack = map_heartbeat_ack();
    TEST_CHECK("S1-U26b",
               (E_TMOUT == s1_net_map_receive(&request, TMO_POL)) &&
                   (1u == (after.map_rejected - before.map_rejected)) &&
                   ((uint8_t)S1_MAP_RESULT_REJECTED == ack.result),
               "bad MAP text: nothing queued, counted, heartbeat says REJECTED");

    /* S1-U26c: only a reply makes an update ACCEPTED; an older report never
     * overwrites a newer one. */
    TEST_CHECK("S1-U26c",
               (E_OK == s1_net_map_report(request.seq + 5u,
                                          S1_MAP_RESULT_ACCEPTED, 261u)) &&
                   (E_OK == s1_net_map_report(request.seq,
                                              S1_MAP_RESULT_QUEUED, 0u)) &&
                   ((uint8_t)S1_MAP_RESULT_ACCEPTED ==
                    map_heartbeat_ack().result) &&
                   (5u == map_heartbeat_ack().revision) &&
                   (E_PAR == s1_net_map_report(1u, S1_MAP_RESULT_NONE, 0u)),
               "ACCEPTED from the reply is kept (revision low 8 bits); stale report ignored");

    /* S1-U26d: map traffic never delays an emergency stop. */
    for (before.map_dropped = 0u; before.map_dropped < 6u;
         before.map_dropped++)
    {
        map_hook("MAP G=LEFT");
    }
    hook_command(S1_CMD_EMERGENCY_STOP, 0u);
    TEST_CHECK("S1-U26d", s1_net_estop_latched(),
               "EMERGENCY_STOP latches at once behind a flood of MAP updates");
    while (E_OK == s1_net_map_receive(&request, TMO_POL))
    {
        /* Drain the updates left in the buffer. */
    }
    estop_reset();
    TEST_SUMMARY("subsystem1 mapping update path");
}
#endif /* !S1_WIRE_JSON */

/* Entry point called by usermain() in a TEST=1 build. */
void subsystem1_test(void)
{
    test_codec();
    test_commands_and_backoff();
    test_telemetry_buffer();
    test_command_path();
    test_link_events();
    test_backoff_notification();
    test_concurrent_status();
    test_estop_with_held_mutex();
    test_estop_latch();
    test_estop_post_failure();
    test_command_validation();
#if !S1_WIRE_JSON
    test_binary_wire();
    test_fault_mask();
    test_fault_two_core();
    test_map_path();
#endif
    test_simulator();
    test_sim_switch();
}

#endif /* APP_TEST */

/*** end of file ***/
