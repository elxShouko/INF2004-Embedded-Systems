#include "line/junction.h"

#include <stddef.h>
#include <string.h>

enum
{
    JUNCTION_WAIT_CLEAR = 0,
    JUNCTION_CLEAR,
    JUNCTION_ARMED,
    JUNCTION_BLACK,
    JUNCTION_EXIT
};

static bool config_valid(const junction_config_t *p_config)
{
    return (p_config != NULL) && (p_config->max_barcode_bar_um > 0U) &&
           (p_config->min_span_um > p_config->max_barcode_bar_um) &&
           ((uint64_t)p_config->max_span_um >=
            (uint64_t)p_config->min_span_um + p_config->max_sample_step_um) &&
           (p_config->clear_span_um > 0U) &&
           (p_config->max_sample_step_um > 0U) &&
           (p_config->max_sample_step_um < p_config->clear_span_um) &&
           (p_config->max_sample_step_um < p_config->min_barcode_gap_um) &&
           (p_config->max_sample_step_um <
            (p_config->min_span_um - p_config->max_barcode_bar_um)) &&
           (p_config->max_sample_gap_us > 0U) &&
           (p_config->min_black_us > 0U) &&
           (p_config->max_crossing_us > p_config->min_black_us);
}

bool junction_init(junction_detector_t *p_detector,
                   const junction_config_t *p_config)
{
    if (p_detector == NULL)
    {
        return false;
    }
    /* Copy first in case caller passes the detector's own config for reset. */
    if (!config_valid(p_config))
    {
        memset(p_detector, 0, sizeof(*p_detector));
        return false;
    }
    const junction_config_t config = *p_config;
    memset(p_detector, 0, sizeof(*p_detector));
    p_detector->config = config;
    return true;
}

junction_result_t junction_update(junction_detector_t *p_detector,
                                  const junction_sample_t *p_sample,
                                  nav_event_t *p_event)
{
    if ((p_detector == NULL) || (p_sample == NULL) || (p_event == NULL) ||
        !config_valid(&p_detector->config))
    {
        return JUNCTION_RESULT_INVALID;
    }
    const bool invalid = ((p_sample->black_mask & ~7U) != 0U) ||
        (p_detector->started &&
         ((p_sample->timestamp_us < p_detector->last_us) ||
          (p_sample->forward_distance_um < p_detector->last_distance_um) ||
          ((p_sample->timestamp_us - p_detector->last_us) >
           p_detector->config.max_sample_gap_us) ||
          ((p_sample->forward_distance_um - p_detector->last_distance_um) >
           p_detector->config.max_sample_step_um)));
    p_detector->last_us = p_sample->timestamp_us;
    p_detector->last_distance_um = p_sample->forward_distance_um;
    p_detector->started = true;
    if (invalid || !p_sample->tracking_forward ||
        ((p_sample->black_mask & 3U) != 3U))
    {
        p_detector->phase = JUNCTION_WAIT_CLEAR;
        return invalid ? JUNCTION_RESULT_INVALID : JUNCTION_RESULT_PENDING;
    }

    const bool side_black = (p_sample->black_mask & 4U) != 0U;
    const uint64_t distance = p_sample->forward_distance_um;
    const uint64_t now = p_sample->timestamp_us;
    switch (p_detector->phase)
    {
        case JUNCTION_WAIT_CLEAR:
            if (!side_black)
            {
                p_detector->clear_start_um = distance;
                p_detector->phase = JUNCTION_CLEAR;
            }
            break;

        case JUNCTION_CLEAR:
            if (side_black)
            {
                p_detector->phase = JUNCTION_WAIT_CLEAR;
            }
            else if ((distance - p_detector->clear_start_um) >=
                     p_detector->config.clear_span_um)
            {
                p_detector->phase = JUNCTION_ARMED;
            }
            break;

        case JUNCTION_ARMED:
            if (side_black)
            {
                p_detector->black_start_um = distance;
                p_detector->black_start_us = now;
                p_detector->phase = JUNCTION_BLACK;
            }
            break;

        case JUNCTION_BLACK:
        {
            const uint64_t span = distance - p_detector->black_start_um;
            const uint64_t elapsed = now - p_detector->black_start_us;
            if ((span > p_detector->config.max_span_um) ||
                (elapsed > p_detector->config.max_crossing_us))
            {
                p_detector->phase = JUNCTION_WAIT_CLEAR;
            }
            else if (!side_black)
            {
                /* Subtract one maximum sample step from the observed width
                 * for a conservative lower bound at uncertain edges.
                 */
                if ((span >= (uint64_t)p_detector->config.min_span_um +
                             p_detector->config.max_sample_step_um) &&
                    (elapsed >= p_detector->config.min_black_us))
                {
                    p_detector->first_white_um = distance;
                    p_detector->first_white_us = now;
                    p_detector->phase = JUNCTION_EXIT;
                }
                else
                {
                    p_detector->clear_start_um = distance;
                    p_detector->phase = JUNCTION_CLEAR;
                }
            }
            break;
        }

        case JUNCTION_EXIT:
            if (side_black)
            {
                /* A white glitch or next barcode bar cancels confirmation. */
                p_detector->phase = JUNCTION_WAIT_CLEAR;
            }
            else if ((distance - p_detector->first_white_um) >=
                     p_detector->config.clear_span_um)
            {
                memset(p_event, 0, sizeof(*p_event));
                p_event->timestamp_us = p_detector->first_white_us;
                p_event->kind = NAV_EVENT_JUNCTION;
                p_event->command = NAV_COMMAND_NONE;
                p_event->state = LINE_STATE_TRACKING;
                p_event->black_mask = p_sample->black_mask;
                p_detector->phase = JUNCTION_ARMED;
                return JUNCTION_RESULT_DETECTED;
            }
            break;

        default:
            p_detector->phase = JUNCTION_WAIT_CLEAR;
            return JUNCTION_RESULT_INVALID;
    }
    return JUNCTION_RESULT_PENDING;
}
