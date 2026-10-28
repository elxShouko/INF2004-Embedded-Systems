/* Host (PC) tests of the MQTT client state machine (mqtt_utk.c) with a mocked
 * lwIP and a fake clock. Build and run with run_host_tests.sh. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_utk.h"
#include "lwip/apps/mqtt.h"
#include "tm/tmonitor.h"

/* ---- fake clock and mock lwIP --------------------------------------- */
static uint32_t g_now_ms = 0u;
uint32_t sys_now(void) { return g_now_ms; }
int tm_putstring(const UB *s)
{
    (void)s;
    return 0;
}
int ip4addr_aton(const char *s, ip4_addr_t *a)
{
    a->addr = (uint32_t)strlen(s);
    return 1;
}

static mqtt_client_t g_client;
static mqtt_connection_cb_t g_conn_cb;
static mqtt_request_cb_t g_sub_cb;
static mqtt_incoming_publish_cb_t g_in_pub;
static mqtt_incoming_data_cb_t g_in_data;
static int g_connect_rc = ERR_OK, g_publish_rc = ERR_OK, g_sub_rc = ERR_OK;
static int g_connect_calls = 0, g_disconnects = 0;
static uint32_t g_connect_times[64];
static uint32_t g_tel_times[8192];
static int g_tel_count = 0;
static uint32_t g_hb_times[512];
static int g_hb_count = 0;
static char g_last_payload[300];

mqtt_client_t *mqtt_client_new(void)
{
    g_client.connected = 0;
    return &g_client;
}
err_t mqtt_client_connect(mqtt_client_t *c, const ip_addr_t *ip, u16_t port, mqtt_connection_cb_t cb, void *arg, const struct mqtt_connect_client_info_t *ci)
{
    (void)ip;
    (void)port;
    (void)arg;
    (void)ci;
    if (g_connect_calls < 64)
    {
        g_connect_times[g_connect_calls] = g_now_ms;
    }
    g_connect_calls++;
    if (g_connect_rc != ERR_OK)
    {
        return (err_t)g_connect_rc;
    }
    g_conn_cb = cb;
    c->connected = 1;
    return ERR_OK;
}
void mqtt_disconnect(mqtt_client_t *c)
{
    c->connected = 0;
    g_disconnects++;
}
u8_t mqtt_client_is_connected(mqtt_client_t *c) { return (u8_t)c->connected; }
err_t mqtt_publish(mqtt_client_t *c, const char *t, const void *p, u16_t len, u8_t q, u8_t r, mqtt_request_cb_t cb, void *arg)
{
    (void)c;
    (void)q;
    (void)r;
    (void)cb;
    (void)arg;
    if (g_publish_rc != ERR_OK)
    {
        return (err_t)g_publish_rc;
    }
    memcpy(g_last_payload, p, len);
    g_last_payload[len] = 0;
    if (0 == strcmp(t, "robot/telemetry"))
    {
        if (g_tel_count < 8192)
        {
            g_tel_times[g_tel_count] = g_now_ms;
        }
        g_tel_count++;
    }
    else if (0 == strcmp(t, "robot/status"))
    {
        if (g_hb_count < 512)
        {
            g_hb_times[g_hb_count] = g_now_ms;
        }
        g_hb_count++;
    }
    return ERR_OK;
}
err_t mqtt_sub_unsub(mqtt_client_t *c, const char *t, u8_t q, mqtt_request_cb_t cb, void *arg, u8_t s)
{
    (void)c;
    (void)t;
    (void)q;
    (void)arg;
    (void)s;
    g_sub_cb = cb;
    return (err_t)g_sub_rc;
}
void mqtt_set_inpub_callback(mqtt_client_t *c, mqtt_incoming_publish_cb_t p, mqtt_incoming_data_cb_t d, void *a)
{
    (void)c;
    (void)a;
    g_in_pub = p;
    g_in_data = d;
}

