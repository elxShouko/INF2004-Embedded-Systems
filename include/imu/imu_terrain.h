/**
 * @file    imu_terrain.h
 * @brief   Buddy 4 subsystem state machine (Design Review Section 6.4):
 *          INITIALISATION -> CALIBRATION -> MONITORING
 *          { HUMP_ASCENT -> HUMP_PEAK -> HUMP_DESCENT },
 *          TILT/TURN EVENT reporting, IMPACT EVENT -> FAULT HANDLING,
 *          SENSOR FAULT -> FAULT/SAFE STATE -> RECOVERY -> MONITORING.
 *
 * RTOS-independent: imu_terrain_step() performs ONE sampling period of work
 * and is called by the uT-Kernel IMU task (imu_rtos.c) or by a test harness.
 * It is NOT thread-safe by itself; imu_rtos.c provides the locking.
 *
 * Mapping of the statechart to the implementation (docs Part 3):
 *   - HUMP PROCESSING is delegated to imu_hump.c; its phase is mirrored into
 *     the subsystem state (MONITORING / HUMP_ASCENT / HUMP_PEAK /
 *     HUMP_DESCENT).
 *   - TILT EVENT -> EVENT REPORTING -> MONITORING and TURN EVENT -> EVENT
 *     REPORTING -> MONITORING are run-to-completion transient states inside
 *     one step: they are recorded in the transition trace but never block
 *     sampling, so tilt/turn detection can run in parallel with hump
 *     processing (a hump can be crossed during a gentle turn).
 *   - IMPACT EVENT -> FAULT HANDLING -> FAULT/SAFE STATE for CRITICAL impacts
 *     (MINOR impacts are reported and monitoring continues, configurable).
 *   - SENSOR FAULT -> FAULT/SAFE STATE; RECOVERY verifies the sensor before
 *     MONITORING resumes. A collision fault additionally needs the Mission
 *     Controller's CLEAR_FAULT acknowledgement and settled conditions.
 *   - Unresolved faults never silently return to MONITORING.
 */

#ifndef IMU_TERRAIN_H
#define IMU_TERRAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "imu/imu_calibration.h"
#include "imu/imu_config.h"
#include "imu/imu_events.h"
#include "imu/imu_hump.h"
#include "imu/imu_lsm303dlhc.h"
#include "imu/imu_processing.h"
#include "imu/imu_types.h"

/** Commands accepted by the subsystem (bit mask, processed next step). */
#define IMU_CMD_MISSION_START   (0x0001U)  /**< reset run statistics      */
#define IMU_CMD_RESET           (0x0002U)  /**< back to INITIALISATION    */
#define IMU_CMD_CLEAR_FAULT     (0x0004U)  /**< MC acknowledges collision */
#define IMU_CMD_MAG_CAL_BEGIN   (0x0008U)  /**< start magnetometer cal    */
#define IMU_CMD_MAG_CAL_END     (0x0010U)  /**< finish magnetometer cal   */
#define IMU_CMD_RECALIBRATE     (0x0020U)  /**< redo stationary cal       */
#define IMU_CMD_STOP            (0x0040U)  /**< shutdown sensor           */
#define IMU_CMD_ALL             (0x007FU)

/** One entry of the state-transition trace. */
typedef struct
{
    uint32_t t_ms;
    uint8_t  from;      /**< imu_state_t */
    uint8_t  to;        /**< imu_state_t */
} imu_trace_entry_t;

/** Complete status snapshot (what other tasks may read). */
typedef struct
{
    uint32_t              timestamp_ms;
    imu_state_t           state;
    imu_fault_t           fault;
    imu_err_t             last_error;
    bool                  b_calibrated;
    bool                  b_sensor_ok;
    bool                  b_mag_cal_valid;
    bool                  b_mag_cal_active;

    imu_motion_class_t    motion;
    imu_confidence_t      motion_conf;

    float                 pitch_deg;
    float                 roll_deg;
    float                 tilt_deg;
    bool                  b_attitude_reliable;
    bool                  b_tilted;

    float                 heading_deg;
    imu_heading_quality_t heading_quality;
    float                 turn_rate_dps;
    bool                  b_turn_rate_valid;
    imu_turn_dir_t        turn_dir;

    float                 fwd_accel_g;
    float                 vert_accel_g;
    float                 vibration_g;
    float                 mag_field_gauss;

    imu_hump_phase_t      hump_phase;
    imu_hump_stats_t      hump;
    bool                  b_has_impact;
    imu_impact_t          last_impact;

    float                 cal_g_ref;
    float                 cal_noise_g;
    float                 cal_mount_tilt_deg;
    imu_mag_cal_t         mag_cal;

    lsm303_stats_t        i2c;
    imu_event_stats_t     events;
    uint32_t              cycles;
    uint32_t              consecutive_failures;
    uint32_t              recovery_attempts;
    uint32_t              hints_rejected;
} imu_status_t;

