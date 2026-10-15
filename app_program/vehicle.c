/** @file vehicle.c @brief Mission arbitration, conservative provider gates. */
#include "vehicle.h"
#include <limits.h>
#include <math.h>
#include <stddef.h>

static bool vehicle_fresh (uint64_t now, uint64_t sample, uint32_t age);
static void vehicle_fault (vehicle_t * p_vehicle, vehicle_output_t * p_output);
static bool vehicle_command (vehicle_t * p_vehicle,
    vehicle_input_t const * p_input, vehicle_output_t * p_output);

bool vehicle_init (vehicle_t * p_vehicle, vehicle_config_t const * p_config)
{
    bool b_ok = (NULL != p_vehicle) && (NULL != p_config);

    if (b_ok)
    {
        b_ok = (p_config->sensor_age_us > 0U) &&
            (p_config->sensor_age_us < 1000000U) &&
            (p_config->command_ttl_us > p_config->sensor_age_us) &&
            (p_config->transition_timeout_us > p_config->command_ttl_us) &&
            (p_config->remote_age_ms > 0U) &&
            (p_config->remote_age_ms < 0x80000000U) &&
            (p_config->forward_mm_s > 0) &&
            (p_config->correction_mm_s >= 0) &&
            (p_config->correction_mm_s < p_config->forward_mm_s) &&
            (p_config->turn_mm_s > 0) && (p_config->straight_um > 0) &&
            (p_config->obstacle_stop_cm > 0U) && p_config->b_calibrated;
        if (b_ok)
        {
            *p_vehicle = (vehicle_t){0};
            p_vehicle->config = *p_config;
            p_vehicle->state = VEHICLE_BLOCKED;
            p_vehicle->b_initialised = true;
        }
    }
    return (b_ok);
}

