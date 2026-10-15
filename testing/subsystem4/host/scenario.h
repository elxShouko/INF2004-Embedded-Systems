/**
 * @file    scenario.h
 * @brief   HOST-ONLY physically-based vehicle motion generator used to feed
 *          the LSM303DLHC simulator with known ground truth.
 *
 * Model: rigid car, wheelbase L, IMU at fraction k of L from the rear axle,
 * driving along a straight or curved path at speed v(t). Humps have a
 * raised-cosine profile z(x) = h/2 (1 - cos(2 pi x / Lh)). Pitch is
 * asin((z_front - z_rear) / L). Specific force = R^T (a_world + g z_up)
 * plus optional impact pulses, motor vibration and sensor noise. The
 * magnetometer sees the Earth field (plus optional hard/soft iron and a
 * disturbance source). The sensor frame = M * vehicle frame (mounting).
 */

#ifndef SCENARIO_H
#define SCENARIO_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_types.h"

#define SCN_MAX_HUMPS     (8U)
#define SCN_MAX_IMPACTS   (8U)
#define SCN_MAX_SEGMENTS  (16U)

typedef struct
{
    double start_m;      /* distance along path where the hump starts */
    double height_m;
    double length_m;
} scn_hump_t;

typedef struct
{
    double     t_s;
    double     duration_s;
    double     peak_g;
    imu_vec3_t dir_vehicle;   /* unit direction, vehicle frame */
} scn_impact_t;

/** Piecewise motion segment: from t_start for duration. */
typedef struct
{
    double t_start_s;
    double duration_s;
    double v_start_mps;
    double v_end_mps;
    double yaw_rate_dps;      /* + = clockwise (right turn) */
    double roll_deg;          /* static roll (side slope / lifted) */
    double pitch_deg;         /* static pitch (ramp) */
} scn_segment_t;

typedef struct
{
    /* vehicle */
    double wheelbase_m;
    double imu_pos_frac;
    double heading0_deg;

    /* environment */
    double bh_gauss;          /* horizontal earth field */
    double bup_gauss;         /* vertical (up) component */

    /* mounting: sensor = M * vehicle */
    double mount[3][3];
    imu_vec3_t hard_iron;     /* sensor frame, gauss */
    imu_vec3_t soft_scale;    /* sensor frame */
    imu_vec3_t disturb_gauss; /* extra field, sensor frame */
    double disturb_t0_s;
    double disturb_t1_s;

    /* noise */
    double accel_noise_g;     /* sensor white noise (sigma) */
    double vib_noise_g;       /* extra when moving (sigma) */
    double mag_noise_gauss;
    uint32_t seed;

    scn_segment_t seg[SCN_MAX_SEGMENTS];
    uint32_t      n_seg;
    scn_hump_t    hump[SCN_MAX_HUMPS];
    uint32_t      n_hump;
    scn_impact_t  imp[SCN_MAX_IMPACTS];
    uint32_t      n_imp;

    /* integrated state */
    double t_s;
    double s_m;
    double heading_deg;
    double v_mps;

    /* ground truth log */
    double max_pitch_deg;
} scenario_t;

void scn_init(scenario_t * p_s);
void scn_add_segment(scenario_t * p_s, double t0, double dur, double v0,
                     double v1, double yaw_rate_dps);
void scn_add_hump(scenario_t * p_s, double start_m, double height_m,
                  double length_m);
void scn_add_impact(scenario_t * p_s, double t_s, double dur_s,
                    double peak_g, float dx, float dy, float dz);

/** Stimulus callback for the simulator (sim_stimulus_fn_t). */
void scn_stimulus(void * p_ctx, double t_s, imu_vec3_t * p_accel_g,
                  imu_vec3_t * p_mag_gauss);

/** Speed / along-path acceleration at time t (encoder truth). */
void scn_speed_at(scenario_t const * p_s, double t_s, double * p_v,
                  double * p_a);

/** Ground-truth peak pitch (deg) for a hump with the car's wheelbase. */
double scn_true_peak_pitch_deg(double height_m, double length_m,
                               double wheelbase_m);

#endif /* SCENARIO_H */
