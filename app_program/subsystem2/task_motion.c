/** @file task_motion.c
 * @brief One motor owner, copied mailbox, dedicated latched emergency path.
 */
#if !defined(MOTION_CONFIG_HOST_TEST) || defined(MOTION_RTOS_HOST_TEST)
#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include "task_motion.h"
#include "motion_api.h"
#include "drv_rp2040.h"
#if !defined(MOTION_RTOS_HOST_TEST)
#include <tk/syslib.h>
#if TK_SUPPORT_SMP
#include <tk/smp.h>
#endif
#endif

#define TASK_MOTION_WAKE (1U)
#define TASK_MOTION_ESTOP (1U)
#define TASK_MOTION_SENSOR (2U)
#define TASK_MOTION_INTERNAL (4U)
#define TASK_MOTION_PRIORITY (15) /* Candidate within existing S2 band. */
#define TASK_MOTION_STACK (4096) /* Candidate; target watermark not measured. */

static uint64_t g_task_motion_stack[TASK_MOTION_STACK/sizeof(uint64_t)];
static ID g_task_motion_id;
static ID g_task_motion_mutex;
static ID g_task_motion_wake;
static ID g_task_motion_safety;
static bool g_task_motion_ready;
static motion_bus_t g_task_motion_bus;
static motion_run_t g_task_motion_run;
static enc_capture_t g_task_motion_capture;
static drv_rp2040_t g_task_motion_backend;
static drv_motor_io_t g_task_motion_raw_io;
static drv_encoder_port_t g_task_motion_encoder;
static bool g_task_motion_encoder_open;
#if TK_SUPPORT_SMP
static T_SPLOCK g_task_motion_capture_lock;
#endif

static void task_motion_entry (INT stacd, void * p_exinf);
static ER task_motion_signal (UINT cause);
static UINT task_motion_safety (void);
static bool task_motion_capture_enter (UINT * p_saved);
static void task_motion_capture_leave (UINT saved);
static void task_motion_edges (void * p_context, uint32_t events,
                                uint32_t levels);
static bool task_motion_prepare (void * p_context, uint16_t period,
                                 uint32_t clock_hz);
static bool task_motion_apply (void * p_context, drv_motor_wheel_t wheel,
                               uint16_t level_a, uint16_t level_b);
static bool task_motion_stop_outputs (void * p_context);
static motion_api_result_t task_motion_post (
    motion_bus_request_t const * p_request);
static void task_motion_process (motion_bus_request_t const * p_request,
                                 uint64_t now_us);
static void task_motion_cycle (uint64_t now_us);
static void task_motion_cleanup (void);
static motion_api_result_t task_motion_lock (void);

ER task_motion_init (void)
{
    T_CMTX mutex = {0};
    T_CFLG flag = {0};
    T_CTSK task = {0};
    ER error = E_OBJ;

    if (!g_task_motion_ready && (g_task_motion_id <= 0) &&
        (g_task_motion_mutex <= 0) && (g_task_motion_wake <= 0) &&
        (g_task_motion_safety <= 0))
    {
        mutex.mtxatr = TA_INHERIT;
        g_task_motion_mutex = tk_cre_mtx(&mutex);
        error = (g_task_motion_mutex > 0) ? E_OK : g_task_motion_mutex;
        flag.flgatr = TA_TFIFO;
        if (E_OK == error)
        {
            g_task_motion_wake = tk_cre_flg(&flag);
            error = (g_task_motion_wake > 0) ? E_OK : g_task_motion_wake;
        }
        if (E_OK == error)
        {
            g_task_motion_safety = tk_cre_flg(&flag);
            error = (g_task_motion_safety > 0) ? E_OK : g_task_motion_safety;
        }
        if (E_OK == error)
        {
#if TK_SUPPORT_SMP
            error = InitSpinLock(&g_task_motion_capture_lock);
#else
            error = E_OK;
#endif
            if (E_OK == error)
            {
                task.task = task_motion_entry;
                task.itskpri = TASK_MOTION_PRIORITY;
                task.stksz = TASK_MOTION_STACK;
                task.tskatr = TA_HLNG | TA_RNG3 | TA_USERBUF;
                task.bufptr = g_task_motion_stack;
#if TK_SUPPORT_SMP
                task.tskatr |= TA_ASSPRC;
                task.assprc = TP_PRC2;
#endif
                g_task_motion_id = tk_cre_tsk(&task);
                error = (g_task_motion_id > 0) ? E_OK : g_task_motion_id;
            }
        }
        if (E_OK != error)
        {
            task_motion_cleanup();
        }
        g_task_motion_ready = (E_OK == error);
    }
    return (error);
}

