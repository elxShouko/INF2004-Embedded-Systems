/** @file drv_ir_rtos.c
 * @brief Member 3 drv ir rtos module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#include "drv_ir_rtos.h"

/* RP2040 addresses match the template BSP and SDK hardware definitions.
 * Reading raw timer needs no Pico SDK runtime or radio dependency.
 */
#define S3_SIO_GPIO_IN (*(volatile const uint32_t *)0xd0000004UL)
#define S3_TIMER_HIGH (*(volatile const uint32_t *)0x40054024UL)
#define S3_TIMER_LOW (*(volatile const uint32_t *)0x40054028UL)

/**
 * @brief Read a rollover-consistent RP2040 microsecond timestamp.
 * @note Widening the raw 32-bit timer words to uint64_t preserves all bits.
 */
uint64_t s3_clock_us (void)
{
    uint32_t high_before = 0U;
    uint32_t low         = 0U;
    uint32_t high_after  = 0U;

    do
    {
        high_before = S3_TIMER_HIGH;
        low         = S3_TIMER_LOW;
        high_after  = S3_TIMER_HIGH;
    } while (high_before != high_after);

    return ((uint64_t)high_before << 32U) | low;
}

/**
 * @brief Configure only the three owned IR GPIOs as digital inputs.
 * @note The RP2040 pad addresses are aligned 32-bit MMIO locations; uintptr_t
 * preserves their address bits.
 */
ER s3_ir_init (void)
{
    static const UINT pins[]    = {2U, 4U, 27U};
    const uint32_t    pin_count = sizeof(pins) / sizeof(pins[0]);

    for (uint32_t element_index = 0U; element_index < pin_count;
         ++element_index)
    {
        ER result = gpio_set_pin(pins[element_index], GPIO_MODE_IN);

        if (result < E_OK)
        {
            return result;
        }

        /* Disable both internal pulls on this owner's sensor inputs only. */
        volatile uint32_t * p_pad =
            (volatile uint32_t *)(uintptr_t)(0x4001c004UL +
                                             4U * pins[element_index]);
        *p_pad &= ~0x0cUL;
    }

    return E_OK;
}

/**
 * @brief Snapshot the owned GPIO levels and their observation timestamp.
 * @note The GPIO fields are masked to one bit each, so the combined result fits
 * uint8_t.
 */
void s3_ir_read (ir_sample_t * p_sample)
{
    const uint32_t gpio    = S3_SIO_GPIO_IN;
    p_sample->timestamp_us = s3_clock_us();
    p_sample->levels =
        (uint8_t)(((gpio >> 2U) & 1U) | (((gpio >> 4U) & 1U) << 1U) |
                  (((gpio >> 27U) & 1U) << 2U));
}

/*** End of file. ***/

