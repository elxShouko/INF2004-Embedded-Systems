/**
 * @file    test_unit_processing.c
 * @brief   Unit tests: attitude, heading, turn rate, turn/tilt detectors,
 *          impact detector, motion classifier, encoder hint handling.
 */

#include <string.h>

#include "imu/imu_math.h"
#include "imu/imu_processing.h"
#include "test_util.h"

#define BH   (0.40f)
#define BUP  (0.12f)

static imu_params_t g_p;
static imu_proc_t   g_proc;
static uint32_t     g_t;

static void proc_setup(void)
{
    imu_accel_cal_t ac;
    imu_mag_cal_t   mc;

    imu_params_default(&g_p);
    TEST_ASSERT_EQ(imu_proc_init(&g_proc, &g_p), IMU_OK);
    (void)memset(&ac, 0, sizeof(ac));
    imu_mat_identity(&ac.level_rot);
    ac.g_ref   = 1.0f;
    ac.b_valid = true;
    (void)memset(&mc, 0, sizeof(mc));
    mc.b_valid = true;
    mc.soft_iron_scale.x = 1.0f;
    mc.soft_iron_scale.y = 1.0f;
    mc.soft_iron_scale.z = 1.0f;
    mc.ref_field_gauss = sqrtf((BH * BH) + (BUP * BUP));
    imu_proc_set_calibration(&g_proc, &ac, &mc);
    g_t = 1000U;
}

static imu_vec3_t mag_for(float heading_deg)
{
    float      a = heading_deg * IMU_DEG_TO_RAD;
    imu_vec3_t m = { BH * cosf(a), BH * sinf(a), BUP };

    return (m);
}

static void step(imu_vec3_t acc, imu_vec3_t const * p_mag, bool b_hump,
                 imu_proc_out_t * p_out)
{
    g_t += g_p.sample_period_ms;
    imu_proc_step(&g_proc, &acc, p_mag, g_t, b_hump, p_out);
}

static imu_vec3_t level(void)
{
    imu_vec3_t a = { 0.0f, 0.0f, 1.0f };

    return (a);
}

static void t_pitch_roll_sign(void)
{
    imu_proc_out_t out;
    imu_vec3_t     m = mag_for(0.0f);
    float          a = 10.0f * IMU_DEG_TO_RAD;
    imu_vec3_t     nose_up = { sinf(a), 0.0f, cosf(a) };
    imu_vec3_t     left_up = { 0.0f, sinf(a), cosf(a) };
    int            i;

    proc_setup();
    for (i = 0; i < 50; i++)
    {
        step(nose_up, &m, false, &out);
    }
    TEST_ASSERT_NEAR(out.pitch_deg, 10.0, 0.05);
    TEST_ASSERT_NEAR(out.roll_deg, 0.0, 0.05);
    TEST_ASSERT_NEAR(out.tilt_deg, 10.0, 0.05);
    TEST_ASSERT(out.b_attitude_reliable);

    proc_setup();
    for (i = 0; i < 50; i++)
    {
        step(left_up, &m, false, &out);
    }
    TEST_ASSERT_NEAR(out.roll_deg, 10.0, 0.05);
    TEST_ASSERT_NEAR(out.pitch_deg, 0.0, 0.05);
}

static void t_attitude_reliability(void)
{
    imu_proc_out_t out;
    imu_vec3_t     heavy = { 0.0f, 0.0f, 1.3f };
    int            i;

    proc_setup();
    for (i = 0; i < 30; i++)
    {
        step(heavy, NULL, false, &out);
    }
    TEST_ASSERT(!out.b_attitude_reliable);
    TEST_ASSERT_NEAR(out.vert_accel_g, 0.3, 0.01);
}

