/** @file vehicle_rt.c @brief Bounded API adapter, no fake production sources. */
#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include "vehicle_rt.h"
#include "gpio_bank.h"
#include "subsystem1/s1_net.h"
#include "subsystem1/s1_map.h"
#include "subsystem3/drv_ir_rtos.h"
#include "subsystem4/imu_runtime.h"
#include "subsystem4/drv_imu.h"
#include "subsystem2/motion_api.h"
#include "vehicle_profile.h"
#include <math.h>
#if !defined(VEHICLE_HOST_TEST)
#include <tk/syslib.h>
#endif

#define VEHICLE_PERIOD_MS (10U) /* Service polling, not motor control timing. */
#define VEHICLE_PRIORITY (16)
#define VEHICLE_STACK (8192U)
static uint64_t g_vehicle_stack[VEHICLE_STACK / sizeof(uint64_t)];
static ID g_vehicle_mutex;
static ID g_vehicle_task;
static uint32_t g_vehicle_started_mask;
static bool g_vehicle_started;
static bool g_vehicle_have_setup;
static vehicle_setup_t g_vehicle_setup;
static vehicle_t g_vehicle_owner;
static vehicle_t g_vehicle_published;
static s5_clearance_t g_vehicle_clearance;
static gpio_bank_t g_vehicle_bank;
#if APP_ROBOT
static vehicle_robot_profile_t g_vehicle_robot;
static uint32_t g_vehicle_robot_session;
static uint32_t g_vehicle_robot_lease_ms;
static bool g_vehicle_robot_have_lease;
static uint64_t g_vehicle_robot_run_us;
static bool g_vehicle_robot_run_started;
static bool g_vehicle_robot_prepared;
static uint8_t g_vehicle_robot_pending;
static uint32_t g_vehicle_robot_pending_ms;
static uint64_t g_vehicle_robot_log_us;
static void vehicle_robot_command (s1_codec_command_t const * p_command,
    vehicle_input_t const * p_input);
static void vehicle_robot_guard (vehicle_input_t * p_input);
static void vehicle_robot_log (vehicle_input_t const * p_input);
#endif

static void vehicle_task (INT code, void * p_context);
static void vehicle_irq (UINT number);
static uint32_t vehicle_read (uint32_t address);
static void vehicle_write (uint32_t address, uint32_t value);
static uint32_t vehicle_core (void);
static bool vehicle_enter (uint32_t * p_saved);
static void vehicle_leave (uint32_t saved);
static bool vehicle_install (void);
static void vehicle_irq_fault (void);
static void vehicle_release (void * p_context, uint32_t mask);
static void vehicle_collect (vehicle_input_t * p_input);
static void vehicle_apply (vehicle_output_t const * p_output);

ER vehicle_runtime_init(void)
{
    if ((g_vehicle_mutex > 0) || (g_vehicle_task > 0))
    {
        return (E_OBJ);
    }
    T_CMTX mutex = {0};
    T_CTSK task = {0};
    gpio_bank_io_t io = {vehicle_read, vehicle_write, vehicle_core,
                         vehicle_enter, vehicle_leave, vehicle_install, vehicle_irq_fault};

    if (!gpio_bank_init(&g_vehicle_bank, &io,
                        ENC_CAPTURE_MASK | (1UL << 6U)))
    {
        return (E_SYS);
    }
    mutex.mtxatr = TA_INHERIT;
    g_vehicle_mutex = tk_cre_mtx(&mutex);
    if (g_vehicle_mutex < E_OK)
    {
        return (g_vehicle_mutex);
    }
    task.task = (FP)vehicle_task; /* Supplied RTOS callback ABI. */
    task.itskpri = VEHICLE_PRIORITY;
    task.stksz = VEHICLE_STACK;
    task.tskatr = TA_HLNG | TA_RNG3 | TA_USERBUF;
    task.bufptr = g_vehicle_stack;
#if TK_SUPPORT_SMP
    task.tskatr |= TA_ASSPRC;
    task.assprc = TP_PRC1;
#endif
    g_vehicle_task = tk_cre_tsk(&task);
    if (g_vehicle_task < E_OK)
    {
        ER error = g_vehicle_task;

        if (E_OK == tk_del_mtx(g_vehicle_mutex))
        {
            g_vehicle_mutex = 0;
        }
        return (error);
    }
    return (E_OK);
}

