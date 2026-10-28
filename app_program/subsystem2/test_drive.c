/** @file test_drive.c
 * @brief Synthetic fixtures only; never register IRQs or bind real motors.
 */
#include "test_drive.h"
#if defined(MOTION_CONFIG_HOST_TEST) || APP_TEST
#include "enc_capture.h"
#include "motion_bus.h"
#include "motion_foundation_test.h"
#include "test_motor.h"
#include <stddef.h>
#include <limits.h>
#include <math.h>
#if defined(MOTION_CONFIG_HOST_TEST)
#include <stdio.h>
#else
#define PROHIBIT_DEF_SIZE_T 1
#include "subsystem2.h"
#endif
static motion_config_t test_motion_config (void);
static bool test_motion_capture (void);
static bool test_motion_pair (void);
static bool test_motion_invalid (void);
static bool test_motion_args (void);
static bool test_motion_limit (void);

static bool test_motion_bus (void);
static bool test_motion_cancel (void);
static wheel_loop_config_t test_motion_loop_config (void);
static bool test_motion_prepare (void * p_context, uint16_t period,
    uint32_t clock);
static bool test_motion_apply (void * p_context, drv_motor_wheel_t wheel,
                               uint16_t a, uint16_t b);
static bool test_motion_stop_outputs (void * p_context);
static enc_capture_sample_t test_motion_sample (int64_t left, int64_t right,
    uint64_t now);
static bool test_motion_loop (void);
static bool test_motion_stale (void);
static bool test_motion_reset (void);
static bool test_motion_overspeed (void);
static bool test_motion_loop_bad_config (void);
static bool test_motion_output_error (void);
static bool test_motion_loop_reversal (void);
static bool test_motion_plan_forward (void);
static bool test_motion_plan_reverse (void);
static bool test_motion_plan_profile (void);
static bool test_motion_plan_invalid (void);
static bool test_motion_plan_overflow (void);
static bool test_motion_tick (motion_run_t * p_run, int64_t left,
    int64_t right, uint64_t now);
static move_plan_command_t test_motion_command (move_plan_kind_t kind);
static bool test_motion_run_travel (void);
static bool test_motion_run_reverse (void);
static bool test_motion_run_cancel (void);
static bool test_motion_run_expire (void);
static bool test_motion_run_timeout (void);
static bool test_motion_run_faults (void);
static bool test_motion_run_stop_deadline (void);
static bool test_motion_turn_goals (void);
static bool test_motion_turn_scale (void);
static bool test_motion_turn_overflow (void);
static bool test_motion_turn_completion (void);
static bool test_motion_turn_overshoot (void);
static bool test_motion_encoder_fault (void);
static bool test_motion_bus_inflight (void);
static bool test_motion_sample_time (void);
static bool test_motion_progress_timeout (void);
static bool test_motion_config_copy (void);
static bool test_motion_freshness (void);
static bool test_motion_admission_result (void);
static bool test_motion_durable_fault (void);
static bool test_motion_reversal_timeout (void);
static bool test_motion_end_to_end (void);
static bool test_motion_packed_flags (void);
static bool test_motion_payload_copy (void);
static bool test_motion_payload_invalid (void);
static uint32_t test_drive_run (void);
#if !defined(MOTION_CONFIG_HOST_TEST)
static uint64_t g_test_drive_stack[1024]; /* 8 KiB, synthetic TEST=1 worker. */
static ID g_test_drive_flag;
static uint32_t g_test_drive_result;
static bool g_test_drive_started;
static bool (*g_test_drive_config_tests)(void);
static void test_drive_entry (INT code, void * p_context);
#endif

typedef struct
{
    uint16_t a[2];
    uint16_t b[2];
    uint32_t prepares;
    uint32_t applies;
    uint32_t stops;
    bool b_fail;
} test_motion_io_t;
static bool test_motion_run_ready (motion_run_t * p_run,
    test_motion_io_t * p_fake);
static bool test_motion_ready_config (motion_run_t * p_run,
    test_motion_io_t * p_fake,
                                      wheel_loop_config_t const * p_config);

uint32_t test_drive (bool (*p_config_tests)(void))
{
#if defined(MOTION_CONFIG_HOST_TEST)
    (void)p_config_tests;
    return (test_drive_run());
#else
    uint32_t result = UINT32_MAX;

    if (!g_test_drive_started)
    {
        T_CFLG flag = {0};
        T_CTSK task = {0};
        ID task_id = E_OBJ;

        /* One-shot test worker. Never reuse its stack while it can be alive. */
        g_test_drive_started = true;
        g_test_drive_config_tests = p_config_tests;
        g_test_drive_result = UINT32_MAX;
        g_test_drive_flag = tk_cre_flg(&flag);
        if (g_test_drive_flag > 0)
        {
            task.task = test_drive_entry;
            task.itskpri = 15;
            task.stksz = (SZ)sizeof(g_test_drive_stack);
            task.bufptr = g_test_drive_stack;
            task.tskatr = TA_HLNG | TA_RNG3 | TA_USERBUF;
            task_id = tk_cre_tsk(&task);
            if (task_id > 0)
            {
                ER error = tk_sta_tsk(task_id,0);

                if (E_OK == error)
                {
                    UINT events;

                    error = tk_wai_flg(g_test_drive_flag,1U,TWF_ORW,
                                       &events,30000);
                    if (E_OK == error)
                    {
                        result = g_test_drive_result;
                        (void)tk_del_flg(g_test_drive_flag);
                    }
                    /* Timeout: do not terminate a worker holding log/kernel
                     * locks. Leave its completion flag alive; report FAIL. */
                }
                else
                {
                    (void)tk_del_tsk(task_id);
                    (void)tk_del_flg(g_test_drive_flag);
                }
            }
            else
            {
                (void)tk_del_flg(g_test_drive_flag);
            }
        }
    }
    if (UINT32_MAX == result)
    {
        tm_printf((UB*)"[TEST] S2 motion worker FAIL unavailable/timeout\n");
    }
    return (result);
#endif
}

static uint32_t test_drive_run (void)
{
    uint32_t failed = 0U;
    static bool (*const tests[])(void) = {test_motion_capture, test_motion_pair,
        test_motion_invalid, test_motion_args, test_motion_limit,
        test_motion_bus, test_motion_cancel,
        test_motion_loop, test_motion_stale, test_motion_reset,
        test_motion_overspeed, test_motion_loop_bad_config,
        test_motion_output_error, test_motion_loop_reversal,
        test_motion_plan_forward, test_motion_plan_reverse,
            test_motion_plan_profile,
        test_motion_plan_invalid, test_motion_plan_overflow,
        test_motion_run_travel, test_motion_run_reverse, test_motion_run_cancel,
        test_motion_run_expire, test_motion_run_timeout, test_motion_run_faults,
        test_motion_run_stop_deadline, test_motion_turn_goals,
        test_motion_turn_scale, test_motion_turn_overflow,
        test_motion_turn_completion, test_motion_turn_overshoot,
        test_motion_encoder_fault, test_motion_bus_inflight,
        test_motion_sample_time, test_motion_progress_timeout,
        test_motion_config_copy, test_motion_freshness,
        test_motion_admission_result, test_motion_durable_fault,
        test_motion_reversal_timeout, test_motion_end_to_end,
        test_motion_packed_flags, test_motion_payload_copy,
        test_motion_payload_invalid};

    for (size_t index = 0U; index < sizeof(tests)/sizeof(tests[0]); ++index)
    {
        bool b_pass = tests[index]();
        /* Preserve U57..U97; U98..U140 belong to adapter/lifecycle tests. */
        unsigned int ident = (index < 41U) ? 57U + (unsigned int)index :
            141U + (unsigned int)index - 41U;

        failed += b_pass ? 0U : 1U;
#if defined(MOTION_CONFIG_HOST_TEST)
        printf("[TEST] S2-U%u %s motion integration\n", ident,
               b_pass ? "PASS" : "FAIL");
#else
        tm_printf((UB*)"[TEST] S2-U%u %s motion integration\n", ident,
                  b_pass ? "PASS" : "FAIL");
#endif
    }
    return (failed);
}

