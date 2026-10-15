/**
 * @file    imu_lsm303dlhc.c
 * @brief   Low-level LSM303DLHC driver (see imu_lsm303dlhc.h).
 */

#include <stddef.h>
#include <string.h>

#include "imu/imu_lsm303dlhc.h"
#include "imu/lsm303dlhc_regs.h"

#define ACCEL_SAMPLE_BYTES     (6U)
#define MAG_SAMPLE_BYTES       (6U)
#define MAG_ID_BYTES           (3U)
#define ACCEL_JUSTIFY_DIV      (16)     /* 12-bit left-justified -> >> 4 */

/* ------------------------------------------------------------------------ */
/*  Pure helpers                                                             */
/* ------------------------------------------------------------------------ */

int16_t lsm303_decode_accel_axis(uint8_t lo, uint8_t hi)
{
    int32_t word = (int32_t)(int16_t)(uint16_t)(((uint16_t)hi << 8) | lo);

    /* Portable arithmetic right shift by 4 (floor division by 16). */
    if (word < 0)
    {
        word = (word - (ACCEL_JUSTIFY_DIV - 1)) / ACCEL_JUSTIFY_DIV;
    }
    else
    {
        word = word / ACCEL_JUSTIFY_DIV;
    }

    return ((int16_t)word);
}

int16_t lsm303_decode_mag_axis(uint8_t hi, uint8_t lo)
{
    return ((int16_t)(uint16_t)(((uint16_t)hi << 8) | lo));
}

float lsm303_accel_sensitivity_g(uint8_t fs_code)
{
    float sens;

    /* Datasheet Table 3, high-resolution (12-bit) mode. */
    switch (fs_code)
    {
        case LSM303_ACCEL_FS_2G:  sens = 0.001f; break;
        case LSM303_ACCEL_FS_4G:  sens = 0.002f; break;
        case LSM303_ACCEL_FS_8G:  sens = 0.004f; break;
        case LSM303_ACCEL_FS_16G: sens = 0.012f; break;
        default:                  sens = 0.0f;   break;
    }

    return (sens);
}

static float lsm303_accel_full_scale(uint8_t fs_code)
{
    float fs;

    switch (fs_code)
    {
        case LSM303_ACCEL_FS_2G:  fs = 2.0f;  break;
        case LSM303_ACCEL_FS_4G:  fs = 4.0f;  break;
        case LSM303_ACCEL_FS_8G:  fs = 8.0f;  break;
        case LSM303_ACCEL_FS_16G: fs = 16.0f; break;
        default:                  fs = 0.0f;  break;
    }

    return (fs);
}

float lsm303_mag_lsb_per_gauss(uint8_t gain_code, bool b_z)
{
    /* Datasheet Table 3 / Table 75 (GN bits), X/Y and Z gains differ. */
    static float const k_xy[8] = { 0.0f, 1100.0f, 855.0f, 670.0f,
                                   450.0f, 400.0f, 330.0f, 230.0f };
    static float const k_z[8]  = { 0.0f, 980.0f, 760.0f, 600.0f,
                                   400.0f, 355.0f, 295.0f, 205.0f };
    float lsb = 0.0f;

    if (gain_code < 8U)
    {
        lsb = b_z ? k_z[gain_code] : k_xy[gain_code];
    }

    return (lsb);
}

imu_err_t lsm303_map_hal(imu_hal_status_t status)
{
    imu_err_t err;

    switch (status)
    {
        case IMU_HAL_OK:      err = IMU_OK;          break;
        case IMU_HAL_TIMEOUT: err = IMU_ERR_TIMEOUT; break;
        default:              err = IMU_ERR_I2C;     break;
    }

    return (err);
}

/* ------------------------------------------------------------------------ */
/*  Bus access with bounded retries                                          */
/* ------------------------------------------------------------------------ */

static void lsm303_account(lsm303_t * p_dev, imu_hal_status_t status)
{
    p_dev->stats.last_status = status;

    if (IMU_HAL_NACK == status)
    {
        p_dev->stats.nacks++;
    }
    else if (IMU_HAL_TIMEOUT == status)
    {
        p_dev->stats.timeouts++;
    }
    else
    {
        /* OK or generic error: nothing extra to count */
    }
}

static imu_err_t lsm303_read_regs(lsm303_t * p_dev, uint8_t addr7,
                                  uint8_t reg, uint8_t * p_buf, size_t len)
{
    imu_hal_status_t status = IMU_HAL_ERROR;
    uint8_t          attempt;

    p_dev->stats.transfers++;

    for (attempt = 0U; attempt <= p_dev->cfg.i2c_max_retries; attempt++)
    {
        if (attempt > 0U)
        {
            p_dev->stats.retries++;
        }

        status = imu_hal_i2c_write_read(addr7, &reg, 1U, p_buf, len,
                                        p_dev->cfg.i2c_timeout_us);
        lsm303_account(p_dev, status);

        if (IMU_HAL_OK == status)
        {
            break;
        }
    }

    if (IMU_HAL_OK != status)
    {
        p_dev->stats.failed_transfers++;
    }

    return (lsm303_map_hal(status));
}

