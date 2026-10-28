/**
 * @file drv_motor.c
 * @brief Bounded effort conversion and nonblocking reversal state machine.
 */
#include "drv_motor.h"
#include <math.h>
#include <stddef.h>

static drv_motor_result_t drv_motor_fail (drv_motor_t * p_motor,
                                         drv_motor_result_t cause);
static drv_motor_result_t drv_motor_ready (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, uint64_t now_us);
static bool drv_motor_mode_valid (drv_motor_stop_t mode);
static bool drv_motor_config_valid (drv_motor_config_t const * p_config);
static drv_motor_result_t drv_motor_neutral (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, drv_motor_stop_t mode, uint64_t now_us);
static drv_motor_result_t drv_motor_drive (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, int8_t direction, uint16_t level);

drv_motor_result_t drv_motor_init (drv_motor_t * p_motor,
    drv_motor_config_t const * p_config, drv_motor_io_t const * p_io)
{
    drv_motor_result_t result = DRV_MOTOR_BAD_PARAM;

    if (NULL != p_motor)
    {
        if (p_motor->b_initialised)
        {
            result = drv_motor_fail(p_motor, DRV_MOTOR_BAD_PARAM);
        }
        else if ((NULL != p_config) && (NULL != p_io) &&
                 (NULL != p_io->p_prepare) && (NULL != p_io->p_apply) &&
                 (NULL != p_io->p_stop_all) &&
                 (drv_motor_config_valid(p_config)))
        {
            uint64_t period = ((uint64_t)p_config->clock_hz +
                p_config->pwm_hz - 1U) / p_config->pwm_hz;

            if ((period >= 2U) && (period <= UINT16_MAX))
            {
                /* The binary32 value times a 16-bit integer fits double's
                 * checked precision exactly. Avoid rounding UP across a
                 * count boundary before this truncation. Init only.
                 */
                uint16_t ceiling = (uint16_t)((double)p_config->max_duty *
                                              (double)period);

                if (0U != ceiling)
                {
                    *p_motor = (drv_motor_t){0};
                    p_motor->config = *p_config;
                    p_motor->io = *p_io;
                    p_motor->period = (uint16_t)period;
                    p_motor->duty_limit = ceiling;
                    p_motor->b_initialised = true;
                    result = DRV_MOTOR_OK;
                }
            }
        }
        else
        {
            /* Invalid candidates perform no backend operations. */
        }
    }

    return (result);
}

drv_motor_result_t drv_motor_arm (drv_motor_t * p_motor, uint64_t now_us)
{
    drv_motor_result_t result = DRV_MOTOR_BAD_PARAM;

    if ((NULL != p_motor) && p_motor->b_initialised)
    {
        result = DRV_MOTOR_FAULT;
        if (!p_motor->b_fault)
        {
            if (p_motor->b_armed)
            {
                result = drv_motor_fail(p_motor, DRV_MOTOR_BAD_PARAM);
            }
            else if (now_us < p_motor->last_time_us)
            {
                result = drv_motor_fail(p_motor, DRV_MOTOR_BAD_TIME);
            }
            else
            {
                p_motor->b_prepare_attempted = true;
                if (p_motor->io.p_prepare(p_motor->io.p_context,
                    p_motor->period, p_motor->config.clock_hz))
                {
                    p_motor->b_armed = true;
                    p_motor->b_stop_confirmed = true;
                    p_motor->last_time_us = now_us;
                    for (uint32_t index = 0U; index < DRV_MOTOR_WHEELS; ++index)
                    {
                        p_motor->channel[index].state = DRV_MOTOR_STOPPED;
                        p_motor->channel[index].b_neutral = true;
                        p_motor->channel[index].neutral_since_us = now_us;
                    }
                    result = DRV_MOTOR_OK;
                }
                else
                {
                    result = drv_motor_fail(p_motor, DRV_MOTOR_HARDWARE);
                }
            }
        }
    }

    return (result);
}

