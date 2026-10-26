/* Host tests for the BINARY path of s1_net.c (default wire format). The older
 * application tests (subsystem1_test.c) run with -DS1_WIRE_JSON=1 and cover the
 * JSON debug path; this file covers the binary path end to end:
 * frame -> packed buffer -> pull hook -> decode, and command bytes -> latch. */
#include <stdio.h>
#include <string.h>
#ifdef HOST_THREADS
#include <pthread.h>
#endif

#include "../../../app_program/subsystem1/s1_net.h"

static int g_checks = 0, g_failures = 0;
static void check(int ok, const char *name)
{
    g_checks++;
    if (!ok)
    {
        g_failures++;
        printf("  FAIL %s\n", name);
    }
}

static s1_codec_frame_t make_frame(uint32_t stamp)
{
    s1_codec_frame_t f;
    memset(&f, 0, sizeof f);
    f.timestamp_ms = stamp;
    f.robot_state = 2;
    f.line_status = 3;
    f.motor_speed_left = 100;
    f.motor_speed_right = -100;
    f.encoder_count_left = 10;
    f.encoder_count_right = 20;
    f.encoder_generation = 1;
    f.encoder_timestamp_us = (uint64_t)stamp * 1000u - 2000u;
    f.hump_peak_x10 = 50;
    f.obstacle_distance_x10 = 300;
    f.sensor_age_us[0] = 5000;
    f.sensor_age_us[1] = 6000;
    f.sensor_age_us[2] = 7000;
    f.validity = S1_VALID_ALL;
    f.schema_version = 2;
    return f;
}

static uint32_t pull(s1_codec_bin_telemetry_t *p_out, int *p_ok)
{
    char buf[S1_CODEC_TELEMETRY_JSON_CAPACITY];
    uint32_t n = s1_net_hook_pull_telemetry(buf, sizeof buf);
    *p_ok = (n == 29u) && s1_codec_decode_telemetry_bin((const uint8_t *)buf, n, p_out);
    return n;
}

static void drain(void)
{
    char buf[S1_CODEC_TELEMETRY_JSON_CAPACITY];
    while (s1_net_hook_pull_telemetry(buf, sizeof buf) > 0u)
    {
    }
}

static void send_cmd(uint8_t byte)
{
    SYSTIM t = {0};
    tk_get_otm(&t);
    s1_net_hook_on_command((const char *)&byte, 1u, t.lo);
}

#ifdef HOST_THREADS
#define FAULT_ROUNDS 20000
static void *fault_worker_a(void *p)
{
    int i;
    (void)p;
    for (i = 0; i < FAULT_ROUNDS; i++)
    {
        s1_net_fault_set(S1_FAULT_MOTOR_STALL);
        s1_net_fault_clear(S1_FAULT_MOTOR_STALL);
    }
    s1_net_fault_set(S1_FAULT_MOTOR_STALL);
    return NULL;
}
static void *fault_worker_b(void *p)
{
    int i;
    (void)p;
    for (i = 0; i < FAULT_ROUNDS; i++)
    {
        s1_net_fault_set(S1_FAULT_IMU);
        s1_net_fault_clear(S1_FAULT_IMU);
    }
    s1_net_fault_set(S1_FAULT_IMU);
    return NULL;
}
#endif

