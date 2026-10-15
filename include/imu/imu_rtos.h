/**
 * @file    imu_rtos.h
 * @brief   Buddy 4 IMU / Terrain Task on uT-Kernel 3.0 and the thread-safe
 *          public API used by the Mission Controller, Buddy 1 and Buddy 2.
 *
 * RTOS objects (all statically allocated, TA_USERBUF):
 *   - IMU / Terrain Task        IMU_RTOS_DEFAULT_PRIORITY (Medium-High)
 *   - cyclic handler            wakes the task every sample_period_ms
 *                               (drift-free period, overrun detection)
 *   - mutex (TA_INHERIT)        protects the shared status snapshot and the
 *                               command / hint mailboxes
 *   - message buffer            terrain_event_q (created here unless the
 *                               integration team passes an existing ID)
 *   - event flag (optional)     direct COLLISION / IMU_FAULT notification
 *                               to the Mission Controller (tk_set_flg)
 *
 * The IMU task never holds the mutex while talking to the sensor, never
 * blocks on a queue (TMO_POL), never performs network/MQTT operations and
 * never calls Buddy 2's motor API.
 *
 * INTEGRATION ITEMS TO CONFIRM WITH THE TEAM (marked "CONFIRM" below):
 *   - final numeric task priorities of all buddies
 *   - whether the Mission Controller creates terrain_event_q itself
 *   - event-flag ID and bit patterns used for direct notifications
 *   - kernel object limits in config.h (CNF_MAX_MBFID etc.)
 */

#ifndef IMU_RTOS_H
#define IMU_RTOS_H

#include <stdbool.h>
#include <stdint.h>

#include <tk/tkernel.h>

#include "imu/imu_events.h"
#include "imu/imu_hump.h"
#include "imu/imu_terrain.h"
#include "imu/imu_types.h"

/* CONFIRM: uT-Kernel priority 1 = highest. Proposed team mapping:
 * Motion Control 4 (Very High), Line/Barcode 6, Obstacle Monitor 6,
 * Mission Controller 7 (High), IMU 9 / Obstacle Handling 9 (Medium-High),
 * Wi-Fi/Telemetry 14 (Low). */
#ifndef IMU_RTOS_DEFAULT_PRIORITY
#define IMU_RTOS_DEFAULT_PRIORITY      (9)
#endif

/** Task stack size in bytes (static buffer; see docs Part 3 budget). */
#ifndef IMU_RTOS_STACK_BYTES
#define IMU_RTOS_STACK_BYTES           (2048U)
#endif

/** Depth of terrain_event_q when created by Buddy 4. */
#ifndef IMU_RTOS_EVENT_Q_DEPTH
#define IMU_RTOS_EVENT_Q_DEPTH         (16U)
#endif

/** Bounded mutex wait for API callers (ms; rounded up to kernel ticks). */
#ifndef IMU_RTOS_LOCK_TMO_MS
#define IMU_RTOS_LOCK_TMO_MS           (10)
#endif

/** Task watchdog: a missing cyclic wake-up is detected after N periods. */
#define IMU_RTOS_WAKE_TIMEOUT_FACTOR   (3)

/** Start-up configuration. */
typedef struct
{
    PRI                  task_priority;
    imu_params_t const * p_params;          /**< NULL = defaults            */
    ID                   terrain_event_q;   /**< 0 = create (CONFIRM)       */
    ID                   mc_flag_id;        /**< 0 = no direct notification */
    UINT                 collision_flag_ptn;/**< set on critical IMPACT     */
    UINT                 fault_flag_ptn;    /**< set on IMU_FAULT / tilt    */
    /** Optional hook called from the IMU task every telemetry_period_ms
     *  with a fresh telemetry record. MUST NOT BLOCK (e.g. TMO_POL send to
     *  Buddy 1's telemetry_q). NULL = disabled. */
    void (*p_telemetry_hook)(imu_telemetry_t const * p_tele);
    uint32_t             telemetry_period_ms;
} imu_rtos_config_t;