ER vehicle_runtime_setup(vehicle_setup_t const *p_setup)
{
    vehicle_t candidate = {0};

    if ((NULL == p_setup) || (g_vehicle_task <= 0) ||
        g_vehicle_started || g_vehicle_have_setup)
    {
        return (E_OBJ);
    }
    if (!vehicle_init(&candidate, &p_setup->mission) ||
        !wheel_loop_validate(&p_setup->motion) ||
        (p_setup->mission.forward_mm_s >
         p_setup->motion.geometry.max_wheel_speed_mm_s) ||
        (p_setup->mission.turn_mm_s >
         p_setup->motion.geometry.max_wheel_speed_mm_s))
    {
        return (E_PAR);
    }
    if (p_setup->mission.route.b_calibrated &&
        (!p_setup->b_obstacle_requested ||
         ((uint64_t)(uint32_t)p_setup->mission.route.half_width_um * 2U <
          p_setup->motion.geometry.track_width_um) ||
         ((uint64_t)p_setup->navigation.recovery_timeout_us <
          (uint64_t)p_setup->mission.route.step_timeout_us +
              p_setup->navigation.line_found_us + p_setup->mission.sensor_age_us) ||
         (p_setup->mission.route.speed_mm_s >
          p_setup->motion.geometry.max_wheel_speed_mm_s)))
    {
        return (E_PAR);
    }
    if (p_setup->b_obstacle_requested)
    {
        uint64_t expected_gap_us =
            (uint64_t)p_setup->obstacle.acquisition.sample_period_us +
            p_setup->obstacle.acquisition.servo_settle_us +
            p_setup->obstacle.acquisition.echo_timeout_us + 20000U;

        if (p_setup->mission.sensor_age_us < expected_gap_us)
        {
            return (E_PAR); /* Two 10 ms service quanta; not measured WCET. */
        }
        drv_encoder_port_t port = {&g_vehicle_bank,
                                   gpio_bank_reserve, gpio_bank_enable, vehicle_release};
        ER error = s5_live_configure(&p_setup->obstacle, &port);

        if (E_OK != error)
        {
            return (error);
        }
    }
    g_vehicle_setup = *p_setup;
    g_vehicle_owner = candidate;
    g_vehicle_have_setup = true;
    return (E_OK);
}

ER vehicle_runtime_start(uint32_t started_mask)
{
    if ((g_vehicle_task <= 0) || g_vehicle_started)
    {
        return (E_OBJ);
    }
    g_vehicle_started_mask = started_mask;
    g_vehicle_started = true; /* Set BEFORE another core can run the task. */
#if APP_TEST
    /* Professor self-tests own the S1/S3 queues while they run. Do not add a
     * concurrent command consumer or telemetry producer to that test image.
     */
    return (E_OK);
#else
    ER error = tk_sta_tsk(g_vehicle_task, 0);

    if (E_OK != error)
    {
        g_vehicle_started = false; /* Failed start leaves the task dormant. */
    }
    return (error);
#endif
}

#if APP_ROBOT
ER vehicle_runtime_robot_prepare (vehicle_robot_profile_t const * p_profile)
{
    if ((NULL == p_profile) || g_vehicle_started ||
        g_vehicle_robot_prepared || (g_vehicle_task <= 0) ||
        (p_profile->stage > VEHICLE_STAGE_BARCODE)) { return (E_PAR); }
    g_vehicle_robot = *p_profile;
    /* A boot challenge protects against accidentally replaying old bench
     * packets. ROSC samples/time are not a cryptographic authentication API. */
#if defined(VEHICLE_HOST_TEST)
    g_vehicle_robot_session = 0x12345678U;
#else
    for (uint32_t index = 0U; index < 32U; ++index)
    {
        g_vehicle_robot_session = (g_vehicle_robot_session << 1U) |
            (*(volatile uint32_t const *)0x4006001cUL & 1U);
    }
    g_vehicle_robot_session ^= (uint32_t)s3_clock_us();
    if (0U == g_vehicle_robot_session) { g_vehicle_robot_session = 1U; }
#endif
    drv_encoder_port_t port = {&g_vehicle_bank, gpio_bank_reserve,
        gpio_bank_enable, vehicle_release};
    ER error = E_OK;
    if (VEHICLE_STAGE_BENCH == p_profile->stage)
    {
        if (!motion_api_bench_prepare(&p_profile->bench, &port))
        {
            return (E_SYS);
        }
        if (p_profile->setup.b_obstacle_requested)
        {
            error = s5_live_configure(&p_profile->setup.obstacle, &port);
        }
    }
    else
    {
        /* Keep every mandatory readiness provider. No bypass/height claim. */
        bool b_line = (VEHICLE_STAGE_LINE == p_profile->stage);
        if ((0U == p_profile->run_limit_ms) ||
            (p_profile->run_limit_ms > 30000U) ||
            (p_profile->setup.mission.command_ttl_us > MOTION_BENCH_LEASE_US) ||
            !p_profile->setup.b_obstacle_requested ||
            (b_line != p_profile->setup.mission.b_line_only) ||
            (b_line != p_profile->setup.motion.profile.b_straight_only) ||
            p_profile->setup.mission.route.b_calibrated ||
            (!b_line && !p_profile->setup.navigation.junction_enabled))
        {
            return (E_PAR);
        }
        error = vehicle_runtime_setup(&p_profile->setup);
    }
    g_vehicle_robot_prepared = (E_OK == error);
    return (error);
}

