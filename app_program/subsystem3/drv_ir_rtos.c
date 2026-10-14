#include "drv_ir_rtos.h"

/* RP2040 addresses match the template BSP and SDK hardware definitions.
 * Reading raw timer needs no Pico SDK runtime or radio dependency.
 */
#define S3_SIO_GPIO_IN (*(volatile const uint32_t *)0xd0000004UL)
#define S3_TIMER_HIGH (*(volatile const uint32_t *)0x40054024UL)
#define S3_TIMER_LOW (*(volatile const uint32_t *)0x40054028UL)

uint64_t s3_clock_us(void)
{
    uint32_t high_before, low, high_after;
    do
    {
        high_before = S3_TIMER_HIGH;
        low = S3_TIMER_LOW;
        high_after = S3_TIMER_HIGH;
    } while (high_before != high_after);
    return ((uint64_t)high_before << 32U) | low;
}

ER s3_ir_init(void)
{
    static const UINT pins[] = {2U, 4U, 27U};
    for (unsigned int i = 0U; i < 3U; ++i)
    {
        ER result = gpio_set_pin(pins[i], GPIO_MODE_IN);
        if (result < E_OK)
        {
            return result;
        }
        /* Disable both internal pulls on this owner's sensor inputs only. */
        volatile uint32_t *p_pad = (volatile uint32_t *)(uintptr_t)
            (0x4001c004UL + 4U * pins[i]);
        *p_pad &= ~0x0cUL;
    }
    return E_OK;
}

void s3_ir_read(ir_sample_t *p_sample)
{
    const uint32_t gpio = S3_SIO_GPIO_IN;
    p_sample->timestamp_us = s3_clock_us();
    p_sample->levels = (uint8_t)(((gpio >> 2U) & 1U) |
        (((gpio >> 4U) & 1U) << 1U) | (((gpio >> 27U) & 1U) << 2U));
}
