/**
 * @file motion_pid.c
 * @brief Bounded single-precision PID with explicit failure semantics.
 */
#include "motion_pid.h"
#include <math.h>
#include <stddef.h>

#define MOTION_PID_US_PER_SECOND (1000000.0F)

static bool motion_pid_finite (motion_pid_value_t value);
static motion_pid_value_t motion_pid_clamp (motion_pid_value_t value,
    motion_pid_value_t minimum, motion_pid_value_t maximum);
static void motion_pid_clear (motion_pid_t * p_controller);
static motion_pid_result_t motion_pid_calculate (motion_pid_t * p_controller,
    motion_types_speed_mm_s_t target_mm_s,
    motion_types_speed_mm_s_t measured_mm_s, uint32_t elapsed_us,
    motion_pid_value_t * p_effort);

motion_pid_result_t motion_pid_validate (motion_pid_config_t const * p_config)
{
    motion_pid_result_t result = MOTION_PID_BAD_CONFIG;

    if (NULL == p_config)
    {
        result = MOTION_PID_NULL;
    }
    else if ((motion_pid_finite(p_config->proportional_gain)) &&
             (motion_pid_finite(p_config->integral_gain)) &&
             (motion_pid_finite(p_config->derivative_gain)) &&
             (motion_pid_finite(p_config->output_min)) &&
             (motion_pid_finite(p_config->output_max)) &&
             (motion_pid_finite(p_config->integral_min)) &&
             (motion_pid_finite(p_config->integral_max)) &&
             (p_config->proportional_gain >= 0.0F) &&
             (p_config->integral_gain >= 0.0F) &&
             (p_config->derivative_gain >= 0.0F) &&
             (p_config->output_min >= -1.0F) &&
             (p_config->output_min <= 0.0F) &&
             (p_config->output_max <= 1.0F) &&
             (p_config->output_max >= 0.0F) &&
             (p_config->output_min < p_config->output_max) &&
             (p_config->integral_min <= 0.0F) &&
             (p_config->integral_max >= 0.0F) &&
             (0U != p_config->max_elapsed_us) &&
             ((MOTION_CONFIG_CALIBRATION_CONFIRMED == p_config->calibration) ||
              ((MOTION_CONFIG_CALIBRATION_DEVELOPMENT == p_config->calibration) &&
               (p_config->output_min >= -0.25F) &&
               (p_config->output_max <= 0.25F))))
    {
        result = MOTION_PID_OK;
    }
    else
    {
        /* Missing confirmation, invalid bounds or nonfinite/negative gains. */
    }

    return (result);
}

motion_pid_result_t motion_pid_init (motion_pid_t * p_controller,
                                   motion_pid_config_t const * p_config)
{
    motion_pid_result_t result = MOTION_PID_NULL;

    if (NULL != p_controller)
    {
        /* Copy before clearing so re-init from this instance's own config
         * also has deterministic reset semantics.
         */
        motion_pid_config_t config = {0};

        if (NULL != p_config)
        {
            config = *p_config;
        }
        *p_controller = (motion_pid_t){0};
        result = (NULL == p_config) ? MOTION_PID_NULL :
                                    motion_pid_validate(&config);
        if (MOTION_PID_OK == result)
        {
            p_controller->config = config;
            p_controller->b_initialised = true;
        }
    }

    return (result);
}

motion_pid_result_t motion_pid_reset (motion_pid_t * p_controller)
{
    motion_pid_result_t result = MOTION_PID_NULL;

    if (NULL != p_controller)
    {
        motion_pid_clear(p_controller);
        result = p_controller->b_initialised ? MOTION_PID_OK :
                                             MOTION_PID_UNINITIALISED;
    }

    return (result);
}

motion_pid_result_t motion_pid_update (motion_pid_t * p_controller,
    motion_types_speed_mm_s_t target_mm_s,
    motion_types_speed_mm_s_t measured_mm_s, uint32_t elapsed_us,
    bool b_active, motion_pid_value_t * p_effort)
{
    motion_pid_result_t result = MOTION_PID_NULL;

    if (NULL != p_effort)
    {
        *p_effort = 0.0F;
    }
    if ((NULL == p_controller) || (NULL == p_effort))
    {
        result = MOTION_PID_NULL;
    }
    else if (!p_controller->b_initialised)
    {
        result = MOTION_PID_UNINITIALISED;
    }
    else if (MOTION_PID_OK != motion_pid_validate(&p_controller->config))
    {
        motion_pid_clear(p_controller);
        p_controller->b_initialised = false;
        result = MOTION_PID_BAD_CONFIG;
    }
    else if ((!b_active) || (0 == target_mm_s))
    {
        motion_pid_clear(p_controller);
        result = MOTION_PID_IDLE;
    }
    else if ((0U == elapsed_us) ||
             (elapsed_us > p_controller->config.max_elapsed_us))
    {
        motion_pid_clear(p_controller);
        result = MOTION_PID_BAD_TIME;
    }
    else
    {
        result = motion_pid_calculate(p_controller, target_mm_s,
                                      measured_mm_s, elapsed_us, p_effort);
        if (MOTION_PID_OK != result)
        {
            motion_pid_clear(p_controller);
            p_controller->b_initialised = false;
            *p_effort = 0.0F;
        }
    }

    return (result);
}