#if !defined(MOTION_CONFIG_HOST_TEST)
static void test_drive_entry (INT code, void * p_context)
{
    (void)code;
    (void)p_context;
    g_test_drive_result = ((NULL != g_test_drive_config_tests) &&
        g_test_drive_config_tests()) ? 0U : 1U;
    g_test_drive_result += motion_foundation_test();
    g_test_drive_result += test_motor();
    g_test_drive_result += test_drive_run();
    (void)tk_set_flg(g_test_drive_flag,1U);
    tk_exd_tsk();
}
#endif

static motion_config_t test_motion_config (void)
{
    motion_config_t cfg = {0};

    cfg.left = (motion_config_wheel_t){1000U,100000U,
        MOTION_TYPES_POLARITY_NORMAL,MOTION_TYPES_POLARITY_NORMAL};
    cfg.right = cfg.left;
    cfg.track_width_um = 150000U;
    cfg.max_wheel_speed_mm_s = 500;
    cfg.calibration = MOTION_CONFIG_CALIBRATION_CONFIRMED;
    return (cfg);
}

static bool test_motion_capture (void)
{
    motion_config_t cfg = test_motion_config();
    enc_capture_t capture = {0};
    enc_capture_sample_t sample = {0};
    static uint32_t const levels[] = {0U,1UL<<28U,
        (1UL<<28U)|(1UL<<7U),1UL<<7U,0U};
    bool b_pass = enc_capture_init(&capture, &cfg, 1U);

    b_pass = (enc_capture_feed(&capture,ENC_CAPTURE_MASK,0U)) && b_pass;
    for (size_t index=1U; index<5U; ++index)
    {
        b_pass = (enc_capture_feed(&capture,ENC_CAPTURE_LEFT_MASK,
                                  levels[index])) && b_pass;
    }
    b_pass = (enc_capture_get(&capture,10000U,&sample)) &&
        (4==sample.sample[0].count) && (0==sample.sample[1].count) &&
        (10000U==sample.sample[0].timestamp_us) &&
        (1U==sample.sample[1].generation) && b_pass;
    return (b_pass);
}

static bool test_motion_pair (void)
{
    motion_config_t cfg = test_motion_config();
    enc_capture_t capture = {0};
    enc_capture_sample_t sample = {0};
    bool b_pass;

    cfg.right.encoder_polarity = MOTION_TYPES_POLARITY_REVERSED;
    b_pass = enc_capture_init(&capture,&cfg,2U);
    b_pass = (enc_capture_feed(&capture,ENC_CAPTURE_MASK,0U)) && b_pass;
    b_pass = (enc_capture_feed(&capture,ENC_CAPTURE_MASK,
        (1UL<<28U)|(1UL<<1U))) && b_pass;
    b_pass = (enc_capture_get(&capture,123U,&sample)) &&
        (1==sample.sample[0].count) && (-1==sample.sample[1].count) && b_pass;
    b_pass = (enc_capture_get(&capture,456U,&sample)) &&
        (456U==sample.sample[1].timestamp_us) &&
        (1==sample.sample[0].count) && b_pass;
    return (b_pass);
}

static bool test_motion_invalid (void)
{
    motion_config_t cfg = test_motion_config();
    enc_capture_t capture = {0};
    enc_capture_sample_t sample = {0};
    bool b_pass = enc_capture_init(&capture,&cfg,1U);

    b_pass = (enc_capture_feed(&capture,ENC_CAPTURE_MASK,0U)) && b_pass;
    b_pass = !(enc_capture_feed(&capture,ENC_CAPTURE_LEFT_MASK,
        ENC_CAPTURE_LEFT_MASK)) && b_pass;
    b_pass = !(enc_capture_get(&capture,1U,&sample)) &&
        !sample.b_healthy && (1U==sample.invalid[0]) &&
        (0==sample.sample[0].count) && b_pass;
    b_pass = !(enc_capture_feed(&capture,ENC_CAPTURE_LEFT_MASK,1UL<<7U)) &&
        b_pass;
    return (b_pass);
}

static bool test_motion_args (void)
{
    enc_capture_t capture = {0};
    enc_capture_sample_t sample = {0};
    motion_config_t cfg = test_motion_config();
    bool b_pass = !(enc_capture_init(NULL,&cfg,1U));

    b_pass = !(enc_capture_feed(NULL,0U,0U)) &&
        !(enc_capture_get(NULL,0U,&sample)) && !sample.b_healthy && b_pass;
    b_pass = !(enc_capture_init(&capture,&cfg,0U)) &&
        !(enc_capture_get(&capture,0U,&sample)) && b_pass;
    cfg.calibration = MOTION_CONFIG_CALIBRATION_UNVERIFIED;
    b_pass = !(enc_capture_init(&capture,&cfg,1U)) && b_pass;
    return (b_pass);
}

static bool test_motion_limit (void)
{
    enc_capture_t capture = {0};
    motion_config_t cfg = test_motion_config();
    bool b_pass = enc_capture_init(&capture,&cfg,1U);

    b_pass = (enc_capture_feed(&capture,ENC_CAPTURE_MASK,0U)) && b_pass;
    capture.decoder[0].count=INT64_MAX;
    b_pass = !(enc_capture_feed(&capture,ENC_CAPTURE_LEFT_MASK,1UL<<28U)) &&
        capture.decoder[0].b_count_limit &&
        (INT64_MAX==capture.decoder[0].count) && b_pass;
    return (b_pass);
}

static bool test_motion_bus (void)
{
    motion_bus_t bus = {0};
    motion_bus_request_t request = {0};
    motion_bus_request_t copy = {0};
    bool b_present=false, b_stop=false, b_cancel=false;
    bool b_pass = motion_bus_put(&bus,&request);

    request.payload.configure.config.geometry.track_width_um=999U;
    b_pass = !(motion_bus_put(&bus,&request)) && b_pass;
    motion_bus_take(&bus,&copy,&b_present,&b_stop,&b_cancel);
    b_pass = b_present && !b_stop &&
        (0U==copy.payload.configure.config.geometry.track_width_um) && b_pass;
    bus.b_inflight=false;
    bus.status.state=MOTION_RUN_READY;
    request.kind=MOTION_BUS_COMMAND;
    request.payload.command.id=1U;
    b_pass = (motion_bus_put(&bus,&request)) && b_pass;
    bus.b_pending=false;
    bus.status.terminal_id=1U;
    b_pass = !(motion_bus_put(&bus,&request)) && b_pass;
    request.kind=MOTION_BUS_ACK; request.payload.ack_id=1U;
    b_pass = (motion_bus_put(&bus,&request)) && b_pass;
    return (b_pass);
}

