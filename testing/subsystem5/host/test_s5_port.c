/** @file test_s5_port.c @brief Real register adapter, synthetic MMIO only. */
#include "test_s5_port.h"
#include "../../../app_program/subsystem5/drv_s5.h"
#include <stdio.h>
#include <string.h>

typedef struct { uint32_t address; uint32_t value; } reg_t;
static reg_t g_regs[128];
static size_t g_count;
static uint64_t g_now;
static uint32_t g_lock;
static uint32_t g_reserves;
static uint32_t g_releases;
static uint32_t g_writes;
static uint32_t g_failure;
static bool g_bad;
static bool g_reserved;
static drv_encoder_sink_t g_sink;
static void * g_context;
static uint32_t g_failures;

static uint32_t * reg (uint32_t address)
{
    for (size_t index = 0U; index < g_count; ++index)
    {
        if (g_regs[index].address == address) { return (&g_regs[index].value); }
    }
    if (g_count >= 128U) { g_bad = true; return (&g_regs[0].value); }
    g_regs[g_count] = (reg_t){address,0U};
    return (&g_regs[g_count++].value);
}
uint32_t in_w (uint32_t address)
{
    if (0x40054024UL == address) { return ((uint32_t)(g_now >> 32U)); }
    if (0x40054028UL == address) { return ((uint32_t)g_now); }
    return (*reg(address));
}
void out_w (uint32_t address,uint32_t value)
{
    ++g_writes;
    if (GPIO_OE_SET == address) { *reg(GPIO_OE) |= value; }
    else if (GPIO_OE_CLR == address) { *reg(GPIO_OE) &= ~value; }
    else if (0xD0000014UL == address) { *reg(0xD0000010UL) |= value; }
    else if (0xD0000018UL == address) { *reg(0xD0000010UL) &= ~value; }
    else { *reg(address) = value; }
}
void clr_w (uint32_t address,uint32_t value)
{ ++g_writes; *reg(address) &= ~value; }
ER gpio_set_pin (UINT pin,UINT mode)
{
    if ((pin != 6U) && (pin != 15U) && (pin != 26U)) { g_bad = true; }
    if ((3U == g_failure) && (26U == pin)) { return (E_SYS); }
    *reg(GPIO_CTRL(pin)) = 5U;
    if (GPIO_MODE_OUT == mode) { *reg(GPIO_OE) |= 1UL << pin; }
    else { *reg(GPIO_OE) &= ~(1UL << pin); }
    return (E_OK);
}
ER pwm_set_pin (UINT pin)
{
    if (15U != pin) { g_bad = true; return (E_SYS); }
    *reg(GPIO_CTRL(pin)) = 4U; return (E_OK);
}
void WaitUsec (uint32_t delay)
{ if (10U != delay) { g_bad = true; } g_now += delay; }
uint32_t test_s5_enter (void)
{ uint32_t old = g_lock; g_lock = 1U; return (old); }
void test_s5_leave (uint32_t saved) { g_lock = saved; }
static bool reserve (void * p,uint32_t mask,drv_encoder_sink_t sink,void * ctx)
{
    (void)p; ++g_reserves;
    if ((mask != (1UL << 6U)) || g_reserved) { g_bad = true; }
    if (2U == g_failure) { return (false); }
    g_sink = sink; g_context = ctx; g_reserved = true; return (true);
}
static bool enable (void * p,uint32_t mask)
{
    (void)p;
    if (!g_reserved || (mask != (1UL << 6U))) { g_bad = true; }
    return (4U != g_failure);
}
static void release (void * p,uint32_t mask)
{
    (void)p;
    if (!g_reserved || (mask != (1UL << 6U)) || (0U != g_lock)) { g_bad = true; }
    g_reserved = false; ++g_releases;
}
static void check (uint32_t id,bool pass)
{
    (void)printf("[TEST] S5-U%u %s real GPIO/PWM/IRQ adapter\n",id,
        (pass && !g_bad) ? "PASS" : "FAIL");
    if (!pass || g_bad) { ++g_failures; }
}
static void reset (void)
{
    g_count = 0U; g_now = 0U; g_reserves = 0U; g_releases = 0U;
    g_writes = 0U; g_failure = 0U; g_reserved = false; g_bad = false;
    *reg(0xD0000000UL) = 1U;
    *reg(XOSC_STATUS) = 0x80000000U;
    *reg(PLL_SYS_BASE + PLL_CS) = 0x80000001U;
    *reg(PLL_SYS_BASE + PLL_FBDIV_INT) = 125U;
    *reg(PLL_SYS_BASE + PLL_PRIM) = 0x62000U;
    *reg(CLK_SYS + CLK_x_CTRL) = 1U;
    *reg(CLK_SYS + CLK_x_SELECTED) = 2U;
    *reg(CLK_SYS + CLK_x_DIV) = 256U;
    *reg(RESETS_RESET_DONE) = RESETS_RESET_PWM;
    *reg(PWM_CH4_BASE + PWM_CHx_CC) = 0x12345678U;
    *reg(PWM_CH5_BASE + PWM_CHx_CC) = 0x87654321U;
    *reg(GPIO_CTRL(6U)) = 31U;
    *reg(GPIO_CTRL(15U)) = 31U;
    *reg(GPIO_CTRL(26U)) = 31U;
}
int main (void)
{
    s5_acquire_config_t config = {61000U,24000U,10000U,2000000U,
        20000U,1000U,2000U,58U,400U,15U,true,true};
    drv_encoder_port_t irq = {NULL,reserve,enable,release};
    drv_s5_t driver = {0};
    s5_acquire_io_t io = {0};
    uint32_t width = 0U;
    uint64_t capture = 0U;

    reset();
    bool result = drv_s5_bind(&driver,&irq,&io) && (0U == g_writes) &&
        io.p_open(io.p_context,&config) && (1U == g_reserves) &&
        (5U == in_w(GPIO_CTRL(26U)));
    check(64U,result);
    if (!result || (NULL == io.p_servo) || (NULL == io.p_trigger) ||
        (NULL == io.p_echo) || (NULL == io.p_close) || (NULL == g_sink))
    { return 1; }
    result = io.p_servo(io.p_context,0U) &&
        (1000UL << 16U) == in_w(PWM_CH7_BASE + PWM_CHx_CC) &&
        io.p_servo(io.p_context,180U) &&
        ((2000UL << 16U) == in_w(PWM_CH7_BASE + PWM_CHx_CC)) &&
        (0x12345678U == in_w(PWM_CH4_BASE + PWM_CHx_CC)) &&
        (0x87654321U == in_w(PWM_CH5_BASE + PWM_CHx_CC));
    check(65U,result);
    result = io.p_trigger(io.p_context);
    if (!result || (NULL == g_sink)) { return 1; }
    g_now = 100U; g_sink(g_context,1UL << 6U,1UL << 6U);
    g_now = 680U; g_sink(g_context,1UL << 6U,0U);
    result = result && (1 == io.p_echo(io.p_context,&width,&capture)) &&
        (580U == width) && (680U == capture) &&
        (0U == (in_w(0xD0000010UL) & (1UL << 26U)));
    check(66U,result);
    io.p_close(io.p_context); io.p_close(io.p_context);
    check(67U,(1U == g_releases) && !driver.b_reserved && !driver.b_claimed &&
        (0U == in_w(PWM_CH7_BASE + PWM_CHx_CSR)));
    for (uint32_t failure = 1U; failure <= 4U; ++failure)
    {
        reset(); driver = (drv_s5_t){0}; g_failure = failure;
        if (1U == failure) { *reg(GPIO_OE) |= 1UL << 26U; }
        result = drv_s5_bind(&driver,&irq,&io) &&
            !io.p_open(io.p_context,&config) && !driver.b_reserved &&
            !driver.b_claimed && ((failure < 3U) ? (0U == g_releases) :
            (1U == g_releases));
        check(67U + failure,result);
    }
    reset(); driver = (drv_s5_t){0};
    result = drv_s5_bind(&driver,&irq,&io) && io.p_open(io.p_context,&config) &&
        io.p_trigger(io.p_context);
    if (!result || (NULL == g_sink)) { return 1; }
    g_now = 100U; g_sink(g_context,1UL << 6U,1UL << 6U);
    g_now = 101U; g_sink(g_context,1UL << 6U,1UL << 6U);
    result = result && (-1 == io.p_echo(io.p_context,&width,&capture));
    io.p_close(io.p_context);
    check(72U,result);
    return ((0U == g_failures) ? 0 : 1);
}
/*** end of file ***/
