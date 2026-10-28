/**
 * @file mqtt_utk.c
 * @brief MQTT client for micro T-Kernel (RP2040, lwIP NO_SYS=1).
 *
 * Design rules (Section 6.2 statechart of the design review):
 *   - Nothing here blocks. Every wait is a timestamp compare.
 *   - lwIP callbacks only record events. All state changes happen in
 *     mqtt_utk_poll(), so there is one place to reason about them.
 *   - Reconnection uses binary exponential backoff: 1 s, 2 s, 4 s, 8 s, 8 s.
 *     The delay returns to 1 s after a successful connection (FR19).
 *   - Telemetry goes out every 100 ms and the heartbeat every second, on
 *     fixed-rate schedules that do not drift (FR16, FR20).
 *
 * ASSUMPTION: lwIP callbacks run inside sys_check_timeouts() and the
 * cyw43 poll, both called only from cyw43_utk_task(). They therefore never
 * run concurrently with mqtt_utk_poll().
 */

#include "mqtt_utk.h"

#include <stddef.h>
#include <tm/tmonitor.h>

#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/sys.h"
#include "net_backoff.h"
#include "wifi_secrets.h"

/* ---------------------------------------------------------------------- */
/* Constants                                                              */
/* ---------------------------------------------------------------------- */

#ifndef MQTT_UTK_TOPIC_TELEMETRY
#define MQTT_UTK_TOPIC_TELEMETRY "robot/telemetry"
#endif
#ifndef MQTT_UTK_TOPIC_STATUS
#define MQTT_UTK_TOPIC_STATUS "robot/status"
#endif
#ifndef MQTT_UTK_TOPIC_COMMAND
#define MQTT_UTK_TOPIC_COMMAND "robot/cmd"
#endif
#define MQTT_UTK_WILL_MESSAGE "{\"status\":\"OFFLINE\"}"

#define MQTT_UTK_TELEMETRY_PERIOD_MS 100u  /* FR16: 100 ms +/- 10 ms. */
#define MQTT_UTK_HEARTBEAT_PERIOD_MS 1000u /* FR20: 1000 ms +/- 50 ms. */
#define MQTT_UTK_HEARTBEAT_CAPACITY 48u
#define MQTT_UTK_CONNECT_TIMEOUT_MS 5000u /* lwIP's own default is 100 s. */
#define MQTT_UTK_KEEPALIVE_SECONDS 5u     /* A dead broker is seen in ~8 s. */
#define MQTT_UTK_QOS_AT_MOST_ONCE 0u
#define MQTT_UTK_QOS_AT_LEAST_ONCE 1u
#define MQTT_UTK_NO_RETAIN 0u
#define MQTT_UTK_SUBSCRIBE 1u
#define MQTT_UTK_PUBLISH_FAIL_LIMIT 10u /* 10 failed frames in a row = 1 s. */
#define MQTT_UTK_JSON_CAPACITY 256u
#define MQTT_UTK_RX_TOPIC_MAX 32u
#define MQTT_UTK_RX_DATA_MAX 48u
#define MQTT_UTK_DATA_FLAG_LAST 1u /* Checked against lwIP below. */
#define MQTT_UTK_DECIMAL_BASE 10u
#define MQTT_UTK_NUMBER_DIGITS 11u

/* Compile-time check that our flag value matches lwIP's. The array size is
 * negative, and the build fails, if the two ever differ. lwIP declares the
 * constant as an enum, so the preprocessor cannot be used here. */
typedef char mqtt_utk_flag_check_t[(MQTT_DATA_FLAG_LAST == 1) ? 1 : -1];

/* Event flags set by lwIP callbacks and consumed in mqtt_utk_poll(). */
#define MQTT_UTK_FLAG_UP 0x01u
#define MQTT_UTK_FLAG_DOWN 0x02u
#define MQTT_UTK_FLAG_SUBSCRIBE_FAILED 0x04u

/* ---------------------------------------------------------------------- */
/* Types                                                                  */
/* ---------------------------------------------------------------------- */

