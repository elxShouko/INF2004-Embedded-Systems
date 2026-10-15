/**
 * @file motion_config.c
 * @brief Allocation-free configuration reset and consistency validation.
 */
#include "motion_config.h"

#include <stddef.h>

static bool motion_config_polarity_valid (motion_types_polarity_t polarity);
static bool motion_config_wheel_valid (motion_config_wheel_t const * p_wheel);

bool motion_config_init (motion_config_t * p_config)
{
    bool b_initialised = false;

    if (NULL != p_config)
    {
        *p_config = (motion_config_t){0};
        b_initialised = true;
    }

    return (b_initialised);
}

motion_config_result_t motion_config_validate (
    motion_config_t const * p_config)
{
    motion_config_result_t result = MOTION_CONFIG_VALID;

    if (NULL == p_config)
    {
        result = MOTION_CONFIG_NULL;
    }
    else if (MOTION_CONFIG_CALIBRATION_UNCONFIGURED == p_config->calibration)
    {
        result = MOTION_CONFIG_UNCONFIGURED;
    }
    else if ((MOTION_CONFIG_CALIBRATION_UNVERIFIED != p_config->calibration) &&
             (MOTION_CONFIG_CALIBRATION_CONFIRMED != p_config->calibration))
    {
        result = MOTION_CONFIG_BAD_CALIBRATION;
    }
    else if (!motion_config_wheel_valid(&p_config->left))
    {
        result = MOTION_CONFIG_BAD_LEFT_WHEEL;
    }
    else if (!motion_config_wheel_valid(&p_config->right))
    {
        result = MOTION_CONFIG_BAD_RIGHT_WHEEL;
    }
    else if (0U == p_config->track_width_um)
    {
        result = MOTION_CONFIG_BAD_TRACK_WIDTH;
    }
    else if (0 >= p_config->max_wheel_speed_mm_s)
    {
        result = MOTION_CONFIG_BAD_SPEED_LIMIT;
    }
    else if (MOTION_CONFIG_CALIBRATION_UNVERIFIED == p_config->calibration)
    {
        result = MOTION_CONFIG_UNVERIFIED;
    }
    else
    {
        /* The caller supplied complete values and confirmation evidence. */
    }

    return (result);
}

static bool motion_config_polarity_valid (motion_types_polarity_t polarity)
{
    return ((MOTION_TYPES_POLARITY_NORMAL == polarity) ||
            (MOTION_TYPES_POLARITY_REVERSED == polarity));
}

static bool motion_config_wheel_valid (motion_config_wheel_t const * p_wheel)
{
    return ((0U != p_wheel->counts_per_wheel_rev) &&
            (0U != p_wheel->wheel_circumference_um) &&
            (motion_config_polarity_valid(p_wheel->motor_polarity)) &&
            (motion_config_polarity_valid(p_wheel->encoder_polarity)));
}

/*** end of file ***/