static imu_err_t lsm303_write_reg(lsm303_t * p_dev, uint8_t addr7,
                                  uint8_t reg, uint8_t value)
{
    uint8_t const    buf[2] = { reg, value };
    imu_hal_status_t status = IMU_HAL_ERROR;
    uint8_t          attempt;

    p_dev->stats.transfers++;

    for (attempt = 0U; attempt <= p_dev->cfg.i2c_max_retries; attempt++)
    {
        if (attempt > 0U)
        {
            p_dev->stats.retries++;
        }

        status = imu_hal_i2c_write(addr7, buf, sizeof(buf),
                                   p_dev->cfg.i2c_timeout_us);
        lsm303_account(p_dev, status);

        if (IMU_HAL_OK == status)
        {
            break;
        }
    }

    if (IMU_HAL_OK != status)
    {
        p_dev->stats.failed_transfers++;
    }

    return (lsm303_map_hal(status));
}

/** Write a register and read it back (catches wrong device / bad wiring). */
static imu_err_t lsm303_write_verify(lsm303_t * p_dev, uint8_t addr7,
                                     uint8_t reg, uint8_t value)
{
    uint8_t   readback = 0U;
    imu_err_t err      = lsm303_write_reg(p_dev, addr7, reg, value);

    if (IMU_OK == err)
    {
        err = lsm303_read_regs(p_dev, addr7, reg, &readback, 1U);
    }

    if ((IMU_OK == err) && (readback != value))
    {
        err = IMU_ERR_VERIFY;
    }

    return (err);
}

/* ------------------------------------------------------------------------ */
/*  Configuration                                                            */
/* ------------------------------------------------------------------------ */

static imu_err_t lsm303_configure(lsm303_t * p_dev)
{
    uint8_t   id[MAG_ID_BYTES] = { 0U, 0U, 0U };
    uint8_t   ctrl1;
    uint8_t   ctrl4;
    imu_err_t err;

    /* 1. Magnetometer identity ('H','4','3'); auto-increment is implicit. */
    err = lsm303_read_regs(p_dev, LSM303_MAG_ADDR, LSM303_REG_IRA_REG_M,
                           id, sizeof(id));
    if ((IMU_OK == err) && p_dev->cfg.b_mag_id_check)
    {
        if ((LSM303_IRA_VALUE != id[0]) || (LSM303_IRB_VALUE != id[1])
            || (LSM303_IRC_VALUE != id[2]))
        {
            err = IMU_ERR_ID;
        }
    }

    /* 2. Accelerometer: ODR + all axes, BDU + FS + HR, FIFO. Read-back of
     *    CTRL_REG1_A/CTRL_REG4_A doubles as the accelerometer presence check
     *    (the DLHC accelerometer has no documented WHO_AM_I). */
    ctrl1 = (uint8_t)((uint8_t)(p_dev->cfg.accel_odr_code
                                << LSM303_CTRL1_ODR_SHIFT)
                      | LSM303_CTRL1_XYZ_EN);
    ctrl4 = (uint8_t)(LSM303_CTRL4_BDU
                      | (uint8_t)(p_dev->cfg.accel_fs_code
                                  << LSM303_CTRL4_FS_SHIFT)
                      | LSM303_CTRL4_HR);

    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_ACCEL_ADDR,
                                  LSM303_REG_CTRL_REG1_A, ctrl1);
    }
    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_ACCEL_ADDR,
                                  LSM303_REG_CTRL_REG4_A, ctrl4);
    }
    if (IMU_OK == err)
    {
        /* Bypass first: clears any stale FIFO content. */
        err = lsm303_write_reg(p_dev, LSM303_ACCEL_ADDR,
                               LSM303_REG_FIFO_CTRL_REG_A,
                               LSM303_FIFO_MODE_BYPASS);
    }
    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_ACCEL_ADDR,
                                  LSM303_REG_CTRL_REG5_A,
                                  p_dev->cfg.b_accel_fifo
                                      ? LSM303_CTRL5_FIFO_EN : 0U);
    }
    if ((IMU_OK == err) && p_dev->cfg.b_accel_fifo)
    {
        err = lsm303_write_verify(p_dev, LSM303_ACCEL_ADDR,
                                  LSM303_REG_FIFO_CTRL_REG_A,
                                  LSM303_FIFO_MODE_STREAM);
    }

    /* 3. Magnetometer: ODR, gain, continuous conversion. */
    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_MAG_ADDR,
                                  LSM303_REG_CRA_REG_M,
                                  (uint8_t)(p_dev->cfg.mag_odr_code
                                            << LSM303_CRA_DO_SHIFT));
    }
    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_MAG_ADDR,
                                  LSM303_REG_CRB_REG_M,
                                  (uint8_t)(p_dev->cfg.mag_gain_code
                                            << LSM303_CRB_GN_SHIFT));
    }
    if (IMU_OK == err)
    {
        err = lsm303_write_reg(p_dev, LSM303_MAG_ADDR, LSM303_REG_MR_REG_M,
                               LSM303_MR_CONTINUOUS);
    }

    return (err);
}

