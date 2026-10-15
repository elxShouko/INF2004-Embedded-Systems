/**
 * @file net_backoff.h
 * @brief Binary exponential backoff and wrap-safe millisecond comparisons.
 *
 * Header-only and free of any kernel, lwIP or libc dependency, so the Wi-Fi
 * rejoin (cyw43_utk.c), the MQTT client (mqtt_utk.c) and the PC unit tests
 * all use the same code (FR19: 1 s, 2 s, 4 s, 8 s, then 8 s).
 */

#ifndef NET_BACKOFF_H
#define NET_BACKOFF_H

#include <stdbool.h>
#include <stdint.h>

/** First reconnection delay (FR19). */
#define NET_BACKOFF_INITIAL_MS 1000u

/** Longest reconnection delay (FR19). */
#define NET_BACKOFF_MAX_MS 8000u

#define NET_BACKOFF_FACTOR 2u
#define NET_BACKOFF_HALF_RANGE_MS 0x80000000u

/**
 * @brief Next reconnection delay: doubled, but never beyond the cap.
 * @param current_ms The delay that was just used.
 */
static inline uint32_t
net_backoff_next_ms (uint32_t current_ms)
{
    uint32_t next_ms = NET_BACKOFF_MAX_MS;

    if (current_ms < (NET_BACKOFF_MAX_MS / NET_BACKOFF_FACTOR))
    {
        next_ms = current_ms * NET_BACKOFF_FACTOR;
    }
    return next_ms;
}

/**
 * @brief True once now_ms has reached deadline_ms. Correct across the 32-bit
 * millisecond wrap if the two times are less than 24 days apart.
 */
static inline bool
net_backoff_time_reached (uint32_t now_ms, uint32_t deadline_ms)
{
    return ((now_ms - deadline_ms) < NET_BACKOFF_HALF_RANGE_MS);
}

/** @brief True when at least duration_ms has passed since start_ms. */
static inline bool
net_backoff_time_elapsed (uint32_t now_ms, uint32_t start_ms,
                          uint32_t duration_ms)
{
    return ((now_ms - start_ms) >= duration_ms);
}

#endif /* NET_BACKOFF_H */

/*** end of file ***/
