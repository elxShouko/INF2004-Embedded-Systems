/** @file bench_motion.h @brief Restricted production-driver bench policy. */
#ifndef MOTION_BENCH_H
#define MOTION_BENCH_H
#include "drv_motor.h"
#ifndef APP_ROBOT
#define APP_ROBOT 0
#endif

/* Engineering test limits, NOT calibrated vehicle operating values. */
#define MOTION_BENCH_POLL_US 10000U
#define MOTION_BENCH_LEASE_US 500000U
#define MOTION_BENCH_ARM_US 5000000U
#define MOTION_BENCH_MAX_PULSE_US 500000U
#define MOTION_BENCH_QUIET_US 100000U /* Count-quiet guard only. */

typedef enum
{
    MOTION_BENCH_ENCODERS = 1, MOTION_BENCH_ARM, MOTION_BENCH_PULSE
} motion_bench_request_t;

typedef struct
{
    drv_motor_config_t motor;
    int16_t effort_permille[2];
    uint32_t pulse_us;
    bool b_power_authorised : 1;
    bool b_wheels_raised : 1;
    bool b_encoder_on_boot : 1;
} motion_bench_config_t;

typedef struct
{
    uint64_t timestamp_us;
    uint64_t deadline_us;
    uint32_t invalid_edges[2];
    uint16_t pwm_level[2];
    uint16_t last_pulse_pwm[2]; /* Held commands, never physical feedback. */
    int16_t effort_permille[2];
    uint8_t result : 2; /* 0 none, 1 accepted, 2 rejected. */
    bool b_encoders : 1;
    bool b_count_quiet : 1;
    bool b_armed : 1;
    bool b_running : 1;
    bool b_used : 1;
    bool b_fault : 1;
} motion_bench_status_t;

typedef struct
{
    motion_bench_config_t config;
    motion_bench_status_t status;
    drv_motor_t motor;
    uint64_t previous_us;
    bool b_initialised : 1;
} motion_bench_t;

bool motion_bench_init (motion_bench_t * p_bench,
    motion_bench_config_t const * p_config, drv_motor_io_t const * p_io);
bool motion_bench_arm (motion_bench_t * p_bench, uint64_t now_us,
    bool b_encoder_healthy, bool b_lease);
bool motion_bench_pulse (motion_bench_t * p_bench, uint64_t now_us,
    bool b_encoder_healthy, bool b_lease);
void motion_bench_tick (motion_bench_t * p_bench, uint64_t now_us,
    bool b_encoder_healthy, bool b_lease, bool b_stop, bool b_emergency);
#endif /* MOTION_BENCH_H */

/*** end of file ***/