static void vehicle_robot_command (s1_codec_command_t const * p_command,
    vehicle_input_t const * p_input)
{
    if (!g_vehicle_robot_prepared || p_input->b_estop ||
        p_input->b_provider_fault || !p_input->b_link_up ||
        (p_command->session != g_vehicle_robot_session) ||
        ((uint32_t)(p_input->now_us / 1000U) - p_command->rx_timestamp_ms >=
            MOTION_BENCH_LEASE_US / 1000U))
    {
        tm_printf((UB*)"[ROBOT] rejected stale/session/not-ready command=%u\n",
            (unsigned int)p_command->command_id);
        return;
    }
    if (S1_CMD_ROBOT_LEASE == p_command->command_id)
    {
        uint32_t delta = p_command->rx_timestamp_ms - g_vehicle_robot_lease_ms;
        if (!g_vehicle_robot_have_lease ||
            ((0U != delta) && (delta < 0x80000000U)))
        {
            g_vehicle_robot_lease_ms = p_command->rx_timestamp_ms;
            g_vehicle_robot_have_lease = true;
        }
        if (VEHICLE_STAGE_BENCH == g_vehicle_robot.stage)
        {
            if (MOTION_API_ERROR ==
                motion_api_bench_lease(p_command->rx_timestamp_ms))
            {
                (void)motion_api_estop();
            }
        }
        return;
    }
    /* Defer actuation requests until every provider has been checked. */
    if (0U == g_vehicle_robot_pending)
    {
        g_vehicle_robot_pending = (uint8_t)p_command->command_id;
        g_vehicle_robot_pending_ms = p_command->rx_timestamp_ms;
    }
}

static void vehicle_robot_guard (vehicle_input_t * p_input)
{
    uint8_t pending = g_vehicle_robot_pending;
    g_vehicle_robot_pending = 0U;
    if (p_input->b_provider_fault || p_input->b_estop)
    {
        (void)motion_api_estop();
        return;
    }
    bool b_lease = p_input->b_link_up && g_vehicle_robot_have_lease &&
        ((uint32_t)(p_input->now_us / 1000U) - g_vehicle_robot_lease_ms <
            MOTION_BENCH_LEASE_US / 1000U);
    if (!g_vehicle_robot_prepared)
    {
        p_input->remote.command_id = S1_CMD_NONE;
        return;
    }
    if (((uint32_t)(p_input->now_us / 1000U) - g_vehicle_robot_pending_ms >=
            MOTION_BENCH_LEASE_US / 1000U) || !p_input->b_link_up ||
        (S1_CMD_PAUSE == p_input->remote.command_id) ||
        (S1_CMD_EMERGENCY_STOP == p_input->remote.command_id))
    {
        pending = 0U;
    }
    if (S1_CMD_ROBOT_SENSORS == pending)
    {
        ER error = s5_live_start_sensing();
        tm_printf((UB*)"[ROBOT] sensor start=%d (may centre servo GP15)\n", error);
    }
    if (VEHICLE_STAGE_BENCH == g_vehicle_robot.stage)
    {
        if ((pending >= S1_CMD_BENCH_ENCODERS) &&
            (pending <= S1_CMD_BENCH_PULSE))
        {
            motion_bench_request_t request = (motion_bench_request_t)
                (pending - S1_CMD_BENCH_ENCODERS + 1);
            motion_api_result_t result = motion_api_bench_request(request);
            tm_printf((UB*)"[ROBOT] bench command=%u intake=%u; owner result follows\n",
                (unsigned int)request, (unsigned int)result);
        }
        /* PC/broker loss cannot leave an armed bench owner renewing itself. */
        if (!p_input->b_link_up) { (void)motion_api_stop(); }
        p_input->remote.command_id =
            (S1_CMD_PAUSE == p_input->remote.command_id) ? S1_CMD_PAUSE :
            S1_CMD_NONE;
        return;
    }
    if (((S1_CMD_START_RUN == p_input->remote.command_id) ||
         (S1_CMD_RESUME == p_input->remote.command_id)) && !b_lease)
    {
        p_input->remote.command_id = S1_CMD_NONE;
    }
    if (((S1_CMD_START_RUN == p_input->remote.command_id) ||
         (S1_CMD_RESUME == p_input->remote.command_id)) &&
        !g_vehicle_robot_run_started)
    {
        g_vehicle_robot_run_us = p_input->now_us;
        g_vehicle_robot_run_started = true;
    }
    if (g_vehicle_robot_run_started &&
        (!b_lease || (p_input->now_us < g_vehicle_robot_run_us) ||
         (p_input->now_us - g_vehicle_robot_run_us >=
            (uint64_t)g_vehicle_robot.run_limit_ms * 1000U)))
    {
        /* One bounded trial per boot. S2 command TTL also acts if this task
         * stalls; it never depends on this logging or on MQTT queue delivery. */
        p_input->b_estop = true;
        (void)motion_api_estop();
    }
}

static int32_t vehicle_robot_milli (float value)
{
    if (!isfinite(value) || (value > 1000000.0F) || (value < -1000000.0F))
    {
        return (INT32_MIN);
    }
    return ((int32_t)(value * 1000.0F));
}

