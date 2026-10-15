/**
 * @file s1_net.c
 * @brief Buddy 1: kernel objects and API between the application and the
 * Wi-Fi/MQTT service task.
 *
 * SHARED STATUS: The status block (s1_net_stats_t) is written by the Wi-Fi
 * service task (core 1) and read by application tasks that may run on the
 * other core. Plain loads or volatile are not enough on SMP, so every access
 * goes through one brief micro T-Kernel mutex (priority inheritance). The
 * lock covers only memory copies and counter updates. No kernel waiting call,
 * lwIP call, MQTT call or print is made while it is held, and a reader gets
 * a coherent snapshot taken at a single moment.
 *
 * SAFETY PATH: The emergency-stop signal and the connected state do NOT use
 * that mutex. They live in a separate kernel event flag (state flag), which
 * is atomic across cores and never blocks. When an EMERGENCY_STOP arrives,
 * the hook parses it and posts the latch and the pulse flag first; only then
 * does it queue the command and update the counters. A full command queue or
 * a stuck diagnostic reader therefore cannot hide an emergency. Application
 * tasks never wait unboundedly on the status mutex.
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
#define S1_NET_ESTOP_POST_TRIES 3u
#define S1_NET_CLOCK_HALF_RANGE                                                \
    0x80000000u                      /* Ages beyond this are future times. */
#define S1_NET_STATE_ESTOP 0x01u     /* Latched until s1_net_estop_clear(). */
#define S1_NET_STATE_CONNECTED 0x02u /* Mirrors the connected state. */