imu_err_t lsm303_init(lsm303_t * p_dev, lsm303_config_t const * p_cfg)
{
    imu_err_t err;

    if ((NULL == p_dev) || (NULL == p_cfg))
    {
        return (IMU_ERR_PARAM);
    }

    if ((0.0f >= lsm303_accel_sensitivity_g(p_cfg->accel_fs_code))
        || (0.0f >= lsm303_mag_lsb_per_gauss(p_cfg->mag_gain_code, false))
        || (p_cfg->accel_odr_code > LSM303_ACCEL_ODR_400HZ)
        || (0U == p_cfg->accel_odr_code)
        || (p_cfg->mag_odr_code > LSM303_MAG_ODR_220HZ))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_dev, 0, sizeof(*p_dev));
    p_dev->cfg = *p_cfg;
    p_dev->accel_g_per_lsb      =
        lsm303_accel_sensitivity_g(p_cfg->accel_fs_code);
    p_dev->accel_full_scale_g   = lsm303_accel_full_scale(p_cfg->accel_fs_code);
    p_dev->mag_xy_gauss_per_lsb =
        1.0f / lsm303_mag_lsb_per_gauss(p_cfg->mag_gain_code, false);
    p_dev->mag_z_gauss_per_lsb  =
        1.0f / lsm303_mag_lsb_per_gauss(p_cfg->mag_gain_code, true);

    err = lsm303_map_hal(imu_hal_i2c_init(p_cfg->i2c_baud_hz));

    if (IMU_OK == err)
    {
        err = lsm303_configure(p_dev);
    }

    p_dev->b_ready = (IMU_OK == err);

    return (err);
}

imu_err_t lsm303_recover(lsm303_t * p_dev)
{
    imu_err_t err;

    if (NULL == p_dev)
    {
        return (IMU_ERR_PARAM);
    }

    p_dev->stats.recoveries++;
    p_dev->b_ready = false;

    imu_hal_i2c_bus_recover();
    err = lsm303_map_hal(imu_hal_i2c_init(p_dev->cfg.i2c_baud_hz));

    if (IMU_OK == err)
    {
        err = lsm303_configure(p_dev);
    }

    p_dev->b_ready = (IMU_OK == err);

    return (err);
}

void lsm303_shutdown(lsm303_t * p_dev)
{
    if (NULL == p_dev)
    {
        return;
    }

    if (p_dev->b_ready)
    {
        (void)lsm303_write_reg(p_dev, LSM303_MAG_ADDR, LSM303_REG_MR_REG_M,
                               LSM303_MR_SLEEP);
        (void)lsm303_write_reg(p_dev, LSM303_ACCEL_ADDR,
                               LSM303_REG_CTRL_REG1_A, 0U); /* power-down */
    }

    p_dev->b_ready = false;
    imu_hal_i2c_deinit();
}

/* ------------------------------------------------------------------------ */
/*  Data acquisition                                                         */
/* ------------------------------------------------------------------------ */

static imu_err_t lsm303_read_one_accel(lsm303_t * p_dev, imu_raw3_t * p_out)
{
    uint8_t   buf[ACCEL_SAMPLE_BYTES];
    imu_err_t err;

    err = lsm303_read_regs(p_dev, LSM303_ACCEL_ADDR,
                           (uint8_t)(LSM303_REG_OUT_X_L_A
                                     | LSM303_ACCEL_AUTO_INC),
                           buf, sizeof(buf));
    if (IMU_OK == err)
    {
        p_out->x = lsm303_decode_accel_axis(buf[0], buf[1]);
        p_out->y = lsm303_decode_accel_axis(buf[2], buf[3]);
        p_out->z = lsm303_decode_accel_axis(buf[4], buf[5]);
    }

    return (err);
}

