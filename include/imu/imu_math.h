/**
 * @file    imu_math.h
 * @brief   Small, allocation-free math helpers for the Buddy 4 subsystem
 *          (vector algebra, frame mapping, angle wrapping, IIR filters).
 */

#ifndef IMU_MATH_H
#define IMU_MATH_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_types.h"

#define IMU_PI_F            (3.14159265358979f)
#define IMU_RAD_TO_DEG      (57.2957795130823f)
#define IMU_DEG_TO_RAD      (0.0174532925199433f)

/** 3x3 rotation matrix, row-major. */
typedef struct
{
    float m[3][3];
} imu_mat3_t;

float imu_vec_norm(imu_vec3_t const * p_v);
float imu_vec_dot(imu_vec3_t const * p_a, imu_vec3_t const * p_b);
void  imu_vec_cross(imu_vec3_t const * p_a, imu_vec3_t const * p_b,
                    imu_vec3_t * p_out);
void  imu_vec_sub(imu_vec3_t const * p_a, imu_vec3_t const * p_b,
                  imu_vec3_t * p_out);
void  imu_vec_scale(imu_vec3_t const * p_a, float k, imu_vec3_t * p_out);
bool  imu_vec_is_finite(imu_vec3_t const * p_v);

/** @return false if |v| is too small to normalise (output unchanged). */
bool  imu_vec_normalise(imu_vec3_t const * p_v, imu_vec3_t * p_out);

void  imu_mat_identity(imu_mat3_t * p_m);
void  imu_mat_apply(imu_mat3_t const * p_m, imu_vec3_t const * p_v,
                    imu_vec3_t * p_out);

/**
 * @brief  Rotation that maps unit vector @p p_from onto +Z (Rodrigues).
 * @return false if @p p_from is (anti)parallel to -Z or not finite.
 */
bool  imu_mat_align_to_z(imu_vec3_t const * p_from, imu_mat3_t * p_m);

/**
 * @brief  Sensor frame -> vehicle frame using an axis permutation + signs.
 *         vehicle[k] = sign[k] * sensor[axis[k]].
 */
void  imu_frame_map(uint8_t const axis[3], int8_t const sign[3],
                    imu_vec3_t const * p_sensor, imu_vec3_t * p_vehicle);

/** Wrap an angle to [0, 360). */
float imu_wrap_360(float deg);

/** Wrap an angle difference to (-180, 180]. */
float imu_wrap_180(float deg);

/** First-order IIR coefficient for cut-off @p fc_hz at sample time @p dt_s. */
float imu_lpf_alpha(float fc_hz, float dt_s);

/** y += alpha * (x - y). */
float imu_lpf_step(float y, float x, float alpha);

/** Clamp helper. */
float imu_clampf(float x, float lo, float hi);

#endif /* IMU_MATH_H */
