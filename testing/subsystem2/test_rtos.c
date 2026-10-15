/** @file test_rtos.c
 * @brief Host-only tests of the actual owner and encoder adapter sources.
 */
#if defined(MOTION_RTOS_HOST_TEST)
#include "test_rtos.h"
#include "subsystem2.h"
#include "task_motion.h"
#include "motion_api.h"
#include "drv_rp2040.h"
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum
{
    TEST_RTOS_MUTEX = 1, TEST_RTOS_FLAG, TEST_RTOS_TASK,
    TEST_RTOS_SEM, TEST_RTOS_POOL
} test_rtos_kind_t;

typedef struct
{
    test_rtos_kind_t kind;
    bool b_live;
    bool b_running;
    bool b_locked;
} test_rtos_object_t;

typedef struct
{
    uint32_t case_id;
    uint32_t creates;
    uint32_t deletes;
    uint32_t starts;
    uint32_t fail_create;
    uint32_t flags[32];
    uint32_t posts[32];
    test_rtos_object_t objects[32];
    ID fail_delete;
    uint32_t producer_calls;
    uint32_t now_ms;
    uint32_t waits;
    uint32_t stage;
    uint32_t prepares;
    uint32_t applies;
    uint32_t stops;
    uint32_t reserves;
    uint32_t enables;
    uint32_t releases;
    uint32_t gpio_writes;
    uint32_t control[30];
    uint32_t fail_pin;
    uint32_t first_drive_ms;
    uint64_t encoder_sample_us;
    uint16_t levels[2][2];
    T_CTSK task;
    ID task_id;
    drv_encoder_sink_t p_sink;
    void * p_sink_context;
    bool b_ok;
    bool b_mutex_busy;
    bool b_spin_busy;
    bool b_capture_locked;
    bool b_clock_error;
    bool b_wait_error;
    bool b_ref_error;
    bool b_exited;
    bool b_spin_init_error;
    bool b_reserved;
    bool b_checked;
} test_rtos_fake_t;

static test_rtos_fake_t g_test_rtos;
static jmp_buf g_test_rtos_exit;
static wheel_loop_config_t test_rtos_config (void);
static bool test_rtos_reserve (void * p_context, uint32_t mask,
    drv_encoder_sink_t p_sink, void * p_sink_context);
static bool test_rtos_enable (void * p_context, uint32_t mask);
static void test_rtos_release (void * p_context, uint32_t mask);
static bool test_rtos_prepare (void * p_context, uint16_t period,
    uint32_t clock_hz);
static bool test_rtos_apply (void * p_context, drv_motor_wheel_t wheel,
    uint16_t a, uint16_t b);
static bool test_rtos_stop (void * p_context);
static ID test_rtos_create (test_rtos_kind_t kind);
static bool test_rtos_object (ID id, test_rtos_kind_t kind);
static ER test_rtos_delete (ID id, test_rtos_kind_t kind);
static uint32_t test_rtos_live (void);
static bool test_rtos_lifecycle (void);
static void test_rtos_script (void);
static void test_rtos_active (motion_run_status_t const * p_status);
static void test_rtos_run (void);
static bool test_rtos_case (void);
static bool test_rtos_feedback (void);
static void test_rtos_sink (void * p_context, uint32_t events,
    uint32_t levels);

/* The fake implements only the kernel ABI used by Buddy 2. It does not
 * emulate scheduling, memory ordering, priority inheritance or IRQ latency.
 */
ID tk_cre_mtx (T_CMTX const * p_info)
{
    g_test_rtos.b_ok = (TA_INHERIT == p_info->mtxatr) && g_test_rtos.b_ok;
    return (test_rtos_create(TEST_RTOS_MUTEX));
}

ID tk_cre_flg (T_CFLG const * p_info)
{
    g_test_rtos.b_ok = (TA_TFIFO == p_info->flgatr) && g_test_rtos.b_ok;
    return (test_rtos_create(TEST_RTOS_FLAG));
}

ID tk_cre_tsk (T_CTSK const * p_info)
{
    g_test_rtos.task = *p_info;
    ID id = test_rtos_create(TEST_RTOS_TASK);

    if (id > 0) { g_test_rtos.task_id = id; }
    return (id);
}

ER tk_sta_tsk (ID id, INT code)
{
    (void)code;
    ++g_test_rtos.starts;
    if (!test_rtos_object(id, TEST_RTOS_TASK))
    {
        return (E_SYS);
    }
    if ((104U == g_test_rtos.case_id) || (130U == g_test_rtos.case_id))
    {
        return (E_SYS);
    }
    if (g_test_rtos.objects[id].b_running)
    {
        return (E_OBJ);
    }
    g_test_rtos.objects[id].b_running = true;
    return (E_OK);
}

ER tk_del_tsk (ID id)
{
    return (test_rtos_delete(id, TEST_RTOS_TASK));
}

ER tk_del_flg (ID id)
{
    return (test_rtos_delete(id, TEST_RTOS_FLAG));
}

ER tk_del_mtx (ID id)
{
    return (test_rtos_delete(id, TEST_RTOS_MUTEX));
}

ID tk_cre_sem (T_CSEM const * p_info)
{
    g_test_rtos.b_ok = (TA_TFIFO == p_info->sematr) &&
        (S2_POOL_COUNT == p_info->isemcnt) &&
        (S2_POOL_COUNT == p_info->maxsem) && g_test_rtos.b_ok;
    return (test_rtos_create(TEST_RTOS_SEM));
}

