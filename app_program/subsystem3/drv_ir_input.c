/** @file drv_ir_input.c
 * @brief Member 3 drv ir input module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#include "drv_ir_input.h"

#include <stddef.h>
#include <string.h>

#define IR_CHANNELS_TOTAL 3U
#define IR_ALL_CHANNELS_MASK ((1U << IR_CHANNEL_COUNT) - 1U)

static bool sample_valid (const ir_sample_t * p_sample);

/**
 * @brief Accumulate a raw reference sample without counter overflow.
 * @note Each channel index is below three, so sample mask operations fit
 * uint32_t.
 */
bool ir_observe (ir_observations_t * p_observations,
                 const ir_sample_t * p_sample)
{
    if ((NULL == p_observations) || (!sample_valid(p_sample)) ||
        (UINT32_MAX == p_observations->samples))
    {
        return false;
    }

    for (uint32_t channel = 0U; channel < IR_CHANNELS_TOTAL; ++channel)
    {
        if (p_observations->high_count[channel] > p_observations->samples)
        {
            return false;
        }
    }

    ++p_observations->samples;

    for (uint32_t channel = 0U; channel < IR_CHANNELS_TOTAL; ++channel)
    {
        if ((p_sample->levels & (1U << channel)) != 0U)
        {
            ++p_observations->high_count[channel];
        }
    }

    return true;
}

/**
 * @brief Validate opposite stable black and white reference readings.
 * @note Channel bits are below eight; narrowing the constructed masks to
 * uint8_t is lossless.
 */
bool ir_calibrate (const ir_observations_t * p_black,
                   const ir_observations_t * p_white, uint32_t minimum_samples,
                   ir_calibration_t * p_calibration)
{
    if (NULL == p_calibration)
    {
        return false;
    }

    memset(p_calibration, 0, sizeof(*p_calibration));

    if ((NULL == p_black) || (NULL == p_white) || (0U == minimum_samples) ||
        (p_black->samples < minimum_samples) ||
        (p_white->samples < minimum_samples))
    {
        return false;
    }

    for (uint32_t channel = 0U; channel < IR_CHANNELS_TOTAL; ++channel)
    {
        const uint32_t black        = p_black->high_count[channel];
        const uint32_t white        = p_white->high_count[channel];
        const bool     b_black_high = (black == p_black->samples);
        const bool     b_black_low  = (0U == black);
        const bool     b_white_high = (white == p_white->samples);
        const bool     b_white_low  = (0U == white);

        if ((b_black_high && b_white_low) || (b_black_low && b_white_high))
        {
            p_calibration->valid_mask |= (uint8_t)(1U << channel);

            if (b_black_high)
            {
                p_calibration->black_high_mask |= (uint8_t)(1U << channel);
            }
        }
    }

    return IR_ALL_CHANNELS_MASK == p_calibration->valid_mask;
}

/**
 * @brief Convert calibrated GPIO levels into a black-surface mask.
 * @note GPIO bytes widen losslessly; the final three-bit mask fits uint8_t.
 */
bool ir_normalize (const ir_sample_t *      p_sample,
                   const ir_calibration_t * p_calibration,
                   uint8_t *                p_black_mask)
{
    if ((!sample_valid(p_sample)) || (NULL == p_calibration) ||
        (NULL == p_black_mask) ||
        (p_calibration->valid_mask != IR_ALL_CHANNELS_MASK) ||
        ((p_calibration->black_high_mask & ~IR_ALL_CHANNELS_MASK) != 0U))
    {
        return false;
    }

    *p_black_mask = (uint8_t)(~((uint32_t)p_sample->levels ^
                                (uint32_t)p_calibration->black_high_mask) &
                              IR_ALL_CHANNELS_MASK);

    return true;
}

/**
 * @brief Reset stability filtering using a caller-selected interval.
 */
bool ir_filter_init (ir_filter_t * p_filter, uint32_t stable_us)
{
    if (NULL == p_filter)
    {
        return false;
    }

    memset(p_filter, 0, sizeof(*p_filter));
    p_filter->stable_us = stable_us;

    return true;
}

/**
 * @brief Retain the last stable pattern until a new pattern settles.
 */
bool ir_filter_update (ir_filter_t * p_filter, const ir_sample_t * p_sample,
                       ir_sample_t * p_filtered)
{
    if ((NULL == p_filter) || (!sample_valid(p_sample)) ||
        (NULL == p_filtered) ||
        ((p_filter->started) &&
         (p_sample->timestamp_us < p_filter->last_timestamp_us)))
    {
        return false;
    }

    if ((!p_filter->started) || (p_sample->levels != p_filter->candidate))
    {
        p_filter->candidate          = (uint8_t)(p_sample->levels & 7U);
        p_filter->candidate_since_us = p_sample->timestamp_us;
        p_filter->started            = true;
    }

    p_filter->last_timestamp_us = p_sample->timestamp_us;

    if ((p_sample->timestamp_us - p_filter->candidate_since_us) >=
        p_filter->stable_us)
    {
        p_filter->stable = p_filter->candidate;
        p_filter->valid  = true;
    }

    if (!p_filter->valid)
    {
        return false;
    }

    p_filtered->timestamp_us = p_sample->timestamp_us;
    p_filtered->levels       = p_filter->stable;

    return true;
}

/**
 * @brief Check that a sample contains only the three owned channels.
 */
static bool sample_valid (const ir_sample_t * p_sample)
{
    return (p_sample != NULL) &&
           ((p_sample->levels & ~IR_ALL_CHANNELS_MASK) == 0U);
}

/*** End of file. ***/

