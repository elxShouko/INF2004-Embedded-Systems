/** @file wheel_loop.c @brief Bounded single-owner feedback loop. */
#include "wheel_loop.h"
#include <limits.h>
#include <stddef.h>

static bool wheel_loop_dummy_prepare (void * p_context, uint16_t period,
    uint32_t clock);
static bool wheel_loop_dummy_apply (void * p_context, drv_motor_wheel_t wheel,
                                    uint16_t a, uint16_t b);
static bool wheel_loop_dummy_stop (void * p_context);
static int32_t wheel_loop_slew (wheel_loop_t * p_loop, uint32_t wheel,
                                int32_t desired, uint32_t elapsed_us);

bool wheel_loop_validate (wheel_loop_config_t const * p_config)
{
    drv_motor_t motor = {0};
    drv_motor_io_t io = {NULL,wheel_loop_dummy_prepare,wheel_loop_dummy_apply,
                         wheel_loop_dummy_stop};
    bool b_ok = (NULL != p_config);

    if (b_ok)
    {
        b_ok = (MOTION_CONFIG_VALID == motion_config_validate(
            &p_config->geometry)) &&
            (DRV_MOTOR_OK == drv_motor_init(&motor,&p_config->motor,&io)) &&
            move_plan_validate(&p_config->profile,
                p_config->geometry.max_wheel_speed_mm_s) &&
            (p_config->motor.polarity[0] ==
             p_config->geometry.left.motor_polarity) &&
            (p_config->motor.polarity[1] ==
             p_config->geometry.right.motor_polarity) &&
            (p_config->period_us >= 10000U) &&
            (p_config->period_us <= p_config->sample_limit_us) &&
            (p_config->stop_dwell_us >= p_config->period_us) &&
            (p_config->stop_timeout_us > p_config->stop_dwell_us) &&
            (p_config->progress_timeout_us > p_config->sample_limit_us) &&
            (p_config->stop_speed_mm_s >= 0) &&
            (p_config->stop_speed_mm_s <=
             p_config->geometry.max_wheel_speed_mm_s) &&
            (p_config->overspeed_mm_s >=
             p_config->geometry.max_wheel_speed_mm_s);
        for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
        {
            b_ok = (MOTION_PID_OK == motion_pid_validate(
                &p_config->pid[wheel])) &&
                (p_config->pid[wheel].max_elapsed_us >=
                 p_config->sample_limit_us) && b_ok;
        }
        if (MOTION_CONFIG_CALIBRATION_DEVELOPMENT ==
            p_config->geometry.calibration)
        {
            b_ok = b_ok &&
                (MOTION_CONFIG_CALIBRATION_DEVELOPMENT == p_config->motor.calibration) &&
                (MOTION_CONFIG_CALIBRATION_DEVELOPMENT == p_config->pid[0].calibration) &&
                (MOTION_CONFIG_CALIBRATION_DEVELOPMENT == p_config->pid[1].calibration);
        }
    }
    return (b_ok);
}

bool wheel_loop_init (wheel_loop_t * p_loop,
    wheel_loop_config_t const * p_config, drv_motor_io_t const * p_io)
{
    bool b_ok = (NULL != p_loop) && wheel_loop_validate(p_config);

    if (b_ok)
    {
        wheel_loop_t candidate = {0};

        candidate.config = *p_config;
        b_ok = (DRV_MOTOR_OK == drv_motor_init(&candidate.motor,
            &p_config->motor,p_io));
        for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
        {
            b_ok = (MOTION_ESTIMATOR_OK == motion_estimator_init(
                &candidate.estimator[wheel],
                &p_config->geometry,(motion_estimator_wheel_t)wheel,
                    p_config->sample_limit_us)) && b_ok;
            b_ok = (MOTION_PID_OK == motion_pid_init(&candidate.pid[wheel],
                &p_config->pid[wheel])) && b_ok;
        }
        if (b_ok)
        {
            *p_loop = candidate;
        }
    }
    return (b_ok);
}

bool wheel_loop_enable (wheel_loop_t * p_loop, uint64_t now_us)
{
    bool b_ok = (NULL != p_loop) && !p_loop->b_enabled && !p_loop->b_fault &&
                p_loop->motor.b_initialised;

    if (b_ok)
    {
        b_ok = (DRV_MOTOR_OK == drv_motor_arm(&p_loop->motor,now_us));
        p_loop->b_enabled = b_ok;
        p_loop->last_tick_us = now_us;
        if (!b_ok)
        {
            wheel_loop_fault(p_loop);
        }
    }
    return (b_ok);
}