/* ---- the application hooks (what subsystem 1 implements) ------------- */
static int g_frames_waiting = 0, g_events[8], g_commands = 0, g_event_value = 0;
static char g_command_text[64];
static uint32_t g_command_rx;
static uint32_t hook_pull(char *b, uint32_t cap)
{
    (void)cap;
    if (g_frames_waiting > 0)
    {
        g_frames_waiting--;
        strcpy(b, "{\"f\":1}");
        return 7u;
    }
    return 0u;
}
static uint32_t hook_heartbeat(char *b, uint32_t cap, uint32_t up)
{
    (void)cap;
    strcpy(b, "{\"status\":\"ALIVE\"}");
    (void)up;
    return 17u;
}
static void hook_command(const char *p, uint32_t n, uint32_t rx)
{
    memcpy(g_command_text, p, n);
    g_command_text[n] = 0;
    g_command_rx = rx;
    g_commands++;
}
static void hook_event(mqtt_utk_event_t e, uint32_t v)
{
    g_events[e]++;
    if (MQTT_UTK_EVENT_BACKOFF == e)
    {
        g_event_value = (int)v;
    }
}
static const mqtt_utk_app_t g_app = {hook_pull, hook_heartbeat, hook_command, hook_event};

/* ---- helpers ---------------------------------------------------------- */
static int g_checks = 0, g_fails = 0;
#define CHECK(c, ...)                             \
    do                                            \
    {                                             \
        g_checks++;                               \
        if (!(c))                                 \
        {                                         \
            g_fails++;                            \
            printf("  FAIL line %d: ", __LINE__); \
            printf(__VA_ARGS__);                  \
            printf("\n");                         \
        }                                         \
    } while (0)
static void step(uint32_t ms)
{
    g_now_ms += ms;
    mqtt_utk_poll(1);
}
static void run(uint32_t total, uint32_t each)
{
    uint32_t t;
    for (t = 0u; t < total; t += each)
    {
        step(each);
    }
}
static void reset(void)
{
    memset(g_events, 0, sizeof(g_events));
    g_connect_rc = ERR_OK;
    g_publish_rc = ERR_OK;
    g_sub_rc = ERR_OK;
    g_connect_calls = 0;
    g_disconnects = 0;
    g_tel_count = 0;
    g_hb_count = 0;
    g_commands = 0;
    g_frames_waiting = 0;
}
/* Bring the client to CONNECTED from whatever state the last test left it in:
 * force any open session down, wait for the next real connect attempt, then
 * let the broker accept it. */
static void bring_up(void)
{
    uint32_t waited = 0u;

    reset();
    if (NULL != g_conn_cb)
    {
        g_conn_cb(&g_client, NULL, MQTT_CONNECT_DISCONNECTED);
    }
    while ((0 == g_connect_calls) && (waited < 30000u))
    {
        step(10);
        waited += 10u;
    }
    g_conn_cb(&g_client, NULL, MQTT_CONNECT_ACCEPTED);
    step(10);
}

static void send_command(const char *topic, const char *text, int fragmented)
{
    size_t n = strlen(text);
    g_in_pub(NULL, topic, (u32_t)n);
    if (fragmented && n > 4u)
    {
        g_in_data(NULL, (const u8_t *)text, 4, 0);
        g_in_data(NULL, (const u8_t *)text + 4, (u16_t)(n - 4u), MQTT_DATA_FLAG_LAST);
    }
    else
    {
        g_in_data(NULL, (const u8_t *)text, (u16_t)n, MQTT_DATA_FLAG_LAST);
    }
}

