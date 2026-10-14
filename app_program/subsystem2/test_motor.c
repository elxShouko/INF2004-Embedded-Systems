/**
 * @file test_motor.c
 * @brief S2-U43..U56 synthetic motor tests; NO physical backend calls.
 * All clock, guard, polarity and duty settings here are fixtures only.
 */
#include "test_motor.h"
#if defined(MOTION_CONFIG_HOST_TEST) || APP_TEST
#include "drv_motor.h"
#include <stddef.h>
#include <math.h>
#include <float.h>
#if defined(MOTION_CONFIG_HOST_TEST)
#include <stdio.h>
#else
#define PROHIBIT_DEF_SIZE_T 1
#include "subsystem2.h"
#endif

typedef struct
{
    uint16_t level_a[2];
    uint16_t level_b[2];
    uint16_t period;
    uint32_t prepares;
    uint32_t applies;
    uint32_t stops;
    bool b_fail_prepare;
    bool b_fail_apply;
    bool b_fail_stop;
} test_motor_fake_t;

typedef struct
{
    drv_motor_t motor;
    test_motor_fake_t fake;
} test_motor_fixture_t;

static drv_motor_config_t test_motor_config (void);
static bool test_motor_prepare (void * p_context, uint16_t period,
                                    uint32_t clock_hz);
static bool test_motor_apply (void * p_context, drv_motor_wheel_t wheel,
                                  uint16_t level_a, uint16_t level_b);
static bool test_motor_stop (void * p_context);
static bool test_motor_start (test_motor_fixture_t * p_fixture,
                                  drv_motor_config_t config);
static uint32_t test_motor_report (bool b_pass, char const * p_id);
static bool test_motor_validate (void);
static bool test_motor_effort (void);
static bool test_motor_polarity (void);
static bool test_motor_zero (void);
static bool test_motor_startup (void);
static bool test_motor_reversal (void);
static bool test_motor_bypass (void);
static bool test_motor_pending (void);
static bool test_motor_arguments (void);
static bool test_motor_nonfinite (void);
static bool test_motor_faults (void);
static bool test_motor_clocks (void);
static bool test_motor_independent (void);
static bool test_motor_disarm (void);

uint32_t test_motor (void)
{
    uint32_t failed = 0U;

    failed += test_motor_report(test_motor_validate(), "S2-U43");
    failed += test_motor_report(test_motor_effort(), "S2-U44");
    failed += test_motor_report(test_motor_polarity(), "S2-U45");
    failed += test_motor_report(test_motor_zero(), "S2-U46");
    failed += test_motor_report(test_motor_startup(), "S2-U47");
    failed += test_motor_report(test_motor_reversal(), "S2-U48");
    failed += test_motor_report(test_motor_bypass(), "S2-U49");
    failed += test_motor_report(test_motor_pending(), "S2-U50");
    failed += test_motor_report(test_motor_arguments(), "S2-U51");
    failed += test_motor_report(test_motor_nonfinite(), "S2-U52");
    failed += test_motor_report(test_motor_faults(), "S2-U53");
    failed += test_motor_report(test_motor_clocks(), "S2-U54");
    failed += test_motor_report(test_motor_independent(), "S2-U55");
    failed += test_motor_report(test_motor_disarm(), "S2-U56");
#if defined(MOTION_CONFIG_HOST_TEST)
    printf("[TEST] subsystem2 M2 SUMMARY %u passed, %u failed\n",
           (unsigned int)(14U - failed), (unsigned int)failed);
#else
    tm_printf((UB*)"[TEST] subsystem2 M2 SUMMARY %u passed, %u failed\n",
              (unsigned int)(14U - failed), (unsigned int)failed);
#endif
    return (failed);
}

static drv_motor_config_t test_motor_config (void)
{
    drv_motor_config_t config = {125000000U, 20000U, 100U, 1.0F,
        {MOTION_TYPES_POLARITY_NORMAL, MOTION_TYPES_POLARITY_NORMAL},
        DRV_MOTOR_BRAKE, MOTION_CONFIG_CALIBRATION_CONFIRMED, true};

    return (config);
}