static void test_faults(void)
{
    s1_codec_frame_t f = make_frame(9000u);
    s1_codec_bin_telemetry_t d;
    int ok = 0;

    drain();
    check(s1_net_fault_get() == 0u, "F0 no faults at the start");
    check(E_OK == s1_net_fault_set(S1_FAULT_MOTOR_STALL | S1_FAULT_IMU), "F1 set two bits");
    check(s1_net_fault_get() == 0x05u, "F1 mask is 0x05");
    s1_net_telemetry_send(&f);
    pull(&d, &ok);
    check(ok && d.fault_mask == 0x05u, "F1 the frame carries 0x05");
    check(E_OK == s1_net_fault_clear(S1_FAULT_MOTOR_STALL) && s1_net_fault_get() == 0x04u, "F2 clearing one bit keeps the other");
    check(E_OK == s1_net_fault_set(0u) && E_OK == s1_net_fault_clear(0u) && s1_net_fault_get() == 0x04u, "F2 zero mask is a no-op");

    check(E_PAR == s1_net_fault_set(S1_FAULT_WIFI_LOSS), "F3 set of a Buddy 1 bit refused");
    check(E_PAR == s1_net_fault_set(S1_FAULT_ESTOP_LATCHED | S1_FAULT_BARCODE), "F3 mixed mask refused as a whole");
    check(E_PAR == s1_net_fault_clear(S1_FAULT_ESTOP_LATCHED), "F3 clear of a Buddy 1 bit refused");
    check(s1_net_fault_get() == 0x04u, "F3 refused calls changed nothing");

    s1_net_hook_on_command("\x01", 1u, 0u);
    check(s1_net_estop_latched() && (s1_net_fault_get() & S1_FAULT_ESTOP_LATCHED) != 0u, "F4 latch shows as bit 5");
    s1_net_telemetry_send(&f);
    pull(&d, &ok);
    check(ok && d.fault_mask == 0x24u, "F4 frame carries the latch and the IMU bit");
    s1_net_estop_clear();
    check((s1_net_fault_get() & S1_FAULT_ESTOP_LATCHED) == 0u, "F4 bit 5 follows the latch when cleared");

    /* Wi-Fi loss: reported once, in the first frame after the link returns. */
    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    check((s1_net_fault_get() & S1_FAULT_WIFI_LOSS) == 0u, "F5 first connect is not a loss");
    s1_net_hook_on_event(S1_NET_EVENT_DISCONNECTED, 0u);
    check((s1_net_fault_get() & S1_FAULT_WIFI_LOSS) != 0u, "F5 disconnect sets bit 4");
    s1_net_hook_on_event(S1_NET_EVENT_BACKOFF, 1000u);
    s1_net_hook_on_event(S1_NET_EVENT_CONNECTED, 0u);
    s1_net_telemetry_send(&f);
    pull(&d, &ok);
    check(ok && (d.fault_mask & S1_FAULT_WIFI_LOSS) != 0u && (d.fault_mask & S1_FAULT_IMU) != 0u, "F5 first frame after the outage carries bit 4");
    s1_net_telemetry_send(&f);
    pull(&d, &ok);
    check(ok && (d.fault_mask & S1_FAULT_WIFI_LOSS) == 0u && (d.fault_mask & S1_FAULT_IMU) != 0u, "F5 second frame: bit 4 gone, IMU bit untouched");

#ifdef HOST_THREADS
    {
        pthread_t ta, tb;
        s1_net_fault_clear(S1_FAULT_IMU);
        pthread_create(&ta, NULL, fault_worker_a, NULL);
        pthread_create(&tb, NULL, fault_worker_b, NULL);
        pthread_join(ta, NULL);
        pthread_join(tb, NULL);
        check(s1_net_fault_get() == (S1_FAULT_MOTOR_STALL | S1_FAULT_IMU), "F6 two threads, different bits: neither bit lost");
    }
#endif
}

