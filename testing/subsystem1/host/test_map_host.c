/* Host tests for the mapping-update gateway: robot/cmd "MAP ..." text ->
 * nav_update_parse() (Buddy 3) -> s1_map_pump() -> s3_submit() (mocked) ->
 * S3 reply -> s1_map_on_reply() -> heartbeat acknowledgement.
 * s3_submit() is a mock that records the request exactly as Buddy 3 would
 * receive it; nothing here builds Buddy 3's queue bytes. */
#include <stdio.h>
#include <string.h>

#include "../../../app_program/subsystem1/s1_map.h"

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

/* ---- mock of Buddy 3's API ------------------------------------------- */
static int g_submits = 0;
static ER g_submit_result = E_OK;
static s3_input_t g_last_input;

ER s3_submit(const s3_input_t *p_input, TMO timeout)
{
    (void)timeout;
    g_submits++;
    g_last_input = *p_input;
    return g_submit_result;
}

static void hook(const char *text)
{
    s1_net_hook_on_command(text, (uint32_t)strlen(text), 0u);
}

static s1_codec_map_ack_t heartbeat_ack(void)
{
    char buf[16];
    uint32_t up = 0u;
    bool es = false;
    s1_codec_map_ack_t ack = {99u, 99u, 99u};
    uint32_t n = s1_net_hook_make_heartbeat(buf, sizeof buf, 1u);

    if (!((8u == n) && s1_codec_decode_heartbeat_bin((const uint8_t *)buf, n, &up, &es, &ack)))
    {
        ack.result = 99u;
    }
    return ack;
}

static s3_output_t reply(uint32_t id, bool accepted, uint32_t revision)
{
    s3_output_t o = {0};
    o.kind = S3_OUTPUT_REPLY;
    o.request_id = id;
    o.accepted = accepted ? 1u : 0u;
    o.map_revision = revision;
    return o;
}

