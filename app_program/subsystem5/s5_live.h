/** @file s5_live.h @brief Bounded copied obstacle provider, task context. */
#ifndef S5_LIVE_H
#define S5_LIVE_H
#include <stddef.h>
#ifndef PROHIBIT_DEF_SIZE_T
#define PROHIBIT_DEF_SIZE_T 1
#endif
#include "../app.h"
#include "drv_s5.h"

typedef struct
{
    s5_acquire_config_t acquisition;
    s5_runtime_config_t policy;
} s5_live_config_t;

typedef struct
{
    s5_runtime_status_t policy;
    uint64_t scan_id_us;
    uint64_t scan_completed_us;
    uint64_t forward_started_us; /* Conservative clock read BEFORE trigger. */
    uint64_t forward_timestamp_us; /* Actual echo capture, raw timer domain. */
    uint32_t max_age_us;
    uint16_t forward_cm;
    bool b_configured;
    bool b_forward_valid;
    bool b_fault;
} s5_live_status_t;

ER s5_live_init (void);
ER s5_live_start (void);
/* Pre-start initial task only; immutable and explicit calibration opt-in.
 * No defaults, no powered GPIO from automatic init/start. Only confirmed GP15
 * servo wiring is supported; do not set confirmation before physical checks.
 */
ER s5_live_configure (s5_live_config_t const * p_config,
    drv_encoder_port_t const * p_irq);
ER s5_live_status (s5_live_status_t * p_status);
/*
 * Controller-only resume after bypass/recovery and matched stopped completion.
 * The exact held-scan ID is required; LINE_FOUND by itself is not a resume.
 */
ER s5_live_recovered (uint64_t scan_id_us);
ER s5_live_fault (void); /* TMO_POL latch read; never clears on read. */
/* Latch; owner disables outputs on its next cycle; reboot to recover. */
ER s5_live_cancel (void);
#endif
/*** end of file ***/