static bool test_motion_cancel (void)
{
    motion_bus_t bus = {0};
    motion_bus_request_t request = {0};
    bool b_present=false, b_stop=false, b_cancel=false;
    bool b_pass = !(motion_bus_stop(&bus,true,99U));

    bus.b_inflight=false;
    bus.status.state=MOTION_RUN_READY;
    request.kind=MOTION_BUS_COMMAND;request.payload.command.id=7U;
    b_pass = (motion_bus_put(&bus,&request)) && b_pass;
    b_pass = (motion_bus_stop(&bus,true,7U)) && b_pass;
    b_pass = !(motion_bus_put(&bus,&request)) && b_pass;
    motion_bus_take(&bus,&request,&b_present,&b_stop,&b_cancel);
    b_pass = b_present && b_stop && b_cancel && !bus.b_pending && b_pass;
    b_pass = (motion_bus_stop(&bus,false,0U)) && b_pass;
    return (b_pass);
}

static wheel_loop_config_t test_motion_loop_config (void)
{
    wheel_loop_config_t cfg = {0};

    cfg.geometry = test_motion_config();
    cfg.motor = (drv_motor_config_t){125000000U,20000U,100U,1.0F,
        {MOTION_TYPES_POLARITY_NORMAL,MOTION_TYPES_POLARITY_NORMAL},
        DRV_MOTOR_COAST,MOTION_CONFIG_CALIBRATION_CONFIRMED,true};
    cfg.pid[0] = (motion_pid_config_t){0.01F,0.0F,0.0F,-1.0F,1.0F,
        -0.5F,0.5F,100000U,MOTION_CONFIG_CALIBRATION_CONFIRMED};
    cfg.pid[1] = cfg.pid[0];
    cfg.profile = (move_plan_config_t){1000U,1000U,1000U,{1000000U,
        1000000U},1.0F,100};
    cfg.period_us = 10000U;
    cfg.sample_limit_us = 100000U;
    cfg.stop_dwell_us = 20000U;
    cfg.stop_timeout_us = 1000000U;
    cfg.progress_timeout_us = 2000000U;
    cfg.stop_speed_mm_s = 0;
    cfg.overspeed_mm_s = 1000;
    return (cfg);
}

static bool test_motion_prepare (void * p_context, uint16_t period,
    uint32_t clock)
{
    test_motion_io_t * p_io = p_context;

    ++p_io->prepares;
    return ((period > 0U) && (125000000U == clock) && !p_io->b_fail);
}

static bool test_motion_apply (void * p_context, drv_motor_wheel_t wheel,
                               uint16_t a, uint16_t b)
{
    test_motion_io_t * p_io = p_context;

    ++p_io->applies;
    p_io->a[wheel] = a;
    p_io->b[wheel] = b;
    return (!p_io->b_fail);
}

static bool test_motion_stop_outputs (void * p_context)
{
    test_motion_io_t * p_io = p_context;

    ++p_io->stops;
    p_io->a[0] = 0U; p_io->a[1] = 0U;
    p_io->b[0] = 0U; p_io->b[1] = 0U;
    return (!p_io->b_fail);
}

static enc_capture_sample_t test_motion_sample (int64_t left, int64_t right,
    uint64_t now)
{
    enc_capture_sample_t sample = {0};

    sample.sample[0] = (motion_estimator_sample_t){left,now,1U};
    sample.sample[1] = (motion_estimator_sample_t){right,now,1U};
    sample.b_healthy = true;
    return (sample);
}

static bool test_motion_loop (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    enc_capture_sample_t sample = test_motion_sample(0,0,0U);
    int32_t target[2] = {100,-100};
    bool b_pass = wheel_loop_init(&loop,&cfg,&io) && (0U == fake.prepares);

    b_pass = wheel_loop_enable(&loop,0U) && (1U == fake.prepares) && b_pass;
    b_pass = wheel_loop_measure(&loop,&sample,0U) && !loop.b_valid && b_pass;
    sample = test_motion_sample(1,-2,10000U);
    b_pass = wheel_loop_measure(&loop,&sample,10000U) && loop.b_valid &&
        (10 == loop.measured[0].speed_mm_s) && (
            -20 == loop.measured[1].speed_mm_s) &&
        (100 == loop.measured[0].position_um) && (
            -200 == loop.measured[1].position_um) && b_pass;
    b_pass = wheel_loop_drive(&loop,target,10000U,10000U) &&
        (10000 == loop.velocity_um_s[0]) && (
            -10000 == loop.velocity_um_s[1]) && b_pass;
    b_pass = wheel_loop_brake(&loop,10000U) && (0U == fake.a[0]) &&
        (0U == fake.b[0]) && (0U == fake.a[1]) && (0U == fake.b[1]) && b_pass;
    return (b_pass);
}

static bool test_motion_stale (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    enc_capture_sample_t sample = test_motion_sample(0,0,0U);
    bool b_pass = wheel_loop_init(&loop,&cfg,&io) && wheel_loop_enable(
        &loop,0U);

    b_pass = wheel_loop_measure(&loop,&sample,0U) && b_pass;
    sample = test_motion_sample(0,0,10000U);
    b_pass = !wheel_loop_measure(&loop,&sample,110001U) && loop.b_fault &&
        !loop.b_enabled && (fake.stops > 0U) && b_pass;
    return (b_pass);
}

static bool test_motion_reset (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    enc_capture_sample_t sample = test_motion_sample(0,0,0U);
    bool b_pass = wheel_loop_init(&loop,&cfg,&io) && wheel_loop_enable(
        &loop,0U);

    b_pass = wheel_loop_measure(&loop,&sample,0U) && b_pass;
    sample = test_motion_sample(0,0,10000U); sample.sample[1].generation = 2U;
    b_pass = !wheel_loop_measure(&loop,&sample,10000U) && loop.b_fault
        && b_pass;
    b_pass = !wheel_loop_measure(NULL,&sample,10000U) &&
        !wheel_loop_drive(NULL,NULL,0U,0U) && !wheel_loop_brake(NULL,0U)
            && b_pass;
    return (b_pass);
}

static bool test_motion_overspeed (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    enc_capture_sample_t sample = test_motion_sample(0,0,0U);
    bool b_pass = wheel_loop_init(&loop,&cfg,&io) && wheel_loop_enable(
        &loop,0U);

    b_pass = wheel_loop_measure(&loop,&sample,0U) && b_pass;
    sample = test_motion_sample(-101,0,10000U);
    b_pass = !wheel_loop_measure(&loop,&sample,10000U) && loop.b_fault
        && b_pass;
    return (b_pass);
}

static bool test_motion_loop_bad_config (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    bool b_pass = wheel_loop_validate(&cfg) && !wheel_loop_validate(NULL);

    cfg.motor.polarity[1] = MOTION_TYPES_POLARITY_REVERSED;
    b_pass = !wheel_loop_validate(&cfg) && b_pass;
    cfg = test_motion_loop_config(); cfg.profile.correction_gain = NAN;
    b_pass = !wheel_loop_validate(&cfg) && b_pass;
    cfg = test_motion_loop_config(); cfg.period_us = 0U;
    b_pass = !wheel_loop_validate(&cfg) && b_pass;
    cfg = test_motion_loop_config(); cfg.pid[0].max_elapsed_us = 99999U;
    b_pass = !wheel_loop_validate(&cfg) && b_pass;
    return (b_pass);
}