int main(void)
{
    s1_codec_map_ack_t a;
    s3_output_t r;
    s1_net_stats_t st;
    uint32_t now = 1000u;
    char big[200];
    int i;

    if (s1_net_init() < 0)
    {
        printf("init failed\n");
        return 2;
    }
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_NONE && a.seq == 0u, "M0 nothing seen yet: result NONE");

    /* M1: valid update is parsed, submitted once, reported as QUEUED only. */
    hook("MAP A=FORWARD B=LEFT Z=UTURN");
    s1_map_pump(now);
    check(g_submits == 1, "M1 one s3_submit()");
    check(g_last_input.kind == S3_INPUT_MAPPING, "M1 kind is S3_INPUT_MAPPING");
    check(g_last_input.request_id == (S1_MAP_REQUEST_ID_FLAG | 1u), "M1 request id = flag | seq 1");
    check(g_last_input.value.mapping.selected == ((1u << 0) | (1u << 1) | (1u << 25)), "M1 selected letters A, B, Z");
    check(nav_action_get(g_last_input.value.mapping.actions, 0u) == NAV_COMMAND_STRAIGHT &&
              nav_action_get(g_last_input.value.mapping.actions, 1u) == NAV_COMMAND_LEFT &&
              nav_action_get(g_last_input.value.mapping.actions, 25u) == NAV_COMMAND_U_TURN,
          "M1 actions read with nav_action_get()");
    check(nav_action_get(g_last_input.value.mapping.actions, 2u) == NAV_COMMAND_NONE, "M1 unselected letter C stays NONE");
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_QUEUED && a.seq == 1u, "M1 heartbeat says QUEUED (not accepted) for seq 1");

    /* M2: only S3's reply makes it ACCEPTED. A stranger's reply is not ours. */
    r = reply(1u, true, 7u);
    check(!s1_map_on_reply(&r), "M2 reply with the controller's own id 1 is not a map reply");
    r = reply(S1_MAP_REQUEST_ID_FLAG | 2u, true, 7u);
    check(!s1_map_on_reply(&r), "M2 reply for another seq is not ours");
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_QUEUED, "M2 still QUEUED after unrelated replies");
    r = reply(S1_MAP_REQUEST_ID_FLAG | 1u, true, 7u);
    check(s1_map_on_reply(&r), "M2 matching reply consumed");
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_ACCEPTED && a.seq == 1u && a.revision == 7u, "M2 ACCEPTED with revision 7");

    /* M3: a reply with accepted = 0 is REJECTED and still consumed (no fault). */
    hook("MAP C=RIGHT");
    s1_map_pump(now += 10u);
    check(g_submits == 2 && g_last_input.request_id == (S1_MAP_REQUEST_ID_FLAG | 2u), "M3 seq 2 submitted");
    r = reply(S1_MAP_REQUEST_ID_FLAG | 2u, false, 7u);
    check(s1_map_on_reply(&r), "M3 rejected reply is consumed by the gateway");
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_REJECTED && a.seq == 2u, "M3 REJECTED seq 2");
    r = reply(S1_MAP_REQUEST_ID_FLAG | 2u, false, 7u);
    check(!s1_map_on_reply(&r), "M3 a duplicate reply is not matched again");

    /* M4: text boundary. Every bad payload is rejected before Buddy 3. */
    {
        const char *bad[] = {"MAP A=SIDEWAYS", "MAP ", "MAP", "MAP a=FORWARD", "MAP A=FORWARD A=LEFT",
                             "MAP A=FORWARD  B", "MAPA=FORWARD", "MAP 1=FORWARD", "MAP A=forward"};
        int before = g_submits;
        uint32_t rej0, rej1;
        s1_net_get_stats(&st);
        rej0 = st.map_rejected;
        for (i = 0; i < (int)(sizeof bad / sizeof bad[0]); i++)
        {
            hook(bad[i]);
            s1_map_pump(now += 10u);
        }
        s1_net_get_stats(&st);
        rej1 = st.map_rejected;
        check(g_submits == before, "M4 no bad text reaches s3_submit()");
        check(rej1 - rej0 == (uint32_t)(sizeof bad / sizeof bad[0]), "M4 each bad text counted as rejected");
        a = heartbeat_ack();
        check(a.result == S1_MAP_RESULT_REJECTED && a.seq == (uint8_t)(2 + (int)(sizeof bad / sizeof bad[0])),
              "M4 heartbeat reports REJECTED for the newest seq");
    }
    s1_net_hook_on_command("MAP A=FORWARD\0B=LEFT", 20u, 0u);
    s1_map_pump(now += 10u);
    {
        int before = g_submits;
        memset(big, 'A', sizeof big);
        memcpy(big, "MAP A=FORWARD", 13u);
        s1_net_hook_on_command(big, 121u, 0u); /* one byte over the cap */
        s1_net_hook_on_command(big, 200u, 0u);
        s1_map_pump(now += 10u);
        check(g_submits == before, "M4 embedded zero and over-long payloads never reach S3");
    }

    /* M5: the S3 queue is full: reported as REJECTED, next update still works. */
    g_submit_result = E_TMOUT;
    hook("MAP D=LEFT");
    s1_map_pump(now += 10u);
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_REJECTED, "M5 s3_submit() failure is REJECTED, not QUEUED");
    g_submit_result = E_OK;
    hook("MAP D=LEFT");
    s1_map_pump(now += 10u);
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_QUEUED, "M5 next update is queued normally");

    /* M6: no reply from Buddy 3 within the timeout. */
    s1_map_pump(now + S1_MAP_REPLY_TIMEOUT_MS - 1u);
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_QUEUED, "M6 still waiting just inside the timeout");
    s1_map_pump(now + S1_MAP_REPLY_TIMEOUT_MS + 2u);
    a = heartbeat_ack();
    check(a.result == S1_MAP_RESULT_REJECTED, "M6 timeout is reported as REJECTED");
    now += S1_MAP_REPLY_TIMEOUT_MS + 10u;

    /* M7: one update at a time; the second waits in the buffer. */
    {
        int before = g_submits;
        hook("MAP E=RIGHT");
        hook("MAP F=LEFT");
        s1_map_pump(now += 10u);
        check(g_submits == before + 1, "M7 only the first is submitted while it is outstanding");
        s1_map_pump(now += 10u);
        check(g_submits == before + 1, "M7 the second waits for the reply");
        r = reply(g_last_input.request_id, true, 9u);
        check(s1_map_on_reply(&r), "M7 first reply consumed");
        s1_map_pump(now += 10u);
        check(g_submits == before + 2, "M7 the second is submitted after the reply");
        check(g_last_input.value.mapping.selected == (1u << 5), "M7 and it is the F update");
        r = reply(g_last_input.request_id, true, 10u);
        check(s1_map_on_reply(&r), "M7 second reply consumed");
        a = heartbeat_ack();
        check(a.result == S1_MAP_RESULT_ACCEPTED && a.revision == 10u, "M7 heartbeat shows the newest result");
    }

    /* M8: a flood of updates cannot delay or hide an emergency stop. */
    for (i = 0; i < 20; i++)
    {
        hook("MAP G=LEFT");
    }
    s1_net_get_stats(&st);
    check(st.map_dropped > 0u, "M8 surplus updates are dropped and counted");
    check(!s1_net_estop_latched(), "M8 map traffic never latches an emergency");
    s1_net_hook_on_command("\x01", 1u, 0u);
    check(s1_net_estop_latched(), "M8 EMERGENCY_STOP latches at once, behind the map flood");
    {
        s1_codec_command_t c;
        check(E_OK == s1_net_command_receive(&c, TMO_POL) && c.command_id == S1_CMD_EMERGENCY_STOP,
              "M8 and is delivered on the command queue (maps use their own buffer)");
    }
    check(s1_net_hook_pull_telemetry(big, sizeof big) == 0u, "M8 no telemetry produced by map traffic");

    /* M9: map traffic does not disturb the controller's own replies. */
    r = reply(1u, true, 0u);
    check(!s1_map_on_reply(&r), "M9 controller reply id 1 never matches a map request");

    printf("map: %d checks, %d failed\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}