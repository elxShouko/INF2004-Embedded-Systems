/**
 * @file    imu_rtos.c
 * @brief   Buddy 4 IMU / Terrain Task (uT-Kernel 3.0) - see imu_rtos.h.
 *
 * uT-Kernel 3.0 service calls used (TEF020-S004 specification; verified
 * against TRON Forum mtk3_bsp branch pico_rp2040, include/tk/syscall.h):
 *   tk_cre_tsk, tk_sta_tsk, tk_slp_tsk, tk_wup_tsk, tk_can_wup, tk_ext_tsk,
 *   tk_cre_cyc, tk_sta_cyc, tk_stp_cyc, tk_cre_mtx, tk_loc_mtx, tk_unl_mtx,
 *   tk_cre_mbf, tk_snd_mbf, tk_set_flg.
 */

#include <string.h>

#include <tk/tkernel.h>

#include "imu/imu_hal.h"
#include "imu/imu_rtos.h"

/* Message-buffer storage: each message = HEADER (INT) + rounded payload. */
#define RTOS_MSG_ROUND(sz)   ((((uint32_t)(sz)) + (sizeof(INT) - 1U)) \
                              & ~(uint32_t)(sizeof(INT) - 1U))
#define RTOS_MBF_BYTES       (IMU_RTOS_EVENT_Q_DEPTH \
                              * (sizeof(INT) \
                                 + RTOS_MSG_ROUND(sizeof(imu_event_t))))

/* ---- Statically allocated objects (no heap) ----------------------------- */
static uint64_t          g_task_stack[IMU_RTOS_STACK_BYTES / 8U];
static uint32_t          g_mbf_buf[(RTOS_MBF_BYTES + 3U) / 4U];
static imu_terrain_t     g_sys;              /* owned by the IMU task      */
static imu_params_t      g_params;
static imu_rtos_config_t g_cfg;

static imu_status_t      g_shared_status;    /* protected by g_mtxid       */
static uint32_t          g_shared_cmds;      /* protected by g_mtxid       */
static imu_motion_hint_t g_shared_hint;      /* protected by g_mtxid       */
static bool              g_b_shared_hint_new;/* protected by g_mtxid       */
static imu_rtos_stats_t  g_stats;            /* protected by g_mtxid       */

static ID                g_tskid    = 0;
static ID                g_cycid    = 0;
static ID                g_mtxid    = 0;
static ID                g_mbfid    = 0;
static volatile bool     g_b_running = false;
static volatile bool     g_b_stop    = false;

/* ------------------------------------------------------------------------ */
/*  Event sink: terrain_event_q + direct notification                       */
/* ------------------------------------------------------------------------ */

static imu_sink_result_t rtos_send(void * p_ctx, imu_event_t const * p_evt)
{
    ER er;

    (void)p_ctx;

    if (g_mbfid <= 0)
    {
        return (IMU_SINK_ERROR);
    }

    er = tk_snd_mbf(g_mbfid, (CONST void *)p_evt, (INT)sizeof(*p_evt),
                    TMO_POL);
    if (E_OK == er)
    {
        return (IMU_SINK_OK);
    }

    return ((E_TMOUT == er) ? IMU_SINK_FULL : IMU_SINK_ERROR);
}

static void rtos_notify(void * p_ctx, imu_event_t const * p_evt)
{
    UINT ptn = 0U;

    (void)p_ctx;

    if (g_cfg.mc_flag_id <= 0)
    {
        return;
    }

    if (IMU_EVT_IMPACT == p_evt->type)
    {
        ptn = g_cfg.collision_flag_ptn;
    }
    else
    {
        ptn = g_cfg.fault_flag_ptn;
    }

    if (0U != ptn)
    {
        (void)tk_set_flg(g_cfg.mc_flag_id, ptn);   /* never blocks */
    }
}

