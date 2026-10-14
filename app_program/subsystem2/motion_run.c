/** @file motion_run.c @brief Owner-only command and stopping state machine. */
#include "motion_run.h"
#include <stddef.h>

static void motion_run_finish (motion_run_t * p_run,
    motion_run_result_t result);
static bool motion_run_at_goal (motion_run_t const * p_run);
static void motion_run_publish (motion_run_t * p_run, uint64_t now_us);

bool motion_run_init (motion_run_t * p_run,
    wheel_loop_config_t const * p_config, drv_motor_io_t const * p_io)
{
    bool b_ok = (NULL != p_run);

    if (b_ok)
    {
        motion_run_t candidate = {0};

        b_ok = wheel_loop_init(&candidate.loop,p_config,p_io);
        if (b_ok)
        {
            candidate.b_initialised = true;
            candidate.status.state = MOTION_RUN_CONFIGURED;
            *p_run = candidate;
        }
    }
    return (b_ok);
}

bool motion_run_enable (motion_run_t * p_run, uint64_t now_us)
{
    bool b_ok = (NULL != p_run) && p_run->b_initialised &&
                (MOTION_RUN_CONFIGURED == p_run->status.state);

    if (b_ok)
    {
        b_ok = wheel_loop_enable(&p_run->loop,now_us);
        if (b_ok)
        {
            p_run->status.state = MOTION_RUN_SETTLING;
            p_run->stopping_since_us = now_us;
            motion_run_publish(p_run,now_us);
        }
        else
        {
            motion_run_emergency(p_run,MOTION_RUN_OUTPUT,now_us);
        }
    }
    return (b_ok);
}

bool motion_run_command (motion_run_t * p_run,
    move_plan_command_t const * p_command, uint64_t now_us)
{
    bool b_ok = (NULL != p_run) && (NULL != p_command) &&
        p_run->b_initialised && p_run->loop.b_enabled && !p_run->loop.b_fault &&
        p_run->status.b_measurement_valid && (
            0U == p_run->status.terminal_id) &&
        (now_us >= p_run->status.measurement_timestamp_us) &&
        ((now_us-p_run->status.measurement_timestamp_us) <=
         p_run->loop.config.sample_limit_us);

    if (b_ok)
    {
        bool b_new = (MOTION_RUN_READY == p_run->status.state) &&
            (p_command->id > p_run->last_id) && p_run->status.b_standstill;
        bool b_refresh = (MOTION_RUN_ACTIVE == p_run->status.state) &&
            (MOVE_PLAN_SPEED == p_run->status.active_kind) &&
            (MOVE_PLAN_SPEED == p_command->kind) &&
            (p_command->id == p_run->status.active_id) &&
            (now_us < p_run->deadline_us) &&
            (p_command->issued_us > p_run->plan.command.issued_us);
        move_plan_t candidate = {0};

        b_ok = (b_new || b_refresh) && move_plan_begin(&candidate,p_command,
            &p_run->loop.config.profile,&p_run->loop.config.geometry,
            p_run->status.position_um,now_us);
        if (b_ok)
        {
            p_run->plan = candidate;
            p_run->deadline_us = p_command->issued_us+p_command->lifetime_us;
            p_run->status.active_id = p_command->id;
            p_run->status.active_kind = p_command->kind;
            p_run->status.state = MOTION_RUN_ACTIVE;
            p_run->last_id = p_command->id;
            p_run->pending_result = MOTION_RUN_NONE;
        }
    }
    if ((NULL != p_run) && (NULL != p_command))
    {
        p_run->status.last_request_id = p_command->id;
        p_run->status.last_request_issued_us = p_command->issued_us;
        p_run->status.b_request_accepted = b_ok;
    }
    return (b_ok);
}

bool motion_run_ack (motion_run_t * p_run, uint32_t id)
{
    bool b_ok = (NULL != p_run) && (id > 0U) &&
                (id == p_run->status.terminal_id);

    if (b_ok)
    {
        p_run->status.terminal_id = 0U;
        p_run->status.terminal_result = MOTION_RUN_NONE;
    }
    return (b_ok);
}