bool vehicle_step (vehicle_t * p_vehicle, vehicle_input_t const * p_input,
    vehicle_output_t * p_output)
{
    if ((NULL == p_vehicle) || (NULL == p_input) || (NULL == p_output))
    {
        return (false);
    }
    *p_output = (vehicle_output_t){0};
    bool b_live = p_vehicle->active_id != 0U;

    if (((uint32_t)p_input->motion.state > MOTION_RUN_FAULT) ||
        ((uint32_t)p_input->line > LINE_OBSERVATION_ABSENT) ||
        ((uint32_t)p_input->line_state > LINE_STATE_FAULT_REPORTED) ||
        ((uint32_t)p_input->line_mode > LINE_MODE_RECOVERY) ||
        ((uint32_t)p_input->navigation.kind > NAV_EVENT_FAULT) ||
        ((uint32_t)p_input->remote.command_id > S1_CMD_RESUME) ||
        ((uint32_t)p_input->imu.phase > IMU_FAULT))
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }

    if (p_input->b_estop || p_input->b_provider_fault ||
        (p_input->b_motion_available && (0U != p_input->motion.active_id) &&
         (p_input->motion.active_id != p_vehicle->active_id)) ||
        (p_input->b_line_available &&
         (LINE_STATE_FAULT_REPORTED == p_input->line_state)) ||
        (NAV_EVENT_FAULT == p_input->navigation.kind) ||
        (S1_CMD_EMERGENCY_STOP == p_input->remote.command_id) ||
        (p_input->b_motion_available &&
         (MOTION_RUN_FAULT == p_input->motion.state)) ||
        (p_input->now_us < p_vehicle->last_now_us) ||
        (VEHICLE_FAULT == p_vehicle->state))
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }
    p_vehicle->last_now_us = p_input->now_us;
    if (!p_vehicle->b_initialised)
    {
        p_vehicle->state = VEHICLE_UNCONFIGURED;
        return (true);
    }
    vehicle_config_t const * p_cfg = &p_vehicle->config;
    bool b_line_fresh = p_input->b_line_available && p_input->b_line_valid &&
        p_input->b_line_calibrated && vehicle_fresh(p_input->raw_now_us,
            p_input->line_time_us, p_cfg->sensor_age_us);
    bool b_imu_fresh = p_input->b_imu_available &&
        ((uint32_t)(p_input->raw_now_us / 1000U) - p_input->imu.time_ms <=
         p_cfg->sensor_age_us / 1000U) &&
        (IMU_CALIBRATING != p_input->imu.phase) &&
        (IMU_FAULT != p_input->imu.phase);
    bool b_obstacle = p_input->b_obstacle_available &&
        p_input->obstacle.valid && vehicle_fresh(p_input->raw_now_us,
            p_input->obstacle_time_us, p_cfg->sensor_age_us);
    bool b_motion = p_input->b_motion_available &&
        ((MOTION_RUN_CONFIGURED == p_input->motion.state) ||
         (p_input->motion.b_measurement_valid && vehicle_fresh(p_input->now_us,
            p_input->motion.measurement_timestamp_us,
            p_input->motion.measurement_max_age_us)));

    p_vehicle->blockers = (b_line_fresh ? 0U : VEHICLE_BLOCK_LINE) |
        (b_imu_fresh ? 0U : VEHICLE_BLOCK_IMU) |
        (b_obstacle ? 0U : VEHICLE_BLOCK_OBSTACLE) |
        (b_motion ? 0U : VEHICLE_BLOCK_MOTION) |
        (p_cfg->b_power_authorised ? 0U : VEHICLE_BLOCK_POWER);
    if (b_live && (0U != p_vehicle->blockers))
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }
    if ((IMU_FAULT == p_input->imu.phase) ||
        (0U != (p_input->imu.events & (IMU_EVENT_FAULT | IMU_EVENT_IMPACT))))
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }
    /* Emergency bypasses freshness/ordinary command admission. */
    if (S1_CMD_EMERGENCY_STOP == p_input->remote.command_id)
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }
    uint32_t age_ms = (uint32_t)(p_input->now_us / 1000U) -
        p_input->remote.rx_timestamp_ms;
    if (age_ms <= p_cfg->remote_age_ms)
    {
        if ((S1_CMD_START_RUN == p_input->remote.command_id) ||
            (S1_CMD_RESUME == p_input->remote.command_id))
        {
            p_vehicle->b_run_requested = true;
            p_vehicle->b_pause_requested = false;
        }
        else if (S1_CMD_PAUSE == p_input->remote.command_id)
        {
            p_vehicle->b_pause_requested = true;
        }
    }
    if ((p_input->navigation_id > p_vehicle->last_navigation_id) &&
        (NAV_EVENT_BARCODE_VALID == p_input->navigation.kind))
    {
        if ((0U != p_vehicle->navigation_id) ||
            !vehicle_fresh(p_input->raw_now_us,
                p_input->navigation.timestamp_us, p_cfg->sensor_age_us) ||
            (p_input->navigation.command < NAV_COMMAND_LEFT) ||
            (p_input->navigation.command > NAV_COMMAND_U_TURN))
        {
            vehicle_fault(p_vehicle, p_output);
            return (true);
        }
        p_vehicle->b_line_found = false;
        p_vehicle->navigation = p_input->navigation.command;
        p_vehicle->navigation_id = p_input->navigation_id;
        p_vehicle->last_navigation_id = p_input->navigation_id;
        p_vehicle->transition_us = p_input->now_us;
    }
    if (0U != p_vehicle->blockers)
    {
        p_vehicle->state = VEHICLE_BLOCKED;
        return (true);
    }
    bool b_near = p_input->obstacle.closest_distance_cm <=
        p_cfg->obstacle_stop_cm;
    line_observation_t raw_line = LINE_OBSERVATION_ABSENT;
    bool b_steer = p_input->b_line_valid &&
        line_classify((uint8_t)(p_input->black_mask & 3U), &raw_line) &&
        (raw_line == p_input->line);
    /* Stop causes preempt command-intake retries and terminal handshakes. */
    if ((VEHICLE_MANOEUVRE == p_vehicle->state) &&
        (b_near || p_vehicle->b_pause_requested))
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }
    if (((VEHICLE_STOPPING == p_vehicle->state) ||
         (VEHICLE_MANOEUVRE == p_vehicle->state) ||
         p_vehicle->b_recovery_required) &&
        ((p_input->now_us - p_vehicle->transition_us) >
         p_cfg->transition_timeout_us))
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }
    bool b_need_stop = b_near || p_vehicle->b_pause_requested ||
        ((0U != p_vehicle->navigation_id) &&
         (VEHICLE_MANOEUVRE != p_vehicle->state)) ||
        (VEHICLE_RECOVERY == p_vehicle->state) ||
        ((VEHICLE_FOLLOW == p_vehicle->state) &&
         (!b_steer ||
          (LINE_OBSERVATION_ABSENT == p_input->line)));
    if (b_need_stop && b_live &&
        (VEHICLE_MANOEUVRE != p_vehicle->state))
    {
        if (VEHICLE_STOPPING != p_vehicle->state)
        {
            p_vehicle->transition_us = p_input->now_us;
        }
        p_vehicle->state = VEHICLE_STOPPING;
        p_vehicle->b_pending_command = false;
        p_output->b_stop = true;
    }
    if (0U != p_input->motion.terminal_id)
    {
        if ((p_input->motion.terminal_id != p_vehicle->active_id) ||
            ((MOTION_RUN_DONE != p_input->motion.terminal_result) &&
             (MOTION_RUN_STOPPED != p_input->motion.terminal_result) &&
             (MOTION_RUN_CANCELLED != p_input->motion.terminal_result)))
        {
            vehicle_fault(p_vehicle, p_output);
            return (true);
        }
        if ((VEHICLE_MANOEUVRE == p_vehicle->state) &&
            (MOTION_RUN_DONE != p_input->motion.terminal_result))
        {
            vehicle_fault(p_vehicle, p_output);
            return (true);
        }
        p_output->b_stop = false;
        p_output->acknowledge_id = p_input->motion.terminal_id;
        if ((VEHICLE_MANOEUVRE == p_vehicle->state) &&
            (MOTION_RUN_DONE == p_input->motion.terminal_result))
        {
            if (!p_vehicle->b_turn_completed)
            {
                p_vehicle->transition_us = p_input->now_us;
            }
            p_vehicle->b_turn_completed = true;
            p_vehicle->b_recovery_required = true;
        }
        return (true); /* ACK intake is not completion of the transaction. */
    }
    if (b_live && (MOTION_RUN_READY == p_input->motion.state))
    {
        if ((VEHICLE_STOPPING == p_vehicle->state) ||
            p_vehicle->b_turn_completed)
        {
            p_vehicle->active_id = 0U;
            b_live = false;
        }
    }
    if ((VEHICLE_STOPPING == p_vehicle->state) &&
        ((p_input->now_us - p_vehicle->transition_us) >
         p_cfg->transition_timeout_us))
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }
    if ((VEHICLE_STOPPING == p_vehicle->state) && b_live)
    {
        p_output->b_stop = true;
        return (true);
    }
    if (p_vehicle->b_pending_command)
    {
        move_plan_command_t const * p_cmd = &p_vehicle->pending_command;

        if ((p_input->now_us < p_cmd->issued_us) ||
            ((p_input->now_us - p_cmd->issued_us) >= p_cmd->lifetime_us))
        {
            vehicle_fault(p_vehicle, p_output);
            return (true);
        }
        if ((p_input->motion.last_request_id == p_cmd->id) &&
            (p_input->motion.last_request_issued_us == p_cmd->issued_us))
        {
            if (!p_input->motion.b_request_accepted)
            {
                vehicle_fault(p_vehicle, p_output);
                return (true);
            }
            p_vehicle->b_pending_command = false;
        }
        else
        {
            p_output->b_command = true;
            p_output->command = *p_cmd;
            return (true); /* BUSY/QUEUED: retry unchanged until owner receipt. */
        }
    }
    if (b_near || p_vehicle->b_pause_requested)
    {
        if (VEHICLE_MANOEUVRE == p_vehicle->state)
        {
            vehicle_fault(p_vehicle, p_output);
        }
        else if (!b_live)
        {
            p_vehicle->state = b_near ? VEHICLE_OBSTACLE : VEHICLE_PAUSED;
            p_output->b_mode = (LINE_MODE_PAUSED != p_input->line_mode);
            p_output->mode = LINE_MODE_PAUSED;
        }
        return (true); /* No executable S5 bypass contract exists. */
    }
    if (p_vehicle->b_turn_completed)
    {
        if (VEHICLE_RECOVERY != p_vehicle->state)
        {
            p_vehicle->transition_us = p_input->now_us;
        }
        p_vehicle->state = VEHICLE_RECOVERY;
        if (p_input->pending_navigation_id == p_vehicle->navigation_id)
        {
            p_output->navigation_done_id = p_vehicle->navigation_id;
        }
        else if (0U == p_input->pending_navigation_id)
        {
            p_vehicle->navigation_id = 0U;
            p_vehicle->b_turn_completed = false;
        }
        return (true);
    }
    if (p_vehicle->b_recovery_required &&
        (0U == p_vehicle->navigation_id) &&
        (NAV_EVENT_LINE_FOUND == p_input->navigation.kind) &&
        vehicle_fresh(p_input->raw_now_us, p_input->navigation.timestamp_us,
            p_cfg->sensor_age_us))
    {
        p_vehicle->b_line_found = true;
        p_vehicle->b_recovery_required = false;
    }
    if (p_vehicle->b_recovery_required)
    {
        p_vehicle->state = VEHICLE_RECOVERY;
        p_output->b_mode = (LINE_MODE_RECOVERY != p_input->line_mode);
        p_output->mode = LINE_MODE_RECOVERY;
        return (true); /* Stationary search only until an approved S5 plan. */
    }
    if (!p_vehicle->b_run_requested)
    {
        p_vehicle->state = VEHICLE_READY;
        return (true);
    }
    if ((0U == p_vehicle->navigation_id) &&
        (LINE_MODE_TRACKING != p_input->line_mode))
    {
        p_output->b_mode = true;
        p_output->mode = LINE_MODE_TRACKING;
        return (true);
    }
    if (MOTION_RUN_CONFIGURED == p_input->motion.state)
    {
        p_output->b_enable = true;
        return (true);
    }
    if (MOTION_RUN_SETTLING == p_input->motion.state)
    {
        return (true);
    }
    if ((MOTION_RUN_READY != p_input->motion.state) &&
        (MOTION_RUN_ACTIVE != p_input->motion.state))
    {
        return (true);
    }
    if (p_input->motion.active_id != p_vehicle->active_id)
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }
    if ((0U != p_vehicle->active_id) &&
        (0U != p_vehicle->navigation_id))
    {
        return (true); /* A finite manoeuvre must never be refreshed. */
    }
    return (vehicle_command(p_vehicle, p_input, p_output));
}