static void t_heading_cardinal(void)
{
    float const hdg[] = { 0.0f, 45.0f, 90.0f, 135.0f, 180.0f, 225.0f,
                          270.0f, 315.0f, 359.0f };
    uint32_t    k;

    for (k = 0U; k < (sizeof(hdg) / sizeof(hdg[0])); k++)
    {
        imu_proc_out_t out;
        imu_vec3_t     m = mag_for(hdg[k]);
        int            i;

        proc_setup();
        for (i = 0; i < 20; i++)
        {
            step(level(), &m, false, &out);
        }
        TEST_ASSERT_NEAR(imu_wrap_180(out.heading_deg - hdg[k]), 0.0, 0.5);
        TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_GOOD);
    }
}

static void t_heading_tilt_compensated(void)
{
    scenario_t s;
    imu_vec3_t a;
    imu_vec3_t m;
    imu_proc_out_t out;
    int        i;

    proc_setup();
    scn_init(&s);
    s.accel_noise_g   = 0.0;
    s.mag_noise_gauss = 0.0;
    s.heading0_deg    = 60.0;
    s.heading_deg     = 60.0;
    s.bh_gauss        = (double)BH;
    s.bup_gauss       = (double)BUP;
    scn_add_segment(&s, 0.0, 100.0, 0.0, 0.0, 0.0);
    s.seg[0].pitch_deg = 15.0;
    s.seg[0].roll_deg  = 10.0;

    for (i = 0; i < 40; i++)
    {
        scn_stimulus(&s, 0.02 * i, &a, &m);
        step(a, &m, false, &out);
    }
    TEST_ASSERT_NEAR(out.pitch_deg, 15.0, 0.2);
    TEST_ASSERT_NEAR(out.roll_deg, 10.0, 0.3);
    TEST_ASSERT_NEAR(imu_wrap_180(out.heading_deg - 60.0f), 0.0, 1.0);
}

/** Drive the heading at a constant rate; returns the last output. */
static void spin(float h0, float rate_dps, uint32_t ms, imu_proc_out_t * p_out,
                 uint32_t * p_starts, uint32_t * p_ends, float * p_delta)
{
    uint32_t t;
    float    h = h0;

    for (t = 0U; t < ms; t += g_p.sample_period_ms)
    {
        imu_vec3_t m = mag_for(imu_wrap_360(h));

        step(level(), &m, false, p_out);
        h += rate_dps * ((float)g_p.sample_period_ms * 0.001f);
        if (p_out->b_turn_event && (IMU_PHASE_START == p_out->turn_phase))
        {
            (*p_starts)++;
        }
        if (p_out->b_turn_event && (IMU_PHASE_END == p_out->turn_phase))
        {
            (*p_ends)++;
            *p_delta = p_out->turn_event_delta_deg;
        }
    }
}

static void t_turn_rate_wraparound(void)
{
    imu_proc_out_t out;
    uint32_t       s = 0U;
    uint32_t       e = 0U;
    float          d = 0.0f;

    proc_setup();
    spin(340.0f, 0.0f, 400U, &out, &s, &e, &d);
    spin(340.0f, 50.0f, 1000U, &out, &s, &e, &d);   /* crosses 360 -> 0 */
    TEST_ASSERT(out.b_turn_rate_valid);
    TEST_ASSERT_NEAR(out.turn_rate_dps, 50.0, 6.0);
    TEST_ASSERT(out.heading_deg < 40.0f);
}

