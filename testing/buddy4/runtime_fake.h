/** @file runtime_fake.h Deterministic RTOS fault injection, no threads. */
#ifndef RUNTIME_FAKE_H
#define RUNTIME_FAKE_H

#include "../../app_program/subsystem4/imu_runtime.h"

typedef struct
{
    int32_t   fail_create;
    uint32_t  create_calls;
    uint32_t  time_ms;
    uint32_t  samples;
    uint32_t  flags;
    uint32_t  exits;
    uint32_t  sends;
    uint32_t  lock_depth;
    bool      b_started;
    bool      b_open_failure;
    bool      b_full_queue;
    bool      b_stale;
    bool      b_delay_failure;
    bool      b_overrun;
    bool      b_lock_failure;
    bool      b_start_failure;
    bool      b_violation;
} runtime_fake_state_t;

extern runtime_fake_state_t g_fake;
void runtime_fake_run (void);

#endif /* RUNTIME_FAKE_H */