ER task_motion_start (void)
{
    return (g_task_motion_ready ? tk_sta_tsk(g_task_motion_id,0) : E_OBJ);
}

bool motion_api_now (uint64_t * p_now_us)
{
    SYSTIM time = {0};
    bool b_ok = (NULL != p_now_us) && (E_OK == tk_get_otm(&time)) &&
                (time.hi >= 0);

    if (b_ok)
    {
        uint64_t ms = ((uint64_t)(UW)time.hi << 32U) | time.lo;

        b_ok = (ms <= (UINT64_MAX/1000U));
        if (b_ok)
        {
            *p_now_us = ms*1000U;
        }
    }
    return (b_ok);
}

motion_api_result_t motion_api_configure (wheel_loop_config_t const * p_config,
    drv_encoder_port_t const * p_encoder_port)
{
    motion_api_result_t result = MOTION_API_INVALID;

    if ((wheel_loop_validate(p_config)) && (NULL != p_encoder_port) &&
        (NULL != p_encoder_port->p_reserve) &&
        (NULL != p_encoder_port->p_enable) &&
        (NULL != p_encoder_port->p_release) &&
        (0U == (p_config->period_us % (CNF_TIMER_PERIOD*1000U))))
    {
        motion_bus_request_t request = {0};

        request.kind = MOTION_BUS_CONFIG;
        request.config = *p_config;
        request.encoder_port = *p_encoder_port;
        result = task_motion_post(&request);
    }
    return (result);
}

motion_api_result_t motion_api_enable (void)
{
    motion_bus_request_t request = {0};

    request.kind = MOTION_BUS_ENABLE;
    return (task_motion_post(&request));
}

motion_api_result_t motion_api_submit (move_plan_command_t const * p_command)
{
    motion_api_result_t result = MOTION_API_INVALID;

    if ((NULL != p_command) && (0U != p_command->id) &&
        (0U != p_command->lifetime_us))
    {
        motion_bus_request_t request = {0};

        request.kind = MOTION_BUS_COMMAND;
        request.command = *p_command;
        result = task_motion_post(&request);
    }
    return (result);
}

motion_api_result_t motion_api_stop (void)
{
    return (motion_api_cancel(0U));
}

motion_api_result_t motion_api_cancel (uint32_t id)
{
    motion_api_result_t result = MOTION_API_UNAVAILABLE;

    if (g_task_motion_ready)
    {
        result = task_motion_lock();
        if (MOTION_API_QUEUED == result)
        {
            bool b_ok = motion_bus_stop(&g_task_motion_bus,(0U!=id),id);
            ER error = tk_unl_mtx(g_task_motion_mutex);

            result = b_ok ? MOTION_API_QUEUED : MOTION_API_INVALID;
            if ((E_OK != error) || (E_OK != tk_set_flg(g_task_motion_wake,1U)))
            {
                (void)task_motion_signal(TASK_MOTION_INTERNAL);
                result = MOTION_API_ERROR;
            }
        }
    }
    return (result);
}

motion_api_result_t motion_api_ack (uint32_t terminal_id)
{
    motion_bus_request_t request = {0};

    request.kind = MOTION_BUS_ACK;
    request.ack_id = terminal_id;
    return (task_motion_post(&request));
}

