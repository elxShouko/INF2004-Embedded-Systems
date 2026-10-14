/** @file enc_capture.h
 * @brief Paired x4 encoder capture; caller serializes feed/snapshot/reset.
 */
#ifndef ENC_CAPTURE_H
#define ENC_CAPTURE_H
#include "motion_quad.h"
#include "motion_estimator.h"

#define ENC_CAPTURE_LEFT_MASK ((1UL << 7U) | (1UL << 28U))
#define ENC_CAPTURE_RIGHT_MASK ((1UL << 0U) | (1UL << 1U))
#define ENC_CAPTURE_MASK (ENC_CAPTURE_LEFT_MASK | ENC_CAPTURE_RIGHT_MASK)

typedef struct
{
    motion_estimator_sample_t sample[2];
    uint32_t invalid[2];
    bool b_healthy;
} enc_capture_sample_t;

typedef struct
{
    motion_quad_t decoder[2];
    uint32_t generation;
    bool b_initialised;
    bool b_healthy;
} enc_capture_t;

/** @brief Init while acquisition stopped; generation must change on reset. */
bool enc_capture_init (enc_capture_t * p_capture,
    motion_config_t const * p_config, uint32_t generation);
/** @brief Feed one simultaneous GPIO_IN read for the indicated encoder pins.
 * Baseline ALL pins before enabling edge events. Unknown events are ignored.
 * Invalid transitions/count limits latch unhealthy, never inferred as zero.
 * ISR lock failure must be reported as a source fault, not silently discarded.
 */
bool enc_capture_feed (enc_capture_t * p_capture, uint32_t events,
                       uint32_t gpio_levels);
/** @brief Coherent count read stamped at snapshot time (fresh even stationary).
 * Caller holds the SAME short SMP/interrupt lock as feed. This is observation
 * time, not last-edge time. GPIO dispatcher health must be checked separately.
 */
bool enc_capture_get (enc_capture_t const * p_capture, uint64_t now_us,
                      enc_capture_sample_t * p_sample);
#endif /* ENC_CAPTURE_H */

/*** end of file ***/