imu_err_t lsm303_read_accel(lsm303_t * p_dev, imu_raw3_t * p_samples,
                            uint8_t max_samples, uint8_t * p_count,
                            bool * p_b_overrun)
{
    uint8_t   reg   = 0U;
    uint8_t   avail = 0U;
    uint8_t   i;
    imu_err_t err;

    if ((NULL == p_dev) || (NULL == p_samples) || (NULL == p_count)
        || (NULL == p_b_overrun) || (0U == max_samples))
    {
        return (IMU_ERR_PARAM);
    }

    *p_count     = 0U;
    *p_b_overrun = false;

    if (!p_dev->b_ready)
    {
        return (IMU_ERR_NOT_READY);
    }

    if (p_dev->cfg.b_accel_fifo)
    {
        err = lsm303_read_regs(p_dev, LSM303_ACCEL_ADDR,
                               LSM303_REG_FIFO_SRC_REG_A, &reg, 1U);
        if (IMU_OK == err)
        {
            if (0U != (reg & LSM303_FIFO_SRC_OVRN))
            {
                /* FIFO full: 32 samples stored, older ones were lost. */
                avail = (uint8_t)LSM303_FIFO_DEPTH;
                *p_b_overrun = true;
                p_dev->stats.fifo_overruns++;
            }
            else if (0U != (reg & LSM303_FIFO_SRC_EMPTY))
            {
                avail = 0U;
            }
            else
            {
                avail = (uint8_t)(reg & LSM303_FIFO_SRC_FSS_MASK);
            }
        }
    }
    else
    {
        err = lsm303_read_regs(p_dev, LSM303_ACCEL_ADDR,
                               LSM303_REG_STATUS_REG_A, &reg, 1U);
        if (IMU_OK == err)
        {
            avail = (0U != (reg & LSM303_STATUS_ZYXDA)) ? 1U : 0U;
            if (0U != (reg & LSM303_STATUS_ZYXOR))
            {
                *p_b_overrun = true;
                p_dev->stats.accel_overruns++;
            }
        }
    }

    if (avail > max_samples)
    {
        avail = max_samples;   /* the rest stays in the FIFO */
    }

    for (i = 0U; (IMU_OK == err) && (i < avail); i++)
    {
        err = lsm303_read_one_accel(p_dev, &p_samples[i]);
        if (IMU_OK == err)
        {
            *p_count = (uint8_t)(i + 1U);
        }
    }

    return (err);
}

imu_err_t lsm303_read_mag(lsm303_t * p_dev, imu_raw3_t * p_raw,
                          bool * p_b_overflow)
{
    uint8_t   buf[MAG_SAMPLE_BYTES];
    imu_err_t err;

    if ((NULL == p_dev) || (NULL == p_raw) || (NULL == p_b_overflow))
    {
        return (IMU_ERR_PARAM);
    }

    *p_b_overflow = false;

    if (!p_dev->b_ready)
    {
        return (IMU_ERR_NOT_READY);
    }

    err = lsm303_read_regs(p_dev, LSM303_MAG_ADDR, LSM303_REG_OUT_X_H_M,
                           buf, sizeof(buf));
    if (IMU_OK == err)
    {
        /* Register order is X, Z, Y (big-endian). */
        p_raw->x = lsm303_decode_mag_axis(buf[0], buf[1]);
        p_raw->z = lsm303_decode_mag_axis(buf[2], buf[3]);
        p_raw->y = lsm303_decode_mag_axis(buf[4], buf[5]);

        if ((LSM303_MAG_OVERFLOW_VALUE == p_raw->x)
            || (LSM303_MAG_OVERFLOW_VALUE == p_raw->y)
            || (LSM303_MAG_OVERFLOW_VALUE == p_raw->z))
        {
            *p_b_overflow = true;
            p_dev->stats.mag_overflows++;
        }
    }

    return (err);
}

void lsm303_accel_to_g(lsm303_t const * p_dev, imu_raw3_t const * p_raw,
                       imu_vec3_t * p_out)
{
    if ((NULL == p_dev) || (NULL == p_raw) || (NULL == p_out))
    {
        return;
    }

    p_out->x = (float)p_raw->x * p_dev->accel_g_per_lsb;
    p_out->y = (float)p_raw->y * p_dev->accel_g_per_lsb;
    p_out->z = (float)p_raw->z * p_dev->accel_g_per_lsb;
}

void lsm303_mag_to_gauss(lsm303_t const * p_dev, imu_raw3_t const * p_raw,
                         imu_vec3_t * p_out)
{
    if ((NULL == p_dev) || (NULL == p_raw) || (NULL == p_out))
    {
        return;
    }

    p_out->x = (float)p_raw->x * p_dev->mag_xy_gauss_per_lsb;
    p_out->y = (float)p_raw->y * p_dev->mag_xy_gauss_per_lsb;
    p_out->z = (float)p_raw->z * p_dev->mag_z_gauss_per_lsb;
}