static void vehicle_robot_log (vehicle_input_t const * p_input)
{
    if ((p_input->now_us >= g_vehicle_robot_log_us) &&
        (p_input->now_us - g_vehicle_robot_log_us < 1000000U)) { return; }
    g_vehicle_robot_log_us = p_input->now_us;
    tm_printf((UB*)"[ROBOT] stage=%u prepared=%u session=0x%08x state=%u blockers=0x%x mqtt=%u faults=0x%x\n",
        (unsigned int)g_vehicle_robot.stage, (unsigned int)g_vehicle_robot_prepared,
        (unsigned int)g_vehicle_robot_session, (unsigned int)g_vehicle_owner.state,
        (unsigned int)g_vehicle_owner.blockers, (unsigned int)p_input->b_link_up,
        (unsigned int)s1_net_fault_get());
    s3_status_t line = {0};
    ER line_error = s3_get_status(&line);
    tm_printf((UB*)"[ROBOT] IR api=%d raw=%u calibrated=%u black=%u valid=%u fault=0x%x sample_us_low=%u\n",
        line_error, (unsigned int)line.raw.levels, (unsigned int)line.calibrated,
        (unsigned int)line.black_mask, (unsigned int)line.line_valid,
        (unsigned int)line.fault, (unsigned int)line.raw.timestamp_us);
    motion_bench_status_t bench = {0};
    motion_api_result_t bench_result = motion_api_bench_status(&bench);
    tm_printf((UB*)"[ROBOT] S2 state=%u fault=%u counts_valid=%u Lhex=%08x:%08x Rhex=%08x:%08x target=%d,%d\n",
        (unsigned int)p_input->motion.state, (unsigned int)p_input->motion.fault_result,
        (unsigned int)p_input->motion.b_encoder_valid,
        (unsigned int)((uint64_t)p_input->motion.encoder_count[0] >> 32U),
        (unsigned int)p_input->motion.encoder_count[0],
        (unsigned int)((uint64_t)p_input->motion.encoder_count[1] >> 32U),
        (unsigned int)p_input->motion.encoder_count[1],
        p_input->motion.target_mm_s[0], p_input->motion.target_mm_s[1]);
    tm_printf((UB*)"[ROBOT] bench_api=%u accepted=1/rejected=2/result=%u enc=%u quiet=%u armed=%u running=%u used=%u fault=%u effort_permille=%d,%d pwm=%u,%u last_pwm=%u,%u invalid=%u,%u\n",
        (unsigned int)bench_result, (unsigned int)bench.result,
        (unsigned int)bench.b_encoders, (unsigned int)bench.b_count_quiet,
        (unsigned int)bench.b_armed,
        (unsigned int)bench.b_running, (unsigned int)bench.b_used,
        (unsigned int)bench.b_fault, (int)bench.effort_permille[0],
        (int)bench.effort_permille[1], (unsigned int)bench.pwm_level[0],
        (unsigned int)bench.pwm_level[1], (unsigned int)bench.last_pulse_pwm[0],
        (unsigned int)bench.last_pulse_pwm[1], (unsigned int)bench.invalid_edges[0],
        (unsigned int)bench.invalid_edges[1]);
    imu_core_sample_t raw = {0};
    uint32_t raw_result = UINT32_MAX;
    ER raw_error = imu_runtime_raw(&raw, &raw_result);
    tm_printf((UB*)"[ROBOT] IMU api=%d transport=%u sample_ms=%u accel_mg=%d,%d,%d mag_mgauss=%d,%d,%d phase=%u height_valid=%u\n",
        raw_error, (unsigned int)raw_result, (unsigned int)raw.time_ms,
        vehicle_robot_milli(raw.accel_g[0]), vehicle_robot_milli(raw.accel_g[1]),
        vehicle_robot_milli(raw.accel_g[2]), vehicle_robot_milli(raw.mag_gauss[0]),
        vehicle_robot_milli(raw.mag_gauss[1]), vehicle_robot_milli(raw.mag_gauss[2]),
        (unsigned int)p_input->imu.phase, (unsigned int)p_input->imu.b_highest_valid);
    s5_live_status_t obstacle = {0};
    ER obstacle_error = s5_live_status(&obstacle);
    tm_printf((UB*)"[ROBOT] S5 api=%d configured=%u forward_cm=%u valid=%u fault=%u acquisition=%u scan=%u width_cm=%u\n",
        obstacle_error, (unsigned int)obstacle.b_configured,
        (unsigned int)obstacle.forward_cm, (unsigned int)obstacle.b_forward_valid,
        (unsigned int)obstacle.b_fault,
        (unsigned int)obstacle.policy.acquisition_status,
        (unsigned int)obstacle.policy.scan_valid,
        (unsigned int)obstacle.policy.profile.estimated_width_cm);
}
#endif /* APP_ROBOT */

ER vehicle_runtime_status(vehicle_t *p_status)
{
    if ((NULL == p_status) || (g_vehicle_mutex <= 0))
    {
        return (E_PAR);
    }
    ER error = tk_loc_mtx(g_vehicle_mutex, TMO_POL);

    if (E_OK == error)
    {
        *p_status = g_vehicle_published;
        error = tk_unl_mtx(g_vehicle_mutex);
    }
    return (error);
}

