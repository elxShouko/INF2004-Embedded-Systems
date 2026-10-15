/** @file test_rtos.h
 * @brief Host-only kernel/GPIO doubles; never part of a firmware profile.
 */
#ifndef TEST_RTOS_H
#define TEST_RTOS_H
#if defined(MOTION_RTOS_HOST_TEST)
#if !defined(MOTION_CONFIG_HOST_TEST) || defined(_PICO_RP2040_)
#error "RTOS doubles require host mode and must never target physical hardware"
#endif
#include <stdint.h>
#include <stddef.h>

#ifndef TK_SUPPORT_SMP
#define TK_SUPPORT_SMP 1
#endif
#define CNF_TIMER_PERIOD (10U)
typedef int32_t ER;
typedef int32_t ID;
typedef int32_t INT;
typedef int32_t TMO;
typedef uint32_t UW;
typedef uint32_t UINT;
#define EXPORT
#define IMPORT extern
typedef struct test_rtos_message { struct test_rtos_message * next; } T_MSG;
typedef struct { UINT sematr; UINT isemcnt; UINT maxsem; } T_CSEM;
typedef struct
{
    UINT mpfatr;
    UINT mpfcnt;
    size_t blfsz;
} T_CMPF;
typedef void (*FP)(INT code, void * p_context);
typedef struct { int32_t hi; uint32_t lo; } SYSTIM;
typedef struct { uint32_t mtxatr; } T_CMTX;
typedef struct { uint32_t flgatr; } T_CFLG;
typedef struct { uint32_t flgptn; } T_RFLG;
typedef struct { uint32_t unused; } T_SPLOCK;
typedef struct
{
    FP task;
    INT itskpri;
    uint32_t stksz;
    uint32_t tskatr;
    uint32_t assprc;
    void * bufptr;
} T_CTSK;
#define E_OK (0)
#define E_SYS (-5)
#define E_OBJ (-41)
#define E_LIMIT (-34)
#define E_TMOUT (-50)
#define E_BUSY (-65)
#define TMO_POL (0)
#define TMO_FEVR (-1)
#define TA_INHERIT (1U)
#define TA_TFIFO (0U)
#define TA_HLNG (1U)
#define TA_RNG3 (2U)
#define TA_USERBUF (4U)
#define TA_ASSPRC (8U)
#define TP_PRC2 (2U)
#define TWF_ORW (1U)
#define TWF_CLR (2U)
#define DEVCNF_USE_IIC (0)
#define DEVCONF_ADC_PIN_INIT_2 (0)
#define BOARD_UART0_FREE (1)
#define TM_CONSOLE_UART (0)
#define GPIO_MODE_IN (0U)
#define GPIO_OE (1U)
#define GPIO_IN (2U)
#define GPIO_CTRL(pin) (0x100U + ((pin) * 4U))
#define DI(saved) ((saved) = 0U)
#define EI(saved) ((void)(saved))

ID tk_cre_mtx (T_CMTX const * p_info);
ID tk_cre_flg (T_CFLG const * p_info);
ID tk_cre_tsk (T_CTSK const * p_info);
ER tk_sta_tsk (ID id, INT code);
ER tk_del_tsk (ID id);
ER tk_del_flg (ID id);
ER tk_del_mtx (ID id);
ID tk_cre_sem (T_CSEM const * p_info);
ID tk_cre_mpf (T_CMPF const * p_info);
ER tk_del_sem (ID id);
ER tk_del_mpf (ID id);
ID app_start_task (FP entry, INT priority, UW processor, char const * p_name);
ER subsystem2_init (void);
ER subsystem2_start (void);
ER tk_loc_mtx (ID id, TMO timeout);
ER tk_unl_mtx (ID id);
ER tk_set_flg (ID id, UINT pattern);
ER tk_ref_flg (ID id, T_RFLG * p_info);
ER tk_wai_flg (ID id, UINT pattern, UINT mode, UINT * p_actual, TMO timeout);
ER tk_get_otm (SYSTIM * p_time);
void tk_ext_tsk (void);
ER InitSpinLock (T_SPLOCK * p_lock);
ER ISpinTryLock (T_SPLOCK * p_lock, UINT * p_saved);
ER ISpinUnlock (T_SPLOCK * p_lock, UINT saved);
uint32_t in_w (uint32_t address);
ER gpio_set_pin (UINT pin, UINT mode);
#endif /* Host doubles only. */
#endif /* TEST_RTOS_H */

/*** end of file ***/