typedef enum
{
    MQTT_UTK_STATE_NO_LINK = 0,
    MQTT_UTK_STATE_CONNECTING,
    MQTT_UTK_STATE_CONNECTED,
    MQTT_UTK_STATE_BACKOFF
} mqtt_utk_state_t;

typedef struct
{
    mqtt_client_t *p_client;
    const mqtt_utk_app_t *p_app;
    mqtt_utk_state_t state;
    bool b_rx_accepted;
    uint8_t pending_flags;
    uint16_t rx_length;
    uint32_t backoff_delay_ms;
    uint32_t state_entry_ms;
    uint32_t next_telemetry_ms;
    uint32_t next_heartbeat_ms;
    uint32_t publish_fail_streak;
    uint32_t rx_arrival_ms;
    char rx_data[MQTT_UTK_RX_DATA_MAX];
} mqtt_utk_module_t;

/* ---------------------------------------------------------------------- */
/* File-scope data                                                        */
/* ---------------------------------------------------------------------- */

static mqtt_utk_module_t g_mqtt = {.backoff_delay_ms = NET_BACKOFF_INITIAL_MS};

/* Last Will: the broker announces the loss even if we cannot (FR20). */
static const struct mqtt_connect_client_info_t g_mqtt_client_info = {
    .client_id = MQTT_CLIENT_ID,
    .client_user = NULL,
    .client_pass = NULL,
    .keep_alive = MQTT_UTK_KEEPALIVE_SECONDS,
    .will_topic = MQTT_UTK_TOPIC_STATUS,
    .will_msg = MQTT_UTK_WILL_MESSAGE,
    .will_qos = MQTT_UTK_QOS_AT_LEAST_ONCE,
    .will_retain = MQTT_UTK_NO_RETAIN};

/* ---------------------------------------------------------------------- */
/* Private function prototypes                                            */
/* ---------------------------------------------------------------------- */

static void log_text(const char *p_text);
static void log_number(uint32_t value);
static void report_event(mqtt_utk_event_t event, uint32_t value);
static bool text_equal(const char *p_first, const char *p_second);
static bool period_elapsed(uint32_t now_ms, uint32_t *p_next_due_ms,
                           uint32_t period_ms);
static void enter_state(mqtt_utk_state_t state, uint32_t now_ms);
static void enter_backoff(uint32_t now_ms, const char *p_reason);
static void start_connect(uint32_t now_ms);
static void handle_events(uint32_t now_ms);
static void on_connected(uint32_t now_ms);
static void poll_without_link(uint32_t now_ms);
static void poll_connecting(uint32_t now_ms);
static void poll_connected(uint32_t now_ms);
static void poll_backoff(uint32_t now_ms);
static void publish_telemetry_tick(uint32_t now_ms);
static void publish_heartbeat(uint32_t now_ms);
static void discard_telemetry_tick(uint32_t now_ms);
static void on_publish_result(void *p_arg, err_t result);
static void on_subscribe_result(void *p_arg, err_t result);
static void on_incoming_publish(void *p_arg, const char *p_topic,
                                u32_t total_length);
static void on_incoming_data(void *p_arg, const u8_t *p_data, u16_t length,
                             u8_t flags);
static void on_connection_status(mqtt_client_t *p_client, void *p_arg,
                                 mqtt_connection_status_t status);

/* ---------------------------------------------------------------------- */
/* Public functions                                                       */
/* ---------------------------------------------------------------------- */

/* Register the application hooks. */
void mqtt_utk_set_app(const mqtt_utk_app_t *p_app)
{
    g_mqtt.p_app = p_app;
}

/* Run one step of the MQTT state machine. Called on every poll iteration of
 * cyw43_utk_task(). */
