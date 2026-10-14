/**
 * @file drv_rp2040.c
 * @brief Robo Pico motor outputs using professor BSP and localized MMIO.
 *
 * Sources: reference_docs/hardware/Robo Pico Board Datasheet.pdf, Tables 2/3;
 * RP2040 datasheet sections 2.19 and 4.5; professor libbsp_pwm.c/sysdef.h.
 * Local MMIO supplies operations absent from BSP: paired CC, divider/counter,
 * reset readiness and atomic pair SIO writes. No SDK runtime dependency.
 */
#include "drv_rp2040.h"

#if defined(_PICO_RP2040_) && !defined(MOTION_CONFIG_HOST_TEST)
#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include <tk/tkernel.h>
#include <tk/syslib.h>
#include <bsp/libbsp.h>
#include <config_device.h>

#if (MOTION_CONFIG_LEFT_MOTOR_A_GPIO != 8U) || \
    (MOTION_CONFIG_LEFT_MOTOR_B_GPIO != 9U) || \
    (MOTION_CONFIG_RIGHT_MOTOR_A_GPIO != 10U) || \
    (MOTION_CONFIG_RIGHT_MOTOR_B_GPIO != 11U)
#error "Review Robo Pico motor pin/slice mapping before changing assignments"
#endif

#define DRV_RP2040_MASK (0x00000F00U)
#define DRV_RP2040_SLICES (0x30U)
#define DRV_RP2040_POLLS (32U)
/* Professor GPIO_OUT is SIO+0x008 (QSPI input). RP2040 Table 16 and SDK
 * SIO_GPIO_OUT_OFFSET verify +0x010. Keep this correction LOCAL to Buddy 2.
 */
#define DRV_RP2040_OUT (SIO_BASE + 0x010U)

static bool drv_rp2040_clock (uint32_t clock_hz);
static bool drv_rp2040_free (void);
static bool drv_rp2040_prepare (void * p_context, uint16_t period,
                                     uint32_t clock_hz);
static bool drv_rp2040_apply (void * p_context,
    drv_motor_wheel_t wheel, uint16_t level_a, uint16_t level_b);
static bool drv_rp2040_stop (void * p_context);
static bool drv_rp2040_brake (drv_rp2040_t * p_backend,
                                   drv_motor_wheel_t wheel);

bool drv_rp2040_bind (drv_rp2040_t * p_backend,
                            drv_motor_io_t * p_io)
{
    bool b_valid = (NULL != p_backend) && (NULL != p_io);

    if (b_valid)
    {
        b_valid = !p_backend->b_claimed;
        if (b_valid)
        {
            *p_io = (drv_motor_io_t){p_backend,
                drv_rp2040_prepare, drv_rp2040_apply,
                drv_rp2040_stop};
        }
    }

    return (b_valid);
}

/** @brief Verify the template's 12 MHz XOSC -> 125 MHz PLL_SYS clock tree. */
static bool drv_rp2040_clock (uint32_t clock_hz)
{
    return ((125000000U == clock_hz) &&
        (0U != (in_w(XOSC_STATUS) & 0x80000000U)) &&
        (0x80000001U == (in_w(PLL_SYS_BASE + PLL_CS) & 0x8000003FU)) &&
        (125U == (in_w(PLL_SYS_BASE + PLL_FBDIV_INT) & 0xFFFU)) &&
        (0x62000U == (in_w(PLL_SYS_BASE + PLL_PRIM) & 0x77000U)) &&
        (1U == (in_w(CLK_SYS + CLK_x_CTRL) & 0xE1U)) &&
        (2U == in_w(CLK_SYS + CLK_x_SELECTED)) &&
        (256U == in_w(CLK_SYS + CLK_x_DIV)));
}