static void t_turn_left_right(void)
{
    imu_proc_out_t out;
    uint32_t       s = 0U;
    uint32_t       e = 0U;
    float          d = 0.0f;

    /* LEFT: heading decreases. 90 deg at 60 deg/s. */
    proc_setup();
    spin(100.0f, 0.0f, 500U, &out, &s, &e, &d);
    spin(100.0f, -60.0f, 1500U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(g_proc.turn_state, IMU_TURN_LEFT);
    TEST_ASSERT_EQ(s, 1);
    spin(10.0f, 0.0f, 1000U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(e, 1);
    TEST_ASSERT_NEAR(d, -90.0, 12.0);
    TEST_ASSERT_EQ(g_proc.turn_state, IMU_TURN_NONE);

    /* RIGHT */
    proc_setup();
    s = 0U; e = 0U;
    spin(200.0f, 0.0f, 500U, &out, &s, &e, &d);
    spin(200.0f, 60.0f, 1500U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(g_proc.turn_state, IMU_TURN_RIGHT);
    spin(290.0f, 0.0f, 1000U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(e, 1);
    TEST_ASSERT_NEAR(d, 90.0, 12.0);
}

static void t_turn_debounce(void)
{
    imu_proc_out_t out;
    uint32_t       s = 0U;
    uint32_t       e = 0U;
    float          d = 0.0f;

    /* 100 ms wiggle at high rate: shorter than turn_min_duration. */
    proc_setup();
    spin(0.0f, 0.0f, 500U, &out, &s, &e, &d);
    spin(0.0f, 80.0f, 100U, &out, &s, &e, &d);
    spin(8.0f, -80.0f, 100U, &out, &s, &e, &d);
    spin(0.0f, 0.0f, 800U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(s, 0);

    /* Slow drift below the turn threshold -> heading-change events. */
    proc_setup();
    {
        uint32_t hc = 0U;
        uint32_t t;
        float    h = 0.0f;

        for (t = 0U; t < 5000U; t += 20U)
        {
            imu_vec3_t m = mag_for(imu_wrap_360(h));

            step(level(), &m, false, &out);
            h += 10.0f * 0.02f;
            if (out.b_turn_event)
            {
                TEST_ASSERT_EQ(out.turn_phase, IMU_PHASE_HEADING_CHANGE);
                hc++;
            }
        }
        TEST_ASSERT(hc >= 2U && hc <= 3U);   /* 50 deg / 20 deg */
    }
}

static void t_heading_disturbed(void)
{
    imu_proc_out_t out;
    uint32_t       starts = 0U;
    uint32_t       t;
    float          h = 0.0f;

    proc_setup();
    for (t = 0U; t < 3000U; t += 20U)
    {
        imu_vec3_t m = mag_for(imu_wrap_360(h));

        if ((t > 1000U) && (t < 2000U))
        {
            m.x += 0.5f;    /* steel / motor field: |B| doubles */
        }
        step(level(), &m, false, &out);
        if ((t > 1100U) && (t < 1900U))
        {
            TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_DISTURBED);
            TEST_ASSERT(!out.b_turn_rate_valid);
        }
        if (out.b_turn_event && (IMU_PHASE_START == out.turn_phase))
        {
            starts++;
        }
    }
    TEST_ASSERT_EQ(starts, 0);
    TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_GOOD);
}

static void t_heading_outlier(void)
{
    imu_proc_out_t out;
    imu_vec3_t     m0 = mag_for(30.0f);
    imu_vec3_t     m1 = mag_for(150.0f);
    int            i;

    proc_setup();
    for (i = 0; i < 30; i++)
    {
        step(level(), &m0, false, &out);
    }
    step(level(), &m1, false, &out);        /* single glitch */
    TEST_ASSERT_NEAR(out.heading_deg, 30.0, 1.0);
    for (i = 0; i < 10; i++)
    {
        step(level(), &m1, false, &out);     /* persistent -> accepted */
    }
    TEST_ASSERT_NEAR(out.heading_deg, 150.0, 3.0);
    step(level(), NULL, false, &out);
    TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_INVALID);
}

static void t_uncalibrated_mag(void)
{
    imu_proc_out_t out;
    imu_vec3_t     m = mag_for(10.0f);
    int            i;

    imu_params_default(&g_p);
    TEST_ASSERT_EQ(imu_proc_init(&g_proc, &g_p), IMU_OK);
    for (i = 0; i < 10; i++)
    {
        step(level(), &m, false, &out);
    }
    TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_UNCALIBRATED);
}

static void tilt_run(imu_vec3_t a, uint32_t ms, bool b_hump,
                     uint32_t * p_start, uint32_t * p_end,
                     imu_severity_t * p_sev)
{
    imu_proc_out_t out;
    imu_vec3_t     m = mag_for(0.0f);
    uint32_t       t;

    for (t = 0U; t < ms; t += g_p.sample_period_ms)
    {
        step(a, &m, b_hump, &out);
        if (out.b_tilt_event)
        {
            if (IMU_PHASE_END == out.tilt_phase)
            {
                (*p_end)++;
            }
            else
            {
                (*p_start)++;
                *p_sev = out.tilt_severity;
            }
        }
    }
}

static imu_vec3_t tilt_vec(float pitch_deg, float roll_deg)
{
    float      p = pitch_deg * IMU_DEG_TO_RAD;
    float      r = roll_deg * IMU_DEG_TO_RAD;
    imu_vec3_t a = { sinf(p), cosf(p) * sinf(r), cosf(p) * cosf(r) };

    return (a);
}

static void t_tilt_detector(void)
{
    uint32_t       s = 0U;
    uint32_t       e = 0U;
    imu_severity_t sev = IMU_SEV_INFO;

    proc_setup();
    tilt_run(level(), 500U, false, &s, &e, &sev);
    tilt_run(tilt_vec(0.0f, 20.0f), 200U, false, &s, &e, &sev); /* short */
    TEST_ASSERT_EQ(s, 0);
    tilt_run(tilt_vec(0.0f, 20.0f), 600U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 1);
    TEST_ASSERT_EQ(sev, IMU_SEV_MINOR);
    /* Hysteresis: 13 deg is below threshold but above threshold - hyst. */
    tilt_run(tilt_vec(0.0f, 13.0f), 1000U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(e, 0);
    tilt_run(tilt_vec(0.0f, 40.0f), 200U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 2);                 /* escalation to critical */
    TEST_ASSERT_EQ(sev, IMU_SEV_CRITICAL);
    tilt_run(level(), 1000U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(e, 1);

    /* Critical tilt is reported immediately (no debounce). */
    proc_setup();
    s = 0U; e = 0U;
    tilt_run(level(), 300U, false, &s, &e, &sev);
    tilt_run(tilt_vec(0.0f, 40.0f), 100U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 1);
    TEST_ASSERT_EQ(sev, IMU_SEV_CRITICAL);

    /* Pitch during a hump is expected: suppressed; roll is not. */
    proc_setup();
    s = 0U; e = 0U;
    tilt_run(level(), 300U, true, &s, &e, &sev);
    tilt_run(tilt_vec(20.0f, 0.0f), 1000U, true, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 0);
    tilt_run(tilt_vec(5.0f, 20.0f), 1000U, true, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 1);
}

static void hf_pulse(float peak, imu_vec3_t dir, uint32_t dur_ms,
                     bool b_hump, imu_impact_t * p_last, uint32_t * p_n)
{
    uint32_t t;

    for (t = 0U; t < (dur_ms + 50U); t += 5U)
    {
        float        k = (t < dur_ms) ? peak * sinf(IMU_PI_F * (float)t
                                                     / (float)dur_ms) : 0.0f;
        imu_vec3_t   a = { dir.x * k, dir.y * k, 1.0f + (dir.z * k) };
        imu_impact_t imp;

        g_t += 5U;
        imu_proc_hf_accel(&g_proc, &a, g_t, b_hump, &imp);
        if (imp.b_detected)
        {
            *p_last = imp;
            (*p_n)++;
        }
    }
}

static void hf_idle(uint32_t ms)
{
    uint32_t     t;
    imu_vec3_t   a = { 0.0f, 0.0f, 1.0f };
    imu_impact_t imp;

    for (t = 0U; t < ms; t += 5U)
    {
        g_t += 5U;
        imu_proc_hf_accel(&g_proc, &a, g_t, false, &imp);
    }
}

static void t_impact_detector(void)
{
    imu_vec3_t const fwd  = { -1.0f, 0.0f, 0.0f };
    imu_vec3_t const vert = { 0.0f, 0.0f, 1.0f };
    imu_impact_t     last;
    uint32_t         n = 0U;

    proc_setup();
    hf_pulse(3.0f, fwd, 20U, false, &last, &n);       /* during warm-up */
    TEST_ASSERT_EQ(n, 0);
    hf_idle(1000U);

    hf_pulse(2.5f, fwd, 20U, false, &last, &n);        /* frontal crash */
    TEST_ASSERT_EQ(n, 1);
    TEST_ASSERT_EQ(last.severity, IMU_SEV_CRITICAL);
    TEST_ASSERT(last.horiz_g > (0.8f * last.peak_g));

    hf_idle(1500U);
    hf_pulse(1.6f, vert, 20U, false, &last, &n);       /* vertical knock */
    TEST_ASSERT_EQ(n, 2);
    TEST_ASSERT_EQ(last.severity, IMU_SEV_MINOR);

    hf_idle(1500U);
    hf_pulse(0.6f, fwd, 20U, false, &last, &n);        /* vibration */
    TEST_ASSERT_EQ(n, 2);

    hf_idle(1500U);
    hf_pulse(1.5f, fwd, 400U, false, &last, &n);       /* slow push */
    TEST_ASSERT_EQ(n, 2);

    /* Sustained step (e.g. pushed against a wall, harsh braking): the
     * high-pass excursion lasts longer than an impact -> rejected. */
    hf_idle(1500U);
    {
        uint32_t     t;
        imu_impact_t imp;

        for (t = 0U; t < 400U; t += 5U)
        {
            imu_vec3_t a = { -3.0f, 0.0f, 1.0f };

            g_t += 5U;
            imu_proc_hf_accel(&g_proc, &a, g_t, false, &imp);
            if (imp.b_detected)
            {
                n++;
            }
        }
        hf_idle(500U);
    }
    TEST_ASSERT_EQ(n, 2);
    TEST_ASSERT(g_proc.imp_rejected_long >= 1U);

    hf_idle(1500U);
    hf_pulse(1.4f, vert, 20U, true, &last, &n);        /* hump: scaled */
    TEST_ASSERT_EQ(n, 2);
    hf_pulse(2.2f, vert, 20U, true, &last, &n);
    TEST_ASSERT_EQ(n, 3);
    TEST_ASSERT_EQ(last.severity, IMU_SEV_MINOR);      /* vertical */
}

static void t_impact_cooldown(void)
{
    imu_vec3_t const side = { 0.0f, 1.0f, 0.0f };
    imu_vec3_t const fwd  = { -1.0f, 0.0f, 0.0f };
    imu_impact_t     last;
    uint32_t         n = 0U;
    int              i;

    proc_setup();
    hf_idle(1000U);
    for (i = 0; i < 5; i++)
    {
        hf_pulse(1.5f, side, 15U, false, &last, &n);   /* minor x5 */
        hf_idle(50U);
    }
    TEST_ASSERT_EQ(n, 1);
    TEST_ASSERT(g_proc.imp_suppressed >= 4U);

    hf_pulse(3.5f, fwd, 15U, false, &last, &n);        /* escalation */
    TEST_ASSERT_EQ(n, 2);
    TEST_ASSERT_EQ(last.severity, IMU_SEV_CRITICAL);
    TEST_ASSERT(last.suppressed >= 4U);

    hf_idle(1200U);
    hf_pulse(1.5f, side, 15U, false, &last, &n);       /* after cooldown */
    TEST_ASSERT_EQ(n, 3);
}

static void classify_run(imu_vec3_t a, uint32_t ms, imu_hump_phase_t hp,
                         imu_proc_out_t * p_out, uint32_t * p_changes)
{
    uint32_t   t;
    imu_vec3_t m = mag_for(0.0f);

    for (t = 0U; t < ms; t += g_p.sample_period_ms)
    {
        step(a, &m, false, p_out);
        imu_proc_classify(&g_proc, g_t, hp, false, p_out);
        if (p_out->b_motion_changed)
        {
            (*p_changes)++;
        }
    }
}

static void t_classifier(void)
{
    imu_proc_out_t    out;
    uint32_t          ch = 0U;
    imu_vec3_t const  acc = { 0.10f, 0.0f, 1.0f };
    imu_vec3_t const  dec = { -0.10f, 0.0f, 1.0f };
    imu_vec3_t const  mid = { 0.045f, 0.0f, 1.0f };
    imu_motion_hint_t h;

    proc_setup();
    classify_run(level(), 500U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_STATIONARY);
    TEST_ASSERT_EQ(out.motion_conf, IMU_CONF_LOW);   /* inferred */

    classify_run(acc, 500U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_ACCELERATING);
    classify_run(mid, 500U, IMU_HUMP_MONITORING, &out, &ch);   /* hyst */
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_ACCELERATING);
    classify_run(dec, 500U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_DECELERATING);
    classify_run(level(), 500U, IMU_HUMP_ASCENT, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_CLIMBING_HUMP);
    classify_run(level(), 100U, IMU_HUMP_DESCENT, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_DESCENDING_HUMP);

    /* With an encoder hint the stationary/steady decision is measured. */
    h.b_valid = true;
    h.b_accel_valid = true;
    h.accel_mm_s2 = 0;
    h.speed_mm_s = 300;
    h.timestamp_ms = g_t;
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_OK);
    classify_run(level(), 100U, IMU_HUMP_MONITORING, &out, &ch);
    h.timestamp_ms = g_t;
    (void)imu_proc_set_hint(&g_proc, &h);
    classify_run(level(), 100U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_STEADY_MOTION);
    TEST_ASSERT_EQ(out.motion_conf, IMU_CONF_HIGH);

    h.speed_mm_s = 0;
    h.timestamp_ms = g_t;
    (void)imu_proc_set_hint(&g_proc, &h);
    classify_run(level(), 180U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_STATIONARY);
    TEST_ASSERT_EQ(out.motion_conf, IMU_CONF_HIGH);

    /* Impact overrides and holds. */
    imu_proc_classify(&g_proc, g_t, IMU_HUMP_MONITORING, true, &out);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_SUDDEN_IMPACT);
}

