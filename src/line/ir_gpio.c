#include "line/ir_gpio.h"

#include <stddef.h>
#include "pico/stdlib.h"

static const uint32_t g_ir_pins[IR_CHANNEL_COUNT] =
{
    IR_LINE_1_GPIO,
    IR_LINE_2_GPIO,
    IR_BARCODE_GPIO
};

void ir_gpio_init(void)
{
    for (uint32_t channel = 0U; channel < IR_CHANNEL_COUNT; ++channel)
    {
        gpio_init(g_ir_pins[channel]);
        gpio_set_dir(g_ir_pins[channel], GPIO_IN);
        /* Observe driven DOUT without assuming pull-up/down requirements. */
        gpio_disable_pulls(g_ir_pins[channel]);
    }
}

bool ir_gpio_read(ir_sample_t *p_sample)
{
    if (p_sample == NULL)
    {
        return false;
    }
    /* Read one GPIO snapshot so channels describe the same observation. */
    const uint32_t inputs = gpio_get_all();
    p_sample->timestamp_us = time_us_64();
    p_sample->levels = 0U;
    for (uint32_t channel = 0U; channel < IR_CHANNEL_COUNT; ++channel)
    {
        if ((inputs & (1UL << g_ir_pins[channel])) != 0U)
        {
            p_sample->levels |= (uint8_t)(1U << channel);
        }
    }
    return true;
}
