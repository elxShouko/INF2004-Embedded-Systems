/** @file drv_line_tracking.h
 * @brief Member 3 drv line tracking module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#ifndef LINE_TRACKING_H
#define LINE_TRACKING_H

#include <stdbool.h>
#include <stdint.h>
#include "drv_ir_input.h"

typedef enum
{
    LINE_OBSERVATION_CENTRED = 0,
    LINE_OBSERVATION_CORRECT_LEFT,
    LINE_OBSERVATION_CORRECT_RIGHT,
    LINE_OBSERVATION_ABSENT
} line_observation_t;

/* Normalized black mask: bit 0 = GP2/left, bit 1 = GP4/right.
 * Both sensing points must sit inside the line edges when centred.
 * This requests correction direction only; Buddy 2 owns actual motion.
 * 3 = centred; 1 = black on left only, correct left; 2 = correct right;
 * 0 = absent. Left/right are viewed from the car facing forward.
 * Magnitude, speed, absent-line handling and motor polarity require the
 * Mission Controller/Buddy 2 agreement and physical mounting validation.
 */
/** @brief Translate the two front observations into correction direction. */
bool line_classify (uint8_t black_mask, line_observation_t * p_observation);

typedef struct
{
    ir_filter_t filter;
} line_tracker_t;

/* Filter ONLY the front pair: barcode transitions must not delay line updates.
 * Set stable_us from actual black tape / white mat measurements.
 * While a new pattern settles, update returns the LAST stable observation.
 * This is not proof that the current raw sample still has that pattern.
 * Owner outputs report changes only. A controller must poll fresh S3 status
 * and refresh continuous motion independently of observation-change events.
 * See docs/buddy3/production-timing-and-handoff.md for the proposed contract.
 */
/** @brief Reset filtering for the front pair independently of barcodes. */
bool line_tracker_init (line_tracker_t * p_tracker, uint32_t stable_us);
/** @brief Update the front-pair observation after stability filtering. */
bool line_tracker_update (line_tracker_t * p_tracker, uint64_t timestamp_us,
                          uint8_t              black_mask,
                          line_observation_t * p_observation);

#endif

/*** End of file. ***/