drv_motor_result_t drv_motor_set (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, motion_pid_value_t effort, uint64_t now_us)
{
    drv_motor_result_t result = drv_motor_ready(p_motor, wheel, now_us);

    if (DRV_MOTOR_OK == result)
    {
        if (0 == isfinite(effort))
        {
            result = drv_motor_fail(p_motor, DRV_MOTOR_BAD_PARAM);
        }
        else
        {
            float magnitude = (effort < 0.0F) ? -effort : effort;
            int8_t direction = (effort < 0.0F) ? -1 : 1;

            if (magnitude > 1.0F)
            {
                magnitude = 1.0F;
            }
            uint32_t rounded = (uint32_t)((magnitude *
                (float)p_motor->period) + 0.5F);
            uint16_t level = (rounded > p_motor->duty_limit) ?
                p_motor->duty_limit : (uint16_t)rounded;
            drv_motor_channel_t * p_channel = &p_motor->channel[wheel];

            if (MOTION_TYPES_POLARITY_REVERSED ==
                p_motor->config.polarity[wheel])
            {
                direction = (int8_t)-direction;
            }
            if (0U == level)
            {
                result = drv_motor_neutral(p_motor, wheel,
                    p_motor->config.zero_mode, now_us);
            }
            else if ((0 != p_channel->last_direction) &&
                     (direction != p_channel->last_direction))
            {
                result = drv_motor_neutral(p_motor, wheel,
                                           DRV_MOTOR_BRAKE, now_us);
                if (DRV_MOTOR_OK == result)
                {
                    p_channel->state = DRV_MOTOR_REVERSING;
                    p_channel->pending_direction = direction;
                    p_channel->pending_level = level;
                    result = DRV_MOTOR_WAIT;
                }
            }
            else
            {
                result = drv_motor_drive(p_motor, wheel, direction, level);
            }
        }
    }

    return (result);
}

drv_motor_result_t drv_motor_stop (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, drv_motor_stop_t mode, uint64_t now_us)
{
    drv_motor_result_t result = drv_motor_ready(p_motor, wheel, now_us);

    if (DRV_MOTOR_OK == result)
    {
        result = (drv_motor_mode_valid(mode)) ?
            drv_motor_neutral(p_motor, wheel, mode, now_us) :
            drv_motor_fail(p_motor, DRV_MOTOR_BAD_PARAM);
    }

    return (result);
}

drv_motor_result_t drv_motor_service (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, uint64_t now_us, bool b_standstill_confirmed)
{
    drv_motor_result_t result = drv_motor_ready(p_motor, wheel, now_us);

    if ((DRV_MOTOR_OK == result) &&
        (DRV_MOTOR_REVERSING == p_motor->channel[wheel].state))
    {
        drv_motor_channel_t * p_channel = &p_motor->channel[wheel];

        result = DRV_MOTOR_WAIT;
        if (b_standstill_confirmed &&
            ((now_us - p_channel->neutral_since_us) >=
             p_motor->config.reversal_guard_us))
        {
            result = drv_motor_drive(p_motor, wheel,
                p_channel->pending_direction, p_channel->pending_level);
        }
    }

    return (result);
}

drv_motor_result_t drv_motor_disarm (drv_motor_t * p_motor)
{
    drv_motor_result_t result = DRV_MOTOR_BAD_PARAM;

    if ((NULL != p_motor) && p_motor->b_initialised)
    {
        if (p_motor->b_prepare_attempted &&
            !(p_motor->io.p_stop_all(p_motor->io.p_context)))
        {
            result = drv_motor_fail(p_motor, DRV_MOTOR_HARDWARE);
        }
        else
        {
            p_motor->b_armed = false;
            p_motor->b_stop_confirmed = p_motor->b_prepare_attempted;
            for (uint32_t index = 0U; index < DRV_MOTOR_WHEELS; ++index)
            {
                p_motor->channel[index].state = DRV_MOTOR_STOPPED;
                p_motor->channel[index].pending_level = 0U;
            }
            result = p_motor->b_fault ? DRV_MOTOR_FAULT : DRV_MOTOR_OK;
        }
    }

    return (result);
}

/** @brief Latch a fault and attempt both brakes without queue processing. */
static drv_motor_result_t drv_motor_fail (drv_motor_t * p_motor,
                                         drv_motor_result_t cause)
{
    p_motor->b_fault = true;
    p_motor->b_armed = false;
    p_motor->b_stop_confirmed = false;
    if (p_motor->b_prepare_attempted)
    {
        p_motor->b_stop_confirmed =
            p_motor->io.p_stop_all(p_motor->io.p_context);
    }
    for (uint32_t index = 0U; index < DRV_MOTOR_WHEELS; ++index)
    {
        p_motor->channel[index].state = DRV_MOTOR_STOPPED;
        p_motor->channel[index].pending_level = 0U;
    }

    return ((p_motor->b_prepare_attempted && (!p_motor->b_stop_confirmed)) ?
            DRV_MOTOR_HARDWARE : cause);
}

