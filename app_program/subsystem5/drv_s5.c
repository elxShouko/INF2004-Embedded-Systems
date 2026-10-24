/** @file drv_s5.c @brief Explicit, bounded RP2040 acquisition. */
#include "drv_s5.h"
#if !defined(VEHICLE_HOST_TEST)
#if defined(S5_DRIVER_HOST_TEST)
#include "test_s5_port.h"
#else
#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include <tk/tkernel.h>
#include <tk/syslib.h>
#include <bsp/libbsp.h>
#include "../../device/adc/sysdepend/rp2040/adc_cnf_sysdep.h"
#endif

#define S5_ECHO_MASK (1UL << 6U)
#define S5_TRIG_MASK (1UL << 26U)
#define S5_SERVO_MASK (1UL << 15U)
#define S5_OUT_CLR (0xD0000018UL)
#define S5_OUT_SET (0xD0000014UL)
#define S5_ECHO_IDLE (0U)
#define S5_ECHO_ARMED (1U)
#define S5_ECHO_RISE (2U)
#define S5_ECHO_COMPLETE (3U)
#define S5_ECHO_ERROR (4U)
/* The port's EI uses inline PRIMASK assembly without a memory clobber.
 * These compiler barriers keep the coherent copy inside IRQ exclusion.
 * GNU syntax is required by the professor's GNU Make toolchain.
 */
#define S5_COMPILER_BARRIER() __asm__ volatile ("" ::: "memory")

static bool drv_s5_open (void * p_context,
    s5_acquire_config_t const * p_config);
static bool drv_s5_servo (void * p_context, uint8_t angle);
static bool drv_s5_trigger (void * p_context);
static int32_t drv_s5_echo (void * p_context, uint32_t * p_width,
    uint64_t * p_capture);
static void drv_s5_close (void * p_context);
static void drv_s5_edge (void * p_context, uint32_t events, uint32_t levels);
static bool drv_s5_free (void);

bool drv_s5_now (uint64_t * p_now)
{
    if (NULL == p_now) { return (false); }
    for (uint32_t attempt = 0U; attempt < 2U; ++attempt)
    {
        uint32_t high = in_w(0x40054024UL);
        uint32_t low = in_w(0x40054028UL);

        if (high == in_w(0x40054024UL))
        {
            *p_now = ((uint64_t)high << 32U) | low;
            return (true);
        }
    }
    return (false);
}

bool drv_s5_bind (drv_s5_t * p_driver, drv_encoder_port_t const * p_irq,
    s5_acquire_io_t * p_io)
{
    if ((NULL == p_driver) || (NULL == p_irq) || (NULL == p_io) ||
        (NULL == p_irq->p_reserve) || (NULL == p_irq->p_enable) ||
        (NULL == p_irq->p_release) || p_driver->b_claimed ||
        p_driver->b_reserved) { return (false); }
    p_driver->irq = *p_irq;
    *p_io = (s5_acquire_io_t){p_driver,drv_s5_open,drv_s5_servo,
        drv_s5_trigger,drv_s5_echo,drv_s5_close};
    return (true);
}

static bool drv_s5_open (void * p_context,
    s5_acquire_config_t const * p_config)
{
    drv_s5_t * p_driver = p_context;

    if ((NULL == p_driver) || !s5_acquire_validate(p_config) ||
        !drv_s5_free() || p_driver->b_claimed || p_driver->b_reserved ||
        (1U != in_w(0xD0000000UL))) { return (false); }
    if (!p_driver->irq.p_reserve(p_driver->irq.p_context,S5_ECHO_MASK,
        drv_s5_edge,p_driver)) { return (false); }
    p_driver->b_reserved = true;
    p_driver->config = *p_config;
    /* Only release reset; never assert/reset an already live PWM block. */
    clr_w(RESETS_RESET,RESETS_RESET_PWM);
    bool b_reset = false;

    for (uint32_t attempt = 0U; attempt < 32U; ++attempt)
    {
        if (0U != (in_w(RESETS_RESET_DONE) & RESETS_RESET_PWM))
        {
            b_reset = true; break;
        }
    }
    if (!b_reset) { drv_s5_close(p_driver); return (false); }
    p_driver->b_claimed = true;
    out_w(S5_OUT_CLR,S5_TRIG_MASK | S5_SERVO_MASK);
    bool b_ok = (E_OK == gpio_set_pin(26U,GPIO_MODE_OUT)) &&
        (E_OK == gpio_set_pin(6U,GPIO_MODE_IN)) &&
        (E_OK == gpio_set_pin(15U,GPIO_MODE_OUT));

    out_w(PWM_CH7_BASE + PWM_CHx_CSR,0U);
    out_w(PWM_CH7_BASE + PWM_CHx_CC,0U);
    out_w(PWM_CH7_BASE + PWM_CHx_CTR,0U);
    out_w(PWM_CH7_BASE + PWM_CHx_DIV,125U * 16U);
    out_w(PWM_CH7_BASE + PWM_CHx_TOP,
        (uint32_t)p_config->servo_period_us - 1U);
    b_ok = b_ok && p_driver->irq.p_enable(p_driver->irq.p_context,S5_ECHO_MASK);
    if (!b_ok) { drv_s5_close(p_driver); }
    return (b_ok);
}

static bool drv_s5_servo (void * p_context, uint8_t angle)
{
    drv_s5_t * p_driver = p_context;

    if (!p_driver->b_claimed || (angle > 180U) ||
        (1U != in_w(0xD0000000UL))) { return (false); }
    uint32_t pulse = p_driver->config.servo_min_us +
        ((uint32_t)(p_driver->config.servo_max_us -
        p_driver->config.servo_min_us) * angle) / 180U;

    /* Owned slice 7 only; GP26 stays SIO and never aliases motor PWM5A. */
    out_w(PWM_CH7_BASE + PWM_CHx_CC,pulse << 16U);
    if (E_OK != pwm_set_pin(15U)) { return (false); }
    out_w(PWM_CH7_BASE + PWM_CHx_CSR,1U);
    return (true);
}

