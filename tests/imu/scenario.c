/**
 * @file    scenario.c
 * @brief   HOST-ONLY vehicle motion generator (see scenario.h).
 */

#include <math.h>
#include <string.h>

#include "scenario.h"

#define SCN_PI        (3.14159265358979323846)
#define SCN_D2R       (SCN_PI / 180.0)
#define SCN_G         (9.80665)
#define SCN_DT_S      (0.0005)

static double scn_randn(scenario_t * p_s)
{
    double u1;
    double u2;

    p_s->seed = (p_s->seed * 1664525U) + 1013904223U;
    u1 = ((double)(p_s->seed >> 8) + 1.0) / 16777217.0;
    p_s->seed = (p_s->seed * 1664525U) + 1013904223U;
    u2 = ((double)(p_s->seed >> 8) + 1.0) / 16777217.0;

    return (sqrt(-2.0 * log(u1)) * cos(2.0 * SCN_PI * u2));
}

void scn_init(scenario_t * p_s)
{
    (void)memset(p_s, 0, sizeof(*p_s));
    p_s->wheelbase_m   = 0.140;
    p_s->imu_pos_frac  = 0.5;
    p_s->bh_gauss      = 0.40;
    p_s->bup_gauss     = 0.12;
    p_s->mount[0][0]   = 1.0;
    p_s->mount[1][1]   = 1.0;
    p_s->mount[2][2]   = 1.0;
    p_s->soft_scale.x  = 1.0f;
    p_s->soft_scale.y  = 1.0f;
    p_s->soft_scale.z  = 1.0f;
    p_s->accel_noise_g = 0.002;
    p_s->vib_noise_g   = 0.0;
    p_s->mag_noise_gauss = 0.002;
    p_s->seed          = 4242U;
    p_s->disturb_t0_s  = -1.0;
    p_s->disturb_t1_s  = -1.0;
}

void scn_add_segment(scenario_t * p_s, double t0, double dur, double v0,
                     double v1, double yaw_rate_dps)
{
    if (p_s->n_seg < SCN_MAX_SEGMENTS)
    {
        scn_segment_t * p = &p_s->seg[p_s->n_seg];

        (void)memset(p, 0, sizeof(*p));
        p->t_start_s    = t0;
        p->duration_s   = dur;
        p->v_start_mps  = v0;
        p->v_end_mps    = v1;
        p->yaw_rate_dps = yaw_rate_dps;
        p_s->n_seg++;
    }
}

void scn_add_hump(scenario_t * p_s, double start_m, double height_m,
                  double length_m)
{
    if (p_s->n_hump < SCN_MAX_HUMPS)
    {
        p_s->hump[p_s->n_hump].start_m  = start_m;
        p_s->hump[p_s->n_hump].height_m = height_m;
        p_s->hump[p_s->n_hump].length_m = length_m;
        p_s->n_hump++;
    }
}

void scn_add_impact(scenario_t * p_s, double t_s, double dur_s,
                    double peak_g, float dx, float dy, float dz)
{
    if (p_s->n_imp < SCN_MAX_IMPACTS)
    {
        scn_impact_t * p = &p_s->imp[p_s->n_imp];

        p->t_s           = t_s;
        p->duration_s    = dur_s;
        p->peak_g        = peak_g;
        p->dir_vehicle.x = dx;
        p->dir_vehicle.y = dy;
        p->dir_vehicle.z = dz;
        p_s->n_imp++;
    }
}

static scn_segment_t const * scn_seg_at(scenario_t const * p_s, double t)
{
    uint32_t i;

    for (i = 0U; i < p_s->n_seg; i++)
    {
        scn_segment_t const * p = &p_s->seg[i];

        if ((t >= p->t_start_s) && (t < (p->t_start_s + p->duration_s)))
        {
            return (p);
        }
    }

    return (NULL);
}

static void scn_kin(scenario_t const * p_s, double t, double * p_v,
                    double * p_a, double * p_yaw_rate,
                    double * p_roll, double * p_pitch)
{
    scn_segment_t const * p = scn_seg_at(p_s, t);

    *p_v = 0.0; *p_a = 0.0; *p_yaw_rate = 0.0; *p_roll = 0.0; *p_pitch = 0.0;
    if (NULL != p)
    {
        double f = (t - p->t_start_s) / p->duration_s;

        *p_a        = (p->v_end_mps - p->v_start_mps) / p->duration_s;
        *p_v        = p->v_start_mps + (f * (p->v_end_mps - p->v_start_mps));
        *p_yaw_rate = p->yaw_rate_dps;
        *p_roll     = p->roll_deg;
        *p_pitch    = p->pitch_deg;
    }
}