/** @brief Reject live slices, timer IRQ use and every mirrored PWM mapping. */
static bool drv_rp2040_free (void)
{
    bool b_free = (0 == DEVCNF_USE_IIC) && (0 == USE_PTMR) &&
        (0U == (in_w(GPIO_OE) & DRV_RP2040_MASK)) &&
        (0U == (in_w(PWM_CH4_BASE + PWM_CHx_CSR) & 1U)) &&
        (0U == (in_w(PWM_CH5_BASE + PWM_CHx_CSR) & 1U)) &&
        (0U == (in_w(PWM_INTE) & DRV_RP2040_SLICES));

    for (uint32_t pin = 0U; pin < 30U; ++pin)
    {
        uint32_t control = in_w(GPIO_CTRL(pin));
        uint32_t function = control & 0x1FU;
        uint32_t slice = (pin >> 1U) & 7U;

        if (((4U == slice) || (5U == slice)) && (4U == function))
        {
            b_free = false;
        }
        if ((pin >= 8U) && (pin <= 11U) &&
            (((5U != function) && (31U != function)) ||
             (0U != (control & 0x3300U))))
        {
            b_free = false;
        }
    }

    return (b_free);
}

/** @brief Claim motors and release PWM reset; never reset live slices. */
static bool drv_rp2040_prepare (void * p_context, uint16_t period,
                                     uint32_t clock_hz)
{
    drv_rp2040_t * p_backend = p_context;
    bool b_ok = (NULL != p_backend) && (period >= 2U) &&
        (drv_rp2040_clock(clock_hz));

    if (b_ok && p_backend->b_claimed)
    {
        b_ok = (period == p_backend->period) &&
               (drv_rp2040_stop(p_context));
    }
    else if (b_ok && (drv_rp2040_free()))
    {
        clr_w(RESETS_RESET, RESETS_RESET_PWM);
        b_ok = false;
        /* Bounded readiness polling, not a calibrated delay or sleep. */
        for (uint32_t index = 0U; index < DRV_RP2040_POLLS; ++index)
        {
            if (0U != (in_w(RESETS_RESET_DONE) & RESETS_RESET_PWM))
            {
                b_ok = true;
                break;
            }
        }
        if (b_ok)
        {
            p_backend->period = period;
            p_backend->b_claimed = true;
            out_w(GPIO_OUT_CLR, DRV_RP2040_MASK);
            /* Preflight requires all four pins to be inputs/SIO or NULL.
             * Preload LOW before enabling any output or changing its mux.
             */
            bool b_left = drv_rp2040_brake(p_backend, DRV_MOTOR_LEFT);
            bool b_right = drv_rp2040_brake(p_backend, DRV_MOTOR_RIGHT);

            b_ok = b_left && b_right;
            for (uint32_t slice = 4U; slice <= 5U; ++slice)
            {
                uint32_t base = PWM_BASE + (slice * 0x14U);

                out_w(base + PWM_CHx_DIV, 16U); /* Divider = 1.0 (8:4). */
                out_w(base + PWM_CHx_CTR, 0U);
                out_w(base + PWM_CHx_TOP, (uint32_t)period - 1U);
                out_w(base + PWM_CHx_CC, 0U);
            }
        }
    }
    else
    {
        b_ok = false; /* Refuse ownership without touching foreign outputs. */
    }

    return (b_ok);
}