ID tk_cre_mpf (T_CMPF const * p_info)
{
    g_test_rtos.b_ok = ((TA_TFIFO | TA_RNG3) == p_info->mpfatr) &&
        (S2_POOL_COUNT == p_info->mpfcnt) &&
        (sizeof(SAMPLE_MSG) == p_info->blfsz) && g_test_rtos.b_ok;
    return (test_rtos_create(TEST_RTOS_POOL));
}

ER tk_del_sem (ID id)
{
    return (test_rtos_delete(id, TEST_RTOS_SEM));
}

ER tk_del_mpf (ID id)
{
    return (test_rtos_delete(id, TEST_RTOS_POOL));
}

void producer_task (INT code, void * p_context)
{
    (void)code; (void)p_context;
}

ID app_start_task (FP entry, INT priority, UW processor, char const * p_name)
{
    (void)p_name;
    ++g_test_rtos.producer_calls;
    g_test_rtos.b_ok = (producer_task == entry) &&
        (S2_PRI_PRODUCER == priority) && (TP_PRC2 == processor) &&
        g_test_rtos.b_ok;
    return ((131U == g_test_rtos.case_id) ? E_SYS : 20);
}

ER tk_loc_mtx (ID id, TMO timeout)
{
    g_test_rtos.b_ok = (TMO_POL == timeout) && g_test_rtos.b_ok;
    if (!test_rtos_object(id, TEST_RTOS_MUTEX))
    {
        return (E_SYS);
    }
    if (g_test_rtos.b_mutex_busy)
    {
        return (E_TMOUT);
    }
    g_test_rtos.b_ok = !g_test_rtos.objects[id].b_locked && g_test_rtos.b_ok;
    g_test_rtos.objects[id].b_locked = true;
    return (E_OK);
}

ER tk_unl_mtx (ID id)
{
    if (!test_rtos_object(id, TEST_RTOS_MUTEX))
    {
        return (E_SYS);
    }
    g_test_rtos.b_ok = g_test_rtos.objects[id].b_locked && g_test_rtos.b_ok;
    g_test_rtos.objects[id].b_locked = false;
    return (E_OK);
}

ER tk_set_flg (ID id, UINT pattern)
{
    if (!test_rtos_object(id, TEST_RTOS_FLAG)) { return (E_SYS); }
    ++g_test_rtos.posts[id];
    if (((107U == g_test_rtos.case_id) && (3 == id)) ||
        ((108U == g_test_rtos.case_id) && (2 == id)))
    {
        return (E_SYS);
    }
    g_test_rtos.flags[id] |= pattern;
    return (E_OK);
}

ER tk_ref_flg (ID id, T_RFLG * p_info)
{
    if (!test_rtos_object(id, TEST_RTOS_FLAG)) { return (E_SYS); }
    p_info->flgptn = g_test_rtos.flags[id];
    return (g_test_rtos.b_ref_error ? E_SYS : E_OK);
}

ER tk_wai_flg (ID id, UINT pattern, UINT mode, UINT * p_actual, TMO timeout)
{
    if (!test_rtos_object(id, TEST_RTOS_FLAG)) { return (E_SYS); }
    g_test_rtos.b_ok = ((TWF_ORW | TWF_CLR) == mode) &&
        (1U == pattern) && g_test_rtos.b_ok;
    ++g_test_rtos.waits;
    if (98U == g_test_rtos.case_id)
    {
        g_test_rtos.b_ok = (TMO_FEVR == timeout) &&
            (0U == g_test_rtos.prepares) && (0U == g_test_rtos.gpio_writes) &&
            g_test_rtos.b_ok;
        longjmp(g_test_rtos_exit, 1);
    }
    if (g_test_rtos.waits > 30U)
    {
        g_test_rtos.b_ok = false;
        longjmp(g_test_rtos_exit, 1);
    }
    test_rtos_script();
    if (g_test_rtos.b_wait_error)
    {
        return (E_SYS);
    }
    if (0U != (g_test_rtos.flags[id] & pattern))
    {
        *p_actual = g_test_rtos.flags[id];
        g_test_rtos.flags[id] = 0U;
        return (E_OK);
    }
    if (timeout < 0)
    {
        g_test_rtos.b_ok = false;
        longjmp(g_test_rtos_exit, 1);
    }
    g_test_rtos.now_ms += (uint32_t)timeout;
    return (E_TMOUT);
}

ER tk_get_otm (SYSTIM * p_time)
{
    p_time->hi = 0;
    p_time->lo = g_test_rtos.now_ms;
    return (g_test_rtos.b_clock_error ? E_SYS : E_OK);
}

void tk_ext_tsk (void)
{
    ID id = g_test_rtos.task_id;

    if (test_rtos_object(id, TEST_RTOS_TASK))
    {
        g_test_rtos.b_ok = g_test_rtos.objects[id].b_running &&
            g_test_rtos.b_ok;
        g_test_rtos.objects[id].b_running = false;
    }
    g_test_rtos.b_exited = true;
    longjmp(g_test_rtos_exit, 1);
}

ER InitSpinLock (T_SPLOCK * p_lock)
{
    (void)p_lock;
    return (g_test_rtos.b_spin_init_error ? E_SYS : E_OK);
}

ER ISpinTryLock (T_SPLOCK * p_lock, UINT * p_saved)
{
    (void)p_lock; *p_saved = 0U;
    if (g_test_rtos.b_spin_busy) { return (E_BUSY); }
    g_test_rtos.b_ok = !g_test_rtos.b_capture_locked && g_test_rtos.b_ok;
    g_test_rtos.b_capture_locked = true;
    return (E_OK);
}

