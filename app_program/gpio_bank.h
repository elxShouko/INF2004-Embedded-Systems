/** @file gpio_bank.h @brief Shared same-core GPIO IRQ subscriptions. */
#ifndef GPIO_BANK_H
#define GPIO_BANK_H
#include <stdbool.h>
#include <stdint.h>
#include "subsystem2/drv_encoder.h"
#define GPIO_BANK_CLIENTS (2U)
#define GPIO_BANK_BASE (0x40014000UL)
#define GPIO_BANK_INTR (GPIO_BANK_BASE + 0xF0UL)
#define GPIO_BANK_INTE0 (GPIO_BANK_BASE + 0x100UL)
#define GPIO_BANK_INTE1 (GPIO_BANK_BASE + 0x130UL)
#define GPIO_BANK_INTF0 (GPIO_BANK_BASE + 0x110UL)
#define GPIO_BANK_INTF1 (GPIO_BANK_BASE + 0x140UL)
#define GPIO_BANK_INTS1 (GPIO_BANK_BASE + 0x150UL)
#define GPIO_BANK_INPUT (0xD0000004UL)

typedef struct
{
    uint32_t (*p_read)(uint32_t address);
    void (*p_write)(uint32_t address, uint32_t value);
    uint32_t (*p_core)(void);
    bool (*p_enter)(uint32_t * p_saved);
    void (*p_leave)(uint32_t saved);
    bool (*p_install)(void);
    void (*p_fault)(void);
} gpio_bank_io_t;

typedef struct
{
    uint32_t mask;
    drv_encoder_sink_t p_sink;
    void * p_context;
    bool b_enabled;
} gpio_bank_client_t;

typedef struct
{
    gpio_bank_io_t io;
    gpio_bank_client_t clients[GPIO_BANK_CLIENTS];
    uint32_t allowed_mask;
    bool b_initialised;
    bool b_installed;
} gpio_bank_t;

/* Init is pre-start only, no MMIO. Other operations require core 1. Hooks and
 * callbacks must be bounded, non-reentrant and have static lifetime. Same-core
 * IRQ exclusion covers reservation through callback completion; cross-core
 * calls fail. Clients must not bypass this owner with SDK exclusive handlers.
 */
bool gpio_bank_init (gpio_bank_t * p_bank, gpio_bank_io_t const * p_io,
    uint32_t allowed_mask);
bool gpio_bank_reserve (void * p_bank, uint32_t mask,
    drv_encoder_sink_t p_sink, void * p_context);
bool gpio_bank_enable (void * p_bank, uint32_t mask);
bool gpio_bank_release (gpio_bank_t * p_bank, uint32_t mask);
void gpio_bank_irq (gpio_bank_t * p_bank);
#endif /* GPIO_BANK_H */
/*** end of file ***/