/** @brief Force pair LOW through SIO before disabling its PWM slice. */
static bool drv_rp2040_brake (drv_rp2040_t * p_backend,
                                   drv_motor_wheel_t wheel)
{
    uint32_t pin_a = 8U + ((uint32_t)wheel * 2U);
    uint32_t base = PWM_BASE + ((pin_a >> 1U) * 0x14U);
    bool b_ok;

    /* Atomic pair CLR also makes COAST -> BRAKE simultaneous. When driving,
     * only one pin is PWM; remuxing it LOW cannot create opposite drive.
     */
    out_w(GPIO_OUT_CLR, 3U << pin_a);
    ER error_a = gpio_set_pin(pin_a, GPIO_MODE_OUT);
    ER error_b = gpio_set_pin(pin_a + 1U, GPIO_MODE_OUT);

    out_w(base + PWM_CHx_CSR, 0U);
    out_w(base + PWM_CHx_CC, 0U);
    out_w(base + PWM_CHx_CTR, 0U);
    p_backend->direction[wheel] = 0;
    b_ok = (E_OK == error_a) && (E_OK == error_b) &&
        (0U == (in_w(DRV_RP2040_OUT) & (3U << pin_a))) &&
        ((3U << pin_a) == (in_w(GPIO_OE) & (3U << pin_a))) &&
        (5U == (in_w(GPIO_CTRL(pin_a)) & 0x1FU)) &&
        (5U == (in_w(GPIO_CTRL(pin_a + 1U)) & 0x1FU));

    return (b_ok);
}

/** @brief Apply paired PWM levels, or static board-specific stop inputs. */
static bool drv_rp2040_apply (void * p_context,
    drv_motor_wheel_t wheel, uint16_t level_a, uint16_t level_b)
{
    drv_rp2040_t * p_backend = p_context;
    bool b_ok = (NULL != p_backend) &&
        ((DRV_MOTOR_LEFT == wheel) || (DRV_MOTOR_RIGHT == wheel));

    if (b_ok)
    {
        b_ok = p_backend->b_claimed && (level_a <= p_backend->period) &&
            (level_b <= p_backend->period) &&
            ((0U == level_a) || (0U == level_b) ||
             ((level_a == p_backend->period) &&
              (level_b == p_backend->period)));
    }
    if (b_ok)
    {
        uint32_t pin_a = 8U + ((uint32_t)wheel * 2U);
        uint32_t base = PWM_BASE + ((pin_a >> 1U) * 0x14U);
        int8_t direction = (0U == level_b) ? 1 : -1;
        bool b_stop = ((0U == level_a) && (0U == level_b)) ||
                     ((0U != level_a) && (0U != level_b));

        if (b_stop || (direction != p_backend->direction[wheel]))
        {
            b_ok = drv_rp2040_brake(p_backend, wheel);
        }
        if (b_ok && b_stop)
        {
            if (0U != level_a)
            {
                /* Both pins are now SIO LOW. Raise together: HIGH/HIGH coast.
                 * Disabled PWM alone would freeze its last level: not a stop.
                 */
                out_w(GPIO_OUT_SET, 3U << pin_a);
                b_ok = ((3U << pin_a) ==
                        (in_w(DRV_RP2040_OUT) & (3U << pin_a)));
            }
        }
        else if (b_ok)
        {
            /* One 32-bit write updates BOTH double-buffered compares. */
            out_w(base + PWM_CHx_CC,
                  (uint32_t)level_a | ((uint32_t)level_b << 16U));
            if (0 == p_backend->direction[wheel])
            {
                uint32_t active_pin = (direction > 0) ? pin_a : pin_a + 1U;

                b_ok = (E_OK == pwm_set_pin(active_pin));
                if (b_ok)
                {
                    out_w(base + PWM_CHx_CSR, 1U);
                }
            }
            if (b_ok)
            {
                p_backend->direction[wheel] = direction;
            }
        }
        else
        {
            /* Caller latches fault and attempts both-wheel brake. */
        }
    }

    return (b_ok);
}

/** @brief Attempt BOTH motor stops without short-circuiting on a failure. */
static bool drv_rp2040_stop (void * p_context)
{
    drv_rp2040_t * p_backend = p_context;
    bool b_ok = (NULL != p_backend) && p_backend->b_claimed;

    if (b_ok)
    {
        bool b_left = drv_rp2040_brake(p_backend, DRV_MOTOR_LEFT);
        bool b_right = drv_rp2040_brake(p_backend, DRV_MOTOR_RIGHT);

        b_ok = b_left && b_right;
    }

    return (b_ok);
}

#endif /* Actual RP2040 firmware only; no host hardware path. */

/*** end of file ***/

