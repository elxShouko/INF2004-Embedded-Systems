#ifndef LINE_TRACKING_H
#define LINE_TRACKING_H

#include <stdbool.h>
#include <stdint.h>
#include "ir_input.h"

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
 */
bool line_classify(uint8_t black_mask, line_observation_t *p_observation);

typedef struct
{
    ir_filter_t filter;
} line_tracker_t;

/* Filter ONLY the front pair: barcode transitions must not delay line updates.
 * Set stable_us from actual black tape / white mat measurements.
 */
bool line_tracker_init(line_tracker_t *p_tracker, uint32_t stable_us);
bool line_tracker_update(line_tracker_t *p_tracker, uint64_t timestamp_us,
                         uint8_t black_mask, line_observation_t *p_observation);

#endif
