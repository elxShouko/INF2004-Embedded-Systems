/**
 * @file    test_unit_hump.c
 * @brief   Unit tests: hump state machine and measurement (direct input).
 */

#include <string.h>

#include "imu/imu_hump.h"
#include "imu/imu_math.h"
#include "test_util.h"

static imu_params_t g_p;
static imu_hump_t   g_hump;
static uint32_t     g_t;

typedef struct
{
    uint32_t detected;
    uint32_t peaks;
    uint32_t completed;
    uint32_t rejected;
    uint32_t new_max;
    imu_hump_meas_t last_done;
    imu_hump_validity_t last_reject;
    imu_hump_phase_t    seq[8];
    uint32_t            n_seq;
} hump_log_t;

static hump_log_t g_log;

static void hump_setup(void)
{
    imu_params_default(&g_p);
    TEST_ASSERT_EQ(imu_hump_init(&g_hump, &g_p), IMU_OK);
    (void)memset(&g_log, 0, sizeof(g_log));
    g_t = 10000U;
}

static void feed(float pitch, bool b_speed, float speed)
{
    imu_hump_input_t in;
    imu_hump_out_t   out;

    (void)memset(&in, 0, sizeof(in));
    in.pitch_deg           = pitch;
    in.b_attitude_reliable = true;
    in.b_speed_valid       = b_speed;
    in.speed_mm_s          = speed;
    g_t += g_p.sample_period_ms;
    imu_hump_step(&g_hump, g_t, &in, &out);

    if ((0U == g_log.n_seq) || (g_log.seq[g_log.n_seq - 1U] != out.phase))
    {
        if (g_log.n_seq < 8U)
        {
            g_log.seq[g_log.n_seq] = out.phase;
            g_log.n_seq++;
        }
    }
    if (out.b_detected)  { g_log.detected++; }
    if (out.b_peak)      { g_log.peaks++; }
    if (out.b_completed) { g_log.completed++; g_log.last_done = out.meas; }
    if (out.b_rejected)  { g_log.rejected++;
                           g_log.last_reject = out.meas.validity; }
    if (out.b_new_max)   { g_log.new_max++; }
}

static void flat(uint32_t ms)
{
    uint32_t t;

    for (t = 0U; t < ms; t += g_p.sample_period_ms)
    {
        feed(0.0f, false, 0.0f);
    }
}

/** Positive lobe, level gap, negative lobe (time-scaled by @p k). */
static void hump(float peak, float trough, float k)
{
    uint32_t const up   = (uint32_t)(400.0f * k);
    uint32_t const gap  = (uint32_t)(100.0f * k);
    uint32_t const down = (uint32_t)(400.0f * k);
    uint32_t       t;

    for (t = 0U; t < up; t += g_p.sample_period_ms)
    {
        feed(peak * sinf(IMU_PI_F * (float)t / (float)up), false, 0.0f);
    }
    for (t = 0U; t < gap; t += g_p.sample_period_ms)
    {
        feed(0.0f, false, 0.0f);
    }
    for (t = 0U; t < down; t += g_p.sample_period_ms)
    {
        feed(-trough * sinf(IMU_PI_F * (float)t / (float)down), false, 0.0f);
    }
    flat(400U);
}

static void t_canonical(void)
{
    hump_setup();
    flat(500U);
    hump(6.0f, 6.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.detected, 1);
    TEST_ASSERT_EQ(g_log.peaks, 1);
    TEST_ASSERT_EQ(g_log.completed, 1);
    TEST_ASSERT_EQ(g_log.rejected, 0);
    TEST_ASSERT_NEAR(g_log.last_done.peak_pitch_deg, 6.0, 0.1);
    TEST_ASSERT(g_log.last_done.min_pitch_deg < -5.5f);
    TEST_ASSERT_EQ(g_log.last_done.index, 1);
    TEST_ASSERT_EQ(g_log.last_done.validity, IMU_HUMP_VALID);
    /* Sequence: MONITORING -> ASCENT -> PEAK -> DESCENT -> MONITORING */
    TEST_ASSERT_EQ(g_log.n_seq, 5);
    TEST_ASSERT_EQ(g_log.seq[1], IMU_HUMP_ASCENT);
    TEST_ASSERT_EQ(g_log.seq[2], IMU_HUMP_PEAK);
    TEST_ASSERT_EQ(g_log.seq[3], IMU_HUMP_DESCENT);
    TEST_ASSERT_EQ(g_log.seq[4], IMU_HUMP_MONITORING);
}

