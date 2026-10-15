/**
 * @file    imu_hal.h
 * @brief   Buddy 4 hardware-abstraction interface (I2C bus + time base).
 *
 * The LSM303DLHC driver only talks to the hardware through these functions.
 * Historical implementations:
 *   - src/imu/imu_hal_pico.c : Raspberry Pi Pico SDK (hardware_i2c), target
 *   - former tests/imu/sim_lsm303.c : host simulator (retired; see Git history)
 * The implementation is selected at link time (no function pointers in the
 * hot path, no RTOS dependency).
 *
 * All functions are bounded in time: no call may block longer than the
 * timeout it is given (bus recovery: < 200 us).
 */

#ifndef IMU_HAL_H
#define IMU_HAL_H

#include <stddef.h>
#include <stdint.h>

/** Result of a HAL bus operation. */
typedef enum
{
    IMU_HAL_OK = 0,
    IMU_HAL_NACK,        /**< address or data not acknowledged */
    IMU_HAL_TIMEOUT,     /**< transfer did not finish in time  */
    IMU_HAL_ERROR        /**< any other bus / parameter error  */
} imu_hal_status_t;

/**
 * @brief  Initialise the I2C peripheral and the SDA/SCL pins.
 * @param  baud_hz  requested SCL frequency.
 */
imu_hal_status_t imu_hal_i2c_init(uint32_t baud_hz);

/** @brief Release the I2C peripheral and return the pins to GPIO inputs. */
void imu_hal_i2c_deinit(void);

/**
 * @brief  Write @p len bytes to 7-bit slave @p addr7 (STOP at the end).
 */
imu_hal_status_t imu_hal_i2c_write(uint8_t addr7, uint8_t const * p_data,
                                   size_t len, uint32_t timeout_us);

/**
 * @brief  Write @p tx_len bytes (no STOP, repeated START) then read
 *         @p rx_len bytes.
 */
imu_hal_status_t imu_hal_i2c_write_read(uint8_t addr7,
                                        uint8_t const * p_tx, size_t tx_len,
                                        uint8_t * p_rx, size_t rx_len,
                                        uint32_t timeout_us);

/**
 * @brief  I2C bus-clear procedure (NXP UM10204 section 3.1.16): with the
 *         peripheral detached, clock SCL up to 9 times until a slave that
 *         holds SDA low releases it, then generate a STOP.
 */
void imu_hal_i2c_bus_recover(void);

/** @return milliseconds since boot (wraps at 2^32). */
uint32_t imu_hal_time_ms(void);

/** @return microseconds since boot, low 32 bits (for execution timing). */
uint32_t imu_hal_time_us(void);

#endif /* IMU_HAL_H */
