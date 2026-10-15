/**
 * @file s1_net.c
 * @brief Buddy 1: kernel objects and API between the application and the
 * Wi-Fi/MQTT service task.
 *
 * ASSUMPTION: The hooks run only in the Wi-Fi service task, so the counters
 * have a single writer. Other tasks read them with plain 32-bit loads, which
 * are atomic on this core. A snapshot may mix values from two moments, which
 * is acceptable for diagnostics.
 */

#include "s1_net.h"

#if TM_MQTT
#include "mqtt_utk.h"
#endif

#define S1_NET_TELEMETRY_DEPTH 16u
#define S1_NET_COMMAND_DEPTH 8u
#define S1_NET_MESSAGE_OVERHEAD 8u /* Kernel bookkeeping per message. */
#define S1_NET_FLUSH_LIMIT 32u
#define S1_NET_SCRATCH_BYTES 8u

typedef struct
{
    ID             telemetry_buffer_id;
    ID             command_buffer_id;
    ID             event_flag_id;
    uint32_t       telemetry_sequence;
    s1_net_stats_t stats;
} s1_net_module_t;

static s1_net_module_t g_s1_net = {0};

#if TM_MQTT
static void on_mqtt_event (mqtt_utk_event_t event, uint32_t value);

static const mqtt_utk_app_t g_s1_net_app = {
    .pull_telemetry = s1_net_hook_pull_telemetry,
    .make_heartbeat = s1_net_hook_make_heartbeat,
    .on_command     = s1_net_hook_on_command,
    .on_event       = on_mqtt_event};
#endif

static void     queue_command (const s1_codec_command_t * p_command);
static void     flush_telemetry (void);
static uint32_t now_ms (void);

/* ---------------------------------------------------------------------- */
/* Public functions: used by any task                                     */
/* ---------------------------------------------------------------------- */

/* Create the two message buffers and the event flag. */
ER
s1_net_init (void)
{
    T_CMBF telemetry_info = {0};
    T_CMBF command_info   = {0};
    T_CFLG flag_info      = {0};
    ER     result         = E_OK;

    telemetry_info.mbfatr = TA_TFIFO;
    telemetry_info.bufsz  = S1_NET_TELEMETRY_DEPTH *
                           (sizeof(s1_codec_frame_t) + S1_NET_MESSAGE_OVERHEAD);
    telemetry_info.maxmsz        = sizeof(s1_codec_frame_t);
    g_s1_net.telemetry_buffer_id = tk_cre_mbf(&telemetry_info);

    command_info.mbfatr = TA_TFIFO;
    command_info.bufsz  = S1_NET_COMMAND_DEPTH *
                         (sizeof(s1_codec_command_t) + S1_NET_MESSAGE_OVERHEAD);
    command_info.maxmsz        = sizeof(s1_codec_command_t);
    g_s1_net.command_buffer_id = tk_cre_mbf(&command_info);

    flag_info.flgatr       = TA_TFIFO | TA_WMUL;
    g_s1_net.event_flag_id = tk_cre_flg(&flag_info);

    if (g_s1_net.telemetry_buffer_id < E_OK)
    {
        result = g_s1_net.telemetry_buffer_id;
    }
    else if (g_s1_net.command_buffer_id < E_OK)
    {
        result = g_s1_net.command_buffer_id;
    }
    else if (g_s1_net.event_flag_id < E_OK)
    {
        result = g_s1_net.event_flag_id;
    }
    else
    {
        result = E_OK;
    }
    return result;
}

#if TM_MQTT
/* Register the hooks with the MQTT client. */
ER
s1_net_start (void)
{
    mqtt_utk_set_app(&g_s1_net_app);
    return E_OK;
}
#else
/* No MQTT in this build: there is nothing to hook up. */
ER
s1_net_start (void)
{
    return E_OK;
}
#endif

/* Queue a telemetry frame without blocking. */
ER
s1_net_telemetry_send (const s1_codec_frame_t * p_frame)
{
    return tk_snd_mbf(g_s1_net.telemetry_buffer_id, p_frame,
                      sizeof(s1_codec_frame_t), TMO_POL);
}