static void t_boundaries(void)
{
    /* Just above the minimum valid peak. */
    hump_setup();
    flat(500U);
    hump(3.8f, 3.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 1);

    /* Crosses the ascent threshold but stays below min valid peak. */
    hump_setup();
    flat(500U);
    hump(3.3f, 3.3f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 0);
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.last_reject, IMU_HUMP_REJ_LOW_PEAK);

    /* Below the ascent threshold: invisible. */
    hump_setup();
    flat(500U);
    hump(2.8f, 2.8f, 1.0f);
    TEST_ASSERT_EQ(g_log.detected, 0);

    /* Descent too shallow: no valid descent. */
    hump_setup();
    flat(500U);
    hump(6.0f, 1.8f, 1.0f);
    flat(1500U);                      /* crest timeout elapses */
    TEST_ASSERT_EQ(g_log.completed, 0);
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.last_reject, IMU_HUMP_REJ_NO_DESCENT);
}

static void t_spike_rejected(void)
{
    hump_setup();
    flat(500U);
    feed(12.0f, false, 0.0f);      /* 20 ms spike */
    feed(-12.0f, false, 0.0f);
    flat(500U);
    feed(9.0f, false, 0.0f);
    feed(9.0f, false, 0.0f);       /* 40 ms < ascent_min 60 ms */
    flat(500U);
    TEST_ASSERT_EQ(g_log.detected, 0);
    TEST_ASSERT_EQ(g_log.completed, 0);
}

static void t_slope_and_no_descent(void)
{
    uint32_t t;

    /* Continuous ramp: never peaks -> rejected as SLOPE. */
    hump_setup();
    flat(500U);
    for (t = 0U; t < 5000U; t += 20U)
    {
        feed(5.0f, false, 0.0f);
    }
    TEST_ASSERT_EQ(g_log.completed, 0);
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.last_reject, IMU_HUMP_REJ_SLOPE);

    /* Acceleration artefact: up then flat, no nose-down -> NO_DESCENT. */
    hump_setup();
    flat(500U);
    for (t = 0U; t < 500U; t += 20U)
    {
        feed(4.5f, false, 0.0f);
    }
    flat(3000U);
    TEST_ASSERT_EQ(g_log.completed, 0);
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.last_reject, IMU_HUMP_REJ_NO_DESCENT);
}

static void t_artefact_then_real_hump(void)
{
    uint32_t t;

    hump_setup();
    flat(500U);
    for (t = 0U; t < 500U; t += 20U)
    {
        feed(4.5f, false, 0.0f);      /* artefact */
    }
    flat(300U);
    hump(7.0f, 7.0f, 1.0f);           /* real hump right behind it */
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.completed, 1);
    TEST_ASSERT_NEAR(g_log.last_done.peak_pitch_deg, 7.0, 0.2);
}