ER vehicle_runtime_clearance(s5_clearance_t const *p_clearance)
{
    if ((NULL == p_clearance) || !p_clearance->b_verified ||
        !p_clearance->b_original_line_verified ||
        (0U == p_clearance->scan_id_us) || (g_vehicle_mutex <= 0))
    {
        return (E_PAR);
    }
    ER error = tk_loc_mtx(g_vehicle_mutex, TMO_POL);

    if (E_OK == error)
    {
        bool b_allowed = g_vehicle_have_setup && g_vehicle_started &&
                         g_vehicle_setup.mission.route.b_calibrated &&
                         g_vehicle_published.b_obstacle_pending &&
                         !g_vehicle_published.b_route_active &&
                         (0U == g_vehicle_published.active_id) &&
                         (VEHICLE_OBSTACLE == g_vehicle_published.state);

        if (b_allowed)
        {
            g_vehicle_clearance = *p_clearance;
        }
        ER release = tk_unl_mtx(g_vehicle_mutex);

        error = (E_OK != release) ? release : (b_allowed ? E_OK : E_OBJ);
    }
    return (error);
}

static void vehicle_task(INT code, void *p_context)
{
    bool b_motion_configured = false;
    bool b_navigation_configured = false;
    uint32_t navigation_done_sent = 0U;
    uint64_t telemetry_last_us = 0U;
    bool b_telemetry_sent = false;
    vehicle_odometry_t odometry = {0};

    (void)code;
    (void)p_context;
    for (;;)
    {
        vehicle_input_t input = {0};
        vehicle_output_t output = {0};

        vehicle_collect(&input); /* Dedicated safety is read FIRST. */
        if (g_vehicle_have_setup && !input.b_estop && !input.b_provider_fault &&
            (VEHICLE_FAULT != g_vehicle_owner.state))
        {
            if (!b_motion_configured && (0U != (g_vehicle_started_mask & 2U)))
            {
                drv_encoder_port_t port = {&g_vehicle_bank,
                                           gpio_bank_reserve, gpio_bank_enable, vehicle_release};
                motion_api_result_t result = motion_api_configure(
                    &g_vehicle_setup.motion, &port);

                b_motion_configured = (MOTION_API_QUEUED == result);
                input.b_provider_fault = (MOTION_API_ERROR == result) ||
                                         (MOTION_API_INVALID == result);
            }
            if (!b_navigation_configured &&
                (0U != (g_vehicle_started_mask & 4U)))
            {
                s3_input_t request = {0};

                request.kind = S3_INPUT_CONFIG;
                request.request_id = 1U;
                request.value.config = g_vehicle_setup.navigation;
                ER error = s3_submit(&request, TMO_POL);

                b_navigation_configured = (E_OK == error);
                input.b_provider_fault = input.b_provider_fault ||
                                         ((E_OK != error) && (E_TMOUT != error));
            }
        }
#if APP_ROBOT
        vehicle_robot_guard(&input);
        if (VEHICLE_STAGE_BENCH != g_vehicle_robot.stage)
#endif
        {
            (void)vehicle_step(&g_vehicle_owner, &input, &output);
        }
#if APP_ROBOT
        else
        {
            g_vehicle_owner.state = (input.b_estop || input.b_provider_fault ||
                (MOTION_RUN_FAULT == input.motion.state)) ?
                VEHICLE_FAULT : VEHICLE_PAUSED;
            if (S1_CMD_PAUSE == input.remote.command_id)
            {
                output.b_stop = true;
            }
        }
#endif
        vehicle_apply(&output);
        if (g_vehicle_have_setup && (VEHICLE_FAULT != g_vehicle_owner.state) &&
            (0U != (g_vehicle_started_mask & 4U)) &&
            vehicle_odometry(&odometry, &input,
                             g_vehicle_setup.motion.geometry.max_wheel_speed_mm_s))
        {
            s3_input_t request = {0};

            request.kind = S3_INPUT_ODOMETRY;
            request.value.odometry.timestamp_us = odometry.raw_timestamp_us;
            request.value.odometry.forward_distance_um = odometry.distance_um;
            request.value.odometry.tracking_forward =
                odometry.b_tracking_forward;
            ER error = s3_submit(&request, TMO_POL);

            if ((E_OK != error) && (E_TMOUT != error))
            {
                g_vehicle_owner.state = VEHICLE_FAULT;
                (void)motion_api_estop();
            }
        }
        if ((0U != output.navigation_done_id) &&
            (navigation_done_sent != output.navigation_done_id))
        {
            s3_input_t request = {0};

            request.kind = S3_INPUT_MANOEUVRE_DONE;
            request.request_id = output.navigation_done_id;
            request.value.manoeuvre_event_id = output.navigation_done_id;
            ER error = s3_submit(&request, TMO_POL);

            if (E_OK == error)
            {
                navigation_done_sent = output.navigation_done_id;
            }
            else if (E_TMOUT != error)
            {
                g_vehicle_owner.state = VEHICLE_FAULT;
                (void)motion_api_estop();
            }
        }
        /* Default traction trust is false. Never assert physical nonslip
         * traction from wheel agreement alone. No scalar speed substitution.
         */
        s1_codec_frame_t frame;

        if ((!b_telemetry_sent || ((input.now_us >= telemetry_last_us) &&
                                   (input.now_us - telemetry_last_us >= 100000U))) &&
            vehicle_frame(&g_vehicle_owner, &input, &frame) &&
            (0U != (g_vehicle_started_mask & 1U)))
        {
            (void)s1_net_telemetry_send(&frame);
            telemetry_last_us = input.now_us;
            b_telemetry_sent = true;
        }
        if (E_OK == tk_loc_mtx(g_vehicle_mutex, TMO_POL))
        {
            g_vehicle_published = g_vehicle_owner;
            (void)tk_unl_mtx(g_vehicle_mutex);
        }
#if APP_ROBOT
        vehicle_robot_log(&input);
#endif
        if (E_OK != tk_dly_tsk(VEHICLE_PERIOD_MS))
        {
            (void)motion_api_estop();
            g_vehicle_owner.state = VEHICLE_FAULT;
            if (E_OK == tk_loc_mtx(g_vehicle_mutex, TMO_POL))
            {
                g_vehicle_published = g_vehicle_owner;
                (void)tk_unl_mtx(g_vehicle_mutex);
            }
            tk_ext_tsk();
            return;
        }
    }
}