/** Run-time counters of the RTOS wrapper. */
typedef struct
{
    uint32_t cycles;
    uint32_t overruns;          /**< queued wake-ups found after a cycle */
    uint32_t missed_wakeups;    /**< tk_slp_tsk timed out                */
    uint32_t lock_timeouts;     /**< publication skipped (mutex busy)    */
    uint32_t max_cycle_us;      /**< worst observed cycle execution time */
} imu_rtos_stats_t;

/** Fill @p p_cfg with defaults (own queue, no flag, no hook). */
void imu_rtos_default_config(imu_rtos_config_t * p_cfg);

/**
 * @brief  Create the RTOS objects and start the IMU / Terrain Task.
 *         Call once from usermain() / the system initialisation task.
 * @return IMU_OK, IMU_ERR_PARAM, IMU_ERR_STATE (already running),
 *         IMU_ERR_RTOS (kernel object creation failed).
 */
imu_err_t imu_rtos_start(imu_rtos_config_t const * p_cfg);

/** Request a clean stop: the cyclic handler stops, the sensor is put to
 *  sleep, and the task exits to DORMANT (restartable with imu_rtos_start). */
imu_err_t imu_rtos_stop(void);

bool      imu_rtos_is_running(void);

/** @return message-buffer ID of terrain_event_q (0 if not started). */
ID        imu_rtos_terrain_event_q(void);

imu_err_t imu_rtos_get_stats(imu_rtos_stats_t * p_out);

/* ------------------------------------------------------------------------
 *  Thread-safe query API (any task). All functions copy from the snapshot
 *  published at the end of each IMU cycle; none touches the sensor.
 *  Return IMU_ERR_BUSY if the snapshot could not be locked in time,
 *  IMU_ERR_NOT_READY if the task has not been started.
 * ---------------------------------------------------------------------- */
imu_err_t imu_get_status(imu_status_t * p_out);
imu_err_t imu_get_telemetry(imu_telemetry_t * p_out);
imu_err_t imu_get_motion(imu_motion_class_t * p_motion,
                         imu_confidence_t * p_conf);
imu_err_t imu_get_tilt(float * p_pitch_deg, float * p_roll_deg,
                       float * p_tilt_deg, bool * p_b_reliable);
imu_err_t imu_get_heading(float * p_heading_deg,
                          imu_heading_quality_t * p_quality,
                          float * p_turn_rate_dps, bool * p_b_rate_valid,
                          imu_turn_dir_t * p_turn_dir);
imu_err_t imu_get_hump_state(imu_hump_phase_t * p_phase);
/** @return IMU_ERR_NO_DATA if no valid hump has been completed yet. */
imu_err_t imu_get_last_hump(imu_hump_meas_t * p_out);
/** @return IMU_ERR_NO_DATA if no valid hump has been completed yet. */
imu_err_t imu_get_max_hump(imu_hump_meas_t * p_out);
imu_err_t imu_get_health(imu_state_t * p_state, imu_fault_t * p_fault,
                         bool * p_b_sensor_ok);

/* ------------------------------------------------------------------------
 *  Thread-safe command API (executed at the start of the next IMU cycle).
 * ---------------------------------------------------------------------- */
imu_err_t imu_mission_start(void);      /**< START_RUN: reset hump stats   */
imu_err_t imu_request_reset(void);      /**< subsystem reset -> INIT       */
imu_err_t imu_clear_fault(void);        /**< MC acknowledges a collision   */
imu_err_t imu_recalibrate(void);        /**< vehicle must be stationary    */
imu_err_t imu_mag_cal_begin(void);      /**< start spin-in-place mag cal   */
imu_err_t imu_mag_cal_end(void);        /**< finish mag cal                */

/** Buddy 2 / Mission Controller: optional encoder speed hint. */
imu_err_t imu_set_motion_hint(imu_motion_hint_t const * p_hint);

#endif /* IMU_RTOS_H */