ER ISpinUnlock (T_SPLOCK * p_lock, UINT saved)
{
    (void)p_lock; (void)saved;
    g_test_rtos.b_ok = g_test_rtos.b_capture_locked && g_test_rtos.b_ok;
    g_test_rtos.b_capture_locked = false;
    return (E_OK);
}

uint32_t in_w (uint32_t address)
{
    uint32_t value = 0U;

    if ((address >= GPIO_CTRL(0U)) && (address <= GPIO_CTRL(29U)))
    {
        value = g_test_rtos.control[(address - GPIO_CTRL(0U)) / 4U];
    }
    return (value);
}

ER gpio_set_pin (UINT pin, UINT mode)
{
    ++g_test_rtos.gpio_writes;
    g_test_rtos.b_ok = (GPIO_MODE_IN == mode) &&
        (pin < 30U) && (0U != ((1UL << pin) & ENC_CAPTURE_MASK)) &&
        g_test_rtos.b_ok;
    return ((pin == g_test_rtos.fail_pin) ? E_SYS : E_OK);
}

bool drv_rp2040_bind (drv_rp2040_t * p_backend, drv_motor_io_t * p_io)
{
    (void)p_backend;
    *p_io = (drv_motor_io_t){NULL, test_rtos_prepare, test_rtos_apply,
                            test_rtos_stop};
    return (true);
}

int main (int argc, char * argv[])
{
    if (2 != argc)
    {
        return (2);
    }
    g_test_rtos.case_id = (uint32_t)strtoul(argv[1], NULL, 10);
    g_test_rtos.b_ok = true;
    g_test_rtos.fail_pin = UINT32_MAX;
    for (uint32_t pin = 0U; pin < 30U; ++pin)
    {
        g_test_rtos.control[pin] = 31U;
    }
    bool b_pass = test_rtos_case() && g_test_rtos.b_ok;

    (void)printf("[TEST] S2-U%u %s actual adapter with host doubles\n",
        g_test_rtos.case_id, b_pass ? "PASS" : "FAIL");
    return (b_pass ? 0 : 1);
}

static wheel_loop_config_t test_rtos_config (void)
{
    wheel_loop_config_t cfg = {0};

    cfg.geometry.left = (motion_config_wheel_t){1000U, 100000U,
        MOTION_TYPES_POLARITY_NORMAL, MOTION_TYPES_POLARITY_NORMAL};
    cfg.geometry.right = cfg.geometry.left;
    cfg.geometry.track_width_um = 150000U;
    cfg.geometry.max_wheel_speed_mm_s = 500;
    cfg.geometry.calibration = MOTION_CONFIG_CALIBRATION_CONFIRMED;
    cfg.motor = (drv_motor_config_t){125000000U, 20000U, 100U, 1.0F,
        {MOTION_TYPES_POLARITY_NORMAL, MOTION_TYPES_POLARITY_NORMAL},
        DRV_MOTOR_BRAKE, MOTION_CONFIG_CALIBRATION_CONFIRMED, true};
    cfg.pid[0] = (motion_pid_config_t){0.01F, 0.0F, 0.0F, -1.0F, 1.0F,
        -0.5F, 0.5F, 100000U, MOTION_CONFIG_CALIBRATION_CONFIRMED};
    cfg.pid[1] = cfg.pid[0];
    cfg.profile = (move_plan_config_t){1000U, 1000U, 1000U,
        {1000000U, 1000000U}, 1.0F, 100};
    cfg.period_us = (105U == g_test_rtos.case_id) ? 30000U : 10000U;
    cfg.sample_limit_us = 100000U;
    cfg.stop_dwell_us = cfg.period_us;
    cfg.stop_timeout_us = 1000000U;
    cfg.progress_timeout_us = 2000000U;
    cfg.overspeed_mm_s = 1000;
    return (cfg);
}

static bool test_rtos_reserve (void * p_context, uint32_t mask,
    drv_encoder_sink_t p_sink, void * p_sink_context)
{
    (void)p_context;
    ++g_test_rtos.reserves;
    g_test_rtos.b_ok = (ENC_CAPTURE_MASK == mask) &&
        (0U == g_test_rtos.gpio_writes) && g_test_rtos.b_ok;
    g_test_rtos.b_ok = !g_test_rtos.b_reserved && g_test_rtos.b_ok;
    g_test_rtos.b_reserved = (109U != g_test_rtos.case_id);
    g_test_rtos.p_sink = p_sink;
    g_test_rtos.p_sink_context = p_sink_context;
    return (109U != g_test_rtos.case_id);
}

static bool test_rtos_enable (void * p_context, uint32_t mask)
{
    (void)p_context;
    ++g_test_rtos.enables;
    g_test_rtos.b_ok = g_test_rtos.b_reserved && g_test_rtos.b_ok;
    g_test_rtos.b_ok = (ENC_CAPTURE_MASK == mask) &&
        (4U == g_test_rtos.gpio_writes) && g_test_rtos.b_ok;
    return (110U != g_test_rtos.case_id);
}

static void test_rtos_release (void * p_context, uint32_t mask)
{
    (void)p_context;
    ++g_test_rtos.releases;
    g_test_rtos.b_ok = g_test_rtos.b_reserved &&
        !g_test_rtos.b_capture_locked && g_test_rtos.b_ok;
    g_test_rtos.b_reserved = false;
    g_test_rtos.b_ok = (ENC_CAPTURE_MASK == mask) && g_test_rtos.b_ok;
}

