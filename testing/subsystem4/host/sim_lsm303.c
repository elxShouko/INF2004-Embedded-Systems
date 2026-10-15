/**
 * @file    sim_lsm303.c
 * @brief   HOST-ONLY LSM303DLHC register simulator (see sim_lsm303.h).
 */

#include <math.h>
#include <string.h>

#include "imu/lsm303dlhc_regs.h"
#include "sim_lsm303.h"

#define SIM_REGS 0x40U

typedef struct
{
    sim_config_t cfg;
    uint8_t      a_regs[SIM_REGS];
    uint8_t      m_regs[SIM_REGS];
    imu_raw3_t   fifo[LSM303_FIFO_DEPTH];
    uint32_t     fifo_count;
    bool         b_fifo_ovrn;
    imu_raw3_t   latest;
    bool         b_new;
    bool         b_overrun;
    double       now_ms;
    double       next_sample_ms;
    imu_raw3_t   frozen_val;
    bool         b_frozen_val;
} sim_state_t;

static sim_state_t g_sim;

static uint32_t sim_rand(void)
{
    g_sim.cfg.rng = (g_sim.cfg.rng * 1664525U) + 1013904223U;
    return (g_sim.cfg.rng >> 8);
}

void sim_reset(void)
{
    (void)memset(&g_sim, 0, sizeof(g_sim));
    g_sim.a_regs[LSM303_REG_CTRL_REG1_A] = 0x07U;   /* power-on default */
    g_sim.m_regs[LSM303_REG_CRA_REG_M]   = 0x10U;
    g_sim.m_regs[LSM303_REG_CRB_REG_M]   = 0x20U;
    g_sim.m_regs[LSM303_REG_MR_REG_M]    = 0x03U;
    g_sim.cfg.const_accel.z = 1.0f;
    g_sim.cfg.const_mag.y   = 0.40f;
    g_sim.cfg.const_mag.z   = 0.12f;
    g_sim.cfg.rng           = 12345U;
    g_sim.cfg.fail_status   = IMU_HAL_NACK;
}

sim_config_t * sim_cfg(void)
{
    return (&g_sim.cfg);
}

uint32_t sim_time_ms(void)
{
    return ((uint32_t)(uint64_t)g_sim.now_ms);
}

void sim_set_time_ms(uint32_t ms)
{
    g_sim.now_ms         = (double)ms;
    g_sim.next_sample_ms = (double)ms;
}

uint32_t sim_fifo_level(void)
{
    return (g_sim.fifo_count);
}

uint8_t sim_accel_reg(uint8_t reg)
{
    return (g_sim.a_regs[reg & 0x3FU]);
}

uint8_t sim_mag_reg(uint8_t reg)
{
    return (g_sim.m_regs[reg & 0x3FU]);
}

static double sim_odr_hz(void)
{
    static double const k_odr[8] = { 0.0, 1.0, 10.0, 25.0, 50.0, 100.0,
                                     200.0, 400.0 };
    uint8_t code = (uint8_t)(g_sim.a_regs[LSM303_REG_CTRL_REG1_A] >> 4);

    return ((code < 8U) ? k_odr[code] : 1344.0);
}

static double sim_accel_sens(void)
{
    static double const k_sens[4] = { 0.001, 0.002, 0.004, 0.012 };

    return (k_sens[(g_sim.a_regs[LSM303_REG_CTRL_REG4_A] >> 4) & 0x03U]);
}

static int16_t sim_quant_accel(float g)
{
    double c = floor(((double)g / sim_accel_sens()) + 0.5);

    if (c > 2047.0)  { c = 2047.0; }
    if (c < -2048.0) { c = -2048.0; }

    return ((int16_t)c);
}

static void sim_stimulus(double t_ms, imu_vec3_t * p_a, imu_vec3_t * p_m)
{
    if (NULL != g_sim.cfg.p_fn)
    {
        g_sim.cfg.p_fn(g_sim.cfg.p_ctx, t_ms * 0.001, p_a, p_m);
    }
    else
    {
        *p_a = g_sim.cfg.const_accel;
        *p_m = g_sim.cfg.const_mag;
    }
}