static void t_multiple_retain_max(void)
{
    hump_setup();
    flat(500U);
    hump(5.0f, 5.0f, 1.0f);
    flat(200U);
    hump(9.0f, 9.0f, 1.0f);
    flat(200U);
    hump(7.0f, 7.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 3);
    TEST_ASSERT_EQ(g_log.new_max, 2);
    TEST_ASSERT_EQ(g_hump.stats.valid_count, 3);
    TEST_ASSERT(g_hump.stats.b_has_max);
    TEST_ASSERT_NEAR(g_hump.stats.max.peak_pitch_deg, 9.0, 0.1);
    TEST_ASSERT_EQ(g_hump.stats.max.index, 2);
    TEST_ASSERT_NEAR(g_hump.stats.last.peak_pitch_deg, 7.0, 0.1);
    TEST_ASSERT_EQ(g_hump.stats.last.index, 3);

    /* Equal peak: maximum keeps the first occurrence (stable). */
    flat(200U);
    hump(9.0f, 9.0f, 1.0f);
    TEST_ASSERT_EQ(g_hump.stats.max.index, 2);
}

static void t_no_double_count(void)
{
    uint32_t t;

    hump_setup();
    flat(500U);
    hump(6.0f, 6.0f, 1.0f);
    /* Wobble after the hump (suspension bounce) below the threshold, and
     * one above-threshold rebound shorter than the re-arm window. */
    for (t = 0U; t < 600U; t += 20U)
    {
        feed(2.0f * sinf((float)t * 0.05f), false, 0.0f);
    }
    TEST_ASSERT_EQ(g_log.completed, 1);
    TEST_ASSERT_EQ(g_log.detected, 1);
}

static void t_rearm(void)
{
    /* Second hump closer than the minimum gap is not counted twice;
     * a properly separated one is. */
    hump_setup();
    flat(500U);
    hump(6.0f, 6.0f, 1.0f);           /* ends with 400 ms flat */
    flat(400U);
    hump(6.0f, 6.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 2);
}

static void t_speed_variation(void)
{
    float const k[] = { 0.25f, 0.5f, 1.0f, 2.0f, 3.0f };
    uint32_t    i;

    for (i = 0U; i < (sizeof(k) / sizeof(k[0])); i++)
    {
        hump_setup();
        flat(500U);
        hump(8.0f, 8.0f, k[i]);
        TEST_ASSERT_MSG(1U == g_log.completed, "time scale %.2f", (double)k[i]);
        TEST_ASSERT_NEAR(g_log.last_done.peak_pitch_deg, 8.0, 0.6);
    }

    /* Far too slow (8 s traversal) exceeds the max event duration. */
    hump_setup();
    flat(500U);
    hump(8.0f, 8.0f, 8.0f);
    TEST_ASSERT_EQ(g_log.completed, 0);
}

static void t_reset_and_abort(void)
{
    uint32_t t;

    hump_setup();
    flat(500U);
    hump(6.0f, 6.0f, 1.0f);
    TEST_ASSERT_EQ(g_hump.stats.valid_count, 1);
    imu_hump_reset_mission(&g_hump);
    TEST_ASSERT_EQ(g_hump.stats.valid_count, 0);
    TEST_ASSERT(!g_hump.stats.b_has_max);
    TEST_ASSERT(!g_hump.stats.b_has_last);

    flat(500U);
    for (t = 0U; t < 200U; t += 20U)
    {
        feed(6.0f, false, 0.0f);
    }
    TEST_ASSERT_EQ(g_hump.phase, IMU_HUMP_ASCENT);
    imu_hump_abort(&g_hump, g_t);
    TEST_ASSERT_EQ(g_hump.phase, IMU_HUMP_MONITORING);
    TEST_ASSERT_EQ(g_hump.stats.rejected_count, 1);
    TEST_ASSERT_EQ(g_hump.stats.valid_count, 0);

    imu_hump_reset_mission(NULL);
    imu_hump_abort(NULL, 0U);
    imu_hump_step(NULL, 0U, NULL, NULL);
    TEST_ASSERT_EQ(imu_hump_init(NULL, &g_p), IMU_ERR_PARAM);
}

static void t_nan_ignored(void)
{
    hump_setup();
    flat(500U);
    feed(NAN, false, 0.0f);
    feed(INFINITY, false, 0.0f);
    hump(6.0f, 6.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 1);
}