motion_api_result_t motion_api_status (motion_run_status_t * p_status)
{
    motion_api_result_t result = MOTION_API_UNAVAILABLE;

    if (NULL == p_status)
    {
        result = MOTION_API_INVALID;
    }
    else if (g_task_motion_ready)
    {
        result = task_motion_lock();
        if (MOTION_API_QUEUED == result)
        {
            *p_status = g_task_motion_bus.status;
            result = (E_OK == tk_unl_mtx(g_task_motion_mutex)) ?
                      MOTION_API_QUEUED : MOTION_API_ERROR;
            if (MOTION_API_ERROR == result)
            {
                (void)task_motion_signal(TASK_MOTION_INTERNAL);
            }
            if (p_status->b_measurement_valid)
            {
                uint64_t now_us = 0U;
                bool b_fresh = motion_api_now(&now_us) &&
                    (now_us >= p_status->measurement_timestamp_us) &&
                    ((now_us-p_status->measurement_timestamp_us) <=
                     p_status->measurement_max_age_us);

                p_status->b_measurement_valid = b_fresh;
                p_status->b_standstill = p_status->b_standstill && b_fresh;
            }
        }
    }
    return (result);
}

motion_api_result_t motion_api_estop (void)
{
    return ((E_OK == task_motion_signal(TASK_MOTION_ESTOP)) ?
            MOTION_API_QUEUED : MOTION_API_ERROR);
}

/** @brief Emergency/fault posts never acquire the command/status mutex. */
static ER task_motion_signal (UINT cause)
{
    ER error = E_OBJ;

    if (g_task_motion_ready)
    {
        error = tk_set_flg(g_task_motion_safety,cause);
        ER wake_error = tk_set_flg(g_task_motion_wake,TASK_MOTION_WAKE);

        if (E_OK == error)
        {
            error = wake_error;
        }
    }
    return (error);
}

static UINT task_motion_safety (void)
{
    T_RFLG flag = {0};

    return ((E_OK == tk_ref_flg(g_task_motion_safety,&flag)) ?
            flag.flgptn : TASK_MOTION_INTERNAL);
}

static bool task_motion_capture_enter (UINT * p_saved)
{
#if TK_SUPPORT_SMP
    return (E_OK == ISpinTryLock(&g_task_motion_capture_lock,p_saved));
#else
    DI(*p_saved);
    return (true);
#endif
}

static void task_motion_capture_leave (UINT saved)
{
#if TK_SUPPORT_SMP
    (void)ISpinUnlock(&g_task_motion_capture_lock,saved);
#else
    EI(saved);
#endif
}

static void task_motion_edges (void * p_context, uint32_t events,
                                uint32_t levels)
{
    UINT saved;
    bool b_ok = false;

    (void)p_context;
    if (task_motion_capture_enter(&saved))
    {
        b_ok = enc_capture_feed(&g_task_motion_capture,events,levels);
        task_motion_capture_leave(saved);
    }
    if (!b_ok)
    {
        (void)task_motion_signal(TASK_MOTION_SENSOR);
    }
}

/** @brief Re-check dedicated safety latch before preparation/each output. */
static bool task_motion_prepare (void * p_context, uint16_t period,
                                 uint32_t clock_hz)
{
    (void)p_context;
    return ((0U == task_motion_safety()) &&
        (g_task_motion_raw_io.p_prepare(g_task_motion_raw_io.p_context,
                                        period,clock_hz)));
}

static bool task_motion_apply (void * p_context, drv_motor_wheel_t wheel,
                               uint16_t level_a, uint16_t level_b)
{
    (void)p_context;
    if (0U != task_motion_safety())
    {
        (void)task_motion_stop_outputs(NULL);
        return (false);
    }
    return (g_task_motion_raw_io.p_apply(g_task_motion_raw_io.p_context,
                                         wheel,level_a,level_b));
}

static bool task_motion_stop_outputs (void * p_context)
{
    (void)p_context;
    return (g_task_motion_raw_io.p_stop_all(g_task_motion_raw_io.p_context));
}

static motion_api_result_t task_motion_post (
    motion_bus_request_t const * p_request)
{
    motion_api_result_t result = MOTION_API_UNAVAILABLE;

    if (g_task_motion_ready)
    {
        result = task_motion_lock();
        if (MOTION_API_QUEUED == result)
        {
            bool b_ok = motion_bus_put(&g_task_motion_bus,p_request);
            ER error = tk_unl_mtx(g_task_motion_mutex);

            result = b_ok ? MOTION_API_QUEUED : MOTION_API_BUSY;
            if ((E_OK != error) ||
                (b_ok && (E_OK != tk_set_flg(g_task_motion_wake,1U))))
            {
                (void)task_motion_signal(TASK_MOTION_INTERNAL);
                result = MOTION_API_ERROR;
            }
        }
    }
    return (result);
}

