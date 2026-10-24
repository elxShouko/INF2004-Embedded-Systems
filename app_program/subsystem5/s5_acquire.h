/** @file s5_acquire.h @brief Single-owner bounded ultrasonic scan engine. */
#ifndef S5_ACQUIRE_H
#define S5_ACQUIRE_H
#include "s5_obstacle_runtime.h"

typedef struct
{
    uint32_t sample_period_us;
    uint32_t echo_timeout_us;
    uint32_t servo_settle_us;
    uint32_t scan_timeout_us;
    uint16_t servo_period_us;
    uint16_t servo_min_us;
    uint16_t servo_max_us;
    uint16_t echo_us_per_cm;
    uint16_t range_delta_cm;
    uint8_t servo_gpio;
    bool b_wiring_confirmed;
    bool b_calibrated;
} s5_acquire_config_t;

typedef struct
{
    void * p_context;
    bool (*p_open)(void *, s5_acquire_config_t const *);
    bool (*p_servo)(void *, uint8_t);
    bool (*p_trigger)(void *);
    /* 0 pending, 1 captured width/time, -1 fault. */
    int32_t (*p_echo)(void *, uint32_t *, uint64_t *);
    void (*p_close)(void *);
} s5_acquire_io_t;

typedef enum
{
    S5_ACQUIRE_IDLE = 0, S5_ACQUIRE_SETTLE, S5_ACQUIRE_ECHO,
    S5_ACQUIRE_FAULT
} s5_acquire_state_t;

typedef struct
{
    s5_acquire_config_t config;
    s5_acquire_io_t io;
    uint64_t last_us;
    uint64_t deadline_us;
    uint64_t scan_start_us;
    uint64_t completed_us;
    uint64_t forward_us;
    uint64_t forward_started_us;
    uint64_t next_trigger_us;
    s5_scan_context_t scan;
    s5_scan_sample_t coarse[S5_COARSE_SCAN_COUNT];
    s5_scan_sample_t fine[S5_FINE_SCAN_MAX_COUNT];
    uint16_t forward_cm;
    uint8_t angles[S5_FINE_SCAN_MAX_COUNT];
    /* Owner-private storage: 0..9 counts, 0..3 states; no wire encoding. */
    uint8_t index;
    uint8_t fine_count;
    uint8_t state;
    bool b_open;
    bool b_scan;
    bool b_fine;
    bool b_scan_complete;
    bool b_forward_valid;
} s5_acquire_t;

bool s5_acquire_validate (s5_acquire_config_t const * p_config);
bool s5_acquire_open (s5_acquire_t * p_owner,
    s5_acquire_config_t const * p_config, s5_acquire_io_t const * p_io,
    uint64_t now_us);
bool s5_acquire_step (s5_acquire_t * p_owner, uint64_t now_us,
    bool b_scan_requested, bool b_stationary, bool b_cancel);
void s5_acquire_close (s5_acquire_t * p_owner);
#endif
/*** end of file ***/
