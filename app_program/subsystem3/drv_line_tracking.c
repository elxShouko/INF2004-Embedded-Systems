#include "line/line_tracking.h"

#include <stddef.h>

bool line_classify(uint8_t black_mask, line_observation_t *p_observation)
{
    if ((p_observation == NULL) || ((black_mask & ~7U) != 0U))
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

bool line_tracker_init(line_tracker_t *p_tracker, uint32_t stable_us)
{
    return (p_tracker != NULL) && ir_filter_init(&p_tracker->filter, stable_us);
}

bool line_tracker_update(line_tracker_t *p_tracker, uint64_t timestamp_us,
                         uint8_t black_mask, line_observation_t *p_observation)
{
    if ((p_tracker == NULL) || (p_observation == NULL) ||
        ((black_mask & ~7U) != 0U))
    {
        return false;
    }
    const ir_sample_t pair = {timestamp_us, (uint8_t)(black_mask & 3U)};
    ir_sample_t filtered;
    return ir_filter_update(&p_tracker->filter, &pair, &filtered) &&
           line_classify(filtered.levels, p_observation);
}
