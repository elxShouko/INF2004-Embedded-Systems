/**
 * @file    imu_math.c
 * @brief   Math helpers for Buddy 4 (see imu_math.h).
 */

#include <math.h>
#include <stddef.h>

#include "imu/imu_math.h"

#define MATH_MIN_NORM      (1.0e-6f)
#define MATH_PARALLEL_EPS  (1.0e-7f)

float imu_vec_norm(imu_vec3_t const * p_v)
{
    return (sqrtf((p_v->x * p_v->x) + (p_v->y * p_v->y)
                  + (p_v->z * p_v->z)));
}

float imu_vec_dot(imu_vec3_t const * p_a, imu_vec3_t const * p_b)
{
    return ((p_a->x * p_b->x) + (p_a->y * p_b->y) + (p_a->z * p_b->z));
}

void imu_vec_cross(imu_vec3_t const * p_a, imu_vec3_t const * p_b,
                   imu_vec3_t * p_out)
{
    imu_vec3_t r;

    r.x = (p_a->y * p_b->z) - (p_a->z * p_b->y);
    r.y = (p_a->z * p_b->x) - (p_a->x * p_b->z);
    r.z = (p_a->x * p_b->y) - (p_a->y * p_b->x);
    *p_out = r;
}

void imu_vec_sub(imu_vec3_t const * p_a, imu_vec3_t const * p_b,
                 imu_vec3_t * p_out)
{
    p_out->x = p_a->x - p_b->x;
    p_out->y = p_a->y - p_b->y;
    p_out->z = p_a->z - p_b->z;
}

void imu_vec_scale(imu_vec3_t const * p_a, float k, imu_vec3_t * p_out)
{
    p_out->x = p_a->x * k;
    p_out->y = p_a->y * k;
    p_out->z = p_a->z * k;
}

bool imu_vec_is_finite(imu_vec3_t const * p_v)
{
    return ((NULL != p_v) && (0 != isfinite(p_v->x))
            && (0 != isfinite(p_v->y)) && (0 != isfinite(p_v->z)));
}

bool imu_vec_normalise(imu_vec3_t const * p_v, imu_vec3_t * p_out)
{
    float n;

    if (!imu_vec_is_finite(p_v))
    {
        return (false);
    }

    n = imu_vec_norm(p_v);
    if ((0 == isfinite(n)) || (n < MATH_MIN_NORM))
    {
        return (false);
    }

    imu_vec_scale(p_v, 1.0f / n, p_out);
    return (true);
}

void imu_mat_identity(imu_mat3_t * p_m)
{
    uint8_t r;
    uint8_t c;

    for (r = 0U; r < 3U; r++)
    {
        for (c = 0U; c < 3U; c++)
        {
            p_m->m[r][c] = (r == c) ? 1.0f : 0.0f;
        }
    }
}

void imu_mat_apply(imu_mat3_t const * p_m, imu_vec3_t const * p_v,
                   imu_vec3_t * p_out)
{
    imu_vec3_t r;

    r.x = (p_m->m[0][0] * p_v->x) + (p_m->m[0][1] * p_v->y)
        + (p_m->m[0][2] * p_v->z);
    r.y = (p_m->m[1][0] * p_v->x) + (p_m->m[1][1] * p_v->y)
        + (p_m->m[1][2] * p_v->z);
    r.z = (p_m->m[2][0] * p_v->x) + (p_m->m[2][1] * p_v->y)
        + (p_m->m[2][2] * p_v->z);
    *p_out = r;
}

bool imu_mat_align_to_z(imu_vec3_t const * p_from, imu_mat3_t * p_m)
{
    static imu_vec3_t const k_z = { 0.0f, 0.0f, 1.0f };
    imu_vec3_t u;
    imu_vec3_t v;
    float      c;
    float      k;
    float      vx[3][3];
    uint8_t    r;
    uint8_t    col;
    uint8_t    i;

    if (!imu_vec_normalise(p_from, &u))
    {
        return (false);
    }

    c = imu_vec_dot(&u, &k_z);
    if (c > (1.0f - MATH_PARALLEL_EPS))
    {
        imu_mat_identity(p_m);
        return (true);
    }
    if (c < (-1.0f + MATH_PARALLEL_EPS))
    {
        return (false);   /* upside-down: rotation axis undefined */
    }

    imu_vec_cross(&u, &k_z, &v);
    k = 1.0f / (1.0f + c);   /* (1 - c) / s^2 with s^2 = 1 - c^2 */

    /* Skew-symmetric matrix [v]x */
    vx[0][0] = 0.0f;  vx[0][1] = -v.z;  vx[0][2] = v.y;
    vx[1][0] = v.z;   vx[1][1] = 0.0f;  vx[1][2] = -v.x;
    vx[2][0] = -v.y;  vx[2][1] = v.x;   vx[2][2] = 0.0f;

    /* R = I + [v]x + [v]x^2 * k */
    for (r = 0U; r < 3U; r++)
    {
        for (col = 0U; col < 3U; col++)
        {
            float sq = 0.0f;

            for (i = 0U; i < 3U; i++)
            {
                sq += vx[r][i] * vx[i][col];
            }
            p_m->m[r][col] = ((r == col) ? 1.0f : 0.0f) + vx[r][col]
                           + (sq * k);
        }
    }

    return (true);
}

void imu_frame_map(uint8_t const axis[3], int8_t const sign[3],
                   imu_vec3_t const * p_sensor, imu_vec3_t * p_vehicle)
{
    float const s[3] = { p_sensor->x, p_sensor->y, p_sensor->z };
    float       v[3];
    uint8_t     k;

    for (k = 0U; k < 3U; k++)
    {
        uint8_t a = (axis[k] < 3U) ? axis[k] : k;

        v[k] = (float)sign[k] * s[a];
    }

    p_vehicle->x = v[0];
    p_vehicle->y = v[1];
    p_vehicle->z = v[2];
}

float imu_wrap_360(float deg)
{
    float r = fmodf(deg, 360.0f);

    if (r < 0.0f)
    {
        r += 360.0f;
    }
    if (r >= 360.0f)
    {
        r = 0.0f;
    }

    return (r);
}

float imu_wrap_180(float deg)
{
    float r = fmodf(deg, 360.0f);

    if (r > 180.0f)
    {
        r -= 360.0f;
    }
    else if (r <= -180.0f)
    {
        r += 360.0f;
    }
    else
    {
        /* already in range */
    }

    return (r);
}

float imu_lpf_alpha(float fc_hz, float dt_s)
{
    float rc;
    float alpha = 1.0f;

    if ((fc_hz > 0.0f) && (dt_s > 0.0f))
    {
        rc    = 1.0f / (2.0f * IMU_PI_F * fc_hz);
        alpha = dt_s / (rc + dt_s);
    }

    return (alpha);
}

float imu_lpf_step(float y, float x, float alpha)
{
    return (y + (alpha * (x - y)));
}

float imu_clampf(float x, float lo, float hi)
{
    float r = x;

    if (r < lo)
    {
        r = lo;
    }
    if (r > hi)
    {
        r = hi;
    }

    return (r);
}
