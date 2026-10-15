/**
 * @file    test_robustness.c
 * @brief   ROBUSTNESS / "PENETRATION" tests: adversarial and malformed
 *          inputs against every external input surface of Buddy 4.
 *
 * Attack / failure surfaces of this subsystem (it has no network surface):
 *   S1  I2C bus data (corrupted, spoofed, garbage, extreme, frozen)
 *   S2  inter-task API (NULL pointers, invalid commands, command floods)
 *   S3  encoder hint from Buddy 2 / MC (spoofed, absurd, replayed, future)
 *   S4  parameter block (out-of-range / NaN / fuzzed configuration)
 *   S5  time base (wrap-around, jumps, time going backwards)
 *   S6  event queue (permanently full consumer, critical-event storms)
 *   S7  floating-point (NaN / Inf injection into every processing entry)
 * Each test asserts: no crash (run under ASan/UBSan), bounded memory, no
 * NaN in published state, state machine stays inside its enum, and no
 * unsafe outcome (e.g. silent return to MONITORING, fake valid hump).
 */

#include <string.h>

#include "imu/imu_calibration.h"
#include "imu/imu_hump.h"
#include "imu/imu_processing.h"
#include "imu/imu_terrain.h"
#include "test_util.h"

static uint32_t g_frng = 2024U;

static uint32_t frand(void)
{
    g_frng ^= g_frng << 13;
    g_frng ^= g_frng >> 17;
    g_frng ^= g_frng << 5;
    return (g_frng);
}

static float frandf(float lo, float hi)
{
    return (lo + ((hi - lo) * ((float)(frand() & 0xFFFFFFU) / 16777215.0f)));
}

static bool status_sane(imu_status_t const * p)
{
    return ((p->state < IMU_STATE_COUNT)
            && (p->motion < IMU_MOTION_COUNT)
            && (p->fault <= IMU_FAULT_COLLISION)
            && (0 != isfinite(p->pitch_deg)) && (0 != isfinite(p->roll_deg))
            && (0 != isfinite(p->tilt_deg))
            && (0 != isfinite(p->heading_deg))
            && (0 != isfinite(p->turn_rate_dps))
            && (0 != isfinite(p->vibration_g))
            && (0 != isfinite(p->hump.max.peak_pitch_deg))
            && (0 != isfinite(p->hump.max.height_est_mm))
            && (p->hump.max.height_est_mm >= 0.0f)
            /* transient states never persist between steps */
            && (p->state != IMU_STATE_TILT_EVENT)
            && (p->state != IMU_STATE_TURN_EVENT)
            && (p->state != IMU_STATE_IMPACT_EVENT)
            && (p->state != IMU_STATE_FAULT_HANDLING)
            && (p->state != IMU_STATE_SENSOR_FAULT));
}

/* S1: random garbage on the bus for a long time. */
static void p01_i2c_garbage_fuzz(void)
{
    uint32_t t;
    bool     b_sane = true;

    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    harness_run_ms(3000U);
    sim_cfg()->b_garbage = true;
    for (t = 0U; t < 30000U; t += 100U)
    {
        imu_status_t st;

        harness_run_ms(100U);
        st = harness_status();
        b_sane = b_sane && status_sane(&st);
        TEST_ASSERT(st.state != IMU_STATE_MONITORING
                    || st.b_calibrated);
    }
    TEST_ASSERT(b_sane);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 0);
    TEST_ASSERT(harness_status().state != IMU_STATE_HUMP_DESCENT);
    TEST_ASSERT(g_cap.count < 3000U);   /* no event storm */
}

/* S1: random fault pattern sweep, every failure ratio. */
static void p02_failure_ratio_sweep(void)
{
    uint32_t every;

    for (every = 1U; every <= 12U; every++)
    {
        imu_status_t st;

        harness_setup();
        scn_add_segment(&g_h.scn, 3.0, 20.0, 0.2, 0.2, 0.0);
        TEST_ASSERT_EQ(harness_start(), IMU_OK);
        (void)harness_until_monitoring(5000U);
        sim_cfg()->fail_every  = every;
        sim_cfg()->fail_status = ((every % 2U) == 0U) ? IMU_HAL_TIMEOUT
                                                      : IMU_HAL_NACK;
        harness_run_ms(5000U);
        st = harness_status();
        TEST_ASSERT(status_sane(&st));
        if (1U == every)
        {
            /* 100 % loss: must end in a reported fault state. */
            TEST_ASSERT_MSG(st.state != IMU_STATE_MONITORING,
                            "every=%u", (unsigned)every);
            TEST_ASSERT(cap_count_type(IMU_EVT_IMU_FAULT) >= 1U);
        }
        else
        {
            /* Up to 50 % isolated NACKs are absorbed by the retries. */
            TEST_ASSERT_MSG(st.state == IMU_STATE_MONITORING,
                            "every=%u state=%s", (unsigned)every,
                            imu_state_name(st.state));
        }
    }
}

