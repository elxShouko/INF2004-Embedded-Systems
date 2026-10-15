/**
 * @file    test_rtos.c
 * @brief   Integration tests of the uT-Kernel wrapper (imu_rtos.c) against a
 *          fake kernel: object attributes, task loop, locking, queue
 *          overflow mapping, direct notification, overruns, start/stop.
 */

#include <string.h>

#include <tk/tkernel.h>

#include "imu/imu_rtos.h"
#include "test_util.h"

static scenario_t g_rscn;

static void rtos_sim_setup(void)
{
    sim_reset();
    fake_tk_reset();
    scn_init(&g_rscn);
    sim_cfg()->p_fn  = scn_stimulus;
    sim_cfg()->p_ctx = &g_rscn;
}

static void r01_not_started(void)
{
    imu_status_t       st;
    imu_motion_class_t m;
    imu_confidence_t   c;

    rtos_sim_setup();
    TEST_ASSERT_EQ(imu_get_status(&st), IMU_ERR_NOT_READY);
    TEST_ASSERT_EQ(imu_get_motion(&m, &c), IMU_ERR_NOT_READY);
    TEST_ASSERT_EQ(imu_mission_start(), IMU_ERR_NOT_READY);
    TEST_ASSERT_EQ(imu_rtos_stop(), IMU_ERR_STATE);
    TEST_ASSERT_EQ(imu_rtos_terrain_event_q(), 0);
    TEST_ASSERT(!imu_rtos_is_running());
    TEST_ASSERT_EQ(imu_rtos_start(NULL), IMU_ERR_PARAM);
}

static void r02_creation_failure(void)
{
    imu_rtos_config_t cfg;

    rtos_sim_setup();
    imu_rtos_default_config(&cfg);
    g_fake_tk.cre_mbf_result = E_NOMEM;
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_ERR_RTOS);
    TEST_ASSERT(!imu_rtos_is_running());
    /* The mutex was created first, with priority inheritance. */
    TEST_ASSERT_EQ(g_fake_tk.n_cre_mtx, 1);
    TEST_ASSERT(0U != (g_fake_tk.cmtx.mtxatr & TA_INHERIT));

    {
        imu_params_t p;

        imu_params_default(&p);
        p.sample_period_ms = 0U;
        cfg.p_params = &p;
        TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_ERR_PARAM);
    }
}

static void r03_objects_and_attributes(void)
{
    imu_rtos_config_t cfg;

    rtos_sim_setup();
    imu_rtos_default_config(&cfg);
    cfg.mc_flag_id         = 11;
    cfg.collision_flag_ptn = 0x01U;
    cfg.fault_flag_ptn     = 0x02U;
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_OK);
    TEST_ASSERT(imu_rtos_is_running());
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_ERR_STATE);

    TEST_ASSERT_EQ(g_fake_tk.n_cre_tsk, 1);
    TEST_ASSERT_EQ(g_fake_tk.ctsk.itskpri, IMU_RTOS_DEFAULT_PRIORITY);
    TEST_ASSERT(0U != (g_fake_tk.ctsk.tskatr & TA_USERBUF));   /* no heap */
    TEST_ASSERT(0U != (g_fake_tk.ctsk.tskatr & TA_HLNG));
    TEST_ASSERT_EQ(g_fake_tk.ctsk.stksz, IMU_RTOS_STACK_BYTES);
    TEST_ASSERT(NULL != g_fake_tk.ctsk.bufptr);
    TEST_ASSERT_EQ(((uintptr_t)g_fake_tk.ctsk.bufptr) % 8U, 0);

    TEST_ASSERT_EQ(g_fake_tk.n_cre_mtx, 0);          /* reused */
    TEST_ASSERT(0U != (g_fake_tk.cmbf.mbfatr & TA_USERBUF));
    TEST_ASSERT_EQ(g_fake_tk.cmbf.maxmsz, (INT)sizeof(imu_event_t));
    TEST_ASSERT(g_fake_tk.cmbf.bufsz
                >= (SZ)(IMU_RTOS_EVENT_Q_DEPTH
                        * (sizeof(imu_event_t) + sizeof(INT))));
    TEST_ASSERT_EQ(g_fake_tk.ccyc.cyctim, IMU_CFG_SAMPLE_PERIOD_MS);
    TEST_ASSERT_EQ(g_fake_tk.n_sta_tsk, 1);
    TEST_ASSERT_EQ(g_fake_tk.n_sta_cyc, 1);
    TEST_ASSERT_EQ(imu_rtos_terrain_event_q(), 9);
}