void mqtt_utk_poll(bool b_link_up)
{
    uint32_t now_ms = sys_now();

    if (NULL != g_mqtt.p_app)
    {
        handle_events(now_ms);

        if (!b_link_up)
        {
            poll_without_link(now_ms);
        }
        else if (MQTT_UTK_STATE_NO_LINK == g_mqtt.state)
        {
            start_connect(now_ms);
        }
        else if (MQTT_UTK_STATE_CONNECTING == g_mqtt.state)
        {
            poll_connecting(now_ms);
        }
        else if (MQTT_UTK_STATE_CONNECTED == g_mqtt.state)
        {
            poll_connected(now_ms);
        }
        else if (MQTT_UTK_STATE_BACKOFF == g_mqtt.state)
        {
            poll_backoff(now_ms);
        }
        else
        {
            enter_state(MQTT_UTK_STATE_NO_LINK, now_ms);
        }

        if (MQTT_UTK_STATE_CONNECTED != g_mqtt.state)
        {
            discard_telemetry_tick(now_ms);
        }
    }
}

/* ---------------------------------------------------------------------- */
/* Private functions: utilities                                           */
/* ---------------------------------------------------------------------- */

/* Print a string on the T-Monitor console. */
static void
log_text(const char *p_text)
{
    /* Cast is safe: tm_putstring only reads the text, and UB is an
     * unsigned 8-bit type with the same size as char. */
    tm_putstring((const UB *)p_text);
}

/* Print an unsigned number in decimal. */
static void
log_number(uint32_t value)
{
    char digits[MQTT_UTK_NUMBER_DIGITS] = {0};
    uint32_t position = MQTT_UTK_NUMBER_DIGITS - 1u;
    uint32_t remaining = value;

    do
    {
        position--;
        digits[position] = '0' + (remaining % MQTT_UTK_DECIMAL_BASE);
        remaining = remaining / MQTT_UTK_DECIMAL_BASE;
    } while (remaining > 0u);
    log_text(&digits[position]);
}

/* Tell the application about an event. */
static void
report_event(mqtt_utk_event_t event, uint32_t value)
{
    if (NULL != g_mqtt.p_app->on_event)
    {
        g_mqtt.p_app->on_event(event, value);
    }
}

/* Compare two NUL-terminated strings. */
static bool
text_equal(const char *p_first, const char *p_second)
{
    const char *p_left = p_first;
    const char *p_right = p_second;

    while (('\0' != *p_left) && (*p_left == *p_right))
    {
        p_left++;
        p_right++;
    }
    return (*p_left == *p_right);
}

/* Fixed-rate scheduler. The due time advances by exactly one period, so the
 * average rate is exact and there is no drift. If the caller falls more than
 * one period behind (a stall), the schedule resynchronises instead of
 * sending a burst. */
static bool
period_elapsed(uint32_t now_ms, uint32_t *p_next_due_ms, uint32_t period_ms)
{
    bool b_due = net_backoff_time_reached(now_ms, *p_next_due_ms);

    if (b_due)
    {
        *p_next_due_ms = *p_next_due_ms + period_ms;
        if (net_backoff_time_reached(now_ms, *p_next_due_ms))
        {
            *p_next_due_ms = now_ms + period_ms;
        }
    }
    return b_due;
}

/* ---------------------------------------------------------------------- */
/* Private functions: state machine                                       */
/* ---------------------------------------------------------------------- */

/* Move to a new state and remember when it was entered. */
static void
enter_state(mqtt_utk_state_t state, uint32_t now_ms)
{
    g_mqtt.state = state;
    g_mqtt.state_entry_ms = now_ms;
}

/* Close the session and wait the current backoff delay before retrying.
 * NOTE: mqtt_disconnect() does not call our callback (checked in the lwIP
 * source), so no event can arrive from this call. */
static void
enter_backoff(uint32_t now_ms, const char *p_reason)
{
    if (NULL != g_mqtt.p_client)
    {
        mqtt_disconnect(g_mqtt.p_client);
    }
    g_mqtt.publish_fail_streak = 0u;
    log_text("mqtt: ");
    log_text(p_reason);
    log_text(", backoff ms=");
    log_number(g_mqtt.backoff_delay_ms);
    log_text("\n");
    report_event(MQTT_UTK_EVENT_BACKOFF, g_mqtt.backoff_delay_ms);
    enter_state(MQTT_UTK_STATE_BACKOFF, now_ms);
}