static bool test_motor_prepare (void * p_context, uint16_t period,
                                    uint32_t clock_hz)
{
    test_motor_fake_t * p_fake = p_context;

    p_fake->prepares += 1U;
    p_fake->period = period;
    for (uint32_t index = 0U; index < 2U; ++index)
    {
        p_fake->level_a[index] = 0U;
        p_fake->level_b[index] = 0U;
    }
    if (p_fake->b_fail_prepare)
    {
        p_fake->level_a[1] = period; /* Simulate partial setup failure. */
    }
    return ((!p_fake->b_fail_prepare) && (125000000U == clock_hz));
}

static bool test_motor_apply (void * p_context, drv_motor_wheel_t wheel,
                                  uint16_t level_a, uint16_t level_b)
{
    test_motor_fake_t * p_fake = p_context;

    p_fake->applies += 1U;
    p_fake->level_a[wheel] = level_a;
    if (!p_fake->b_fail_apply)
    {
        p_fake->level_b[wheel] = level_b;
    }
    return ((!p_fake->b_fail_apply) && (level_a <= p_fake->period) &&
            (level_b <= p_fake->period));
}

static bool test_motor_stop (void * p_context)
{
    test_motor_fake_t * p_fake = p_context;

    p_fake->stops += 1U;
    p_fake->level_a[0] = 0U;
    p_fake->level_b[0] = 0U;
    if (!p_fake->b_fail_stop)
    {
        p_fake->level_a[1] = 0U;
        p_fake->level_b[1] = 0U;
    }
    return (!p_fake->b_fail_stop);
}

static bool test_motor_start (test_motor_fixture_t * p_fixture,
                                  drv_motor_config_t config)
{
    *p_fixture = (test_motor_fixture_t){0};
    drv_motor_io_t io = {&p_fixture->fake, test_motor_prepare,
        test_motor_apply, test_motor_stop};

    return ((DRV_MOTOR_OK == drv_motor_init(&p_fixture->motor, &config, &io)) &&
            (DRV_MOTOR_OK == drv_motor_arm(&p_fixture->motor, 0U)));
}

static uint32_t test_motor_report (bool b_pass, char const * p_id)
{
#if defined(MOTION_CONFIG_HOST_TEST)
    printf("[TEST] %s %s  fake motor backend\n", p_id,
           b_pass ? "PASS" : "FAIL");
#else
    tm_printf((UB*)"[TEST] %s %s  fake motor backend\n", p_id,
              b_pass ? "PASS" : "FAIL");
#endif
    return (b_pass ? 0U : 1U);
}

static bool test_motor_validate (void)
{

    test_motor_fixture_t fixture = {0};
    drv_motor_config_t config = test_motor_config();
    bool b_pass = test_motor_start(&fixture, config);

    b_pass = (6250U == fixture.motor.period) &&
        (6250U == fixture.motor.duty_limit) && b_pass;
    config.pwm_hz = 19999U;
    b_pass = (test_motor_start(&fixture, config)) &&
        (6251U == fixture.motor.period) &&
        ((uint64_t)config.clock_hz <=
         ((uint64_t)config.pwm_hz * fixture.motor.period)) && b_pass;
    for (uint32_t index = 0U; index < 20U; ++index)
    {
        config = test_motor_config();
        fixture = (test_motor_fixture_t){0};
        switch (index)
        {
            case 0U:
                config.pwm_hz = 0U;
            break;
            case 1U:
                config.pwm_hz = 20001U;
            break;
            case 2U:
                config.pwm_hz = 1U;
            break;
            case 3U:
                config.clock_hz = 0U;
            break;
            case 4U:
                config.reversal_guard_us = 0U;
            break;
            case 5U:
                config.max_duty = NAN;
            break;
            case 6U:
                config.max_duty = 0.0F;
            break;
            case 7U:
                config.max_duty = 1.01F;
            break;
            case 8U:
                config.zero_mode = (drv_motor_stop_t)99;
            break;
            case 9U:
                config.polarity[0] = MOTION_TYPES_POLARITY_UNKNOWN;
            break;
            case 10U:
                config.calibration = MOTION_CONFIG_CALIBRATION_UNVERIFIED;
            break;
            case 11U:
                config.b_resources_verified = false;
            break;
            case 12U:
                config.clock_hz = 1U;
            break;
            case 13U:
                config.max_duty = INFINITY;
            break;
            case 14U:
                config.max_duty = -INFINITY;
            break;
            case 15U:
                config.max_duty = 0.000001F;
            break;
            case 16U:
                config.max_duty = -0.1F;
            break;
            case 17U:
                config.polarity[1] = (motion_types_polarity_t)99;
            break;
            case 18U:
                config.calibration = (motion_config_calibration_t)99;
            break;
            default:
                /* Stored float is just BELOW 1/6250. Binary32 multiplication
                 * rounds to one count; exact ceiling calculation must reject.
                 */
                config.max_duty = 0.00016F;
            break;
        }
        b_pass = !(test_motor_start(&fixture, config)) &&
            (0U == fixture.fake.prepares) && (0U == fixture.fake.applies) &&
            (0U == fixture.fake.stops) && !fixture.motor.b_initialised && b_pass;
    }

    return (b_pass);
}

