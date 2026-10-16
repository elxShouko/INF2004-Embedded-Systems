/** @file s5_live.c @brief Core-1 acquisition owner and coherent publication. */
#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include "s5_live.h"
#include "../subsystem2/motion_api.h"

#define S5_LIVE_PERIOD_MS (10)
#define S5_LIVE_STACK (4096U)
static uint64_t g_s5_stack[S5_LIVE_STACK / sizeof(uint64_t)];
static ID g_s5_mutex;
static ID g_s5_task;
static ID g_s5_fault;
static bool g_s5_started;
static bool g_s5_configured;
static uint64_t g_s5_recovered;
static uint64_t g_s5_recovery_pending;
static s5_live_config_t g_s5_config;
static drv_encoder_port_t g_s5_irq;
static s5_live_status_t g_s5_published;
static s5_acquire_t g_s5_acquisition;
static s5_runtime_t g_s5_policy;
static drv_s5_t g_s5_driver;

static void s5_live_task (INT code, void * p_context);
static void s5_live_cleanup (void);
static bool s5_live_cycle (uint64_t now_us, bool * p_scan_requested);

ER s5_live_init (void)
{
    if ((g_s5_mutex > 0) || (g_s5_task > 0) || (g_s5_fault > 0))
    {
        return (E_OBJ);
    }
    T_CMTX mutex = {0};
    T_CFLG flag = {0};
    T_CTSK task = {0};

    mutex.mtxatr = TA_INHERIT;
    flag.flgatr = TA_TFIFO | TA_WMUL;
    g_s5_mutex = tk_cre_mtx(&mutex);
    if (g_s5_mutex < E_OK) { return (g_s5_mutex); }
    g_s5_fault = tk_cre_flg(&flag);
    if (g_s5_fault < E_OK)
    {
        ER error = g_s5_fault;
        s5_live_cleanup(); return (error);
    }
    task.task = (FP)s5_live_task; /* Required kernel callback ABI. */
    task.itskpri = 20;
    task.stksz = S5_LIVE_STACK;
    task.tskatr = TA_HLNG | TA_RNG3 | TA_USERBUF;
    task.bufptr = g_s5_stack;
#if TK_SUPPORT_SMP
    task.tskatr |= TA_ASSPRC;
    task.assprc = TP_PRC2;
#endif
    g_s5_task = tk_cre_tsk(&task);
    if (g_s5_task < E_OK)
    {
        ER error = g_s5_task;
        s5_live_cleanup(); return (error);
    }
    return (E_OK);
}

ER s5_live_start (void)
{
    if ((g_s5_task <= 0) || g_s5_started) { return (E_OBJ); }
    g_s5_started = true;
    ER error = tk_sta_tsk(g_s5_task,0);

    if (E_OK != error) { g_s5_started = false; }
    return (error);
}

ER s5_live_configure (s5_live_config_t const * p_config,
    drv_encoder_port_t const * p_irq)
{
    s5_runtime_t candidate = {0};

    if ((g_s5_task <= 0) || g_s5_started || g_s5_configured)
    {
        return (E_OBJ);
    }
    if ((NULL == p_config) || (NULL == p_irq) ||
        (NULL == p_irq->p_reserve) || (NULL == p_irq->p_enable) ||
        (NULL == p_irq->p_release) ||
        !s5_acquire_validate(&p_config->acquisition) ||
        (S5_LOGIC_OK != s5_runtime_init(&candidate,&p_config->policy)) ||
        (p_config->policy.forward_sample_max_age_ms > 999U) ||
        ((uint64_t)p_config->policy.forward_sample_max_age_ms * 1000U <=
         (uint64_t)p_config->acquisition.sample_period_us +
         p_config->acquisition.servo_settle_us +
         p_config->acquisition.echo_timeout_us + 20000U))
    {
        return (E_PAR);
    }
    g_s5_config = *p_config;
    g_s5_irq = *p_irq;
    g_s5_configured = true;
    return (E_OK);
}