void motion_run_stop (motion_run_t * p_run, bool b_cancel, uint64_t now_us)
{
    if ((NULL != p_run) && p_run->loop.b_enabled &&
        (MOTION_RUN_FAULT != p_run->status.state))
    {
        if (now_us < p_run->loop.last_tick_us)
        {
            motion_run_emergency(p_run,MOTION_RUN_SENSOR,now_us);
        }
        else if (!wheel_loop_brake(&p_run->loop,now_us))
        {
            motion_run_emergency(p_run,MOTION_RUN_OUTPUT,now_us);
        }
        else if (MOTION_RUN_STOPPING != p_run->status.state)
        {
            /* Repeated stop requests cannot extend the stopping deadline. */
            p_run->pending_result = b_cancel ? MOTION_RUN_CANCELLED :
                                              MOTION_RUN_STOPPED;
            p_run->status.state = MOTION_RUN_STOPPING;
            p_run->stopping_since_us = now_us;
        }
        motion_run_publish(p_run,now_us);
    }
}

void motion_run_emergency (motion_run_t * p_run, motion_run_result_t cause,
                           uint64_t now_us)
{
    if (NULL != p_run)
    {
        wheel_loop_fault(&p_run->loop);
        if (MOTION_RUN_FAULT != p_run->status.state)
        {
            p_run->status.fault_result = cause;
            motion_run_finish(p_run,cause);
        }
        p_run->status.state = MOTION_RUN_FAULT;
        p_run->status.target_mm_s[0] = 0;
        p_run->status.target_mm_s[1] = 0;
        motion_run_publish(p_run,now_us);
    }
}

bool motion_run_tick (motion_run_t * p_run,
    enc_capture_sample_t const * p_sample, uint64_t now_us)
{
    bool b_ok = (NULL != p_run) && p_run->b_initialised &&
                p_run->loop.b_enabled && (
                    MOTION_RUN_FAULT != p_run->status.state);

    /* Invalid capture must stop even before the next PID deadline. */
    if (b_ok && ((NULL == p_sample) || !p_sample->b_healthy ||
        (p_sample->sample[0].timestamp_us !=
         p_sample->sample[1].timestamp_us) ||
        (p_sample->sample[0].timestamp_us > now_us) ||
        ((now_us-p_sample->sample[0].timestamp_us) >
         p_run->loop.config.sample_limit_us)))
    {
        motion_run_emergency(p_run,MOTION_RUN_SENSOR,now_us);
        b_ok = false;
    }
    if (b_ok && ((now_us < p_run->loop.last_tick_us) ||
        (now_us < p_run->status.timestamp_us)))
    {
        motion_run_emergency(p_run,MOTION_RUN_SENSOR,now_us);
        b_ok = false;
    }
    if (b_ok && (MOTION_RUN_ACTIVE == p_run->status.state) &&
        (now_us >= p_run->deadline_us))
    {
        if (p_run->plan.b_finite)
        {
            motion_run_emergency(p_run,MOTION_RUN_TIMEOUT,now_us);
            b_ok = false;
        }
        else
        {
            motion_run_stop(p_run,false,now_us);
            p_run->pending_result = MOTION_RUN_EXPIRED;
        }
    }
    if (b_ok)
    {
        uint64_t elapsed = now_us-p_run->loop.last_tick_us;
        bool b_baseline = !p_run->loop.estimator[0].b_have_baseline;

        if (b_baseline || (elapsed >= p_run->loop.config.period_us))
        {
            if ((elapsed > p_run->loop.config.sample_limit_us) ||
                !wheel_loop_measure(&p_run->loop,p_sample,now_us))
            {
                motion_run_emergency(p_run,MOTION_RUN_SENSOR,now_us);
                b_ok = false;
            }
            else
            {
                p_run->loop.last_tick_us = now_us;
                motion_run_publish(p_run,now_us);
                if ((MOTION_RUN_SETTLING == p_run->status.state) ||
                    (MOTION_RUN_STOPPING == p_run->status.state))
                {
                    if (((now_us-p_run->stopping_since_us) >=
                         p_run->loop.config.stop_timeout_us) ||
                        ((MOTION_RUN_DONE == p_run->pending_result) &&
                         (now_us >= p_run->deadline_us)))
                    {
                        motion_run_emergency(p_run,MOTION_RUN_TIMEOUT,now_us);
                    }
                    else if (p_run->status.b_standstill)
                    {
                        if ((MOTION_RUN_DONE == p_run->pending_result) &&
                            !motion_run_at_goal(p_run))
                        {
                            motion_run_emergency(p_run,MOTION_RUN_OVERSHOOT,
                                now_us);
                        }
                        else
                        {
                            if (MOTION_RUN_STOPPING == p_run->status.state)
                            {
                                motion_run_finish(p_run,p_run->pending_result);
                            }
                            p_run->status.state = MOTION_RUN_READY;
                        }
                    }
                }
                else if ((MOTION_RUN_ACTIVE == p_run->status.state)
                    && p_run->loop.b_valid)
                {
                    int32_t target[2] = {0,0};
                    bool b_arrived = false;

                    if (!move_plan_step(&p_run->plan,
                        &p_run->loop.config.profile,
                        p_run->status.position_um,target,&b_arrived))
                    {
                        motion_run_emergency(p_run,MOTION_RUN_OVERSHOOT,now_us);
                    }
                    else if (b_arrived)
                    {
                        motion_run_stop(p_run,false,now_us);
                        p_run->pending_result = MOTION_RUN_DONE;
                    }
                    else if (!wheel_loop_drive(&p_run->loop,target,(
                        uint32_t)elapsed,now_us))
                    {
                        motion_run_emergency(p_run,MOTION_RUN_OUTPUT,now_us);
                    }
                }
                motion_run_publish(p_run,now_us);
            }
        }
        b_ok = (MOTION_RUN_FAULT != p_run->status.state);
    }
    return (b_ok);
}

