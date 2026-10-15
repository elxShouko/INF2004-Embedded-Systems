/** @file move_plan.c @brief Pure relative wheel-travel planning. */
#include "move_plan.h"
#include <limits.h>
#include <math.h>
#include <stddef.h>

static uint64_t move_plan_sqrt (uint64_t value);
static bool move_plan_delta (int64_t position, int64_t origin,
    int64_t * p_delta);

bool move_plan_validate (move_plan_config_t const * p_config, int32_t max_speed)
{
    return ((NULL != p_config) && (max_speed > 0) &&
        (p_config->accel_mm_s2 > 0U) &&
        (p_config->accel_mm_s2 <= INT32_MAX) &&
        (p_config->decel_mm_s2 > 0U) &&
        (p_config->decel_mm_s2 <= INT32_MAX) &&
        (p_config->tolerance_um > 0U) &&
        (p_config->tolerance_um <= INT32_MAX) &&
        (p_config->turn_scale_ppm[0] > 0U) &&
        (p_config->turn_scale_ppm[1] > 0U) &&
        isfinite(p_config->correction_gain) &&
        (p_config->correction_gain >= 0.0F) &&
        (p_config->correction_limit_mm_s >= 0) &&
        (p_config->correction_limit_mm_s <= max_speed));
}

bool move_plan_begin (move_plan_t * p_plan,
    move_plan_command_t const * p_command,
    move_plan_config_t const * p_config, motion_config_t const * p_geometry,
    int64_t const position_um[2], uint64_t now_us)
{
    move_plan_t candidate = {0};
    bool b_ok = (NULL != p_plan) && (NULL != p_command) &&
        (NULL != position_um) &&
        (MOTION_CONFIG_VALID == motion_config_validate(p_geometry));

    if (b_ok)
    {
        b_ok = move_plan_validate(p_config,p_geometry->max_wheel_speed_mm_s) &&
            (p_command->id > 0U) && (p_command->lifetime_us > 0U) &&
            (p_command->issued_us <= now_us) &&
            ((now_us-p_command->issued_us) < p_command->lifetime_us) &&
            (p_command->issued_us <= (UINT64_MAX-p_command->lifetime_us));
    }
    if (b_ok)
    {
        int64_t max_speed = p_geometry->max_wheel_speed_mm_s;

        candidate.command = *p_command;
        candidate.origin_um[0] = position_um[0];
        candidate.origin_um[1] = position_um[1];
        candidate.b_finite = (MOVE_PLAN_SPEED != p_command->kind);
        switch (p_command->kind)
        {
            case MOVE_PLAN_SPEED:
                b_ok = ((int64_t)p_command->left_mm_s >= -max_speed) &&
                    ((int64_t)p_command->left_mm_s <= max_speed) &&
                    ((int64_t)p_command->right_mm_s >= -max_speed) &&
                    ((int64_t)p_command->right_mm_s <= max_speed);
            break;
            case MOVE_PLAN_TRAVEL:
                candidate.goal_um[0] = p_command->distance_um;
                candidate.goal_um[1] = p_command->distance_um;
            break;
            case MOVE_PLAN_LEFT:
            case MOVE_PLAN_RIGHT:
            case MOVE_PLAN_U_LEFT:
            case MOVE_PLAN_U_RIGHT:
            {
                bool b_left = (MOVE_PLAN_LEFT == p_command->kind) ||
                              (MOVE_PLAN_U_LEFT == p_command->kind);
                uint64_t divisor = ((MOVE_PLAN_LEFT == p_command->kind) ||
                    (MOVE_PLAN_RIGHT == p_command->kind)) ? 452U : 226U;
                uint64_t numerator = (uint64_t)p_geometry->track_width_um*355U;

                for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
                {
                    uint64_t scale = p_config->turn_scale_ppm[wheel];

                    if (numerator > (UINT64_MAX/scale))
                    {
                        b_ok = false;
                    }
                    else
                    {
                        uint64_t arc = (numerator*scale)/(divisor*1000000U);

                        if (arc > INT32_MAX)
                        {
                            b_ok = false;
                        }
                        else
                        {
                            bool b_negative = (0U == wheel) ? b_left : !b_left;

                            candidate.goal_um[wheel] = b_negative ?
                                -(int32_t)arc : (int32_t)arc;
                        }
                    }
                }
            }
            break;
            default:
                b_ok = false;
            break;
        }
        if (candidate.b_finite)
        {
            b_ok = b_ok && (p_command->cruise_mm_s > 0) &&
                   ((int64_t)p_command->cruise_mm_s <= max_speed);
            for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
            {
                int64_t goal = candidate.goal_um[wheel];
                uint64_t magnitude = (uint64_t)((goal < 0) ? -goal : goal);

                b_ok = b_ok && (magnitude > p_config->tolerance_um);
            }
        }
        if (b_ok)
        {
            candidate.b_initialised = true;
            *p_plan = candidate;
        }
    }
    return (b_ok);
}