bool vehicle_frame (vehicle_t const * p_vehicle,
    vehicle_input_t const * p_input, s1_codec_frame_t * p_frame)
{
    if ((NULL == p_vehicle) || (NULL == p_input) || (NULL == p_frame) ||
        !p_input->b_counts_valid || !p_input->motion.b_measurement_valid ||
        !p_input->b_line_valid || !p_input->b_obstacle_available ||
        !p_input->obstacle.valid || !p_input->imu.b_highest_valid ||
        !isfinite(p_input->imu.highest_hump_mm) ||
        (p_input->imu.highest_hump_mm < 0.0F) ||
        (p_input->imu.highest_hump_mm > 1000000.0F))
    {
        return (false);
    }
    for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
    {
        if ((p_input->encoder_count[wheel] < 0) ||
            (p_input->encoder_count[wheel] > UINT32_MAX) ||
            (p_input->motion.speed_mm_s[wheel] < INT16_MIN) ||
            (p_input->motion.speed_mm_s[wheel] > INT16_MAX))
        {
            return (false);
        }
    }
    /* Existing 3-bit S1 line convention has no agreed S3 side-sensor mapping. */
    return (false);
}

bool vehicle_odometry (vehicle_odometry_t * p_odometry,
    vehicle_input_t const * p_input, int32_t maximum_mm_s)
{
    if ((NULL == p_odometry) || (NULL == p_input) || (maximum_mm_s <= 0))
    {
        return (false);
    }
    motion_run_status_t const * p_motion = &p_input->motion;

    p_odometry->b_tracking_forward = false;
    if (!p_input->b_motion_available || !p_motion->b_measurement_valid ||
        p_input->b_estop || p_input->b_provider_fault ||
        !vehicle_fresh(p_input->now_us, p_motion->measurement_timestamp_us,
            p_motion->measurement_max_age_us) ||
        (0U == p_motion->measurement_max_age_us) ||
        (p_motion->measurement_max_age_us > 1000000U))
    {
        p_odometry->b_baseline = false;
        return (false);
    }
    uint64_t age = p_input->now_us - p_motion->measurement_timestamp_us;

    if ((age > p_input->raw_now_us) ||
        (p_odometry->b_baseline &&
         (p_motion->measurement_timestamp_us <= p_odometry->previous_us)))
    {
        return (false); /* Never re-stamp duplicate/stale measurements. */
    }
    bool b_forward = (MOTION_RUN_ACTIVE == p_motion->state) &&
        (MOVE_PLAN_SPEED == p_motion->active_kind) &&
        (p_motion->target_mm_s[0] > 0) && (p_motion->target_mm_s[1] > 0) &&
        (p_motion->speed_mm_s[0] > 0) && (p_motion->speed_mm_s[1] > 0);
    if (p_odometry->b_baseline &&
        (p_input->raw_now_us - age < p_odometry->raw_timestamp_us))
    {
        return (false); /* Quantised clocks must not regress S3 timestamps. */
    }
    uint64_t distance = 0U;

    if (p_odometry->b_baseline && p_odometry->b_previous_forward && b_forward)
    {
        uint64_t elapsed = p_motion->measurement_timestamp_us -
            p_odometry->previous_us;
        bool b_contiguous = elapsed <= p_motion->measurement_max_age_us;
        uint64_t delta[2] = {0U, 0U};

        for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
        {
            bool b_increasing = p_motion->position_um[wheel] >=
                p_odometry->previous_um[wheel];

            /* Ordered signed values; unsigned subtraction cannot overflow UB. */
            delta[wheel] = (uint64_t)p_motion->position_um[wheel] -
                (uint64_t)p_odometry->previous_um[wheel];
            b_contiguous = b_contiguous && b_increasing;
            if (elapsed <= 1000000U)
            {
                b_contiguous = b_contiguous && (delta[wheel] <=
                    ((uint64_t)(uint32_t)maximum_mm_s * elapsed) / 1000U);
            }
            else
            {
                b_contiguous = false;
            }
        }
        if (b_contiguous)
        {
            uint64_t remainder = (delta[0] % 2U) + (delta[1] % 2U) +
                p_odometry->half_um;

            distance = (delta[0] / 2U) + (delta[1] / 2U) + remainder / 2U;
            p_odometry->half_um = (uint8_t)(remainder % 2U);
            if (distance > UINT64_MAX - p_odometry->distance_um)
            {
                p_odometry->b_baseline = false;
                return (false);
            }
            p_odometry->b_tracking_forward = true;
        }
    }
    p_odometry->distance_um += distance;
    p_odometry->previous_um[0] = p_motion->position_um[0];
    p_odometry->previous_um[1] = p_motion->position_um[1];
    p_odometry->previous_us = p_motion->measurement_timestamp_us;
    p_odometry->raw_timestamp_us = p_input->raw_now_us - age;
    p_odometry->b_baseline = true;
    p_odometry->b_previous_forward = b_forward;
    return (true);
}

