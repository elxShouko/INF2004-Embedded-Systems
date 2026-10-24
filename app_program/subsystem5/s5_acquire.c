/** @file s5_acquire.c @brief Bounded scan engine with ISR echo timestamps. */
#include "s5_acquire.h"
#include <string.h>
#include <limits.h>

static bool s5_acquire_fault (s5_acquire_t * p_owner);
static bool s5_acquire_position (s5_acquire_t * p_owner, uint64_t now_us);
static bool s5_acquire_sample (s5_acquire_t * p_owner, uint64_t now_us,
    uint32_t width_us, uint64_t captured_us);

bool s5_acquire_validate (s5_acquire_config_t const * p_config)
{
    return ((NULL != p_config) && p_config->b_wiring_confirmed &&
        p_config->b_calibrated && (15U == p_config->servo_gpio) &&
        (p_config->servo_min_us > 0U) &&
        (p_config->servo_max_us > p_config->servo_min_us) &&
        (p_config->servo_period_us > p_config->servo_max_us) &&
        (p_config->echo_us_per_cm > 0U) &&
        (p_config->echo_us_per_cm <= 1000U) &&
        (p_config->echo_timeout_us >=
            (uint32_t)p_config->echo_us_per_cm * S5_MAX_VALID_DISTANCE_CM) &&
        (p_config->echo_timeout_us < 1000000U) &&
        (p_config->sample_period_us > 60000U) &&
        (p_config->sample_period_us > p_config->echo_timeout_us) &&
        (p_config->sample_period_us < 1000000U) &&
        (p_config->servo_settle_us > 0U) &&
        (p_config->servo_settle_us < 1000000U) &&
        (p_config->scan_timeout_us > p_config->sample_period_us) &&
        (p_config->scan_timeout_us > p_config->servo_settle_us) &&
        (p_config->scan_timeout_us < 10000000U) &&
        (p_config->range_delta_cm > 0U));
}

bool s5_acquire_open (s5_acquire_t * p_owner,
    s5_acquire_config_t const * p_config, s5_acquire_io_t const * p_io,
    uint64_t now_us)
{
    if ((NULL == p_owner) || p_owner->b_open ||
        !s5_acquire_validate(p_config) || (NULL == p_io) ||
        (NULL == p_io->p_open) || (NULL == p_io->p_servo) ||
        (NULL == p_io->p_trigger) || (NULL == p_io->p_echo) ||
        (NULL == p_io->p_close) ||
        (now_us > UINT64_MAX - p_config->scan_timeout_us))
    {
        return (false);
    }
    *p_owner = (s5_acquire_t){0};
    p_owner->config = *p_config;
    p_owner->io = *p_io;
    p_owner->last_us = now_us;
    if (!p_io->p_open(p_io->p_context,p_config))
    {
        return (s5_acquire_fault(p_owner));
    }
    p_owner->b_open = true;
    return (s5_acquire_position(p_owner,now_us));
}