/* ------------------------------------------------------------------------ */
/*  Cyclic handler (task-independent part): only wakes the task             */
/* ------------------------------------------------------------------------ */

static void imu_cyc_handler(void * p_exinf)
{
    (void)p_exinf;
    (void)tk_wup_tsk(g_tskid);
}

/* ------------------------------------------------------------------------ */
/*  IMU / Terrain Task                                                      */
/* ------------------------------------------------------------------------ */

static void imu_task(INT stacd, void * p_exinf)
{
    TMO      wake_tmo;
    uint32_t cmds;
    uint32_t t0_us;
    uint32_t elapsed_us;
    uint32_t last_tele_ms = 0U;
    uint32_t now_ms;
    INT      queued;
    ER       er;
    bool     b_hint_new;
    imu_motion_hint_t hint;
    imu_telemetry_t   tele;

    (void)stacd;
    (void)p_exinf;

    wake_tmo = (TMO)(g_params.sample_period_ms
                     * (uint32_t)IMU_RTOS_WAKE_TIMEOUT_FACTOR);

    for (;;)
    {
        er = tk_slp_tsk(wake_tmo);

        if (g_b_stop)
        {
            break;
        }

        /* 1. Collect commands and hint (short critical section). */
        cmds       = 0U;
        b_hint_new = false;
        if (E_OK == tk_loc_mtx(g_mtxid, (TMO)IMU_RTOS_LOCK_TMO_MS))
        {
            if (E_TMOUT == er)
            {
                g_stats.missed_wakeups++;
            }
            cmds          = g_shared_cmds;
            g_shared_cmds = 0U;
            if (g_b_shared_hint_new)
            {
                hint                = g_shared_hint;
                b_hint_new          = true;
                g_b_shared_hint_new = false;
            }
            (void)tk_unl_mtx(g_mtxid);
        }

        if (0U != cmds)
        {
            (void)imu_terrain_command(&g_sys, cmds);
        }
        if (b_hint_new)
        {
            (void)imu_terrain_set_hint(&g_sys, &hint);
        }

        /* 2. One sampling period of work (sensor I/O without the mutex). */
        now_ms = imu_hal_time_ms();
        t0_us  = imu_hal_time_us();
        imu_terrain_step(&g_sys, now_ms);
        elapsed_us = imu_hal_time_us() - t0_us;

        /* 3. Publish the snapshot for other tasks. */
        if (E_OK == tk_loc_mtx(g_mtxid, (TMO)IMU_RTOS_LOCK_TMO_MS))
        {
            g_shared_status = g_sys.status;
            g_stats.cycles++;
            if (elapsed_us > g_stats.max_cycle_us)
            {
                g_stats.max_cycle_us = elapsed_us;
            }
            (void)tk_unl_mtx(g_mtxid);
        }
        else
        {
            g_stats.lock_timeouts++;   /* single writer: benign race */
        }

        /* 4. Optional telemetry hook (must not block). */
        if ((NULL != g_cfg.p_telemetry_hook)
            && (g_cfg.telemetry_period_ms > 0U)
            && ((now_ms - last_tele_ms) >= g_cfg.telemetry_period_ms))
        {
            last_tele_ms = now_ms;
            imu_terrain_make_telemetry(&g_sys.status, &tele);
            g_cfg.p_telemetry_hook(&tele);
        }

        /* 5. Overrun detection: wake-ups queued while we were working mean
         *    the cycle took longer than the period. Discard them so the
         *    task re-aligns with the cyclic handler instead of bursting. */
        queued = tk_can_wup(TSK_SELF);
        if (queued > 0)
        {
            g_stats.overruns += (uint32_t)queued;
        }
    }

    /* Clean shutdown: sensor to sleep, bus released. */
    (void)imu_terrain_command(&g_sys, IMU_CMD_STOP);
    imu_terrain_step(&g_sys, imu_hal_time_ms());
    if (E_OK == tk_loc_mtx(g_mtxid, (TMO)IMU_RTOS_LOCK_TMO_MS))
    {
        g_shared_status = g_sys.status;      /* publish STOPPED */
        (void)tk_unl_mtx(g_mtxid);
    }
    g_b_running = false;
    tk_ext_tsk();
}