static void t_classifier_no_flapping(void)
{
    imu_proc_out_t out;
    uint32_t       ch = 0U;
    uint32_t       t;
    uint32_t       rng = 7U;
    imu_vec3_t     m = mag_for(0.0f);

    proc_setup();
    classify_run(level(), 300U, IMU_HUMP_MONITORING, &out, &ch);
    ch = 0U;
    /* Noise around the accel threshold (0.06 g) +/- 0.03 g every cycle. */
    for (t = 0U; t < 5000U; t += 20U)
    {
        float      n;
        imu_vec3_t a;

        rng = (rng * 1664525U) + 1013904223U;
        n = (((float)(rng >> 8) / 16777216.0f) - 0.5f) * 0.06f;
        a.x = 0.06f + n;
        a.y = 0.0f;
        a.z = 1.0f;
        step(a, &m, false, &out);
        imu_proc_classify(&g_proc, g_t, IMU_HUMP_MONITORING, false, &out);
        if (out.b_motion_changed)
        {
            ch++;
        }
    }
    TEST_ASSERT_MSG(ch <= 6U, "changes=%u", (unsigned)ch);
}

static void t_hint_validation(void)
{
    imu_motion_hint_t h;
    imu_proc_out_t    out;
    int               i;

    proc_setup();
    (void)memset(&h, 0, sizeof(h));
    TEST_ASSERT_EQ(imu_proc_set_hint(NULL, &h), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, NULL), IMU_ERR_PARAM);

    h.b_valid = true;
    h.speed_mm_s = 1000000;
    h.timestamp_ms = g_t;
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(g_proc.hints_rejected, 1);
    TEST_ASSERT(!imu_proc_hint_fresh(&g_proc, g_t));

    h.speed_mm_s = 100;
    h.b_accel_valid = true;
    h.accel_mm_s2 = -900000;
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_ERR_PARAM);

    h.b_accel_valid = false;
    h.timestamp_ms = g_t + 5000U;            /* from the future */
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_OK);
    TEST_ASSERT(!imu_proc_hint_fresh(&g_proc, g_t));

    h.timestamp_ms = g_t - 1000U;            /* stale */
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_OK);
    TEST_ASSERT(!imu_proc_hint_fresh(&g_proc, g_t));

    /* Derived acceleration from successive speeds: 0 -> 98 mm/s in 100 ms
     * = 0.98 m/s^2 = 0.1 g. */
    h.b_valid = true;
    h.speed_mm_s = 0;
    h.timestamp_ms = g_t;
    (void)imu_proc_set_hint(&g_proc, &h);
    h.speed_mm_s = 98;
    h.timestamp_ms = g_t + 100U;
    (void)imu_proc_set_hint(&g_proc, &h);
    TEST_ASSERT(g_proc.b_hint_accel_valid);
    TEST_ASSERT_NEAR(g_proc.hint_accel_g, 0.1, 0.005);

    /* Pitch compensation: 0.1 g forward accel no longer looks like 5.7 deg */
    {
        imu_vec3_t a = { 0.10f, 0.0f, 1.0f };
        imu_vec3_t m = mag_for(0.0f);

        h.b_accel_valid = true;
        h.accel_mm_s2 = 981;
        for (i = 0; i < 30; i++)
        {
            h.timestamp_ms = g_t;
            (void)imu_proc_set_hint(&g_proc, &h);
            step(a, &m, false, &out);
        }
        TEST_ASSERT_NEAR(out.pitch_deg, 0.0, 0.2);
        TEST_ASSERT(out.b_fwd_from_hint);
    }
}

