/**
 * @file    test_blackbox.c
 * @brief   BLACK-BOX system tests: only the public imu_terrain API, the
 *          emitted events and the status snapshot are observed. The sensor
 *          is the register-level simulator driven by the physical vehicle
 *          model (known ground truth). Covers the 16 required scenarios
 *          plus additional integration behaviours.
 *
 * Test design technique per case is noted: EP = equivalence partitioning,
 * BVA = boundary value, ST = state-transition, SC = scenario/use-case.
 */

#include <string.h>

#include "imu/imu_math.h"
#include "imu/imu_terrain.h"
#include "test_util.h"

/* Standard timeline: 0..3 s stationary (calibration), then segments. */
static void bb_boot(void)
{
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
}

static void bb_drive(double t0, double dur, double v)
{
    scn_add_segment(&g_h.scn, t0, dur, v, v, 0.0);
}

static uint32_t count_motion_seen(imu_motion_class_t m)
{
    uint32_t n   = 0U;
    uint32_t lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    uint32_t i;

    for (i = 0U; i < lim; i++)
    {
        if ((IMU_EVT_MOTION == g_cap.ev[i].type)
            && (g_cap.ev[i].data.motion.motion == (uint8_t)m))
        {
            n++;
        }
    }
    return (n);
}

static uint32_t count_status(imu_event_type_t type, imu_status_code_t code)
{
    uint32_t n   = 0U;
    uint32_t lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    uint32_t i;

    for (i = 0U; i < lim; i++)
    {
        if ((g_cap.ev[i].type == (uint8_t)type)
            && (g_cap.ev[i].data.fault.status == (uint8_t)code))
        {
            n++;
        }
    }
    return (n);
}

/* --- 1. Sensor initialisation and communication failures (ST, EP) ---- */
static void bb01_init_failure_then_success(void)
{
    imu_status_t st;

    harness_setup();
    sim_cfg()->b_accel_absent = true;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    harness_run_ms(3000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_INITIALISATION);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_INIT_FAILED);
    TEST_ASSERT(!st.b_sensor_ok);
    TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT, IMU_STATUS_FAULT_RAISED), 1);
    TEST_ASSERT(g_cap.notified >= 1U);
    TEST_ASSERT(!harness_trace_has(IMU_STATE_MONITORING));

    sim_cfg()->b_accel_absent = false;      /* cable re-seated */
    TEST_ASSERT(harness_until_monitoring(6000U));
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_READY), 1);
    TEST_ASSERT_EQ(harness_status().fault, IMU_FAULT_NONE);
}

static void bb02_wrong_device(void)
{
    harness_setup();
    sim_cfg()->b_wrong_id = true;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_INITIALISATION);
    TEST_ASSERT_EQ(harness_status().last_error, IMU_ERR_ID);
}

static void bb03_calibration_while_moving(void)
{
    imu_status_t st;

    harness_setup();
    g_h.scn.vib_noise_g = 0.05;
    bb_drive(0.0, 6.0, 0.2);                 /* moving during start-up */
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    harness_run_ms(5500U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_CALIBRATION);
    TEST_ASSERT(count_status(IMU_EVT_STATUS, IMU_STATUS_CAL_RETRY) >= 1U);
    TEST_ASSERT(harness_until_monitoring(6000U));   /* stopped -> ok */
    st = harness_status();
    TEST_ASSERT(st.b_calibrated);
    TEST_ASSERT(st.cal_noise_g <= g_h.params.cal_max_noise_g);
    TEST_ASSERT(g_h.sys.cal_attempts == 0U);
}

/* --- 2. Normal stationary readings (EP) ------------------------------ */
static void bb04_stationary(void)
{
    imu_status_t st;
    uint32_t     t;

    harness_setup();
    bb_boot();
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_READY), 1);
    for (t = 0U; t < 10000U; t += 500U)
    {
        harness_run_ms(500U);
        st = harness_status();
        TEST_ASSERT(fabsf(st.pitch_deg) < 0.5f);
        TEST_ASSERT(fabsf(st.roll_deg) < 0.5f);
    }
    TEST_ASSERT_EQ(st.motion, IMU_MOTION_STATIONARY);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_DETECTED), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_TILT), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMU_FAULT), 0);
    TEST_ASSERT(st.b_sensor_ok);
    TEST_ASSERT(st.b_calibrated);
    TEST_ASSERT_NEAR(st.cal_g_ref, 1.0, 0.01);
}