static bool vehicle_fresh (uint64_t now, uint64_t sample, uint32_t age)
{
    return ((now >= sample) && ((now - sample) <= age));
}

static void vehicle_fault (vehicle_t * p_vehicle, vehicle_output_t * p_output)
{
    *p_output = (vehicle_output_t){0};
    p_output->b_estop = true;
    p_vehicle->state = VEHICLE_FAULT;
    p_vehicle->b_run_requested = false;
}

static bool vehicle_command (vehicle_t * p_vehicle,
    vehicle_input_t const * p_input, vehicle_output_t * p_output)
{
    bool b_new = (0U == p_vehicle->active_id);

    if (!b_new && (p_input->now_us <=
        p_input->motion.last_request_issued_us))
        {
            return (true);
        }
    if (b_new && (0U == p_vehicle->navigation_id))
    {
        line_observation_t raw = LINE_OBSERVATION_ABSENT;

        if (!line_classify((uint8_t)(p_input->black_mask & 3U), &raw) ||
            (raw != p_input->line) || !p_input->b_line_valid ||
            ((LINE_STATE_TRACKING != p_input->line_state) &&
             (LINE_STATE_BARCODE_DECODING != p_input->line_state)) ||
            (LINE_OBSERVATION_ABSENT == raw))
            {
                return (true);
            }
    }
    if (b_new && (UINT32_MAX == p_vehicle->next_id))
    {
        vehicle_fault(p_vehicle, p_output);
        return (true);
    }

    if (b_new)
    {
        if ((MOTION_RUN_READY != p_input->motion.state) ||
            !p_input->motion.b_standstill || (UINT32_MAX == p_vehicle->next_id))
        {
            return (true);
        }
        p_vehicle->active_id = ++p_vehicle->next_id;
        p_vehicle->transition_us = p_input->now_us;
    }
    move_plan_command_t command = {0};

    command.id = p_vehicle->active_id;
    command.issued_us = p_input->now_us;
    command.lifetime_us = p_vehicle->config.command_ttl_us;
    if (0U != p_vehicle->navigation_id)
    {
        if (p_input->pending_navigation_id != p_vehicle->navigation_id)
        {
            vehicle_fault(p_vehicle, p_output);
            return (true);
        }
        command.kind = (NAV_COMMAND_LEFT == p_vehicle->navigation) ?
            MOVE_PLAN_LEFT : ((NAV_COMMAND_RIGHT == p_vehicle->navigation) ?
            MOVE_PLAN_RIGHT : ((NAV_COMMAND_STRAIGHT == p_vehicle->navigation) ?
            MOVE_PLAN_TRAVEL : (p_vehicle->config.b_uturn_right ?
            MOVE_PLAN_U_RIGHT : MOVE_PLAN_U_LEFT)));
        command.distance_um = p_vehicle->config.straight_um;
        command.cruise_mm_s = p_vehicle->config.turn_mm_s;
        command.lifetime_us = p_vehicle->config.transition_timeout_us;
        p_vehicle->state = VEHICLE_MANOEUVRE;
    }
    else
    {
        line_observation_t raw = LINE_OBSERVATION_ABSENT;

        if (!line_classify((uint8_t)(p_input->black_mask & 3U), &raw) ||
            (raw != p_input->line) || !p_input->b_line_valid ||
            ((LINE_STATE_TRACKING != p_input->line_state) &&
             (LINE_STATE_BARCODE_DECODING != p_input->line_state)) ||
            (LINE_OBSERVATION_ABSENT == p_input->line))
        {
            if (b_new)
            {
                p_vehicle->active_id = 0U;
            }
            return (true);
        }
        command.kind = MOVE_PLAN_SPEED;
        command.left_mm_s = p_vehicle->config.forward_mm_s;
        command.right_mm_s = p_vehicle->config.forward_mm_s;
        if (LINE_OBSERVATION_CORRECT_LEFT == p_input->line)
        {
            command.left_mm_s -= p_vehicle->config.correction_mm_s;
        }
        if (LINE_OBSERVATION_CORRECT_RIGHT == p_input->line)
        {
            command.right_mm_s -= p_vehicle->config.correction_mm_s;
        }
        p_vehicle->state = VEHICLE_FOLLOW;
    }
    p_vehicle->pending_command = command;
    p_vehicle->b_pending_command = true;
    p_output->command = command;
    p_output->b_command = true;
    return (true);
}
/*** end of file ***/
