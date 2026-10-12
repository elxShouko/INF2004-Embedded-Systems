/**
 * @file    imu_hump.h
 * @brief   Buddy 4 hump detection and peak measurement.
 *
 * Detection model (docs Part 3 has the full derivation):
 *   A rigid car of wheelbase L crossing a hump pitches nose-UP while the front
 *   wheels climb, reaches a maximum pitch near the moment the front wheels
 *   reach the crest, and pitches nose-DOWN while the front wheels descend
 *   (and/or the rear wheels climb), before returning to level. The detector
 *   therefore requires the complete signature
 *
 *       level (armed) -> sustained +pitch (ASCENT) -> pitch maximum (PEAK)
 *       -> sustained -pitch (DESCENT) -> level again (complete)
 *
 *   inside a bounded time window. Isolated spikes fail the minimum-duration
 *   tests; a ramp/slope fails the "no descent" / "ascent too long" tests;
 *   a single acceleration step (pitch artefact from linear acceleration)
 *   fails the descent test; double counting is prevented by the re-arm
 *   condition (level for hump_rearm_level_ms and hump_min_gap_ms after the
 *   previous hump).
 *
 * Measurement:
 *   - PRIMARY (directly measured): peak nose-up pitch in degrees relative to
 *     the calibrated level attitude. Used to rank humps by default.
 *   - SECONDARY (estimate, explicit assumptions): physical height in mm.
 *       GEOMETRIC : h = L * sin(theta_peak)   valid if hump length <~ L
 *       INTEGRATED: h = max(integral(v*sin(theta) dt)) / imu_pos_frac
 *                   needs the Buddy 2 speed hint during the whole ascent.
 *     followed by an empirical linear correction (gain, offset) that must be
 *     fitted from humps of known height. Without that fit the estimate is
 *     reported with IMU_CONF_LOW.
 */

#ifndef IMU_HUMP_H
#define IMU_HUMP_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_config.h"
#include "imu/imu_types.h"

/** Validity of a hump measurement. */
typedef enum
{
    IMU_HUMP_VALID = 0,
    IMU_HUMP_PROVISIONAL,     /**< at the peak, traversal not complete */
    IMU_HUMP_REJ_LOW_PEAK,
    IMU_HUMP_REJ_NO_DESCENT,
    IMU_HUMP_REJ_TOO_SHORT,
    IMU_HUMP_REJ_TOO_LONG,
    IMU_HUMP_REJ_SLOPE,
    IMU_HUMP_REJ_ABORTED
} imu_hump_validity_t;

/** One hump measurement. */
typedef struct
{
    uint32_t            index;            /**< 1..n valid humps, 0 if not */
    imu_hump_validity_t validity;
    uint32_t            t_start_ms;
    uint32_t            t_peak_ms;
    uint32_t            t_end_ms;
    uint32_t            ascent_ms;
    uint32_t            total_ms;
    float               peak_pitch_deg;   /**< PRIMARY metric (measured)  */
    float               min_pitch_deg;    /**< most nose-down (negative)  */
    float               peak_vert_up_g;   /**< max upward dynamic accel   */
    float               peak_vert_down_g; /**< max downward (negative)    */
    float               height_est_mm;    /**< estimate, see conf/method  */
    imu_confidence_t    height_conf;
    imu_height_method_t height_method;
    float               rise_mm;          /**< integrated rise (if speed) */
    bool                b_rise_valid;
    float               length_est_mm;    /**< v * duration (if speed)    */
    bool                b_length_valid;
    bool                b_attitude_degraded;
    float               rank_value;       /**< value used for the maximum */
} imu_hump_meas_t;

/** Mission statistics. */
typedef struct
{
    uint32_t        valid_count;
    uint32_t        rejected_count;
    bool            b_has_last;
    imu_hump_meas_t last;           /**< most recently completed valid hump */
    bool            b_has_max;
    imu_hump_meas_t max;            /**< highest valid hump of the mission  */
} imu_hump_stats_t;

/** Inputs for one detector step. */
typedef struct
{
    float pitch_deg;              /**< filtered, levelled (+ hint-compensated)*/
    float vert_accel_g;           /**< cycle vertical dynamic acceleration  */
    bool  b_attitude_reliable;
    bool  b_speed_valid;          /**< fresh Buddy 2 speed hint available   */
    float speed_mm_s;
    bool  b_hint_accel_valid;
    float hint_accel_g;
} imu_hump_input_t;

/** Outputs of one detector step. */
typedef struct
{
    imu_hump_phase_t phase;
    bool             b_detected;   /**< ASCENT confirmed (HUMP_DETECTED)   */
    bool             b_peak;       /**< PEAK identified (HUMP_PEAK)        */
    bool             b_completed;  /**< traversal finished, VALID          */
    bool             b_rejected;   /**< candidate discarded                */
    bool             b_new_max;    /**< completed hump is the new maximum  */
    imu_hump_meas_t  meas;         /**< provisional or final measurement   */
} imu_hump_out_t;

/** Detector instance. */
typedef struct
{
    imu_params_t const * p_params;
    imu_hump_phase_t     phase;
    imu_hump_stats_t     stats;
    imu_hump_meas_t      cur;

    bool     b_armed;
    bool     b_level_timing;
    uint32_t level_start_ms;
    bool     b_has_last_end;
    uint32_t last_end_ms;

    bool     b_cand;
    uint32_t cand_start_ms;
    float    cand_peak_deg;
    uint32_t cand_peak_ms;
    bool     b_desc_cand;
    uint32_t desc_cand_start_ms;
    bool     b_lvl_cand;
    uint32_t lvl_cand_start_ms;

    uint32_t last_t_ms;
    float    rise_mm;
    bool     b_rise_ok;
    float    dist_mm;
    float    dist_at_peak_mm;
} imu_hump_t;

imu_err_t imu_hump_init(imu_hump_t * p_hump, imu_params_t const * p_params);

/** Clear run statistics (new mission / START_RUN). Phase is reset too. */
void imu_hump_reset_mission(imu_hump_t * p_hump);

/** Abort an in-progress hump (impact, fault). Counted as rejected. */
void imu_hump_abort(imu_hump_t * p_hump, uint32_t now_ms);

/** One detector step (call once per sampling period). */
void imu_hump_step(imu_hump_t * p_hump, uint32_t now_ms,
                   imu_hump_input_t const * p_in, imu_hump_out_t * p_out);

/** Height estimate for a given measurement (pure function, exported for
 *  testing and offline re-evaluation of logged peaks). */
void imu_hump_estimate_height(imu_params_t const * p_params,
                              imu_hump_meas_t * p_meas);

char const * imu_hump_validity_name(imu_hump_validity_t validity);

#endif /* IMU_HUMP_H */
