/** @file move_plan.h @brief Pure bounded distance/turn setpoint planning. */
#ifndef MOVE_PLAN_H
#define MOVE_PLAN_H
#include "motion_config.h"
#include "motion_pid.h"
typedef enum
{
    MOVE_PLAN_SPEED = 0, MOVE_PLAN_TRAVEL, MOVE_PLAN_LEFT,
    MOVE_PLAN_RIGHT, MOVE_PLAN_U_LEFT, MOVE_PLAN_U_RIGHT
} move_plan_kind_t;
typedef struct
{
    uint32_t id;
    move_plan_kind_t kind;
    uint64_t issued_us;
    uint32_t lifetime_us;
    int32_t left_mm_s;
    int32_t right_mm_s;
    int32_t distance_um;
    int32_t cruise_mm_s; /* Positive finite-manoeuvre speed magnitude. */
} move_plan_command_t;
typedef struct
{
    uint32_t accel_mm_s2;
    uint32_t decel_mm_s2;
    uint32_t tolerance_um;
    uint32_t turn_scale_ppm[2]; /* Explicit calibration; no nominal default. */
    motion_pid_value_t correction_gain; /* (mm/s)/mm progress mismatch. */
    int32_t correction_limit_mm_s;
} move_plan_config_t;
typedef struct
{
    move_plan_command_t command;
    int64_t origin_um[2];
    int32_t goal_um[2];
    bool b_finite;
    bool b_initialised;
} move_plan_t;
bool move_plan_validate (move_plan_config_t const * p_config,
    int32_t max_speed);
bool move_plan_begin (move_plan_t * p_plan,
    move_plan_command_t const * p_command,
    move_plan_config_t const * p_config, motion_config_t const * p_geometry,
    int64_t const position_um[2], uint64_t now_us);
/* Fails on arithmetic error, wrong-way travel or overshoot beyond tolerance.
 * b_arrived means both POSITION tolerances, never mechanical completion.
 */
bool move_plan_step (move_plan_t const * p_plan,
    move_plan_config_t const * p_config, int64_t const position_um[2],
    int32_t speed_mm_s[2], bool * p_arrived);
#endif /* MOVE_PLAN_H */

/*** end of file ***/