/** Ground height and its first/second derivative w.r.t. distance. */
static void scn_ground(scenario_t const * p_s, double x, double * p_z,
                       double * p_dz, double * p_d2z)
{
    uint32_t i;

    *p_z = 0.0; *p_dz = 0.0; *p_d2z = 0.0;
    for (i = 0U; i < p_s->n_hump; i++)
    {
        scn_hump_t const * p = &p_s->hump[i];
        double             u = x - p->start_m;

        if ((u >= 0.0) && (u <= p->length_m))
        {
            double k = 2.0 * SCN_PI / p->length_m;

            *p_z   += 0.5 * p->height_m * (1.0 - cos(k * u));
            *p_dz  += 0.5 * p->height_m * k * sin(k * u);
            *p_d2z += 0.5 * p->height_m * k * k * cos(k * u);
        }
    }
}

void scn_speed_at(scenario_t const * p_s, double t_s, double * p_v,
                  double * p_a)
{
    double yr;
    double r;
    double pt;

    scn_kin(p_s, t_s, p_v, p_a, &yr, &r, &pt);
}

double scn_true_peak_pitch_deg(double height_m, double length_m,
                               double wheelbase_m)
{
    double best = 0.0;
    double s;
    double k = 2.0 * SCN_PI / length_m;

    for (s = 0.0; s <= (length_m + wheelbase_m); s += 0.0005)
    {
        double zf = ((s >= 0.0) && (s <= length_m))
                    ? 0.5 * height_m * (1.0 - cos(k * s)) : 0.0;
        double sr = s - wheelbase_m;
        double zr = ((sr >= 0.0) && (sr <= length_m))
                    ? 0.5 * height_m * (1.0 - cos(k * sr)) : 0.0;
        double p  = asin((zf - zr) / wheelbase_m) / SCN_D2R;

        if (p > best)
        {
            best = p;
        }
    }

    return (best);
}

static void scn_integrate_to(scenario_t * p_s, double t)
{
    while (p_s->t_s < t)
    {
        double v;
        double a;
        double yr;
        double r;
        double pt;
        double dt = fmin(SCN_DT_S, t - p_s->t_s);

        scn_kin(p_s, p_s->t_s, &v, &a, &yr, &r, &pt);
        p_s->s_m         += v * dt;
        p_s->heading_deg += yr * dt;
        p_s->v_mps        = v;
        p_s->t_s         += dt;
    }
}