/* ------------------------------------------------------------------------ */
/*  Start / stop                                                            */
/* ------------------------------------------------------------------------ */

void imu_rtos_default_config(imu_rtos_config_t * p_cfg)
{
    if (NULL == p_cfg)
    {
        return;
    }

    (void)memset(p_cfg, 0, sizeof(*p_cfg));
    p_cfg->task_priority       = IMU_RTOS_DEFAULT_PRIORITY;
    p_cfg->p_params            = NULL;
    p_cfg->terrain_event_q     = 0;
    p_cfg->mc_flag_id          = 0;
    p_cfg->collision_flag_ptn  = 0U;
    p_cfg->fault_flag_ptn      = 0U;
    p_cfg->p_telemetry_hook    = NULL;
    p_cfg->telemetry_period_ms = 0U;
}

static imu_err_t rtos_create_objects(void)
{
    T_CMTX cmtx;
    T_CMBF cmbf;
    T_CTSK ctsk;
    T_CCYC ccyc;

    if (0 == g_mtxid)
    {
        (void)memset(&cmtx, 0, sizeof(cmtx));
        cmtx.mtxatr = TA_TPRI | TA_INHERIT;   /* avoid priority inversion */
        g_mtxid = tk_cre_mtx(&cmtx);
        if (g_mtxid <= 0)
        {
            g_mtxid = 0;
            return (IMU_ERR_RTOS);
        }
    }

    if (g_cfg.terrain_event_q > 0)
    {
        g_mbfid = g_cfg.terrain_event_q;      /* owned by the integrator */
    }
    else if (0 == g_mbfid)
    {
        (void)memset(&cmbf, 0, sizeof(cmbf));
        cmbf.mbfatr = TA_TFIFO | TA_USERBUF;
        cmbf.bufsz  = (SZ)(sizeof(g_mbf_buf));
        cmbf.maxmsz = (INT)sizeof(imu_event_t);
        cmbf.bufptr = (void *)g_mbf_buf;
        g_mbfid = tk_cre_mbf(&cmbf);
        if (g_mbfid <= 0)
        {
            g_mbfid = 0;
            return (IMU_ERR_RTOS);
        }
    }
    else
    {
        /* already created by a previous start */
    }

    if (0 == g_tskid)
    {
        (void)memset(&ctsk, 0, sizeof(ctsk));
        ctsk.exinf   = NULL;
        ctsk.tskatr  = TA_HLNG | TA_RNG0 | TA_USERBUF;
        ctsk.task    = (FP)imu_task;
        ctsk.itskpri = g_cfg.task_priority;
        ctsk.stksz   = (SZ)sizeof(g_task_stack);
        ctsk.bufptr  = (void *)g_task_stack;
        g_tskid = tk_cre_tsk(&ctsk);
        if (g_tskid <= 0)
        {
            g_tskid = 0;
            return (IMU_ERR_RTOS);
        }
    }

    if (0 == g_cycid)
    {
        (void)memset(&ccyc, 0, sizeof(ccyc));
        ccyc.exinf  = NULL;
        ccyc.cycatr = TA_HLNG;                 /* started explicitly */
        ccyc.cychdr = (FP)imu_cyc_handler;
        ccyc.cyctim = (RELTIM)g_params.sample_period_ms;
        ccyc.cycphs = (RELTIM)g_params.sample_period_ms;
        g_cycid = tk_cre_cyc(&ccyc);
        if (g_cycid <= 0)
        {
            g_cycid = 0;
            return (IMU_ERR_RTOS);
        }
    }

    return (IMU_OK);
}

