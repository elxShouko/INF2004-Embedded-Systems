/** @file imu_runtime.c
 * Creates all objects before starting tasks. One task owns sensor and core;
 * short priority-inheritance locks protect copied snapshots on both cores.
 */
#define PROHIBIT_DEF_SIZE_T 1
#include <stddef.h>
#include "imu_runtime.h"
#include "imu_config.h"
#include "drv_imu.h"
#include "imu_link.h"
#include "imu_packed.h"

#include <math.h>

#define IMU_FAULT_BIT 0x01u
#define IMU_LOCK_TIMEOUT_MS 10
typedef struct
{
    uint32_t epoch;
    uint8_t  b_trusted : 1;
} imu_runtime_trust_t;

typedef struct
{
    imu_packed_status_t sensing;
    uint32_t            dropped_events;
    uint8_t             b_enabled : 1;
} imu_runtime_shared_t;

static void imu_runtime_task (INT start_code, void *p_context);
static ER publish_status (const imu_core_t *p_core);
static void signal_fault (void);
static bool read_distance (imu_core_sample_t *p_sample,
                           uint32_t previous_ms, float32_t wheel_limit_mm_s);
static void sample_loop (imu_core_t *p_core);

static ID g_mutex = 0;
static ID g_events = 0;
static ID g_fault = 0;
static ID g_task = 0;
static imu_runtime_shared_t g_status;
static imu_runtime_trust_t g_trust;

ER imu_runtime_init (void)
{
    T_CMTX mutex = {0};
    T_CMBF buffer = {0};
    T_CFLG flag = {0};
    T_CTSK task = {0};

    /* Lifecycle functions: initial task only, once, before consumers run. */
    if (0 != g_mutex)
    {
        return E_OBJ;
    }
    mutex.mtxatr = TA_INHERIT;
    g_mutex = tk_cre_mtx(&mutex);
    if (g_mutex < E_OK)
    {
        return g_mutex;
    }
    flag.flgatr = TA_TFIFO | TA_WMUL;
    g_fault = tk_cre_flg(&flag);
    if (g_fault < E_OK)
    {
        return g_fault;
    }
    buffer.mbfatr = TA_TFIFO;
    /* Include kernel per-message size overhead, rounded up to UW. */
    buffer.bufsz = S4_IMU_EVENT_DEPTH * (sizeof(imu_packed_status_t) +
                    (2u * sizeof(UW)));
    buffer.maxmsz = sizeof(imu_packed_status_t);
    g_events = tk_cre_mbf(&buffer);
    if (g_events < E_OK)
    {
        return g_events;
    }
    {
        const imu_core_status_t initial =
        {
            .phase = IMU_CALIBRATING
        };

        g_status.b_enabled = (0 != S4_IMU_ENABLE);
        if (!imu_packed_encode(&initial, &g_status.sensing))
        {
            return E_SYS;
        }
    }
    task.task = (FP)imu_runtime_task; /* Required RTOS callback convention. */
    task.itskpri = S4_IMU_PRIORITY;
    task.stksz = TASK_STACK_SIZE;
    task.tskatr = TA_HLNG | TA_RNG3;
    g_task = tk_cre_tsk(&task);
    return (g_task < E_OK) ? g_task : E_OK;
}

ER imu_runtime_start (void)
{
    if (g_task <= 0)
    {
        return E_OBJ;
    }
    /* Unlike app_start_task(), propagate tk_sta_tsk failure to caller. */
    return tk_sta_tsk(g_task, 0);
}

ER imu_runtime_get_status (imu_runtime_status_t *p_status)
{
    imu_runtime_shared_t shared = {0};
    ER result = E_PAR;

    if ((NULL == p_status) || (g_task <= 0))
    {
        return result;
    }
    result = tk_loc_mtx(g_mutex, IMU_LOCK_TIMEOUT_MS);
    if (E_OK == result)
    {
        shared = g_status;
        result = tk_unl_mtx(g_mutex);
    }
    if ((E_OK == result) &&
        (!imu_packed_decode(&shared.sensing, &p_status->sensing)))
    {
        result = E_SYS;
    }
    if (E_OK == result)
    {
        p_status->dropped_events = shared.dropped_events;
        p_status->b_enabled = shared.b_enabled;
    }
    return result;
}