static bool test_motor_effort (void)
{

    test_motor_fixture_t fixture = {0};
    drv_motor_config_t config = test_motor_config();
    bool b_pass = test_motor_start(&fixture, config);

    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        0.5F, 1U)) && (3125U == fixture.fake.level_a[0]) &&
        (0U == fixture.fake.level_b[0]) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        FLT_MAX, 2U)) && (6250U == fixture.fake.level_a[0]) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        0.00001F, 3U)) && (0U == fixture.fake.level_a[0]) &&
        (0U == fixture.fake.level_b[0]) && b_pass;
    config.max_duty = 0.333333F;
    b_pass = (test_motor_start(&fixture, config)) &&
        (2083U == fixture.motor.duty_limit) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        2.0F, 1U)) && (2083U == fixture.fake.level_a[0]) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_RIGHT,
        -FLT_MAX, 2U)) && (2083U == fixture.fake.level_b[1]) &&
        (0U == fixture.fake.level_a[1]) && b_pass;

    return (b_pass);
}

static bool test_motor_polarity (void)
{

    test_motor_fixture_t fixture = {0};
    drv_motor_config_t config = test_motor_config();
    bool b_pass;

    config.polarity[1] = MOTION_TYPES_POLARITY_REVERSED;
    b_pass = test_motor_start(&fixture, config);
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        0.4F, 1U)) && (2500U == fixture.fake.level_a[0]) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_RIGHT,
        0.4F, 1U)) && (2500U == fixture.fake.level_b[1]) &&
        (0U == fixture.fake.level_a[1]) &&
        (1 == fixture.motor.channel[0].last_direction) &&
        (-1 == fixture.motor.channel[1].last_direction) && b_pass;

    return (b_pass);
}

static bool test_motor_zero (void)
{

    test_motor_fixture_t fixture = {0};
    drv_motor_config_t config = test_motor_config();
    bool b_pass = test_motor_start(&fixture, config);

    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        -0.0F, 1U)) && (0U == fixture.fake.level_a[0]) &&
        (0U == fixture.fake.level_b[0]) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_stop(&fixture.motor, DRV_MOTOR_LEFT,
        DRV_MOTOR_COAST, 2U)) && (6250U == fixture.fake.level_a[0]) &&
        (6250U == fixture.fake.level_b[0]) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_stop(&fixture.motor, DRV_MOTOR_LEFT,
        DRV_MOTOR_BRAKE, 3U)) && (0U == fixture.fake.level_a[0]) &&
        (0U == fixture.fake.level_b[0]) && b_pass;
    config.zero_mode = DRV_MOTOR_COAST;
    b_pass = (test_motor_start(&fixture, config)) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_RIGHT,
        0.0F, 1U)) && (6250U == fixture.fake.level_a[1]) &&
        (6250U == fixture.fake.level_b[1]) && b_pass;

    return (b_pass);
}

static bool test_motor_startup (void)
{

    test_motor_fixture_t fixture = {0};
    drv_motor_config_t config = test_motor_config();
    drv_motor_io_t io = {&fixture.fake, test_motor_prepare,
        test_motor_apply, test_motor_stop};
    bool b_pass = (DRV_MOTOR_OK == drv_motor_init(&fixture.motor, &config, &io));

    config.max_duty = NAN;
    io.p_apply = NULL;
    b_pass = (0U == fixture.fake.prepares) &&
        (DRV_MOTOR_DISABLED == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
         1.0F, 0U)) && (0U == fixture.fake.applies) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_arm(&fixture.motor, 100U)) &&
        (1U == fixture.fake.prepares) &&
        (0U == fixture.fake.level_a[0]) && (0U == fixture.fake.level_b[0]) &&
        (0U == fixture.fake.level_a[1]) && (0U == fixture.fake.level_b[1]) &&
        (DRV_MOTOR_STOPPED == fixture.motor.channel[0].state) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        0.5F, 101U)) && (3125U == fixture.fake.level_a[0]) && b_pass;

    return (b_pass);
}

