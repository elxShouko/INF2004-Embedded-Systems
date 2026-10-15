/**
 * @file    tkernel.h  (TEST DOUBLE - HOST ONLY)
 * @brief   Minimal fake of the uT-Kernel 3.0 API subset used by imu_rtos.c.
 *
 * Types, structure layouts and constants mirror include/tk/typedef.h,
 * include/tk/syscall.h and include/tk/errno.h of the TRON Forum uT-Kernel
 * 3.0 RP2040 BSP (with USE_OBJECT_NAME = 0). This file is only on the
 * include path of the host test build; the firmware build uses the real
 * kernel headers. It lets the RTOS wrapper logic (object creation,
 * locking, queue overflow mapping, overrun accounting, shutdown) be
 * exercised without hardware.
 */
#ifndef FAKE_TK_TKERNEL_H
#define FAKE_TK_TKERNEL_H

#include <stdbool.h>
#include <stdint.h>

#define CONST const
#define LOCAL static
#define EXPORT

typedef int32_t  INT;
typedef uint32_t UINT;
typedef int32_t  W;
typedef uint32_t UW;
typedef INT      ID;
typedef INT      PRI;
typedef INT      ER;
typedef INT      SZ;
typedef W        TMO;
typedef UW       RELTIM;
typedef UW       ATR;
typedef void     (*FP)(void);

#define TSK_SELF    0
#define TMO_POL     0
#define TMO_FEVR    (-1)

#define TA_HLNG     0x00000001UL
#define TA_USERBUF  0x00000020UL
#define TA_RNG0     0x00000000UL
#define TA_TFIFO    0x00000000UL
#define TA_TPRI     0x00000001UL
#define TA_INHERIT  0x00000002UL
#define TA_STA      0x00000002UL

#define E_OK        (0)
#define E_PAR       (-17)
#define E_NOMEM     (-33)
#define E_QOVR      (-43)
#define E_TMOUT     (-50)

typedef struct t_ctsk {
    void   *exinf;
    ATR     tskatr;
    FP      task;
    PRI     itskpri;
    SZ      stksz;
    void   *bufptr;
} T_CTSK;

typedef struct t_cmtx {
    void   *exinf;
    ATR     mtxatr;
    PRI     ceilpri;
} T_CMTX;

typedef struct t_cmbf {
    void   *exinf;
    ATR     mbfatr;
    SZ      bufsz;
    INT     maxmsz;
    void   *bufptr;
} T_CMBF;

typedef struct t_ccyc {
    void   *exinf;
    ATR     cycatr;
    FP      cychdr;
    RELTIM  cyctim;
    RELTIM  cycphs;
} T_CCYC;

ID  tk_cre_tsk(CONST T_CTSK *pk_ctsk);
ER  tk_sta_tsk(ID tskid, INT stacd);
ER  tk_slp_tsk(TMO tmout);
ER  tk_wup_tsk(ID tskid);
INT tk_can_wup(ID tskid);
void tk_ext_tsk(void);
ID  tk_cre_cyc(CONST T_CCYC *pk_ccyc);
ER  tk_sta_cyc(ID cycid);
ER  tk_stp_cyc(ID cycid);
ID  tk_cre_mtx(CONST T_CMTX *pk_cmtx);
ER  tk_loc_mtx(ID mtxid, TMO tmout);
ER  tk_unl_mtx(ID mtxid);
ID  tk_cre_mbf(CONST T_CMBF *pk_cmbf);
ER  tk_snd_mbf(ID mbfid, CONST void *msg, INT msgsz, TMO tmout);
INT tk_rcv_mbf(ID mbfid, void *msg, TMO tmout);
ER  tk_set_flg(ID flgid, UINT setptn);

/* ---- test control ---- */
typedef struct
{
    T_CTSK   ctsk;
    T_CMTX   cmtx;
    T_CMBF   cmbf;
    T_CCYC   ccyc;
    uint32_t n_cre_tsk, n_cre_mtx, n_cre_mbf, n_cre_cyc;
    uint32_t n_sta_tsk, n_sta_cyc, n_stp_cyc, n_wup;
    ER       cre_mbf_result;      /* 0 = success */
    bool     b_lock_fail;
    bool     b_slp_timeout;
    INT      can_wup_result;
    uint32_t slp_calls;
    uint32_t stop_after;          /* call imu_rtos_stop() after N sleeps */
    uint32_t period_ms;           /* sim time advanced per sleep */
    uint32_t mbf_capacity;
    uint32_t mbf_level;
    uint32_t mbf_sent;
    uint8_t  mbf_last[64];
    INT      mbf_last_size;
    ID       flg_id;
    UINT     flg_ptn;
    uint32_t flg_calls;
    uint32_t locks;
    uint32_t unlocks;
} fake_tk_t;

extern fake_tk_t g_fake_tk;

void fake_tk_reset(void);
/** Run the pinned task body, whose final statement calls tk_ext_tsk(). */
void fake_tk_run_task(void);

#endif /* FAKE_TK_TKERNEL_H */
