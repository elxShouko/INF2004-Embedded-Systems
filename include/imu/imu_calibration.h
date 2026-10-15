/**
 * @file    imu_calibration.h
 * @brief   Buddy 4 sensor calibration.
 *
 * 1. Stationary accelerometer calibration (run at power-up, vehicle still on
 *    level ground, motors OFF):
 *      - mean specific force  -> gravity / mounting-orientation reference
 *      - |mean|               -> 1 g reference (removes scale/bias error along
 *                                gravity, i.e. vertical offset compensation)
 *      - per-axis std dev     -> noise estimate (used to scale thresholds)
 *      - levelling rotation   -> maps the measured gravity onto vehicle +Z,
 *                                so the sensor does not have to be perfectly
 *                                aligned with the chassis.
 *    LIMITATION: with a single orientation, a constant sensor bias is
 *    indistinguishable from a small mounting tilt. Both are removed together
 *    (which is exactly what tilt/hump detection needs). A per-axis bias from
 *    an optional 6-position bench calibration can be supplied in
 *    imu_params_t.accel_bias_g.
 *
 * 2. Magnetometer hard-/soft-iron calibration (min/max ellipse fit in the
 *    horizontal plane). Samples are collected while the vehicle spins slowly
 *    in place WITH MOTORS RUNNING (so motor magnets and typical motor current
 *    fields are included). Only the two horizontal sensor axes can be
 *    calibrated from a planar rotation; the vertical-axis offset is taken
 *    from imu_params_t (default 0) - heading only depends weakly on it after
 *    tilt compensation at small tilt angles.
 */

#ifndef IMU_CALIBRATION_H
#define IMU_CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_config.h"
#include "imu/imu_math.h"
#include "imu/imu_types.h"

/** Result of the stationary accelerometer calibration. */
typedef struct
{
    bool        b_valid;
    uint32_t    samples;
    imu_vec3_t  gravity_mean_g;     /**< axis-mapped, bias-removed mean    */
    float       g_ref;              /**< |gravity_mean_g| (should be ~1)   */
    float       mount_tilt_deg;     /**< angle between gravity and +Z      */
    float       mount_pitch_deg;    /**< informative                       */
    float       mount_roll_deg;     /**< informative                       */
    imu_mat3_t  level_rot;          /**< maps gravity_mean_g onto +Z       */
    imu_vec3_t  noise_std_g;        /**< per vehicle axis                  */
    float       noise_max_std_g;    /**< max of the three                  */
} imu_accel_cal_t;

/** Running statistics for the stationary calibration (Welford). */
typedef struct
{
    uint32_t   n;
    imu_vec3_t mean;
    imu_vec3_t m2;
} imu_accel_cal_acc_t;

/** Magnetometer calibration result (sensor frame, gauss). */
typedef struct
{
    bool       b_valid;
    uint32_t   samples;
    imu_vec3_t hard_iron_gauss;
    imu_vec3_t soft_iron_scale;
    imu_vec3_t span_gauss;
    float      ref_field_gauss;     /**< expected |B| after correction     */
} imu_mag_cal_t;

/** Running min/max accumulator for magnetometer calibration. */
typedef struct
{
    uint32_t   n;
    imu_vec3_t min;
    imu_vec3_t max;
    imu_vec3_t sum;
} imu_mag_cal_acc_t;

/* ---- accelerometer ---------------------------------------------------- */
void      imu_cal_accel_begin(imu_accel_cal_acc_t * p_acc);

/** @param p_vehicle_g  axis-mapped, bias-removed sample (not levelled). */
void      imu_cal_accel_add(imu_accel_cal_acc_t * p_acc,
                            imu_vec3_t const * p_vehicle_g);

/**
 * @return IMU_OK, IMU_ERR_CAL_SAMPLES (too few), IMU_ERR_CAL_MOTION (noise
 *         above tolerance: vehicle moving or being handled), IMU_ERR_CAL_RANGE
 *         (|g| or mounting tilt out of range), IMU_ERR_PARAM.
 */
imu_err_t imu_cal_accel_finish(imu_accel_cal_acc_t const * p_acc,
                               imu_params_t const * p_params,
                               imu_accel_cal_t * p_out);

/* ---- magnetometer ----------------------------------------------------- */
void      imu_cal_mag_begin(imu_mag_cal_acc_t * p_acc);
void      imu_cal_mag_add(imu_mag_cal_acc_t * p_acc,
                          imu_vec3_t const * p_sensor_gauss);

/**
 * @return IMU_OK, IMU_ERR_CAL_SAMPLES, IMU_ERR_CAL_COVERAGE (vehicle did not
 *         rotate enough / ellipse implausible), IMU_ERR_PARAM.
 */
imu_err_t imu_cal_mag_finish(imu_mag_cal_acc_t const * p_acc,
                             imu_params_t const * p_params,
                             imu_mag_cal_t * p_out);

/** Load a stored magnetometer calibration from the parameter block. */
void      imu_cal_mag_from_params(imu_params_t const * p_params,
                                  imu_mag_cal_t * p_out);

/** Apply hard-/soft-iron correction (sensor frame). */
void      imu_cal_mag_apply(imu_mag_cal_t const * p_cal,
                            imu_vec3_t const * p_raw_gauss,
                            imu_vec3_t * p_out);

#endif /* IMU_CALIBRATION_H */