imu_err_t imu_rtos_start(imu_rtos_config_t const * p_cfg)
{
    imu_event_sink_t sink;
    imu_err_t        err;

    if (NULL == p_cfg)
    {
        return (IMU_ERR_PARAM);
    }
    if (g_b_running)
    {
        return (IMU_ERR_STATE);
    }

    g_cfg = *p_cfg;
    if (NULL != p_cfg->p_params)
    {
        g_params = *p_cfg->p_params;
    }
    else
    {
        imu_params_default(&g_params);
    }

    if (IMU_OK != imu_params_validate(&g_params))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_create_objects();
    if (IMU_OK != err)
    {
        return (err);
    }

    sink.p_send   = rtos_send;
    sink.p_notify = rtos_notify;
    sink.p_ctx    = NULL;

    /* No I/O here: the sensor is initialised inside the task. */
    err = imu_terrain_init(&g_sys, &g_params, &sink);
    if (IMU_OK != err)
    {
        return (err);
    }

    g_shared_status     = g_sys.status;
    g_shared_cmds       = 0U;
    g_b_shared_hint_new = false;
    (void)memset(&g_stats, 0, sizeof(g_stats));
    g_b_stop    = false;
    g_b_running = true;

    if (E_OK != tk_sta_tsk(g_tskid, 0))
    {
        g_b_running = false;
        return (IMU_ERR_RTOS);
    }
    if (E_OK != tk_sta_cyc(g_cycid))
    {
        g_b_stop = true;
        (void)tk_wup_tsk(g_tskid);
        return (IMU_ERR_RTOS);
    }

    return (IMU_OK);
}

imu_err_t imu_rtos_stop(void)
{
    if (!g_b_running)
    {
        return (IMU_ERR_STATE);
    }

    (void)tk_stp_cyc(g_cycid);
    g_b_stop = true;
    (void)tk_wup_tsk(g_tskid);   /* task exits on its next wake-up */

    return (IMU_OK);
}

bool imu_rtos_is_running(void)
{
    return (g_b_running);
}

ID imu_rtos_terrain_event_q(void)
{
    return (g_mbfid);
}

/* ------------------------------------------------------------------------ */
/*  Thread-safe accessors                                                   */
/* ------------------------------------------------------------------------ */

static imu_err_t rtos_lock(void)
{
    if (0 == g_mtxid)
    {
        return (IMU_ERR_NOT_READY);
    }

    return ((E_OK == tk_loc_mtx(g_mtxid, (TMO)IMU_RTOS_LOCK_TMO_MS))
            ? IMU_OK : IMU_ERR_BUSY);
}

static void rtos_unlock(void)
{
    (void)tk_unl_mtx(g_mtxid);
}