static bool test_rtos_prepare (void * p_context, uint16_t period,
    uint32_t clock_hz)
{
    (void)p_context;
    ++g_test_rtos.prepares;
    return ((6250U == period) && (125000000U == clock_hz));
}

static bool test_rtos_apply (void * p_context, drv_motor_wheel_t wheel,
    uint16_t a, uint16_t b)
{
    (void)p_context;
    ++g_test_rtos.applies;
    if (((a > 0U) || (b > 0U)) && (0U == g_test_rtos.first_drive_ms))
    {
        g_test_rtos.first_drive_ms = g_test_rtos.now_ms;
    }
    g_test_rtos.levels[wheel][0] = a;
    g_test_rtos.levels[wheel][1] = b;
    return (true);
}

static bool test_rtos_stop (void * p_context)
{
    (void)p_context;
    ++g_test_rtos.stops;
    for (uint32_t wheel = 0U; wheel < 2U; ++wheel)
    {
        g_test_rtos.levels[wheel][0] = 0U;
        g_test_rtos.levels[wheel][1] = 0U;
    }
    return (true);
}

static ID test_rtos_create (test_rtos_kind_t kind)
{
    ++g_test_rtos.creates;
    if (g_test_rtos.creates == g_test_rtos.fail_create)
    {
        return (E_SYS);
    }
    if (g_test_rtos.creates >= 32U)
    {
        g_test_rtos.b_ok = false;
        return (E_SYS);
    }
    ID id = (ID)g_test_rtos.creates;

    g_test_rtos.objects[id] = (test_rtos_object_t){kind, true, false, false};
    return (id);
}

static bool test_rtos_object (ID id, test_rtos_kind_t kind)
{
    bool b_valid = (id > 0) && (id < 32) &&
        g_test_rtos.objects[id].b_live &&
        (kind == g_test_rtos.objects[id].kind);

    g_test_rtos.b_ok = b_valid && g_test_rtos.b_ok;
    return (b_valid);
}

static ER test_rtos_delete (ID id, test_rtos_kind_t kind)
{
    if (!test_rtos_object(id, kind)) { return (E_SYS); }
    if (id == g_test_rtos.fail_delete) { return (E_SYS); }
    if (g_test_rtos.objects[id].b_running ||
        g_test_rtos.objects[id].b_locked)
    {
        g_test_rtos.b_ok = false;
        return (E_OBJ);
    }
    g_test_rtos.objects[id].b_live = false;
    ++g_test_rtos.deletes;
    return (E_OK);
}

static uint32_t test_rtos_live (void)
{
    uint32_t live = 0U;

    for (uint32_t id = 1U; id < 32U; ++id)
    {
        if (g_test_rtos.objects[id].b_live) { ++live; }
    }
    return (live);
}

static void test_rtos_script (void)
{
    motion_run_status_t status = {0};

    if ((106U == g_test_rtos.case_id) && (2U == g_test_rtos.stage))
    {
        g_test_rtos.b_ok = (g_test_rtos.stops > 0U) && g_test_rtos.b_ok;
        g_test_rtos.b_mutex_busy = false;
        g_test_rtos.stage = 3U;
    }
    if (MOTION_API_QUEUED != motion_api_status(&status))
    {
        g_test_rtos.b_ok = false;
        longjmp(g_test_rtos_exit, 1);
    }
    if ((139U == g_test_rtos.case_id) && status.b_encoder_valid &&
        !status.b_measurement_valid)
    {
        motion_run_status_t copy = {0};
        bool b_pass = (10000U == status.encoder_timestamp_us) &&
            (1U == status.encoder_generation) &&
            (0 == status.encoder_count[0]) && (0 == status.encoder_count[1]);

        g_test_rtos.now_ms = 110U;
        b_pass = (MOTION_API_QUEUED == motion_api_status(&copy)) &&
            copy.b_encoder_valid && !copy.b_measurement_valid && b_pass;
        g_test_rtos.now_ms = 111U;
        b_pass = (MOTION_API_QUEUED == motion_api_status(&copy)) &&
            !copy.b_encoder_valid && (10000U == copy.encoder_timestamp_us) &&
            (1U == copy.encoder_generation) && b_pass;
        g_test_rtos.now_ms = 9U;
        b_pass = (MOTION_API_QUEUED == motion_api_status(&copy)) &&
            !copy.b_encoder_valid && b_pass;
        g_test_rtos.now_ms = 10U;
        g_test_rtos.b_clock_error = true;
        b_pass = (MOTION_API_QUEUED == motion_api_status(&copy)) &&
            !copy.b_encoder_valid && b_pass;
        g_test_rtos.b_clock_error = false;
        b_pass = (MOTION_API_QUEUED == motion_api_status(&copy)) &&
            copy.b_encoder_valid && !copy.b_measurement_valid && b_pass;
        g_test_rtos.b_mutex_busy = true;
        copy.encoder_count[0] = 777;
        b_pass = (MOTION_API_BUSY == motion_api_status(&copy)) &&
            (777 == copy.encoder_count[0]) && b_pass;
        g_test_rtos.b_mutex_busy = false;
        g_test_rtos.b_checked = b_pass;
        longjmp(g_test_rtos_exit, 1);
    }
    if (MOTION_RUN_CONFIGURED == status.state)
    {
        g_test_rtos.b_ok = (MOTION_API_QUEUED == motion_api_enable()) &&
            g_test_rtos.b_ok;
    }
    else if (MOTION_RUN_READY == status.state)
    {
        move_plan_command_t command = {1U, MOVE_PLAN_SPEED,
            (uint64_t)g_test_rtos.now_ms * 1000U, 1000000U, 100, 100, 0, 0};

        g_test_rtos.b_ok = (MOTION_API_QUEUED == motion_api_submit(&command)) &&
            g_test_rtos.b_ok;
    }
    else if (MOTION_RUN_ACTIVE == status.state)
    {
        test_rtos_active(&status);
    }
    else if (MOTION_RUN_FAULT == status.state)
    {
        if (140U == g_test_rtos.case_id)
        {
            g_test_rtos.b_checked = !status.b_encoder_valid &&
                (1 == status.encoder_count[0]) &&
                (-1 == status.encoder_count[1]) &&
                (g_test_rtos.encoder_sample_us ==
                    status.encoder_timestamp_us) &&
                (1U == status.encoder_generation) &&
                (1U == g_test_rtos.releases) && !g_test_rtos.b_reserved;
        }
        longjmp(g_test_rtos_exit, 1);
    }
    else
    {
        /* Configuration/baseline/dwell must precede an active command. */
    }
}