bool move_plan_step (move_plan_t const * p_plan,
    move_plan_config_t const * p_config, int64_t const position_um[2],
    int32_t speed_mm_s[2], bool * p_arrived)
{
    bool b_ok = (NULL != p_plan) && (NULL != p_config) &&
        (NULL != position_um) && (NULL != speed_mm_s) && (NULL != p_arrived);
    int64_t progress[2] = {0,0};
    uint64_t magnitude[2] = {0U,0U};
    int32_t speeds[2] = {0,0};
    bool b_arrived = true;

    b_ok = b_ok && p_plan->b_initialised &&
        move_plan_validate(p_config,INT32_MAX);
    if (b_ok && !p_plan->b_finite)
    {
        speeds[0] = p_plan->command.left_mm_s;
        speeds[1] = p_plan->command.right_mm_s;
        b_arrived = false;
    }
    else if (b_ok)
    {
        b_ok = move_plan_validate(p_config,INT32_MAX) &&
               (p_plan->command.cruise_mm_s > 0);
        for (uint32_t wheel = 0U; b_ok && (wheel < 2U); ++wheel)
        {
            int64_t delta = 0;
            int64_t goal = p_plan->goal_um[wheel];

            magnitude[wheel] = (uint64_t)((goal < 0) ? -goal : goal);
            b_ok = (magnitude[wheel] > p_config->tolerance_um) &&
                move_plan_delta(position_um[wheel],p_plan->origin_um[wheel],
                    &delta);
            if (b_ok && (goal < 0))
            {
                b_ok = (INT64_MIN != delta);
                if (b_ok)
                {
                    delta = -delta;
                }
            }
            if (b_ok)
            {
                b_ok = (delta >= -(int64_t)p_config->tolerance_um) &&
                    (delta <= (int64_t)(
                        magnitude[wheel]+p_config->tolerance_um));
                progress[wheel] = (delta < 0) ? 0 : delta;
            }
            if (b_ok && ((uint64_t)progress[wheel]+p_config->tolerance_um <
                         magnitude[wheel]))
            {
                uint64_t remaining = magnitude[wheel]-(uint64_t)progress[wheel];
                uint64_t squared = (2U*(
                    uint64_t)p_config->decel_mm_s2*remaining)/1000U;
                uint64_t cap = move_plan_sqrt(squared);

                b_arrived = false;
                if (cap > (uint64_t)p_plan->command.cruise_mm_s)
                {
                    cap = (uint64_t)p_plan->command.cruise_mm_s;
                }
                speeds[wheel] = (int32_t)cap;
            }
        }
        if (b_ok)
        {
            int64_t fraction[2] = {
                (progress[0]*1000000)/(int64_t)magnitude[0],
                (progress[1]*1000000)/(int64_t)magnitude[1]};
            uint32_t leader = (fraction[0] > fraction[1]) ? 0U : 1U;
            int64_t difference = fraction[leader]-fraction[1U-leader];
            uint64_t shorter = (
                magnitude[0] < magnitude[1]) ? magnitude[0] : magnitude[1];
            motion_pid_value_t correction = p_config->correction_gain*
                ((motion_pid_value_t)(difference*(
                    int64_t)shorter)/1000000000.0F);

            b_ok = isfinite(correction);
            if (b_ok)
            {
                int32_t limit = p_config->correction_limit_mm_s;
                int32_t reduction = limit;

                /* Compare in double: INT32_MAX rounds UP in binary32. */
                if ((double)correction < (double)limit)
                {
                    reduction = (int32_t)correction;
                }
                speeds[leader] = (reduction < speeds[leader]) ?
                    (speeds[leader]-reduction) : 0;
                for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
                {
                    if (p_plan->goal_um[wheel] < 0)
                    {
                        speeds[wheel] = -speeds[wheel];
                    }
                }
            }
        }
    }
    if (NULL != speed_mm_s)
    {
        speed_mm_s[0] = b_ok ? speeds[0] : 0;
        speed_mm_s[1] = b_ok ? speeds[1] : 0;
    }
    if (NULL != p_arrived)
    {
        *p_arrived = b_ok && b_arrived;
    }
    return (b_ok);
}

static uint64_t move_plan_sqrt (uint64_t value)
{
    uint64_t root = 0U;
    uint64_t bit = UINT64_C(1) << 62U;

    /* Fixed 32 iterations, no data-dependent convergence. */
    for (uint32_t index = 0U; index < 32U; ++index)
    {
        if (value >= (root+bit))
        {
            value -= root+bit;
            root = (root>>1U)+bit;
        }
        else
        {
            root >>= 1U;
        }
        bit >>= 2U;
    }
    return (root);
}

static bool move_plan_delta (int64_t position, int64_t origin,
    int64_t * p_delta)
{
    bool b_ok = !(((origin > 0) && (position < (INT64_MIN+origin))) ||
                 ((origin < 0) && (position > (INT64_MAX+origin))));

    if (b_ok)
    {
        *p_delta = position-origin;
    }
    return (b_ok);
}

/*** end of file ***/
