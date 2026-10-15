/** @file gpio_bank.c @brief Owned-bit acknowledgement; no pin mux changes. */
#include "gpio_bank.h"
#include <stddef.h>

static uint32_t gpio_bank_bits (uint32_t mask, uint32_t group);
static bool gpio_bank_enter (gpio_bank_t * p_bank, uint32_t * p_saved);

bool gpio_bank_init (gpio_bank_t * p_bank, gpio_bank_io_t const * p_io,
    uint32_t allowed_mask)
{
    bool b_ok = (NULL != p_bank) && (NULL != p_io) &&
        (0U != allowed_mask) && (0U == (allowed_mask & 0xC0000000UL));

    if (b_ok)
    {
        b_ok = (NULL != p_io->p_read) && (NULL != p_io->p_write) &&
            (NULL != p_io->p_core) && (NULL != p_io->p_enter) &&
            (NULL != p_io->p_leave) && (NULL != p_io->p_install) &&
            (NULL != p_io->p_fault);
        if (b_ok)
        {
            *p_bank = (gpio_bank_t){0};
            p_bank->io = *p_io;
            p_bank->allowed_mask = allowed_mask;
            p_bank->b_initialised = true;
        }
    }
    return (b_ok);
}

bool gpio_bank_reserve (void * p_context, uint32_t mask,
    drv_encoder_sink_t p_sink, void * p_sink_context)
{
    gpio_bank_t * p_bank = p_context;
    uint32_t saved;

    if ((NULL == p_sink) || (0U == mask) ||
        !gpio_bank_enter(p_bank, &saved))
    {
        return (false);
    }
    bool b_ok = (mask == (mask & p_bank->allowed_mask));
    uint32_t slot = GPIO_BANK_CLIENTS;

    for (uint32_t i = 0U; i < GPIO_BANK_CLIENTS; ++i)
    {
        b_ok = (0U == (mask & p_bank->clients[i].mask)) && b_ok;
        if (0U == p_bank->clients[i].mask)
        {
            slot = i;
        }
    }
    for (uint32_t group = 0U; group < 4U; ++group)
    {
        uint32_t bits = gpio_bank_bits(mask, group);
        uint32_t occupied = p_bank->io.p_read(GPIO_BANK_INTE0 + group * 4U) |
            p_bank->io.p_read(GPIO_BANK_INTE1 + group * 4U) |
            p_bank->io.p_read(GPIO_BANK_INTF0 + group * 4U) |
            p_bank->io.p_read(GPIO_BANK_INTF1 + group * 4U);

        b_ok = (0U == (occupied & bits)) && b_ok;
    }
    b_ok = (slot < GPIO_BANK_CLIENTS) && b_ok;
    if (b_ok && !p_bank->b_installed)
    {
        b_ok = p_bank->io.p_install();
        p_bank->b_installed = b_ok;
    }
    if (b_ok)
    {
        p_bank->clients[slot] = (gpio_bank_client_t){mask, p_sink,
            p_sink_context, false};
    }
    p_bank->io.p_leave(saved);
    return (b_ok);
}

bool gpio_bank_enable (void * p_context, uint32_t mask)
{
    gpio_bank_t * p_bank = p_context;
    uint32_t saved;

    if (!gpio_bank_enter(p_bank, &saved))
    {
        return (false);
    }
    bool b_ok = false;

    for (uint32_t i = 0U; i < GPIO_BANK_CLIENTS; ++i)
    {
        gpio_bank_client_t * p_client = &p_bank->clients[i];

        if ((mask == p_client->mask) && (0U != mask) &&
            !p_client->b_enabled)
        {
            for (uint32_t group = 0U; group < 4U; ++group)
            {
                uint32_t edges = gpio_bank_bits(mask, group) & 0xCCCCCCCCUL;

                /* Clear stale OWNED edges, then SET only owned enable bits. */
                p_bank->io.p_write(GPIO_BANK_INTR + group * 4U, edges);
                p_bank->io.p_write(GPIO_BANK_INTE1 + 0x2000U + group * 4U,
                    edges); /* RP2040 atomic SET alias, no foreign RMW. */
            }
            p_client->b_enabled = true;
            b_ok = true;
        }
    }
    p_bank->io.p_leave(saved);
    return (b_ok);
}

