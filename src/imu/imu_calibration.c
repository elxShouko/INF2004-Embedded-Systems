/**
 * @file    imu_calibration.c
 * @brief   Buddy 4 accelerometer and magnetometer calibration.
 */

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "imu/imu_calibration.h"

#define CAL_MIN_SAMPLES       (10U)
#define CAL_MAX_ELLIPSE_RATIO (2.0f)

static float vec_get(imu_vec3_t const * p_v, uint8_t axis)
{
    float r;

    switch (axis)
    {
        case 0U:  r = p_v->x; break;
        case 1U:  r = p_v->y; break;
        default:  r = p_v->z; break;
    }

    return (r);
}

static void vec_set(imu_vec3_t * p_v, uint8_t axis, float value)
{
    switch (axis)
    {
        case 0U:  p_v->x = value; break;
        case 1U:  p_v->y = value; break;
        default:  p_v->z = value; break;
    }
}

/* ------------------------------------------------------------------------ */
/*  Accelerometer                                                            */
/* ------------------------------------------------------------------------ */

void imu_cal_accel_begin(imu_accel_cal_acc_t * p_acc)
{
    if (NULL != p_acc)
    {
        (void)memset(p_acc, 0, sizeof(*p_acc));
    }
}

static void welford(float * p_mean, float * p_m2, float x, uint32_t n)
{
    float delta = x - *p_mean;

    *p_mean += delta / (float)n;
    *p_m2   += delta * (x - *p_mean);
}

void imu_cal_accel_add(imu_accel_cal_acc_t * p_acc,
                       imu_vec3_t const * p_vehicle_g)
{
    if ((NULL == p_acc) || !imu_vec_is_finite(p_vehicle_g))
    {
        return;   /* non-finite samples are ignored, not accumulated */
    }

    p_acc->n++;
    welford(&p_acc->mean.x, &p_acc->m2.x, p_vehicle_g->x, p_acc->n);
    welford(&p_acc->mean.y, &p_acc->m2.y, p_vehicle_g->y, p_acc->n);
    welford(&p_acc->mean.z, &p_acc->m2.z, p_vehicle_g->z, p_acc->n);
}

imu_err_t imu_cal_accel_finish(imu_accel_cal_acc_t const * p_acc,
                               imu_params_t const * p_params,
                               imu_accel_cal_t * p_out)
{
    imu_vec3_t const z_axis = { 0.0f, 0.0f, 1.0f };
    imu_vec3_t       u;
    float            denom;
    float            g;
    float            c;

    if ((NULL == p_acc) || (NULL == p_params) || (NULL == p_out))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    imu_mat_identity(&p_out->level_rot);
    p_out->samples = p_acc->n;

    if (p_acc->n < CAL_MIN_SAMPLES)
    {
        return (IMU_ERR_CAL_SAMPLES);
    }

    denom = (float)(p_acc->n - 1U);
    p_out->noise_std_g.x = sqrtf(p_acc->m2.x / denom);
    p_out->noise_std_g.y = sqrtf(p_acc->m2.y / denom);
    p_out->noise_std_g.z = sqrtf(p_acc->m2.z / denom);
    p_out->noise_max_std_g = fmaxf(p_out->noise_std_g.x,
                                   fmaxf(p_out->noise_std_g.y,
                                         p_out->noise_std_g.z));
    p_out->gravity_mean_g = p_acc->mean;

    if ((0 == isfinite(p_out->noise_max_std_g))
        || (p_out->noise_max_std_g > p_params->cal_max_noise_g))
    {
        return (IMU_ERR_CAL_MOTION);
    }

    g = imu_vec_norm(&p_acc->mean);
    p_out->g_ref = g;
    if ((0 == isfinite(g)) || (g < p_params->cal_g_min)
        || (g > p_params->cal_g_max))
    {
        return (IMU_ERR_CAL_RANGE);
    }

    (void)imu_vec_normalise(&p_acc->mean, &u);
    c = imu_clampf(imu_vec_dot(&u, &z_axis), -1.0f, 1.0f);
    p_out->mount_tilt_deg  = acosf(c) * IMU_RAD_TO_DEG;
    p_out->mount_pitch_deg = atan2f(u.x, sqrtf((u.y * u.y) + (u.z * u.z)))
                           * IMU_RAD_TO_DEG;
    p_out->mount_roll_deg  = atan2f(u.y, u.z) * IMU_RAD_TO_DEG;

    if (p_out->mount_tilt_deg > p_params->cal_max_mount_tilt_deg)
    {
        return (IMU_ERR_CAL_RANGE);   /* wrong axis map or IMU upside-down */
    }

    if (!imu_mat_align_to_z(&u, &p_out->level_rot))
    {
        return (IMU_ERR_CAL_RANGE);
    }

    p_out->b_valid = true;
    return (IMU_OK);
}

/* ------------------------------------------------------------------------ */
/*  Magnetometer                                                             */
/* ------------------------------------------------------------------------ */

void imu_cal_mag_begin(imu_mag_cal_acc_t * p_acc)
{
    if (NULL != p_acc)
    {
        (void)memset(p_acc, 0, sizeof(*p_acc));
    }
}