int main(void)
{
    s1_codec_frame_t f;
    s1_codec_bin_telemetry_t d;
    s1_net_stats_t st;
    int ok = 0;
    uint32_t i, n;
    char buf[64];

    if (s1_net_init() < 0)
    {
        printf("init failed\n");
        return 2;
    }

    /* B1: one frame, packed, sequence stamped on the way out. */
    f = make_frame(1000u);
    check(E_OK == s1_net_telemetry_send(&f), "B1 send");
    n = pull(&d, &ok);
    check(n == 29u && ok, "B1 pull gives 29 decodable bytes");
    check(d.sequence == 0u && d.timestamp_ms == 1000u && d.speed_right == -100 && d.obstacle_x10 == 300u && d.validity == 63u, "B1 content");
    check(d.fault_mask == 0u, "B1 fault mask is 0 until step 5");
    s1_net_telemetry_send(&f);
    n = pull(&d, &ok);
    check(ok && d.sequence == 1u, "B1 sequence counts up");
    check(0u == s1_net_hook_pull_telemetry(buf, sizeof buf), "B1 empty buffer returns 0");

    /* B2: drop-oldest in the packed buffer; the loss is a sequence gap. */
    drain();
    s1_net_get_stats(&st);
    for (i = 0u; i < 20u; i++)
    {
        f = make_frame(2000u + i);
        check(E_OK == s1_net_telemetry_send(&f), "B2 send never fails");
    }
    n = pull(&d, &ok);
    check(ok && d.timestamp_ms == 2004u, "B2 oldest 4 dropped, frame 5 is first");
    check(d.sequence == 4u + 2u, "B2 sequence skips the 4 dropped frames");
    for (i = 1u; i < 16u; i++)
    {
        n = pull(&d, &ok);
        if (!ok)
        {
            break;
        }
    }
    check(ok && d.timestamp_ms == 2019u, "B2 newest frame kept");
    s1_net_get_stats(&st);
    check(st.telemetry_overwritten >= 4u, "B2 overwritten counted");

    /* B3: out-of-range value -> validity bit cleared, counted, still sent. */
    drain();
    f = make_frame(3000u);
    f.obstacle_distance_x10 = 5;
    check(E_OK == s1_net_telemetry_send(&f), "B3 send");
    n = pull(&d, &ok);
    check(ok && (d.validity & S1_VALID_OBSTACLE) == 0u && (d.validity & S1_VALID_SPEED) != 0u, "B3 only obstacle bit cleared");
    s1_net_get_stats(&st);
    check(st.telemetry_out_of_range == 1u, "B3 counted once");

    /* B4: robot_state outside 0..7 is refused and never buffered. */
    f = make_frame(4000u);
    f.robot_state = 9;
    check(E_PAR == s1_net_telemetry_send(&f), "B4 refused with E_PAR");
    check(0u == s1_net_hook_pull_telemetry(buf, sizeof buf), "B4 nothing buffered");
    check(E_PAR == s1_net_telemetry_send(NULL), "B4 NULL frame");
    s1_net_get_stats(&st);
    check(st.telemetry_out_of_range == 2u, "B4 counted");

    /* B5: a pull buffer that is too small gives nothing and is counted. */
    f = make_frame(5000u);
    s1_net_telemetry_send(&f);
    check(0u == s1_net_hook_pull_telemetry(buf, 28u), "B5 capacity 28 -> 0 bytes");
    s1_net_get_stats(&st);
    check(st.telemetry_oversize >= 1u, "B5 oversize counted");

    /* B6: binary commands. */
    {
        s1_codec_command_t c = {S1_CMD_NONE, 0u};
        UINT fl = 0u;
        s1_net_stats_t a, b;
        s1_net_events_wait(S1_NET_FLG_COMMAND | S1_NET_FLG_ESTOP, &fl, TMO_POL);
        s1_net_get_stats(&a);
        send_cmd(0x03u); /* PAUSE */
        check(!s1_net_estop_latched(), "B6 PAUSE does not latch");
        check(E_OK == s1_net_command_receive(&c, TMO_POL) && c.command_id == S1_CMD_PAUSE, "B6 PAUSE delivered");
        send_cmd(0x00u);
        send_cmd(0x05u);
        send_cmd(0x09u);
        send_cmd(0x81u);
        s1_net_hook_on_command("EMERGENCY_STOP", 14u, 0u); /* old text must be rejected */
        s1_net_hook_on_command("\x01\x00", 2u, 0u);        /* wrong length */
        s1_net_hook_on_command("", 0u, 0u);                /* empty */
        s1_net_get_stats(&b);
        check(b.commands_rejected - a.commands_rejected == 7u, "B6 7 bad payloads rejected");
        check(!s1_net_estop_latched(), "B6 rejected payloads never latch");
        check(E_TMOUT == s1_net_command_receive(&c, TMO_POL), "B6 nothing queued by bad payloads");
        send_cmd(0x01u); /* EMERGENCY_STOP */
        check(s1_net_estop_latched(), "B6 byte 0x01 latches the emergency stop");
        check(E_OK == s1_net_events_wait(S1_NET_FLG_ESTOP, &fl, TMO_POL), "B6 estop flag set");
        check(E_OK == s1_net_command_receive(&c, TMO_POL) && c.command_id == S1_CMD_EMERGENCY_STOP, "B6 EMERGENCY_STOP delivered");
    }

    /* B7: heartbeat reflects the latch. */
    {
        uint32_t up = 0u;
        bool es = false;
        n = s1_net_hook_make_heartbeat(buf, sizeof buf, 777u);
        check(n == 8u && s1_codec_decode_heartbeat_bin((const uint8_t *)buf, n, &up, &es, NULL) && up == 777u && es, "B7 heartbeat 8 bytes, estop set");
        s1_net_estop_clear();
        n = s1_net_hook_make_heartbeat(buf, sizeof buf, 778u);
        check(n == 8u && s1_codec_decode_heartbeat_bin((const uint8_t *)buf, n, &up, &es, NULL) && up == 778u && !es, "B7 estop cleared");
        check(0u == s1_net_hook_make_heartbeat(buf, 7u, 1u), "B7 small buffer");
    }

    test_faults();

    printf("net_bin: %d checks, %d failed\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}