void scn_stimulus(void * p_ctx, double t_s, imu_vec3_t * p_accel_g,
                  imu_vec3_t * p_mag_gauss)
{
    scenario_t * p_s = (scenario_t *)p_ctx;
    double v, a, yr, roll_s, pitch_s;
    double zf, dzf, d2zf, zr, dzr, d2zr;
    double L = p_s->wheelbase_m;
    double k = p_s->imu_pos_frac;
    double pitch, roll, psi;
    double fx[3], ly[3], uz[3], l0[3], u0[3];
    double f_world[3], b_world[3];
    double f_b[3], m_b[3], f_s[3], m_s[3];
    double z_dd;
    uint32_t i;
    uint32_t r;

    scn_integrate_to(p_s, t_s);
    scn_kin(p_s, t_s, &v, &a, &yr, &roll_s, &pitch_s);

    scn_ground(p_s, p_s->s_m, &zf, &dzf, &d2zf);
    scn_ground(p_s, p_s->s_m - L, &zr, &dzr, &d2zr);

    pitch = asin(fmax(-0.99, fmin(0.99, (zf - zr) / L))) + (pitch_s * SCN_D2R);
    roll  = roll_s * SCN_D2R;
    psi   = p_s->heading_deg * SCN_D2R;
    if ((pitch / SCN_D2R) > p_s->max_pitch_deg)
    {
        p_s->max_pitch_deg = pitch / SCN_D2R;
    }

    /* Vertical acceleration of the IMU point. */
    z_dd = ((1.0 - k) * ((d2zr * v * v) + (dzr * a)))
         + (k * ((d2zf * v * v) + (dzf * a)));

    /* Body axes in world ENU. */
    fx[0] = sin(psi) * cos(pitch);
    fx[1] = cos(psi) * cos(pitch);
    fx[2] = sin(pitch);
    l0[0] = -cos(psi);
    l0[1] = sin(psi);
    l0[2] = 0.0;
    u0[0] = (fx[1] * l0[2]) - (fx[2] * l0[1]);
    u0[1] = (fx[2] * l0[0]) - (fx[0] * l0[2]);
    u0[2] = (fx[0] * l0[1]) - (fx[1] * l0[0]);
    for (i = 0U; i < 3U; i++)
    {
        ly[i] = (cos(roll) * l0[i]) + (sin(roll) * u0[i]);
        uz[i] = (-sin(roll) * l0[i]) + (cos(roll) * u0[i]);
    }

    /* World acceleration: along-path + centripetal + vertical, plus 1 g. */
    {
        double omega   = yr * SCN_D2R;             /* clockwise + */
        double lat_acc = -v * omega;               /* to the left */
        double hdir[3] = { sin(psi), cos(psi), 0.0 };

        for (i = 0U; i < 3U; i++)
        {
            f_world[i] = (a * hdir[i]) + (lat_acc * l0[i]);
        }
        f_world[2] += z_dd;
        for (i = 0U; i < 3U; i++)
        {
            f_world[i] /= SCN_G;
        }
        f_world[2] += 1.0;
    }

    b_world[0] = 0.0;
    b_world[1] = p_s->bh_gauss;
    b_world[2] = p_s->bup_gauss;

    f_b[0] = (f_world[0] * fx[0]) + (f_world[1] * fx[1]) + (f_world[2] * fx[2]);
    f_b[1] = (f_world[0] * ly[0]) + (f_world[1] * ly[1]) + (f_world[2] * ly[2]);
    f_b[2] = (f_world[0] * uz[0]) + (f_world[1] * uz[1]) + (f_world[2] * uz[2]);
    m_b[0] = (b_world[0] * fx[0]) + (b_world[1] * fx[1]) + (b_world[2] * fx[2]);
    m_b[1] = (b_world[0] * ly[0]) + (b_world[1] * ly[1]) + (b_world[2] * ly[2]);
    m_b[2] = (b_world[0] * uz[0]) + (b_world[1] * uz[1]) + (b_world[2] * uz[2]);

    /* Impacts (vehicle frame, half-sine pulse). */
    for (i = 0U; i < p_s->n_imp; i++)
    {
        scn_impact_t const * p = &p_s->imp[i];

        if ((t_s >= p->t_s) && (t_s <= (p->t_s + p->duration_s)))
        {
            double amp = p->peak_g * sin(SCN_PI * (t_s - p->t_s)
                                         / p->duration_s);

            f_b[0] += amp * (double)p->dir_vehicle.x;
            f_b[1] += amp * (double)p->dir_vehicle.y;
            f_b[2] += amp * (double)p->dir_vehicle.z;
        }
    }

    /* Noise. */
    for (i = 0U; i < 3U; i++)
    {
        f_b[i] += p_s->accel_noise_g * scn_randn(p_s);
        if (fabs(v) > 1e-3)
        {
            f_b[i] += p_s->vib_noise_g * scn_randn(p_s);
        }
        m_b[i] += p_s->mag_noise_gauss * scn_randn(p_s);
    }

    /* Mounting: sensor = M * vehicle. */
    for (r = 0U; r < 3U; r++)
    {
        f_s[r] = (p_s->mount[r][0] * f_b[0]) + (p_s->mount[r][1] * f_b[1])
               + (p_s->mount[r][2] * f_b[2]);
        m_s[r] = (p_s->mount[r][0] * m_b[0]) + (p_s->mount[r][1] * m_b[1])
               + (p_s->mount[r][2] * m_b[2]);
    }

    /* Hard / soft iron and disturbance (sensor frame). */
    m_s[0] = (m_s[0] * (double)p_s->soft_scale.x) + (double)p_s->hard_iron.x;
    m_s[1] = (m_s[1] * (double)p_s->soft_scale.y) + (double)p_s->hard_iron.y;
    m_s[2] = (m_s[2] * (double)p_s->soft_scale.z) + (double)p_s->hard_iron.z;
    if ((t_s >= p_s->disturb_t0_s) && (t_s < p_s->disturb_t1_s))
    {
        m_s[0] += (double)p_s->disturb_gauss.x;
        m_s[1] += (double)p_s->disturb_gauss.y;
        m_s[2] += (double)p_s->disturb_gauss.z;
    }

    p_accel_g->x   = (float)f_s[0];
    p_accel_g->y   = (float)f_s[1];
    p_accel_g->z   = (float)f_s[2];
    p_mag_gauss->x = (float)m_s[0];
    p_mag_gauss->y = (float)m_s[1];
    p_mag_gauss->z = (float)m_s[2];
}