static bool test_motion_output_error (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    enc_capture_sample_t sample = test_motion_sample(0,0,0U);
    int32_t target[2] = {100,100};
    bool b_pass = wheel_loop_init(&loop,&cfg,&io) && wheel_loop_enable(
        &loop,0U);

    b_pass = wheel_loop_measure(&loop,&sample,0U) && b_pass;
    sample = test_motion_sample(0,0,10000U);
    b_pass = wheel_loop_measure(&loop,&sample,10000U) && b_pass;
    fake.b_fail = true;
    b_pass = !wheel_loop_drive(&loop,target,10000U,10000U) && loop.b_fault &&
        !loop.motor.b_stop_confirmed && (fake.stops > 0U) && b_pass;
    return (b_pass);
}

static bool test_motion_loop_reversal (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    enc_capture_sample_t sample = test_motion_sample(0,0,0U);
    int32_t target[2] = {100,-100};
    bool b_pass = wheel_loop_init(&loop,&cfg,&io) && wheel_loop_enable(
        &loop,0U);

    b_pass = wheel_loop_measure(&loop,&sample,0U) && b_pass;
    sample = test_motion_sample(0,0,10000U);
    b_pass = wheel_loop_measure(&loop,&sample,10000U) &&
        wheel_loop_drive(&loop,target,10000U,10000U) &&
        (DRV_MOTOR_RUNNING == loop.motor.channel[0].state) && b_pass;
    target[0] = -100; target[1] = 100;
    sample = test_motion_sample(1,-1,20000U);
    b_pass = wheel_loop_measure(&loop,&sample,20000U) &&
        wheel_loop_drive(&loop,target,10000U,20000U) &&
        (0 == loop.velocity_um_s[0]) && (0U == fake.a[0]) && (
            0U == fake.b[0]) && b_pass;
    sample = test_motion_sample(1,-1,30000U);
    b_pass = wheel_loop_measure(&loop,&sample,30000U) &&
        wheel_loop_drive(&loop,target,10000U,30000U) &&
        (DRV_MOTOR_REVERSING == loop.motor.channel[0].state) &&
        (DRV_MOTOR_REVERSING == loop.motor.channel[1].state) && b_pass;
    sample = test_motion_sample(1,-1,40000U);
    b_pass = wheel_loop_measure(&loop,&sample,40000U) &&
        wheel_loop_drive(&loop,target,10000U,40000U) &&
        (DRV_MOTOR_REVERSING == loop.motor.channel[0].state) && b_pass;
    sample = test_motion_sample(1,-1,50000U);
    b_pass = wheel_loop_measure(&loop,&sample,50000U) &&
        wheel_loop_drive(&loop,target,10000U,50000U) &&
        (DRV_MOTOR_RUNNING == loop.motor.channel[0].state) &&
        (DRV_MOTOR_RUNNING == loop.motor.channel[1].state) &&
        (0U == fake.a[0]) && (fake.b[0] > 0U) &&
        (fake.a[1] > 0U) && (0U == fake.b[1]) && b_pass;
    return (b_pass);
}

static move_plan_command_t test_motion_command (move_plan_kind_t kind)
{
    move_plan_command_t command = {0};

    command.id = 1U;
    command.kind = kind;
    command.issued_us = 20000U;
    command.lifetime_us = 500000U;
    command.left_mm_s = 100;
    command.right_mm_s = 100;
    command.distance_um = 10000;
    command.cruise_mm_s = 100;
    return (command);
}

static bool test_motion_plan_forward (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    move_plan_t plan = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    int64_t position[2] = {1000,-2000};
    int32_t speed[2] = {0,0};
    bool b_arrived = false;
    bool b_pass = move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U);

    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        (100 == speed[0]) && (100 == speed[1]) && !b_arrived && b_pass;
    position[0] = 10000; position[1] = 7000;
    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        b_arrived && (0 == speed[0]) && (0 == speed[1]) && b_pass;
    position[1] = 6000;
    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        !b_arrived && (0 == speed[0]) && (63 == speed[1]) && b_pass;
    return (b_pass);
}

static bool test_motion_plan_reverse (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    move_plan_t plan = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    int64_t position[2] = {-1000,2000};
    int32_t speed[2] = {0,0};
    bool b_arrived = false;
    bool b_pass;

    command.distance_um = -10000;
    b_pass = move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U);
    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        (-100 == speed[0]) && (-100 == speed[1]) && b_pass;
    position[0] = -10000; position[1] = -7000;
    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived)
        && b_arrived && b_pass;
    position[0] = 1;
    b_pass = !move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        !b_arrived && (0 == speed[0]) && b_pass;
    return (b_pass);
}

static bool test_motion_plan_profile (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    move_plan_t plan = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    int64_t position[2] = {0,0};
    int32_t speed[2] = {0,0};
    bool b_arrived = false;
    bool b_pass;

    command.distance_um = 100000; command.cruise_mm_s = 200;
    b_pass = move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U);
    position[0] = 50000; position[1] = 40000;
    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        (190 == speed[0]) && (200 == speed[1]) && !b_arrived && b_pass;
    position[0] = 98000; position[1] = 98000;
    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        (63 == speed[0]) && (63 == speed[1]) && b_pass;
    position[0] = 101001;
    b_pass = !move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived)
        && b_pass;
    return (b_pass);
}

static bool test_motion_plan_invalid (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    move_plan_t plan = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    int64_t position[2] = {0,0};
    int32_t speed[2] = {123,456};
    bool b_arrived = true;
    bool b_pass = !move_plan_begin(NULL,&command,&cfg.profile,&cfg.geometry,
        position,20000U);

    b_pass = !move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        (0 == speed[0]) && (0 == speed[1]) && !b_arrived && b_pass;

    command.kind = (move_plan_kind_t)255;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) && b_pass;
    command = test_motion_command(MOVE_PLAN_TRAVEL); command.distance_um = 1000;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) && b_pass;
    command = test_motion_command(
        MOVE_PLAN_SPEED); command.left_mm_s = INT32_MIN;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) && b_pass;
    command = test_motion_command(MOVE_PLAN_SPEED); command.issued_us = 20001U;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) && b_pass;
    command = test_motion_command(MOVE_PLAN_SPEED); command.lifetime_us = 0U;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) && b_pass;
    return (b_pass);
}

static bool test_motion_plan_overflow (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    move_plan_t plan = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    int64_t position[2] = {INT64_MIN,0};
    int32_t speed[2] = {0,0};
    bool b_arrived = false;
    bool b_pass = move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U);

    position[0] = INT64_MAX;
    b_pass = !move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived)
        && b_pass;
    command.issued_us = UINT64_MAX-1U;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,UINT64_MAX) && b_pass;
    command = test_motion_command(
        MOVE_PLAN_TRAVEL); command.distance_um = INT32_MIN;
    position[0] = 0; position[1] = 0;
    b_pass = move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) && b_pass;
    cfg.profile.decel_mm_s2 = INT32_MAX;
    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        (-100 == speed[0]) && b_pass;
    return (b_pass);
}