static void sim_push_sample(double t_ms)
{
    imu_vec3_t a;
    imu_vec3_t m;
    imu_raw3_t r;

    sim_stimulus(t_ms, &a, &m);
    r.x = sim_quant_accel(a.x);
    r.y = sim_quant_accel(a.y);
    r.z = sim_quant_accel(a.z);

    if (g_sim.cfg.b_frozen)
    {
        if (!g_sim.b_frozen_val)
        {
            g_sim.frozen_val   = r;
            g_sim.b_frozen_val = true;
        }
        r = g_sim.frozen_val;
    }
    else
    {
        g_sim.b_frozen_val = false;
    }

    if (g_sim.b_new)
    {
        g_sim.b_overrun = true;
    }
    g_sim.latest = r;
    g_sim.b_new  = true;

    if ((0U != (g_sim.a_regs[LSM303_REG_CTRL_REG5_A] & LSM303_CTRL5_FIFO_EN))
        && (LSM303_FIFO_MODE_STREAM
            == (g_sim.a_regs[LSM303_REG_FIFO_CTRL_REG_A] & 0xC0U)))
    {
        if (g_sim.fifo_count >= LSM303_FIFO_DEPTH)
        {
            (void)memmove(&g_sim.fifo[0], &g_sim.fifo[1],
                          sizeof(g_sim.fifo[0]) * (LSM303_FIFO_DEPTH - 1U));
            g_sim.fifo_count  = LSM303_FIFO_DEPTH - 1U;
            g_sim.b_fifo_ovrn = true;
        }
        g_sim.fifo[g_sim.fifo_count] = r;
        g_sim.fifo_count++;
    }
}

void sim_advance_ms(uint32_t ms)
{
    double end = g_sim.now_ms + (double)ms;
    double odr = sim_odr_hz();

    if ((odr > 0.0)
        && (0U != (g_sim.a_regs[LSM303_REG_CTRL_REG1_A] & 0x07U)))
    {
        double period = 1000.0 / odr;

        if (g_sim.next_sample_ms < g_sim.now_ms)
        {
            g_sim.next_sample_ms = g_sim.now_ms;
        }
        while (g_sim.next_sample_ms <= end)
        {
            sim_push_sample(g_sim.next_sample_ms);
            g_sim.next_sample_ms += period;
        }
    }
    g_sim.now_ms = end;
}

/* ---------------------------------------------------------------------- */

static bool sim_inject_failure(imu_hal_status_t * p_status)
{
    g_sim.cfg.transfers++;

    if (g_sim.cfg.b_bus_stuck)
    {
        *p_status = IMU_HAL_TIMEOUT;
        g_sim.cfg.failures_injected++;
        return (true);
    }
    if (g_sim.cfg.fail_next > 0U)
    {
        g_sim.cfg.fail_next--;
        *p_status = g_sim.cfg.fail_status;
        g_sim.cfg.failures_injected++;
        return (true);
    }
    if ((g_sim.cfg.fail_every > 0U)
        && (0U == (g_sim.cfg.transfers % g_sim.cfg.fail_every)))
    {
        *p_status = g_sim.cfg.fail_status;
        g_sim.cfg.failures_injected++;
        return (true);
    }
    if (!g_sim.cfg.b_bus_initialised)
    {
        *p_status = IMU_HAL_ERROR;
        return (true);
    }

    return (false);
}

static void sim_encode_accel(imu_raw3_t const * p_r, uint8_t * p_buf)
{
    int16_t const v[3] = { p_r->x, p_r->y, p_r->z };
    uint8_t       i;

    for (i = 0U; i < 3U; i++)
    {
        uint16_t w = (uint16_t)((uint16_t)v[i] << 4);

        p_buf[2U * i]        = (uint8_t)(w & 0xFFU);
        p_buf[(2U * i) + 1U] = (uint8_t)(w >> 8);
    }
}

