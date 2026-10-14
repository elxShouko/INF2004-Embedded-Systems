/**
 * @file drv_rp2040.h
 * @brief Explicitly bound Robo Pico backend; never called by startup/tests.
 */
#ifndef DRV_RP2040_H
#define DRV_RP2040_H

#include "drv_motor.h"

typedef struct
{
    uint16_t period;
    int8_t direction[DRV_MOTOR_WHEELS];
    bool b_claimed;
} drv_rp2040_t;

/** @brief Bind a fresh, zero-initialized backend; NO hardware operations.
 * One backend owns GP8..11 and PWM4/5 exclusively. Reserve physical timers
 * 5/6 and mirrored PWM pins GP24..27; do not enable PTMR or I2C conflicts.
 * Binding again while claimed is rejected. Call only during explicit bench
 * setup; motor arm performs the real configuration. Not a shared team API.
 */
bool drv_rp2040_bind (drv_rp2040_t * p_backend,
                            drv_motor_io_t * p_io);

#endif /* DRV_RP2040_H */

/*** end of file ***/

