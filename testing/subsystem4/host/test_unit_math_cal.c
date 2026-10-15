/**
 * @file    test_unit_math_cal.c
 * @brief   Unit tests: math helpers, parameter validation, calibration.
 */

#include <string.h>

#include "imu/imu_calibration.h"
#include "imu/imu_config.h"
#include "imu/imu_math.h"
#include "test_util.h"

static uint32_t g_rng = 99U;

static double urand(void)
{
    g_rng = (g_rng * 1664525U) + 1013904223U;
    return ((double)(g_rng >> 8) / 16777216.0);
}

static double nrand(void)
{
    double u1 = urand() + 1e-9;
    double u2 = urand();

    return (sqrt(-2.0 * log(u1)) * cos(6.283185307 * u2));
}

static void t_wrap(void)
{
    TEST_ASSERT_NEAR(imu_wrap_360(-1.0f), 359.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_360(360.0f), 0.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_360(720.5f), 0.5, 1e-3);
    TEST_ASSERT_NEAR(imu_wrap_360(-720.0f), 0.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(190.0f), -170.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(-180.0f), 180.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(180.0f), 180.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(-190.0f), 170.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(10.0f - 350.0f), 20.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(350.0f - 10.0f), -20.0, 1e-4);
}

static void t_lpf(void)
{
    float a1 = imu_lpf_alpha(1.0f, 0.02f);
    float a2 = imu_lpf_alpha(10.0f, 0.02f);
    float y  = 0.0f;
    int   i;

    TEST_ASSERT(a1 > 0.0f && a1 < a2 && a2 < 1.0f);
    TEST_ASSERT_NEAR(imu_lpf_alpha(0.0f, 0.02f), 1.0, 1e-6);
    for (i = 0; i < 500; i++)
    {
        y = imu_lpf_step(y, 1.0f, a1);
    }
    TEST_ASSERT_NEAR(y, 1.0, 1e-3);   /* unity DC gain */
}

static void t_align_to_z(void)
{
    int i;

    for (i = 0; i < 200; i++)
    {
        imu_vec3_t u = { (float)nrand(), (float)nrand(),
                         (float)fabs(nrand()) + 0.2f };
        imu_vec3_t r;
        imu_mat3_t m;
        float      det;

        TEST_ASSERT(imu_mat_align_to_z(&u, &m));
        (void)imu_vec_normalise(&u, &u);
        imu_mat_apply(&m, &u, &r);
        TEST_ASSERT_NEAR(r.x, 0.0, 1e-4);
        TEST_ASSERT_NEAR(r.y, 0.0, 1e-4);
        TEST_ASSERT_NEAR(r.z, 1.0, 1e-4);
        det = (m.m[0][0] * ((m.m[1][1] * m.m[2][2]) - (m.m[1][2] * m.m[2][1])))
            - (m.m[0][1] * ((m.m[1][0] * m.m[2][2]) - (m.m[1][2] * m.m[2][0])))
            + (m.m[0][2] * ((m.m[1][0] * m.m[2][1]) - (m.m[1][1] * m.m[2][0])));
        TEST_ASSERT_NEAR(det, 1.0, 1e-4);   /* proper rotation */
    }
    {
        imu_vec3_t z  = { 0.0f, 0.0f, 2.0f };
        imu_vec3_t nz = { 0.0f, 0.0f, -1.0f };
        imu_vec3_t nan_v = { NAN, 0.0f, 1.0f };
        imu_mat3_t m;

        TEST_ASSERT(imu_mat_align_to_z(&z, &m));
        TEST_ASSERT_NEAR(m.m[0][0], 1.0, 1e-6);
        TEST_ASSERT(!imu_mat_align_to_z(&nz, &m));
        TEST_ASSERT(!imu_mat_align_to_z(&nan_v, &m));
    }
}

static void t_frame_map(void)
{
    uint8_t const ax[3] = { 1U, 0U, 2U };
    int8_t const  sg[3] = { 1, -1, 1 };
    imu_vec3_t    s = { 1.0f, 2.0f, 3.0f };
    imu_vec3_t    v;

    imu_frame_map(ax, sg, &s, &v);
    TEST_ASSERT_NEAR(v.x, 2.0, 1e-6);
    TEST_ASSERT_NEAR(v.y, -1.0, 1e-6);
    TEST_ASSERT_NEAR(v.z, 3.0, 1e-6);
}