bool gpio_bank_release (gpio_bank_t * p_bank, uint32_t mask)
{
    uint32_t saved;

    if (!gpio_bank_enter(p_bank, &saved))
    {
        return (false);
    }
    bool b_ok = false;

    for (uint32_t i = 0U; i < GPIO_BANK_CLIENTS; ++i)
    {
        if ((mask == p_bank->clients[i].mask) && (0U != mask))
        {
            for (uint32_t group = 0U; group < 4U; ++group)
            {
                uint32_t edges = gpio_bank_bits(mask, group) & 0xCCCCCCCCUL;

                p_bank->io.p_write(GPIO_BANK_INTE1 + 0x3000U + group * 4U,
                    edges); /* Atomic CLEAR alias. */
                p_bank->io.p_write(GPIO_BANK_INTR + group * 4U, edges);
            }
            p_bank->clients[i] = (gpio_bank_client_t){0};
            b_ok = true; /* Same-core IRQ exclusion quiesced any callback. */
        }
    }
    p_bank->io.p_leave(saved);
    return (b_ok);
}

void gpio_bank_irq (gpio_bank_t * p_bank)
{
    uint32_t saved;

    if (!gpio_bank_enter(p_bank, &saved))
    {
        return;
    }
    uint32_t pending[4];
    uint32_t after[4];
    uint32_t owned_mask = 0U;

    for (uint32_t i = 0U; i < GPIO_BANK_CLIENTS; ++i)
    {
        if (p_bank->clients[i].b_enabled)
        {
            owned_mask |= p_bank->clients[i].mask;
        }
    }

    for (uint32_t group = 0U; group < 4U; ++group)
    {
        pending[group] = p_bank->io.p_read(GPIO_BANK_INTS1 + group * 4U);
    }
    /* Pending precedes levels. An edge arriving during the snapshot must not
     * be acknowledged against an older GPIO sample and silently disappear.
     */
    uint32_t levels = p_bank->io.p_read(GPIO_BANK_INPUT);

    for (uint32_t group = 0U; group < 4U; ++group)
    {
        after[group] = p_bank->io.p_read(GPIO_BANK_INTS1 + group * 4U);
        if (0U != ((pending[group] | after[group]) &
            ~(gpio_bank_bits(owned_mask, group) & 0xCCCCCCCCUL)))
        {
            p_bank->io.p_fault(); /* Owner disables this bank IRQ, not foreign pins. */
            p_bank->io.p_leave(saved);
            return;
        }
    }
    for (uint32_t i = 0U; i < GPIO_BANK_CLIENTS; ++i)
    {
        gpio_bank_client_t const * p_client = &p_bank->clients[i];
        uint32_t events = 0U;
        bool b_lost = false;

        if (p_client->b_enabled)
        {
            for (uint32_t group = 0U; group < 4U; ++group)
            {
                b_lost = b_lost || (0U != ((after[group] ^ pending[group]) &
                    gpio_bank_bits(p_client->mask, group) & 0xCCCCCCCCUL));
            }
            for (uint32_t pin = 0U; pin < 30U; ++pin)
            {
                uint32_t shift = (pin % 8U) * 4U;
                uint32_t edge = (pending[pin / 8U] >> shift) & 12U;

                if ((0U != (p_client->mask & (1UL << pin))) && (0U != edge))
                {
                    events |= 1UL << pin;
                    b_lost = b_lost || (12U == edge);
                    p_bank->io.p_write(GPIO_BANK_INTR + (pin / 8U) * 4U,
                        edge << shift);
                }
            }
            if (b_lost)
            {
                p_bank->io.p_fault();
                p_bank->io.p_leave(saved);
                return;
            }
            else if (0U != events)
            {
                p_client->p_sink(p_client->p_context, events, levels);
            }
        }
    }
    p_bank->io.p_leave(saved);
}

static uint32_t gpio_bank_bits (uint32_t mask, uint32_t group)
{
    uint32_t bits = 0U;

    for (uint32_t n = 0U; n < 8U; ++n)
    {
        uint32_t pin = group * 8U + n;

        if ((pin < 30U) && (0U != (mask & (1UL << pin))))
        {
            bits |= 15UL << (n * 4U);
        }
    }
    return (bits);
}

static bool gpio_bank_enter (gpio_bank_t * p_bank, uint32_t * p_saved)
{
    return ((NULL != p_bank) && p_bank->b_initialised &&
        (1U == p_bank->io.p_core()) && p_bank->io.p_enter(p_saved));
}
/*** end of file ***/