bool wheel_loop_measure (wheel_loop_t * p_loop,
    enc_capture_sample_t const * p_sample, uint64_t now_us)
{
    bool b_ok = (NULL != p_loop) && (NULL != p_sample) &&
        p_loop->b_enabled && !p_loop->b_fault && p_sample->b_healthy;
    bool b_valid = true;

    if (b_ok)
    {
        b_ok = (p_sample->sample[0].timestamp_us ==
                p_sample->sample[1].timestamp_us);
        for (uint32_t wheel = 0U; b_ok && (wheel < 2U); ++wheel)
        {
            motion_estimator_result_t result = motion_estimator_update(
                &p_loop->estimator[wheel],
                &p_sample->sample[wheel],now_us,&p_loop->measured[wheel]);
            int64_t speed = p_loop->measured[wheel].speed_mm_s;
            bool b_slow = (speed >= -(int64_t)p_loop->config.stop_speed_mm_s) &&
                          (speed <= p_loop->config.stop_speed_mm_s);

            b_ok = ((MOTION_ESTIMATOR_OK == result) || (
                MOTION_ESTIMATOR_BASELINE == result)) &&
                (speed >= -(int64_t)p_loop->config.overspeed_mm_s) &&
                (speed <= p_loop->config.overspeed_mm_s);
            b_valid = p_loop->measured[wheel].b_speed_valid && b_valid;
            if ((!p_loop->b_quiet[wheel]) || !b_slow ||
                (p_sample->sample[wheel].count != p_loop->last_count[wheel]))
            {
                p_loop->quiet_since_us[wheel] = now_us;
            }
            p_loop->b_quiet[wheel] = b_slow;
            if (p_sample->sample[wheel].count != p_loop->last_count[wheel])
            {
                p_loop->progress_since_us[wheel] = now_us;
            }
            p_loop->last_count[wheel] = p_sample->sample[wheel].count;
            p_loop->b_standstill[wheel] =
                p_loop->measured[wheel].b_speed_valid && b_slow &&
                (now_us >= p_loop->quiet_since_us[wheel]) &&
                ((now_us-p_loop->quiet_since_us[wheel]) >=
                 p_loop->config.stop_dwell_us);
        }
        p_loop->b_valid = b_ok && b_valid;
    }
    if (!b_ok && (NULL != p_loop))
    {
        wheel_loop_fault(p_loop);
    }
    return (b_ok);
}

bool wheel_loop_drive (wheel_loop_t * p_loop, int32_t const desired_mm_s[2],
                       uint32_t elapsed_us, uint64_t now_us)
{
    bool b_ok = (NULL != p_loop) && (NULL != desired_mm_s) &&
        p_loop->b_enabled && !p_loop->b_fault && p_loop->b_valid &&
        (elapsed_us > 0U) && (elapsed_us <= p_loop->config.sample_limit_us);

    for (uint32_t wheel = 0U; b_ok && (wheel < 2U); ++wheel)
    {
        int64_t desired = desired_mm_s[wheel];
        int64_t limit = p_loop->config.geometry.max_wheel_speed_mm_s;

        b_ok = (desired >= -limit) && (desired <= limit) &&
            (now_us >= p_loop->measured[wheel].timestamp_us) &&
            ((now_us-p_loop->measured[wheel].timestamp_us) <=
             p_loop->config.sample_limit_us);
    }
    for (uint32_t wheel = 0U; b_ok && (wheel < 2U); ++wheel)
    {
        int32_t target = wheel_loop_slew(p_loop,wheel,desired_mm_s[wheel],
            elapsed_us);
        motion_pid_value_t effort = 0.0F;
        drv_motor_result_t motor_result = DRV_MOTOR_OK;

        if (0 == target)
        {
            p_loop->b_tracking[wheel] = false;
            (void)motion_pid_reset(&p_loop->pid[wheel]);
            motor_result = drv_motor_stop(&p_loop->motor,(
                drv_motor_wheel_t)wheel,
                                           DRV_MOTOR_BRAKE,now_us);
        }
        else
        {
            if (!p_loop->b_tracking[wheel])
            {
                p_loop->progress_since_us[wheel] = now_us;
                p_loop->b_tracking[wheel] = true;
            }
            b_ok = (now_us >= p_loop->progress_since_us[wheel]) &&
                ((now_us-p_loop->progress_since_us[wheel]) <
                 p_loop->config.progress_timeout_us);
            if (DRV_MOTOR_REVERSING == p_loop->motor.channel[wheel].state)
            {
                b_ok = b_ok && (
                    now_us >= p_loop->motor.channel[wheel].neutral_since_us) &&
                    ((now_us-p_loop->motor.channel[wheel].neutral_since_us) <
                     p_loop->config.stop_timeout_us);
                (void)motion_pid_reset(&p_loop->pid[wheel]);
                /* Do not accumulate a large ramp while held in BRAKE. */
                p_loop->velocity_um_s[wheel] = 0;
                p_loop->slew_remainder[wheel] = 0U;
            }
            b_ok = b_ok && (MOTION_PID_OK == motion_pid_update(
                &p_loop->pid[wheel],target,
                p_loop->measured[wheel].speed_mm_s,elapsed_us,true,&effort));
            if (b_ok)
            {
                motor_result = drv_motor_set(&p_loop->motor,(
                    drv_motor_wheel_t)wheel,effort,now_us);
                if (DRV_MOTOR_WAIT == motor_result)
                {
                    motor_result = drv_motor_service(&p_loop->motor,(
                        drv_motor_wheel_t)wheel,
                        now_us,p_loop->b_standstill[wheel]);
                }
            }
        }
        b_ok = b_ok && ((DRV_MOTOR_OK == motor_result) || (
            DRV_MOTOR_WAIT == motor_result));
    }
    if (!b_ok && (NULL != p_loop))
    {
        wheel_loop_fault(p_loop);
    }
    return (b_ok);
}

