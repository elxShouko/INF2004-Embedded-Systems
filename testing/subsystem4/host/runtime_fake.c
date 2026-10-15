/** @file runtime_fake.c API checks cannot establish SMP or real-time safety. */
#include "runtime_fake.h"
#include "../../../app_program/subsystem4/drv_imu.h"

#include <stddef.h>
#include <string.h>

static ID create_object (void);

runtime_fake_state_t g_fake;
static FP g_p_task;
static imu_core_status_t g_event;
static bool g_b_event_pending;

void runtime_fake_run (void)
{
    if (NULL != g_p_task)
    {
        g_p_task(0, NULL);
    }
}

ID tk_cre_mtx (const T_CMTX *p_config)
{
    g_fake.b_violation |= (TA_INHERIT != p_config->mtxatr);
    return create_object();
}

ID tk_cre_mbf (const T_CMBF *p_config)
{
    g_fake.b_violation |= (sizeof(g_event) != p_config->maxmsz);
    return create_object();
}

ID tk_cre_flg (const T_CFLG *p_config)
{
    (void)p_config;
    return create_object();
}

ID tk_cre_tsk (const T_CTSK *p_config)
{
    g_p_task = p_config->task;
    g_fake.b_violation |= (19 != p_config->itskpri);
    return create_object();
}

ER tk_sta_tsk (ID task, INT start_code)
{
    (void)task;
    (void)start_code;
    g_fake.b_violation |= (4u != g_fake.create_calls);
    g_fake.b_started = true;
    return g_fake.b_start_failure ? E_SYS : E_OK;
}

ER tk_loc_mtx (ID mutex, TMO timeout)
{
    (void)mutex;
    g_fake.b_violation |= (timeout < 0);
    if (g_fake.b_lock_failure)
    {
        return E_TMOUT;
    }
    g_fake.b_violation |= (0u != g_fake.lock_depth);
    ++g_fake.lock_depth;
    return E_OK;
}

ER tk_unl_mtx (ID mutex)
{
    (void)mutex;
    g_fake.b_violation |= (1u != g_fake.lock_depth);
    --g_fake.lock_depth;
    return E_OK;
}

ER tk_rcv_mbf (ID buffer, void *p_output, TMO timeout)
{
    (void)buffer;
    g_fake.b_violation |= (TMO_POL != timeout);
    if (!g_b_event_pending)
    {
        return E_TMOUT;
    }
    memcpy(p_output, &g_event, sizeof(g_event));
    g_b_event_pending = false;
    return (ER)sizeof(g_event); /* Small fixed message fits ER. */
}

ER tk_snd_mbf (ID buffer, const void *p_data, INT length, TMO timeout)
{
    (void)buffer;
    g_fake.b_violation |= (TMO_POL != timeout) ||
                          ((INT)sizeof(g_event) != length) ||
                          (0u != g_fake.lock_depth);
    ++g_fake.sends;
    if (g_fake.b_full_queue)
    {
        return E_TMOUT;
    }
    memcpy(&g_event, p_data, sizeof(g_event));
    g_b_event_pending = true;
    return E_OK;
}

ER tk_wai_flg (ID flag, UINT mask, UINT mode, UINT *p_pattern, TMO timeout)
{
    (void)flag;
    (void)timeout;
    g_fake.b_violation |= (TWF_ORW != mode);
    *p_pattern = g_fake.flags;
    return (0u != (g_fake.flags & mask)) ? E_OK : E_TMOUT;
}

ER tk_set_flg (ID flag, UINT mask)
{
    (void)flag;
    g_fake.flags |= mask;
    return E_OK;
}

ER tk_dly_tsk (UW delay_ms)
{
    g_fake.b_violation |= (0u != g_fake.lock_depth) || (delay_ms > 20u);
    g_fake.time_ms += delay_ms;
    return g_fake.b_delay_failure ? E_SYS : E_OK;
}

void tk_ext_tsk (void)
{
    ++g_fake.exits;
}

bool drv_imu_open (void)
{
    return !g_fake.b_open_failure;
}

drv_lsm_result_t drv_imu_read (imu_core_sample_t *p_sample)
{
    ++g_fake.samples;
    g_fake.b_violation |= (0u != g_fake.lock_depth);
    if (g_fake.b_stale)
    {
        return DRV_LSM_NOT_READY;
    }
    if (g_fake.b_overrun)
    {
        g_fake.time_ms += 25u;
    }
    p_sample->accel_g[2] = 1.0f;
    return (g_fake.samples > 102u) ? DRV_LSM_IO_ERROR : DRV_LSM_OK;
}

uint32_t drv_imu_time_ms (void)
{
    return g_fake.time_ms;
}

motion_api_result_t motion_api_status (motion_run_status_t * p_status)
{
    if (g_fake.b_motion_busy)
    {
        return MOTION_API_BUSY;
    }
    *p_status = g_fake.motion;
    return MOTION_API_QUEUED;
}

bool motion_api_now (uint64_t * p_now_us)
{
    if (g_fake.b_motion_clock_fail)
    {
        return false;
    }
    *p_now_us = (uint64_t)g_fake.time_ms * 1000u;
    return true;
}

static ID create_object (void)
{
    ++g_fake.create_calls;
    if (g_fake.fail_create == (int32_t)g_fake.create_calls)
    {
        return E_SYS;
    }
    /* Only four objects are created by the unit under test. */
    return (ID)g_fake.create_calls;
}

/* End of file. */