/* Sensor mounted tilted and rotated: axis map + levelling (SC). */
static void bb05_mounting(void)
{
    double const a = 8.0 * 3.14159265358979 / 180.0;
    imu_status_t st;

    harness_setup();
    /* Sensor rotated +90 deg about Z (sensor X = vehicle LEFT, sensor Y =
     * vehicle BACK), then pitched by 8 deg on its bracket. */
    (void)memset(g_h.scn.mount, 0, sizeof(g_h.scn.mount));
    g_h.scn.mount[0][0] = 0.0;      g_h.scn.mount[0][1] = cos(a);
    g_h.scn.mount[0][2] = sin(a);
    g_h.scn.mount[1][0] = -1.0;
    g_h.scn.mount[2][1] = -sin(a);  g_h.scn.mount[2][2] = cos(a);
    /* vehicle fwd = -sensor Y, vehicle left = +sensor X, up = +sensor Z */
    g_h.params.axis_map[0] = 1U; g_h.params.axis_sign[0] = -1;
    g_h.params.axis_map[1] = 0U; g_h.params.axis_sign[1] = 1;
    g_h.params.axis_map[2] = 2U; g_h.params.axis_sign[2] = 1;
    bb_drive(3.0, 4.0, 0.25);
    scn_add_segment(&g_h.scn, 7.0, 2.0, 0.25, 0.25, -60.0);  /* left */
    bb_drive(9.0, 3.0, 0.25);
    scn_add_hump(&g_h.scn, 0.5, 0.025, 0.10);
    bb_boot();
    st = harness_status();
    TEST_ASSERT_NEAR(st.cal_mount_tilt_deg, 8.0, 0.5);
    TEST_ASSERT(fabsf(st.pitch_deg) < 0.5f);
    harness_run_ms(10000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 1);
    TEST_ASSERT(cap_count_type(IMU_EVT_TURNING) >= 2U);
    {
        imu_event_t const * p = cap_last_of(IMU_EVT_TURNING);

        TEST_ASSERT(NULL != p);
        if (NULL != p)
        {
            TEST_ASSERT_EQ(p->data.turn.direction, IMU_TURN_LEFT);
        }
    }
}

/* --- 3. Normal straight-line movement (EP) --------------------------- */
static void bb06_straight_cruise(void)
{
    imu_status_t st;

    harness_setup();
    g_h.b_hint = true;
    g_h.scn.vib_noise_g = 0.03;
    bb_drive(3.0, 12.0, 0.3);
    bb_boot();
    harness_run_ms(11000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.motion, IMU_MOTION_STEADY_MOTION);
    TEST_ASSERT_EQ(st.motion_conf, IMU_CONF_HIGH);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_DETECTED), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_TILT), 0);
    TEST_ASSERT_EQ(count_motion_seen(IMU_MOTION_TURNING_LEFT), 0);
    TEST_ASSERT_EQ(count_motion_seen(IMU_MOTION_TURNING_RIGHT), 0);
    TEST_ASSERT_EQ(st.heading_quality, IMU_HEADING_UNCALIBRATED);
}

/* --- 4. Acceleration and deceleration (EP) --------------------------- */
static void bb07_accel_decel_with_hint(void)
{
    harness_setup();
    g_h.b_hint = true;
    g_h.scn.vib_noise_g = 0.01;
    scn_add_segment(&g_h.scn, 3.0, 0.6, 0.0, 0.4, 0.0);   /* +0.068 g */
    bb_drive(3.6, 3.0, 0.4);
    scn_add_segment(&g_h.scn, 6.6, 0.6, 0.4, 0.0, 0.0);   /* -0.068 g */
    bb_boot();
    harness_run_ms(7000U);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_ACCELERATING) >= 1U);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_STEADY_MOTION) >= 1U);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_DECELERATING) >= 1U);
    TEST_ASSERT_EQ(harness_status().motion, IMU_MOTION_STATIONARY);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_DETECTED), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
}

static void bb07b_accel_without_hint(void)
{
    harness_setup();
    g_h.scn.vib_noise_g = 0.01;
    scn_add_segment(&g_h.scn, 3.0, 0.6, 0.0, 0.4, 0.0);
    bb_drive(3.6, 4.0, 0.4);
    scn_add_segment(&g_h.scn, 7.6, 0.6, 0.4, 0.0, 0.0);
    bb_boot();
    harness_run_ms(8000U);
    /* Documented limitation: the pitch artefact may open a candidate, but
     * it must never be counted as a hump. */
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 0);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_DECELERATING) >= 1U);
}