ER s5_live_status (s5_live_status_t * p_status)
{
    if ((NULL == p_status) || (g_s5_mutex <= 0)) { return (E_PAR); }
    ER error = tk_loc_mtx(g_s5_mutex,TMO_POL);

    if (E_OK == error)
    {
        s5_live_status_t snapshot = g_s5_published;

        error = tk_unl_mtx(g_s5_mutex);
        if (E_OK == error)
        {
            uint64_t now_us = 0U;

            snapshot.b_forward_valid = snapshot.b_forward_valid &&
                !snapshot.b_fault && drv_s5_now(&now_us) &&
                (now_us >= snapshot.forward_timestamp_us) &&
                (now_us - snapshot.forward_timestamp_us <= snapshot.max_age_us);
            *p_status = snapshot;
        }
    }
    return (error);
}

ER s5_live_recovered (uint64_t scan_id_us)
{
    if ((0U == scan_id_us) || (g_s5_mutex <= 0)) { return (E_PAR); }
    ER error = tk_loc_mtx(g_s5_mutex,TMO_POL);

    if (E_OK == error)
    {
        bool b_match = (g_s5_recovered == scan_id_us) ||
            ((g_s5_published.scan_id_us == scan_id_us) &&
             (S5_RUNTIME_HOLD == g_s5_published.policy.state) &&
             g_s5_published.policy.scan_valid && !g_s5_published.b_fault);

        if (b_match && (g_s5_recovered != scan_id_us))
        {
            g_s5_recovery_pending = scan_id_us;
        }
        ER release = tk_unl_mtx(g_s5_mutex);

        error = (E_OK != release) ? release : (b_match ? E_OK : E_OBJ);
    }
    return (error);
}

ER s5_live_fault (void)
{
    if (g_s5_fault <= 0) { return (E_OBJ); }
    UINT pattern = 0U;

    return (tk_wai_flg(g_s5_fault,1U,TWF_ORW,&pattern,TMO_POL));
}

ER s5_live_cancel (void)
{
    return ((g_s5_fault > 0) ? tk_set_flg(g_s5_fault,1U) : E_OBJ);
}

static void s5_live_task (INT code, void * p_context)
{
    bool b_scan_requested = false;

    (void)code; (void)p_context;
    for (;;)
    {
        uint64_t now_us = 0U;
        bool b_ok = drv_s5_now(&now_us);
        s5_live_status_t snapshot = {0};
        ER fault = s5_live_fault();

        b_ok = b_ok && (E_TMOUT == fault);
        if (b_ok && g_s5_configured)
        {
            b_ok = s5_live_cycle(now_us,&b_scan_requested);
        }
        if (!b_ok)
        {
            s5_acquire_close(&g_s5_acquisition);
            (void)s5_live_cancel();
        }
        snapshot.policy = g_s5_policy.status;
        if (!b_ok)
        {
            snapshot.policy.state = S5_RUNTIME_FAULT;
            snapshot.policy.busy = false;
            snapshot.policy.scan_busy = false;
            snapshot.policy.forward_valid = false;
            snapshot.policy.acquisition_status =
                g_s5_acquisition.scan.acquisition_status;
            snapshot.policy.scan_context = g_s5_acquisition.scan;
            if (S5_RUNTIME_FAULT_NONE == snapshot.policy.fault)
            {
                snapshot.policy.fault = !g_s5_configured ?
                    S5_RUNTIME_FAULT_INVALID_INPUT :
                    (g_s5_acquisition.scan.cancelled ?
                    S5_RUNTIME_FAULT_SCAN_CANCELLED :
                    (g_s5_acquisition.b_scan ? S5_RUNTIME_FAULT_SCAN_INVALID :
                    S5_RUNTIME_FAULT_FORWARD_SENSOR));
            }
        }
        snapshot.b_fault = !b_ok;
        snapshot.b_configured = g_s5_configured;
        snapshot.scan_id_us = g_s5_acquisition.scan_start_us;
        snapshot.scan_completed_us = g_s5_acquisition.completed_us;
        snapshot.forward_cm = g_s5_acquisition.forward_cm;
        snapshot.forward_timestamp_us = g_s5_acquisition.forward_us;
        snapshot.forward_started_us = g_s5_acquisition.forward_started_us;
        snapshot.max_age_us =
            g_s5_config.policy.forward_sample_max_age_ms * 1000U;
        snapshot.b_forward_valid = b_ok &&
            g_s5_acquisition.b_forward_valid && !g_s5_acquisition.b_scan;
        ER error = tk_loc_mtx(g_s5_mutex,TMO_POL);

        if (E_OK == error)
        {
            g_s5_published = snapshot;
            if (E_OK != tk_unl_mtx(g_s5_mutex)) { (void)s5_live_cancel(); }
        }
        else if (E_TMOUT != error) { (void)s5_live_cancel(); }
        if (E_OK != tk_dly_tsk(S5_LIVE_PERIOD_MS))
        {
            s5_acquire_close(&g_s5_acquisition);
            (void)s5_live_cancel();
            tk_ext_tsk(); return;
        }
    }
}