/* Take the oldest remote command. */
ER
s1_net_command_receive (s1_codec_command_t * p_command, TMO timeout)
{
    INT size   = tk_rcv_mbf(g_s1_net.command_buffer_id, p_command, timeout);
    ER  result = E_OK;

    if (size < E_OK)
    {
        result = size;
    }
    return result;
}

/* Wait for event flag bits and clear the ones that matched. */
ER
s1_net_events_wait (UINT pattern, UINT * p_flags, TMO timeout)
{
    return tk_wai_flg(g_s1_net.event_flag_id, pattern, TWF_ORW | TWF_BITCLR,
                      p_flags, timeout);
}

/* True while the MQTT session is open. */
bool
s1_net_is_connected (void)
{
    return g_s1_net.stats.b_connected;
}

/* Copy the diagnostic counters. */
void
s1_net_get_stats (s1_net_stats_t * p_stats)
{
    *p_stats = g_s1_net.stats;
}

/* ---------------------------------------------------------------------- */
/* Public functions: hooks, called from the Wi-Fi service task            */
/* ---------------------------------------------------------------------- */

/* Take one frame from the buffer and turn it into JSON. */
uint32_t
s1_net_hook_pull_telemetry (char * p_buffer, uint32_t capacity)
{
    s1_codec_frame_t frame  = {0};
    uint32_t         length = 0u;
    INT size = tk_rcv_mbf(g_s1_net.telemetry_buffer_id, &frame, TMO_POL);

    if (size > 0)
    {
        g_s1_net.stats.telemetry_pulled++;
        /* The sequence counts every frame that leaves the buffer, so a drop
         * shows up as a gap in the numbers the laptop sees. */
        length = s1_codec_encode_telemetry(&frame, g_s1_net.telemetry_sequence,
                                           p_buffer, capacity);
        g_s1_net.telemetry_sequence++;
        if (0u == length)
        {
            g_s1_net.stats.telemetry_oversize++;
        }
    }
    return length;
}

/* Build the heartbeat JSON (FR20). */
uint32_t
s1_net_hook_make_heartbeat (char * p_buffer, uint32_t capacity,
                            uint32_t uptime_ms)
{
    return s1_codec_encode_heartbeat(uptime_ms, p_buffer, capacity);
}

/* Validate one complete command message and forward it (FR18). */
void
s1_net_hook_on_command (const char * p_payload, uint32_t length, uint32_t rx_ms)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};

    g_s1_net.stats.commands_received++;
    command.command_id      = s1_codec_parse_command(p_payload, length);
    command.rx_timestamp_ms = rx_ms;

    if (S1_CMD_NONE == command.command_id)
    {
        g_s1_net.stats.commands_rejected++;
    }
    else
    {
        queue_command(&command);
    }
}

/* Count the connection events and keep the buffer fresh. */
void
s1_net_hook_on_event (uint32_t event, uint32_t value)
{
    if (S1_NET_EVENT_CONNECTED == event)
    {
        g_s1_net.stats.connects++;
        g_s1_net.stats.b_connected = true;
        flush_telemetry();
        tk_set_flg(g_s1_net.event_flag_id, S1_NET_FLG_LINK_CHANGED);
    }
    else if (S1_NET_EVENT_DISCONNECTED == event)
    {
        g_s1_net.stats.disconnects++;
        g_s1_net.stats.b_connected = false;
        tk_set_flg(g_s1_net.event_flag_id, S1_NET_FLG_LINK_CHANGED);
    }
    else if (S1_NET_EVENT_BACKOFF == event)
    {
        g_s1_net.stats.backoff_count++;
        g_s1_net.stats.backoff_last_ms = value;
        g_s1_net.stats.b_connected     = false;
    }
    else if (S1_NET_EVENT_PUBLISH_OK == event)
    {
        g_s1_net.stats.telemetry_published++;
    }
    else if (S1_NET_EVENT_PUBLISH_FAILED == event)
    {
        g_s1_net.stats.telemetry_failed++;
    }
    else if (S1_NET_EVENT_TELEMETRY_DISCARDED == event)
    {
        g_s1_net.stats.telemetry_discarded++;
    }
    else
    {
        /* Unknown event: ignore it. */
    }
}