/* --- 5. Left and right turns (EP) ------------------------------------ */
static void bb08_turns(void)
{
    uint32_t i;
    uint32_t lim;
    uint32_t left_end = 0U;
    uint32_t right_end = 0U;

    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    bb_drive(3.0, 1.0, 0.2);
    scn_add_segment(&g_h.scn, 4.0, 1.5, 0.2, 0.2, -60.0);   /* 90 L */
    bb_drive(5.5, 2.0, 0.2);
    scn_add_segment(&g_h.scn, 7.5, 1.5, 0.2, 0.2, 60.0);    /* 90 R */
    bb_drive(9.0, 2.0, 0.2);
    bb_boot();
    harness_run_ms(8500U);

    lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    for (i = 0U; i < lim; i++)
    {
        imu_event_t const * p = &g_cap.ev[i];

        if ((IMU_EVT_TURNING == p->type) && (IMU_PHASE_END == p->data.turn.phase))
        {
            if (IMU_TURN_LEFT == p->data.turn.direction)
            {
                left_end++;
                TEST_ASSERT_NEAR((double)p->data.turn.delta_cdeg / 100.0,
                                 -90.0, 15.0);
            }
            else
            {
                right_end++;
                TEST_ASSERT_NEAR((double)p->data.turn.delta_cdeg / 100.0,
                                 90.0, 15.0);
            }
        }
    }
    TEST_ASSERT_EQ(left_end, 1);
    TEST_ASSERT_EQ(right_end, 1);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_TURNING_LEFT) >= 1U);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_TURNING_RIGHT) >= 1U);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
}

/* --- 6/7. Small and large hump (EP, accuracy vs ground truth) -------- */
static void run_single_hump(double h_m, double len_m, double v,
                            imu_hump_meas_t * p_out)
{
    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    bb_drive(3.0, 30.0, v);
    scn_add_hump(&g_h.scn, 0.4, h_m, len_m);
    bb_boot();
    harness_run_ms((uint32_t)(1000.0 * ((0.4 + len_m + 0.6) / v)) + 1500U);
    (void)memset(p_out, 0, sizeof(*p_out));
    if (harness_status().hump.b_has_last)
    {
        *p_out = harness_status().hump.last;
    }
}

static void bb09_small_hump(void)
{
    imu_hump_meas_t m;
    double const    truth = scn_true_peak_pitch_deg(0.012, 0.10, 0.14);

    run_single_hump(0.012, 0.10, 0.25, &m);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_DETECTED), 1);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_PEAK), 1);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 1);
    TEST_ASSERT_NEAR(m.peak_pitch_deg, truth, 0.25 * truth);
    TEST_ASSERT_NEAR(m.height_est_mm, 12.0, 4.0);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_LOW);
    printf("      small hump: truth %.2f deg, measured %.2f deg, est %.1f mm "
           "(true 12.0)\n", truth, (double)m.peak_pitch_deg,
           (double)m.height_est_mm);
}

static void bb10_large_hump(void)
{
    imu_hump_meas_t m;
    double const    truth = scn_true_peak_pitch_deg(0.030, 0.10, 0.14);

    run_single_hump(0.030, 0.10, 0.25, &m);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT_NEAR(m.peak_pitch_deg, truth, 0.20 * truth);
    TEST_ASSERT_NEAR(m.height_est_mm, 30.0, 7.0);
    TEST_ASSERT(m.peak_vert_up_g > 0.0f);
    printf("      large hump: truth %.2f deg, measured %.2f deg, est %.1f mm "
           "(true 30.0)\n", truth, (double)m.peak_pitch_deg,
           (double)m.height_est_mm);
}

/* --- 8. Multiple humps, retain highest (SC) -------------------------- */
static void bb11_multiple_humps(void)
{
    imu_status_t        st;
    imu_telemetry_t     tele;
    imu_event_t const * p_last;
    uint32_t            i;
    uint32_t            lim;
    uint32_t            new_max = 0U;

    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    bb_drive(3.0, 30.0, 0.25);
    scn_add_hump(&g_h.scn, 0.4, 0.015, 0.10);
    scn_add_hump(&g_h.scn, 1.2, 0.035, 0.10);
    scn_add_hump(&g_h.scn, 2.0, 0.020, 0.10);
    bb_boot();
    harness_run_ms(11000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 3);
    TEST_ASSERT_EQ(st.hump.max.index, 2);
    TEST_ASSERT_EQ(st.hump.last.index, 3);
    TEST_ASSERT(st.hump.max.peak_pitch_deg > st.hump.last.peak_pitch_deg);
    TEST_ASSERT_EQ(st.hump.rejected_count, 0);

    lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    for (i = 0U; i < lim; i++)
    {
        if ((IMU_EVT_HUMP_COMPLETED == g_cap.ev[i].type)
            && (0U != g_cap.ev[i].data.hump.b_new_max))
        {
            new_max++;
        }
    }
    TEST_ASSERT_EQ(new_max, 2);
    p_last = cap_last_of(IMU_EVT_HUMP_COMPLETED);
    TEST_ASSERT(NULL != p_last);
    if (NULL != p_last)
    {
        TEST_ASSERT_EQ(p_last->data.hump.max_peak_pitch_cdeg,
                       (int16_t)lroundf(st.hump.max.peak_pitch_deg * 100.0f));
    }
    imu_terrain_make_telemetry(&st, &tele);
    TEST_ASSERT_EQ(tele.hump_count, 3);
    TEST_ASSERT_EQ(tele.hump_peak_cdeg,
                   (int16_t)lroundf(st.hump.max.peak_pitch_deg * 100.0f));
    TEST_ASSERT(tele.hump_peak_height_mm > 25);
}

