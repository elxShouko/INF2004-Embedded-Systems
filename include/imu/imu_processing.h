/**
 * @file    imu_processing.h
 * @brief   Buddy 4 signal processing: frame mapping, attitude (pitch / roll /
 *          tilt), tilt-compensated magnetic heading, heading-derived turn
 *          rate, turning detection, tilt detection, impact detection and
 *          motion classification.
 *
 * Hardware-independent and RTOS-independent: works on physical units only.
 *
 * What the LSM303DLHC can and cannot provide (see docs Part 2):
 *   - It measures SPECIFIC FORCE (gravity + linear acceleration) and the
 *     magnetic field. It has no gyroscope.
 *   - Pitch/roll are derived from the gravity direction. During linear
 *     acceleration a: pitch error ~ asin(a/g) (0.05 g -> ~2.9 deg). The
 *     estimate is flagged unreliable when |f| departs from 1 g and can be
 *     corrected with the optional Buddy 2 acceleration hint.
 *   - "Turn rate" is the time derivative of the magnetic heading over a
 *     window (default 200 ms). It is NOT a gyroscope angular velocity: it is
 *     band-limited, delayed by ~window/2 and corrupted by magnetic
 *     disturbances (motors, steel), which are detected and flagged.
 */

#ifndef IMU_PROCESSING_H
#define IMU_PROCESSING_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_calibration.h"
#include "imu/imu_config.h"
#include "imu/imu_math.h"
#include "imu/imu_types.h"

/** Phase of a reported tilt / turn event. */
typedef enum
{
    IMU_PHASE_START = 0,
    IMU_PHASE_END,
    IMU_PHASE_HEADING_CHANGE,
    IMU_PHASE_ESCALATE
} imu_event_phase_t;

/** Impact-detector result (one per detected pulse). */
typedef struct
{
    bool           b_detected;
    imu_severity_t severity;
    float          peak_g;           /**< |high-pass accel| at the peak   */
    float          horiz_g;          /**< horizontal component at peak    */
    float          vert_g;           /**< vertical component at peak      */
    uint32_t       t_ms;             /**< time of the peak                */
    uint32_t       duration_ms;      /**< time above threshold            */
    uint32_t       suppressed;       /**< pulses suppressed by cool-down  */
} imu_impact_t;

/** Per-cycle processing output. */
typedef struct
{
    /* Attitude */
    float      pitch_deg;            /**< relative to calibrated level    */
    float      roll_deg;
    float      tilt_deg;
    bool       b_attitude_reliable;  /**< |f| close to 1 g                */
    imu_vec3_t accel_lp_g;           /**< levelled, low-pass               */
    imu_vec3_t accel_cycle_g;        /**< levelled, cycle average          */
    float      fwd_accel_g;          /**< forward linear accel estimate    */
    bool       b_fwd_from_hint;      /**< fwd_accel_g came from encoders   */
    float      vert_accel_g;         /**< cycle vertical dynamic accel     */
    float      vibration_g;          /**< RMS of high-passed |accel|       */

    /* Heading / turning */
    float                 heading_deg;
    imu_heading_quality_t heading_quality;
    float                 mag_field_gauss;
    float                 turn_rate_dps;   /**< heading-derived, see note */
    bool                  b_turn_rate_valid;
    imu_turn_dir_t        turn_dir;        /**< current debounced state   */

    /* Events produced during this step */
    bool              b_tilt_event;
    imu_event_phase_t tilt_phase;
    imu_severity_t    tilt_severity;

    bool              b_turn_event;
    imu_event_phase_t turn_phase;
    imu_turn_dir_t    turn_event_dir;
    float             turn_event_delta_deg;  /**< heading change of turn  */
    uint32_t          turn_event_duration_ms;
    bool              b_turn_event_unreliable;

    /* Motion classification (filled by imu_proc_classify) */
    imu_motion_class_t motion;
    imu_motion_class_t motion_prev;
    imu_confidence_t   motion_conf;
    bool               b_motion_changed;
} imu_proc_out_t;

