/** @file transport_main.c
 * Host SDK double checks bounded calls; not physical stuck-bus evidence.
 */
#include "../../../app_program/subsystem4/drv_imu.h"
#include "../../../app_program/subsystem4/imu_config.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/structs/timer.h"

#include <stdio.h>
#include <stdlib.h>

static i2c_inst_t g_i2c = {0};
static s4_fake_timer_t g_timer = {0};
i2c_inst_t * i2c0 = &g_i2c;
s4_fake_timer_t * timer_hw = &g_timer;
static uint32_t g_deadlines;
static uint32_t g_block_writes;
static uint32_t g_block_reads;
static uint32_t g_timeout_writes;
static bool g_b_bad_args;
static bool g_b_short_read;

uint32_t i2c_init (i2c_inst_t * p_i2c, uint32_t baud_hz)
{
    g_b_bad_args |= (p_i2c != i2c0) ||
                    (baud_hz != S4_IMU_BAUD_HZ);
    return baud_hz;
}

void gpio_set_function (uint32_t pin, uint32_t function)
{
    g_b_bad_args |= ((pin != S4_IMU_SDA) && (pin != S4_IMU_SCL)) ||
                    (function != GPIO_FUNC_I2C);
}

absolute_time_t make_timeout_time_us (uint64_t timeout_us)
{
    g_b_bad_args |= (timeout_us != S4_IMU_BUS_TIMEOUT_US);
    ++g_deadlines;
    return 1000u + timeout_us;
}

int i2c_write_blocking_until (i2c_inst_t * p_i2c, uint8_t address,
                              const uint8_t * p_src, size_t length,
                              bool b_no_stop, absolute_time_t deadline)
{
    (void)address;
    g_b_bad_args |= (p_i2c != i2c0) || (NULL == p_src) ||
                    (1u != length) || (!b_no_stop) ||
                    (deadline != (1000u + S4_IMU_BUS_TIMEOUT_US));
    ++g_block_writes;
    return 1;
}

int i2c_read_blocking_until (i2c_inst_t * p_i2c, uint8_t address,
                             uint8_t * p_dst, size_t length,
                             bool b_no_stop, absolute_time_t deadline)
{
    g_b_bad_args |= (p_i2c != i2c0) || (NULL == p_dst) || b_no_stop ||
                    (deadline != (1000u + S4_IMU_BUS_TIMEOUT_US));
    ++g_block_reads;
    if (g_b_short_read)
    {
        return 0;
    }
    if ((DRV_LSM_MAG_ADDR == address) && (3u == length))
    {
        p_dst[0] = 0x48u;
        p_dst[1] = 0x34u;
        p_dst[2] = 0x33u;
    }
    return (int)length;
}

int i2c_write_timeout_us (i2c_inst_t * p_i2c, uint8_t address,
                          const uint8_t * p_src, size_t length,
                          bool b_no_stop, uint32_t timeout_us)
{
    (void)address;
    g_b_bad_args |= (p_i2c != i2c0) || (NULL == p_src) ||
                    (2u != length) || b_no_stop ||
                    (timeout_us != S4_IMU_BUS_TIMEOUT_US);
    ++g_timeout_writes;
    return -1; /* Simulated SDK timeout. */
}

int main (int argc, char * argv[])
{
    const unsigned long number = (2 == argc) ?
                                 strtoul(argv[1], NULL, 10) : 0u;
    imu_core_sample_t sample = {0};
    bool b_pass = false;

    if (202u == number)
    {
        g_b_short_read = true;
        b_pass = (DRV_LSM_IO_ERROR == drv_imu_read(&sample)) &&
                 (1u == g_deadlines) && (1u == g_block_writes) &&
                 (1u == g_block_reads);
    }
    else if (203u == number)
    {
        b_pass = (!drv_imu_open()) && (1u == g_deadlines) &&
                 (1u == g_block_writes) && (1u == g_block_reads) &&
                 (1u == g_timeout_writes);
    }
    b_pass = b_pass && (!g_b_bad_args);
    (void)printf("[TEST] S4-U%lu %s bounded I2C transport\n", number,
                 b_pass ? "PASS" : "FAIL");
    return b_pass ? 0 : 1;
}