/* --- 9. Noise and false hump detections (EP) ------------------------- */
static void bb12_noise_no_false_humps(void)
{
    uint32_t k;

    harness_setup();
    g_h.scn.vib_noise_g = 0.06;          /* rough floor, worn motors */
    bb_drive(3.0, 30.0, 0.3);
    for (k = 0U; k < 6U; k++)            /* 2 mm ridges: below threshold */
    {
        scn_add_hump(&g_h.scn, 0.3 + (0.5 * (double)k), 0.002, 0.04);
    }
    for (k = 0U; k < 4U; k++)            /* vertical knocks */
    {
        scn_add_impact(&g_h.scn, 5.0 + (double)k * 1.7, 0.01, 0.8,
                       0.0f, 0.0f, 1.0f);
    }
    bb_boot();
    harness_run_ms(12000U);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
    TEST_ASSERT(harness_status().b_sensor_ok);
}

/* --- 10. Short sudden impact -> fault handling (ST) ------------------ */
static void bb13_impact_collision(void)
{
    imu_status_t        st;
    imu_event_t const * p;

    harness_setup();
    bb_drive(3.0, 2.0, 0.3);
    scn_add_impact(&g_h.scn, 4.5, 0.015, 3.0, -1.0f, 0.0f, 0.0f);
    bb_boot();
    harness_run_ms(2400U);
    TEST_ASSERT(harness_status().state != IMU_STATE_FAULT_SAFE);
    harness_run_ms(200U);                    /* impact at t = 4.5 s */
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_FAULT_SAFE);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_COLLISION);
    TEST_ASSERT_EQ(st.motion, IMU_MOTION_SUDDEN_IMPACT);
    TEST_ASSERT(st.b_sensor_ok);                 /* sensor itself is fine */
    p = cap_last_of(IMU_EVT_IMPACT);
    TEST_ASSERT(NULL != p);
    if (NULL != p)
    {
        TEST_ASSERT_EQ(p->severity, IMU_SEV_CRITICAL);
        TEST_ASSERT(p->data.impact.peak_mg > 2000);
        TEST_ASSERT(p->data.impact.horiz_mg > p->data.impact.vert_mg);
    }
    TEST_ASSERT(g_cap.notified >= 1U);
    TEST_ASSERT_EQ(g_cap.last_notified.type, IMU_EVT_IMPACT);
    TEST_ASSERT(harness_trace_has(IMU_STATE_IMPACT_EVENT));
    TEST_ASSERT(harness_trace_has(IMU_STATE_FAULT_HANDLING));

    /* No silent return: stays in FAULT/SAFE without acknowledgement. */
    harness_run_ms(5000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_FAULT_SAFE);

    TEST_ASSERT_EQ(imu_terrain_command(&g_h.sys, IMU_CMD_CLEAR_FAULT), IMU_OK);
    harness_run_ms(1000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_RESUMED), 1);
}

/* --- 11. Repeated impact suppression (BVA on cool-down) -------------- */
static void bb14_repeated_impacts(void)
{
    uint32_t            k;
    imu_event_t const * p;

    harness_setup();
    bb_drive(3.0, 6.0, 0.2);
    for (k = 0U; k < 5U; k++)
    {
        scn_add_impact(&g_h.scn, 4.0 + (0.12 * (double)k), 0.012, 1.6,
                       0.0f, 1.0f, 0.0f);          /* side scrapes */
    }
    scn_add_impact(&g_h.scn, 6.0, 0.012, 1.6, 0.0f, 1.0f, 0.0f);
    bb_boot();
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 2);
    p = cap_last_of(IMU_EVT_IMPACT);
    TEST_ASSERT(NULL != p);
    if (NULL != p)
    {
        TEST_ASSERT_EQ(p->severity, IMU_SEV_MINOR);
        TEST_ASSERT(p->data.impact.suppressed >= 3U);
    }
    TEST_ASSERT(harness_status().state != IMU_STATE_FAULT_SAFE);
}