/** Processing context (all state is static-size, no allocation). */
typedef struct
{
    imu_params_t const * p_params;
    imu_accel_cal_t      accel_cal;
    imu_mag_cal_t        mag_cal;

    /* Timing */
    bool     b_seeded;
    uint32_t last_t_ms;

    /* Attitude */
    imu_vec3_t accel_lp;
    imu_vec3_t up_last;
    float      pitch_deg;
    float      roll_deg;
    float      tilt_deg;
    bool       b_attitude_reliable;

    /* High-rate impact path */
    bool         b_hf_seeded;
    uint32_t     hf_seed_t_ms;
    imu_vec3_t   hf_lp;
    float        hf_alpha;
    float        vib_var;
    float        vib_alpha;
    bool         b_imp_active;
    bool         b_imp_sustained;
    uint32_t     imp_start_ms;
    uint32_t     imp_last_above_ms;
    imu_impact_t imp_peak;
    uint32_t     imp_cooldown_until;
    bool         b_imp_cooldown;
    imu_severity_t imp_last_sev;
    uint32_t     imp_suppressed;
    uint32_t     imp_rejected_long;
    uint32_t     imp_last_report_ms;
    bool         b_imp_has_report;

    /* Heading */
    bool     b_heading_seeded;
    float    heading_f_deg;
    float    heading_unwrapped;
    uint32_t heading_outliers;
    uint32_t hist_t[IMU_HEADING_HIST_LEN];
    float    hist_h[IMU_HEADING_HIST_LEN];
    uint8_t  hist_head;
    uint8_t  hist_count;
    float    heading_ref_unwrapped;
    float    turn_rate_dps;
    bool     b_turn_rate_valid;
    imu_heading_quality_t heading_quality;
    float    mag_field;

    /* Turn detector */
    imu_turn_dir_t turn_state;
    imu_turn_dir_t turn_cand_dir;
    uint32_t       turn_cand_start;
    bool           b_turn_cand;
    float          turn_cand_unw;
    float          turn_start_unw;
    uint32_t       turn_start_ms;
    uint32_t       turn_exit_start;
    bool           b_turn_exit_timing;

    /* Tilt detector */
    bool     b_tilted;
    bool     b_tilt_critical_sent;
    bool     b_tilt_cand;
    uint32_t tilt_cand_start;

    /* Motion classifier */
    imu_motion_class_t motion;
    imu_motion_class_t motion_cand;
    uint32_t           motion_cand_start;
    imu_confidence_t   motion_conf;
    float              fwd_accel_g;
    bool               b_fwd_from_hint;
    uint32_t           impact_hold_until;
    bool               b_impact_hold;

    /* Optional encoder hint (Buddy 2 via Mission Controller) */
    imu_motion_hint_t hint;
    float             hint_accel_g;
    bool              b_hint_accel_valid;
    uint32_t          hints_rejected;
} imu_proc_t;

/** Initialise; @p p_params must stay valid for the life of @p p_proc. */
imu_err_t imu_proc_init(imu_proc_t * p_proc, imu_params_t const * p_params);

/** Install calibration results (either may be NULL to keep current). */
void imu_proc_set_calibration(imu_proc_t * p_proc,
                              imu_accel_cal_t const * p_accel,
                              imu_mag_cal_t const * p_mag);

/** Clear all filters and detectors (after a fault / time discontinuity). */
void imu_proc_reset_dynamic(imu_proc_t * p_proc);

/** Sensor-frame accel (g) -> vehicle axes, stored bias removed, NOT levelled
 *  (used to feed the stationary calibration). */
void imu_proc_map_accel(imu_proc_t const * p_proc,
                        imu_vec3_t const * p_sensor_g,
                        imu_vec3_t * p_vehicle_g);

/** Sensor-frame accel (g) -> levelled vehicle frame. */
void imu_proc_level_accel(imu_proc_t const * p_proc,
                          imu_vec3_t const * p_sensor_g,
                          imu_vec3_t * p_level_g);

/**
 * @brief  High-rate impact path, called for EVERY accelerometer sample
 *         (all FIFO entries), oldest first.
 * @param  b_hump_active  raises the threshold while crossing a hump.
 * @param  p_out          b_detected = true when a pulse was classified.
 */
void imu_proc_hf_accel(imu_proc_t * p_proc, imu_vec3_t const * p_sensor_g,
                       uint32_t t_ms, bool b_hump_active,
                       imu_impact_t * p_out);

/**
 * @brief  Main periodic step (attitude, heading, turn and tilt detection).
 * @param  p_accel_sensor_g  cycle-average accel, sensor frame (not NULL)
 * @param  p_mag_sensor_gauss  magnetometer sample, sensor frame, or NULL if
 *                             unavailable/overflowed this cycle.
 * @param  b_hump_active  tilt events from pitch are suppressed in a hump.
 */
void imu_proc_step(imu_proc_t * p_proc, imu_vec3_t const * p_accel_sensor_g,
                   imu_vec3_t const * p_mag_sensor_gauss, uint32_t now_ms,
                   bool b_hump_active, imu_proc_out_t * p_out);

/**
 * @brief  Motion classification; call after imu_proc_step and the hump
 *         detector in the same cycle.
 */
void imu_proc_classify(imu_proc_t * p_proc, uint32_t now_ms,
                       imu_hump_phase_t hump_phase, bool b_new_impact,
                       imu_proc_out_t * p_out);

/**
 * @brief  Supply / refresh the optional Buddy 2 motion hint.
 * @return IMU_OK, or IMU_ERR_PARAM if the hint is implausible (rejected).
 */
imu_err_t imu_proc_set_hint(imu_proc_t * p_proc,
                            imu_motion_hint_t const * p_hint);

/** @return true if a hint younger than hint_max_age_ms is available. */
bool imu_proc_hint_fresh(imu_proc_t const * p_proc, uint32_t now_ms);

/** Name helpers for logs / telemetry (never NULL). */
char const * imu_motion_name(imu_motion_class_t motion);
char const * imu_heading_quality_name(imu_heading_quality_t quality);

#endif /* IMU_PROCESSING_H */