static bool test_motion_tick (motion_run_t * p_run, int64_t left,
    int64_t right, uint64_t now)
{
    enc_capture_sample_t sample = test_motion_sample(left,right,now);

    return (motion_run_tick(p_run,&sample,now));
}

static bool test_motion_run_ready (motion_run_t * p_run,
    test_motion_io_t * p_fake)
{
    wheel_loop_config_t cfg = test_motion_loop_config();

    return (test_motion_ready_config(p_run,p_fake,&cfg));
}

static bool test_motion_ready_config (motion_run_t * p_run,
    test_motion_io_t * p_fake,
                                      wheel_loop_config_t const * p_config)
{
    drv_motor_io_t io = {p_fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    bool b_pass = motion_run_init(p_run,p_config,&io) &&
        (MOTION_RUN_CONFIGURED == p_run->status.state) && (
            0U == p_fake->prepares);

    b_pass = motion_run_enable(p_run,0U) && test_motion_tick(p_run,0,0,0U) &&
        (MOTION_RUN_SETTLING == p_run->status.state) && b_pass;
    b_pass = test_motion_tick(p_run,0,0,10000U) &&
        (MOTION_RUN_SETTLING == p_run->status.state) && b_pass;
    b_pass = test_motion_tick(p_run,0,0,20000U) &&
        (MOTION_RUN_READY == p_run->status.state)
            && p_run->status.b_standstill && b_pass;
    return (b_pass);
}

static bool test_motion_run_travel (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    bool b_pass = test_motion_run_ready(&run,&fake);
    uint32_t calls = fake.applies;

    b_pass = motion_run_command(&run,&command,20000U) && (
        calls == fake.applies) && b_pass;
    b_pass = test_motion_tick(&run,0,0,30000U) &&
        (10 == run.status.target_mm_s[0]) && b_pass;
    b_pass = test_motion_tick(&run,90,90,40000U) &&
        (MOTION_RUN_STOPPING == run.status.state) && (
            0U == run.status.terminal_id) && b_pass;
    b_pass = test_motion_tick(&run,90,90,50000U) &&
        test_motion_tick(&run,90,90,60000U) && (
            0U == run.status.terminal_id) && b_pass;
    b_pass = test_motion_tick(&run,90,90,70000U) &&
        (MOTION_RUN_READY == run.status.state) && (
            1U == run.status.terminal_id) &&
        (MOTION_RUN_DONE == run.status.terminal_result)
            && run.status.b_standstill && b_pass;
    command.id = 2U; command.issued_us = 70000U;
    b_pass = !motion_run_command(&run,&command,70000U) &&
        !motion_run_ack(&run,2U) && motion_run_ack(&run,1U) &&
        motion_run_command(&run,&command,70000U) && b_pass;
    return (b_pass);
}

static bool test_motion_run_reverse (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    bool b_pass = test_motion_run_ready(&run,&fake);

    command.distance_um = -10000;
    b_pass = motion_run_command(&run,&command,20000U) &&
        test_motion_tick(&run,0,0,30000U) && (
            -10 == run.status.target_mm_s[1]) && b_pass;
    b_pass = test_motion_tick(&run,-90,-90,40000U) &&
        test_motion_tick(&run,-90,-90,50000U) &&
        test_motion_tick(&run,-90,-90,60000U) &&
        test_motion_tick(&run,-90,-90,70000U) &&
        (MOTION_RUN_DONE == run.status.terminal_result) &&
        (-9000 == run.status.position_um[0]) && b_pass;
    return (b_pass);
}

static bool test_motion_run_cancel (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    bool b_pass = test_motion_run_ready(&run,&fake);

    b_pass = motion_run_command(&run,&command,20000U) &&
        test_motion_tick(&run,0,0,30000U) && b_pass;
    motion_run_stop(&run,true,30000U);
    b_pass = (MOTION_RUN_STOPPING == run.status.state) &&
        (0U == fake.a[0]) && (0U == fake.b[0]) && b_pass;
    b_pass = test_motion_tick(&run,0,0,40000U) &&
        (MOTION_RUN_CANCELLED == run.status.terminal_result) &&
        (1U == run.status.terminal_id) && b_pass;
    b_pass = motion_run_ack(&run,1U) && !motion_run_command(&run,&command,
        40000U) && b_pass;
    return (b_pass);
}

static bool test_motion_run_expire (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_SPEED);
    bool b_pass = test_motion_run_ready(&run,&fake);

    command.lifetime_us = 30000U;
    b_pass = motion_run_command(&run,&command,20000U) &&
        test_motion_tick(&run,0,0,30000U) && b_pass;
    command.issued_us = 30000U;
    b_pass = motion_run_command(&run,&command,30000U) &&
        !motion_run_command(&run,&command,30000U) &&
        test_motion_tick(&run,0,0,50000U) && (
            MOTION_RUN_ACTIVE == run.status.state) && b_pass;
    b_pass = test_motion_tick(&run,0,0,60000U) &&
        (MOTION_RUN_EXPIRED == run.status.terminal_result) &&
        (MOTION_RUN_READY == run.status.state) && (0U == fake.a[1]) && b_pass;
    return (b_pass);
}

static bool test_motion_run_timeout (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_TRAVEL);
    bool b_pass = test_motion_run_ready(&run,&fake);

    command.lifetime_us = 10000U;
    b_pass = motion_run_command(&run,&command,20000U) &&
        !test_motion_tick(&run,0,0,30000U) &&
        (MOTION_RUN_FAULT == run.status.state) &&
        (MOTION_RUN_TIMEOUT == run.status.terminal_result) &&
        !run.loop.b_enabled && (fake.stops > 0U) && b_pass;
    return (b_pass);
}

static bool test_motion_run_faults (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_SPEED);
    bool b_pass = test_motion_run_ready(&run,&fake);

    b_pass = motion_run_command(&run,&command,20000U) &&
        test_motion_tick(&run,0,0,30000U) && b_pass;
    motion_run_emergency(&run,MOTION_RUN_EMERGENCY,30000U);
    b_pass = (MOTION_RUN_FAULT == run.status.state) &&
        (MOTION_RUN_EMERGENCY == run.status.terminal_result) &&
        (1U == run.status.terminal_id) && !run.status.b_measurement_valid &&
        !run.loop.b_enabled && (0U == fake.a[0]) && b_pass;
    b_pass = motion_run_ack(&run,1U) && !motion_run_enable(&run,40000U) &&
        !motion_run_command(&run,&command,40000U) && b_pass;
    motion_run_emergency(NULL,MOTION_RUN_EMERGENCY,0U);
    b_pass = !motion_run_tick(NULL,NULL,0U) && !motion_run_ack(NULL,1U)
        && b_pass;
    return (b_pass);
}

static bool test_motion_run_stop_deadline (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    bool b_pass;

    cfg.stop_timeout_us = 30000U;
    b_pass = motion_run_init(&run,&cfg,&io) && motion_run_enable(&run,0U) &&
        test_motion_tick(&run,0,0,0U) && test_motion_tick(&run,1,1,10000U);
    motion_run_stop(&run,false,10000U);
    motion_run_stop(&run,false,20000U);
    b_pass = (10000U == run.stopping_since_us) &&
        test_motion_tick(&run,2,2,20000U) && test_motion_tick(&run,3,3,
            30000U) &&
        !test_motion_tick(&run,4,4,40000U) &&
        (MOTION_RUN_TIMEOUT == run.status.terminal_result) && b_pass;
    return (b_pass);
}