/* Start the asynchronous connection to the broker. */
static void
start_connect(uint32_t now_ms)
{
    ip_addr_t broker_address = {0};
    err_t result = ERR_OK;

    if (NULL == g_mqtt.p_client)
    {
        g_mqtt.p_client = mqtt_client_new();
    }

    if (NULL == g_mqtt.p_client)
    {
        enter_backoff(now_ms, "mqtt_client_new failed");
    }
    else if (0 == ip4addr_aton(MQTT_BROKER_IP, ip_2_ip4(&broker_address)))
    {
        enter_backoff(now_ms, "bad MQTT_BROKER_IP");
    }
    else
    {
        IP_SET_TYPE(&broker_address, IPADDR_TYPE_V4);
        result = mqtt_client_connect(g_mqtt.p_client, &broker_address,
                                     MQTT_BROKER_PORT, on_connection_status,
                                     NULL, &g_mqtt_client_info);
        if (ERR_OK == result)
        {
            enter_state(MQTT_UTK_STATE_CONNECTING, now_ms);
        }
        else
        {
            enter_backoff(now_ms, "mqtt_client_connect refused");
        }
    }
}

/* Act on the events recorded by the lwIP callbacks. Loss events take
 * priority over the connected event. */
static void
handle_events(uint32_t now_ms)
{
    uint8_t flags = g_mqtt.pending_flags;
    bool b_session_active = ((MQTT_UTK_STATE_CONNECTING == g_mqtt.state) ||
                             (MQTT_UTK_STATE_CONNECTED == g_mqtt.state));

    g_mqtt.pending_flags = 0u;

    if ((0u != (flags & MQTT_UTK_FLAG_SUBSCRIBE_FAILED)) && b_session_active)
    {
        enter_backoff(now_ms, "subscribe failed");
    }
    else if ((0u != (flags & MQTT_UTK_FLAG_DOWN)) && b_session_active)
    {
        if (MQTT_UTK_STATE_CONNECTED == g_mqtt.state)
        {
            report_event(MQTT_UTK_EVENT_DISCONNECTED, 0u);
        }
        enter_backoff(now_ms, "broker connection lost");
    }
    else if ((0u != (flags & MQTT_UTK_FLAG_UP)) &&
             (MQTT_UTK_STATE_CONNECTING == g_mqtt.state))
    {
        on_connected(now_ms);
    }
    else
    {
        /* No event, or an event that does not apply in this state. */
    }
}

/* The broker accepted the connection: start streaming. */
static void
on_connected(uint32_t now_ms)
{
    enter_state(MQTT_UTK_STATE_CONNECTED, now_ms);
    g_mqtt.backoff_delay_ms = NET_BACKOFF_INITIAL_MS; /* Next: 1 s. */
    g_mqtt.publish_fail_streak = 0u;
    g_mqtt.next_telemetry_ms = now_ms + MQTT_UTK_TELEMETRY_PERIOD_MS;
    g_mqtt.next_heartbeat_ms = now_ms; /* First heartbeat at once. */
    log_text("mqtt: connected to " MQTT_BROKER_IP "\n");
    report_event(MQTT_UTK_EVENT_CONNECTED, 0u);
}

/* The Wi-Fi link is down: drop the session. The Wi-Fi task retries the
 * join with its own backoff, so there is no MQTT delay to apply here. */
static void
poll_without_link(uint32_t now_ms)
{
    if (MQTT_UTK_STATE_NO_LINK != g_mqtt.state)
    {
        if (MQTT_UTK_STATE_CONNECTED == g_mqtt.state)
        {
            report_event(MQTT_UTK_EVENT_DISCONNECTED, 0u);
        }
        if (NULL != g_mqtt.p_client)
        {
            mqtt_disconnect(g_mqtt.p_client);
        }
        log_text("mqtt: wifi link lost\n");
        enter_state(MQTT_UTK_STATE_NO_LINK, now_ms);
    }
}