static bool drv_s5_trigger (void * p_context)
{
    drv_s5_t * p_driver = p_context;
    UINT saved;

    if (!p_driver->b_claimed || (1U != in_w(0xD0000000UL)) ||
        (0U != (in_w(GPIO_IN) & S5_ECHO_MASK))) { return (false); }
    DI(saved);
    S5_COMPILER_BARRIER();
    p_driver->echo_state = S5_ECHO_ARMED;
    S5_COMPILER_BARRIER();
    EI(saved);
    out_w(S5_OUT_SET,S5_TRIG_MASK);
    WaitUsec(10U); /* HC-SR04 trigger protocol, NOT an RTOS settle delay. */
    out_w(S5_OUT_CLR,S5_TRIG_MASK);
    return (true);
}

static int32_t drv_s5_echo (void * p_context, uint32_t * p_width,
    uint64_t * p_capture)
{
    drv_s5_t * p_driver = p_context;
    UINT saved;

    if (1U != in_w(0xD0000000UL)) { return (-1); }
    DI(saved); /* Same core as ISR, never used as cross-core synchronization. */
    S5_COMPILER_BARRIER();
    uint8_t state = p_driver->echo_state;
    int32_t result = (state >= S5_ECHO_ERROR) ? -1 :
        ((S5_ECHO_COMPLETE == state) ? 1 : 0);

    *p_width = p_driver->width_us;
    *p_capture = p_driver->fall_us;
    S5_COMPILER_BARRIER();
    EI(saved);
    return (result);
}

static void drv_s5_close (void * p_context)
{
    drv_s5_t * p_driver = p_context;

    if (1U != in_w(0xD0000000UL)) { return; }
    if (p_driver->b_reserved)
    {
        p_driver->irq.p_release(p_driver->irq.p_context,S5_ECHO_MASK);
        p_driver->b_reserved = false;
    }
    if (p_driver->b_claimed)
    {
        out_w(S5_OUT_CLR,S5_TRIG_MASK | S5_SERVO_MASK);
        (void)gpio_set_pin(15U,GPIO_MODE_OUT);
        out_w(PWM_CH7_BASE + PWM_CHx_CSR,0U);
        out_w(PWM_CH7_BASE + PWM_CHx_CC,0U);
        p_driver->b_claimed = false;
    }
    p_driver->echo_state = S5_ECHO_IDLE;
}

static void drv_s5_edge (void * p_context, uint32_t events, uint32_t levels)
{
    drv_s5_t * p_driver = p_context;
    uint64_t now_us = 0U;

    uint8_t state = p_driver->echo_state;

    if (((S5_ECHO_ARMED != state) && (S5_ECHO_RISE != state)) ||
        (S5_ECHO_MASK != events)) { return; }
    if (!drv_s5_now(&now_us))
    {
        p_driver->echo_state = S5_ECHO_ERROR;
        return;
    }
    if (0U != (levels & S5_ECHO_MASK))
    {
        if (S5_ECHO_RISE == state)
        {
            p_driver->echo_state = S5_ECHO_ERROR;
            return;
        }
        p_driver->rise_us = now_us;
        p_driver->echo_state = S5_ECHO_RISE;
    }
    else if ((S5_ECHO_RISE != state) || (now_us <= p_driver->rise_us) ||
        (now_us - p_driver->rise_us > p_driver->config.echo_timeout_us))
    {
        p_driver->echo_state = S5_ECHO_ERROR;
    }
    else
    {
        p_driver->fall_us = now_us;
        p_driver->width_us = (uint32_t)(now_us - p_driver->rise_us);
        p_driver->echo_state = S5_ECHO_COMPLETE;
    }
}

static bool drv_s5_free (void)
{
    /* Verify the template's 125 MHz system clock before divider use. */
    bool b_ok = (0 == DEVCONF_ADC_PIN_INIT_0) &&
        (0U == (in_w(GPIO_OE) &
        (S5_TRIG_MASK | S5_SERVO_MASK | S5_ECHO_MASK))) &&
        (0U == (in_w(PWM_CH7_BASE + PWM_CHx_CSR) & 1U)) &&
        (0U == (in_w(PWM_INTE) & (1UL << 7U))) &&
        (0U != (in_w(XOSC_STATUS) & 0x80000000U)) &&
        (0x80000001U == (in_w(PLL_SYS_BASE + PLL_CS) & 0x8000003FU)) &&
        (125U == (in_w(PLL_SYS_BASE + PLL_FBDIV_INT) & 0xFFFU)) &&
        (0x62000U == (in_w(PLL_SYS_BASE + PLL_PRIM) & 0x77000U)) &&
        (1U == (in_w(CLK_SYS + CLK_x_CTRL) & 0xE1U)) &&
        (2U == in_w(CLK_SYS + CLK_x_SELECTED)) &&
        (256U == in_w(CLK_SYS + CLK_x_DIV));
    const uint32_t pins[] = {6U,15U,26U};

    for (uint32_t index = 0U; index < 3U; ++index)
    {
        uint32_t control = in_w(GPIO_CTRL(pins[index]));
        uint32_t function = control & 31U;

        b_ok = b_ok && ((5U == function) || (31U == function)) &&
            (0U == (control & 0x30033300UL));
    }
    b_ok = b_ok && (4U != (in_w(GPIO_CTRL(14U)) & 31U));
    return (b_ok);
}
#endif
/*** end of file ***/