static bool test_motion_turn_goals (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    move_plan_t plan = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_LEFT);
    int64_t position[2] = {0,0};
    int32_t speed[2] = {0,0};
    bool b_arrived = false;
    bool b_pass = true;
    static move_plan_kind_t const kinds[4] = {
        MOVE_PLAN_LEFT,MOVE_PLAN_RIGHT,MOVE_PLAN_U_LEFT,MOVE_PLAN_U_RIGHT};

    for (uint32_t index = 0U; index < 4U; ++index)
    {
        int32_t arc = (index < 2U) ? 117809 : 235619;
        int32_t sign = (0U == (index%2U)) ? -1 : 1;

        command.kind = kinds[index];
        b_pass = move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
            position,20000U) &&
            (sign*arc == plan.goal_um[0]) && (-sign*arc == plan.goal_um[1]) &&
            move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
            (sign*100 == speed[0]) && (-sign*100 == speed[1])
                && !b_arrived && b_pass;
    }
    return (b_pass);
}

static bool test_motion_turn_scale (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    move_plan_t plan = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_LEFT);
    int64_t position[2] = {0,0};
    int32_t speed[2] = {0,0};
    bool b_arrived = false;
    bool b_pass;

    cfg.profile.turn_scale_ppm[1] = 1100000U;
    b_pass = move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) &&
        (-117809 == plan.goal_um[0]) && (129590 == plan.goal_um[1]);
    position[0] = -58904; position[1] = 32397;
    b_pass = move_plan_step(&plan,&cfg.profile,position,speed,&b_arrived) &&
        (speed[0] < 0) && (speed[0] > -100) && (100 == speed[1])
            && !b_arrived && b_pass;
    return (b_pass);
}

static bool test_motion_turn_overflow (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    move_plan_t plan = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_U_RIGHT);
    int64_t position[2] = {0,0};
    bool b_pass;

    cfg.geometry.track_width_um = UINT32_MAX;
    cfg.profile.turn_scale_ppm[0] = UINT32_MAX;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U);
    cfg = test_motion_loop_config(); cfg.profile.turn_scale_ppm[1] = 0U;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) && b_pass;
    cfg = test_motion_loop_config(); cfg.geometry.track_width_um = 1U;
    b_pass = !move_plan_begin(&plan,&command,&cfg.profile,&cfg.geometry,
        position,20000U) && b_pass;
    return (b_pass);
}

static bool test_motion_turn_completion (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_LEFT);
    wheel_loop_config_t cfg = test_motion_loop_config();
    bool b_pass;

    /* Synthetic trajectory, not a motor/plant model: raise fault threshold
     * explicitly to exercise turn state transitions with short test traces. */
    cfg.overspeed_mm_s = 20000;
    b_pass = test_motion_ready_config(&run,&fake,&cfg);
    b_pass = motion_run_command(&run,&command,20000U) &&
        test_motion_tick(&run,0,0,30000U) &&
        (-10 == run.status.target_mm_s[0]) && (
            10 == run.status.target_mm_s[1]) && b_pass;
    b_pass = test_motion_tick(&run,-1170,1000,40000U) &&
        (MOTION_RUN_ACTIVE == run.status.state) && (
            0U == run.status.terminal_id) && b_pass;
    b_pass = test_motion_tick(&run,-1170,1170,50000U) &&
        (MOTION_RUN_STOPPING == run.status.state) && b_pass;
    b_pass = test_motion_tick(&run,-1170,1170,60000U) &&
        test_motion_tick(&run,-1170,1170,70000U) && (
            0U == run.status.terminal_id) &&
        test_motion_tick(&run,-1170,1170,80000U) &&
        (MOTION_RUN_DONE == run.status.terminal_result) &&
        (MOTION_RUN_READY == run.status.state) && b_pass;
    return (b_pass);
}

static bool test_motion_turn_overshoot (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_RIGHT);
    wheel_loop_config_t cfg = test_motion_loop_config();
    bool b_pass;

    cfg.overspeed_mm_s = 20000;
    b_pass = test_motion_ready_config(&run,&fake,&cfg);
    b_pass = motion_run_command(&run,&command,20000U) &&
        test_motion_tick(&run,1170,-1170,30000U) &&
        (MOTION_RUN_STOPPING == run.status.state) && b_pass;
    b_pass = test_motion_tick(&run,1190,-1170,40000U) &&
        test_motion_tick(&run,1190,-1170,50000U) &&
        test_motion_tick(&run,1190,-1170,60000U) &&
        !test_motion_tick(&run,1190,-1170,70000U) &&
        (MOTION_RUN_OVERSHOOT == run.status.terminal_result) && b_pass;
    return (b_pass);
}

static bool test_motion_encoder_fault (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_SPEED);
    enc_capture_sample_t sample = test_motion_sample(0,0,30000U);
    bool b_pass = test_motion_run_ready(&run,&fake);

    b_pass = motion_run_command(&run,&command,20000U) && b_pass;
    sample.b_healthy = false;
    sample.sample[0].timestamp_us = 20000U;
    sample.sample[1].timestamp_us = 20000U;
    b_pass = !motion_run_tick(&run,&sample,20000U) &&
        (MOTION_RUN_SENSOR == run.status.terminal_result) &&
        !run.loop.b_enabled && (fake.stops > 0U) && b_pass;
    return (b_pass);
}

static bool test_motion_bus_inflight (void)
{
    motion_bus_t bus = {0};
    motion_bus_request_t request = {0};
    bool b_present = false, b_stop = false, b_cancel = false;
    bool b_pass = motion_bus_put(&bus,&request);

    motion_bus_take(&bus,&request,&b_present,&b_stop,&b_cancel);
    b_pass = b_present && bus.b_inflight && !motion_bus_put(&bus,&request)
        && b_pass;
    motion_bus_take(&bus,&request,&b_present,&b_stop,&b_cancel);
    b_pass = !b_present && bus.b_inflight && !motion_bus_put(&bus,
        &request) && b_pass;
    bus.b_inflight = false; /* Only successful owner publication clears it. */
    b_pass = motion_bus_put(&bus,&request) && b_pass;
    return (b_pass);
}

static bool test_motion_sample_time (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    bool b_pass = test_motion_run_ready(&run,&fake);
    uint32_t calls = fake.applies;

    b_pass = test_motion_tick(&run,0,0,20000U) && (calls == fake.applies) &&
        test_motion_tick(&run,0,0,29999U) && (
            20000U == run.loop.last_tick_us) &&
        test_motion_tick(&run,0,0,30000U) && (
            30000U == run.loop.last_tick_us) && b_pass;
    b_pass = !test_motion_tick(&run,0,0,29999U) &&
        (MOTION_RUN_FAULT == run.status.state) && b_pass;
    /* ACTIVE refreshes must neither run PID early nor move its deadline. */
    fake = (test_motion_io_t){0};
    b_pass = test_motion_run_ready(&run,&fake) && b_pass;
    move_plan_command_t command = test_motion_command(MOVE_PLAN_SPEED);

    b_pass = motion_run_command(&run,&command,20000U) && b_pass;
    calls = fake.applies;
    for (uint64_t now = 21000U; now < 30000U; now += 1000U)
    {
        command.issued_us = now;
        b_pass = motion_run_command(&run,&command,now) &&
            test_motion_tick(&run,0,0,now) &&
            (20000U == run.loop.last_tick_us) &&
            (calls == fake.applies) && b_pass;
    }
    b_pass = test_motion_tick(&run,0,0,30000U) &&
        (30000U == run.loop.last_tick_us) &&
        (fake.applies == calls + 2U) && b_pass;
    calls = fake.applies;
    b_pass = test_motion_tick(&run,0,0,30000U) &&
        (fake.applies == calls) && b_pass;
    return (b_pass);
}