/* --- 12. Tilt events (EP, BVA) --------------------------------------- */
static void bb15_tilt(void)
{
    imu_event_t const * p;
    uint32_t            i;
    uint32_t            lim;
    uint32_t            starts = 0U;
    uint32_t            ends   = 0U;

    harness_setup();
    scn_add_segment(&g_h.scn, 4.0, 2.0, 0.0, 0.0, 0.0);
    g_h.scn.seg[0].roll_deg = 20.0;              /* side slope */
    scn_add_segment(&g_h.scn, 8.0, 1.0, 0.0, 0.0, 0.0);
    g_h.scn.seg[1].roll_deg = 40.0;              /* about to roll over */
    bb_boot();
    harness_run_ms(8000U);

    lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    for (i = 0U; i < lim; i++)
    {
        p = &g_cap.ev[i];
        if (IMU_EVT_TILT == p->type)
        {
            if (IMU_PHASE_END == p->data.tilt.phase)
            {
                ends++;
            }
            else
            {
                starts++;
            }
        }
    }
    TEST_ASSERT_EQ(starts, 2);
    TEST_ASSERT_EQ(ends, 2);
    TEST_ASSERT(g_cap.notified >= 1U);           /* critical tilt */
    TEST_ASSERT_EQ(g_cap.last_notified.type, IMU_EVT_TILT);
    TEST_ASSERT(harness_trace_has(IMU_STATE_TILT_EVENT) || (starts == 2U));
    TEST_ASSERT(harness_status().state == IMU_STATE_MONITORING);
}

/* --- 13/14. Disconnection, invalid data, recovery (ST) --------------- */
static void bb16_disconnect_and_recover(void)
{
    imu_status_t st;

    harness_setup();
    bb_drive(3.0, 20.0, 0.2);
    bb_boot();
    harness_run_ms(1000U);
    sim_cfg()->b_bus_stuck = true;           /* SDA held low / unplugged */
    harness_run_ms(200U);
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_FAULT_SAFE);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_I2C);
    TEST_ASSERT(!st.b_sensor_ok);
    TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT, IMU_STATUS_FAULT_RAISED), 1);
    TEST_ASSERT_EQ(g_cap.last_notified.type, IMU_EVT_IMU_FAULT);
    TEST_ASSERT(harness_trace_has(IMU_STATE_SENSOR_FAULT));

    harness_run_ms(2000U);                   /* bus-clear frees the bus */
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_NONE);
    TEST_ASSERT(harness_trace_has(IMU_STATE_RECOVERY));
    TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT, IMU_STATUS_RECOVERED), 1);
    TEST_ASSERT(st.b_calibrated);            /* calibration retained */
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_READY), 1);
    TEST_ASSERT(sim_cfg()->recover_calls >= 1U);
}

static void bb17_persistent_fault(void)
{
    uint32_t t;

    harness_setup();
    bb_boot();
    sim_cfg()->b_bus_stuck = true;
    sim_cfg()->b_stuck_survives_recovery = true;
    for (t = 0U; t < 60000U; t += 1000U)
    {
        harness_run_ms(1000U);
        TEST_ASSERT(harness_status().state != IMU_STATE_MONITORING);
    }
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_FAULT_SAFE);
    TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT,
                                IMU_STATUS_FAULT_PERSISTENT), 1);
    /* Bounded recovery effort: back-off up to 2 s -> < 40 attempts/min. */
    TEST_ASSERT(sim_cfg()->recover_calls < 40U);
    TEST_ASSERT(sim_cfg()->recover_calls >= 10U);
}

static void bb18_invalid_and_frozen_data(void)
{
    imu_status_t st;

    harness_setup();
    bb_boot();
    sim_cfg()->b_frozen = true;
    harness_run_ms(1200U);
    st = harness_status();
    TEST_ASSERT((st.state == IMU_STATE_FAULT_SAFE)
                || (st.state == IMU_STATE_RECOVERY));
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_FROZEN_DATA);
    {
        uint32_t t;

        /* A frozen sensor must never pass recovery verification. */
        for (t = 0U; t < 30000U; t += 500U)
        {
            harness_run_ms(500U);
            TEST_ASSERT(harness_status().state != IMU_STATE_MONITORING);
        }
        TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT, IMU_STATUS_RECOVERED),
                       0);
        TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT,
                                    IMU_STATUS_FAULT_PERSISTENT), 1);
    }
    sim_cfg()->b_frozen = false;            /* sensor comes back to life */
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);

    harness_setup();
    bb_boot();
    sim_cfg()->const_accel.z = 0.0f;           /* dead output: 0 g */
    sim_cfg()->p_fn = NULL;
    harness_run_ms(500U);
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_FAULT_SAFE);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_INVALID_DATA);
}

