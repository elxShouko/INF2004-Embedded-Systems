#include "line/ir_input.h"

#include <stddef.h>
#include <string.h>

#define IR_ALL_CHANNELS_MASK ((1U << IR_CHANNEL_COUNT) - 1U)

static bool sample_valid(const ir_sample_t *p_sample)
{
    return (p_sample != NULL) &&
           ((p_sample->levels & ~IR_ALL_CHANNELS_MASK) == 0U);
}

bool ir_observe(ir_observations_t *p_observations,
                const ir_sample_t *p_sample)
{
    if ((p_observations == NULL) || !sample_valid(p_sample) ||
        (p_observations->samples == UINT32_MAX))
    {
        return false;
    }

    for (uint32_t channel = 0U; channel < IR_CHANNEL_COUNT; ++channel)
    {
        if (p_observations->high_count[channel] > p_observations->samples)
        {
            return false;
        }
    }
    ++p_observations->samples;
    for (uint32_t channel = 0U; channel < IR_CHANNEL_COUNT; ++channel)
    {
        if ((p_sample->levels & (1U << channel)) != 0U)
        {
            ++p_observations->high_count[channel];
        }
    }
    return true;
}

bool ir_calibrate(const ir_observations_t *p_black,
                  const ir_observations_t *p_white,
                  uint32_t minimum_samples,
                  ir_calibration_t *p_calibration)
{
    if (p_calibration == NULL)
    {
        return false;
    }
    memset(p_calibration, 0, sizeof(*p_calibration));
    if ((p_black == NULL) || (p_white == NULL) || (minimum_samples == 0U) ||
        (p_black->samples < minimum_samples) ||
        (p_white->samples < minimum_samples))
    {
        return false;
    }
    for (uint32_t channel = 0U; channel < IR_CHANNEL_COUNT; ++channel)
    {
        const uint32_t black = p_black->high_count[channel];
        const uint32_t white = p_white->high_count[channel];
        const bool black_high = (black == p_black->samples);
        const bool black_low = (black == 0U);
        const bool white_high = (white == p_white->samples);
        const bool white_low = (white == 0U);

        if ((black_high && white_low) || (black_low && white_high))
        {
            p_calibration->valid_mask |= (uint8_t)(1U << channel);
            if (black_high)
            {
                p_calibration->black_high_mask |= (uint8_t)(1U << channel);
            }
        }
    }
    return p_calibration->valid_mask == IR_ALL_CHANNELS_MASK;
}

bool ir_normalize(const ir_sample_t *p_sample,
                  const ir_calibration_t *p_calibration,
                  uint8_t *p_black_mask)
{
    if (!sample_valid(p_sample) || (p_calibration == NULL) ||
        (p_black_mask == NULL) ||
        (p_calibration->valid_mask != IR_ALL_CHANNELS_MASK) ||
        ((p_calibration->black_high_mask & ~IR_ALL_CHANNELS_MASK) != 0U))
    {
        return false;
    }
    *p_black_mask = (uint8_t)(~(p_sample->levels ^
                             p_calibration->black_high_mask) &
                             IR_ALL_CHANNELS_MASK);
    return true;
}

bool ir_filter_init(ir_filter_t *p_filter, uint32_t stable_us)
{
    if (p_filter == NULL)
    {
        return false;
    }
    memset(p_filter, 0, sizeof(*p_filter));
    p_filter->stable_us = stable_us;
    return true;
}

bool ir_filter_update(ir_filter_t *p_filter, const ir_sample_t *p_sample,
                      ir_sample_t *p_filtered)
{
    if ((p_filter == NULL) || !sample_valid(p_sample) ||
        (p_filtered == NULL) ||
        (p_filter->started &&
         (p_sample->timestamp_us < p_filter->last_timestamp_us)))
    {
        return false;
    }
    if (!p_filter->started || (p_sample->levels != p_filter->candidate))
    {
        p_filter->candidate = p_sample->levels;
        p_filter->candidate_since_us = p_sample->timestamp_us;
        p_filter->started = true;
    }
    p_filter->last_timestamp_us = p_sample->timestamp_us;
    if ((p_sample->timestamp_us - p_filter->candidate_since_us) >=
        p_filter->stable_us)
    {
        p_filter->stable = p_filter->candidate;
        p_filter->valid = true;
    }
    if (!p_filter->valid)
    {
        return false;
    }
    p_filtered->timestamp_us = p_sample->timestamp_us;
    p_filtered->levels = p_filter->stable;
    return true;
}