static void task_motion_process (motion_bus_request_t const * p_request,
                                 uint64_t now_us)
{
    switch (p_request->kind)
    {
        case MOTION_BUS_CONFIG:
        {
            drv_motor_io_t io = {NULL,task_motion_prepare,task_motion_apply,
                                 task_motion_stop_outputs};
            bool b_ok = drv_rp2040_bind(&g_task_motion_backend,
                                       &g_task_motion_raw_io);

            b_ok = (motion_run_init(&g_task_motion_run,&p_request->config,
                                   &io)) && b_ok;
            g_task_motion_encoder = p_request->encoder_port;
            if (!b_ok)
            {
                motion_run_emergency(&g_task_motion_run,MOTION_RUN_OUTPUT,
                                     now_us);
            }
        }
        break;
        case MOTION_BUS_ENABLE:
        {
            UINT saved;
            bool b_ok = task_motion_capture_enter(&saved);

            if (b_ok)
            {
                b_ok = enc_capture_init(&g_task_motion_capture,
                    &g_task_motion_run.loop.config.geometry,1U);
                task_motion_capture_leave(saved);
            }
            if (b_ok)
            {
                b_ok = drv_encoder_open(&g_task_motion_encoder,
                                         task_motion_edges,NULL);
                g_task_motion_encoder_open = b_ok;
            }
            if (b_ok && (0U == task_motion_safety()))
            {
                b_ok = motion_run_enable(&g_task_motion_run,now_us);
            }
            else
            {
                b_ok = false;
            }
            if (!b_ok)
            {
                motion_run_emergency(&g_task_motion_run,MOTION_RUN_SENSOR,
                                     now_us);
            }
        }
        break;
        case MOTION_BUS_COMMAND:
            if (!motion_run_command(&g_task_motion_run,&p_request->command,
                                    now_us))
            {
                if ((0U == g_task_motion_run.status.active_id) &&
                    (0U == g_task_motion_run.status.terminal_id))
                {
                    g_task_motion_run.status.terminal_id=p_request->command.id;
                    g_task_motion_run.status.terminal_result =
                        MOTION_RUN_REJECTED;
                }
            }
        break;
        case MOTION_BUS_ACK:
            (void)motion_run_ack(&g_task_motion_run,p_request->ack_id);
        break;
        default:
            motion_run_emergency(&g_task_motion_run,MOTION_RUN_REJECTED,now_us);
        break;
    }
}

static void task_motion_cycle (uint64_t now_us)
{
    motion_bus_request_t request = {0};
    bool b_present=false, b_stop=false, b_cancel=false;
    UINT safety = task_motion_safety();

    if (0U != safety)
    {
        motion_run_emergency(&g_task_motion_run,
            (0U!=(safety&TASK_MOTION_ESTOP)) ? MOTION_RUN_EMERGENCY :
            ((0U!=(safety&TASK_MOTION_SENSOR)) ? MOTION_RUN_SENSOR :
                                               MOTION_RUN_OUTPUT),now_us);
    }
    else
    {
        if (MOTION_API_QUEUED == task_motion_lock())
        {
            motion_bus_take(&g_task_motion_bus,&request,&b_present,
                            &b_stop,&b_cancel);
            if (E_OK != tk_unl_mtx(g_task_motion_mutex))
            {
                (void)task_motion_signal(TASK_MOTION_INTERNAL);
            }
        }
        if (b_present)
        {
            task_motion_process(&request,now_us);
        }
        if (b_stop)
        {
            motion_run_stop(&g_task_motion_run,b_cancel,now_us);
        }
        if (g_task_motion_run.loop.b_enabled)
        {
            UINT saved;
            enc_capture_sample_t sample = {0};
            bool b_ok = task_motion_capture_enter(&saved);

            if (b_ok)
            {
                b_ok = enc_capture_get(&g_task_motion_capture,now_us,&sample);
                task_motion_capture_leave(saved);
            }
            if (b_ok)
            {
                (void)motion_run_tick(&g_task_motion_run,&sample,now_us);
            }
            else
            {
                motion_run_emergency(&g_task_motion_run,MOTION_RUN_SENSOR,
                                     now_us);
            }
        }
    }
    if ((MOTION_RUN_FAULT == g_task_motion_run.status.state) &&
        g_task_motion_encoder_open)
    {
        drv_encoder_close(&g_task_motion_encoder);
        g_task_motion_encoder_open=false;
    }
    if (MOTION_API_QUEUED == task_motion_lock())
    {
        g_task_motion_run.status.timestamp_us=now_us;
        g_task_motion_bus.status=g_task_motion_run.status;
        g_task_motion_bus.b_inflight=false;
        if (E_OK != tk_unl_mtx(g_task_motion_mutex))
        {
            (void)task_motion_signal(TASK_MOTION_INTERNAL);
        }
    }
}