static void t_params_validate(void)
{
    imu_params_t p;

    imu_params_default(&p);
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_OK);
    TEST_ASSERT_EQ(imu_params_validate(NULL), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.axis_sign[1] = -1;                 /* mirror: left-handed */
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);              /* 90 deg yaw mount: valid */
    p.axis_map[0] = 1U; p.axis_sign[0] = 1;
    p.axis_map[1] = 0U; p.axis_sign[1] = -1;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_OK);

    imu_params_default(&p);              /* upside-down: x, -y, -z valid */
    p.axis_sign[1] = -1; p.axis_sign[2] = -1;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_OK);

    imu_params_default(&p);
    p.axis_map[1] = 0U;                  /* duplicate axis */
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.axis_sign[2] = 0;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.turn_exit_dps = p.turn_enter_dps + 1.0f;   /* inverted hysteresis */
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.impact_minor_g = NAN;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.sample_period_ms = 0U;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.turn_rate_window_ms = 1000U;       /* longer than heading history */
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.hump_min_peak_deg = p.hump_ascent_enter_deg - 0.5f;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.accel_odr_code = 0x0FU;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.wheelbase_mm = INFINITY;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.impact_critical_g = p.impact_minor_g - 0.1f;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);
}

static void cal_feed(imu_accel_cal_acc_t * p_acc, imu_vec3_t mean,
                     double sigma, uint32_t n)
{
    uint32_t i;

    for (i = 0U; i < n; i++)
    {
        imu_vec3_t v = { mean.x + (float)(sigma * nrand()),
                         mean.y + (float)(sigma * nrand()),
                         mean.z + (float)(sigma * nrand()) };

        imu_cal_accel_add(p_acc, &v);
    }
}

static void t_accel_cal_ok(void)
{
    imu_params_t        p;
    imu_accel_cal_acc_t acc;
    imu_accel_cal_t     cal;
    imu_vec3_t          m = { 0.0f, 0.0f, 1.02f };
    imu_vec3_t          r;

    imu_params_default(&p);
    imu_cal_accel_begin(&acc);
    cal_feed(&acc, m, 0.003, 100U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_OK);
    TEST_ASSERT(cal.b_valid);
    TEST_ASSERT_NEAR(cal.g_ref, 1.02, 0.002);
    TEST_ASSERT_NEAR(cal.noise_max_std_g, 0.003, 0.0015);
    TEST_ASSERT_NEAR(cal.mount_tilt_deg, 0.0, 0.5);
    imu_mat_apply(&cal.level_rot, &cal.gravity_mean_g, &r);
    TEST_ASSERT_NEAR(r.z, 1.02, 0.002);
}

static void t_accel_cal_mount_tilt(void)
{
    imu_params_t        p;
    imu_accel_cal_acc_t acc;
    imu_accel_cal_t     cal;
    float const         a = 10.0f * IMU_DEG_TO_RAD;
    imu_vec3_t          m = { sinf(a), 0.0f, cosf(a) };
    imu_vec3_t          r;

    imu_params_default(&p);
    imu_cal_accel_begin(&acc);
    cal_feed(&acc, m, 0.002, 100U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_OK);
    TEST_ASSERT_NEAR(cal.mount_pitch_deg, 10.0, 0.3);
    TEST_ASSERT_NEAR(cal.mount_tilt_deg, 10.0, 0.3);
    imu_mat_apply(&cal.level_rot, &m, &r);
    TEST_ASSERT_NEAR(r.x, 0.0, 0.003);
    TEST_ASSERT_NEAR(r.z, 1.0, 0.003);
}