static int16_t sim_mag_count(float gauss, bool b_z)
{
    static double const k_xy[8] = { 0, 1100, 855, 670, 450, 400, 330, 230 };
    static double const k_z[8]  = { 0, 980, 760, 600, 400, 355, 295, 205 };
    uint8_t gn = (uint8_t)(g_sim.m_regs[LSM303_REG_CRB_REG_M] >> 5);
    double  c  = floor(((double)gauss * (b_z ? k_z[gn] : k_xy[gn])) + 0.5);

    if (g_sim.cfg.b_mag_overflow || (c > 2047.0) || (c < -2048.0))
    {
        return ((int16_t)LSM303_MAG_OVERFLOW_VALUE);
    }

    return ((int16_t)c);
}

static void sim_read_accel(uint8_t reg, uint8_t * p_rx, size_t len)
{
    uint8_t addr = (uint8_t)(reg & 0x7FU);
    size_t  i;

    if ((LSM303_REG_OUT_X_L_A == addr) && (6U == len))
    {
        imu_raw3_t r;

        if ((0U != (g_sim.a_regs[LSM303_REG_CTRL_REG5_A]
                    & LSM303_CTRL5_FIFO_EN)) && (g_sim.fifo_count > 0U))
        {
            r = g_sim.fifo[0];
            (void)memmove(&g_sim.fifo[0], &g_sim.fifo[1],
                          sizeof(g_sim.fifo[0]) * (LSM303_FIFO_DEPTH - 1U));
            g_sim.fifo_count--;
            g_sim.b_fifo_ovrn = false;
        }
        else
        {
            r = g_sim.latest;
        }
        g_sim.b_new     = false;
        g_sim.b_overrun = false;
        sim_encode_accel(&r, p_rx);
        return;
    }

    for (i = 0U; i < len; i++)
    {
        uint8_t a = (uint8_t)((addr + i) & 0x3FU);
        uint8_t v = g_sim.a_regs[a];

        if (LSM303_REG_STATUS_REG_A == a)
        {
            v = (uint8_t)((g_sim.b_new ? LSM303_STATUS_ZYXDA : 0U)
                          | (g_sim.b_overrun ? LSM303_STATUS_ZYXOR : 0U));
        }
        else if (LSM303_REG_FIFO_SRC_REG_A == a)
        {
            if (g_sim.fifo_count >= LSM303_FIFO_DEPTH)
            {
                v = (uint8_t)(LSM303_FIFO_SRC_OVRN | 0x1FU);
            }
            else if (0U == g_sim.fifo_count)
            {
                v = LSM303_FIFO_SRC_EMPTY;
            }
            else
            {
                v = (uint8_t)g_sim.fifo_count;
            }
        }
        else
        {
            /* plain register */
        }
        p_rx[i] = v;
    }
}

static void sim_read_mag(uint8_t reg, uint8_t * p_rx, size_t len)
{
    imu_vec3_t a;
    imu_vec3_t m;
    int16_t    out[3];
    uint8_t    buf[SIM_REGS];
    size_t     i;

    (void)memcpy(buf, g_sim.m_regs, sizeof(buf));
    buf[LSM303_REG_IRA_REG_M]      = g_sim.cfg.b_wrong_id ? 0x00U
                                                          : LSM303_IRA_VALUE;
    buf[LSM303_REG_IRA_REG_M + 1U] = LSM303_IRB_VALUE;
    buf[LSM303_REG_IRA_REG_M + 2U] = LSM303_IRC_VALUE;
    buf[LSM303_REG_SR_REG_M]       = LSM303_SR_DRDY;

    sim_stimulus(g_sim.now_ms, &a, &m);
    out[0] = sim_mag_count(m.x, false);
    out[1] = sim_mag_count(m.z, true);
    out[2] = sim_mag_count(m.y, false);
    for (i = 0U; i < 3U; i++)
    {
        buf[LSM303_REG_OUT_X_H_M + (2U * i)]      =
            (uint8_t)((uint16_t)out[i] >> 8);
        buf[LSM303_REG_OUT_X_H_M + (2U * i) + 1U] =
            (uint8_t)((uint16_t)out[i] & 0xFFU);
    }

    for (i = 0U; i < len; i++)
    {
        p_rx[i] = buf[(reg + i) & 0x3FU];
    }
}