static void bb19_intermittent_absorbed(void)
{
    harness_setup();
    bb_drive(3.0, 20.0, 0.2);
    scn_add_hump(&g_h.scn, 0.5, 0.025, 0.10);
    bb_boot();
    sim_cfg()->fail_every = 7U;               /* 1 in 7 transfers NACK */
    harness_run_ms(6000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT(harness_status().i2c.retries > 10U);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMU_FAULT), 0);
}

/* --- 15. Event generation and queue overflow (EP) -------------------- */
static void bb20_queue_overflow(void)
{
    imu_status_t st;

    harness_setup();
    bb_drive(3.0, 10.0, 0.25);
    scn_add_hump(&g_h.scn, 0.3, 0.025, 0.10);
    scn_add_hump(&g_h.scn, 1.0, 0.025, 0.10);
    scn_add_impact(&g_h.scn, 9.0, 0.015, 3.0, -1.0f, 0.0f, 0.0f);
    cap_reset(2U);
    g_h.b_no_drain = true;                   /* MC stalled */
    bb_boot();
    harness_run_ms(7000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 2);   /* detection unaffected */
    TEST_ASSERT(st.events.dropped[IMU_EVT_HUMP_COMPLETED]
                + st.events.dropped[IMU_EVT_HUMP_DETECTED] >= 2U);
    TEST_ASSERT_EQ(g_cap.count, 2);
    TEST_ASSERT(g_cap.notified >= 1U);        /* direct path still works */
    TEST_ASSERT(imu_events_pending(&g_h.sys.disp) >= 1U);
    TEST_ASSERT_EQ(st.events.critical_lost, 0);

    cap_drain();                              /* MC catches up */
    g_h.b_no_drain = false;
    harness_run_ms(100U);
    TEST_ASSERT_EQ(imu_events_pending(&g_h.sys.disp), 0);
    TEST_ASSERT(NULL != cap_last_of(IMU_EVT_IMPACT));
}

/* --- 16. Mission restart and peak reset (ST) ------------------------- */
static void bb21_mission_restart(void)
{
    imu_status_t    st;
    imu_telemetry_t tele;

    harness_setup();
    bb_drive(3.0, 30.0, 0.25);
    scn_add_hump(&g_h.scn, 0.3, 0.030, 0.10);
    scn_add_hump(&g_h.scn, 1.5, 0.015, 0.10);
    bb_boot();
    harness_run_ms(4000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 1);
    TEST_ASSERT(st.hump.b_has_max);

    TEST_ASSERT_EQ(imu_terrain_command(&g_h.sys, IMU_CMD_MISSION_START),
                   IMU_OK);
    harness_run_ms(40U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 0);
    TEST_ASSERT(!st.hump.b_has_max);
    imu_terrain_make_telemetry(&st, &tele);
    TEST_ASSERT_EQ(tele.hump_peak_cdeg, -1);
    TEST_ASSERT_EQ(tele.hump_peak_height_mm, -1);
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_MISSION_RESET), 1);

    harness_run_ms(5000U);                     /* smaller hump, new run */
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 1);
    TEST_ASSERT_EQ(st.hump.max.index, 1);
    TEST_ASSERT(st.hump.max.peak_pitch_deg < 8.0f);
}

/* --- Additional integration behaviours ------------------------------- */
static void bb22_hump_during_turn(void)
{
    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    bb_drive(3.0, 1.0, 0.25);
    scn_add_segment(&g_h.scn, 4.0, 3.0, 0.25, 0.25, 35.0);
    bb_drive(7.0, 3.0, 0.25);
    scn_add_hump(&g_h.scn, 0.45, 0.025, 0.10);
    bb_boot();
    harness_run_ms(7000U);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT(cap_count_type(IMU_EVT_TURNING) >= 2U);
}

static void bb23_fast_hump_no_collision(void)
{
    harness_setup();
    g_h.scn.vib_noise_g = 0.03;
    bb_drive(3.0, 10.0, 0.5);
    scn_add_hump(&g_h.scn, 0.5, 0.030, 0.08);
    bb_boot();
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT(harness_status().state != IMU_STATE_FAULT_SAFE);
    {
        imu_event_t const * p = cap_last_of(IMU_EVT_IMPACT);

        TEST_ASSERT((NULL == p) || (IMU_SEV_CRITICAL != p->severity));
    }
}