/* ---------------------------------------------------------------------- */
/* Private functions                                                      */
/* ---------------------------------------------------------------------- */

/* Milliseconds since boot, the same clock lwIP uses. */
static uint32_t
now_ms (void)
{
    SYSTIM time = {0};

    tk_get_otm(&time);
    return time.lo;
}

/* Put a command in the buffer and wake the Vehicle Controller. An
 * EMERGENCY_STOP is never refused: if the buffer is full the oldest command
 * is discarded to make room. Any other command is dropped when it is full. */
static void
queue_command (const s1_codec_command_t * p_command)
{
    uint8_t  scratch[S1_NET_SCRATCH_BYTES] = {0};
    uint32_t forward_ms                    = 0u;
    UINT     flags                         = S1_NET_FLG_COMMAND;
    ER       result = tk_snd_mbf(g_s1_net.command_buffer_id, p_command,
                                 sizeof(s1_codec_command_t), TMO_POL);

    if ((E_OK != result) && (S1_CMD_EMERGENCY_STOP == p_command->command_id))
    {
        g_s1_net.stats.commands_dropped++;
        tk_rcv_mbf(g_s1_net.command_buffer_id, scratch, TMO_POL);
        result = tk_snd_mbf(g_s1_net.command_buffer_id, p_command,
                            sizeof(s1_codec_command_t), TMO_POL);
    }
    else if (E_OK != result)
    {
        g_s1_net.stats.commands_dropped++;
    }
    else
    {
        /* Queued on the first attempt. */
    }

    if (E_OK == result)
    {
        forward_ms = now_ms() - p_command->rx_timestamp_ms;
        if (forward_ms > g_s1_net.stats.command_forward_max_ms)
        {
            g_s1_net.stats.command_forward_max_ms = forward_ms;
        }
        if (S1_CMD_EMERGENCY_STOP == p_command->command_id)
        {
            flags |= S1_NET_FLG_ESTOP;
        }
        tk_set_flg(g_s1_net.event_flag_id, flags);
    }
}

/* Drop every stale frame so the stream restarts with live data. */
static void
flush_telemetry (void)
{
    s1_codec_frame_t frame = {0};
    uint32_t         count = 0u;
    INT              size  = 1;

    while ((size > 0) && (count < S1_NET_FLUSH_LIMIT))
    {
        size = tk_rcv_mbf(g_s1_net.telemetry_buffer_id, &frame, TMO_POL);
        if (size > 0)
        {
            g_s1_net.stats.telemetry_flushed++;
        }
        count++;
    }
}

#if TM_MQTT
/* Translate a library event into an application event. */
static void
on_mqtt_event (mqtt_utk_event_t event, uint32_t value)
{
    uint32_t s1_event = S1_NET_EVENT_NONE;

    if (MQTT_UTK_EVENT_CONNECTED == event)
    {
        s1_event = S1_NET_EVENT_CONNECTED;
    }
    else if (MQTT_UTK_EVENT_DISCONNECTED == event)
    {
        s1_event = S1_NET_EVENT_DISCONNECTED;
    }
    else if (MQTT_UTK_EVENT_BACKOFF == event)
    {
        s1_event = S1_NET_EVENT_BACKOFF;
    }
    else if (MQTT_UTK_EVENT_PUBLISH_OK == event)
    {
        s1_event = S1_NET_EVENT_PUBLISH_OK;
    }
    else if (MQTT_UTK_EVENT_PUBLISH_FAILED == event)
    {
        s1_event = S1_NET_EVENT_PUBLISH_FAILED;
    }
    else if (MQTT_UTK_EVENT_TELEMETRY_DISCARDED == event)
    {
        s1_event = S1_NET_EVENT_TELEMETRY_DISCARDED;
    }
    else
    {
        s1_event = S1_NET_EVENT_NONE;
    }
    s1_net_hook_on_event(s1_event, value);
}
#endif

/*** end of file ***/