static void t_height_estimates(void)
{
    imu_hump_meas_t m;

    imu_params_default(&g_p);
    (void)memset(&m, 0, sizeof(m));
    m.peak_pitch_deg = 12.37f;        /* asin(30 / 140) */
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_method, IMU_HEIGHT_METHOD_GEOMETRIC);
    TEST_ASSERT_NEAR(m.height_est_mm, 30.0, 0.1);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_LOW);      /* uncalibrated */

    g_p.b_hump_height_calibrated = true;
    g_p.hump_height_gain = 1.1f;
    g_p.hump_height_offset_mm = -1.0f;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_NEAR(m.height_est_mm, 32.0, 0.1);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_MEDIUM);

    m.b_length_valid = true;
    m.length_est_mm  = 400.0f;        /* long hump: pitch = slope only */
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_LOW);

    imu_params_default(&g_p);
    g_p.height_method = IMU_HEIGHT_METHOD_INTEGRATED;
    m.b_rise_valid = true;
    m.rise_mm      = 15.0f;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_method, IMU_HEIGHT_METHOD_INTEGRATED);
    TEST_ASSERT_NEAR(m.height_est_mm, 30.0, 0.1);   /* 15 / 0.5 */
    m.b_rise_valid = false;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_method, IMU_HEIGHT_METHOD_GEOMETRIC);

    g_p.height_method = IMU_HEIGHT_METHOD_NONE;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_NONE);

    imu_params_default(&g_p);
    m.peak_pitch_deg = -3.0f;          /* never negative heights */
    m.b_attitude_degraded = true;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT(m.height_est_mm >= 0.0f);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_LOW);
}

static void t_hint_gating(void)
{
    uint32_t t;

    /* Vehicle stationary (encoders): tilting by hand is not a hump. */
    hump_setup();
    for (t = 0U; t < 500U; t += 20U)
    {
        feed(0.0f, true, 0.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(8.0f * sinf(IMU_PI_F * (float)t / 400.0f), true, 0.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(-8.0f * sinf(IMU_PI_F * (float)t / 400.0f), true, 0.0f);
    }
    TEST_ASSERT_EQ(g_log.detected, 0);

    /* Moving with speed: integrated rise and length are measured. */
    hump_setup();
    for (t = 0U; t < 500U; t += 20U)
    {
        feed(0.0f, true, 300.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(8.0f * sinf(IMU_PI_F * (float)t / 400.0f), true, 300.0f);
    }
    for (t = 0U; t < 100U; t += 20U)
    {
        feed(0.0f, true, 300.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(-8.0f * sinf(IMU_PI_F * (float)t / 400.0f), true, 300.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(0.0f, true, 300.0f);
    }
    TEST_ASSERT_EQ(g_log.completed, 1);
    TEST_ASSERT(g_log.last_done.b_rise_valid);
    TEST_ASSERT(g_log.last_done.rise_mm > 5.0f);
    TEST_ASSERT(g_log.last_done.length_est_mm > 200.0f);
}

static void t_names(void)
{
    TEST_ASSERT(0 == strcmp(imu_hump_validity_name(IMU_HUMP_VALID), "VALID"));
    TEST_ASSERT(0 == strcmp(imu_hump_validity_name((imu_hump_validity_t)99),
                            "UNKNOWN"));
}

void suite_unit_hump(void)
{
    test_section("UNIT (white-box): hump detector");
    RUN_TEST(t_canonical);
    RUN_TEST(t_boundaries);
    RUN_TEST(t_spike_rejected);
    RUN_TEST(t_slope_and_no_descent);
    RUN_TEST(t_artefact_then_real_hump);
    RUN_TEST(t_multiple_retain_max);
    RUN_TEST(t_no_double_count);
    RUN_TEST(t_rearm);
    RUN_TEST(t_speed_variation);
    RUN_TEST(t_reset_and_abort);
    RUN_TEST(t_nan_ignored);
    RUN_TEST(t_height_estimates);
    RUN_TEST(t_hint_gating);
    RUN_TEST(t_names);
}