static bool test_motor_reversal (void)
{

    test_motor_fixture_t fixture = {0};
    bool b_pass = test_motor_start(&fixture, test_motor_config());

    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        0.5F, 10U)) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        -0.5F, 20U)) && (0U == fixture.fake.level_a[0]) &&
        (0U == fixture.fake.level_b[0]) &&
        (DRV_MOTOR_REVERSING == fixture.motor.channel[0].state) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_service(&fixture.motor,
        DRV_MOTOR_LEFT, 119U, true)) && (0U == fixture.fake.level_b[0]) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_service(&fixture.motor,
        DRV_MOTOR_LEFT, 120U, false)) && (0U == fixture.fake.level_b[0]) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_service(&fixture.motor,
        DRV_MOTOR_LEFT, 120U, true)) && (3125U == fixture.fake.level_b[0]) &&
        (-1 == fixture.motor.channel[0].last_direction) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        0.5F, 130U)) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_service(&fixture.motor,
        DRV_MOTOR_LEFT, 230U, true)) && (3125U == fixture.fake.level_a[0]) &&
        (0U == fixture.fake.level_b[0]) && b_pass;

    return (b_pass);
}

static bool test_motor_bypass (void)
{

    bool b_pass = true;

    for (uint32_t mode = 0U; mode < 3U; ++mode)
    {
        test_motor_fixture_t fixture = {0};

        b_pass = (test_motor_start(&fixture, test_motor_config())) &&
                 b_pass;
        b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
            1.0F, 10U)) && b_pass;
        drv_motor_result_t result = (2U == mode) ?
            drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT, 0.0F, 20U) :
            drv_motor_stop(&fixture.motor, DRV_MOTOR_LEFT,
                           (drv_motor_stop_t)mode, 20U);

        b_pass = (DRV_MOTOR_OK == result) && b_pass;
        b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor,
            DRV_MOTOR_LEFT, -1.0F, 30U)) &&
            (20U == fixture.motor.channel[0].neutral_since_us) &&
            (0U == fixture.fake.level_a[0]) &&
            (0U == fixture.fake.level_b[0]) && b_pass;
        b_pass = (DRV_MOTOR_WAIT == drv_motor_service(&fixture.motor,
            DRV_MOTOR_LEFT, 119U, true)) && b_pass;
        b_pass = (DRV_MOTOR_OK == drv_motor_service(&fixture.motor,
            DRV_MOTOR_LEFT, 120U, true)) &&
            (6250U == fixture.fake.level_b[0]) && b_pass;
    }

    return (b_pass);
}

static bool test_motor_pending (void)
{

    test_motor_fixture_t fixture = {0};
    bool b_pass = test_motor_start(&fixture, test_motor_config());

    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        0.5F, 10U)) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        -0.5F, 20U)) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        -0.25F, 50U)) && (20U == fixture.motor.channel[0].neutral_since_us) &&
        b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_service(&fixture.motor,
        DRV_MOTOR_LEFT, 120U, true)) && (1563U == fixture.fake.level_b[0]) &&
        b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        1.0F, 130U)) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_stop(&fixture.motor, DRV_MOTOR_LEFT,
        DRV_MOTOR_BRAKE, 140U)) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_service(&fixture.motor,
        DRV_MOTOR_LEFT, 1000U, true)) && (0U == fixture.fake.level_a[0]) &&
        (0U == fixture.fake.level_b[0]) && b_pass;

    return (b_pass);
}