/* Report one shared fault bit (s1_fault.h) for telemetry. Atomic across both
 * cores: a kernel event flag, never read-modify-write of a shared word. */
static void vehicle_fault_bit(uint8_t bit, bool b_on)
{
    if (b_on)
    {
        (void)s1_net_fault_set(bit);
    }
    else
    {
        (void)s1_net_fault_clear(bit);
    }
}

static void vehicle_collect(vehicle_input_t *p_input)
{
    if (0U != (g_vehicle_started_mask & 1U))
    {
        UINT flags = 0U;
        ER error = s1_net_events_wait(S1_NET_FLG_ESTOP, &flags, TMO_POL);

        p_input->b_estop = s1_net_estop_latched() ||
                           ((E_OK == error) && (0U != flags));
        p_input->b_link_up = s1_net_is_connected();
        p_input->b_provider_fault = (E_OK != error) && (E_TMOUT != error);
    }
    if (0U != (g_vehicle_started_mask & 8U))
    {
        ER error = imu_runtime_wait_fault(TMO_POL);

        p_input->b_provider_fault = p_input->b_provider_fault ||
                                    (E_TMOUT != error); /* Includes latched fault or API failure. */
        /* Named shared fault bit (Buddy 4's), telemetry only: it never
         * replaces the provider-fault stop above. */
        vehicle_fault_bit(S1_FAULT_IMU, E_TMOUT != error);
    }
    if (0U != (g_vehicle_started_mask & 16U))
    {
        ER error = s5_live_fault();

        p_input->b_provider_fault = p_input->b_provider_fault ||
                                    (E_TMOUT != error);
    }
    if (p_input->b_estop || p_input->b_provider_fault)
    {
        (void)motion_api_estop(); /* Before ordinary/status/telemetry APIs. */
    }
    if (!motion_api_now(&p_input->now_us))
    {
        p_input->b_provider_fault = true;
        (void)motion_api_estop();
    }
    p_input->raw_now_us = s3_clock_us();
    if (0U != (g_vehicle_started_mask & 2U))
    {
        motion_api_result_t result = motion_api_status(&p_input->motion);

        p_input->b_motion_available = (MOTION_API_QUEUED == result);
        if (p_input->b_motion_available)
        {
            p_input->b_counts_valid = p_input->motion.b_encoder_valid;
            p_input->encoder_count[0] = p_input->motion.encoder_count[0];
            p_input->encoder_count[1] = p_input->motion.encoder_count[1];
        }
        p_input->b_provider_fault = p_input->b_provider_fault ||
                                    (MOTION_API_ERROR == result);
    }
    if (0U != (g_vehicle_started_mask & 1U))
    {
        for (uint32_t n = 0U; n < 8U; ++n)
        {
            s1_codec_command_t command = {0};
            ER error = s1_net_command_receive(&command, TMO_POL);

            if (E_TMOUT == error)
            {
                break;
            }
            if ((E_OK != error) ||
                ((uint32_t)command.command_id > S1_CMD_LAST))
            {
                p_input->b_provider_fault = true;
                break;
            }
#if APP_ROBOT
            if (command.command_id > S1_CMD_RESUME)
            {
                vehicle_robot_command(&command, p_input);
                continue;
            }
#endif
            /* Safety precedence within one bounded drain: ESTOP, PAUSE,
             * then the latest START/RESUME. No second queue consumer.
             */
            if ((S1_CMD_EMERGENCY_STOP == command.command_id) ||
                ((S1_CMD_EMERGENCY_STOP != p_input->remote.command_id) &&
                 ((S1_CMD_PAUSE == command.command_id) ||
                  (S1_CMD_PAUSE != p_input->remote.command_id))))
            {
                p_input->remote = command;
            }
        }
    }
    if (p_input->b_provider_fault ||
        (S1_CMD_EMERGENCY_STOP == p_input->remote.command_id))
    {
        (void)motion_api_estop();
    }
    if (0U != (g_vehicle_started_mask & 4U))
    {
        s3_status_t status = {0};
        ER status_error = s3_get_status(&status); /* Provider uses TMO_POL. */

        p_input->b_line_available = (E_OK == status_error);
        p_input->b_provider_fault = p_input->b_provider_fault ||
                                    ((E_OK != status_error) && (E_TMOUT != status_error));
        if (p_input->b_line_available)
        {
            p_input->line_time_us = status.raw.timestamp_us;
            p_input->line = status.line;
            p_input->line_state = status.state;
            p_input->line_mode = status.mode;
            p_input->black_mask = status.black_mask;
            p_input->b_line_calibrated = status.calibrated;
            p_input->b_line_valid = status.line_valid;
            p_input->pending_navigation_id = status.pending_manoeuvre_event_id;
            p_input->b_provider_fault = p_input->b_provider_fault ||
                                        (S3_FAULT_NONE != status.fault);
            vehicle_fault_bit(S1_FAULT_LINE_SENSOR,
                              0U != ((uint32_t)status.fault & (uint32_t)S3_FAULT_DRIVER));
        }
        if (0U != (g_vehicle_started_mask & 1U))
        {
            /* Buddy 1 mapping gateway: submit at most one update to Buddy 3. */
            s1_map_pump((uint32_t)(p_input->now_us / 1000U));
        }
        for (uint32_t n = 0U; n < 4U; ++n)
        {
            s3_output_t event = {0};
            ER error = s3_receive(&event, TMO_POL);

            if (E_TMOUT == error)
            {
                break;
            }
            if (E_OK != error)
            {
                p_input->b_provider_fault = true;
                break;
            }
            if ((0U != (g_vehicle_started_mask & 1U)) && s1_map_on_reply(&event))
            {
                /* A rejected mapping update is reported to the dashboard by
                 * Buddy 1. It is not a provider fault and must not stop motion. */
                p_input->b_provider_fault = p_input->b_provider_fault ||
                                            (S3_FAULT_NONE != event.fault);
                continue;
            }
            if ((S3_FAULT_NONE != event.fault) ||
                ((S3_OUTPUT_REPLY == event.kind) && !event.accepted))
            {
                p_input->b_provider_fault = true;
            }
            if ((S3_OUTPUT_NAVIGATION == event.kind) &&
                ((NAV_EVENT_FAULT == event.navigation.kind) ||
                 ((NAV_EVENT_FAULT != p_input->navigation.kind) &&
                  !((NAV_EVENT_JUNCTION == p_input->navigation.kind) &&
                    (NAV_COMMAND_NONE != p_input->navigation.command)))))
            {
                /* Match event IDs against the copied pending-owner status. */
                p_input->navigation = event.navigation;
                p_input->navigation_id = event.event_id;
            }
        }
    }
    if (p_input->b_provider_fault)
    {
        (void)motion_api_estop();
    }
    if (0U != (g_vehicle_started_mask & 8U))
    {
        imu_runtime_status_t status = {0};
        ER error = imu_runtime_get_status(&status); /* Provider bound: 10 ms. */

        p_input->b_provider_fault = p_input->b_provider_fault ||
                                    ((E_OK != error) && (E_TMOUT != error));
        p_input->b_imu_available = (E_OK == error) && status.b_enabled;
        p_input->imu = status.sensing;
        /* Consume transitions without letting an event backlog starve safety.
         * Highest hump remains provider-owned in the coherent snapshot.
         */
        for (uint32_t count = 0U; count < 4U; ++count)
        {
            imu_core_status_t event = {0};
            ER event_error = imu_runtime_get_event(&event);

            if (E_TMOUT == event_error)
            {
                break;
            }
            if ((E_OK != event_error) || (IMU_FAULT == event.phase) ||
                (0U != (event.events & (IMU_EVENT_FAULT | IMU_EVENT_IMPACT))))
            {
                p_input->b_provider_fault = true;
                (void)motion_api_estop();
                break;
            }
        }
    }
    /* Providers may run on the other core or block for their documented bound.
     * Refresh both clocks AFTER reading their snapshots, so a newer sample is
     * not falsely treated as a future timestamp using this cycle's early clock.
     */
    if (!motion_api_now(&p_input->now_us))
    {
        p_input->b_provider_fault = true;
        (void)motion_api_estop();
    }
    p_input->raw_now_us = s3_clock_us();
    if (0U != (g_vehicle_started_mask & 16U))
    {
        s5_live_status_t status = {0};
        ER error = s5_live_status(&status);

        p_input->b_obstacle_available = (E_OK == error) && status.b_configured;
        p_input->b_forward_valid = (E_OK == error) && status.b_forward_valid;
        p_input->forward_cm = status.forward_cm;
        p_input->obstacle_time_us = status.forward_timestamp_us;
        p_input->obstacle_started_us = status.forward_started_us;
        p_input->obstacle = status.policy.profile;
        p_input->scan_id_us = status.scan_id_us;
        p_input->scan_completed_us = status.scan_completed_us;
        p_input->b_scan_held = status.policy.scan_valid &&
                               (S5_RUNTIME_HOLD == status.policy.state);
        p_input->b_provider_fault = p_input->b_provider_fault ||
                                    ((E_OK != error) && (E_TMOUT != error)) || status.b_fault;
    }
    ER certificate = tk_loc_mtx(g_vehicle_mutex, TMO_POL);

    if (E_OK == certificate)
    {
        p_input->clearance = g_vehicle_clearance;
        if (E_OK != tk_unl_mtx(g_vehicle_mutex))
        {
            p_input->b_provider_fault = true;
        }
    }
    else if (E_TMOUT != certificate)
    {
        p_input->b_provider_fault = true;
    }
    /* Refresh after the last provider too; source clocks are never replaced. */
    if (!motion_api_now(&p_input->now_us))
    {
        p_input->b_provider_fault = true;
        (void)motion_api_estop();
    }
    p_input->raw_now_us = s3_clock_us();
}

