/** @file enc_capture.c
 * @brief Bounded pair acquisition logic; no GPIO/IRQ/kernel operations.
 */
#include "enc_capture.h"
#include <stddef.h>

bool enc_capture_init_raw (enc_capture_t * p_capture, uint32_t generation)
{
    if ((NULL == p_capture) || (0U == generation)) { return (false); }
    *p_capture = (enc_capture_t){0};
    bool b_ok = (MOTION_QUAD_OK == motion_quad_init(&p_capture->decoder[0],
        MOTION_TYPES_POLARITY_NORMAL)) &&
        (MOTION_QUAD_OK == motion_quad_init(&p_capture->decoder[1],
        MOTION_TYPES_POLARITY_NORMAL));
    p_capture->generation = generation;
    p_capture->b_initialised = b_ok;
    p_capture->b_healthy = b_ok;
    return (b_ok);
}

bool enc_capture_init (enc_capture_t * p_capture,
    motion_config_t const * p_config, uint32_t generation)
{
    bool b_ok = (NULL != p_capture) &&
        (MOTION_CONFIG_VALID == motion_config_validate(p_config)) &&
        (0U != generation);

    if (NULL != p_capture)
    {
        *p_capture = (enc_capture_t){0};
        if (b_ok)
        {
            b_ok = (MOTION_QUAD_OK == motion_quad_init(&p_capture->decoder[0],
                p_config->left.encoder_polarity)) &&
                (MOTION_QUAD_OK == motion_quad_init(&p_capture->decoder[1],
                p_config->right.encoder_polarity));
            p_capture->generation = generation;
            p_capture->b_initialised = b_ok;
            p_capture->b_healthy = b_ok;
        }
    }

    return (b_ok);
}

bool enc_capture_feed (enc_capture_t * p_capture, uint32_t events,
                       uint32_t gpio_levels)
{
    bool b_ok = (NULL != p_capture) && p_capture->b_initialised;
    static uint32_t const masks[2] =
        {ENC_CAPTURE_LEFT_MASK, ENC_CAPTURE_RIGHT_MASK};
    static uint32_t const pin_a[2] = {7U, 0U};
    static uint32_t const pin_b[2] = {28U, 1U};

    if (b_ok)
    {
        for (uint32_t index = 0U; index < 2U; ++index)
        {
            if (0U != (events & masks[index]))
            {
                uint8_t state = (uint8_t)
                    ((((gpio_levels >> pin_a[index]) & 1U) << 1U) |
                     ((gpio_levels >> pin_b[index]) & 1U));
                motion_quad_result_t result =
                    motion_quad_update(&p_capture->decoder[index], state);

                if ((MOTION_QUAD_OK != result) &&
                    (MOTION_QUAD_BASELINE != result) &&
                    (MOTION_QUAD_REPEATED != result))
                {
                    p_capture->b_healthy = false;
                }
            }
        }
        b_ok = p_capture->b_healthy;
    }

    return (b_ok);
}

bool enc_capture_get (enc_capture_t const * p_capture, uint64_t now_us,
                      enc_capture_sample_t * p_sample)
{
    bool b_ok = (NULL != p_capture) && (NULL != p_sample) &&
                p_capture->b_initialised;

    if (NULL != p_sample)
    {
        *p_sample = (enc_capture_sample_t){0};
        if (b_ok)
        {
            b_ok = p_capture->b_healthy;
            for (uint32_t index = 0U; index < 2U; ++index)
            {
                p_sample->sample[index] = (motion_estimator_sample_t)
                    {p_capture->decoder[index].count, now_us,
                     p_capture->generation};
                p_sample->invalid[index] =
                    p_capture->decoder[index].invalid_transitions;
                b_ok = b_ok && p_capture->decoder[index].b_have_state;
            }
            p_sample->b_healthy = b_ok;
        }
    }

    return (b_ok);
}

/*** end of file ***/

