/**
 * @file    fake_tk.c  (TEST DOUBLE - HOST ONLY)
 * @brief   Fake uT-Kernel service calls for host tests of imu_rtos.c.
 */
#include <setjmp.h>
#include <string.h>

#include <tk/tkernel.h>

#include "imu/imu_rtos.h"
#include "sim_lsm303.h"

fake_tk_t g_fake_tk;
static jmp_buf g_exit_jmp;
static FP      g_saved_task;   /* survives fake_tk_reset(): kernel objects persist */

void fake_tk_reset(void)
{
    (void)memset(&g_fake_tk, 0, sizeof(g_fake_tk));
    g_fake_tk.period_ms    = 20U;
    g_fake_tk.mbf_capacity = 16U;
}

ID tk_cre_tsk(CONST T_CTSK *pk)  { g_fake_tk.ctsk = *pk; g_saved_task = pk->task; g_fake_tk.n_cre_tsk++; return 3; }
ER tk_sta_tsk(ID id, INT stacd)  { (void)id; (void)stacd; g_fake_tk.n_sta_tsk++; return E_OK; }
ER tk_wup_tsk(ID id)             { (void)id; g_fake_tk.n_wup++; return E_OK; }
INT tk_can_wup(ID id)            { (void)id; return g_fake_tk.can_wup_result; }
ID tk_cre_cyc(CONST T_CCYC *pk)  { g_fake_tk.ccyc = *pk; g_fake_tk.n_cre_cyc++; return 5; }
ER tk_sta_cyc(ID id)             { (void)id; g_fake_tk.n_sta_cyc++; return E_OK; }
ER tk_stp_cyc(ID id)             { (void)id; g_fake_tk.n_stp_cyc++; return E_OK; }
ID tk_cre_mtx(CONST T_CMTX *pk)  { g_fake_tk.cmtx = *pk; g_fake_tk.n_cre_mtx++; return 7; }

ER tk_loc_mtx(ID id, TMO tmo)
{
    (void)id; (void)tmo;
    if (g_fake_tk.b_lock_fail) { return E_TMOUT; }
    g_fake_tk.locks++;
    return E_OK;
}

ER tk_unl_mtx(ID id) { (void)id; g_fake_tk.unlocks++; return E_OK; }

ID tk_cre_mbf(CONST T_CMBF *pk)
{
    g_fake_tk.cmbf = *pk;
    g_fake_tk.n_cre_mbf++;
    return (0 != g_fake_tk.cre_mbf_result) ? g_fake_tk.cre_mbf_result : 9;
}

ER tk_snd_mbf(ID id, CONST void *msg, INT sz, TMO tmo)
{
    (void)id;
    if (TMO_POL != tmo) { return E_PAR; }       /* must never block */
    if (sz > g_fake_tk.cmbf.maxmsz && g_fake_tk.cmbf.maxmsz > 0) { return E_PAR; }
    if (g_fake_tk.mbf_level >= g_fake_tk.mbf_capacity) { return E_TMOUT; }
    g_fake_tk.mbf_level++;
    g_fake_tk.mbf_sent++;
    g_fake_tk.mbf_last_size = sz;
    (void)memcpy(g_fake_tk.mbf_last, msg, (size_t)sz);
    return E_OK;
}

INT tk_rcv_mbf(ID id, void *msg, TMO tmo) { (void)id; (void)msg; (void)tmo; return E_TMOUT; }

ER tk_set_flg(ID id, UINT ptn)
{
    g_fake_tk.flg_id = id;
    g_fake_tk.flg_ptn |= ptn;
    g_fake_tk.flg_calls++;
    return E_OK;
}

ER tk_slp_tsk(TMO tmo)
{
    (void)tmo;
    g_fake_tk.slp_calls++;
    if ((g_fake_tk.stop_after > 0U) && (g_fake_tk.slp_calls >= g_fake_tk.stop_after))
    {
        (void)imu_rtos_stop();
    }
    sim_advance_ms(g_fake_tk.period_ms);
    return g_fake_tk.b_slp_timeout ? E_TMOUT : E_OK;
}

void tk_ext_tsk(void)
{
    longjmp(g_exit_jmp, 1);
}

void fake_tk_run_task(void)
{
    if (0 == setjmp(g_exit_jmp))
    {
        void (*p_task)(INT, void *) = (void (*)(INT, void *))g_saved_task;
        if (NULL != p_task)
        {
            p_task(0, NULL);
        }
    }
}
