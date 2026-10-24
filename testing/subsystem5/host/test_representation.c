/** @file test_representation.c @brief Acquisition numerical bounds, host only. */
#include "../../../app_program/subsystem5/s5_acquire.h"
#include <stdio.h>

static uint64_t g_trigger_us;
static uint64_t g_capture_us;
static uint64_t g_now_us;
static uint32_t g_width_us;
static uint32_t g_failures;
static bool g_forged_capture;
static bool g_closed;

static bool port_open (void * p_context, s5_acquire_config_t const * p_config)
{
    (void)p_context; (void)p_config;
    g_closed = false;
    return (true);
}
static bool port_servo (void * p_context, uint8_t angle)
{
    (void)p_context;
    return (angle <= 180U);
}
static bool port_trigger (void * p_context)
{
    (void)p_context;
    return (true);
}
static int32_t port_echo (void * p_context, uint32_t * p_width,
    uint64_t * p_capture)
{
    (void)p_context;
    *p_width = g_width_us;
    *p_capture = g_forged_capture ? g_capture_us : g_trigger_us + g_width_us;
    return ((*p_capture <= g_now_us) ? 1 : 0);
}
static void port_close (void * p_context)
{
    (void)p_context;
    g_closed = true;
}
static s5_acquire_config_t config (void)
{
    const s5_acquire_config_t fixture = {61000U,24000U,10000U,2000000U,
        20000U,1000U,2000U,58U,400U,15U,true,true};
    return (fixture); /* Synthetic fixture; never a production calibration. */
}
static s5_acquire_io_t io (void)
{
    const s5_acquire_io_t fixture = {NULL,port_open,port_servo,port_trigger,
        port_echo,port_close};
    return (fixture);
}
static void check (uint32_t id, bool b_pass)
{
    (void)printf("[TEST] S5-U%u %s representation and timing bounds\n",
        id,b_pass ? "PASS" : "FAIL");
    if (!b_pass) { ++g_failures; }
}
static bool step (s5_acquire_t * p_owner, uint64_t now_us,
    bool b_scan, bool b_stationary, bool b_cancel)
{
    g_now_us = now_us;
    return (s5_acquire_step(p_owner,now_us,b_scan,b_stationary,b_cancel));
}
static bool pulse (uint16_t scale, uint32_t width, bool b_expected,
    uint16_t expected_cm)
{
    s5_acquire_t owner = {0};
    s5_acquire_config_t fixture = config();
    s5_acquire_io_t port = io();

    fixture.echo_us_per_cm = scale;
    if (scale > 58U)
    {
        fixture.echo_timeout_us = 500000U;
        fixture.sample_period_us = 600000U;
    }
    g_width_us = width;
    g_trigger_us = fixture.servo_settle_us;
    bool b_open = s5_acquire_open(&owner,&fixture,&port,0U);
    bool b_step = b_open && step(&owner,g_trigger_us,
        false,false,false);
    b_step = b_step && step(&owner,g_trigger_us + width,
        false,false,false);
    bool b_ok = b_expected ? (b_step && owner.b_forward_valid &&
        (expected_cm == owner.forward_cm)) :
        (!b_step && g_closed && !owner.b_forward_valid && !owner.b_open);

    s5_acquire_close(&owner);
    return (b_ok);
}
int main (void)
{
    check(101U,pulse(58U,115U,false,0U) && pulse(58U,116U,true,2U) &&
        pulse(58U,23200U,true,400U) && pulse(58U,23201U,false,0U));
    check(102U,pulse(1000U,400000U,true,400U) &&
        pulse(1000U,400001U,false,0U));

    s5_acquire_io_t port = io();
    s5_acquire_config_t fixture = config();
    bool b_ok = true;

    for (uint32_t state = 4U; state <= UINT8_MAX; ++state)
    {
        s5_acquire_t owner = {0};

        b_ok = s5_acquire_open(&owner,&fixture,&port,0U) && b_ok;
        owner.state = (uint8_t)state;
        b_ok = !step(&owner,1U,false,false,false) &&
            g_closed && !owner.b_open && b_ok;
    }
    check(103U,b_ok);

    s5_acquire_t owner = {0};
    uint64_t start = UINT32_MAX - 5000U;

    g_width_us = 580U;
    g_trigger_us = start + fixture.servo_settle_us;
    b_ok = s5_acquire_open(&owner,&fixture,&port,start) &&
        step(&owner,g_trigger_us,false,false,false) &&
        step(&owner,g_trigger_us + 580U,false,false,false) &&
        owner.b_forward_valid && (owner.forward_us > UINT32_MAX);
    b_ok = !step(&owner,start,false,false,false) && g_closed && b_ok;
    owner = (s5_acquire_t){0};
    b_ok = !s5_acquire_open(&owner,&fixture,&port,UINT64_MAX) && b_ok;
    check(104U,b_ok);

    owner = (s5_acquire_t){0};
    g_forged_capture = true; g_capture_us = 10001U;
    b_ok = s5_acquire_open(&owner,&fixture,&port,0U) &&
        step(&owner,10000U,false,false,false) &&
        !step(&owner,10001U,false,false,false) && g_closed;
    g_forged_capture = false;
    check(105U,b_ok);

    b_ok = true;
    const uint8_t counts[] = {0U,2U,10U,UINT8_MAX};

    for (size_t index = 0U; index < sizeof(counts); ++index)
    {
        owner = (s5_acquire_t){0};
        b_ok = s5_acquire_open(&owner,&fixture,&port,0U) && b_ok;
        owner.b_scan = true; owner.b_fine = true;
        owner.fine_count = counts[index];
        b_ok = !step(&owner,1U,false,true,false) && g_closed && b_ok;
    }
    owner = (s5_acquire_t){0};
    b_ok = s5_acquire_open(&owner,&fixture,&port,0U) && b_ok;
    owner.b_scan = true; owner.index = UINT8_MAX;
    b_ok = !step(&owner,1U,false,true,false) && g_closed && b_ok;
    check(106U,b_ok && (1U == sizeof(owner.state)) &&
        (1U == sizeof(owner.index)) && (1U == sizeof(owner.fine_count)));
    return ((0U == g_failures) ? 0 : 1);
}