static bool motion_pid_finite (motion_pid_value_t value)
{
    return (0 != isfinite(value));
}

static motion_pid_value_t motion_pid_clamp (motion_pid_value_t value,
    motion_pid_value_t minimum, motion_pid_value_t maximum)
{
    motion_pid_value_t bounded = value;

    if (value < minimum)
    {
        bounded = minimum;
    }
    else if (value > maximum)
    {
        bounded = maximum;
    }
    else
    {
        /* Already within the explicit bounds. */
    }

    return (bounded);
}

static void motion_pid_clear (motion_pid_t * p_controller)
{
    p_controller->integral = 0.0F;
    p_controller->previous_measurement_mm_s = 0;
    p_controller->previous_target_mm_s = 0;
    p_controller->b_have_measurement = false;
}

static motion_pid_result_t motion_pid_calculate (motion_pid_t * p_controller,
    motion_types_speed_mm_s_t target_mm_s,
    motion_types_speed_mm_s_t measured_mm_s, uint32_t elapsed_us,
    motion_pid_value_t * p_effort)
{
    motion_pid_result_t result = MOTION_PID_ARITHMETIC;
    motion_pid_config_t const * p_config = &p_controller->config;
    /* int32 differences fit int64; float conversion may round, never exceed
     * the floating-point range. Microseconds also fit that range.
     */
    motion_pid_value_t error = (motion_pid_value_t)
        ((int64_t)target_mm_s - measured_mm_s);
    motion_pid_value_t delta = (motion_pid_value_t)
        ((int64_t)measured_mm_s - p_controller->previous_measurement_mm_s);
    motion_pid_value_t elapsed = (motion_pid_value_t)elapsed_us /
                                 MOTION_PID_US_PER_SECOND;
    motion_pid_value_t integral = p_controller->integral;
    bool b_state_valid = (motion_pid_finite(integral)) &&
        (integral >= p_config->integral_min) &&
        (integral <= p_config->integral_max);
    motion_pid_value_t proportional = p_config->proportional_gain * error;
    motion_pid_value_t derivative = 0.0F;

    if (((target_mm_s > 0) && (p_controller->previous_target_mm_s < 0)) ||
        ((target_mm_s < 0) && (p_controller->previous_target_mm_s > 0)))
    {
        integral = 0.0F;
    }
    if (p_controller->b_have_measurement)
    {
        derivative = -p_config->derivative_gain * (delta / elapsed);
    }

    motion_pid_value_t trial = integral +
        ((p_config->integral_gain * error) * elapsed);
    motion_pid_value_t output = 0.0F;
    bool b_finite = (motion_pid_finite(proportional)) &&
        (motion_pid_finite(derivative)) && (motion_pid_finite(integral)) &&
        (motion_pid_finite(trial)) && b_state_valid;

    if (b_finite)
    {
        trial = motion_pid_clamp(trial, p_config->integral_min,
                                 p_config->integral_max);
        output = (proportional + derivative) + trial;
        b_finite = motion_pid_finite(output);
    }
    if (b_finite)
    {
        /* Clip only the proposed change, never move I beyond that change.
         * For a rising I: Inew = max(Iold, min(Itrial, umax - (P + D))).
         * For a falling I use the symmetric lower-bound expression.
         * If P/D alone saturates, retain I rather than injecting opposite I.
         */
        motion_pid_value_t base = proportional + derivative;
        motion_pid_value_t boundary = 0.0F;

        b_finite = motion_pid_finite(base);
        if (b_finite && (trial > integral) &&
            (output > p_config->output_max))
        {
            boundary = p_config->output_max - base;
            b_finite = motion_pid_finite(boundary);
            trial = motion_pid_clamp(boundary, integral, trial);
        }
        else if (b_finite && (trial < integral) &&
                 (output < p_config->output_min))
        {
            boundary = p_config->output_min - base;
            b_finite = motion_pid_finite(boundary);
            trial = motion_pid_clamp(boundary, trial, integral);
        }
        else
        {
            /* Within output headroom, or moving away from saturation. */
        }
        integral = trial;
        output = base + integral;
        b_finite = b_finite && (motion_pid_finite(output));
    }
    if (b_finite)
    {
        *p_effort = motion_pid_clamp(output, p_config->output_min,
                                    p_config->output_max);
        p_controller->integral = integral;
        p_controller->previous_measurement_mm_s = measured_mm_s;
        p_controller->previous_target_mm_s = target_mm_s;
        p_controller->b_have_measurement = true;
        result = MOTION_PID_OK;
    }

    return (result);
}

/*** end of file ***/

