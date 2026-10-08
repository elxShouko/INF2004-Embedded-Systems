#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>
#include <stdint.h>

bool servo_init(uint32_t gpio_pin);

bool servo_set_angle(uint8_t angle_deg);

uint8_t servo_get_angle(void);

bool servo_is_ready(void);

#endif