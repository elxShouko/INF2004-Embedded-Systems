/**
 * @file motion_quad.h
 * @brief Pure x4 quadrature decoding; internal Buddy 2 interface.
 */
#ifndef MOTION_QUAD_H
#define MOTION_QUAD_H

#include <stdbool.h>
#include <stdint.h>
#include "motion_types.h"

typedef enum
{
    MOTION_QUAD_OK = 0,
    MOTION_QUAD_BASELINE,
    MOTION_QUAD_REPEATED,
    MOTION_QUAD_INVALID_TRANSITION,
    MOTION_QUAD_NULL,
    MOTION_QUAD_UNINITIALISED,
    MOTION_QUAD_BAD_POLARITY,
    MOTION_QUAD_BAD_STATE,
    MOTION_QUAD_COUNT_LIMIT
} motion_quad_result_t;

/* Instance-owned state. Initialise before use; zero state is unavailable.
 * Only the owner may update it. Future ISR/task snapshots need separate
 * synchronisation. Do not modify fields during an active acquisition.
 */
typedef struct
{
    motion_types_encoder_count_t count;
    uint32_t invalid_transitions;
    motion_types_polarity_t polarity;
    uint8_t previous_ab;
    bool b_initialised;
    bool b_have_state;
    bool b_count_limit;
} motion_quad_t;

/* Clears position/diagnostics/baseline. Invalid polarity leaves it disabled.
 * Call again only while acquisition is stopped, and change the estimator's
 * source generation whenever resetting the encoder count origin.
 */
motion_quad_result_t motion_quad_init (motion_quad_t * p_decoder,
                                      motion_types_polarity_t polarity);

/* state_ab = (A << 1) | B, range 0..3. Raw-positive: 00,01,11,10,00.
 * First sample establishes state only. Adjacent edges count +/-1 (x4).
 * Polarity is applied HERE once: count is vehicle-oriented, not raw.
 * Two-bit jumps resynchronise and saturating-increment the diagnostic,
 * without guessing missed counts. Out-of-range samples leave state intact.
 * Signed count never wraps: overflow/underflow latches COUNT_LIMIT, holding
 * position until explicit init. All calls thereafter report COUNT_LIMIT.
 */
motion_quad_result_t motion_quad_update (motion_quad_t * p_decoder,
                                        uint8_t state_ab);

#endif /* MOTION_QUAD_H */

/*** end of file ***/