static void t_nan_input(void)
{
    imu_proc_out_t out;
    imu_vec3_t     bad = { NAN, 0.0f, 1.0f };
    imu_vec3_t     inf = { 0.0f, INFINITY, 1.0f };
    imu_impact_t   imp;

    proc_setup();
    step(level(), NULL, false, &out);
    step(bad, &bad, false, &out);
    step(inf, &inf, false, &out);
    imu_proc_hf_accel(&g_proc, &bad, g_t, false, &imp);
    TEST_ASSERT(!imp.b_detected);
    step(level(), NULL, false, &out);
    TEST_ASSERT(0 != isfinite(out.pitch_deg));
    TEST_ASSERT(0 != isfinite(g_proc.accel_lp.x));
    TEST_ASSERT(0 != isfinite(g_proc.heading_f_deg));
}

void suite_unit_processing(void)
{
    test_section("UNIT (white-box): processing");
    test_run("S4-U028", "t_pitch_roll_sign", t_pitch_roll_sign);
    test_run("S4-U029", "t_attitude_reliability", t_attitude_reliability);
    test_run("S4-U030", "t_heading_cardinal", t_heading_cardinal);
    test_run("S4-U031", "t_heading_tilt_compensated", t_heading_tilt_compensated);
    test_run("S4-U032", "t_turn_rate_wraparound", t_turn_rate_wraparound);
    test_run("S4-U033", "t_turn_left_right", t_turn_left_right);
    test_run("S4-U034", "t_turn_debounce", t_turn_debounce);
    test_run("S4-U035", "t_heading_disturbed", t_heading_disturbed);
    test_run("S4-U036", "t_heading_outlier", t_heading_outlier);
    test_run("S4-U037", "t_uncalibrated_mag", t_uncalibrated_mag);
    test_run("S4-U038", "t_tilt_detector", t_tilt_detector);
    test_run("S4-U039", "t_impact_detector", t_impact_detector);
    test_run("S4-U040", "t_impact_cooldown", t_impact_cooldown);
    test_run("S4-U041", "t_classifier", t_classifier);
    test_run("S4-U042", "t_classifier_no_flapping", t_classifier_no_flapping);
    test_run("S4-U043", "t_hint_validation", t_hint_validation);
    test_run("S4-U044", "t_nan_input", t_nan_input);
}
