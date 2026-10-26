/**
 * @file s1_map.h
 * @brief Buddy 1: mapping-update gateway between robot/cmd and Buddy 3.
 *
 * Flow (Tribunal change, Buddy 1 row):
 *   dashboard "MAP A=FORWARD B=LEFT"  (text, up to S1_NET_MAP_TEXT_MAX bytes)
 *     -> Wi-Fi task: nav_update_parse()          (s1_net.c, handle_map_update)
 *     -> s1_net_map_receive()                    (this module, controller task)
 *     -> s3_submit(S3_INPUT_MAPPING)             E_OK means QUEUED, nothing
 * more
 *     -> s3_receive() reply, matched by request_id (controller loop)
 *     -> s1_net_map_report()                     result in the next heartbeat
 *
 * RULES: Buddy 1 never builds Buddy 3's queue bytes and never reads or writes
 * its structures directly: only nav_update_parse(), s3_submit() and the reply
 * fields of s3_output_t are used. A rejected mapping is reported, it is NOT a
 * provider fault, so it must not stop the robot (see s1_map_on_reply()).
 *
 * NOTE: Both functions are called from ONE task, the Vehicle Controller loop.
 * They keep private state and take no lock.
 */

#ifndef S1_MAP_H
#define S1_MAP_H

#include <stdbool.h>
#include <stdint.h>

#include "../subsystem3/member3_navigation.h"
#include "s1_net.h"

/** Bit 31 of the S3 request_id marks a Buddy 1 mapping request. The
 *  controller's own request ids (a constant 1, and manoeuvre event ids) are
 *  small counters, so they never carry this bit. */
#define S1_MAP_REQUEST_ID_FLAG 0x80000000u

/** No reply from Buddy 3 within this time: report REJECTED (timeout). */
#define S1_MAP_REPLY_TIMEOUT_MS 1000u

/**
 * @brief Move one update forward: retry an unsent report, time out a request
 * with no reply, or take the next parsed update and submit it to Buddy 3.
 * Never blocks (TMO_POL). Call once per controller cycle.
 * @param now_ms Milliseconds, any monotonic wrapping clock.
 */
void s1_map_pump(uint32_t now_ms);

/**
 * @brief Offer a reply taken from s3_receive().
 * @return true if it answers the outstanding mapping request (the result is
 * recorded); the caller must then NOT treat accepted == 0 as a provider
 * fault. false for every other reply, which the caller handles as before.
 */
bool s1_map_on_reply(const s3_output_t *p_reply);

#endif /* S1_MAP_H */

/*** end of file ***/