static void task_motion_entry (INT stacd, void * p_exinf)
{
    UINT events;

    (void)stacd;(void)p_exinf;
    for (;;)
    {
        uint64_t now_us;
        TMO wait_ms = g_task_motion_run.b_initialised ?
            (TMO)(g_task_motion_run.loop.config.period_us/1000U) : TMO_FEVR;
        if (g_task_motion_run.loop.b_enabled && motion_api_now(&now_us))
        {
            uint64_t last_us = g_task_motion_run.loop.last_tick_us;
            uint64_t period_us = g_task_motion_run.loop.config.period_us;

            if (now_us < last_us)
            {
                wait_ms = TMO_POL;
            }
            else
            {
                uint64_t elapsed_us = now_us-last_us;

                /* A command wake cannot postpone the next control deadline. */
                wait_ms = (elapsed_us >= period_us) ? TMO_POL :
                    (TMO)((period_us-elapsed_us+999U)/1000U);
            }
        }
        ER error = tk_wai_flg(g_task_motion_wake,1U,TWF_ORW|TWF_CLR,
                              &events,wait_ms);

        if (((E_OK != error) && (E_TMOUT != error)) ||
            !(motion_api_now(&now_us)))
        {
            motion_run_emergency(&g_task_motion_run,MOTION_RUN_OUTPUT,
                                 g_task_motion_run.status.timestamp_us);
            if (g_task_motion_encoder_open)
            {
                drv_encoder_close(&g_task_motion_encoder);
                g_task_motion_encoder_open = false;
            }
            if (MOTION_API_QUEUED == task_motion_lock())
            {
                g_task_motion_bus.status = g_task_motion_run.status;
                (void)tk_unl_mtx(g_task_motion_mutex);
            }
            (void)task_motion_signal(TASK_MOTION_INTERNAL);
            tk_ext_tsk();
            return;
        }
        task_motion_cycle(now_us);
    }
}

static void task_motion_cleanup (void)
{
    if (g_task_motion_id > 0)
    {
        if (E_OK == tk_del_tsk(g_task_motion_id))
        {
            g_task_motion_id = 0;
        }
    }
    if (g_task_motion_safety > 0)
    {
        if (E_OK == tk_del_flg(g_task_motion_safety))
        {
            g_task_motion_safety = 0;
        }
    }
    if (g_task_motion_wake > 0)
    {
        if (E_OK == tk_del_flg(g_task_motion_wake))
        {
            g_task_motion_wake = 0;
        }
    }
    if (g_task_motion_mutex > 0)
    {
        if (E_OK == tk_del_mtx(g_task_motion_mutex))
        {
            g_task_motion_mutex = 0;
        }
    }
}

static motion_api_result_t task_motion_lock (void)
{
    ER error = tk_loc_mtx(g_task_motion_mutex,TMO_POL);
    motion_api_result_t result = MOTION_API_QUEUED;

    if (E_TMOUT == error)
    {
        result = MOTION_API_BUSY;
    }
    else if (E_OK != error)
    {
        (void)task_motion_signal(TASK_MOTION_INTERNAL);
        result = MOTION_API_ERROR;
    }
    return (result);
}
#else
typedef int task_motion_host_excluded_t;
#endif /* Actual micro T-Kernel firmware, never host hardware. */

/*** end of file ***/

