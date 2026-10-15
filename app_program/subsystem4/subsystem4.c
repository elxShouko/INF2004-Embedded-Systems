/** @file subsystem4.c
 * Template entry points: preserve monitor objects used by S3, then create
 * Buddy 4 IMU objects. No subsystem task starts during initialization.
 */
#include "subsystem4.h"

/* Published legacy object names are retained for interface compatibility. */
EXPORT ID mbf_summary;
EXPORT ID flg_events;
EXPORT ID alm_monitor;

static ER g_init_result = E_OBJ;

EXPORT ER subsystem4_init (void)
{
    T_CFLG flag = {0};
    T_CMBF buffer = {0};
    T_CALM alarm = {0};

    flag.flgatr = TA_TFIFO | TA_WMUL;
    flg_events = tk_cre_flg(&flag);
    if (flg_events < E_OK)
    {
        return flg_events;
    }
    buffer.mbfatr = TA_TFIFO;
    buffer.bufsz = 4 * S4_SUMMARY_MAX;
    buffer.maxmsz = S4_SUMMARY_MAX;
    mbf_summary = tk_cre_mbf(&buffer);
    if (mbf_summary < E_OK)
    {
        return mbf_summary;
    }
    alarm.almatr = TA_HLNG;
    /* The kernel represents all handler signatures with FP. */
    alarm.almhdr = (FP)monitor_alarm_handler;
    alm_monitor = tk_cre_alm(&alarm);
    if (alm_monitor < E_OK)
    {
        return alm_monitor;
    }
    g_init_result = imu_runtime_init();
    return g_init_result;
}

EXPORT ER subsystem4_start (void)
{
    ER result = g_init_result;
    ID task = 0;

    if (E_OK != result)
    {
        return result;
    }
    result = imu_runtime_start();
    if (E_OK != result)
    {
        return result;
    }
    /* Existing IF-03/04/05 demonstration remains separate from sensing. */
    task = app_start_task(monitor_task, S4_PRI_MONITOR, 0, "monitor_task");
    return (task < E_OK) ? task : E_OK;
}

/* End of file. */