bool s5_acquire_step (s5_acquire_t * p_owner, uint64_t now_us,
    bool b_scan_requested, bool b_stationary, bool b_cancel)
{
    if ((NULL == p_owner) || !p_owner->b_open ||
        (S5_ACQUIRE_FAULT == p_owner->state)) { return (false); }
    if ((p_owner->state > S5_ACQUIRE_ECHO) ||
        (p_owner->b_scan &&
         ((p_owner->index >= (p_owner->b_fine ? p_owner->fine_count :
                              S5_COARSE_SCAN_COUNT)) ||
          (p_owner->b_fine && ((p_owner->fine_count < 3U) ||
                              (p_owner->fine_count > S5_FINE_SCAN_MAX_COUNT))))))
    {
        return (s5_acquire_fault(p_owner));
    }
    if ((now_us < p_owner->last_us) ||
        (now_us > UINT64_MAX - p_owner->config.scan_timeout_us) ||
        b_cancel || (p_owner->b_scan && (!b_stationary ||
        (now_us - p_owner->scan_start_us >=
         p_owner->config.scan_timeout_us))))
    {
        p_owner->scan.cancelled = b_cancel;
        return (s5_acquire_fault(p_owner));
    }
    p_owner->last_us = now_us;
    if (b_scan_requested && !p_owner->b_scan)
    {
        if (!b_stationary || (S5_ACQUIRE_ECHO == p_owner->state))
        {
            return (true); /* Owner retries request; never interrupt a pulse. */
        }
        p_owner->scan = (s5_scan_context_t){0};
        p_owner->scan.started_at_ms = (uint32_t)(now_us / 1000U);
        p_owner->scan.max_duration_ms =
            p_owner->config.scan_timeout_us / 1000U;
        p_owner->scan.max_age_ms = p_owner->config.sample_period_us / 1000U;
        p_owner->scan.max_adjacent_range_delta_cm =
            p_owner->config.range_delta_cm;
        p_owner->scan.vehicle_stationary = true;
        p_owner->b_scan_complete = false;
        p_owner->completed_us = 0U;
        p_owner->b_scan = true;
        p_owner->b_fine = false;
        p_owner->index = 0U;
        p_owner->scan_start_us = now_us;
        (void)s5_make_coarse_scan_angles(p_owner->angles,
            S5_FINE_SCAN_MAX_COUNT);
        return (s5_acquire_position(p_owner,now_us));
    }
    if ((S5_ACQUIRE_IDLE == p_owner->state) &&
        (now_us >= p_owner->deadline_us))
    {
        return (s5_acquire_position(p_owner,now_us));
    }
    if ((S5_ACQUIRE_SETTLE == p_owner->state) &&
        (now_us >= p_owner->deadline_us))
    {
        if (!p_owner->io.p_trigger(p_owner->io.p_context))
        {
            p_owner->scan.acquisition_status = S5_ACQUISITION_TRIGGER_FAILED;
            return (s5_acquire_fault(p_owner));
        }
        p_owner->state = S5_ACQUIRE_ECHO;
        p_owner->deadline_us = now_us + p_owner->config.echo_timeout_us;
    }
    if (S5_ACQUIRE_ECHO == p_owner->state)
    {
        uint32_t width = 0U;
        uint64_t captured_us = 0U;
        int32_t result = p_owner->io.p_echo(p_owner->io.p_context,&width,
            &captured_us);

        if ((0 == result) && (now_us >= p_owner->deadline_us))
        {
            p_owner->scan.acquisition_status = S5_ACQUISITION_ECHO_TIMEOUT;
            return (s5_acquire_fault(p_owner));
        }
        if ((result < 0) || (result > 1))
        {
            return (s5_acquire_fault(p_owner));
        }
        if (1 == result)
        {
            if ((captured_us < p_owner->deadline_us -
                 p_owner->config.echo_timeout_us) ||
                (captured_us > p_owner->deadline_us) ||
                ((uint64_t)width > captured_us - (p_owner->deadline_us -
                 p_owner->config.echo_timeout_us)))
            {
                return (s5_acquire_fault(p_owner));
            }
            if (captured_us > now_us)
            {
                /* ISR may finish after this cycle's clock read. The port
                 * retains its capture; consume it on the next cycle rather
                 * than faulting a valid edge. The echo deadline still bounds
                 * acceptance, even when the task is delayed.
                 */
                return (true);
            }
            return (s5_acquire_sample(p_owner,now_us,width,captured_us));
        }
    }
    return (true);
}

void s5_acquire_close (s5_acquire_t * p_owner)
{
    if ((NULL != p_owner) && p_owner->b_open)
    {
        p_owner->io.p_close(p_owner->io.p_context);
        p_owner->b_open = false;
        p_owner->b_forward_valid = false;
        p_owner->state = S5_ACQUIRE_FAULT;
    }
}

static bool s5_acquire_fault (s5_acquire_t * p_owner)
{
    s5_acquire_close(p_owner);
    p_owner->state = S5_ACQUIRE_FAULT;
    p_owner->b_forward_valid = false;
    if (S5_ACQUISITION_OK == p_owner->scan.acquisition_status)
    {
        p_owner->scan.acquisition_status = S5_ACQUISITION_ABORTED;
    }
    return (false);
}

