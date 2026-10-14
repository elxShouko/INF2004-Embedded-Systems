#ifndef DRV_IR_RTOS_H
#define DRV_IR_RTOS_H

#include "../app.h"
#include "line/ir_input.h"

ER s3_ir_init(void);
void s3_ir_read(ir_sample_t *p_sample);
uint64_t s3_clock_us(void);

#endif