static bool test_motor_arguments (void)
{

    bool b_pass = (DRV_MOTOR_BAD_PARAM == drv_motor_set(NULL,
        DRV_MOTOR_LEFT, 1.0F, 0U));

    b_pass = (DRV_MOTOR_BAD_PARAM == drv_motor_arm(NULL, 0U)) &&
        (DRV_MOTOR_BAD_PARAM == drv_motor_disarm(NULL)) &&
        (DRV_MOTOR_BAD_PARAM == drv_motor_stop(NULL, DRV_MOTOR_LEFT,
         DRV_MOTOR_BRAKE, 0U)) &&
        (DRV_MOTOR_BAD_PARAM == drv_motor_service(NULL, DRV_MOTOR_LEFT,
         0U, true)) && b_pass;
    test_motor_fixture_t candidate = {0};
    drv_motor_config_t config = test_motor_config();
    drv_motor_io_t io = {&candidate.fake, test_motor_prepare,
        test_motor_apply, test_motor_stop};

    b_pass = (DRV_MOTOR_BAD_PARAM == drv_motor_init(NULL, &config, &io)) &&
        (DRV_MOTOR_BAD_PARAM == drv_motor_init(&candidate.motor, NULL, &io)) &&
        (DRV_MOTOR_BAD_PARAM == drv_motor_init(&candidate.motor, &config,
                                             NULL)) && b_pass;
    for (uint32_t index = 0U; index < 3U; ++index)
    {
        drv_motor_io_t invalid = io;

        if (0U == index)
        {
            invalid.p_prepare = NULL;
        }
        else if (1U == index)
        {
            invalid.p_apply = NULL;
        }
        else
        {
            invalid.p_stop_all = NULL;
        }
        b_pass = (DRV_MOTOR_BAD_PARAM == drv_motor_init(&candidate.motor,
            &config, &invalid)) && !candidate.motor.b_initialised && b_pass;
    }
    b_pass = (0U == candidate.fake.prepares) &&
        (0U == candidate.fake.applies) && (0U == candidate.fake.stops) && b_pass;
    for (uint32_t index = 0U; index < 2U; ++index)
    {
        test_motor_fixture_t fixture = {0};

        b_pass = (test_motor_start(&fixture, test_motor_config())) &&
                 b_pass;
        b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_RIGHT,
            1.0F, 1U)) && b_pass;
        drv_motor_result_t result = (0U == index) ?
            drv_motor_set(&fixture.motor, (drv_motor_wheel_t)99, 1.0F, 2U) :
            drv_motor_stop(&fixture.motor, DRV_MOTOR_LEFT,
                           (drv_motor_stop_t)-1, 2U);

        b_pass = (DRV_MOTOR_BAD_PARAM == result) && fixture.motor.b_fault &&
            fixture.motor.b_stop_confirmed && !fixture.motor.b_armed &&
            (0U == fixture.fake.level_a[1]) &&
            (0U == fixture.fake.level_b[1]) && b_pass;
    }

    return (b_pass);
}

static bool test_motor_nonfinite (void)
{

    static float const invalid[] = {NAN, INFINITY, -INFINITY};
    bool b_pass = true;

    for (size_t index = 0U; index < 3U; ++index)
    {
        test_motor_fixture_t fixture = {0};

        b_pass = (test_motor_start(&fixture, test_motor_config())) &&
                 b_pass;
        b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_RIGHT,
            1.0F, 1U)) && b_pass;
        b_pass = (DRV_MOTOR_BAD_PARAM == drv_motor_set(&fixture.motor,
            DRV_MOTOR_LEFT, invalid[index], 2U)) &&
            fixture.motor.b_fault && fixture.motor.b_stop_confirmed &&
            (1U == fixture.fake.stops) &&
            (0U == fixture.fake.level_a[1]) && b_pass;
        b_pass = (DRV_MOTOR_FAULT == drv_motor_set(&fixture.motor,
            DRV_MOTOR_RIGHT, 1.0F, 3U)) &&
            (0U == fixture.fake.level_a[1]) && b_pass;
    }

    return (b_pass);
}

