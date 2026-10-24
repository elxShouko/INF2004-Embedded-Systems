/** @file drv_s5.h @brief Explicit opt-in GPIO/servo/echo port, core 1 only. */
#ifndef DRV_S5_H
#define DRV_S5_H
#include "s5_acquire.h"
#include "../subsystem2/drv_encoder.h"

typedef struct
{
    drv_encoder_port_t irq;
    s5_acquire_config_t config;
    volatile uint64_t rise_us;
    volatile uint64_t fall_us;
    volatile uint32_t width_us;
    bool b_reserved;
    bool b_claimed;
    /* Private same-core ISR/task state, not an atomic SMP publication.
     * IDLE=0, ARMED=1, RISE=2, COMPLETE=3, ERROR=4; all others fail closed.
     * Task reads/updates use DI/EI; the shared dispatcher runs on core 1.
     */
    volatile uint8_t echo_state;
} drv_s5_t;

bool drv_s5_now (uint64_t * p_now);
bool drv_s5_bind (drv_s5_t * p_driver, drv_encoder_port_t const * p_irq,
    s5_acquire_io_t * p_io);
#endif
/*** end of file ***/