static bool test_motion_progress_timeout (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    enc_capture_sample_t sample = test_motion_sample(0,0,0U);
    int32_t target[2] = {100,100};
    bool b_pass;

    cfg.progress_timeout_us = 120000U;
    b_pass = wheel_loop_init(&loop,&cfg,&io) && wheel_loop_enable(&loop,0U) &&
        wheel_loop_measure(&loop,&sample,0U);
    for (uint64_t now = 10000U; now <= 120000U; now += 10000U)
    {
        sample = test_motion_sample(0,0,now);
        b_pass = wheel_loop_measure(&loop,&sample,now) &&
            wheel_loop_drive(&loop,target,10000U,now) && b_pass;
    }
    sample = test_motion_sample(0,0,130000U);
    b_pass = wheel_loop_measure(&loop,&sample,130000U) &&
        !wheel_loop_drive(&loop,target,10000U,130000U) && loop.b_fault &&
        !loop.b_enabled && (fake.stops > 0U) && b_pass;
    return (b_pass);
}

static bool test_motion_config_copy (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
        test_motion_stop_outputs};
    bool b_pass = wheel_loop_init(&loop,&cfg,&io);

    cfg.geometry.left.wheel_circumference_um = 1U;
    cfg.pid[0].proportional_gain = 123.0F;
    cfg.profile.tolerance_um = 2U;
    b_pass = (100000U == loop.config.geometry.left.wheel_circumference_um) &&
        (100000U == loop.estimator[0].circumference_um) &&
        (0.01F == loop.pid[0].config.proportional_gain) &&
        (1000U == loop.config.profile.tolerance_um) && (
            0U == fake.prepares) && b_pass;
    return (b_pass);
}

static bool test_motion_freshness (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_SPEED);
    bool b_pass = test_motion_run_ready(&run,&fake);

    /* Publication of newer status must not refresh old feedback. */
    run.status.timestamp_us = 120001U;
    command.issued_us = 120001U;
    b_pass = !motion_run_command(&run,&command,120001U) &&
        !run.status.b_request_accepted && (
            20000U == run.status.measurement_timestamp_us) && b_pass;
    b_pass = !wheel_loop_drive(&run.loop,
        (int32_t const[2]){100,100},10000U,120001U) && run.loop.b_fault
            && b_pass;
    return (b_pass);
}

static bool test_motion_admission_result (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_SPEED);
    bool b_pass = test_motion_run_ready(&run,&fake);

    b_pass = motion_run_command(&run,&command,20000U) &&
        run.status.b_request_accepted && (1U == run.status.last_request_id) &&
        (20000U == run.status.last_request_issued_us) && b_pass;
    command.issued_us = 30000U; command.left_mm_s = 501;
    b_pass = !motion_run_command(&run,&command,30000U) &&
        !run.status.b_request_accepted && (
            30000U == run.status.last_request_issued_us) &&
        (20000U == run.plan.command.issued_us) && (
            1U == run.status.active_id) && b_pass;
    motion_run_stop(&run,false,30000U);
    b_pass = run.status.b_stop_command_confirmed && b_pass;
    return (b_pass);
}

static bool test_motion_durable_fault (void)
{
    motion_run_t run = {0};
    test_motion_io_t fake = {0};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_SPEED);
    bool b_pass = test_motion_run_ready(&run,&fake);

    b_pass = motion_run_command(&run,&command,20000U) && b_pass;
    motion_run_stop(&run,true,20000U);
    b_pass = test_motion_tick(&run,0,0,30000U) &&
        (MOTION_RUN_CANCELLED == run.status.terminal_result) && b_pass;
    motion_run_emergency(&run,MOTION_RUN_EMERGENCY,30000U);
    b_pass = (MOTION_RUN_CANCELLED == run.status.terminal_result) &&
        (1U == run.status.terminal_id) &&
        (MOTION_RUN_EMERGENCY == run.status.fault_result) &&
        (MOTION_RUN_FAULT == run.status.state) && b_pass;
    b_pass = motion_run_ack(&run,1U) &&
        (MOTION_RUN_EMERGENCY == run.status.fault_result) && b_pass;
    return (b_pass);
}

static bool test_motion_reversal_timeout (void)
{
    wheel_loop_t loop = {0};
    wheel_loop_config_t cfg = test_motion_loop_config();
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
                         test_motion_stop_outputs};
    enc_capture_sample_t sample = test_motion_sample(0,0,0U);
    int32_t target[2] = {100,100};
    bool b_pass;

    cfg.stop_timeout_us = 30000U;
    b_pass = wheel_loop_init(&loop,&cfg,&io) &&
        wheel_loop_enable(&loop,0U) && wheel_loop_measure(&loop,&sample,0U);
    sample = test_motion_sample(0,0,10000U);
    b_pass = wheel_loop_measure(&loop,&sample,10000U) &&
        wheel_loop_drive(&loop,target,10000U,10000U) && b_pass;
    target[0] = -100; target[1] = -100;
    for (uint64_t now = 20000U; now < 50000U; now += 10000U)
    {
        sample = test_motion_sample((int64_t)(now/10000U),0,now);
        b_pass = wheel_loop_measure(&loop,&sample,now) &&
            wheel_loop_drive(&loop,target,10000U,now) && b_pass;
    }
    sample = test_motion_sample(5,0,50000U);
    b_pass = wheel_loop_measure(&loop,&sample,50000U) &&
        !wheel_loop_drive(&loop,target,10000U,50000U) &&
        loop.b_fault && !loop.b_enabled && (fake.stops > 0U) && b_pass;
    return (b_pass);
}

static bool test_motion_end_to_end (void)
{
    wheel_loop_config_t cfg = test_motion_loop_config();
    motion_run_t run = {0};
    enc_capture_t capture = {0};
    enc_capture_sample_t sample = {0};
    test_motion_io_t fake = {0};
    drv_motor_io_t io = {&fake,test_motion_prepare,test_motion_apply,
                         test_motion_stop_outputs};
    move_plan_command_t command = test_motion_command(MOVE_PLAN_SPEED);
    bool b_pass = enc_capture_init(&capture,&cfg.geometry,1U) &&
        enc_capture_feed(&capture,ENC_CAPTURE_MASK,0U) &&
        motion_run_init(&run,&cfg,&io) && motion_run_enable(&run,0U);

    for (uint64_t now = 0U; now <= 20000U; now += 10000U)
    {
        b_pass = enc_capture_get(&capture,now,&sample) &&
            motion_run_tick(&run,&sample,now) && b_pass;
    }
    b_pass = motion_run_command(&run,&command,20000U) &&
        enc_capture_feed(&capture,ENC_CAPTURE_MASK,
                         (1UL<<28U)|(1UL<<1U)) &&
        enc_capture_get(&capture,30000U,&sample) &&
        motion_run_tick(&run,&sample,30000U) &&
        (100 == run.status.position_um[0]) &&
        (10 == run.status.speed_mm_s[1]) && b_pass;
    b_pass = enc_capture_get(&capture,40000U,&sample) &&
        motion_run_tick(&run,&sample,40000U) &&
        (20 == run.status.target_mm_s[0]) &&
        (1250U == fake.a[0]) && (0U == fake.b[0]) &&
        (1250U == fake.a[1]) && (0U == fake.b[1]) && b_pass;
    b_pass = !enc_capture_feed(&capture,ENC_CAPTURE_LEFT_MASK,
                              1UL<<7U) &&
        !enc_capture_get(&capture,50000U,&sample) &&
        !motion_run_tick(&run,&sample,50000U) &&
        (MOTION_RUN_SENSOR == run.status.fault_result) &&
        (0U == fake.a[0]) && (0U == fake.a[1]) && b_pass;
    return (b_pass);
}