int main(void)
{
    mqtt_utk_set_app(&g_app);

    printf("T0 broker never answers from boot: retries follow 1,2,4 s (FR19)\n");
    reset();
    g_connect_rc = ERR_CONN;
    g_conn_cb = NULL;
    mqtt_utk_poll(1);
    g_now_ms += 10;
    mqtt_utk_poll(1);
    run(20000u, 10u);
    {
        const uint32_t want[] = {1000u, 2000u, 4000u};
        int k;
        CHECK(g_connect_calls >= 4, "attempts before any success: %d", g_connect_calls);
        for (k = 1; k <= 3 && k < g_connect_calls; k++)
        {
            int gap = (int)(g_connect_times[k] - g_connect_times[k - 1]);
            printf("  gap %d = %d ms (expected %u)\n", k, gap, want[k - 1]);
            CHECK(abs(gap - (int)want[k - 1]) <= 20, "first-boot gap %d is %d", k, gap);
        }
    }
    g_connect_rc = ERR_OK;

    printf("T1 connect, subscribe, CONNECTED event\n");
    bring_up();
    CHECK(g_connect_calls == 1, "one connect call");
    CHECK(g_events[MQTT_UTK_EVENT_CONNECTED] == 1, "CONNECTED event");

    printf("T2 backoff 1,2,4,8,8 s while the broker refuses, then reset after success (FR19)\n");
    reset();
    g_connect_rc = ERR_CONN;
    g_conn_cb = NULL;
    mqtt_utk_poll(0);
    g_now_ms += 10;
    mqtt_utk_poll(0); /* make sure we are in NO_LINK */
    run(60000u, 10u);
    {
        const uint32_t want[] = {1000u, 2000u, 4000u, 8000u, 8000u};
        int k;
        CHECK(g_connect_calls >= 6, "attempts: %d", g_connect_calls);
        for (k = 1; k <= 5 && k < g_connect_calls; k++)
        {
            int gap = (int)(g_connect_times[k] - g_connect_times[k - 1]);
            printf("  gap %d = %d ms (expected %u)\n", k, gap, want[k - 1]);
            CHECK(abs(gap - (int)want[k - 1]) <= 20, "gap %d is %d", k, gap);
        }
    }
    g_connect_rc = ERR_OK;
    {
        int before = g_connect_calls;
        uint32_t waited = 0u;
        while ((g_connect_calls == before) && (waited < 20000u))
        {
            step(10);
            waited += 10u;
        }
    }
    g_conn_cb(&g_client, NULL, MQTT_CONNECT_ACCEPTED);
    step(10);
    CHECK(g_events[MQTT_UTK_EVENT_CONNECTED] >= 1, "connected after the broker returns");
    g_events[MQTT_UTK_EVENT_BACKOFF] = 0;
    g_conn_cb(&g_client, NULL, MQTT_CONNECT_DISCONNECTED);
    step(10);
    CHECK(g_event_value == 1000, "first backoff after a success is 1000 ms (was %d)", g_event_value);

    printf("T3 connect timeout (no answer) after 5 s\n");
    bring_up();
    g_conn_cb(&g_client, NULL, MQTT_CONNECT_DISCONNECTED);
    step(10);
    {
        int before = g_connect_calls;
        uint32_t waited = 0u; /* wait for the retry that gets no answer */
        while ((g_connect_calls == before) && (waited < 20000u))
        {
            step(10);
            waited += 10u;
        }
    }
    g_events[MQTT_UTK_EVENT_BACKOFF] = 0;
    run(4900u, 10u);
    CHECK(g_events[MQTT_UTK_EVENT_BACKOFF] == 0, "no backoff before 5 s");
    run(300u, 10u);
    CHECK(g_events[MQTT_UTK_EVENT_BACKOFF] == 1, "timeout -> backoff");

    printf("T4 subscribe failure -> backoff\n");
    bring_up();
    g_sub_cb(NULL, ERR_CONN);
    step(10);
    CHECK(g_events[MQTT_UTK_EVENT_BACKOFF] >= 1, "subscribe failed -> backoff");

    printf("T5 dead link from publish failures: 10 in a row yes, a 4-frame stall no\n");
    bring_up();
    g_frames_waiting = 100000;
    g_publish_rc = ERR_MEM;
    run(300u, 10u);
    g_publish_rc = ERR_OK;
    run(300u, 10u);
    CHECK(g_events[MQTT_UTK_EVENT_BACKOFF] == 0, "short stall tolerated");
    g_publish_rc = ERR_MEM;
    run(1300u, 10u);
    CHECK(g_events[MQTT_UTK_EVENT_BACKOFF] >= 1, "sustained failure -> backoff");
    CHECK(g_events[MQTT_UTK_EVENT_PUBLISH_FAILED] >= 10, "failures reported");

    printf("T6 Wi-Fi link loss: no backoff, discards frames, reconnects at once when the link returns\n");
    bring_up();
    g_frames_waiting = 1000;
    mqtt_utk_poll(0);
    g_events[MQTT_UTK_EVENT_TELEMETRY_DISCARDED] = 0;
    {
        int k;
        for (k = 0; k < 100; k++)
        {
            g_now_ms += 10u;
            mqtt_utk_poll(0);
        }
    }
    CHECK(g_events[MQTT_UTK_EVENT_DISCONNECTED] >= 1, "DISCONNECTED reported");
    CHECK(g_events[MQTT_UTK_EVENT_TELEMETRY_DISCARDED] >= 8 && g_events[MQTT_UTK_EVENT_TELEMETRY_DISCARDED] <= 11, "discards about 10/s: %d", g_events[MQTT_UTK_EVENT_TELEMETRY_DISCARDED]);
    {
        int before = g_connect_calls;
        step(10);
        CHECK(g_connect_calls == before + 1, "connects immediately on link up");
    }

    printf("T7 commands: fragments, wrong topic, oversize\n");
    bring_up();
    send_command("robot/cmd", "EMERGENCY_STOP", 1);
    CHECK(g_commands == 1 && 0 == strcmp(g_command_text, "EMERGENCY_STOP"), "fragmented message reassembled");
    send_command("robot/other", "PAUSE", 0);
    CHECK(g_commands == 1, "wrong topic ignored");
    {
        char big[200];
        memset(big, 'A', 199);
        big[199] = 0;
        send_command("robot/cmd", big, 0);
        CHECK(g_commands == 1, "oversize ignored");
    }
    send_command("robot/cmd", "PAUSE", 0);
    CHECK(g_commands == 2, "next command still works");

    printf("T8 cadence: telemetry 100 +/- 10 ms, heartbeat 1000 +/- 50 ms, for several poll intervals\n");
    {
        const uint32_t steps[] = {3u, 5u, 12u, 20u};
        int s;
        for (s = 0; s < 4; s++)
        {
            int worst = 0, hb_worst = 0, k;
            bring_up();
            g_frames_waiting = 1000000;
            g_tel_count = 0;
            g_hb_count = 0;
            srand(3);
            {
                uint32_t t0 = g_now_ms;
                while (g_now_ms - t0 < 60000u)
                {
                    step(steps[s] > 3u ? steps[s] : 1u + (uint32_t)(rand() % 4));
                }
            }
            for (k = 1; k < g_tel_count && k < 8192; k++)
            {
                int e = abs((int)(g_tel_times[k] - g_tel_times[k - 1]) - 100);
                if (e > worst)
                {
                    worst = e;
                }
            }
            for (k = 1; k < g_hb_count && k < 512; k++)
            {
                int e = abs((int)(g_hb_times[k] - g_hb_times[k - 1]) - 1000);
                if (e > hb_worst)
                {
                    hb_worst = e;
                }
            }
            printf("  poll every %2u ms: telemetry frames %d, worst period error %2d ms | heartbeat worst error %2d ms\n", steps[s], g_tel_count, worst, hb_worst);
            if (steps[s] <= 5u)
            {
                CHECK(worst <= 10, "FR16 jitter %d with poll %u", worst, steps[s]);
            }
            CHECK(hb_worst <= 50, "FR20 jitter %d", hb_worst);
            CHECK(g_tel_count >= 595 && g_tel_count <= 601, "rate %d in 60 s", g_tel_count);
        }
    }

    printf("\n%d checks, %d failed\n", g_checks, g_fails);
    return (0 == g_fails) ? 0 : 1;
}