typedef struct
{
    ID             status_mutex_id;
    ID             telemetry_buffer_id;
    ID             command_buffer_id;
    ID             event_flag_id;
    ID             state_flag_id;
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

static bool     status_lock (void);
static ER       status_lock_bounded (TMO timeout);
static void     status_unlock (void);
static void     status_count (uint32_t * p_counter);
static void     status_count_nowait (uint32_t * p_counter);
static void     post_estop (void);
static void     set_state (UINT bits, bool b_on);
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
    T_CFLG state_info     = {0};
    T_CMTX mutex_info     = {0};
    ER     result         = E_OK;

    mutex_info.mtxatr        = TA_INHERIT;
    g_s1_net.status_mutex_id = tk_cre_mtx(&mutex_info);

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

    state_info.flgatr      = TA_TFIFO | TA_WMUL;
    g_s1_net.state_flag_id = tk_cre_flg(&state_info);

    if (g_s1_net.status_mutex_id < E_OK)
    {
        result = g_s1_net.status_mutex_id;
    }
    else if (g_s1_net.telemetry_buffer_id < E_OK)
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
    else if (g_s1_net.state_flag_id < E_OK)
    {
        result = g_s1_net.state_flag_id;
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

/* Take the oldest valid remote command (see s1_net.h). */
ER
s1_net_command_receive (s1_codec_command_t * p_command, TMO timeout)
{
    s1_codec_command_t message = {S1_CMD_NONE, 0u};
    TMO                wait    = timeout;
    ER                 result  = E_TMOUT;
    bool               b_done  = false;
    uint32_t           tries   = 0u;
    uint32_t           age_ms  = 0u;
    INT                size    = 0;

    *p_command = message;
    while ((!b_done) && (tries <= S1_NET_COMMAND_DEPTH))
    {
        tries++;
        size   = tk_rcv_mbf(g_s1_net.command_buffer_id, &message, wait);
        wait   = TMO_POL; /* After a discarded message, do not wait again. */
        age_ms = now_ms() - message.rx_timestamp_ms; /* Wrap-safe. */
        if (size < E_OK)
        {
            result = size; /* Kernel error, for example E_TMOUT, unchanged. */
            b_done = true;
        }
        else if ((sizeof(message) != (uint32_t) size) ||
                 (message.command_id <= S1_CMD_NONE) ||
                 (message.command_id > S1_CMD_RESUME))
        {
            status_count_nowait(&g_s1_net.stats.commands_bad_size);
            message = (s1_codec_command_t){S1_CMD_NONE, 0u};
            result  = E_SYS; /* Never hand out a partly filled command. */
            b_done  = true;
        }
        else if ((S1_CMD_EMERGENCY_STOP != message.command_id) &&
                 (age_ms > S1_NET_COMMAND_MAX_AGE_MS) &&
                 (age_ms < S1_NET_CLOCK_HALF_RANGE))
        {
            status_count_nowait(&g_s1_net.stats.commands_stale);
            message = (s1_codec_command_t){S1_CMD_NONE, 0u};
            result  = E_TMOUT; /* Stale: look at the next one, if any. */
        }
        else
        {
            *p_command = message;
            result     = E_OK;
            b_done     = true;
        }
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

/* True while the MQTT session is open. Lock-free; false if unreadable. */
bool
s1_net_is_connected (void)
{
    T_RFLG state       = {0};
    bool   b_connected = false;

    if (E_OK == tk_ref_flg(g_s1_net.state_flag_id, &state))
    {
        b_connected = (0u != (state.flgptn & S1_NET_STATE_CONNECTED));
    }
    return b_connected;
}

/* True while an emergency is latched. Lock-free; true if unreadable. */
bool
s1_net_estop_latched (void)
{
    T_RFLG state     = {0};
    bool   b_latched = true; /* Fail safe. */

    if (E_OK == tk_ref_flg(g_s1_net.state_flag_id, &state))
    {
        b_latched = (0u != (state.flgptn & S1_NET_STATE_ESTOP));
    }
    return b_latched;
}

/* Deliberate clear by the safety-policy owner. Touches nothing else. */
ER
s1_net_estop_clear (void)
{
    return tk_clr_flg(g_s1_net.state_flag_id, ~S1_NET_STATE_ESTOP);
}

#if APP_TEST
ID
s1_net_test_status_mutex (void)
{
    return g_s1_net.status_mutex_id;
}

ID
s1_net_test_command_buffer (void)
{
    return g_s1_net.command_buffer_id;
}
#endif

/* Copy the whole status block at one moment. Bounded wait; zeroed on error. */
ER
s1_net_get_stats (s1_net_stats_t * p_stats)
{
    ER result = status_lock_bounded(S1_NET_STATUS_LOCK_MS);

    if (E_OK == result)
    {
        *p_stats = g_s1_net.stats;
        status_unlock();
    }
    else
    {
        *p_stats = (s1_net_stats_t){0};
    }
    return result;
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

    if ((size > 0) && (sizeof(frame) != (uint32_t) size))
    {
        status_count(&g_s1_net.stats.telemetry_bad_size);
    }
    else if (size > 0)
    {
        status_count(&g_s1_net.stats.telemetry_pulled);
        /* The sequence counts every frame that leaves the buffer, so a drop
         * shows up as a gap in the numbers the laptop sees. */
        length = s1_codec_encode_telemetry(&frame, g_s1_net.telemetry_sequence,
                                           p_buffer, capacity);
        g_s1_net.telemetry_sequence++;
        if (0u == length)
        {
            status_count(&g_s1_net.stats.telemetry_oversize);
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

/* Validate one complete command message and forward it (FR18). Order
 * matters: parse (pure), publish an emergency, and only then queue and count.
 * Nothing before the emergency is posted can wait on a lock or a full queue. */
void
s1_net_hook_on_command (const char * p_payload, uint32_t length, uint32_t rx_ms)
{
    s1_codec_command_t command = {S1_CMD_NONE, 0u};

    command.command_id      = s1_codec_parse_command(p_payload, length);
    command.rx_timestamp_ms = rx_ms;

    if (S1_CMD_EMERGENCY_STOP == command.command_id)
    {
        post_estop();
    }
    status_count(&g_s1_net.stats.commands_received);
    if (S1_CMD_NONE == command.command_id)
    {
        status_count(&g_s1_net.stats.commands_rejected);
    }
    else
    {
        queue_command(&command);
    }
}

/* Count the connection events and keep the buffer fresh. A change of the
 * connected state is announced once, whichever event caused it. */
void
s1_net_hook_on_event (uint32_t event, uint32_t value)
{
    bool b_changed = false;

    if (S1_NET_EVENT_CONNECTED == event)
    {
        if (status_lock())
        {
            g_s1_net.stats.connects++;
            g_s1_net.stats.b_connected = true;
            set_state(S1_NET_STATE_CONNECTED, true);
            status_unlock();
        }
        flush_telemetry();
        tk_set_flg(g_s1_net.event_flag_id, S1_NET_FLG_LINK_CHANGED);
    }
    else if ((S1_NET_EVENT_DISCONNECTED == event) ||
             (S1_NET_EVENT_BACKOFF == event))
    {
        /* BACKOFF can arrive without a DISCONNECTED before it (for example
         * when publishing keeps failing), so both leave the connected state
         * here. Only the first one is a change. */
        if (status_lock())
        {
            if (S1_NET_EVENT_BACKOFF == event)
            {
                g_s1_net.stats.backoff_count++;
                g_s1_net.stats.backoff_last_ms = value;
            }
            if (g_s1_net.stats.b_connected)
            {
                g_s1_net.stats.disconnects++;
                g_s1_net.stats.b_connected = false;
                set_state(S1_NET_STATE_CONNECTED, false);
                b_changed = true;
            }
            status_unlock();
        }
        if (b_changed)
        {
            tk_set_flg(g_s1_net.event_flag_id, S1_NET_FLG_LINK_CHANGED);
        }
    }
    else if (S1_NET_EVENT_PUBLISH_OK == event)
    {
        status_count(&g_s1_net.stats.telemetry_published);
    }
    else if (S1_NET_EVENT_PUBLISH_FAILED == event)
    {
        status_count(&g_s1_net.stats.telemetry_failed);
    }
    else if (S1_NET_EVENT_TELEMETRY_DISCARDED == event)
    {
        status_count(&g_s1_net.stats.telemetry_discarded);
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

/* Take the status mutex. It is held only for a copy or a counter update. */
static bool
status_lock (void)
{
    return (E_OK == tk_loc_mtx(g_s1_net.status_mutex_id, TMO_FEVR));
}

/* Take the status mutex for at most timeout ms (application tasks). */
static ER
status_lock_bounded (TMO timeout)
{
    return tk_loc_mtx(g_s1_net.status_mutex_id, timeout);
}

/* Release the status mutex. */
static void
status_unlock (void)
{
    tk_unl_mtx(g_s1_net.status_mutex_id);
}

/* Add one to a counter of the status block. */
static void
status_count (uint32_t * p_counter)
{
    if (status_lock())
    {
        (*p_counter)++;
        status_unlock();
    }
}

/* Add one to a counter without ever waiting: for tasks other than the Wi-Fi
 * task. A busy mutex loses the count; diagnostics never delay a caller. */
static void
status_count_nowait (uint32_t * p_counter)
{
    if (E_OK == status_lock_bounded(TMO_POL))
    {
        (*p_counter)++;
        status_unlock();
    }
}

/* Set or clear bits of the state flag. Kernel calls only, so it is also safe
 * under the status mutex. */
static void
set_state (UINT bits, bool b_on)
{
    if (b_on)
    {
        tk_set_flg(g_s1_net.state_flag_id, bits);
    }
    else
    {
        tk_clr_flg(g_s1_net.state_flag_id, ~bits);
    }
}

/* Publish an emergency through the dedicated safety path: the latch first,
 * then the pulse. Each is retried; a persistent failure is counted (without
 * waiting) and the queued command remains a second path to the consumer. */
static void
post_estop (void)
{
    bool     b_latched = false;
    bool     b_pulsed  = false;
    uint32_t tries     = 0u;

    for (tries = 0u; tries < S1_NET_ESTOP_POST_TRIES; tries++)
    {
        if (!b_latched)
        {
            b_latched = (E_OK == tk_set_flg(g_s1_net.state_flag_id,
                                            S1_NET_STATE_ESTOP));
        }
        if (!b_pulsed)
        {
            b_pulsed =
                (E_OK == tk_set_flg(g_s1_net.event_flag_id,
                                    S1_NET_FLG_COMMAND | S1_NET_FLG_ESTOP));
        }
    }
    if (!(b_latched && b_pulsed))
    {
        status_count_nowait(&g_s1_net.stats.estop_post_failures);
    }
    status_count(&g_s1_net.stats.estop_received);
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
        status_count(&g_s1_net.stats.commands_dropped);
        tk_rcv_mbf(g_s1_net.command_buffer_id, scratch, TMO_POL);
        result = tk_snd_mbf(g_s1_net.command_buffer_id, p_command,
                            sizeof(s1_codec_command_t), TMO_POL);
    }
    else if (E_OK != result)
    {
        status_count(&g_s1_net.stats.commands_dropped);
    }
    else
    {
        /* Queued on the first attempt. */
    }

    if (E_OK == result)
    {
        forward_ms = now_ms() - p_command->rx_timestamp_ms;
        if (status_lock())
        {
            if (forward_ms > g_s1_net.stats.command_forward_max_ms)
            {
                g_s1_net.stats.command_forward_max_ms = forward_ms;
            }
            status_unlock();
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
            status_count(&g_s1_net.stats.telemetry_flushed);
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