/** Copy the snapshot under the lock (single copy keeps lock time short). */
static imu_err_t rtos_snapshot(imu_status_t * p_out)
{
    imu_err_t err = rtos_lock();

    if (IMU_OK == err)
    {
        *p_out = g_shared_status;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_rtos_get_stats(imu_rtos_stats_t * p_out)
{
    imu_err_t err;

    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_out = g_stats;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_status(imu_status_t * p_out)
{
    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    return (rtos_snapshot(p_out));
}

imu_err_t imu_get_telemetry(imu_telemetry_t * p_out)
{
    imu_status_t st;
    imu_err_t    err;

    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_snapshot(&st);
    if (IMU_OK == err)
    {
        imu_terrain_make_telemetry(&st, p_out);
    }

    return (err);
}

imu_err_t imu_get_motion(imu_motion_class_t * p_motion,
                         imu_confidence_t * p_conf)
{
    imu_err_t err;

    if ((NULL == p_motion) || (NULL == p_conf))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_motion = g_shared_status.motion;
        *p_conf   = g_shared_status.motion_conf;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_tilt(float * p_pitch_deg, float * p_roll_deg,
                       float * p_tilt_deg, bool * p_b_reliable)
{
    imu_err_t err;

    if ((NULL == p_pitch_deg) || (NULL == p_roll_deg) || (NULL == p_tilt_deg)
        || (NULL == p_b_reliable))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_pitch_deg  = g_shared_status.pitch_deg;
        *p_roll_deg   = g_shared_status.roll_deg;
        *p_tilt_deg   = g_shared_status.tilt_deg;
        *p_b_reliable = g_shared_status.b_attitude_reliable;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_heading(float * p_heading_deg,
                          imu_heading_quality_t * p_quality,
                          float * p_turn_rate_dps, bool * p_b_rate_valid,
                          imu_turn_dir_t * p_turn_dir)
{
    imu_err_t err;

    if ((NULL == p_heading_deg) || (NULL == p_quality)
        || (NULL == p_turn_rate_dps) || (NULL == p_b_rate_valid)
        || (NULL == p_turn_dir))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_heading_deg   = g_shared_status.heading_deg;
        *p_quality       = g_shared_status.heading_quality;
        *p_turn_rate_dps = g_shared_status.turn_rate_dps;
        *p_b_rate_valid  = g_shared_status.b_turn_rate_valid;
        *p_turn_dir      = g_shared_status.turn_dir;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_hump_state(imu_hump_phase_t * p_phase)
{
    imu_err_t err;

    if (NULL == p_phase)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_phase = g_shared_status.hump_phase;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_last_hump(imu_hump_meas_t * p_out)
{
    imu_err_t err;

    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        if (g_shared_status.hump.b_has_last)
        {
            *p_out = g_shared_status.hump.last;
        }
        else
        {
            err = IMU_ERR_NO_DATA;
        }
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_max_hump(imu_hump_meas_t * p_out)
{
    imu_err_t err;

    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        if (g_shared_status.hump.b_has_max)
        {
            *p_out = g_shared_status.hump.max;
        }
        else
        {
            err = IMU_ERR_NO_DATA;
        }
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_health(imu_state_t * p_state, imu_fault_t * p_fault,
                         bool * p_b_sensor_ok)
{
    imu_err_t err;

    if ((NULL == p_state) || (NULL == p_fault) || (NULL == p_b_sensor_ok))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_state       = g_shared_status.state;
        *p_fault       = g_shared_status.fault;
        *p_b_sensor_ok = g_shared_status.b_sensor_ok;
        rtos_unlock();
    }

    return (err);
}

/* ------------------------------------------------------------------------ */
/*  Commands                                                                */
/* ------------------------------------------------------------------------ */

static imu_err_t rtos_post(uint32_t cmd)
{
    imu_err_t err;

    if (!g_b_running)
    {
        return (IMU_ERR_NOT_READY);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        g_shared_cmds |= cmd;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_mission_start(void)
{
    return (rtos_post(IMU_CMD_MISSION_START));
}

imu_err_t imu_request_reset(void)
{
    return (rtos_post(IMU_CMD_RESET));
}

imu_err_t imu_clear_fault(void)
{
    return (rtos_post(IMU_CMD_CLEAR_FAULT));
}

imu_err_t imu_recalibrate(void)
{
    return (rtos_post(IMU_CMD_RECALIBRATE));
}

imu_err_t imu_mag_cal_begin(void)
{
    return (rtos_post(IMU_CMD_MAG_CAL_BEGIN));
}

imu_err_t imu_mag_cal_end(void)
{
    return (rtos_post(IMU_CMD_MAG_CAL_END));
}

imu_err_t imu_set_motion_hint(imu_motion_hint_t const * p_hint)
{
    imu_err_t err;

    if (NULL == p_hint)
    {
        return (IMU_ERR_PARAM);
    }
    if (!g_b_running)
    {
        return (IMU_ERR_NOT_READY);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        g_shared_hint       = *p_hint;
        g_b_shared_hint_new = true;
        rtos_unlock();
    }

    return (err);
}