static void vehicle_apply(vehicle_output_t const *p_output)
{
    motion_api_result_t result = MOTION_API_QUEUED;

    if (p_output->b_estop)
    {
        result = motion_api_estop();
        if (0U != (g_vehicle_started_mask & 16U))
        {
            (void)s5_live_cancel();
        }
    }
    else if (0U != p_output->obstacle_recovered_us)
    {
        ER error = s5_live_recovered(p_output->obstacle_recovered_us);

        if ((E_OK != error) && (E_TMOUT != error))
        {
            (void)motion_api_estop();
            g_vehicle_owner.state = VEHICLE_FAULT;
        }
    }
    else if (p_output->b_stop)
    {
        result = motion_api_stop();
    }
    else if (0U != p_output->acknowledge_id)
    {
        result = motion_api_ack(p_output->acknowledge_id);
    }
    else if (p_output->b_enable)
    {
        result = motion_api_enable();
    }
    else if (p_output->b_command)
    {
        result = motion_api_submit(&p_output->command);
    }
    if ((MOTION_API_ERROR == result) || (MOTION_API_INVALID == result) ||
        (MOTION_API_UNAVAILABLE == result))
    {
        g_vehicle_owner.state = VEHICLE_FAULT;
        (void)motion_api_estop();
    }
    if (p_output->b_mode)
    {
        s3_input_t request = {0};

        request.kind = S3_INPUT_MODE;
        request.value.mode = p_output->mode;
        ER error = s3_submit(&request, TMO_POL);

        if ((E_OK != error) && (E_TMOUT != error))
        {
            g_vehicle_owner.state = VEHICLE_FAULT;
            (void)motion_api_estop();
        }
    }
}