ER imu_runtime_get_event (imu_core_status_t *p_event)
{
    imu_packed_status_t packed = {0};
    ER result = E_PAR;

    if (NULL != p_event)
    {
        result = tk_rcv_mbf(g_events, &packed, TMO_POL);
        if ((ER)sizeof(packed) == result)
        {
            result = imu_packed_decode(&packed, p_event) ? E_OK : E_SYS;
        }
        else if (result >= E_OK)
        {
            result = E_SYS;
        }
        else
        {
            /* Preserve timeout or kernel error for the caller. */
        }
    }
    return result;
}

ER imu_runtime_wait_fault (TMO timeout)
{
    UINT pattern = 0u;

    return tk_wai_flg(g_fault, IMU_FAULT_BIT, TWF_ORW, &pattern, timeout);
}

ER imu_runtime_set_speed (float32_t speed_mm_s, uint32_t time_ms,
                          bool b_valid)
{
    (void)speed_mm_s;
    (void)time_ms;
    const ER result = imu_runtime_set_motion_trust(false);

    return ((E_OK == result) && b_valid) ? E_PAR : result;
}

ER imu_runtime_set_motion_trust (bool b_trusted)
{
    ER result = E_OBJ;

    if (g_mutex <= 0)
    {
        return result;
    }
    result = tk_loc_mtx(g_mutex, IMU_LOCK_TIMEOUT_MS);
    if (E_OK == result)
    {
        if (!b_trusted)
        {
            if (UINT32_MAX == g_trust.epoch)
            {
                g_trust.b_trusted = false;
                result = E_OBJ;
            }
            else
            {
                ++g_trust.epoch;
            }
        }
        if (E_OK == result)
        {
            g_trust.b_trusted = b_trusted;
        }
        if (E_OK != tk_unl_mtx(g_mutex))
        {
            result = E_SYS;
        }
    }
    return result;
}

static void imu_runtime_task (INT start_code, void *p_context)
{
    imu_core_t core = {0};
    imu_core_config_t config = imu_core_defaults();

    (void)start_code;
    (void)p_context;
    config.b_mag_calibrated = S4_MAG_CALIBRATED;
    config.mag_offset[0] = S4_MAG_OFFSET_X;
    config.mag_offset[1] = S4_MAG_OFFSET_Y;
    config.mag_offset[2] = S4_MAG_OFFSET_Z;
    config.mag_scale[0] = S4_MAG_SCALE_X;
    config.mag_scale[1] = S4_MAG_SCALE_Y;
    config.mag_scale[2] = S4_MAG_SCALE_Z;
    config.b_height_calibrated = S4_HEIGHT_CALIBRATED;
    config.maximum_accel_mm_s2 = S4_HEIGHT_MAX_ACCEL_MM_S2;
    config.speed_band_mm_s = S4_HEIGHT_SPEED_BAND_MM_S;
    config.wheel_delta_mm_s = S4_HEIGHT_WHEEL_DELTA_MM_S;
    config.minimum_speed_mm_s = S4_HEIGHT_MIN_SPEED_MM_S;
    config.settle_ms = S4_HEIGHT_SETTLE_MS;
    config.ascent_ms = S4_HEIGHT_ASCENT_MS;
    config.level_ms = S4_HEIGHT_LEVEL_MS;
    if (0 == S4_IMU_ENABLE)
    {
        tk_ext_tsk();
        return;
    }
    if ((!imu_core_init(&core, &config)) || (!drv_imu_open()))
    {
        imu_core_fault(&core);
        signal_fault();
        if (E_OK != publish_status(&core))
        {
            signal_fault();
        }
    }
    else
    {
        sample_loop(&core);
    }
    tk_ext_tsk();
}

