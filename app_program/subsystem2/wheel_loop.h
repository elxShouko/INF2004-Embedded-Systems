/** @file wheel_loop.h @brief Single-owner encoder/estimator/PID/motor path. */
#ifndef WHEEL_LOOP_H
#define WHEEL_LOOP_H
#include "enc_capture.h"
#include "drv_motor.h"
#include "move_plan.h"
typedef struct
{
    motion_config_t geometry;
    drv_motor_config_t motor;
    motion_pid_config_t pid[2];
    move_plan_config_t profile;
    uint32_t period_us;
    uint32_t sample_limit_us;
    uint32_t stop_dwell_us;
    uint32_t stop_timeout_us;
    uint32_t progress_timeout_us; /* Explicit no-encoder-progress deadline. */
    int32_t stop_speed_mm_s;
    int32_t overspeed_mm_s;
} wheel_loop_config_t;
typedef struct
{
    wheel_loop_config_t config;
    drv_motor_t motor;
    motion_estimator_t estimator[2];
    motion_pid_t pid[2];
    motion_estimator_measurement_t measured[2];
    int64_t velocity_um_s[2];
    uint64_t slew_remainder[2];
    int64_t last_count[2];
    uint64_t quiet_since_us[2];
    uint64_t progress_since_us[2];
    uint64_t last_tick_us;
    bool b_quiet[2];
    bool b_standstill[2];
    bool b_tracking[2];
    /* Scalar owner flags; indexed wheel arrays above retain their API. */
    bool b_valid : 1;
    bool b_enabled : 1;
    bool b_fault : 1;
} wheel_loop_t;
bool wheel_loop_validate (wheel_loop_config_t const * p_config);
bool wheel_loop_init (wheel_loop_t * p_loop,
    wheel_loop_config_t const * p_config, drv_motor_io_t const * p_io);
bool wheel_loop_enable (wheel_loop_t * p_loop, uint64_t now_us);
/* Returns false and disables outputs on unhealthy/stale/count reset/overflow.
 * First snapshot establishes baseline without speed. Call only once per tick.
 */
bool wheel_loop_measure (wheel_loop_t * p_loop,
    enc_capture_sample_t const * p_sample, uint64_t now_us);
bool wheel_loop_drive (wheel_loop_t * p_loop, int32_t const desired_mm_s[2],
                       uint32_t elapsed_us, uint64_t now_us);
bool wheel_loop_brake (wheel_loop_t * p_loop, uint64_t now_us);
void wheel_loop_fault (wheel_loop_t * p_loop);
#endif /* WHEEL_LOOP_H */

/*** end of file ***/

