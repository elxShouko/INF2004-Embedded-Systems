/**
 * @file    imu_hal_pico.c
 * @brief   Buddy 4 HAL implementation on the Raspberry Pi Pico C SDK.
 *
 * Uses hardware_i2c on I2C0, GP16 (SDA) / GP17 (SCL) = Robo Pico Grove 4,
 * as allocated in Design Review Section 2. No other GPIO is touched.
 *
 * Timing notes (RTOS integration):
 *   - i2c_write_timeout_us()/i2c_read_timeout_us() poll the peripheral in the
 *     calling task. They are preemptible by higher-priority uT-Kernel tasks
 *     and bounded by the timeout argument, so they never block the Motion
 *     Control / Line tasks. A typical 6-byte read at 400 kHz takes ~200 us.
 *   - Bus recovery busy-waits at most 10 x 2 x 5 us = 100 us.
 *   - sleep_ms()/sleep_us() are deliberately NOT used: under uT-Kernel the
 *     task must delay with tk_dly_tsk()/tk_slp_tsk() instead.
 */

#include "imu/imu_hal.h"
#include "imu/imu_config.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/error.h"
#include "hardware/timer.h"
#include "pico/time.h"

#if (IMU_I2C_INSTANCE == 0U)
#define IMU_I2C_PORT        (i2c0)
#else
#define IMU_I2C_PORT        (i2c1)
#endif

#define HAL_RECOVER_CLOCKS     (9U)
#define HAL_RECOVER_HALF_US    (5U)    /* 100 kHz bit-banged clock */

static uint32_t g_hal_baud_hz = IMU_CFG_I2C_BAUD_HZ;

static imu_hal_status_t hal_map_result(int result, size_t expected)
{
    imu_hal_status_t status;

    if ((result >= 0) && ((size_t)result == expected))
    {
        status = IMU_HAL_OK;
    }
    else if (PICO_ERROR_TIMEOUT == result)
    {
        status = IMU_HAL_TIMEOUT;
    }
    else if (PICO_ERROR_GENERIC == result)
    {
        status = IMU_HAL_NACK;   /* SDK reports address/data NACK this way */
    }
    else
    {
        status = IMU_HAL_ERROR;
    }

    return (status);
}

imu_hal_status_t imu_hal_i2c_init(uint32_t baud_hz)
{
    uint actual;

    g_hal_baud_hz = baud_hz;
    actual = i2c_init(IMU_I2C_PORT, baud_hz);

    gpio_set_function(IMU_I2C_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(IMU_I2C_SCL_GPIO, GPIO_FUNC_I2C);
#if (IMU_I2C_INTERNAL_PULLUPS != 0)
    gpio_pull_up(IMU_I2C_SDA_GPIO);
    gpio_pull_up(IMU_I2C_SCL_GPIO);
#endif

    return ((0U == actual) ? IMU_HAL_ERROR : IMU_HAL_OK);
}

void imu_hal_i2c_deinit(void)
{
    i2c_deinit(IMU_I2C_PORT);
    gpio_set_function(IMU_I2C_SDA_GPIO, GPIO_FUNC_SIO);
    gpio_set_function(IMU_I2C_SCL_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(IMU_I2C_SDA_GPIO, GPIO_IN);
    gpio_set_dir(IMU_I2C_SCL_GPIO, GPIO_IN);
}

imu_hal_status_t imu_hal_i2c_write(uint8_t addr7, uint8_t const * p_data,
                                   size_t len, uint32_t timeout_us)
{
    int result;

    if ((NULL == p_data) || (0U == len))
    {
        return (IMU_HAL_ERROR);
    }

    result = i2c_write_timeout_us(IMU_I2C_PORT, addr7, p_data, len,
                                  false, timeout_us);

    return (hal_map_result(result, len));
}

imu_hal_status_t imu_hal_i2c_write_read(uint8_t addr7,
                                        uint8_t const * p_tx, size_t tx_len,
                                        uint8_t * p_rx, size_t rx_len,
                                        uint32_t timeout_us)
{
    int              result;
    imu_hal_status_t status;

    if ((NULL == p_tx) || (NULL == p_rx) || (0U == tx_len) || (0U == rx_len))
    {
        return (IMU_HAL_ERROR);
    }

    /* nostop = true -> repeated START before the read phase. */
    result = i2c_write_timeout_us(IMU_I2C_PORT, addr7, p_tx, tx_len,
                                  true, timeout_us);
    status = hal_map_result(result, tx_len);

    if (IMU_HAL_OK == status)
    {
        result = i2c_read_timeout_us(IMU_I2C_PORT, addr7, p_rx, rx_len,
                                     false, timeout_us);
        status = hal_map_result(result, rx_len);
    }

    return (status);
}

void imu_hal_i2c_bus_recover(void)
{
    uint32_t pulse;

    i2c_deinit(IMU_I2C_PORT);

    /* Release both lines (open-drain emulation: input = released). */
    gpio_init(IMU_I2C_SDA_GPIO);
    gpio_init(IMU_I2C_SCL_GPIO);
    gpio_pull_up(IMU_I2C_SDA_GPIO);
    gpio_pull_up(IMU_I2C_SCL_GPIO);
    gpio_set_dir(IMU_I2C_SDA_GPIO, GPIO_IN);
    gpio_set_dir(IMU_I2C_SCL_GPIO, GPIO_IN);
    gpio_put(IMU_I2C_SCL_GPIO, false);
    gpio_put(IMU_I2C_SDA_GPIO, false);
    busy_wait_us_32(HAL_RECOVER_HALF_US);

    for (pulse = 0U; pulse < HAL_RECOVER_CLOCKS; pulse++)
    {
        if (gpio_get(IMU_I2C_SDA_GPIO))
        {
            break;   /* slave released SDA */
        }
        gpio_set_dir(IMU_I2C_SCL_GPIO, GPIO_OUT);      /* SCL low  */
        busy_wait_us_32(HAL_RECOVER_HALF_US);
        gpio_set_dir(IMU_I2C_SCL_GPIO, GPIO_IN);       /* SCL high */
        busy_wait_us_32(HAL_RECOVER_HALF_US);
    }

    /* STOP condition: SDA low -> high while SCL is high. */
    gpio_set_dir(IMU_I2C_SDA_GPIO, GPIO_OUT);
    busy_wait_us_32(HAL_RECOVER_HALF_US);
    gpio_set_dir(IMU_I2C_SDA_GPIO, GPIO_IN);
    busy_wait_us_32(HAL_RECOVER_HALF_US);

    (void)imu_hal_i2c_init(g_hal_baud_hz);
}

uint32_t imu_hal_time_ms(void)
{
    return (to_ms_since_boot(get_absolute_time()));
}

uint32_t imu_hal_time_us(void)
{
    return (time_us_32());
}
