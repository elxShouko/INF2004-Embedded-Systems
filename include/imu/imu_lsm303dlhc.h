/**
 * @file    imu_lsm303dlhc.h
 * @brief   Low-level LSM303DLHC accelerometer + magnetometer driver.
 *
 * Responsibilities: register configuration, identity verification, data
 * acquisition (FIFO or single sample), byte-order/sign decoding, unit
 * conversion, bounded retries, error accounting and bus/device recovery.
 *
 * NOT responsible for: calibration, filtering, motion/terrain logic, RTOS.
 * The LSM303DLHC has NO gyroscope; this driver exposes no angular-rate data.
 */

#ifndef IMU_LSM303DLHC_H
#define IMU_LSM303DLHC_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_hal.h"
#include "imu/imu_types.h"

/** Driver configuration (a subset of imu_params_t). */
typedef struct
{
    uint8_t  accel_odr_code;
    uint8_t  accel_fs_code;
    bool     b_accel_fifo;
    uint8_t  mag_odr_code;
    uint8_t  mag_gain_code;
    bool     b_mag_id_check;
    uint32_t i2c_baud_hz;
    uint32_t i2c_timeout_us;
    uint8_t  i2c_max_retries;
} lsm303_config_t;

/** Driver error / health counters (monotonic, wrap at 2^32). */
typedef struct
{
    uint32_t transfers;
    uint32_t failed_transfers;     /**< after all retries          */
    uint32_t retries;              /**< individual retry attempts  */
    uint32_t nacks;
    uint32_t timeouts;
    uint32_t fifo_overruns;
    uint32_t accel_overruns;       /**< STATUS_REG_A.ZYXOR         */
    uint32_t mag_overflows;
    uint32_t recoveries;
    imu_hal_status_t last_status;
} lsm303_stats_t;

/** Driver instance. */
typedef struct
{
    lsm303_config_t cfg;
    lsm303_stats_t  stats;
    float           accel_g_per_lsb;    /* after the >>4 justification */
    float           mag_xy_gauss_per_lsb;
    float           mag_z_gauss_per_lsb;
    float           accel_full_scale_g;
    bool            b_ready;
} lsm303_t;

/**
 * @brief  Initialise the I2C bus, verify identity and configure both
 *         sensors. Configuration registers are read back and compared.
 * @return IMU_OK, IMU_ERR_PARAM, IMU_ERR_I2C, IMU_ERR_TIMEOUT, IMU_ERR_ID,
 *         IMU_ERR_VERIFY.
 */
imu_err_t lsm303_init(lsm303_t * p_dev, lsm303_config_t const * p_cfg);

/**
 * @brief  Read all available accelerometer samples.
 *
 * FIFO mode: reads FIFO_SRC_REG_A and then every stored sample (max
 * @p max_samples); oldest first. Single mode: checks STATUS_REG_A.ZYXDA and
 * reads one sample.
 *
 * @param[out] p_samples    raw samples (12-bit, already right-justified)
 * @param[out] p_count      number of samples written (0 if none new)
 * @param[out] p_b_overrun  true if samples were lost (FIFO/ZYXOR overrun)
 * @return IMU_OK (count may be 0), IMU_ERR_I2C, IMU_ERR_TIMEOUT, ...
 */
imu_err_t lsm303_read_accel(lsm303_t * p_dev, imu_raw3_t * p_samples,
                            uint8_t max_samples, uint8_t * p_count,
                            bool * p_b_overrun);

/**
 * @brief  Read one magnetometer sample (big-endian X, Z, Y register order
 *         is reordered to X, Y, Z here).
 * @param[out] p_b_overflow  true if any axis reported ADC overflow (-4096)
 */
imu_err_t lsm303_read_mag(lsm303_t * p_dev, imu_raw3_t * p_raw,
                          bool * p_b_overflow);

/** @brief  Raw accelerometer counts -> g (sensor frame). */
void lsm303_accel_to_g(lsm303_t const * p_dev, imu_raw3_t const * p_raw,
                       imu_vec3_t * p_out);

/** @brief  Raw magnetometer counts -> gauss (sensor frame). */
void lsm303_mag_to_gauss(lsm303_t const * p_dev, imu_raw3_t const * p_raw,
                         imu_vec3_t * p_out);

/**
 * @brief  Recovery: bus-clear, re-initialise the peripheral, re-verify and
 *         re-configure the device. Bounded in time.
 */
imu_err_t lsm303_recover(lsm303_t * p_dev);

/** @brief  Put the magnetometer to sleep and release the bus. */
void lsm303_shutdown(lsm303_t * p_dev);

/* ---- Pure helpers, exported for unit testing ------------------------- */

/** Little-endian, left-justified 12-bit accelerometer word -> counts. */
int16_t lsm303_decode_accel_axis(uint8_t lo, uint8_t hi);

/** Big-endian 16-bit magnetometer word -> counts. */
int16_t lsm303_decode_mag_axis(uint8_t hi, uint8_t lo);

/** @return mg per LSB in high-resolution mode, 0 for an invalid code. */
float lsm303_accel_sensitivity_g(uint8_t fs_code);

/** @return LSB/gauss for X/Y (b_z = false) or Z, 0 for an invalid code. */
float lsm303_mag_lsb_per_gauss(uint8_t gain_code, bool b_z);

/** @return imu_err_t equivalent of a HAL status. */
imu_err_t lsm303_map_hal(imu_hal_status_t status);

#endif /* IMU_LSM303DLHC_H */