static void sample_loop (imu_core_t *p_core)
{
    const uint32_t startup_ms = drv_imu_time_ms();
    uint32_t last_sample_ms = startup_ms;
    bool b_running = true;

    while (b_running)
    {
        const uint32_t start_ms = drv_imu_time_ms();
        imu_core_sample_t sample = {0};
        const drv_lsm_result_t result = drv_imu_read(&sample);
        uint32_t work_ms = 0u;

        sample.time_ms = drv_imu_time_ms();
        if (DRV_LSM_OK == result)
        {
            if (read_distance(&sample, last_sample_ms,
                              p_core->config.wheel_delta_mm_s))
            {
                (void)imu_core_update(p_core, &sample);
                last_sample_ms = sample.time_ms;
            }
            else
            {
                imu_core_fault(p_core);
            }
        }
        else if ((DRV_LSM_NOT_READY != result) ||
                 ((sample.time_ms - last_sample_ms) >
                  p_core->config.maximum_gap_ms))
        {
            imu_core_fault(p_core);
        }
        else
        {
            p_core->status.events = 0u;
        }
        if ((IMU_CALIBRATING == p_core->status.phase) &&
            ((sample.time_ms - startup_ms) > S4_IMU_CAL_TIMEOUT_MS))
        {
            imu_core_fault(p_core);
        }
        if (IMU_FAULT == p_core->status.phase)
        {
            signal_fault(); /* Before potentially full ordinary queue. */
            b_running = false;
        }
        if (E_OK != publish_status(p_core))
        {
            signal_fault();
            b_running = false;
        }
        work_ms = drv_imu_time_ms() - start_ms;
        if (b_running && (work_ms >= S4_IMU_PERIOD_MS))
        {
            signal_fault();
            b_running = false;
        }
        if (b_running &&
            (E_OK != tk_dly_tsk(S4_IMU_PERIOD_MS - work_ms)))
        {
            signal_fault();
            b_running = false;
        }
    }
    /* Timing/kernel errors must invalidate the snapshot as well as latch. */
    imu_core_fault(p_core);
    if (E_OK != publish_status(p_core))
    {
        signal_fault();
    }
}

static ER publish_status (const imu_core_t *p_core)
{
    imu_packed_status_t packed = {0};
    ER result = E_OK;
    bool b_dropped = false;

    if (!imu_packed_encode(&p_core->status, &packed))
    {
        return E_SYS;
    }

    if (0u != p_core->status.events)
    {
        result = tk_snd_mbf(g_events, &packed, sizeof(packed), TMO_POL);
        b_dropped = (E_TMOUT == result);
        if ((!b_dropped) && (E_OK != result))
        {
            return result;
        }
    }
    result = tk_loc_mtx(g_mutex, IMU_LOCK_TIMEOUT_MS);
    if (E_OK == result)
    {
        g_status.sensing = packed;
        if (b_dropped && (UINT32_MAX != g_status.dropped_events))
        {
            /* Drop newest; the status snapshot stays current. */
            ++g_status.dropped_events;
        }
        result = tk_unl_mtx(g_mutex);
    }
    return result;
}

static void signal_fault (void)
{
    const ER result = tk_set_flg(g_fault, IMU_FAULT_BIT);

    if (E_OK != result)
    {
        /* No safe local repair of a broken kernel object. Task exits;
         * consumer must treat timeout/stale samples/API errors as faults. */
        tk_ext_tsk();
    }
}

static bool read_distance (imu_core_sample_t *p_sample,
                           uint32_t previous_ms, float32_t wheel_limit_mm_s)
{
    imu_runtime_trust_t trust = {0};
    ER result = tk_loc_mtx(g_mutex, IMU_LOCK_TIMEOUT_MS);

    if (E_OK != result)
    {
        return false;
    }
    trust = g_trust;
    result = tk_unl_mtx(g_mutex);
    if (E_OK != result)
    {
        return false;
    }
    p_sample->motion_epoch = trust.epoch;
    p_sample->b_traction_trusted = trust.b_trusted;
    imu_link_read(p_sample, previous_ms, wheel_limit_mm_s);
    return true;
}

/* End of file. */