static void t_accel_cal_failures(void)
{
    imu_params_t        p;
    imu_accel_cal_acc_t acc;
    imu_accel_cal_t     cal;
    imu_vec3_t          level = { 0.0f, 0.0f, 1.0f };
    imu_vec3_t          low   = { 0.0f, 0.0f, 0.7f };
    imu_vec3_t          tilt  = { 0.70710678f, 0.0f, 0.70710678f };
    imu_vec3_t          nan_v = { NAN, 0.0f, 1.0f };

    imu_params_default(&p);

    imu_cal_accel_begin(&acc);
    cal_feed(&acc, level, 0.05, 100U);           /* moving / handled */
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_MOTION);
    TEST_ASSERT(!cal.b_valid);

    imu_cal_accel_begin(&acc);
    cal_feed(&acc, low, 0.002, 100U);            /* wrong scale */
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_RANGE);

    imu_cal_accel_begin(&acc);
    cal_feed(&acc, tilt, 0.002, 100U);           /* 45 deg: axis map? */
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_RANGE);

    imu_cal_accel_begin(&acc);
    cal_feed(&acc, level, 0.002, 5U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_SAMPLES);

    imu_cal_accel_begin(&acc);
    imu_cal_accel_add(&acc, &nan_v);             /* ignored */
    TEST_ASSERT_EQ(acc.n, 0);
    TEST_ASSERT_EQ(imu_cal_accel_finish(NULL, &p, &cal), IMU_ERR_PARAM);

    /* Boundary: noise just below / just above tolerance. */
    imu_cal_accel_begin(&acc);
    cal_feed(&acc, level, 0.016, 400U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_OK);
    imu_cal_accel_begin(&acc);
    cal_feed(&acc, level, 0.025, 400U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_MOTION);
}

static void t_mag_cal(void)
{
    imu_params_t      p;
    imu_mag_cal_acc_t acc;
    imu_mag_cal_t     cal;
    int               i;
    float const       hx = 0.10f;
    float const       hy = -0.05f;
    float const       sx = 1.2f;
    float const       sy = 0.9f;

    imu_params_default(&p);
    imu_cal_mag_begin(&acc);
    for (i = 0; i < 360; i += 3)
    {
        float      a = (float)i * IMU_DEG_TO_RAD;
        imu_vec3_t m = { (0.40f * cosf(a) * sx) + hx,
                         (0.40f * sinf(a) * sy) + hy, 0.12f };

        imu_cal_mag_add(&acc, &m);
    }
    TEST_ASSERT_EQ(imu_cal_mag_finish(&acc, &p, &cal), IMU_OK);
    TEST_ASSERT_NEAR(cal.hard_iron_gauss.x, hx, 0.005);
    TEST_ASSERT_NEAR(cal.hard_iron_gauss.y, hy, 0.005);
    /* Corrected radii must be equal (circle). */
    TEST_ASSERT_NEAR(cal.soft_iron_scale.x * sx, cal.soft_iron_scale.y * sy,
                     0.02);
    TEST_ASSERT_NEAR(cal.ref_field_gauss,
                     sqrt(pow(0.4 * 1.05, 2) + (0.12 * 0.12)), 0.03);
    {
        imu_vec3_t raw = { (0.40f * sx) + hx, hy, 0.12f };
        imu_vec3_t c;

        imu_cal_mag_apply(&cal, &raw, &c);
        TEST_ASSERT_NEAR(c.y, 0.0, 0.01);
    }

    /* Partial rotation (60 deg) -> coverage error. */
    imu_cal_mag_begin(&acc);
    for (i = 0; i < 60; i++)
    {
        float      a = (float)i * IMU_DEG_TO_RAD;
        imu_vec3_t m = { 0.40f * cosf(a), 0.40f * sinf(a), 0.12f };

        imu_cal_mag_add(&acc, &m);
    }
    TEST_ASSERT_EQ(imu_cal_mag_finish(&acc, &p, &cal), IMU_ERR_CAL_COVERAGE);
    TEST_ASSERT(!cal.b_valid);

    imu_cal_mag_begin(&acc);
    TEST_ASSERT_EQ(imu_cal_mag_finish(&acc, &p, &cal), IMU_ERR_CAL_SAMPLES);
}

void suite_unit_math_cal(void)
{
    test_section("UNIT (white-box): math, parameters, calibration");
    test_run("S4-U019", "t_wrap", t_wrap);
    test_run("S4-U020", "t_lpf", t_lpf);
    test_run("S4-U021", "t_align_to_z", t_align_to_z);
    test_run("S4-U022", "t_frame_map", t_frame_map);
    test_run("S4-U023", "t_params_validate", t_params_validate);
    test_run("S4-U024", "t_accel_cal_ok", t_accel_cal_ok);
    test_run("S4-U025", "t_accel_cal_mount_tilt", t_accel_cal_mount_tilt);
    test_run("S4-U026", "t_accel_cal_failures", t_accel_cal_failures);
    test_run("S4-U027", "t_mag_cal", t_mag_cal);
}
