#ifndef S4_FAKE_HARDWARE_GPIO_H
#define S4_FAKE_HARDWARE_GPIO_H
#include <stdint.h>
#define GPIO_FUNC_I2C 1u
void gpio_set_function (uint32_t pin, uint32_t function);
#endif