bool wheel_loop_brake (wheel_loop_t * p_loop, uint64_t now_us)
{
    bool b_ok = (NULL != p_loop) && p_loop->b_enabled && !p_loop->b_fault;

    if (b_ok)
    {
        for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
        {
            b_ok = (DRV_MOTOR_OK == drv_motor_stop(&p_loop->motor,(
                drv_motor_wheel_t)wheel,
                                                   DRV_MOTOR_BRAKE,
                                                       now_us)) && b_ok;
            (void)motion_pid_reset(&p_loop->pid[wheel]);
            p_loop->velocity_um_s[wheel] = 0;
            p_loop->slew_remainder[wheel] = 0U;
            p_loop->b_tracking[wheel] = false;
        }
        if (!b_ok)
        {
            wheel_loop_fault(p_loop);
        }
    }
    return (b_ok);
}

void wheel_loop_fault (wheel_loop_t * p_loop)
{
    if (NULL != p_loop)
    {
        if (p_loop->motor.b_initialised && p_loop->motor.b_prepare_attempted)
        {
            (void)drv_motor_disarm(&p_loop->motor);
        }
        p_loop->b_fault = true;
        p_loop->b_enabled = false;
        p_loop->b_valid = false;
        p_loop->b_standstill[0] = false;
        p_loop->b_standstill[1] = false;
    }
}

static bool wheel_loop_dummy_prepare (void * p_context, uint16_t period,
    uint32_t clock)
{
    (void)p_context; (void)period; (void)clock;
    return (false); /* Validation must never invoke backend callbacks. */
}

static bool wheel_loop_dummy_apply (void * p_context, drv_motor_wheel_t wheel,
                                    uint16_t a, uint16_t b)
{
    (void)p_context; (void)wheel; (void)a; (void)b;
    return (false);
}

static bool wheel_loop_dummy_stop (void * p_context)
{
    (void)p_context;
    return (false);
}

static int32_t wheel_loop_slew (wheel_loop_t * p_loop, uint32_t wheel,
                                int32_t desired, uint32_t elapsed_us)
{
    int64_t current = p_loop->velocity_um_s[wheel];
    int64_t target = (int64_t)desired*1000;
    uint32_t rate = p_loop->config.profile.decel_mm_s2;
    bool b_reversing = ((current > 0) && (target < 0)) ||
                       ((current < 0) && (target > 0));

    if (b_reversing)
    {
        target = 0; /* Cross through zero on a separate update. */
    }
    else if (((current >= 0) && (target > current)) ||
             ((current <= 0) && (target < current)))
    {
        rate = p_loop->config.profile.accel_mm_s2;
    }
    uint64_t numerator = ((
        uint64_t)rate*elapsed_us)+p_loop->slew_remainder[wheel];
    int64_t step = (int64_t)(numerator/1000U);
    int64_t difference = target-current;

    p_loop->slew_remainder[wheel] = numerator%1000U;
    if (((difference >= 0) && (difference <= step)) ||
        ((difference < 0) && (-difference <= step)))
    {
        current = target;
        p_loop->slew_remainder[wheel] = 0U;
    }
    else
    {
        current += (difference > 0) ? step : -step;
    }
    p_loop->velocity_um_s[wheel] = current;
    return ((int32_t)(current/1000));
}

/*** end of file ***/