static void test_rtos_active (motion_run_status_t const * p_status)
{
    if (140U == g_test_rtos.case_id)
    {
        if (0U == g_test_rtos.stage)
        {
            g_test_rtos.stage = 1U;
            g_test_rtos.p_sink(g_test_rtos.p_sink_context,
                ENC_CAPTURE_MASK, (1UL << 28U) | (1UL << 0U));
        }
        else if ((1U == g_test_rtos.stage) &&
            (p_status->measurement_timestamp_us >= 20000U))
        {
            g_test_rtos.b_ok = p_status->b_encoder_valid &&
                (1 == p_status->encoder_count[0]) &&
                (-1 == p_status->encoder_count[1]) &&
                (1U == p_status->encoder_generation) &&
                (p_status->measurement_timestamp_us ==
                 p_status->encoder_timestamp_us) && g_test_rtos.b_ok;
            g_test_rtos.encoder_sample_us = p_status->encoder_timestamp_us;
            g_test_rtos.b_ok = (MOTION_API_QUEUED == motion_api_estop()) &&
                g_test_rtos.b_ok;
            g_test_rtos.stage = 2U;
        }
    }
    else if (119U == g_test_rtos.case_id)
    {
        if (0U == g_test_rtos.stage)
        {
            g_test_rtos.stage = 1U;
            g_test_rtos.p_sink(g_test_rtos.p_sink_context,
                ENC_CAPTURE_LEFT_MASK, 1UL << 28U);
        }
        else if (p_status->measurement_timestamp_us >= 20000U)
        {
            g_test_rtos.b_ok = p_status->b_encoder_valid &&
                (1 == p_status->encoder_count[0]) &&
                (0 == p_status->encoder_count[1]) &&
                (1U == p_status->encoder_generation) &&
                (p_status->encoder_timestamp_us ==
                    p_status->measurement_timestamp_us) &&
                (100 == p_status->position_um[0]) &&
                (0 == p_status->position_um[1]) &&
                (10 == p_status->speed_mm_s[0]) &&
                (0 == p_status->speed_mm_s[1]) && g_test_rtos.b_ok;
            g_test_rtos.b_checked = true;
            longjmp(g_test_rtos_exit, 1);
        }
    }
    else if (105U == g_test_rtos.case_id)
    {
        if (g_test_rtos.stage < 2U)
        {
            ++g_test_rtos.stage;
            g_test_rtos.now_ms = 30U + (g_test_rtos.stage * 10U);
            move_plan_command_t command = {1U, MOVE_PLAN_SPEED,
                (uint64_t)g_test_rtos.now_ms * 1000U,
                1000000U, 100, 100, 0, 0};

            g_test_rtos.b_ok = (0U == g_test_rtos.applies) &&
                (MOTION_API_QUEUED == motion_api_submit(&command)) &&
                g_test_rtos.b_ok;
        }
        else if (g_test_rtos.applies > 0U)
        {
            g_test_rtos.b_ok = (60U == g_test_rtos.first_drive_ms) &&
                (60000U == p_status->measurement_timestamp_us) &&
                (2U == g_test_rtos.applies) && g_test_rtos.b_ok;
            g_test_rtos.b_checked = true;
            longjmp(g_test_rtos_exit, 1);
        }
    }
    else if (g_test_rtos.applies > 0U)
    {
        if (116U == g_test_rtos.case_id)
        {
            motion_run_status_t stale = {0};

            g_test_rtos.b_ok = p_status->b_measurement_valid &&
                p_status->b_standstill && g_test_rtos.b_ok;
            g_test_rtos.now_ms += 101U;
            g_test_rtos.b_ok =
                (MOTION_API_QUEUED == motion_api_status(&stale)) &&
                !stale.b_measurement_valid && !stale.b_standstill &&
                !stale.b_encoder_valid &&
                (stale.encoder_count[0] == p_status->encoder_count[0]) &&
                (stale.encoder_count[1] == p_status->encoder_count[1]) &&
                (stale.encoder_timestamp_us ==
                    p_status->encoder_timestamp_us) &&
                (stale.encoder_generation == p_status->encoder_generation) &&
                (stale.measurement_timestamp_us ==
                    p_status->measurement_timestamp_us) &&
                (stale.timestamp_us == p_status->timestamp_us) &&
                g_test_rtos.b_ok;
            g_test_rtos.b_checked = true;
            longjmp(g_test_rtos_exit, 1);
        }
        if (0U == g_test_rtos.stage)
        {
            g_test_rtos.stage = 1U;
            if (106U == g_test_rtos.case_id)
            {
                g_test_rtos.b_mutex_busy = true;
                g_test_rtos.b_ok = (MOTION_API_BUSY == motion_api_stop()) &&
                    (MOTION_API_QUEUED == motion_api_estop()) &&
                    g_test_rtos.b_ok;
                g_test_rtos.stage = 2U;
            }
            else if ((112U == g_test_rtos.case_id) ||
                     (113U == g_test_rtos.case_id))
            {
                g_test_rtos.b_spin_busy = (113U == g_test_rtos.case_id);
                g_test_rtos.p_sink(g_test_rtos.p_sink_context,
                    ENC_CAPTURE_LEFT_MASK, (1UL << 7U) | (1UL << 28U));
                g_test_rtos.b_spin_busy = false;
            }
            else
            {
                g_test_rtos.b_clock_error = (114U == g_test_rtos.case_id);
                g_test_rtos.b_wait_error = (115U == g_test_rtos.case_id);
                g_test_rtos.b_ref_error = (120U == g_test_rtos.case_id);
            }
        }
    }
}

