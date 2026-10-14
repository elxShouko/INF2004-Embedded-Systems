/** @file motion_run.h @brief Single-owner motion executive; provisional API. */
#ifndef MOTION_RUN_H
#define MOTION_RUN_H
#include "wheel_loop.h"
typedef enum
{
    MOTION_RUN_UNCONFIGURED = 0, MOTION_RUN_CONFIGURED, MOTION_RUN_SETTLING,
    MOTION_RUN_READY, MOTION_RUN_ACTIVE, MOTION_RUN_STOPPING, MOTION_RUN_FAULT
} motion_run_state_t;
typedef enum
{
    MOTION_RUN_NONE = 0, MOTION_RUN_DONE, MOTION_RUN_STOPPED,
    MOTION_RUN_CANCELLED, MOTION_RUN_EXPIRED, MOTION_RUN_TIMEOUT,
    MOTION_RUN_EMERGENCY, MOTION_RUN_SENSOR, MOTION_RUN_OUTPUT,
    MOTION_RUN_REJECTED, MOTION_RUN_OVERSHOOT
} motion_run_result_t;
typedef struct
{
    uint64_t timestamp_us;
    motion_run_state_t state;
    uint32_t active_id;
    move_plan_kind_t active_kind;
    uint32_t terminal_id;
    motion_run_result_t terminal_result;
    /* Latched independently of the command's terminal slot. */
    motion_run_result_t fault_result;
    uint32_t last_request_id;
    uint64_t last_request_issued_us;
    bool b_request_accepted;
    uint64_t measurement_timestamp_us;
    uint32_t measurement_max_age_us;
    int64_t position_um[2];
    int32_t speed_mm_s[2];
    int32_t target_mm_s[2];
    bool b_measurement_valid;
    bool b_standstill;
    bool b_stop_command_confirmed;
} motion_run_status_t;
typedef struct
{
    wheel_loop_t loop;
    move_plan_t plan;
    motion_run_status_t status;
    uint64_t deadline_us;
    uint64_t stopping_since_us;
    uint32_t last_id;
    motion_run_result_t pending_result;
    bool b_initialised;
} motion_run_t;
bool motion_run_init (motion_run_t * p_run,
    wheel_loop_config_t const * p_config, drv_motor_io_t const * p_io);
bool motion_run_enable (motion_run_t * p_run, uint64_t now_us);
/* Admission never moves outputs. IDs increase without wrap; refresh requires
 * SAME active continuous ID. Durable terminal slot must be acknowledged before
 * a new ID; rejected commands do not overwrite another command's result.
 */
bool motion_run_command (motion_run_t * p_run,
    move_plan_command_t const * p_command, uint64_t now_us);
bool motion_run_ack (motion_run_t * p_run, uint32_t id);
void motion_run_stop (motion_run_t * p_run, bool b_cancel, uint64_t now_us);
void motion_run_emergency (motion_run_t * p_run, motion_run_result_t cause,
                           uint64_t now_us);
bool motion_run_tick (motion_run_t * p_run,
    enc_capture_sample_t const * p_sample, uint64_t now_us);
#endif /* MOTION_RUN_H */

/*** end of file ***/