static void bb24_speed_repeatability(void)
{
    double const    v[] = { 0.12, 0.2, 0.3, 0.45 };
    double          mn  = 1e9;
    double          mx  = -1e9;
    uint32_t        i;
    imu_hump_meas_t m;

    for (i = 0U; i < 4U; i++)
    {
        run_single_hump(0.020, 0.10, v[i], &m);
        TEST_ASSERT_MSG(1U == harness_status().hump.valid_count,
                        "speed %.2f", v[i]);
        mn = fmin(mn, (double)m.peak_pitch_deg);
        mx = fmax(mx, (double)m.peak_pitch_deg);
        printf("      v=%.2f m/s: peak %.2f deg, est %.1f mm\n", v[i],
               (double)m.peak_pitch_deg, (double)m.height_est_mm);
    }
    printf("      truth %.2f deg; spread %.1f %%\n",
           scn_true_peak_pitch_deg(0.020, 0.10, 0.14),
           100.0 * (mx - mn) / mx);
    TEST_ASSERT(((mx - mn) / mx) < 0.30);
}

static void bb25_mag_calibration_spin(void)
{
    imu_status_t st;

    harness_setup();
    g_h.scn.hard_iron.x = 0.15f;              /* motor magnets */
    g_h.scn.hard_iron.y = -0.10f;
    g_h.scn.soft_scale.x = 1.1f;
    g_h.scn.vib_noise_g = 0.02;
    scn_add_segment(&g_h.scn, 4.0, 9.0, 0.0, 0.0, 45.0);  /* 405 deg spin */
    bb_drive(13.0, 4.0, 0.0);
    bb_boot();
    harness_run_ms(100U);
    TEST_ASSERT_EQ(harness_status().heading_quality,
                   IMU_HEADING_UNCALIBRATED);
    harness_run_ms(800U);
    TEST_ASSERT_EQ(imu_terrain_command(&g_h.sys, IMU_CMD_MAG_CAL_BEGIN),
                   IMU_OK);
    harness_run_ms(9200U);
    TEST_ASSERT_EQ(imu_terrain_command(&g_h.sys, IMU_CMD_MAG_CAL_END), IMU_OK);
    harness_run_ms(500U);
    st = harness_status();
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_MAG_CAL_OK), 1);
    TEST_ASSERT(st.b_mag_cal_valid);
    TEST_ASSERT_NEAR(st.mag_cal.hard_iron_gauss.x, 0.15, 0.02);
    TEST_ASSERT_NEAR(st.mag_cal.hard_iron_gauss.y, -0.10, 0.02);
    TEST_ASSERT_EQ(st.heading_quality, IMU_HEADING_GOOD);

    /* Not enough rotation -> rejected, previous state kept. */
    harness_setup();
    scn_add_segment(&g_h.scn, 4.0, 1.0, 0.0, 0.0, 45.0);  /* 45 deg */
    bb_boot();
    harness_run_ms(900U);
    (void)imu_terrain_command(&g_h.sys, IMU_CMD_MAG_CAL_BEGIN);
    harness_run_ms(1500U);
    (void)imu_terrain_command(&g_h.sys, IMU_CMD_MAG_CAL_END);
    harness_run_ms(100U);
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_MAG_CAL_FAILED), 1);
    TEST_ASSERT(!harness_status().b_mag_cal_valid);
}

static void bb26_magnetic_disturbance(void)
{
    uint32_t i;
    uint32_t lim;
    uint32_t starts = 0U;
    bool     b_seen_disturbed = false;
    uint32_t t;

    harness_setup();
    g_h.params.b_mag_cal_valid     = true;    /* stored calibration */
    g_h.params.mag_ref_field_gauss = 0.4176f;
    bb_drive(3.0, 10.0, 0.25);
    g_h.scn.disturb_gauss.x = 0.35f;          /* steel plate / cable */
    g_h.scn.disturb_gauss.y = 0.25f;
    g_h.scn.disturb_t0_s = 5.0;
    g_h.scn.disturb_t1_s = 6.5;
    bb_boot();
    for (t = 0U; t < 6000U; t += 100U)
    {
        harness_run_ms(100U);
        if (IMU_HEADING_DISTURBED == harness_status().heading_quality)
        {
            b_seen_disturbed = true;
        }
    }
    lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    for (i = 0U; i < lim; i++)
    {
        if ((IMU_EVT_TURNING == g_cap.ev[i].type)
            && (IMU_PHASE_START == g_cap.ev[i].data.turn.phase))
        {
            starts++;
        }
    }
    TEST_ASSERT(b_seen_disturbed);
    TEST_ASSERT_EQ(starts, 0);
    TEST_ASSERT_EQ(harness_status().heading_quality, IMU_HEADING_GOOD);
}