/** @brief Reject disabled/faulty, bad-wheel and nonmonotonic requests. */
static drv_motor_result_t drv_motor_ready (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, uint64_t now_us)
{
    drv_motor_result_t result = DRV_MOTOR_BAD_PARAM;

    if ((NULL != p_motor) && p_motor->b_initialised)
    {
        if (p_motor->b_fault)
        {
            result = DRV_MOTOR_FAULT;
        }
        else if (!p_motor->b_armed)
        {
            result = DRV_MOTOR_DISABLED;
        }
        else if ((DRV_MOTOR_LEFT != wheel) && (DRV_MOTOR_RIGHT != wheel))
        {
            result = drv_motor_fail(p_motor, DRV_MOTOR_BAD_PARAM);
        }
        else if (now_us < p_motor->last_time_us)
        {
            result = drv_motor_fail(p_motor, DRV_MOTOR_BAD_TIME);
        }
        else
        {
            p_motor->last_time_us = now_us;
            result = DRV_MOTOR_OK;
        }
    }

    return (result);
}

/** @brief Accept only board stop-table modes. */
static bool drv_motor_mode_valid (drv_motor_stop_t mode)
{
    return ((DRV_MOTOR_BRAKE == mode) || (DRV_MOTOR_COAST == mode));
}

/** @brief Reject unknown calibration, timing, ownership and limits. */
static bool drv_motor_config_valid (drv_motor_config_t const * p_config)
{
    bool b_valid = (0U != p_config->clock_hz) &&
        (0U != p_config->pwm_hz) && (DRV_MOTOR_MAX_HZ >= p_config->pwm_hz) &&
        (0U != p_config->reversal_guard_us) &&
        (0 != isfinite(p_config->max_duty)) &&
        (p_config->max_duty > 0.0F) && (p_config->max_duty <= 1.0F) &&
        (drv_motor_mode_valid(p_config->zero_mode)) &&
        ((MOTION_CONFIG_CALIBRATION_CONFIRMED == p_config->calibration) ||
         ((MOTION_CONFIG_CALIBRATION_DEVELOPMENT == p_config->calibration) &&
          (p_config->max_duty <= 0.25F))) &&
        p_config->b_resources_verified;

    for (uint32_t index = 0U; index < DRV_MOTOR_WHEELS; ++index)
    {
        b_valid =
            ((MOTION_TYPES_POLARITY_NORMAL == p_config->polarity[index]) ||
            (MOTION_TYPES_POLARITY_REVERSED == p_config->polarity[index])) &&
            b_valid;
    }

    return (b_valid);
}

/** @brief Stop without forgetting drive direction or extending neutral time. */
static drv_motor_result_t drv_motor_neutral (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, drv_motor_stop_t mode, uint64_t now_us)
{
    uint16_t level = (DRV_MOTOR_COAST == mode) ? p_motor->period : 0U;
    drv_motor_channel_t * p_channel = &p_motor->channel[wheel];
    drv_motor_result_t result = DRV_MOTOR_OK;

    if (!(p_motor->io.p_apply(p_motor->io.p_context, wheel, level, level)))
    {
        result = drv_motor_fail(p_motor, DRV_MOTOR_HARDWARE);
    }
    else
    {
        if (!p_channel->b_neutral)
        {
            p_channel->neutral_since_us = now_us;
        }
        p_channel->b_neutral = true;
        p_channel->state = DRV_MOTOR_STOPPED;
        p_channel->pending_level = 0U;
    }

    return (result);
}

/** @brief Apply exactly one electrical direction and commit on success only. */
static drv_motor_result_t drv_motor_drive (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, int8_t direction, uint16_t level)
{
    uint16_t level_a = (direction > 0) ? level : 0U;
    uint16_t level_b = (direction < 0) ? level : 0U;
    drv_motor_result_t result = DRV_MOTOR_OK;

    if (!(p_motor->io.p_apply(p_motor->io.p_context, wheel, level_a, level_b)))
    {
        result = drv_motor_fail(p_motor, DRV_MOTOR_HARDWARE);
    }
    else
    {
        p_motor->channel[wheel].state = DRV_MOTOR_RUNNING;
        p_motor->channel[wheel].last_direction = direction;
        p_motor->channel[wheel].pending_level = 0U;
        p_motor->channel[wheel].b_neutral = false;
        p_motor->b_stop_confirmed = false;
    }

    return (result);
}

/*** end of file ***/