/** Compact integer telemetry record for Buddy 1 (no floats). */
typedef struct
{
    uint32_t timestamp_ms;
    uint8_t  state;              /**< imu_state_t                       */
    uint8_t  motion;             /**< imu_motion_class_t                */
    uint8_t  hump_phase;         /**< imu_hump_phase_t                  */
    uint8_t  fault;              /**< imu_fault_t                       */
    int16_t  pitch_cdeg;
    int16_t  roll_cdeg;
    int16_t  heading_cdeg;       /**< -1 = heading invalid              */
    int16_t  turn_rate_cdps;
    uint16_t hump_count;
    int16_t  last_hump_peak_cdeg;  /**< -1 = none                       */
    int16_t  hump_peak_cdeg;       /**< mission max pitch, -1 = none    */
    int16_t  hump_peak_height_mm;  /**< mission max height est, -1 = n/a*/
    uint8_t  hump_height_conf;     /**< imu_confidence_t                */
    uint8_t  b_sensor_ok;
} imu_telemetry_t;

/** Subsystem instance. */
typedef struct
{
    imu_params_t           params;
    lsm303_t               dev;
    imu_proc_t             proc;
    imu_hump_t             hump;
    imu_event_dispatcher_t disp;

    imu_state_t            state;
    imu_fault_t            fault;
    imu_err_t              last_error;
    uint32_t               next_action_ms;
    uint32_t               attempts;
    uint32_t               retry_interval_ms;
    bool                   b_persistent_reported;

    imu_accel_cal_acc_t    cal_acc;
    uint32_t               cal_attempts;
    bool                   b_cal_fault_reported;
    imu_mag_cal_acc_t      mag_acc;
    bool                   b_mag_cal_active;

    imu_raw3_t             accel_buf[IMU_ACCEL_BATCH_MAX];
    imu_raw3_t             last_raw;
    bool                   b_last_raw_valid;
    uint32_t               frozen_count;
    uint32_t               consec_fail;
    uint32_t               consec_invalid;
    uint32_t               last_accel_ok_ms;
    uint32_t               verify_count;

    bool                   b_fault_ack;
    bool                   b_settle_timing;
    uint32_t               settle_start_ms;

    uint32_t               pending_cmds;
    imu_proc_out_t         last_out;
    imu_status_t           status;

    imu_trace_entry_t      trace[IMU_STATE_TRACE_LEN];
    uint8_t                trace_head;
    uint8_t                trace_count;
    uint32_t               cycles;
} imu_terrain_t;

/**
 * @brief  Initialise the subsystem (no I/O; the sensor is initialised in the
 *         first imu_terrain_step()).
 * @param  p_params  parameter set (copied); NULL = defaults.
 * @param  p_sink    event sink (copied); NULL = events counted as dropped.
 */
imu_err_t imu_terrain_init(imu_terrain_t * p_sys,
                           imu_params_t const * p_params,
                           imu_event_sink_t const * p_sink);

/** One sampling period. @p now_ms = milliseconds since boot. */
void imu_terrain_step(imu_terrain_t * p_sys, uint32_t now_ms);

/** Post commands (IMU_CMD_*), executed at the start of the next step. */
imu_err_t imu_terrain_command(imu_terrain_t * p_sys, uint32_t cmd_mask);

/** Supply the optional Buddy 2 motion hint. */
imu_err_t imu_terrain_set_hint(imu_terrain_t * p_sys,
                               imu_motion_hint_t const * p_hint);

/** Copy the latest status snapshot. */
imu_err_t imu_terrain_get_status(imu_terrain_t const * p_sys,
                                 imu_status_t * p_out);

/** Copy the transition trace (oldest first). @return entries copied. */
uint8_t imu_terrain_get_trace(imu_terrain_t const * p_sys,
                              imu_trace_entry_t * p_out, uint8_t max);

/** Convert a status snapshot into the compact telemetry record. */
void imu_terrain_make_telemetry(imu_status_t const * p_status,
                                imu_telemetry_t * p_out);

/**
 * @brief  Format the Buddy 4 telemetry fields as a JSON fragment (no braces)
 *         for Buddy 1's serializer, e.g.
 *         "hump_peak":523,"hump_peak_mm":12,"hump_count":2,...
 *         "hump_peak" is in centi-degrees of peak pitch (directly measured).
 * @return number of characters written (excluding NUL), or -1 if @p len is
 *         too small (buffer is then an empty string).
 */
int32_t imu_terrain_telemetry_json(imu_telemetry_t const * p_tele,
                                   char * p_buf, size_t len);

char const * imu_state_name(imu_state_t state);
char const * imu_fault_name(imu_fault_t fault);

#endif /* IMU_TERRAIN_H */