/* CONNECTING: wait for the broker's answer, with our own timeout. */
static void
poll_connecting(uint32_t now_ms)
{
    if (net_backoff_time_elapsed(now_ms, g_mqtt.state_entry_ms,
                                 MQTT_UTK_CONNECT_TIMEOUT_MS))
    {
        enter_backoff(now_ms, "connect timeout");
    }
}

/* CONNECTED: check the session, then publish on the fixed schedules. */
static void
poll_connected(uint32_t now_ms)
{
    if (0u == mqtt_client_is_connected(g_mqtt.p_client))
    {
        report_event(MQTT_UTK_EVENT_DISCONNECTED, 0u);
        enter_backoff(now_ms, "session closed");
    }
    else
    {
        if (period_elapsed(now_ms, &g_mqtt.next_heartbeat_ms,
                           MQTT_UTK_HEARTBEAT_PERIOD_MS))
        {
            publish_heartbeat(now_ms);
        }
        if (period_elapsed(now_ms, &g_mqtt.next_telemetry_ms,
                           MQTT_UTK_TELEMETRY_PERIOD_MS))
        {
            publish_telemetry_tick(now_ms);
        }
    }
}

/* BACKOFF: wait, then double the delay up to the cap and try again. */
static void
poll_backoff(uint32_t now_ms)
{
    if (net_backoff_time_elapsed(now_ms, g_mqtt.state_entry_ms,
                                 g_mqtt.backoff_delay_ms))
    {
        g_mqtt.backoff_delay_ms = net_backoff_next_ms(g_mqtt.backoff_delay_ms);
        start_connect(now_ms);
    }
}

/* ---------------------------------------------------------------------- */
/* Private functions: publishing (FR16, FR17, FR20)                       */
/* ---------------------------------------------------------------------- */

/* Publish the telemetry frame that is due. A run of failed publishes is
 * treated as a dead link (about 1 s), well before the keepalive notices. */
static void
publish_telemetry_tick(uint32_t now_ms)
{
    char json[MQTT_UTK_JSON_CAPACITY] = {0};
    uint32_t length = g_mqtt.p_app->pull_telemetry(json, sizeof(json));
    err_t result = ERR_OK;

    if (length > 0u)
    {
        result = mqtt_publish(g_mqtt.p_client, MQTT_UTK_TOPIC_TELEMETRY, json,
                              length, MQTT_UTK_QOS_AT_MOST_ONCE,
                              MQTT_UTK_NO_RETAIN, on_publish_result, NULL);
        if (ERR_OK == result)
        {
            g_mqtt.publish_fail_streak = 0u;
            report_event(MQTT_UTK_EVENT_PUBLISH_OK, 0u);
        }
        else
        {
            g_mqtt.publish_fail_streak++;
            report_event(MQTT_UTK_EVENT_PUBLISH_FAILED,
                         g_mqtt.publish_fail_streak);
            if (g_mqtt.publish_fail_streak >= MQTT_UTK_PUBLISH_FAIL_LIMIT)
            {
                enter_backoff(now_ms, "publish failing, link presumed dead");
            }
        }
    }
}

/* Publish the heartbeat (FR20). */
static void
publish_heartbeat(uint32_t now_ms)
{
    char json[MQTT_UTK_HEARTBEAT_CAPACITY] = {0};
    uint32_t length = g_mqtt.p_app->make_heartbeat(json, sizeof(json), now_ms);

    if (length > 0u)
    {
        mqtt_publish(g_mqtt.p_client, MQTT_UTK_TOPIC_STATUS, json, length,
                     MQTT_UTK_QOS_AT_MOST_ONCE, MQTT_UTK_NO_RETAIN,
                     on_publish_result, NULL);
    }
}

/* While not connected, throw away one stale frame per period. The buffer
 * then always holds the freshest frames and memory stays bounded. */
static void
discard_telemetry_tick(uint32_t now_ms)
{
    char json[MQTT_UTK_JSON_CAPACITY] = {0};
    uint32_t length = 0u;

    if (period_elapsed(now_ms, &g_mqtt.next_telemetry_ms,
                       MQTT_UTK_TELEMETRY_PERIOD_MS))
    {
        length = g_mqtt.p_app->pull_telemetry(json, sizeof(json));
        if (length > 0u)
        {
            report_event(MQTT_UTK_EVENT_TELEMETRY_DISCARDED, 0u);
        }
    }
}