static bool test_motor_faults (void)
{

    bool b_pass = true;

    for (uint32_t index = 0U; index < 3U; ++index)
    {
        test_motor_fixture_t fixture = {0};

        b_pass = (test_motor_start(&fixture, test_motor_config())) &&
                 b_pass;
        b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_RIGHT,
            1.0F, 1U)) && b_pass;
        fixture.fake.b_fail_apply = true;
        fixture.fake.b_fail_stop = (1U == index);
        b_pass = (DRV_MOTOR_HARDWARE == drv_motor_set(&fixture.motor,
            DRV_MOTOR_LEFT, 1.0F, 2U)) && fixture.motor.b_fault &&
            !fixture.motor.b_armed &&
            (fixture.motor.b_stop_confirmed == (1U != index)) && b_pass;
        if (1U != index)
        {
            b_pass = (0U == fixture.fake.level_a[0]) &&
                (0U == fixture.fake.level_a[1]) && b_pass;
        }
    }
    test_motor_fixture_t fixture = {0};
    drv_motor_config_t config = test_motor_config();
    drv_motor_io_t io = {&fixture.fake, test_motor_prepare,
        test_motor_apply, test_motor_stop};

    fixture.fake.b_fail_prepare = true;
    b_pass = (DRV_MOTOR_OK == drv_motor_init(&fixture.motor, &config, &io)) &&
        b_pass;
    b_pass = (DRV_MOTOR_HARDWARE == drv_motor_arm(&fixture.motor, 0U)) &&
        fixture.motor.b_fault && (1U == fixture.fake.stops) &&
        (0U == fixture.fake.level_a[1]) && b_pass;

    return (b_pass);
}

static bool test_motor_clocks (void)
{

    test_motor_fixture_t fixture = {0};
    bool b_pass = test_motor_start(&fixture, test_motor_config());

    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        1.0F, UINT64_MAX - 1U)) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        -1.0F, UINT64_MAX)) && b_pass;
    b_pass = (DRV_MOTOR_BAD_TIME == drv_motor_service(&fixture.motor,
        DRV_MOTOR_LEFT, 0U, true)) && fixture.motor.b_fault &&
        fixture.motor.b_stop_confirmed &&
        (0U == fixture.fake.level_a[0]) && (0U == fixture.fake.level_b[0]) &&
        b_pass;

    return (b_pass);
}

static bool test_motor_independent (void)
{

    test_motor_fixture_t fixture = {0};
    bool b_pass = test_motor_start(&fixture, test_motor_config());

    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        0.5F, 10U)) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_RIGHT,
        -0.75F, 10U)) && (4688U == fixture.fake.level_b[1]) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        -1.0F, 20U)) && (4688U == fixture.fake.level_b[1]) &&
        (DRV_MOTOR_RUNNING == fixture.motor.channel[1].state) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_stop(&fixture.motor, DRV_MOTOR_RIGHT,
        DRV_MOTOR_COAST, 30U)) &&
        (DRV_MOTOR_REVERSING == fixture.motor.channel[0].state) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_service(&fixture.motor,
        DRV_MOTOR_LEFT, 120U, true)) && (6250U == fixture.fake.level_b[0]) &&
        (6250U == fixture.fake.level_a[1]) &&
        (6250U == fixture.fake.level_b[1]) && b_pass;

    return (b_pass);
}

static bool test_motor_disarm (void)
{

    test_motor_fixture_t fixture = {0};
    bool b_pass = test_motor_start(&fixture, test_motor_config());

    b_pass = (DRV_MOTOR_OK == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        1.0F, 10U)) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_disarm(&fixture.motor)) &&
        !fixture.motor.b_armed && (0U == fixture.fake.level_a[0]) && b_pass;
    b_pass = (DRV_MOTOR_DISABLED == drv_motor_set(&fixture.motor,
        DRV_MOTOR_LEFT, -1.0F, 11U)) && b_pass;
    b_pass = (DRV_MOTOR_OK == drv_motor_arm(&fixture.motor, 20U)) && b_pass;
    b_pass = (DRV_MOTOR_WAIT == drv_motor_set(&fixture.motor, DRV_MOTOR_LEFT,
        -1.0F, 21U)) && b_pass;
    b_pass = (DRV_MOTOR_BAD_PARAM == drv_motor_arm(&fixture.motor, 22U)) &&
        fixture.motor.b_fault && b_pass;
    b_pass = (DRV_MOTOR_FAULT == drv_motor_disarm(&fixture.motor)) && b_pass;
    b_pass = (DRV_MOTOR_FAULT == drv_motor_arm(&fixture.motor, 23U)) && b_pass;

    return (b_pass);
}

#endif /* Host or APP_TEST: fake backend ONLY. */

/*** end of file ***/

