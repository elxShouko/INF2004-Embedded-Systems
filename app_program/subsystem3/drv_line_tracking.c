/** @file drv_line_tracking.c
 * @brief Member 3 drv line tracking module.
 * @note See BARR_COMPLIANCE.md for compatibility exceptions under review.
 */

#include "drv_line_tracking.h"

#include <stddef.h>

/**
 * @brief Translate the two front observations into correction direction.
 */
bool line_classify (uint8_t black_mask, line_observation_t * p_observation)
{
    if ((NULL == p_observation) || ((black_mask & ~7U) != 0U))
    {
        return false;
    }

    switch (black_mask & 3U)
    {
        case 3U:
            *p_observation = LINE_OBSERVATION_CENTRED;
        break;
        case 1U:
            *p_observation = LINE_OBSERVATION_CORRECT_LEFT;
        break;
        case 2U:
            *p_observation = LINE_OBSERVATION_CORRECT_RIGHT;
        break;
        default:
            *p_observation = LINE_OBSERVATION_ABSENT;
        break;
    }

    return true;
}

/**
 * @brief Reset filtering for the front pair independently of barcodes.
 */
bool line_tracker_init (line_tracker_t * p_tracker, uint32_t stable_us)
{
    return (p_tracker != NULL) &&
           (ir_filter_init(&p_tracker->filter, stable_us));
}

/**
 * @brief Update the front-pair observation after stability filtering.
 * @note Only two mask bits survive before conversion to uint8_t.
 */
bool line_tracker_update (line_tracker_t * p_tracker, uint64_t timestamp_us,
                          uint8_t              black_mask,
                          line_observation_t * p_observation)
{
    if ((NULL == p_tracker) || (NULL == p_observation) ||
        ((black_mask & ~7U) != 0U))
    {
        return false;
    }

    const ir_sample_t pair = {timestamp_us, (uint8_t)(black_mask & 3U)};
    ir_sample_t       filtered;

    return ir_filter_update(&p_tracker->filter, &pair, &filtered) &&
           (line_classify(filtered.levels, p_observation));
}

/*** End of file. ***/