static void r04_task_loop_and_api(void)
{
    imu_status_t          st;
    imu_telemetry_t       tele;
    imu_motion_class_t    m;
    imu_confidence_t      c;
    float                 p, r, t, h, rate;
    bool                  b, b2;
    imu_heading_quality_t q;
    imu_turn_dir_t        dir;
    imu_hump_phase_t      ph;
    imu_hump_meas_t       meas;
    imu_state_t           s;
    imu_fault_t           f;
    imu_rtos_stats_t      rs;

    /* Continue from r03: run 4 s of the real task body. */
    g_fake_tk.stop_after = 200U;
    fake_tk_run_task();
    TEST_ASSERT(!imu_rtos_is_running());
    TEST_ASSERT_EQ(g_fake_tk.n_stp_cyc, 1);
    TEST_ASSERT_EQ(g_fake_tk.locks, g_fake_tk.unlocks);   /* balanced */

    TEST_ASSERT_EQ(imu_get_status(&st), IMU_OK);
    TEST_ASSERT_EQ(st.state, IMU_STATE_STOPPED);           /* clean stop */
    TEST_ASSERT(st.b_calibrated);
    TEST_ASSERT(st.cycles >= 199U);
    TEST_ASSERT_EQ(imu_get_telemetry(&tele), IMU_OK);
    TEST_ASSERT_EQ(tele.hump_peak_cdeg, -1);
    TEST_ASSERT_EQ(imu_get_motion(&m, &c), IMU_OK);
    TEST_ASSERT_EQ(imu_get_tilt(&p, &r, &t, &b), IMU_OK);
    TEST_ASSERT(fabsf(p) < 1.0f);
    TEST_ASSERT_EQ(imu_get_heading(&h, &q, &rate, &b2, &dir), IMU_OK);
    TEST_ASSERT_EQ(imu_get_hump_state(&ph), IMU_OK);
    TEST_ASSERT_EQ(imu_get_last_hump(&meas), IMU_ERR_NO_DATA);
    TEST_ASSERT_EQ(imu_get_max_hump(&meas), IMU_ERR_NO_DATA);
    TEST_ASSERT_EQ(imu_get_health(&s, &f, &b), IMU_OK);
    TEST_ASSERT_EQ(imu_rtos_get_stats(&rs), IMU_OK);
    TEST_ASSERT(rs.cycles >= 199U);

    /* READY status event went through the message buffer. */
    TEST_ASSERT(g_fake_tk.mbf_sent >= 1U);
    TEST_ASSERT_EQ(g_fake_tk.mbf_last_size, (INT)sizeof(imu_event_t));

    TEST_ASSERT_EQ(imu_get_status(NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_get_tilt(NULL, &r, &t, &b), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_get_last_hump(NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_set_motion_hint(NULL), IMU_ERR_PARAM);
}

static void r05_restart_hump_collision_overflow(void)
{
    imu_rtos_config_t cfg;
    imu_hump_meas_t   meas;
    imu_status_t      st;
    imu_rtos_stats_t  rs;

    rtos_sim_setup();
    scn_add_segment(&g_rscn, 3.0, 30.0, 0.25, 0.25, 0.0);
    scn_add_hump(&g_rscn, 0.3, 0.025, 0.10);
    scn_add_impact(&g_rscn, 6.5, 0.015, 3.0, -1.0f, 0.0f, 0.0f);
    imu_rtos_default_config(&cfg);
    cfg.mc_flag_id         = 11;
    cfg.collision_flag_ptn = 0x01U;
    cfg.fault_flag_ptn     = 0x02U;
    g_fake_tk.mbf_capacity = 3U;       /* tiny queue, never drained */
    g_fake_tk.can_wup_result = 1;      /* every cycle overran once */
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_OK);   /* restart */
    TEST_ASSERT_EQ(g_fake_tk.n_cre_tsk, 0);         /* objects reused */
    TEST_ASSERT_EQ(imu_mission_start(), IMU_OK);
    g_fake_tk.stop_after = 400U;                    /* 8 s */
    fake_tk_run_task();

    TEST_ASSERT_EQ(imu_get_max_hump(&meas), IMU_OK);
    TEST_ASSERT_EQ(meas.index, 1);
    TEST_ASSERT_EQ(imu_get_status(&st), IMU_OK);
    TEST_ASSERT(st.events.dropped[IMU_EVT_HUMP_COMPLETED]
                + st.events.dropped[IMU_EVT_HUMP_PEAK]
                + st.events.dropped[IMU_EVT_MOTION] >= 1U);
    TEST_ASSERT(g_fake_tk.flg_calls >= 1U);           /* direct path */
    TEST_ASSERT(0U != (g_fake_tk.flg_ptn & 0x01U));
    TEST_ASSERT_EQ(g_fake_tk.flg_id, 11);
    TEST_ASSERT_EQ(g_fake_tk.mbf_level, 3);           /* bounded */
    TEST_ASSERT_EQ(imu_rtos_get_stats(&rs), IMU_OK);
    TEST_ASSERT(rs.overruns >= 390U);
}

static void r06_lock_contention_and_timeouts(void)
{
    imu_rtos_config_t  cfg;
    imu_status_t       st;
    imu_motion_class_t m;
    imu_confidence_t   c;
    imu_rtos_stats_t   rs;
    imu_motion_hint_t  h;

    rtos_sim_setup();
    imu_rtos_default_config(&cfg);
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_OK);
    g_fake_tk.b_lock_fail = true;
    TEST_ASSERT_EQ(imu_get_status(&st), IMU_ERR_BUSY);
    TEST_ASSERT_EQ(imu_get_motion(&m, &c), IMU_ERR_BUSY);
    TEST_ASSERT_EQ(imu_clear_fault(), IMU_ERR_BUSY);
    (void)memset(&h, 0, sizeof(h));
    TEST_ASSERT_EQ(imu_set_motion_hint(&h), IMU_ERR_BUSY);
    g_fake_tk.b_slp_timeout = true;           /* cyclic handler missing */
    g_fake_tk.stop_after    = 50U;
    fake_tk_run_task();                       /* must not deadlock */
    g_fake_tk.b_lock_fail = false;
    TEST_ASSERT_EQ(imu_rtos_get_stats(&rs), IMU_OK);
    TEST_ASSERT_EQ(rs.cycles, 0);             /* publication was skipped */
    TEST_ASSERT(rs.lock_timeouts >= 49U);

    /* Commands posted while running are applied at the next cycle. */
    rtos_sim_setup();
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_OK);
    g_fake_tk.b_slp_timeout = true;
    TEST_ASSERT_EQ(imu_request_reset(), IMU_OK);
    TEST_ASSERT_EQ(imu_recalibrate(), IMU_OK);
    TEST_ASSERT_EQ(imu_mag_cal_begin(), IMU_OK);
    TEST_ASSERT_EQ(imu_mag_cal_end(), IMU_OK);
    h.b_valid = true;
    h.speed_mm_s = 100;
    h.timestamp_ms = 0U;
    TEST_ASSERT_EQ(imu_set_motion_hint(&h), IMU_OK);
    g_fake_tk.stop_after = 150U;
    fake_tk_run_task();
    TEST_ASSERT_EQ(imu_rtos_get_stats(&rs), IMU_OK);
    TEST_ASSERT(rs.missed_wakeups >= 140U);
    TEST_ASSERT(rs.max_cycle_us < 20000U);
}

void suite_rtos(void)
{
    test_section("INTEGRATION: uT-Kernel wrapper (fake kernel)");
    test_run("S4-U110", "r01_not_started", r01_not_started);
    test_run("S4-U111", "r02_creation_failure", r02_creation_failure);
    test_run("S4-U112", "r03_objects_and_attributes", r03_objects_and_attributes);
    test_run("S4-U113", "r04_task_loop_and_api", r04_task_loop_and_api);
    test_run("S4-U114", "r05_restart_hump_collision_overflow", r05_restart_hump_collision_overflow);
    test_run("S4-U115", "r06_lock_contention_and_timeouts", r06_lock_contention_and_timeouts);
}
