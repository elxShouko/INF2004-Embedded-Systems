/** @file drv_imu.c
 * Application driver, not a kernel device driver. SDK calls stay in owner.
 * Board pins follow Week 6 design, not a claim of verified physical wiring.
 */
#include "drv_imu.h"
#include "imu_config.h"
#include "config_device.h"

#if S4_IMU_ENABLE && DEVCNF_USE_IIC
#error "Buddy 4 I2C0 must not share ownership with the kernel I2C driver"
#endif

#if defined(TM_WIFI_CYW43)
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/structs/timer.h"

static bool bus_read (uint8_t address, uint8_t reg,
                      uint8_t *p_data, uint32_t length);
static bool bus_write (uint8_t address, uint8_t reg, uint8_t value);

static const drv_lsm_bus_t g_bus = {bus_read, bus_write};

bool drv_imu_open (void)
{
    if (0 == S4_IMU_ENABLE)
    {
        return false;
    }
    /* The shared template fixes clk_sys to 125 MHz. Its SDK clock hook
     * must be updated if the team changes that clock (see README). */
    (void)i2c_init(i2c0, S4_IMU_BAUD_HZ);
    gpio_set_function(S4_IMU_SDA, GPIO_FUNC_I2C);
    gpio_set_function(S4_IMU_SCL, GPIO_FUNC_I2C);
    /* External 3.3 V pull-ups must be verified; do not rely on weak pulls. */
    return drv_lsm_init(&g_bus);
}

drv_lsm_result_t drv_imu_read (imu_core_sample_t *p_sample)
{
    return drv_lsm_read(&g_bus, p_sample);
}

uint32_t drv_imu_time_ms (void)
{
    /* SDK timer register access, independent of Wi-Fi's time helper. */
    uint32_t high_before = 0u;
    uint32_t high_after = 0u;
    uint32_t low = 0u;
    uint64_t elapsed_us = 0u;

    do
    {
        high_before = timer_hw->timerawh;
        low = timer_hw->timerawl;
        high_after = timer_hw->timerawh;
    } while (high_before != high_after);
    elapsed_us = ((uint64_t)high_before << 32u) | (uint64_t)low;
    /* Intentional modulo-2^32 timestamp; intervals bounded below 2^31. */
    return (uint32_t)(elapsed_us / 1000u);
}

static bool bus_read (uint8_t address, uint8_t reg,
                      uint8_t *p_data, uint32_t length)
{
    const absolute_time_t deadline =
        make_timeout_time_us(S4_IMU_BUS_TIMEOUT_US);
    int transferred = 0; /* SDK transfer result is int by contract. */

    transferred = i2c_write_blocking_until(i2c0, address, &reg, 1u,
                                          true, deadline);
    if (1 != transferred)
    {
        return false;
    }
    transferred = i2c_read_blocking_until(i2c0, address, p_data, length,
                                         false, deadline);
    /* Length is bounded to six bytes by drv_lsm. */
    return (int)length == transferred;
}

static bool bus_write (uint8_t address, uint8_t reg, uint8_t value)
{
    const uint8_t data[2] = {reg, value};
    const int transferred = i2c_write_timeout_us(i2c0, address, data,
                                                sizeof(data), false,
                                                S4_IMU_BUS_TIMEOUT_US);

    return 2 == transferred;
}

#else

#if S4_IMU_ENABLE
#error "S4 IMU transport requires the template WIFI=cyw43 SDK hooks"
#endif

bool drv_imu_open (void)
{
    return false;
}

drv_lsm_result_t drv_imu_read (imu_core_sample_t *p_sample)
{
    (void)p_sample;
    return DRV_LSM_IO_ERROR;
}

uint32_t drv_imu_time_ms (void)
{
    return 0u;
}

#endif /* TM_WIFI_CYW43 */

/* End of file. */

