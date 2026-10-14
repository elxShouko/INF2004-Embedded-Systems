/**
 * @file motion_quad.c
 * @brief Bounded transition decoder; no GPIO, interrupt or RTOS operations.
 */
#include "motion_quad.h"
#include <stddef.h>

#define MOTION_QUAD_STATE_MAX (3U)
#define MOTION_QUAD_TABLE_SIZE (16U)

static int8_t const g_motion_quad_step[MOTION_QUAD_TABLE_SIZE] =
{
     0,  1, -1,  0,
    -1,  0,  0,  1,
     1,  0,  0, -1,
     0, -1,  1,  0
};

static bool motion_quad_polarity_valid (motion_types_polarity_t polarity);
static motion_quad_result_t motion_quad_edge (motion_quad_t * p_decoder,
                                             uint8_t state_ab);

motion_quad_result_t motion_quad_init (motion_quad_t * p_decoder,
                                      motion_types_polarity_t polarity)
{
    motion_quad_result_t result = MOTION_QUAD_NULL;

    if (NULL != p_decoder)
    {
        *p_decoder = (motion_quad_t){0};
        result = MOTION_QUAD_BAD_POLARITY;
        if (motion_quad_polarity_valid(polarity))
        {
            p_decoder->polarity = polarity;
            p_decoder->b_initialised = true;
            result = MOTION_QUAD_OK;
        }
    }

    return (result);
}

motion_quad_result_t motion_quad_update (motion_quad_t * p_decoder,
                                        uint8_t state_ab)
{
    motion_quad_result_t result = MOTION_QUAD_NULL;

    if (NULL == p_decoder)
    {
        result = MOTION_QUAD_NULL;
    }
    else if (!p_decoder->b_initialised)
    {
        result = MOTION_QUAD_UNINITIALISED;
    }
    else if (!motion_quad_polarity_valid(p_decoder->polarity))
    {
        result = MOTION_QUAD_BAD_POLARITY;
    }
    else if (p_decoder->b_count_limit)
    {
        result = MOTION_QUAD_COUNT_LIMIT;
    }
    else if ((state_ab > MOTION_QUAD_STATE_MAX) ||
             (p_decoder->previous_ab > MOTION_QUAD_STATE_MAX))
    {
        result = MOTION_QUAD_BAD_STATE;
    }
    else if (!p_decoder->b_have_state)
    {
        p_decoder->previous_ab = state_ab;
        p_decoder->b_have_state = true;
        result = MOTION_QUAD_BASELINE;
    }
    else
    {
        result = motion_quad_edge(p_decoder, state_ab);
    }

    return (result);
}

static bool motion_quad_polarity_valid (motion_types_polarity_t polarity)
{
    return ((MOTION_TYPES_POLARITY_NORMAL == polarity) ||
            (MOTION_TYPES_POLARITY_REVERSED == polarity));
}

static motion_quad_result_t motion_quad_edge (motion_quad_t * p_decoder,
                                             uint8_t state_ab)
{
    motion_quad_result_t result = MOTION_QUAD_OK;
    uint32_t previous = p_decoder->previous_ab;
    uint32_t changed = previous ^ state_ab;
    int32_t step = g_motion_quad_step[(previous << 2U) | state_ab];

    p_decoder->previous_ab = state_ab;
    if (MOTION_TYPES_POLARITY_REVERSED == p_decoder->polarity)
    {
        step = -step; /* Only -1, 0, +1; negation cannot overflow. */
    }

    if (0U == changed)
    {
        result = MOTION_QUAD_REPEATED;
    }
    else if (MOTION_QUAD_STATE_MAX == changed)
    {
        if (UINT32_MAX != p_decoder->invalid_transitions)
        {
            ++p_decoder->invalid_transitions;
        }
        result = MOTION_QUAD_INVALID_TRANSITION;
    }
    else if (((0 < step) && (INT64_MAX == p_decoder->count)) ||
             ((0 > step) && (INT64_MIN == p_decoder->count)))
    {
        p_decoder->b_count_limit = true;
        result = MOTION_QUAD_COUNT_LIMIT;
    }
    else
    {
        p_decoder->count += step;
    }

    return (result);
}

/*** end of file ***/

