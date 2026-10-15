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
 * non-blocking, except s1_net_command_receive() and s1_net_events_wait()
 * (the caller chooses the timeout) and s1_net_get_stats() (waits at most
 * S1_NET_STATUS_LOCK_MS). s1_net_is_connected() and s1_net_estop_latched()
 * never wait.
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

/** Event flag: pulse set the moment an EMERGENCY_STOP arrives, before the
 *  command is queued or any diagnostic lock is taken. Consumed by whoever
 *  waits on it. The lasting record is s1_net_estop_latched(). */
#define S1_NET_FLG_ESTOP 0x02u

/** A non-EMERGENCY_STOP command older than this when it is taken from the
 *  queue is discarded and counted. EMERGENCY_STOP is never discarded as stale.
 */
#define S1_NET_COMMAND_MAX_AGE_MS 1000u

/** Longest wait for the status mutex by an application task. */
#define S1_NET_STATUS_LOCK_MS 5

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
    uint32_t commands_bad_size; /**< Queued message of the wrong size or id. */
    uint32_t commands_stale;    /**< Discarded: older than the age limit. */
    uint32_t estop_received;    /**< EMERGENCY_STOP commands seen. */
    uint32_t estop_post_failures; /**< Safety flag could not be set. */
    uint32_t telemetry_bad_size;  /**< Queued frame of the wrong size. */
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
 * @brief Take the oldest valid remote command.
 *
 * The message must be exactly sizeof(s1_codec_command_t) with a known command
 * id; otherwise it is discarded and counted, and E_SYS is returned (the
 * caller's output is zeroed, never partly filled). Kernel errors such as
 * E_TMOUT are returned unchanged. A non-EMERGENCY_STOP command older than
 * S1_NET_COMMAND_MAX_AGE_MS is discarded and counted, and the next one is
 * tried without waiting again. This function never waits for the status mutex.
 * @param p_command Receives the command.
 * @param timeout   TMO_POL (do not wait), TMO_FEVR, or milliseconds.
 * @return E_OK, E_TMOUT if there was none, E_SYS for a malformed message, or
 * another kernel error.
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

/** @brief True while the MQTT session to the broker is open. Never blocks and
 *  takes no mutex. False if the state cannot be read. */
bool s1_net_is_connected (void);

/**
 * @brief True from the moment an EMERGENCY_STOP command arrives until
 * s1_net_estop_clear() is called. Nothing else clears it: not a later command
 * (RESUME, START_RUN, PAUSE), not a reconnect, not a full queue. Never blocks,
 * takes no mutex, and is SMP-safe (a kernel event flag). Fail-safe: returns
 * true if the latch cannot be read.
 *
 * Buddy 1 only DELIVERS the emergency signal. Deciding to inhibit or stop
 * motion belongs to the vehicle controller.
 */
bool s1_net_estop_latched (void);

/**
 * @brief Clear the emergency latch. Only the owner of the safety policy (the
 * vehicle controller) may call this, as a deliberate act after the robot is
 * safe. It does not touch the command queue or the event flags.
 * @return E_OK, or the kernel error.
 */
ER s1_net_estop_clear (void);

#if APP_TEST
/** @brief Self-test only: the status mutex, so a test can hold it. */
ID s1_net_test_status_mutex (void);

/** @brief Self-test only: the command message buffer, so a test can insert a
 *  malformed message. */
ID s1_net_test_command_buffer (void);
#endif

/**
 * @brief Copy the whole status block at one moment (one coherent snapshot).
 * @param p_stats Receives the copy; zeroed if the status is unavailable.
 * @return E_OK, or the kernel error (E_TMOUT if the mutex stayed busy for
 * S1_NET_STATUS_LOCK_MS). The wait is bounded; the copy is then zeroed.
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