static bool s5_live_cycle (uint64_t now_us, bool * p_scan_requested)
{
    motion_run_status_t motion = {0};
    motion_api_result_t result = motion_api_status(&motion);
    bool b_stationary = (MOTION_API_QUEUED == result) &&
        motion.b_measurement_valid && motion.b_standstill &&
        (0U == motion.active_id) && (MOTION_RUN_READY == motion.state);
    bool b_cancel = (MOTION_API_ERROR == result) ||
        ((MOTION_API_QUEUED == result) && (MOTION_RUN_FAULT == motion.state));

    if (!g_s5_acquisition.b_open)
    {
        s5_acquire_io_t io = {0};

        if (!drv_s5_bind(&g_s5_driver,&g_s5_irq,&io) ||
            !s5_acquire_open(&g_s5_acquisition,&g_s5_config.acquisition,
                &io,now_us)) { return (false); }
    }
    if (!s5_acquire_step(&g_s5_acquisition,now_us,*p_scan_requested,
        b_stationary,b_cancel)) { return (false); }
    if (g_s5_acquisition.b_scan) { *p_scan_requested = false; }
    if (!g_s5_policy.initialised && g_s5_acquisition.b_forward_valid)
    {
        if (S5_LOGIC_OK != s5_runtime_init(&g_s5_policy,&g_s5_config.policy))
        {
            return (false);
        }
    }
    if (g_s5_policy.initialised)
    {
        s5_runtime_input_t input = {0};
        s5_runtime_events_t events = {0};

        input.controller_owned = true;
        ER lock = tk_loc_mtx(g_s5_mutex,TMO_POL);

        if (E_OK == lock)
        {
            input.line_found = (S5_RUNTIME_HOLD == g_s5_policy.status.state) &&
                (0U != g_s5_recovery_pending) &&
                (g_s5_recovery_pending == g_s5_acquisition.scan_start_us);
            if (input.line_found) { g_s5_recovered = g_s5_recovery_pending; }
            g_s5_recovery_pending = 0U;
            if (E_OK != tk_unl_mtx(g_s5_mutex)) { return (false); }
        }
        else if (E_TMOUT != lock) { return (false); }
        input.now_ms = (uint32_t)(now_us / 1000U);
        input.forward_sample_time_ms =
            (uint32_t)(g_s5_acquisition.forward_us / 1000U);
        input.forward_distance_cm = g_s5_acquisition.forward_cm;
        input.forward_valid = g_s5_acquisition.b_forward_valid;
        input.vehicle_stopped = b_stationary;
        input.scan_busy = g_s5_acquisition.b_scan;
        input.scan_complete = g_s5_acquisition.b_scan_complete;
        input.scan_context = &g_s5_acquisition.scan;
        input.coarse_samples = g_s5_acquisition.coarse;
        input.coarse_count = S5_COARSE_SCAN_COUNT;
        input.fine_samples = g_s5_acquisition.fine;
        input.fine_count = g_s5_acquisition.fine_count;
        g_s5_acquisition.scan.evaluated_at_ms = input.now_ms;
        if ((S5_LOGIC_OK != s5_runtime_step(&g_s5_policy,&input,&events)) ||
            events.request_fault) { return (false); }
        if (input.line_found && (S5_RUNTIME_MONITOR == g_s5_policy.status.state))
        {
            g_s5_acquisition.b_scan_complete = false;
            g_s5_acquisition.scan.complete = false;
        }
        *p_scan_requested = *p_scan_requested || events.request_scan;
    }
    return (true);
}

static void s5_live_cleanup (void)
{
    if ((g_s5_task > 0) && (E_OK == tk_del_tsk(g_s5_task))) { g_s5_task = 0; }
    if ((g_s5_fault > 0) && (E_OK == tk_del_flg(g_s5_fault)))
    { g_s5_fault = 0; }
    if ((g_s5_mutex > 0) && (E_OK == tk_del_mtx(g_s5_mutex)))
    { g_s5_mutex = 0; }
}
/*** end of file ***/
