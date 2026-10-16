/** @file drv_s5.h @brief Explicit opt-in GPIO/servo/echo port, core 1 only. */
#ifndef DRV_S5_H
#define DRV_S5_H
#include "s5_acquire.h"
#include "../subsystem2/drv_encoder.h"

typedef struct
{
    drv_encoder_port_t irq;
    s5_acquire_config_t config;
    uint64_t rise_us;
    uint64_t fall_us;
    uint32_t width_us;
    bool b_reserved;
    bool b_claimed;
    bool b_armed;
    bool b_rise;
    bool b_complete;
    bool b_error;
} drv_s5_t;

bool drv_s5_now (uint64_t * p_now);
bool drv_s5_bind (drv_s5_t * p_driver, drv_encoder_port_t const * p_irq,
    s5_acquire_io_t * p_io);
#endif
/*** end of file ***/