static void bb27_reset_and_recalibrate_and_stop(void)
{
    harness_setup();
    bb_drive(3.0, 2.0, 0.3);
    scn_add_impact(&g_h.scn, 4.0, 0.015, 3.0, -1.0f, 0.0f, 0.0f);
    bb_boot();
    harness_run_ms(2500U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_FAULT_SAFE);
    (void)imu_terrain_command(&g_h.sys, IMU_CMD_RESET);
    harness_run_ms(40U);
    TEST_ASSERT(harness_trace_has(IMU_STATE_INITIALISATION));
    TEST_ASSERT(harness_until_monitoring(4000U));

    (void)imu_terrain_command(&g_h.sys, IMU_CMD_RECALIBRATE);
    harness_run_ms(40U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_CALIBRATION);
    TEST_ASSERT(harness_until_monitoring(4000U));

    (void)imu_terrain_command(&g_h.sys, IMU_CMD_STOP);
    harness_run_ms(40U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_STOPPED);
    TEST_ASSERT_EQ(sim_mag_reg(0x02U), 0x03);       /* magnetometer asleep */
    harness_run_ms(1000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_STOPPED);
}

static void bb28_telemetry_json(void)
{
    imu_status_t    st;
    imu_telemetry_t tele;
    char            buf[256];
    char            small[16];
    int32_t         n;

    harness_setup();
    bb_drive(3.0, 10.0, 0.25);
    scn_add_hump(&g_h.scn, 0.3, 0.025, 0.10);
    bb_boot();
    harness_run_ms(3500U);
    st = harness_status();
    imu_terrain_make_telemetry(&st, &tele);
    n = imu_terrain_telemetry_json(&tele, buf, sizeof(buf));
    TEST_ASSERT(n > 0);
    TEST_ASSERT(n < 160);      /* leaves room in Buddy 1's 256 B frame */
    TEST_ASSERT(NULL != strstr(buf, "\"hump_peak\":"));
    {
        char expect[32];

        (void)snprintf(expect, sizeof(expect), "\"hump_peak\":%d,",
                       (int)tele.hump_peak_cdeg);
        TEST_ASSERT(NULL != strstr(buf, expect));
    }
    TEST_ASSERT(tele.hump_peak_cdeg > 300);
    TEST_ASSERT_EQ(imu_terrain_telemetry_json(&tele, small, sizeof(small)),
                   -1);
    TEST_ASSERT_EQ(small[0], '\0');
    printf("      telemetry fragment (%d B): %s\n", (int)n, buf);
}

void suite_blackbox(void)
{
    test_section("BLACK-BOX: system scenarios (public API + events only)");
    RUN_TEST(bb01_init_failure_then_success);
    RUN_TEST(bb02_wrong_device);
    RUN_TEST(bb03_calibration_while_moving);
    RUN_TEST(bb04_stationary);
    RUN_TEST(bb05_mounting);
    RUN_TEST(bb06_straight_cruise);
    RUN_TEST(bb07_accel_decel_with_hint);
    RUN_TEST(bb07b_accel_without_hint);
    RUN_TEST(bb08_turns);
    RUN_TEST(bb09_small_hump);
    RUN_TEST(bb10_large_hump);
    RUN_TEST(bb11_multiple_humps);
    RUN_TEST(bb12_noise_no_false_humps);
    RUN_TEST(bb13_impact_collision);
    RUN_TEST(bb14_repeated_impacts);
    RUN_TEST(bb15_tilt);
    RUN_TEST(bb16_disconnect_and_recover);
    RUN_TEST(bb17_persistent_fault);
    RUN_TEST(bb18_invalid_and_frozen_data);
    RUN_TEST(bb19_intermittent_absorbed);
    RUN_TEST(bb20_queue_overflow);
    RUN_TEST(bb21_mission_restart);
    RUN_TEST(bb22_hump_during_turn);
    RUN_TEST(bb23_fast_hump_no_collision);
    RUN_TEST(bb24_speed_repeatability);
    RUN_TEST(bb25_mag_calibration_spin);
    RUN_TEST(bb26_magnetic_disturbance);
    RUN_TEST(bb27_reset_and_recalibrate_and_stop);
    RUN_TEST(bb28_telemetry_json);
}