/* ---------------------------------------------------------------------- */
/* Private functions: lwIP MQTT callbacks                                 */
/* These only record events. They never block and never change the state. */
/* ---------------------------------------------------------------------- */

/* A publish finished with an error. */
static void
on_publish_result(void *p_arg, err_t result)
{
    (void)p_arg; /* Cast is safe: discards an unused callback parameter. */
    if (ERR_OK != result)
    {
        g_mqtt.pending_flags |= MQTT_UTK_FLAG_DOWN;
    }
}

/* The broker answered the subscription request. */
static void
on_subscribe_result(void *p_arg, err_t result)
{
    (void)p_arg; /* Cast is safe: discards an unused callback parameter. */
    if (ERR_OK != result)
    {
        g_mqtt.pending_flags |= MQTT_UTK_FLAG_SUBSCRIBE_FAILED;
    }
}

/* Start of an inbound message: remember its arrival time and check it. */
static void
on_incoming_publish(void *p_arg, const char *p_topic, u32_t total_length)
{
    (void)p_arg; /* Cast is safe: discards an unused callback parameter. */

    g_mqtt.rx_arrival_ms = sys_now();
    g_mqtt.rx_length = 0u;
    g_mqtt.b_rx_accepted = (text_equal(p_topic, MQTT_UTK_TOPIC_COMMAND) &&
                            (total_length < MQTT_UTK_RX_DATA_MAX));
}

/* A message can arrive in several fragments. Act only on the last one. */
static void
on_incoming_data(void *p_arg, const u8_t *p_data, u16_t length, u8_t flags)
{
    uint32_t position = 0u;

    (void)p_arg; /* Cast is safe: discards an unused callback parameter. */

    if (g_mqtt.b_rx_accepted &&
        ((g_mqtt.rx_length + length) < MQTT_UTK_RX_DATA_MAX))
    {
        for (position = 0u; position < length; position++)
        {
            g_mqtt.rx_data[g_mqtt.rx_length] = p_data[position];
            g_mqtt.rx_length++;
        }
    }
    else
    {
        g_mqtt.b_rx_accepted = false;
    }

    if (0u != (flags & MQTT_UTK_DATA_FLAG_LAST))
    {
        if (g_mqtt.b_rx_accepted && (NULL != g_mqtt.p_app->on_command))
        {
            g_mqtt.p_app->on_command(g_mqtt.rx_data, g_mqtt.rx_length,
                                     g_mqtt.rx_arrival_ms);
        }
        g_mqtt.rx_length = 0u;
        g_mqtt.b_rx_accepted = false;
    }
}

/* The connection attempt finished, or an open connection was lost. */
static void
on_connection_status(mqtt_client_t *p_client, void *p_arg,
                     mqtt_connection_status_t status)
{
    err_t result = ERR_OK;

    (void)p_arg; /* Cast is safe: discards an unused callback parameter. */

    if (MQTT_CONNECT_ACCEPTED == status)
    {
        /* Callbacks must be set before subscribing, or the first delivery
         * has nowhere to go. */
        mqtt_set_inpub_callback(p_client, on_incoming_publish, on_incoming_data,
                                NULL);
        result = mqtt_sub_unsub(p_client, MQTT_UTK_TOPIC_COMMAND,
                                MQTT_UTK_QOS_AT_LEAST_ONCE, on_subscribe_result,
                                NULL, MQTT_UTK_SUBSCRIBE);
        if (ERR_OK != result)
        {
            g_mqtt.pending_flags |= MQTT_UTK_FLAG_SUBSCRIBE_FAILED;
        }
        g_mqtt.pending_flags |= MQTT_UTK_FLAG_UP;
    }
    else
    {
        g_mqtt.pending_flags |= MQTT_UTK_FLAG_DOWN;
    }
}

/*** end of file ***/