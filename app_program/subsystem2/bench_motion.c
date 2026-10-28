/** @file bench_motion.c @brief One-shot electrical direction test. */
#include "bench_motion.h"
#include <stddef.h>

static void motion_bench_stop (motion_bench_t * p_bench, bool b_fault);

bool motion_bench_init (motion_bench_t * p_bench,
    motion_bench_config_t const * p_config, drv_motor_io_t const * p_io)
{
    if ((NULL == p_bench) || (NULL == p_config) || (NULL == p_io))
    {
        return (false);
    }
    *p_bench = (motion_bench_t){0};
    p_bench->config = *p_config;
    /* Invalid/unknown motor settings still permit encoder-only operation. */
    p_bench->b_initialised = (DRV_MOTOR_OK ==
        drv_motor_init(&p_bench->motor, &p_config->motor, p_io));
    return (true);
}

bool motion_bench_arm (motion_bench_t * p_bench, uint64_t now_us,
    bool b_encoder_healthy, bool b_lease)
{
    if ((NULL == p_bench) || !p_bench->b_initialised ||
        p_bench->status.b_fault || p_bench->status.b_used ||
        p_bench->status.b_armed || !b_encoder_healthy || !b_lease ||
        !p_bench->config.b_power_authorised ||
        !p_bench->config.b_wheels_raised ||
        (0U == p_bench->config.pulse_us) ||
        (p_bench->config.pulse_us > MOTION_BENCH_MAX_PULSE_US) ||
        (0U != (p_bench->config.pulse_us % MOTION_BENCH_POLL_US)) ||
        (now_us < p_bench->previous_us) ||
        (now_us > UINT64_MAX - MOTION_BENCH_ARM_US))
    {
        return (false);
    }
    bool b_nonzero = false;
    for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
    {
        int32_t effort = p_bench->config.effort_permille[wheel];
        if ((effort < -1000) || (effort > 1000)) { return (false); }
        b_nonzero = b_nonzero || (0 != effort);
    }
    if (!b_nonzero) { return (false); }
    if (DRV_MOTOR_OK != drv_motor_arm(&p_bench->motor, now_us))
    {
        motion_bench_stop(p_bench, true);
        return (false);
    }
    p_bench->status.b_armed = true;
    p_bench->status.deadline_us = now_us + MOTION_BENCH_ARM_US;
    p_bench->previous_us = now_us;
    return (true);
}

bool motion_bench_pulse (motion_bench_t * p_bench, uint64_t now_us,
    bool b_encoder_healthy, bool b_lease)
{
    if ((NULL == p_bench) || !p_bench->status.b_armed ||
        p_bench->status.b_fault || p_bench->status.b_used ||
        !b_encoder_healthy || !b_lease ||
        (now_us < p_bench->previous_us) ||
        (now_us >= p_bench->status.deadline_us) ||
        (now_us > UINT64_MAX - p_bench->config.pulse_us))
    {
        return (false);
    }
    /* Consume before touching either wheel; no reversal or repeat. */
    p_bench->status.b_used = true;
    p_bench->status.deadline_us = now_us + p_bench->config.pulse_us;
    for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
    {
        motion_pid_value_t effort =
            (motion_pid_value_t)p_bench->config.effort_permille[wheel] /
                1000.0F;
        if (DRV_MOTOR_OK != drv_motor_set(&p_bench->motor,
            (drv_motor_wheel_t)wheel, effort, now_us))
        {
            motion_bench_stop(p_bench, true);
            return (false);
        }
        p_bench->status.effort_permille[wheel] =
            p_bench->config.effort_permille[wheel];
    }
    p_bench->status.b_running = true;
    p_bench->previous_us = now_us;
    return (true);
}

void motion_bench_tick (motion_bench_t * p_bench, uint64_t now_us,
    bool b_encoder_healthy, bool b_lease, bool b_stop, bool b_emergency)
{
    if (NULL == p_bench) { return; }
    bool b_bad_time = (now_us < p_bench->previous_us);
    bool b_active = p_bench->status.b_armed || p_bench->status.b_running;
    if (b_emergency || b_bad_time || (b_active &&
        (b_stop || !b_encoder_healthy || !b_lease ||
         (now_us >= p_bench->status.deadline_us))))
    {
        motion_bench_stop(p_bench, b_emergency || b_bad_time ||
            !b_encoder_healthy || !b_lease);
    }
    p_bench->previous_us = now_us;
    p_bench->status.timestamp_us = now_us;
}

static void motion_bench_stop (motion_bench_t * p_bench, bool b_fault)
{
    if (p_bench->motor.b_initialised && p_bench->motor.b_prepare_attempted)
    {
        if (DRV_MOTOR_OK != drv_motor_disarm(&p_bench->motor))
        {
            b_fault = true;
        }
    }
    p_bench->status.b_fault = p_bench->status.b_fault || b_fault;
    p_bench->status.b_used = p_bench->status.b_used ||
        p_bench->status.b_armed;
    p_bench->status.b_armed = false;
    p_bench->status.b_running = false;
    p_bench->status.pwm_level[0] = 0U;
    p_bench->status.pwm_level[1] = 0U;
    p_bench->status.effort_permille[0] = 0;
    p_bench->status.effort_permille[1] = 0;
}

/*** end of file ***/
