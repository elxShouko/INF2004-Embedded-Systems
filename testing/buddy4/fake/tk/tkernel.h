/** @file tkernel.h Minimal host-only API facade, NOT a scheduler model. */
#ifndef BUDDY4_FAKE_TKERNEL_H
#define BUDDY4_FAKE_TKERNEL_H

#include <stdint.h>

typedef int32_t ID;
typedef int32_t ER;
typedef int32_t INT;
typedef int32_t PRI;
typedef int32_t TMO;
typedef uint32_t UW;
typedef uint32_t UINT;
typedef uint8_t UB;
typedef void (*FP) (INT, void *);

typedef struct
{
    UW mtxatr;
} T_CMTX;
typedef struct
{
    UW mbfatr;
    UW bufsz;
    UW maxmsz;
} T_CMBF;
typedef struct
{
    UW flgatr;
} T_CFLG;
typedef struct
{
    FP  task;
    PRI itskpri;
    UW  stksz;
    UW  tskatr;
} T_CTSK;

#define IMPORT extern
#define E_OK 0
#define E_SYS (-5)
#define E_PAR (-17)
#define E_OBJ (-41)
#define E_TMOUT (-50)
#define TMO_POL 0
#define TA_INHERIT 1u
#define TA_TFIFO 0u
#define TA_WMUL 8u
#define TA_HLNG 1u
#define TA_RNG3 3u
#define TWF_ORW 1u

ID tk_cre_mtx (const T_CMTX *p_config);
ID tk_cre_mbf (const T_CMBF *p_config);
ID tk_cre_flg (const T_CFLG *p_config);
ID tk_cre_tsk (const T_CTSK *p_config);
ER tk_sta_tsk (ID task, INT start_code);
ER tk_loc_mtx (ID mutex, TMO timeout);
ER tk_unl_mtx (ID mutex);
ER tk_rcv_mbf (ID buffer, void *p_output, TMO timeout);
ER tk_snd_mbf (ID buffer, const void *p_data, INT length, TMO timeout);
ER tk_wai_flg (ID flag, UINT mask, UINT mode, UINT *p_pattern, TMO timeout);
ER tk_set_flg (ID flag, UINT mask);
ER tk_dly_tsk (UW delay_ms);
void tk_ext_tsk (void);

#endif /* BUDDY4_FAKE_TKERNEL_H */

