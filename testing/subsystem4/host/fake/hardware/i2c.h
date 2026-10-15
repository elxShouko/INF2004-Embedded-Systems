#ifndef S4_FAKE_HARDWARE_I2C_H
#define S4_FAKE_HARDWARE_I2C_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct
{
    uint32_t marker;
} i2c_inst_t;
typedef uint64_t absolute_time_t;
extern i2c_inst_t * i2c0;
uint32_t i2c_init (i2c_inst_t * p_i2c, uint32_t baud_hz);
absolute_time_t make_timeout_time_us (uint64_t timeout_us);
int i2c_write_blocking_until (i2c_inst_t * p_i2c, uint8_t address,
                              const uint8_t * p_src, size_t length,
                              bool b_no_stop, absolute_time_t deadline);
int i2c_read_blocking_until (i2c_inst_t * p_i2c, uint8_t address,
                             uint8_t * p_dst, size_t length,
                             bool b_no_stop, absolute_time_t deadline);
int i2c_write_timeout_us (i2c_inst_t * p_i2c, uint8_t address,
                          const uint8_t * p_src, size_t length,
                          bool b_no_stop, uint32_t timeout_us);
#endif