/* S1: extreme but well-formed values (saturation on every axis). */
static void p03_extreme_values(void)
{
    imu_status_t st;

    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    sim_cfg()->p_fn = NULL;
    sim_cfg()->const_accel.x = -100.0f;
    sim_cfg()->const_accel.y = 100.0f;
    sim_cfg()->const_accel.z = -100.0f;
    sim_cfg()->const_mag.x = 50.0f;
    harness_run_ms(2000U);
    st = harness_status();
    TEST_ASSERT(status_sane(&st));
    TEST_ASSERT(st.heading_quality == IMU_HEADING_INVALID);
    TEST_ASSERT(st.state != IMU_STATE_HUMP_PEAK);
}

/* S2: NULL / invalid arguments to every public entry point. */
static void p04_api_misuse(void)
{
    imu_terrain_t * p_sys = &g_h.sys;
    imu_status_t    st;
    imu_params_t    bad;
    imu_trace_entry_t tr[4];

    harness_setup();
    TEST_ASSERT_EQ(imu_terrain_init(NULL, NULL, NULL), IMU_ERR_PARAM);
    imu_params_default(&bad);
    bad.sample_period_ms = 1000000U;
    TEST_ASSERT_EQ(imu_terrain_init(p_sys, &bad, NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_init(p_sys, NULL, NULL), IMU_OK); /* defaults */
    imu_terrain_step(NULL, 0U);
    TEST_ASSERT_EQ(imu_terrain_command(NULL, IMU_CMD_RESET), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_command(p_sys, 0U), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_command(p_sys, 0x8000U), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_command(p_sys, 0xFFFFFFFFU), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_set_hint(NULL, NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_set_hint(p_sys, NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_get_status(NULL, &st), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_get_status(p_sys, NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_get_trace(NULL, tr, 4U), 0);
    TEST_ASSERT_EQ(imu_terrain_get_trace(p_sys, NULL, 4U), 0);
    imu_terrain_make_telemetry(NULL, NULL);
    TEST_ASSERT_EQ(imu_terrain_telemetry_json(NULL, NULL, 0U), -1);
    TEST_ASSERT(0 == strcmp(imu_state_name((imu_state_t)200), "INVALID"));
    TEST_ASSERT(0 == strcmp(imu_fault_name((imu_fault_t)200), "INVALID"));
    TEST_ASSERT(0 == strcmp(imu_motion_name((imu_motion_class_t)200),
                            "INVALID"));
    imu_params_default(NULL);
    TEST_ASSERT_EQ(imu_proc_init(NULL, NULL), IMU_ERR_PARAM);
    imu_proc_step(NULL, NULL, NULL, 0U, false, NULL);
    imu_proc_classify(NULL, 0U, IMU_HUMP_MONITORING, false, NULL);
    imu_proc_hf_accel(NULL, NULL, 0U, false, NULL);
    imu_cal_accel_add(NULL, NULL);
    imu_cal_mag_add(NULL, NULL);
    imu_cal_mag_apply(NULL, NULL, NULL);
}

/* S2: command flooding every cycle. */
static void p05_command_flood(void)
{
    uint32_t t;
    uint32_t mask = 0U;
    bool     b_sane = true;

    harness_setup();
    scn_add_segment(&g_h.scn, 3.0, 20.0, 0.2, 0.2, 0.0);
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    for (t = 0U; t < 15000U; t += 20U)
    {
        imu_status_t st;

        mask = (frand() & (IMU_CMD_ALL & ~IMU_CMD_STOP));
        if (0U != mask)
        {
            (void)imu_terrain_command(&g_h.sys, mask);
        }
        harness_run_ms(20U);
        st = harness_status();
        b_sane = b_sane && status_sane(&st);
    }
    TEST_ASSERT(b_sane);
    /* Event production is bounded by the period, not by the attacker. */
    TEST_ASSERT(g_cap.count <= (2U * (15000U / 20U)));

    /* A RESET storm keeps the subsystem out of MONITORING (it re-inits),
     * but never crashes and never produces false terrain events. */
    cap_reset(CAP_MAX);
    for (t = 0U; t < 3000U; t += 20U)
    {
        (void)imu_terrain_command(&g_h.sys, IMU_CMD_RESET);
        harness_run_ms(20U);
    }
    TEST_ASSERT(harness_status().state != IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
}

/* S3: spoofed / absurd encoder hints. */
static void p06_hint_spoofing(void)
{
    imu_motion_hint_t h;
    uint32_t          t;
    uint32_t          rejected;

    harness_setup();
    scn_add_segment(&g_h.scn, 3.0, 20.0, 0.25, 0.25, 0.0);
    scn_add_hump(&g_h.scn, 0.6, 0.025, 0.10);
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));

    for (t = 0U; t < 6000U; t += 20U)
    {
        (void)memset(&h, 0, sizeof(h));
        h.b_valid       = true;
        h.b_accel_valid = (0U != (frand() & 1U));
        h.speed_mm_s    = (int32_t)frand();          /* mostly absurd */
        h.accel_mm_s2   = (int32_t)frand();
        h.timestamp_ms  = frand();                   /* random age   */
        (void)imu_terrain_set_hint(&g_h.sys, &h);
        harness_run_ms(20U);
    }
    rejected = harness_status().hints_rejected;
    TEST_ASSERT(rejected > 200U);
    TEST_ASSERT(status_sane(&g_h.sys.status));
    /* The real hump is still detected despite the hostile hint stream. */
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
}

/* S3: hint replayed from far in the past and "future" hints. */
static void p07_hint_replay(void)
{
    imu_motion_hint_t h;

    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    (void)memset(&h, 0, sizeof(h));
    h.b_valid = true;
    h.speed_mm_s = 500;
    h.timestamp_ms = g_h.now_ms - 60000U;
    TEST_ASSERT_EQ(imu_terrain_set_hint(&g_h.sys, &h), IMU_OK);
    harness_run_ms(500U);
    TEST_ASSERT_EQ(harness_status().motion_conf, IMU_CONF_LOW); /* ignored */
    h.timestamp_ms = g_h.now_ms + 60000U;
    TEST_ASSERT_EQ(imu_terrain_set_hint(&g_h.sys, &h), IMU_OK);
    harness_run_ms(500U);
    TEST_ASSERT_EQ(harness_status().motion_conf, IMU_CONF_LOW);
}

/* S4: random parameter blocks. */
static void p08_parameter_fuzz(void)
{
    uint32_t i;
    uint32_t accepted = 0U;

    for (i = 0U; i < 300U; i++)
    {
        imu_params_t p;
        imu_err_t    err;

        imu_params_default(&p);
        switch (frand() % 8U)
        {
            case 0U: p.hump_ascent_enter_deg = frandf(-10.0f, 60.0f); break;
            case 1U: p.impact_minor_g = frandf(-1.0f, 20.0f);         break;
            case 2U: p.axis_map[frand() % 3U] = (uint8_t)(frand() % 5U); break;
            case 3U: p.axis_sign[frand() % 3U] = (int8_t)((int)(frand() % 5U) - 2); break;
            case 4U: p.sample_period_ms = frand() % 200U;              break;
            case 5U: p.attitude_lpf_hz = frandf(-5.0f, 200.0f);       break;
            case 6U: p.wheelbase_mm = (0U != (frand() & 1U)) ? NAN
                                      : frandf(0.0f, 3000.0f);         break;
            default: p.turn_exit_dps = frandf(0.0f, 60.0f);           break;
        }

        harness_setup();
        g_h.params = p;
        err = harness_start();
        if (IMU_OK == err)
        {
            accepted++;
            TEST_ASSERT_EQ(imu_params_validate(&p), IMU_OK);
            harness_run_ms(3000U);
            TEST_ASSERT(status_sane(&g_h.sys.status));
        }
        else
        {
            TEST_ASSERT_EQ(err, IMU_ERR_PARAM);
        }
    }
    TEST_ASSERT(accepted > 20U);
    TEST_ASSERT(accepted < 300U);
}

/* S5: millisecond counter wrap-around during a hump. */
static void p09_time_wraparound(void)
{
    uint32_t const start = 0xFFFFFFFFU - 4500U;   /* wraps after 4.5 s */

    harness_setup();
    sim_set_time_ms(start);
    g_h.scn.t_s = (double)start * 0.001;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    /* Drive over a hump that straddles the wrap. */
    scn_add_segment(&g_h.scn, g_h.scn.t_s, 1.0e6, 0.25, 0.25, 0.0);
    scn_add_hump(&g_h.scn, 0.3, 0.025, 0.10);
    harness_run_ms(6000U);
    TEST_ASSERT(sim_time_ms() < 10000U);            /* really wrapped */
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMU_FAULT), 0);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
}

/* S5: time jumps forward / backwards (debugger halt, bad time source). */
static void p10_time_jumps(void)
{
    uint32_t k;

    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    for (k = 0U; k < 20U; k++)
    {
        uint32_t jump = (0U != (k & 1U)) ? 50000U : (uint32_t)(-30000);

        imu_terrain_step(&g_h.sys, g_h.now_ms + jump);
        imu_terrain_step(&g_h.sys, g_h.now_ms);
        TEST_ASSERT(status_sane(&g_h.sys.status));
    }
    harness_run_ms(2000U);
    TEST_ASSERT(status_sane(&g_h.sys.status));
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
}

/* S6: consumer never reads + storm of critical events. */
static void p11_event_storm(void)
{
    uint32_t k;

    harness_setup();
    g_h.params.impact_cooldown_ms = 10U;
    scn_add_segment(&g_h.scn, 3.0, 30.0, 0.2, 0.2, 0.0);
    for (k = 0U; k < SCN_MAX_IMPACTS; k++)
    {
        scn_add_impact(&g_h.scn, 4.0 + (0.3 * (double)k), 0.01, 3.5,
                       -1.0f, 0.0f, 0.0f);
    }
    cap_reset(0U);
    g_h.b_no_drain = true;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    for (k = 0U; k < 40U; k++)
    {
        (void)imu_terrain_command(&g_h.sys, IMU_CMD_CLEAR_FAULT);
        harness_run_ms(100U);
        TEST_ASSERT(imu_events_pending(&g_h.sys.disp)
                    <= IMU_EVENT_CRITICAL_PENDING);
    }
    TEST_ASSERT(g_cap.notified >= 1U);    /* direct path independent */
    /* Repeated hits keep "conditions normal" false: even with the MC
     * spamming CLEAR_FAULT the subsystem does not resume monitoring
     * while the vehicle is still being struck. */
    TEST_ASSERT_EQ(g_h.sys.fault, IMU_FAULT_COLLISION);
    TEST_ASSERT(status_sane(&g_h.sys.status));
}

/* S7: NaN / Inf everywhere. */
static void p12_nan_injection(void)
{
    imu_params_t        p;
    imu_proc_t          proc;
    imu_hump_t          hump;
    imu_proc_out_t      out;
    imu_hump_out_t      hout;
    imu_hump_input_t    hin;
    imu_accel_cal_acc_t acc;
    imu_mag_cal_acc_t   macc;
    uint32_t            i;
    float const         evil[] = { NAN, INFINITY, -INFINITY, 1e30f, -1e30f,
                                   0.0f };

    imu_params_default(&p);
    TEST_ASSERT_EQ(imu_proc_init(&proc, &p), IMU_OK);
    TEST_ASSERT_EQ(imu_hump_init(&hump, &p), IMU_OK);
    imu_cal_accel_begin(&acc);
    imu_cal_mag_begin(&macc);
    for (i = 0U; i < 600U; i++)
    {
        imu_vec3_t   v = { evil[i % 6U], evil[(i / 6U) % 6U],
                           evil[(i / 36U) % 6U] };
        imu_impact_t imp;

        imu_proc_hf_accel(&proc, &v, i * 5U, false, &imp);
        imu_proc_step(&proc, &v, &v, i * 20U, false, &out);
        imu_proc_classify(&proc, i * 20U, IMU_HUMP_MONITORING, false, &out);
        (void)memset(&hin, 0, sizeof(hin));
        hin.pitch_deg    = v.x;
        hin.vert_accel_g = v.y;
        hin.speed_mm_s   = v.z;
        hin.b_speed_valid = true;
        imu_hump_step(&hump, i * 20U, &hin, &hout);
        imu_cal_accel_add(&acc, &v);
        imu_cal_mag_add(&macc, &v);
    }
    TEST_ASSERT_EQ(hump.stats.valid_count, 0);
    TEST_ASSERT(acc.n < 600U);
    TEST_ASSERT(0 != isfinite(proc.heading_f_deg));
}

/* S1: magnetometer overflow stream does not fault the IMU. */
static void p13_mag_overflow_stream(void)
{
    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    sim_cfg()->b_mag_overflow = true;
    harness_run_ms(3000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(harness_status().heading_quality, IMU_HEADING_INVALID);
    TEST_ASSERT(!harness_status().b_turn_rate_valid);
    sim_cfg()->b_mag_overflow = false;
    harness_run_ms(500U);
    TEST_ASSERT(harness_status().heading_quality != IMU_HEADING_INVALID);
}

/* S1: frozen-output boundary (BVA): N-1 cycles OK, N cycles -> fault. */
static void p14_frozen_boundary(void)
{
    harness_setup();
    g_h.params.frozen_cycles = 10U;
    g_h.scn.accel_noise_g = 0.003;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    sim_cfg()->b_frozen = true;
    harness_run_ms(9U * 20U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    harness_run_ms(3U * 20U);
    TEST_ASSERT_EQ(harness_status().fault, IMU_FAULT_FROZEN_DATA);
}

/* Static memory / message size budget (NFR14). */
static void p15_memory_budget(void)
{
    printf("      sizeof(imu_terrain_t)=%u  imu_status_t=%u  imu_event_t=%u "
           "imu_telemetry_t=%u imu_params_t=%u\n",
           (unsigned)sizeof(imu_terrain_t), (unsigned)sizeof(imu_status_t),
           (unsigned)sizeof(imu_event_t), (unsigned)sizeof(imu_telemetry_t),
           (unsigned)sizeof(imu_params_t));
    TEST_ASSERT(sizeof(imu_terrain_t) < 6144U);
    TEST_ASSERT(sizeof(imu_event_t) <= 40U);
    TEST_ASSERT(sizeof(imu_telemetry_t) <= 32U);
}

/* Long soak: 10 minutes of mixed driving, counts match ground truth. */
static void p16_soak(void)
{
    uint32_t k;
    double   t = 3.0;

    harness_setup();
    g_h.scn.vib_noise_g = 0.03;
    for (k = 0U; k < 8U; k++)
    {
        scn_add_segment(&g_h.scn, t, 40.0, 0.25, 0.25, 0.0);
        scn_add_hump(&g_h.scn, 2.0 + (10.0 * (double)k),
                     0.012 + (0.003 * (double)k), 0.10);
        t += 40.0;
        scn_add_segment(&g_h.scn, t, 1.5, 0.25, 0.25,
                        (0U != (k & 1U)) ? 60.0 : -60.0);
        t += 1.5;
    }
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    harness_run_ms((uint32_t)(t * 1000.0));
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 8);
    TEST_ASSERT_EQ(harness_status().hump.max.index, 8);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMU_FAULT), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
    TEST_ASSERT(status_sane(&g_h.sys.status));
}

void suite_robustness(void)
{
    test_section("ROBUSTNESS / PENETRATION (adversarial inputs)");
    test_run("S4-U094", "p01_i2c_garbage_fuzz", p01_i2c_garbage_fuzz);
    test_run("S4-U095", "p02_failure_ratio_sweep", p02_failure_ratio_sweep);
    test_run("S4-U096", "p03_extreme_values", p03_extreme_values);
    test_run("S4-U097", "p04_api_misuse", p04_api_misuse);
    test_run("S4-U098", "p05_command_flood", p05_command_flood);
    test_run("S4-U099", "p06_hint_spoofing", p06_hint_spoofing);
    test_run("S4-U100", "p07_hint_replay", p07_hint_replay);
    test_run("S4-U101", "p08_parameter_fuzz", p08_parameter_fuzz);
    test_run("S4-U102", "p09_time_wraparound", p09_time_wraparound);
    test_run("S4-U103", "p10_time_jumps", p10_time_jumps);
    test_run("S4-U104", "p11_event_storm", p11_event_storm);
    test_run("S4-U105", "p12_nan_injection", p12_nan_injection);
    test_run("S4-U106", "p13_mag_overflow_stream", p13_mag_overflow_stream);
    test_run("S4-U107", "p14_frozen_boundary", p14_frozen_boundary);
    test_run("S4-U108", "p15_memory_budget", p15_memory_budget);
    test_run("S4-U109", "p16_soak", p16_soak);
}
