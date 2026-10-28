/**
 * @file drv_motor.h
 * @brief Buddy 2 internal motor output logic; provisional, not a team API.
 */
#ifndef DRV_MOTOR_H
#define DRV_MOTOR_H

#include "motion_pid.h"

/* Exact binary32 duty * uint16 period during initialization only. */
#if DBL_MANT_DIG < (FLT_MANT_DIG + 16)
#error "motor duty ceiling requires sufficient double precision"
#endif

#define DRV_MOTOR_WHEELS (2U)
#define DRV_MOTOR_MAX_HZ (20000U)

typedef enum
{
    DRV_MOTOR_LEFT = 0,
    DRV_MOTOR_RIGHT
} drv_motor_wheel_t;

typedef enum
{
    DRV_MOTOR_BRAKE = 0, /* Robo Pico Table 3: A=0, B=0. */
    DRV_MOTOR_COAST      /* Robo Pico Table 3: A=1, B=1. */
} drv_motor_stop_t;

typedef enum
{
    DRV_MOTOR_OK = 0,
    DRV_MOTOR_WAIT,
    DRV_MOTOR_BAD_PARAM,
    DRV_MOTOR_BAD_TIME,
    DRV_MOTOR_DISABLED,
    DRV_MOTOR_FAULT,
    DRV_MOTOR_HARDWARE
} drv_motor_result_t;

typedef enum
{
    DRV_MOTOR_STOPPED = 0,
    DRV_MOTOR_RUNNING,
    DRV_MOTOR_REVERSING
} drv_motor_state_t;

typedef struct
{
    uint32_t clock_hz;
    uint32_t pwm_hz; /* Upper requested frequency; actual = clock/period. */
    /* Explicit measured/reviewed guard, no default. */
    uint32_t reversal_guard_us;
    motion_pid_value_t max_duty; /* (0,1], quantized DOWN to a duty ceiling. */
    motion_types_polarity_t polarity[DRV_MOTOR_WHEELS];
    drv_motor_stop_t zero_mode;
    motion_config_calibration_t calibration;
    bool b_resources_verified; /* Pins/slices/power checked by bench owner. */
} drv_motor_config_t;

/* Internal backend contract. prepare is explicit and leaves BOTH wheels
 * braked. apply accepts (0,0), (period,period), (duty,0) or (0,duty).
 * stop_all must attempt BOTH wheels even following a partial prepare/apply
 * failure, returning false if a non-driving output cannot be established.
 * Callbacks/context must remain alive; no allocation, transport or RTOS.
 */
typedef struct
{
    void * p_context;
    bool (*p_prepare)(void * p_context, uint16_t period, uint32_t clock_hz);
    bool (*p_apply)(void * p_context, drv_motor_wheel_t wheel,
                    uint16_t level_a, uint16_t level_b);
    bool (*p_stop_all)(void * p_context);
} drv_motor_io_t;

typedef struct
{
    drv_motor_state_t state;
    int8_t last_direction; /* Retained across every zero/stop request. */
    int8_t pending_direction;
    uint16_t pending_level;
    uint64_t neutral_since_us;
    bool b_neutral;
} drv_motor_channel_t;

/* Owner-only state; zero-initialize BEFORE init. One serialized owner across
 * both cores; local interrupt masking/volatile is insufficient. Do not use
 * these task-context APIs from ISRs or edit active config/backend state.
 */
typedef struct
{
    drv_motor_config_t config;
    drv_motor_io_t io;
    drv_motor_channel_t channel[DRV_MOTOR_WHEELS];
    uint64_t last_time_us;
    uint16_t period;
    uint16_t duty_limit;
    /* Owner-only flags, never a wire format or independent atomic fields. */
    bool b_initialised : 1;
    bool b_armed : 1;
    bool b_fault : 1;
    bool b_prepare_attempted : 1;
    /* Backend confirms a stop COMMAND, never physical standstill/current. */
    bool b_stop_confirmed : 1;
} drv_motor_t;

/** @brief Validate and copy immutable settings; performs NO hardware access. */
drv_motor_result_t drv_motor_init (drv_motor_t * p_motor,
    drv_motor_config_t const * p_config, drv_motor_io_t const * p_io);

/** @brief Explicit bench-only enable: configure hardware to BRAKE, no motion.
 * Never call from automatic startup or TEST=1. Live re-arm is rejected.
 */
drv_motor_result_t drv_motor_arm (drv_motor_t * p_motor, uint64_t now_us);

/** @brief Clamp finite signed vehicle effort to [-1,1] and the duty ceiling.
 * NaN/Inf, invalid arguments, backend failure or clock rollback latch a fault
 * and attempt BOTH-wheel brake. Zero/quantized-zero uses configured zero_mode.
 * An opposite electrical direction enters REVERSING with BRAKE immediately.
 * Repeated requests replace pending effort without restarting the guard.
 */
drv_motor_result_t drv_motor_set (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, motion_pid_value_t effort, uint64_t now_us);

/** @brief Cancel pending effort and command the explicit stop truth table.
 * Last drive direction and neutral timing survive stops to prevent bypassing
 * reversal protection via drive -> zero/brake/coast -> opposite drive.
 */
drv_motor_result_t drv_motor_stop (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, drv_motor_stop_t mode, uint64_t now_us);

/** @brief Nonblocking reversal progress; no sleeps and no inferred standstill.
 * Requires elapsed guard AND fresh external physical-standstill confirmation.
 * Future owner must define feedback freshness/thresholds; elapsed time alone
 * is NEVER permission to reverse. Confirmation must correspond to this wheel.
 */
drv_motor_result_t drv_motor_service (drv_motor_t * p_motor,
    drv_motor_wheel_t wheel, uint64_t now_us, bool b_standstill_confirmed);

/** @brief Brake both wheels and disable requests; faults remain latched.
 * Fault recovery requires a separately reviewed, unpowered recovery procedure;
 * do not clear a live object to bypass its fault or direction history.
 */
drv_motor_result_t drv_motor_disarm (drv_motor_t * p_motor);

#endif /* DRV_MOTOR_H */

/*** end of file ***/