static void test_rtos_run (void)
{
    if (0 == setjmp(g_test_rtos_exit))
    {
        g_test_rtos.task.task(0, NULL);
    }
}

static bool test_rtos_case (void)
{
    uint32_t id = g_test_rtos.case_id;
    wheel_loop_config_t cfg = test_rtos_config();
    drv_encoder_port_t port = {NULL, test_rtos_reserve, test_rtos_enable,
                               test_rtos_release};
    motion_run_status_t status = {0};

    if ((id >= 135U) && (id <= 138U)) { return (test_rtos_feedback()); }
    if (139U == id)
    {
        if ((MOTION_API_UNAVAILABLE != motion_api_status(&status)) ||
            status.b_encoder_valid ||
            (MOTION_API_INVALID != motion_api_status(NULL))) { return (false); }
        g_test_rtos.now_ms = 10U;
    }
    if ((id < 98U) || (id > 140U))
    {
        return (false);
    }
    if ((id >= 121U) && (id <= 134U)) { return (test_rtos_lifecycle()); }
    g_test_rtos.b_spin_init_error = (102U == id);
    if ((id >= 99U) && (id <= 101U))
    {
        g_test_rtos.fail_create = id - 98U;
    }
    else if (103U == id)
    {
        g_test_rtos.fail_create = 4U;
    }
    ER result = task_motion_init();

    if ((id >= 99U) && (id <= 103U))
    {
        uint32_t expected = (102U == id) ? 3U :
                             g_test_rtos.fail_create - 1U;
        bool b_pass = (E_SYS == result) &&
            (expected == g_test_rtos.deletes) && (0U == test_rtos_live()) &&
            (0U == g_test_rtos.gpio_writes) &&
            (MOTION_API_UNAVAILABLE == motion_api_enable());
        g_test_rtos.fail_create = 0U;
        g_test_rtos.b_spin_init_error = false;
        b_pass = (E_OK == task_motion_init()) &&
            (4U == test_rtos_live()) && b_pass;
        uint32_t calls = g_test_rtos.creates;

        return ((E_OBJ == task_motion_init()) &&
            (calls == g_test_rtos.creates) && b_pass);
    }
    if (E_OK != result)
    {
        return (false);
    }
    if (104U == id)
    {
        return ((E_SYS == task_motion_start()) &&
                (0U == g_test_rtos.prepares));
    }
    if (98U == id)
    {
        if ((0U != g_test_rtos.starts) ||
            (NULL == g_test_rtos.task.task) ||
            (NULL == g_test_rtos.task.bufptr) ||
            (4096U != g_test_rtos.task.stksz) ||
            (E_OK != task_motion_start()))
        {
            return (false);
        }
        test_rtos_run();
        return ((1U == g_test_rtos.waits) &&
            (MOTION_API_QUEUED == motion_api_status(&status)) &&
            (MOTION_RUN_UNCONFIGURED == status.state) &&
            (MOTION_API_BUSY == motion_api_enable()) &&
            (0U == g_test_rtos.gpio_writes) && (0U == g_test_rtos.prepares));
    }
    if ((107U == id) || (108U == id))
    {
        return ((MOTION_API_ERROR == motion_api_estop()) &&
            (1U == g_test_rtos.posts[2]) && (1U == g_test_rtos.posts[3]) &&
            ((108U == id) ? (1U == g_test_rtos.flags[3]) :
                           (1U == g_test_rtos.flags[2])));
    }
    if (117U == id)
    {
        port.p_reserve = NULL;
        return ((MOTION_API_INVALID == motion_api_configure(&cfg, &port)) &&
            !drv_encoder_open(&port, test_rtos_sink, NULL) &&
            (0U == g_test_rtos.gpio_writes));
    }
    if (118U == id)
    {
        g_test_rtos.control[7] = 4U;
    }
    if (111U == id)
    {
        g_test_rtos.fail_pin = 28U;
    }
    if ((E_OK != task_motion_start()) ||
        (MOTION_API_QUEUED != motion_api_configure(&cfg, &port)))
    {
        return (false);
    }
    test_rtos_run();
    g_test_rtos.b_clock_error = false;
    if (MOTION_API_QUEUED != motion_api_status(&status))
    {
        return (false);
    }
    if (139U == id)
    {
        return (g_test_rtos.b_checked && status.b_encoder_valid &&
            !status.b_measurement_valid && (MOTION_RUN_SETTLING ==
                status.state));
    }
    if (140U == id)
    {
        return (g_test_rtos.b_checked && !status.b_encoder_valid &&
            (MOTION_RUN_FAULT == status.state) &&
            (MOTION_RUN_EMERGENCY == status.fault_result));
    }
    if ((105U == id) || (116U == id) || (119U == id))
    {
        return (g_test_rtos.b_checked &&
            (MOTION_RUN_ACTIVE == status.state));
    }
    motion_run_result_t expected_fault = MOTION_RUN_OUTPUT;

    if (106U == id)
    {
        expected_fault = MOTION_RUN_EMERGENCY;
    }
    else if (((id >= 109U) && (id <= 113U)) || (118U == id))
    {
        expected_fault = MOTION_RUN_SENSOR;
    }
    bool b_stage = true;

    if ((id >= 109U) && (id <= 111U))
    {
        b_stage = (1U == g_test_rtos.reserves) &&
            (g_test_rtos.enables == ((110U == id) ? 1U : 0U)) &&
            (g_test_rtos.gpio_writes == ((109U == id) ? 0U : 4U));
    }
    else if (118U == id)
    {
        b_stage = (0U == g_test_rtos.reserves) &&
            (0U == g_test_rtos.gpio_writes);
    }
    else
    {
        b_stage = (g_test_rtos.applies > 0U) &&
            (g_test_rtos.stage > 0U);
    }
    if ((114U == id) || (115U == id))
    {
        b_stage = g_test_rtos.b_exited && b_stage;
    }
    return (b_stage && !g_test_rtos.b_reserved &&
        (MOTION_RUN_FAULT == status.state) &&
        (expected_fault == status.fault_result) &&
        !status.b_measurement_valid && !status.b_standstill &&
        !status.b_encoder_valid &&
        (0U == g_test_rtos.levels[0][0]) &&
        (0U == g_test_rtos.levels[0][1]) &&
        (0U == g_test_rtos.levels[1][0]) &&
        (0U == g_test_rtos.levels[1][1]) &&
        (((id >= 109U) && (id <= 111U)) || (118U == id) ?
            (0U == g_test_rtos.prepares) :
            (g_test_rtos.stops > 0U)) &&
        (g_test_rtos.releases == (((109U == id) || (118U == id)) ? 0U : 1U)));
}