static bool test_motion_packed_flags (void)
{
    bool b_pass = true;

    for (uint32_t bits = 0U; bits < 32U; ++bits)
    {
        drv_motor_t motor = {0};
        motion_quad_t decoder = {0};
        wheel_loop_t loop = {0};

        motor.period = 3456U;
        motor.last_time_us = UINT64_MAX;
        motor.b_initialised = (0U != (bits & 1U));
        motor.b_armed = (0U != (bits & 2U));
        motor.b_fault = (0U != (bits & 4U));
        motor.b_prepare_attempted = (0U != (bits & 8U));
        motor.b_stop_confirmed = (0U != (bits & 16U));
        decoder.count = INT64_MIN;
        decoder.invalid_transitions = UINT32_MAX;
        decoder.previous_ab = 3U;
        decoder.b_initialised = motor.b_initialised;
        decoder.b_have_state = motor.b_armed;
        decoder.b_count_limit = motor.b_fault;
        loop.last_tick_us = UINT64_MAX;
        loop.b_tracking[0] = true;
        loop.b_tracking[1] = false;
        loop.b_valid = motor.b_initialised;
        loop.b_enabled = motor.b_armed;
        loop.b_fault = motor.b_fault;

        const drv_motor_t copied_motor = motor;
        const motion_quad_t copied_decoder = decoder;
        const wheel_loop_t copied_loop = loop;

        motor.b_armed = false;
        decoder.b_have_state = false;
        loop.b_enabled = false;
        b_pass = (copied_motor.b_initialised == (0U != (bits & 1U))) &&
            (copied_motor.b_armed == (0U != (bits & 2U))) &&
            (copied_motor.b_fault == (0U != (bits & 4U))) &&
            (copied_motor.b_prepare_attempted == (0U != (bits & 8U))) &&
            (copied_motor.b_stop_confirmed == (0U != (bits & 16U))) &&
            (copied_decoder.b_initialised == copied_motor.b_initialised) &&
            (copied_decoder.b_have_state == copied_motor.b_armed) &&
            (copied_decoder.b_count_limit == copied_motor.b_fault) &&
            (copied_loop.b_valid == copied_motor.b_initialised) &&
            (copied_loop.b_enabled == copied_motor.b_armed) &&
            (copied_loop.b_fault == copied_motor.b_fault) &&
            !motor.b_armed && !decoder.b_have_state && !loop.b_enabled &&
            (motor.b_fault == copied_motor.b_fault) &&
            (decoder.b_count_limit == copied_decoder.b_count_limit) &&
            (loop.b_fault == copied_loop.b_fault) &&
            (3456U == copied_motor.period) &&
            (UINT64_MAX == copied_motor.last_time_us) &&
            (INT64_MIN == copied_decoder.count) &&
            (UINT32_MAX == copied_decoder.invalid_transitions) &&
            (3U == copied_decoder.previous_ab) &&
            (UINT64_MAX == copied_loop.last_tick_us) &&
            copied_loop.b_tracking[0] && !copied_loop.b_tracking[1] && b_pass;
    }
    return (b_pass);
}

static bool test_motion_payload_copy (void)
{
    bool b_pass = true;
    uint32_t context = 123U;

    for (uint32_t kind = 0U; kind < 4U; ++kind)
    {
        motion_bus_t bus = {0};
        motion_bus_request_t request = {0};
        motion_bus_request_t copied = {0};
        bool b_present = false, b_stop = false, b_cancel = false;

        request.kind = (motion_bus_kind_t)kind;
        switch (request.kind)
        {
            case MOTION_BUS_CONFIG:
                request.payload.configure.config = test_motion_loop_config();
                request.payload.configure.encoder_port.p_context = &context;
            break;
            case MOTION_BUS_ENABLE:
                bus.status.state = MOTION_RUN_CONFIGURED;
            break;
            case MOTION_BUS_COMMAND:
                bus.status.state = MOTION_RUN_READY;
                request.payload.command = test_motion_command(MOVE_PLAN_TRAVEL);
                request.payload.command.distance_um = INT32_MIN;
            break;
            case MOTION_BUS_ACK:
                request.payload.ack_id = UINT32_MAX;
                bus.status.terminal_id = UINT32_MAX;
            break;
            default:
                return (false);
            break;
        }
        b_pass = motion_bus_put(&bus,&request) && b_pass;
        request.payload = (motion_bus_payload_t){0};
        motion_bus_take(&bus,&copied,&b_present,&b_stop,&b_cancel);
        b_pass = b_present && !b_stop && !b_cancel && bus.b_inflight &&
            (copied.kind == (motion_bus_kind_t)kind) && b_pass;
        if (MOTION_BUS_CONFIG == copied.kind)
        {
            b_pass = (150000U ==
                copied.payload.configure.config.geometry.track_width_um) &&
                (&context == copied.payload.configure.encoder_port.p_context)
                && b_pass;
        }
        else if (MOTION_BUS_COMMAND == copied.kind)
        {
            b_pass = (INT32_MIN == copied.payload.command.distance_um) &&
                (MOVE_PLAN_TRAVEL == copied.payload.command.kind) && b_pass;
        }
        else if (MOTION_BUS_ACK == copied.kind)
        {
            b_pass = (UINT32_MAX == copied.payload.ack_id) && b_pass;
        }
    }
    return (b_pass);
}

static bool test_motion_payload_invalid (void)
{
    motion_bus_t bus = {0};
    motion_bus_request_t request = {0};
    const motion_bus_kind_t invalid[] = {(motion_bus_kind_t)255,
                                         (motion_bus_kind_t)-1};
    bool b_pass = true;

    bus.status.state = MOTION_RUN_READY;
    bus.request.kind = MOTION_BUS_COMMAND;
    bus.request.payload.command.id = 777U;
    for (size_t index = 0U; index < sizeof(invalid)/sizeof(invalid[0]); ++index)
    {
        request.kind = invalid[index];
        request.payload.ack_id = UINT32_MAX;
        b_pass = !motion_bus_put(&bus,&request) && !bus.b_pending &&
            !bus.b_inflight && !bus.b_stop && !bus.b_cancel &&
            (MOTION_BUS_COMMAND == bus.request.kind) &&
            (777U == bus.request.payload.command.id) && b_pass;
    }
    return (b_pass);
}

#endif /* Pure tests only. */

/*** end of file ***/

