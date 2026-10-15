/**
 * @file s1_net.h
 * @brief Buddy 1: kernel objects and API between the application and the
 * Wi-Fi/MQTT service task.
 *
 * Interfaces (TEAM.md section 5):
 *   IF-06  any task   -> Wi-Fi task  message buffer: telemetry frames
 *   IF-07  Wi-Fi task -> any task    message buffer: remote commands
 *   IF-08  Wi-Fi task -> any task    event flag: command and link events
 *
 * RULE: No application task calls cyw43, lwIP or MQTT functions (TEAM.md
 * section 3). Application tasks use only the functions in this file. They are
 * all non-blocking except s1_net_command_receive() and s1_net_events_wait(),
 * whose timeout the caller chooses.
 */

#ifndef S1_NET_H
#define S1_NET_H

#include <stdbool.h>
#include <stdint.h>

#include "../app.h"
#include "s1_codec.h"

/** Event flag: a remote command was queued (read it with
 *  s1_net_command_receive). */
#define S1_NET_FLG_COMMAND 0x01u

/** Event flag: set in addition to S1_NET_FLG_COMMAND for EMERGENCY_STOP. */
#define S1_NET_FLG_ESTOP 0x02u

/** Event flag: the MQTT link came up or went down (see s1_net_is_connected).*/
#define S1_NET_FLG_LINK_CHANGED 0x04u

/** Connection events handled by s1_net_hook_on_event(). */
#define S1_NET_EVENT_NONE 0u
#define S1_NET_EVENT_CONNECTED 1u
#define S1_NET_EVENT_DISCONNECTED 2u
#define S1_NET_EVENT_BACKOFF 3u /* value = delay in ms. */
#define S1_NET_EVENT_PUBLISH_OK 4u
#define S1_NET_EVENT_PUBLISH_FAILED 5u
#define S1_NET_EVENT_TELEMETRY_DISCARDED 6u

/** Status and diagnostic counters. Written by the Wi-Fi service task, read
 *  through s1_net_get_stats() / s1_net_is_connected() only (mutex-protected).
 */
typedef struct
{
    uint32_t telemetry_pulled;    /**< Frames taken from the buffer. */
    uint32_t telemetry_published; /**< Frames accepted by lwIP. */
    uint32_t telemetry_failed;    /**< Frames refused by lwIP. */
    uint32_t telemetry_discarded; /**< Dropped while not connected. */
    uint32_t telemetry_flushed;   /**< Stale frames dropped on reconnect. */
    uint32_t telemetry_oversize;  /**< JSON did not fit, not sent. */
    uint32_t commands_received;
    uint32_t commands_rejected; /**< Unknown or malformed payload. */
    uint32_t commands_dropped;  /**< Command buffer full. */
    uint32_t command_forward_max_ms;
    uint32_t backoff_count;
    uint32_t backoff_last_ms;
    uint32_t connects;
    uint32_t disconnects;
    bool     b_connected;
} s1_net_stats_t;

/** @brief Create the kernel objects. Called from subsystem1_init(). */
ER s1_net_init (void);

/** @brief Register the hooks with the MQTT client. Called from
 *  subsystem1_start(). Does nothing in a build without MQTT=1. */
ER s1_net_start (void);

/**
 * @brief Queue a telemetry frame for publishing. Never blocks.
 * @param p_frame Frame to copy into the buffer.
 * @return E_OK, or E_TMOUT if the buffer is full. While the broker is
 * unreachable the Wi-Fi task discards the oldest frames, so the buffer stays
 * fresh and a full buffer is rare.
 */
ER s1_net_telemetry_send (const s1_codec_frame_t * p_frame);

/**
 * @brief Take the oldest remote command.
 * @param p_command Receives the command.
 * @param timeout   TMO_POL (do not wait), TMO_FEVR, or milliseconds.
 * @return E_OK, or E_TMOUT if there was none.
 */
ER s1_net_command_receive (s1_codec_command_t * p_command, TMO timeout);

/**
 * @brief Wait for any of the event flag bits, then clear the ones that
 * matched.
 * @param pattern  S1_NET_FLG_* bits to wait for.
 * @param p_flags  Receives the bits that were set.
 * @param timeout  TMO_POL, TMO_FEVR, or milliseconds.
 * @return E_OK, or E_TMOUT.
 */
ER s1_net_events_wait (UINT pattern, UINT * p_flags, TMO timeout);

/** @brief True while the MQTT session to the broker is open. */
bool s1_net_is_connected (void);

/**
 * @brief Copy the whole status block at one moment (one coherent snapshot).
 * @param p_stats Receives the copy; zeroed if the status is unavailable.
 * @return E_OK, or E_SYS if the status mutex could not be taken.
 */
ER s1_net_get_stats (s1_net_stats_t * p_stats);

/** @brief Build the next telemetry JSON from the buffer. Returns the length,
 *  or 0 if the buffer was empty. Called by the Wi-Fi task (and by tests). */
uint32_t s1_net_hook_pull_telemetry (char * p_buffer, uint32_t capacity);

/** @brief Build the heartbeat JSON. */
uint32_t s1_net_hook_make_heartbeat (char * p_buffer, uint32_t capacity,
                                     uint32_t uptime_ms);

/** @brief Handle a complete payload from the command topic. */
void s1_net_hook_on_command (const char * p_payload, uint32_t length,
                             uint32_t rx_ms);

/** @brief Handle a connection event (an S1_NET_EVENT_* value). */
void s1_net_hook_on_event (uint32_t event, uint32_t value);

#endif /* S1_NET_H */

/*** end of file ***/