void imu_cal_mag_add(imu_mag_cal_acc_t * p_acc,
                     imu_vec3_t const * p_sensor_gauss)
{
    if ((NULL == p_acc) || !imu_vec_is_finite(p_sensor_gauss))
    {
        return;
    }

    if (0U == p_acc->n)
    {
        p_acc->min = *p_sensor_gauss;
        p_acc->max = *p_sensor_gauss;
    }
    else
    {
        p_acc->min.x = fminf(p_acc->min.x, p_sensor_gauss->x);
        p_acc->min.y = fminf(p_acc->min.y, p_sensor_gauss->y);
        p_acc->min.z = fminf(p_acc->min.z, p_sensor_gauss->z);
        p_acc->max.x = fmaxf(p_acc->max.x, p_sensor_gauss->x);
        p_acc->max.y = fmaxf(p_acc->max.y, p_sensor_gauss->y);
        p_acc->max.z = fmaxf(p_acc->max.z, p_sensor_gauss->z);
    }

    p_acc->sum.x += p_sensor_gauss->x;
    p_acc->sum.y += p_sensor_gauss->y;
    p_acc->sum.z += p_sensor_gauss->z;
    p_acc->n++;
}

imu_err_t imu_cal_mag_finish(imu_mag_cal_acc_t const * p_acc,
                             imu_params_t const * p_params,
                             imu_mag_cal_t * p_out)
{
    uint8_t up_axis;
    uint8_t h[2];
    uint8_t k;
    uint8_t a;
    float   r[2];
    float   r_avg;
    float   v_mean;
    float   v_off;

    if ((NULL == p_acc) || (NULL == p_params) || (NULL == p_out))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    p_out->soft_iron_scale.x = 1.0f;
    p_out->soft_iron_scale.y = 1.0f;
    p_out->soft_iron_scale.z = 1.0f;
    p_out->samples = p_acc->n;

    if (p_acc->n < p_params->mag_cal_min_samples)
    {
        return (IMU_ERR_CAL_SAMPLES);
    }

    /* The two sensor axes that are horizontal on the vehicle. */
    up_axis = (p_params->axis_map[2] < 3U) ? p_params->axis_map[2] : 2U;
    k = 0U;
    for (a = 0U; a < 3U; a++)
    {
        if ((a != up_axis) && (k < 2U))
        {
            h[k] = a;
            k++;
        }
    }

    imu_vec_sub(&p_acc->max, &p_acc->min, &p_out->span_gauss);

    for (k = 0U; k < 2U; k++)
    {
        float lo   = vec_get(&p_acc->min, h[k]);
        float hi   = vec_get(&p_acc->max, h[k]);

        r[k] = 0.5f * (hi - lo);
        vec_set(&p_out->hard_iron_gauss, h[k], 0.5f * (hi + lo));

        if ((hi - lo) < p_params->mag_cal_min_span_gauss)
        {
            return (IMU_ERR_CAL_COVERAGE);
        }
    }

    if (((r[0] / r[1]) > CAL_MAX_ELLIPSE_RATIO)
        || ((r[1] / r[0]) > CAL_MAX_ELLIPSE_RATIO))
    {
        return (IMU_ERR_CAL_COVERAGE);   /* distorted / partial rotation */
    }

    r_avg = 0.5f * (r[0] + r[1]);
    vec_set(&p_out->soft_iron_scale, h[0], r_avg / r[0]);
    vec_set(&p_out->soft_iron_scale, h[1], r_avg / r[1]);

    /* Vertical axis: cannot be observed by a planar spin. */
    v_off = vec_get(&p_params->mag_hard_iron_gauss, up_axis);
    vec_set(&p_out->hard_iron_gauss, up_axis, v_off);
    vec_set(&p_out->soft_iron_scale, up_axis,
            vec_get(&p_params->mag_soft_iron_scale, up_axis));

    v_mean = (vec_get(&p_acc->sum, up_axis) / (float)p_acc->n) - v_off;
    p_out->ref_field_gauss = sqrtf((r_avg * r_avg) + (v_mean * v_mean));

    if ((0 == isfinite(p_out->ref_field_gauss))
        || (p_out->ref_field_gauss <= 0.0f))
    {
        return (IMU_ERR_CAL_COVERAGE);
    }

    p_out->b_valid = true;
    return (IMU_OK);
}

void imu_cal_mag_from_params(imu_params_t const * p_params,
                             imu_mag_cal_t * p_out)
{
    if ((NULL == p_params) || (NULL == p_out))
    {
        return;
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    p_out->b_valid         = p_params->b_mag_cal_valid;
    p_out->hard_iron_gauss = p_params->mag_hard_iron_gauss;
    p_out->soft_iron_scale = p_params->mag_soft_iron_scale;
    p_out->ref_field_gauss = p_params->mag_ref_field_gauss;
}

void imu_cal_mag_apply(imu_mag_cal_t const * p_cal,
                       imu_vec3_t const * p_raw_gauss,
                       imu_vec3_t * p_out)
{
    imu_vec3_t d;

    if ((NULL == p_cal) || (NULL == p_raw_gauss) || (NULL == p_out))
    {
        return;
    }

    imu_vec_sub(p_raw_gauss, &p_cal->hard_iron_gauss, &d);
    p_out->x = d.x * p_cal->soft_iron_scale.x;
    p_out->y = d.y * p_cal->soft_iron_scale.y;
    p_out->z = d.z * p_cal->soft_iron_scale.z;
}