/* ------------------------------ HAL ---------------------------------- */

imu_hal_status_t imu_hal_i2c_init(uint32_t baud_hz)
{
    (void)baud_hz;
    g_sim.cfg.init_calls++;
    g_sim.cfg.b_bus_initialised = true;
    return (IMU_HAL_OK);
}

void imu_hal_i2c_deinit(void)
{
    g_sim.cfg.b_bus_initialised = false;
}

imu_hal_status_t imu_hal_i2c_write(uint8_t addr7, uint8_t const * p_data,
                                   size_t len, uint32_t timeout_us)
{
    imu_hal_status_t st;
    uint8_t          reg;

    (void)timeout_us;
    if (sim_inject_failure(&st))
    {
        return (st);
    }
    if ((LSM303_ACCEL_ADDR == addr7) && g_sim.cfg.b_accel_absent)
    {
        return (IMU_HAL_NACK);
    }
    if ((LSM303_MAG_ADDR == addr7) && g_sim.cfg.b_mag_absent)
    {
        return (IMU_HAL_NACK);
    }
    if ((LSM303_ACCEL_ADDR != addr7) && (LSM303_MAG_ADDR != addr7))
    {
        return (IMU_HAL_NACK);
    }
    if ((NULL == p_data) || (len < 2U))
    {
        return (IMU_HAL_ERROR);
    }

    reg = (uint8_t)(p_data[0] & 0x3FU);
    if (!g_sim.cfg.b_ignore_writes)
    {
        if (LSM303_ACCEL_ADDR == addr7)
        {
            g_sim.a_regs[reg] = p_data[1];
            if ((LSM303_REG_FIFO_CTRL_REG_A == reg)
                && (LSM303_FIFO_MODE_BYPASS == (p_data[1] & 0xC0U)))
            {
                g_sim.fifo_count  = 0U;
                g_sim.b_fifo_ovrn = false;
            }
        }
        else
        {
            g_sim.m_regs[reg] = p_data[1];
        }
    }

    return (IMU_HAL_OK);
}

imu_hal_status_t imu_hal_i2c_write_read(uint8_t addr7,
                                        uint8_t const * p_tx, size_t tx_len,
                                        uint8_t * p_rx, size_t rx_len,
                                        uint32_t timeout_us)
{
    imu_hal_status_t st;
    size_t           i;

    (void)timeout_us;
    if (sim_inject_failure(&st))
    {
        return (st);
    }
    if ((LSM303_ACCEL_ADDR == addr7) && g_sim.cfg.b_accel_absent)
    {
        return (IMU_HAL_NACK);
    }
    if ((LSM303_MAG_ADDR == addr7) && g_sim.cfg.b_mag_absent)
    {
        return (IMU_HAL_NACK);
    }
    if ((LSM303_ACCEL_ADDR != addr7) && (LSM303_MAG_ADDR != addr7))
    {
        return (IMU_HAL_NACK);
    }
    if ((NULL == p_tx) || (NULL == p_rx) || (1U != tx_len) || (0U == rx_len))
    {
        return (IMU_HAL_ERROR);
    }

    if (LSM303_ACCEL_ADDR == addr7)
    {
        sim_read_accel(p_tx[0], p_rx, rx_len);
    }
    else
    {
        sim_read_mag(p_tx[0], p_rx, rx_len);
    }

    if (g_sim.cfg.b_garbage)
    {
        for (i = 0U; i < rx_len; i++)
        {
            p_rx[i] = (uint8_t)(sim_rand() & 0xFFU);
        }
    }

    return (IMU_HAL_OK);
}

void imu_hal_i2c_bus_recover(void)
{
    g_sim.cfg.recover_calls++;
    if (!g_sim.cfg.b_stuck_survives_recovery)
    {
        g_sim.cfg.b_bus_stuck = false;
    }
}

uint32_t imu_hal_time_ms(void)
{
    return (sim_time_ms());
}

uint32_t imu_hal_time_us(void)
{
    return ((uint32_t)(uint64_t)(g_sim.now_ms * 1000.0));
}
