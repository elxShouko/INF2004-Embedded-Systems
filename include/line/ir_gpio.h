#ifndef IR_GPIO_H
#define IR_GPIO_H

#include "line/ir_input.h"

/* Digital DOUT pins from the Week 6 pin table. No motor GPIOs are used. */
#define IR_LINE_1_GPIO 2U
#define IR_LINE_2_GPIO 4U
#define IR_BARCODE_GPIO 27U

void ir_gpio_init(void);
bool ir_gpio_read(ir_sample_t *p_sample);

#endif