static bool test_rtos_feedback (void)
{
    uint32_t id = g_test_rtos.case_id;
    motion_run_t run = {0};
    wheel_loop_config_t cfg = test_rtos_config();
    drv_motor_io_t io = {NULL, test_rtos_prepare, test_rtos_apply,
        test_rtos_stop};
    enc_capture_sample_t sample = {0};

    sample.b_healthy = true;
    sample.sample[0].generation = 9U;
    sample.sample[1].generation = 9U;
    if (!motion_run_init(&run, &cfg, &io) || run.status.b_encoder_valid ||
        (0U != run.status.encoder_generation) ||
        !motion_run_enable(&run, 0U) || run.status.b_encoder_valid)
    { return (false); }
    if (138U == id)
    {
        sample.sample[1].generation = 10U;
        if (motion_run_tick(&run, &sample, 0U) || run.status.b_encoder_valid)
        { return (false); }
        if (!motion_run_init(&run, &cfg, &io) || !motion_run_enable(&run, 0U))
        { return (false); }
        sample.sample[0].generation = 0U;
        sample.sample[1].generation = 0U;
        return (!motion_run_tick(&run, &sample, 0U) &&
            !run.status.b_encoder_valid && (MOTION_RUN_SENSOR ==
                run.status.fault_result));
    }
    if (135U == id)
    {
        sample.sample[0].count = INT64_MAX - 1;
        sample.sample[1].count = INT64_MIN + 1;
    }
    else
    {
        sample.sample[0].count = 1;
        sample.sample[1].count = -1;
    }
    if (!motion_run_tick(&run, &sample, 0U) || !run.status.b_encoder_valid ||
        run.status.b_measurement_valid ||
            (9U != run.status.encoder_generation) ||
        (run.status.encoder_count[0] != sample.sample[0].count) ||
        (run.status.encoder_count[1] != sample.sample[1].count))
    { return (false); }
    if (137U == id)
    {
        sample.sample[0].count = 2;
        sample.sample[1].count = -2;
        sample.sample[0].timestamp_us = 5000U;
        sample.sample[1].timestamp_us = 5000U;
        if (!motion_run_tick(&run, &sample, 5000U) ||
            (1 != run.status.encoder_count[0]) ||
            (-1 != run.status.encoder_count[1]) ||
            (0U != run.status.encoder_timestamp_us)) { return (false); }
        motion_run_stop(&run, false, 5000U);
        if (!run.status.b_encoder_valid ||
            (0U != run.status.encoder_timestamp_us))
        { return (false); }
        motion_run_emergency(&run, MOTION_RUN_EMERGENCY, 6000U);
        return (!run.status.b_encoder_valid &&
            (1 == run.status.encoder_count[0]) && (-1 ==
                run.status.encoder_count[1]) &&
            (0U == run.status.encoder_timestamp_us) && (9U ==
                run.status.encoder_generation) &&
            !motion_run_tick(&run, &sample, 10000U));
    }
    ++sample.sample[0].count;
    --sample.sample[1].count;
    sample.sample[0].timestamp_us = 10000U;
    sample.sample[1].timestamp_us = 10000U;
    if (!motion_run_tick(&run, &sample, 10000U) ||
        !run.status.b_encoder_valid ||
        !run.status.b_measurement_valid ||
            (10000U != run.status.encoder_timestamp_us) ||
        (run.status.encoder_count[0] != sample.sample[0].count) ||
        (run.status.encoder_count[1] != sample.sample[1].count))
        { return (false); }
    if (135U == id)
    {
        return ((INT64_MAX == run.status.encoder_count[0]) &&
            (INT64_MIN == run.status.encoder_count[1]));
    }
    motion_types_encoder_count_t left = run.status.encoder_count[0];
    motion_types_encoder_count_t right = run.status.encoder_count[1];

    ++sample.sample[0].count;
    /* Left accepts before right conversion fails. */
    sample.sample[1].count = INT64_MAX;
    sample.sample[0].timestamp_us = 20000U;
    sample.sample[1].timestamp_us = 20000U;
    return (!motion_run_tick(&run, &sample, 20000U) &&
        !run.status.b_encoder_valid && (left == run.status.encoder_count[0]) &&
        (right == run.status.encoder_count[1]) &&
        (10000U == run.status.encoder_timestamp_us) && (9U ==
            run.status.encoder_generation));
}