static bool s5_acquire_position (s5_acquire_t * p_owner, uint64_t now_us)
{
    uint8_t angle = p_owner->b_scan ? p_owner->angles[p_owner->index] : 90U;

    if (!p_owner->io.p_servo(p_owner->io.p_context,angle))
    {
        p_owner->scan.acquisition_status = S5_ACQUISITION_SERVO_POSITION_FAILED;
        return (s5_acquire_fault(p_owner));
    }
    p_owner->state = S5_ACQUIRE_SETTLE;
    p_owner->deadline_us = now_us + p_owner->config.servo_settle_us;
    if (p_owner->next_trigger_us > p_owner->deadline_us)
    {
        p_owner->deadline_us = p_owner->next_trigger_us;
    }
    return (true);
}

static bool s5_acquire_sample (s5_acquire_t * p_owner, uint64_t now_us,
    uint32_t width_us, uint64_t captured_us)
{
    uint32_t cm = width_us / p_owner->config.echo_us_per_cm;

    /* Check the physical range before flooring to whole cm. In particular,
     * max_range * scale + 1 must not round back into the accepted range.
     */
    if ((width_us < (uint32_t)p_owner->config.echo_us_per_cm *
                    S5_MIN_VALID_DISTANCE_CM) ||
        (width_us > (uint32_t)p_owner->config.echo_us_per_cm *
                    S5_MAX_VALID_DISTANCE_CM) ||
        (width_us > p_owner->config.echo_timeout_us) ||
        (cm < S5_MIN_VALID_DISTANCE_CM) || (cm > S5_MAX_VALID_DISTANCE_CM))
    {
        return (s5_acquire_fault(p_owner));
    }
    p_owner->next_trigger_us = captured_us + p_owner->config.sample_period_us;
    if (!p_owner->b_scan)
    {
        p_owner->forward_cm = (uint16_t)cm;
        p_owner->forward_us = captured_us;
        p_owner->forward_started_us = p_owner->deadline_us -
            p_owner->config.echo_timeout_us;
        p_owner->b_forward_valid = true;
        p_owner->state = S5_ACQUIRE_IDLE;
        p_owner->deadline_us = now_us + p_owner->config.sample_period_us;
        return (true);
    }
    s5_scan_sample_t sample = {p_owner->angles[p_owner->index],
        (uint16_t)cm,(uint32_t)(captured_us / 1000U),true};

    if (p_owner->b_fine) { p_owner->fine[p_owner->index] = sample; }
    else { p_owner->coarse[p_owner->index] = sample; }
    ++p_owner->index;
    if (!p_owner->b_fine && (S5_COARSE_SCAN_COUNT == p_owner->index))
    {
        size_t closest = 0U;
        size_t fine_count = 0U;

        for (size_t index = 1U; index < S5_COARSE_SCAN_COUNT; ++index)
        {
            if (p_owner->coarse[index].distance_cm <
                p_owner->coarse[closest].distance_cm) { closest = index; }
        }
        if (S5_LOGIC_OK != s5_make_fine_scan_angles(
            p_owner->coarse[closest].angle_deg,p_owner->angles,
            S5_FINE_SCAN_MAX_COUNT,&fine_count) ||
            (fine_count < 3U) || (fine_count > S5_FINE_SCAN_MAX_COUNT))
        {
            return (s5_acquire_fault(p_owner));
        }
        p_owner->fine_count = (uint8_t)fine_count;
        p_owner->b_fine = true;
        p_owner->index = 0U;
    }
    else if (p_owner->b_fine && (p_owner->fine_count == p_owner->index))
    {
        p_owner->scan.complete = true;
        p_owner->scan.servo_settled = true;
        p_owner->scan.completed_at_ms = (uint32_t)(now_us / 1000U);
        p_owner->scan.evaluated_at_ms = p_owner->scan.completed_at_ms;
        p_owner->b_scan_complete = true;
        p_owner->completed_us = now_us;
        p_owner->b_scan = false;
    }
    return (s5_acquire_position(p_owner,now_us));
}
/*** end of file ***/