static void motion_run_finish (motion_run_t * p_run, motion_run_result_t result)
{
    if (0U == p_run->status.terminal_id)
    {
        p_run->status.terminal_id = p_run->status.active_id;
        p_run->status.terminal_result = result;
    }
    p_run->status.active_id = 0U;
    p_run->pending_result = MOTION_RUN_NONE;
}

static bool motion_run_at_goal (motion_run_t const * p_run)
{
    int32_t unused[2] = {0,0};
    bool b_arrived = false;

    return (move_plan_step(&p_run->plan,&p_run->loop.config.profile,
        p_run->status.position_um,unused,&b_arrived) && b_arrived);
}

static void motion_run_publish (motion_run_t * p_run, uint64_t now_us)
{
    p_run->status.timestamp_us = now_us;
    p_run->status.measurement_timestamp_us =
        p_run->loop.measured[0].timestamp_us;
    p_run->status.measurement_max_age_us = p_run->loop.config.sample_limit_us;
    p_run->status.b_measurement_valid = p_run->loop.b_valid;
    p_run->status.b_standstill = p_run->loop.b_valid &&
        p_run->loop.b_standstill[0] && p_run->loop.b_standstill[1];
    p_run->status.b_stop_command_confirmed =
        p_run->loop.motor.b_stop_confirmed ||
        (p_run->loop.b_enabled && !p_run->loop.b_fault &&
         p_run->loop.motor.channel[0].b_neutral &&
         p_run->loop.motor.channel[1].b_neutral);
    for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
    {
        p_run->status.position_um[wheel] =
            p_run->loop.measured[wheel].position_um;
        p_run->status.speed_mm_s[wheel] = p_run->loop.b_valid ?
            p_run->loop.measured[wheel].speed_mm_s : 0;
        p_run->status.target_mm_s[wheel] = p_run->loop.b_enabled ?
            (int32_t)(p_run->loop.velocity_um_s[wheel]/1000) : 0;
    }
}

/*** end of file ***/