static bool test_rtos_lifecycle (void)
{
    uint32_t id = g_test_rtos.case_id;
    bool b_pass;

    if (134U == id)
    {
        ID mutex = tk_cre_mtx(&(T_CMTX){TA_INHERIT});

        b_pass = (E_SYS == tk_del_flg(mutex)) && !g_test_rtos.b_ok &&
            (1U == test_rtos_live());
        g_test_rtos.b_ok = true; /* Expected fake diagnostic, not production. */
        b_pass = (E_OK == tk_del_mtx(mutex)) && b_pass;
        b_pass = (E_SYS == tk_del_mtx(mutex)) && !g_test_rtos.b_ok && b_pass;
        g_test_rtos.b_ok = true;
        b_pass = (E_SYS == tk_del_mtx(-1)) && !g_test_rtos.b_ok && b_pass;
        g_test_rtos.b_ok = true;
        return (b_pass && (0U == test_rtos_live()) &&
            (1U == g_test_rtos.deletes));
    }
    if ((132U == id) || (133U == id))
    {
        g_test_rtos.fail_create = 2U;
        g_test_rtos.fail_delete = 1;
        ER result = (132U == id) ? subsystem2_init() : task_motion_init();
        uint32_t calls = g_test_rtos.creates;

        b_pass = (E_SYS == result) && (1U == test_rtos_live());
        result = (132U == id) ? subsystem2_init() : task_motion_init();
        return (b_pass && (E_OBJ == result) &&
            (calls == g_test_rtos.creates) &&
            (1U == test_rtos_live()) && (0U == g_test_rtos.producer_calls));
    }
    if (129U == id)
    {
        return ((E_OBJ == subsystem2_start()) &&
            (0U == test_rtos_live()) && (0U == g_test_rtos.producer_calls) &&
            (0U == g_test_rtos.starts));
    }
    if (id <= 127U)
    {
        g_test_rtos.fail_create = (id <= 125U) ? id - 120U :
            ((127U == id) ? 6U : 0U);
        g_test_rtos.b_spin_init_error = (126U == id);
        b_pass = (E_SYS == subsystem2_init()) &&
            (0U == test_rtos_live()) && (sem_slots <= 0) &&
            (mpf_samples <= 0) && (E_OBJ == subsystem2_start()) &&
            (0U == g_test_rtos.producer_calls);
        g_test_rtos.fail_create = 0U;
        g_test_rtos.b_spin_init_error = false;
        b_pass = (E_OK == subsystem2_init()) &&
            (6U == test_rtos_live()) && b_pass;
        uint32_t calls = g_test_rtos.creates;
        ID semaphore = sem_slots;
        ID pool = mpf_samples;

        return ((E_OBJ == subsystem2_init()) &&
            (calls == g_test_rtos.creates) && (semaphore == sem_slots) &&
            (pool == mpf_samples) && b_pass);
    }
    b_pass = (E_OK == subsystem2_init()) && (6U == test_rtos_live());
    uint32_t calls = g_test_rtos.creates;
    ID semaphore = sem_slots;
    ID pool = mpf_samples;

    b_pass = (E_OBJ == subsystem2_init()) &&
        (calls == g_test_rtos.creates) && b_pass;
    ER expected = (128U == id) ? E_OK : E_SYS;

    b_pass = (expected == subsystem2_start()) &&
        (1U == g_test_rtos.producer_calls) &&
        (g_test_rtos.starts == ((131U == id) ? 0U : 1U)) && b_pass;
    return ((E_OBJ == subsystem2_start()) &&
        (1U == g_test_rtos.producer_calls) &&
        (g_test_rtos.starts == ((131U == id) ? 0U : 1U)) &&
        (6U == test_rtos_live()) && (semaphore == sem_slots) &&
        (pool == mpf_samples) && b_pass);
}

static void test_rtos_sink (void * p_context, uint32_t events,
    uint32_t levels)
{
    (void)p_context; (void)events; (void)levels;
}
#else
typedef int test_rtos_firmware_excluded_t;
#endif /* Host executable only, never automatic TEST=1 hardware access. */

/*** end of file ***/
