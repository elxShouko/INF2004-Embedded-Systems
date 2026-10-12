/**
 * @file    sim_lsm303.h
 * @brief   HOST-ONLY register-level LSM303DLHC simulator implementing the
 *          imu_hal.h interface. Used by unit, black-box and robustness tests.
 *          Never linked into firmware.
 *
 * Models: accelerometer CTRL/STATUS/FIFO (stream mode, 32 deep, OVRN),
 * 12-bit left-justified little-endian output, ODR from CTRL_REG1_A,
 * full-scale from CTRL_REG4_A; magnetometer CRA/CRB/MR, ID registers,
 * big-endian X/Z/Y output, -4096 overflow, gain-dependent scaling.
 *
 * Fault injection: absent device (NACK), timeouts, intermittent failures,
 * stuck bus (fails until bus-recovery), wrong ID, write-ignore (read-back
 * mismatch), frozen output, random garbage bytes, magnetometer overflow.
 */

#ifndef SIM_LSM303_H
#define SIM_LSM303_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_hal.h"
#include "imu/imu_types.h"

/** Physical stimulus: specific force (g) and field (gauss), SENSOR frame. */
typedef void (*sim_stimulus_fn_t)(void * p_ctx, double t_s,
                                  imu_vec3_t * p_accel_g,
                                  imu_vec3_t * p_mag_gauss);

typedef struct
{
    /* stimulus */
    sim_stimulus_fn_t p_fn;
    void *            p_ctx;
    imu_vec3_t        const_accel;   /* used when p_fn == NULL */
    imu_vec3_t        const_mag;

    /* fault injection */
    bool             b_accel_absent;
    bool             b_mag_absent;
    bool             b_wrong_id;
    bool             b_ignore_writes;
    bool             b_frozen;
    bool             b_garbage;
    bool             b_mag_overflow;
    bool             b_bus_stuck;        /* cleared by bus recovery */
    bool             b_stuck_survives_recovery;
    uint32_t         fail_next;          /* fail the next N transfers */
    imu_hal_status_t fail_status;
    uint32_t         fail_every;         /* fail every Nth transfer (0=off) */
    uint32_t         rng;

    /* observation */
    uint32_t transfers;
    uint32_t failures_injected;
    uint32_t recover_calls;
    uint32_t init_calls;
    bool     b_bus_initialised;
} sim_config_t;

/** Reset the simulator (registers to power-on defaults, time = 0). */
void   sim_reset(void);

/** Access the configuration / fault-injection block. */
sim_config_t * sim_cfg(void);

/** Advance simulated time, generating accelerometer samples at the ODR. */
void   sim_advance_ms(uint32_t ms);

/** Set absolute time without generating samples (for wrap tests). */
void   sim_set_time_ms(uint32_t ms);

uint32_t sim_time_ms(void);

/** Number of samples currently in the simulated FIFO. */
uint32_t sim_fifo_level(void);

/** Read a raw register value (white-box checks). */
uint8_t sim_accel_reg(uint8_t reg);
uint8_t sim_mag_reg(uint8_t reg);

#endif /* SIM_LSM303_H */