static void vehicle_release(void *p_context, uint32_t mask)
{
    if (!gpio_bank_release(p_context, mask))
    {
        (void)motion_api_estop();
    }
}

static void vehicle_irq(UINT number)
{
    (void)number;
    gpio_bank_irq(&g_vehicle_bank);
}

static void vehicle_irq_fault(void)
{
#if !defined(VEHICLE_HOST_TEST)
    DisableInt(13U); /* Prevent an unowned/level source starving the stop task. */
#endif
    (void)motion_api_estop();
}

#if defined(VEHICLE_HOST_TEST)
static uint32_t vehicle_read(uint32_t address)
{
    return (test_port_read(address));
}
static void vehicle_write(uint32_t address, uint32_t value)
{
    test_port_write(address, value);
}
static uint32_t vehicle_core(void)
{
    return (test_port_core());
}
static bool vehicle_enter(uint32_t *p_saved)
{
    return (test_port_enter(p_saved));
}
static void vehicle_leave(uint32_t saved)
{
    test_port_leave(saved);
}
static bool vehicle_install(void)
{
    return (test_port_install(vehicle_irq));
}
#else
static uint32_t vehicle_read(uint32_t address)
{
    return (in_w(address));
}
static void vehicle_write(uint32_t address, uint32_t value)
{
    out_w(address, value);
}
static uint32_t vehicle_core(void)
{
    return (in_w(0xD0000000UL));
}
static bool vehicle_enter(uint32_t *p_saved)
{
    DI(*p_saved);
    return (true);
}
static void vehicle_leave(uint32_t saved)
{
    EI(saved);
}
static bool vehicle_install(void)
{
    extern void Default_Handler(void); /* Actual supplied vector default. */
    uint32_t vector = in_w(0xE000ED08UL);

    if ((vector < 0x20000000UL) || (vector >= 0x20042000UL) ||
        (0U != (in_w(0xE000E100UL) & (1UL << 13U))))
    {
        return (false);
    }
    uint32_t current = in_w(vector + (16U + 13U) * 4U);

    if ((current & ~1UL) != ((uint32_t)(uintptr_t)Default_Handler & ~1UL))
    {
        return (false); /* Never replace an existing, even disabled, handler. */
    }
    for (uint32_t group = 0U; group < 4U; ++group)
    {
        if (0U != (in_w(GPIO_BANK_INTE0 + group * 4U) |
                   in_w(GPIO_BANK_INTE1 + group * 4U) |
                   in_w(GPIO_BANK_INTF0 + group * 4U) |
                   in_w(GPIO_BANK_INTF1 + group * 4U)))
        {
            return (false);
        }
    }
    T_DINT handler = {TA_HLNG, (FP)vehicle_irq};

    if (E_OK != tk_def_int(13U, &handler))
    {
        return (false);
    }
    EnableInt(13U, 2); /* Edge sources stay disabled until explicit enable. */
    return (true);
}
#endif
/*** end of file ***/
