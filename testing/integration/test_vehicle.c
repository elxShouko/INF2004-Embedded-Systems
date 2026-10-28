/** @file test_vehicle.c @brief Real shared policy/dispatcher with boundary fakes. */
#include "test_ports.h"
#include "vehicle_rt.h"
#include "gpio_bank.h"
#include "subsystem1/s1_net.h"
#include "subsystem1/s1_map.h"
#include "subsystem1/s1_sim.h"
#include "subsystem2/motion_api.h"
#include "subsystem3/drv_ir_rtos.h"
#include "subsystem4/imu_runtime.h"
/* These C-only task fakes perform nonlocal exits, not Windows exception
 * unwinding. Use MinGW's documented non-SEH setjmp mode; production ARM
 * code never includes this host harness or macro.
 */
#if defined(__MINGW32__)
#define __USE_MINGW_SETJMP_NON_SEH 1
#endif
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t g_case;
static uint32_t g_created;
static ID g_mutex;
static ID g_task_id;
static T_CTSK g_task_info;
static uint32_t g_inits;
static uint32_t g_starts;
static uint32_t g_estops;
static uint32_t g_enables;
static uint32_t g_submits;
static uint32_t g_deletes;
static uint32_t g_sim_disabled;
static uint32_t g_s3_reads;
static uint32_t g_start_attempts;
static uint32_t g_imu_events;
static uint32_t g_telemetry;
static uint32_t g_delays;
static uint32_t g_self_tests;
static uint32_t g_s5_configures;
static uint32_t g_mapping_calls;
static bool g_exit;
static bool g_late_sample;
static bool g_ok = true;
static jmp_buf g_end;
static uint32_t g_regs[1024];
static uint32_t g_levels;
static uint32_t g_core = 1U;
static uint32_t g_reads;
static uint32_t g_faults;
static uint32_t g_callbacks;
static bool g_locked;
static bool g_install_ok = true;
static void (*g_irq)(UINT);

static bool test_case (void);
static bool test_route (void);
static vehicle_input_t test_input (void);
static vehicle_config_t test_config (void);
static wheel_loop_config_t test_motion_config (void);
static bool test_ready (motion_run_t * p_run);
static bool test_prepare (void * p_context, uint16_t period, uint32_t clock);
static bool test_apply (void * p_context, drv_motor_wheel_t wheel,
    uint16_t a, uint16_t b);
static bool test_stop (void * p_context);
static void test_sink (void * p_context, uint32_t events, uint32_t levels);
static void test_fault (void);
static bool test_bank (void);
static bool test_bank_install (void);
static void test_run_owner (void);
static void test_task (INT code, void * p_context);

ID tk_cre_mtx (T_CMTX const * p_info)
{
    g_ok = (TA_INHERIT == p_info->mtxatr) && (0 == g_mutex) && g_ok;
    g_mutex = (ID)++g_created;
    return (g_mutex);
}
ID tk_cre_tsk (T_CTSK const * p_info)
{
    g_ok = (NULL != p_info->task) && (0 == g_task_id) && g_ok;
    g_task_info = *p_info;
    if (31U == g_case) { return (E_SYS); }
    g_task_id = (ID)++g_created;
    return (g_task_id);
}
ER tk_sta_tsk (ID id, INT code)
{
    (void)code;
    g_ok = (id == g_task_id) && g_ok;
    ++g_start_attempts;
    return (((5U == g_case) ||
        ((52U == g_case) && (1U == g_start_attempts))) ? E_SYS : E_OK);
}
ER tk_del_tsk (ID id)
{
    g_ok = (id == g_task_id) && (id > 0) && g_ok;
    g_task_id = 0;
    ++g_deletes;
    return (E_OK);
}
ER tk_del_mtx (ID id)
{
    g_ok = (id == g_mutex) && (id > 0) && g_ok;
    g_mutex = 0;
    ++g_deletes;
    return (E_OK);
}
ER tk_loc_mtx (ID id, TMO timeout)
{
    g_ok = (id == g_mutex) && (TMO_POL == timeout) && !g_locked && g_ok;
    g_locked = true;
    return (E_OK);
}
ER tk_unl_mtx (ID id)
{
    g_ok = (id == g_mutex) && g_locked && g_ok;
    g_locked = false;
    return (E_OK);
}
ER tk_dly_tsk (TMO delay)
{
    g_ok = (10 == delay) && g_ok;
    if (9U == g_case) { return (E_SYS); }
    ++g_delays;
    if (((62U == g_case) && (g_delays < 22U)) ||
        ((83U == g_case) && (g_delays < 2U))) { return (E_OK); }
    longjmp(g_end, 1); /* End host observation, not production task behaviour. */
}
void tk_ext_tsk (void) { g_exit = true; longjmp(g_end, 1); }
ER tk_slp_tsk (TMO delay)
{
    g_ok = (TMO_FEVR == delay) && g_ok;
    longjmp(g_end, 1);
}
ER tk_ref_ver (T_RVER * p_version)
{
    *p_version = (T_RVER){0};
    return (E_OK);
}
INT tk_get_prc (void) { return (1); }
void tm_printf (UB * p_format, ...) { (void)p_format; }
ER subsystem1_init (void) { ++g_inits; return (E_OK); }
ER subsystem2_init (void)
{ ++g_inits; return ((4U == g_case) ? E_SYS : E_OK); }
ER subsystem3_init (void) { ++g_inits; return (E_OK); }
ER subsystem4_init (void) { ++g_inits; return (E_OK); }
ER subsystem5_init (void) { ++g_inits; return (E_OK); }
ER subsystem1_start (void) { g_ok = (5U == g_inits) && g_ok; ++g_starts; return (E_OK); }
ER subsystem2_start (void) { g_ok = (5U == g_inits) && g_ok; ++g_starts; return (E_OK); }
ER subsystem3_start (void) { g_ok = (5U == g_inits) && g_ok; ++g_starts; return ((45U == g_case) ? E_SYS : E_OK); }
ER subsystem4_start (void) { g_ok = (5U == g_inits) && g_ok; ++g_starts; return (E_OK); }
ER subsystem5_start (void) { g_ok = (5U == g_inits) && g_ok; ++g_starts; return (E_NOSPT); }
void subsystem1_test (void) { ++g_self_tests; }
void subsystem2_test (void) { ++g_self_tests; }
void subsystem3_test (void) { ++g_self_tests; }
void subsystem4_test (void) { ++g_self_tests; }
void subsystem5_test (void) { ++g_self_tests; }
void s1_sim_set_enabled (bool enabled)
{
    g_ok = !enabled && (0U == g_starts) && g_ok;
    ++g_sim_disabled;
}
ER s1_net_events_wait (UINT pattern, UINT * p_flags, TMO timeout)
{
    g_ok = (S1_NET_FLG_ESTOP == pattern) && (TMO_POL == timeout) && g_ok;
    *p_flags = (7U == g_case) ? S1_NET_FLG_ESTOP : 0U;
    return ((7U == g_case) ? E_OK : E_TMOUT);
}
bool s1_net_estop_latched (void)
{ return (48U == g_case); }
bool s1_net_is_connected (void) { return (true); }
ER s1_net_command_receive (s1_codec_command_t * p_cmd, TMO timeout)
{
    static bool b_mapping_sent;
    g_ok = (TMO_POL == timeout) && g_ok;
    if ((85U == g_case) && !b_mapping_sent)
    {
        b_mapping_sent = true;
        *p_cmd = (s1_codec_command_t){0};
        p_cmd->command_id = S1_CMD_START_RUN;
        p_cmd->rx_timestamp_ms = 20U;
        return (E_OK);
    }
    return (E_TMOUT);
}
static s1_codec_map_result_t g_map_result;
static uint8_t g_fault_bits;
ER s1_net_fault_set (uint8_t bits)
{ g_fault_bits |= bits; return (E_OK); }
ER s1_net_fault_clear (uint8_t bits)
{ g_fault_bits &= (uint8_t)~bits; return (E_OK); }
ER s1_net_map_receive (s1_net_map_request_t * p_request, TMO timeout)
{
    static bool b_sent;
    g_ok = (TMO_POL == timeout) && g_ok;
    if (((83U == g_case) || (84U == g_case)) && !b_sent)
    {
        b_sent = true;
        *p_request = (s1_net_map_request_t){0};
        p_request->seq = 1U;
        p_request->rx_ms = 20U;
        return (nav_update_parse(&p_request->update,
            "MAP W=FORWARD X=UTURN Y=LEFT Z=RIGHT") ? E_OK : E_SYS);
    }
    return (E_TMOUT);
}
ER s1_net_map_report (uint32_t seq, s1_codec_map_result_t result,
                      uint32_t revision)
{
    (void)revision;
    g_ok = (1U == seq) && g_ok;
    g_map_result = result;
    return (E_OK);
}
ER s1_net_telemetry_send (s1_codec_frame_t const * p_frame)
{
    ++g_telemetry;
    g_ok = (NULL != p_frame) && (2U == p_frame->schema_version) && g_ok;
    if (61U == g_case)
    {
        char json[S1_CODEC_TELEMETRY_JSON_CAPACITY];

        g_ok = (INT64_MIN == p_frame->encoder_count_left) &&
            (INT64_MAX == p_frame->encoder_count_right) &&
            (0U != (p_frame->validity & S1_VALID_ENCODER)) &&
            (0U == (p_frame->validity & S1_VALID_SPEED)) &&
            (20000U == p_frame->encoder_timestamp_us) &&
            (s1_codec_encode_telemetry(p_frame,0U,json,sizeof(json)) > 0U) &&
            (NULL != strstr(json,"-9223372036854775808")) && g_ok;
    }
    return (E_OK);
}
ER s5_live_recovered (uint64_t scan_id_us)
{ return ((scan_id_us > 0U) ? E_OK : E_PAR); }
ER s5_live_fault (void) { return ((57U == g_case) ? E_OK : E_TMOUT); }
ER s5_live_status (s5_live_status_t * p_status)
{
    *p_status = (s5_live_status_t){0};
    if (58U == g_case) { return (E_TMOUT); }
    if (((g_case >= 59U) && (g_case <= 61U)) || (85U == g_case))
    {
        p_status->b_configured = true;
        p_status->b_forward_valid = true;
        p_status->forward_cm = 100U;
        p_status->forward_timestamp_us = (59U == g_case) ? 1U : 1020000U;
    }
    return (E_OK);
}
ER s5_live_cancel (void) { return (E_OK); }
ER s5_live_configure (s5_live_config_t const * p_config,
    drv_encoder_port_t const * p_irq)
{
    (void)p_config;
    ++g_s5_configures;
    g_ok = (NULL != p_irq) && (NULL != p_irq->p_reserve) &&
        (NULL != p_irq->p_enable) && (NULL != p_irq->p_release) && g_ok;
    return (E_OK);
}
motion_api_result_t motion_api_estop (void)
{ ++g_estops; return (MOTION_API_QUEUED); }
bool motion_api_now (uint64_t * p_now)
{ *p_now = g_late_sample ? 40000U : 20000U;
  if (62U == g_case) { *p_now += (uint64_t)g_delays * 10000U; }
  return (8U != g_case); }
motion_api_result_t motion_api_status (motion_run_status_t * p_status)
{
    *p_status = (motion_run_status_t){0};
    if (85U == g_case) { *p_status = test_input().motion; }
    if (44U == g_case)
    {
        *p_status = test_input().motion;
        p_status->measurement_timestamp_us = 30000U;
    }
    if ((60U == g_case) || (61U == g_case))
    {
        p_status->state = MOTION_RUN_CONFIGURED;
        p_status->b_encoder_valid = true;
        p_status->encoder_count[0] = INT64_MIN;
        p_status->encoder_count[1] = INT64_MAX;
        p_status->encoder_timestamp_us = 20000U;
        p_status->encoder_generation = 1U;
        p_status->measurement_max_age_us = 50000U;
    }
    return (MOTION_API_QUEUED);
}
motion_api_result_t motion_api_enable (void)
{ ++g_enables; return (MOTION_API_QUEUED); }
motion_api_result_t motion_api_configure (wheel_loop_config_t const * p_config,
    drv_encoder_port_t const * p_port)
{
    /* Configuration intake is not a movement command. */
    (void)p_config; (void)p_port; return (MOTION_API_QUEUED);
}
motion_api_result_t motion_api_submit (move_plan_command_t const * p_cmd)
{
    if (85U == g_case) { g_ok = (MOVE_PLAN_LEFT == p_cmd->kind) && g_ok; }
    ++g_submits;
    return (MOTION_API_QUEUED);
}
motion_api_result_t motion_api_stop (void) { return (MOTION_API_QUEUED); }
motion_api_result_t motion_api_ack (uint32_t id)
{ (void)id; return (MOTION_API_QUEUED); }
uint64_t s3_clock_us (void)
{ return ((g_late_sample ? 1040000U : 1020000U) +
    ((62U == g_case) ? (uint64_t)g_delays * 10000U : 0U)); }
ER s3_get_status (s3_status_t * p_status)
{
    ++g_s3_reads;
    if ((57U == g_case) && (0U == g_estops)) { g_ok = false; }
    if (50U == g_case) { return (E_TMOUT); }
    *p_status = (s3_status_t){0};
    if ((49U == g_case) || (51U == g_case) || (53U == g_case) || (56U == g_case) || (85U == g_case) || ((g_case >= 58U) && (g_case <= 61U)))
    {
        p_status->calibrated = true;
        p_status->line_valid = true;
        p_status->raw.timestamp_us = (53U == g_case) ? 1U : 1020000U;
        p_status->line = LINE_OBSERVATION_CENTRED;
        p_status->black_mask = 3U;
        p_status->state = LINE_STATE_TRACKING;
        p_status->mode = LINE_MODE_TRACKING;
        if (85U == g_case)
        {
            p_status->state = LINE_STATE_AWAITING_MANOEUVRE;
            p_status->pending_manoeuvre_event_id = 11U;
        }
        if (51U == g_case) { p_status->fault = S3_FAULT_DRIVER; }
    }
    return (E_OK);
}
ER s3_submit (s3_input_t const * p_request, TMO timeout)
{
    g_ok = (TMO_POL == timeout) && g_ok;
    if (S3_INPUT_MAPPING == p_request->kind)
    {
        ++g_mapping_calls;
        g_ok = ((S1_MAP_REQUEST_ID_FLAG | 1U) == p_request->request_id) &&
            (NAV_COMMAND_LEFT == nav_action_get(p_request->value.mapping.actions, 24U)) &&
            (NAV_COMMAND_RIGHT == nav_action_get(p_request->value.mapping.actions, 25U)) &&
            (NAV_COMMAND_STRAIGHT == nav_action_get(p_request->value.mapping.actions, 22U)) &&
            (NAV_COMMAND_U_TURN == nav_action_get(p_request->value.mapping.actions, 23U)) && g_ok;
        if ((83U == g_case) && (1U == g_mapping_calls)) { return (E_TMOUT); }
    }
    return (E_OK);
}
ER s3_receive (s3_output_t * p_event, TMO timeout)
{
    static bool b_sent;
    static uint32_t observations;

    g_ok = (TMO_POL == timeout) && g_ok;
    if ((85U == g_case) && (observations < 2U))
    {
        *p_event = (s3_output_t){0};
        p_event->kind = S3_OUTPUT_NAVIGATION;
        p_event->event_id = 11U + observations;
        p_event->navigation.timestamp_us = 1020000U;
        p_event->navigation.kind = (0U == observations) ?
            NAV_EVENT_JUNCTION : NAV_EVENT_BARCODE_VALID;
        p_event->navigation.command = NAV_COMMAND_LEFT;
        ++observations;
        return (E_OK);
    }
    if ((84U == g_case) && !b_sent)
    {
        b_sent = true;
        *p_event = (s3_output_t){0};
        p_event->kind = S3_OUTPUT_REPLY;
        p_event->request_id = S1_MAP_REQUEST_ID_FLAG | 1U;
        return (E_OK);
    }
    if ((10U == g_case) && !b_sent)
    {
        b_sent = true;
        *p_event = (s3_output_t){0};
        p_event->kind = S3_OUTPUT_REPLY;
        return (E_OK);
    }
    return (E_TMOUT);
}
ER imu_runtime_wait_fault (TMO timeout)
{ g_ok = (TMO_POL == timeout) && g_ok; return (E_TMOUT); }
ER imu_runtime_get_event (imu_core_status_t * p_event)
{
    ++g_imu_events;
    if (55U == g_case) { return (E_SYS); }
    *p_event = (imu_core_status_t){0};
    if (54U == g_case)
    {
        p_event->phase = IMU_FAULT;
        p_event->events = IMU_EVENT_IMPACT;
        return (E_OK);
    }
    return ((56U == g_case) ? E_OK : E_TMOUT);
}
ER imu_runtime_get_status (imu_runtime_status_t * p_status)
{
    if ((7U == g_case) || (8U == g_case) || (10U == g_case))
    {
        g_ok = (g_estops > 0U) && g_ok; /* Stop posted before bounded getter. */
    }
    *p_status = (imu_runtime_status_t){0};
    if ((60U == g_case) || (61U == g_case) || (85U == g_case))
    {
        p_status->b_enabled = true;
        p_status->sensing.phase = IMU_MONITORING;
        p_status->sensing.time_ms = 1020U;
    }
    if (44U == g_case)
    {
        g_late_sample = true;
        p_status->b_enabled = true;
        p_status->sensing.phase = IMU_MONITORING;
        p_status->sensing.time_ms = 1040U;
    }
    return (E_OK);
}
uint32_t test_port_read (uint32_t address)
{
    if (GPIO_BANK_INPUT == address)
    {
        ++g_reads;
        if (38U == g_case) { g_regs[0xFCU / 4U] |= 4UL << 16U; }
        return (g_levels);
    }
    if ((address >= GPIO_BANK_INTS1) && (address < GPIO_BANK_INTS1 + 16U))
    {
        uint32_t group = (address - GPIO_BANK_INTS1) / 4U;

        return (g_regs[(GPIO_BANK_INTR - GPIO_BANK_BASE) / 4U + group] &
            g_regs[(GPIO_BANK_INTE1 - GPIO_BANK_BASE) / 4U + group]);
    }
    return (g_regs[(address - GPIO_BANK_BASE) / 4U]);
}
void test_port_write (uint32_t address, uint32_t value)
{
    uint32_t offset = address - GPIO_BANK_BASE;
    uint32_t alias = offset & 0x3000U;
    uint32_t index = (offset & 0xFFFU) / 4U;

    if (0x2000U == alias) { g_regs[index] |= value; }
    else if ((0x3000U == alias) ||
        ((offset >= 0xF0U) && (offset < 0x100U))) { g_regs[index] &= ~value; }
    else { g_regs[index] = value; }
}
uint32_t test_port_core (void) { return (g_core); }
bool test_port_enter (uint32_t * p_saved)
{
    *p_saved = 0U;
    if (g_locked) { return (false); }
    g_locked = true;
    return (true);
}
void test_port_leave (uint32_t saved) { (void)saved; g_locked = false; }
bool test_port_install (void (*p_handler)(UINT))
{ g_irq = p_handler; return (g_install_ok); }

int main (int argc, char * argv[])
{
    if (2 != argc) { return (2); }
    g_case = (uint32_t)strtoul(argv[1], NULL, 10);
    bool b_pass = test_case() && g_ok;

    (void)printf("[TEST] INT-%02u %s shared production integration\n",
        g_case, b_pass ? "PASS" : "FAIL");
    return (b_pass ? 0 : 1);
}

static vehicle_config_t test_config (void)
{
    return ((vehicle_config_t){100000U, 300000U, 2000000U, 100U,
        100, 20, 80, 100000, 30U, true, true, false, {0}, 0U, false});
}
static vehicle_input_t test_input (void)
{
    vehicle_input_t input = {0};

    input.now_us = 20000U;
    input.raw_now_us = 1020000U;
    input.motion.state = MOTION_RUN_READY;
    input.motion.b_measurement_valid = true;
    input.motion.b_standstill = true;
    input.motion.measurement_timestamp_us = input.now_us;
    input.motion.measurement_max_age_us = 100000U;
    input.b_motion_available = true;
    input.b_line_available = true;
    input.b_line_calibrated = true;
    input.b_line_valid = true;
    input.line_time_us = input.raw_now_us;
    input.line = LINE_OBSERVATION_CENTRED;
    input.black_mask = 3U;
    input.line_mode = LINE_MODE_TRACKING;
    input.line_state = LINE_STATE_TRACKING;
    input.b_imu_available = true;
    input.imu.phase = IMU_MONITORING;
    input.imu.time_ms = (uint32_t)(input.raw_now_us / 1000U);
    input.b_obstacle_available = true;
    input.obstacle.valid = true;
    input.b_forward_valid = true;
    input.forward_cm = 100U;
    input.obstacle.closest_distance_cm = 100U;
    input.obstacle_time_us = input.raw_now_us;
    return (input);
}
static wheel_loop_config_t test_motion_config (void)
{
    wheel_loop_config_t cfg = {0};

    cfg.geometry.left = (motion_config_wheel_t){1000U, 100000U,
        MOTION_TYPES_POLARITY_NORMAL, MOTION_TYPES_POLARITY_NORMAL};
    cfg.geometry.right = cfg.geometry.left;
    cfg.geometry.track_width_um = 150000U;
    cfg.geometry.max_wheel_speed_mm_s = 500;
    cfg.geometry.calibration = MOTION_CONFIG_CALIBRATION_CONFIRMED;
    cfg.motor = (drv_motor_config_t){125000000U, 20000U, 100U, 1.0F,
        {MOTION_TYPES_POLARITY_NORMAL, MOTION_TYPES_POLARITY_NORMAL},
        DRV_MOTOR_BRAKE, MOTION_CONFIG_CALIBRATION_CONFIRMED, true};
    cfg.pid[0] = (motion_pid_config_t){0.01F, 0.0F, 0.0F, -1.0F, 1.0F,
        -0.5F, 0.5F, 100000U, MOTION_CONFIG_CALIBRATION_CONFIRMED};
    cfg.pid[1] = cfg.pid[0];
    cfg.profile = (move_plan_config_t){1000U, 1000U, 1000U,
        {1000000U, 1000000U}, 1.0F, 100};
    cfg.period_us = 10000U;
    cfg.sample_limit_us = 100000U;
    cfg.stop_dwell_us = 10000U;
    cfg.stop_timeout_us = 1000000U;
    cfg.progress_timeout_us = 2000000U;
    cfg.overspeed_mm_s = 1000;
    return (cfg);
}
static bool test_prepare (void * p_context, uint16_t period, uint32_t clock)
{ (void)p_context; return ((6250U == period) && (125000000U == clock)); }
static bool test_apply (void * p_context, drv_motor_wheel_t wheel,
    uint16_t a, uint16_t b)
{ (void)p_context; (void)wheel; (void)a; (void)b; return (true); }
static bool test_stop (void * p_context) { (void)p_context; return (true); }
static bool test_ready (motion_run_t * p_run)
{
    wheel_loop_config_t cfg = test_motion_config();
    drv_motor_io_t io = {NULL, test_prepare, test_apply, test_stop};
    enc_capture_sample_t sample = {0};

    sample.b_healthy = true;
    sample.sample[0].generation = 1U;
    sample.sample[1].generation = 1U;
    if (!motion_run_init(p_run, &cfg, &io) || !motion_run_enable(p_run, 0U) ||
        !motion_run_tick(p_run, &sample, 0U)) { return (false); }
    sample.sample[0].timestamp_us = 10000U;
    sample.sample[1].timestamp_us = 10000U;
    return (motion_run_tick(p_run, &sample, 10000U) &&
        (MOTION_RUN_READY == p_run->status.state));
}
static void test_run_owner (void)
{
    if (NULL == g_task_info.task) { g_ok = false; return; }
    if (0 == setjmp(g_end)) { g_task_info.task(0, NULL); }
}
static bool test_case (void)
{
    if (85U == g_case)
    {
        vehicle_setup_t setup = {0};
        vehicle_t status;
        setup.mission = test_config();
        setup.motion = test_motion_config();
        if ((E_OK != vehicle_runtime_init()) ||
            (E_OK != vehicle_runtime_setup(&setup)) ||
            (E_OK != vehicle_runtime_start(31U))) { return (false); }
        test_run_owner();
        return ((E_OK == vehicle_runtime_status(&status)) &&
            (VEHICLE_MANOEUVRE == status.state) &&
            (11U == status.navigation_id) && (0U == g_estops));
    }
    if ((83U == g_case) || (84U == g_case))
    {
        vehicle_t status;
        if ((E_OK != vehicle_runtime_init()) ||
            (E_OK != vehicle_runtime_start(5U))) { return (false); }
        test_run_owner();
        return ((E_OK == vehicle_runtime_status(&status)) &&
            (VEHICLE_UNCONFIGURED == status.state) && (0U == g_estops) &&
            (g_mapping_calls == ((83U == g_case) ? 2U : 1U)) &&
            (g_map_result == ((83U == g_case) ? S1_MAP_RESULT_QUEUED :
                S1_MAP_RESULT_REJECTED)));
    }
    if (g_case >= 65U) { return (test_route()); }
    if (64U == g_case)
    {
        vehicle_setup_t setup = {0};

        setup.mission = test_config(); setup.motion = test_motion_config();
        setup.b_obstacle_requested = true;
        setup.obstacle.acquisition = (s5_acquire_config_t){61000U,24000U,
            10000U,2000000U,20000U,1000U,2000U,58U,400U,15U,true,true};
        setup.obstacle.policy = (s5_runtime_config_t){200U,100U,100U,2000U,
            1000U,1000U,25U,2U};
        if ((E_OK != vehicle_runtime_init()) ||
            (E_PAR != vehicle_runtime_setup(&setup)) ||
            (0U != g_s5_configures)) { return (false); }
        setup.mission.sensor_age_us = 200000U;
        return ((E_OK == vehicle_runtime_setup(&setup)) &&
            (E_OBJ == vehicle_runtime_setup(&setup)) &&
            (1U == g_s5_configures) && (0U == g_enables));
    }
    if (63U == g_case)
    {
        if (0 == setjmp(g_end)) { (void)usermain(); }
        return ((5U == g_inits) && (5U == g_starts) &&
            (5U == g_self_tests) && (0U == g_start_attempts) &&
            (0U == g_sim_disabled) && (0U == g_enables) &&
            (0U == g_telemetry));
    }
    if ((g_case >= 57U) && (g_case <= 62U))
    {
        vehicle_setup_t setup = {0};
        vehicle_t status = {0};

        setup.mission = test_config(); setup.motion = test_motion_config();
        if ((E_OK != vehicle_runtime_init()) ||
            (E_OK != vehicle_runtime_setup(&setup)) ||
            (E_OK != vehicle_runtime_start(31U))) { return (false); }
        test_run_owner();
        if ((E_OK != vehicle_runtime_status(&status)) || (0U != g_enables))
        { return (false); }
        if (57U == g_case) { return ((VEHICLE_FAULT == status.state) &&
            (0U == g_submits) && (g_estops > 0U)); }
        if ((60U == g_case) || (61U == g_case))
        {
            return ((VEHICLE_READY == status.state) &&
                (0U == status.blockers) && (1U == g_telemetry));
        }
        return ((VEHICLE_BLOCKED == status.state) &&
            (0U != (status.blockers & VEHICLE_BLOCK_OBSTACLE)) &&
            ((62U != g_case) || ((22U == g_delays) && (3U == g_telemetry))));
    }
    if (52U == g_case)
    {
        return ((E_OK == vehicle_runtime_init()) &&
            (E_SYS == vehicle_runtime_start(15U)) &&
            (E_OK == vehicle_runtime_start(15U)) &&
            (E_OBJ == vehicle_runtime_start(15U)) &&
            (2U == g_start_attempts));
    }
    if ((g_case >= 48U) && (g_case <= 56U))
    {
        vehicle_setup_t setup = {0};
        vehicle_t status = {0};

        setup.mission = test_config();
        setup.motion = test_motion_config();
        if ((E_OK != vehicle_runtime_init()) ||
            (E_OK != vehicle_runtime_setup(&setup)) ||
            (E_OK != vehicle_runtime_start(15U))) { return (false); }
        test_run_owner();
        if ((E_OK != vehicle_runtime_status(&status)) ||
            (0U != g_enables) || (1U != g_s3_reads)) { return (false); }
        if ((48U == g_case) || (51U == g_case) ||
            (54U == g_case) || (55U == g_case))
        {
            return ((g_estops > 0U) && (VEHICLE_FAULT == status.state) &&
                (0U == g_submits));
        }
        return ((VEHICLE_BLOCKED == status.state) &&
            (0U != (status.blockers & VEHICLE_BLOCK_OBSTACLE)) &&
            ((56U != g_case) || (4U == g_imu_events)) &&
            (((49U == g_case) || (56U == g_case)) ?
                (0U == (status.blockers & VEHICLE_BLOCK_LINE)) :
                (0U != (status.blockers & VEHICLE_BLOCK_LINE))));
    }
    if (44U == g_case)
    {
        vehicle_setup_t setup = {0};
        vehicle_t status;

        setup.mission = test_config();
        setup.motion = test_motion_config();
        if ((E_OK != vehicle_runtime_init()) ||
            (E_OK != vehicle_runtime_setup(&setup)) ||
            (E_OK != vehicle_runtime_start(15U))) { return (false); }
        test_run_owner();
        return ((E_OK == vehicle_runtime_status(&status)) &&
            (VEHICLE_BLOCKED == status.state) &&
            (0U == (status.blockers & (VEHICLE_BLOCK_MOTION | VEHICLE_BLOCK_IMU))) &&
            (0U != (status.blockers & VEHICLE_BLOCK_LINE)) &&
            (0U != (status.blockers & VEHICLE_BLOCK_OBSTACLE)) &&
            (0U == g_enables));
    }
    if (45U == g_case)
    {
        if (0 == setjmp(g_end)) { (void)usermain(); }
        test_run_owner();
        return ((5U == g_inits) && (3U == g_starts) &&
            (g_estops > 0U) && (0U == g_enables));
    }
    if (((g_case >= 23U) && (g_case <= 29U)) || (38U == g_case) || (41U == g_case)) { return (test_bank()); }
    if (5U == g_case)
    {
        return ((E_SYS == app_start_task((FP)test_task, 16, TP_PRC1, "test")) &&
            (1U == g_deletes));
    }
    if (31U == g_case)
    {
        return ((E_SYS == vehicle_runtime_init()) &&
            (0 == g_mutex) && (1U == g_deletes));
    }
    if ((g_case >= 3U) && (g_case <= 10U))
    {
        if (0 == setjmp(g_end)) { (void)usermain(); }
        if (4U == g_case)
        {
            return ((5U == g_inits) && (0U == g_starts) &&
                (0U == g_enables) && (0U == g_sim_disabled));
        }
        if (3U == g_case)
        {
            return ((5U == g_inits) && (5U == g_starts) &&
                (1U == g_sim_disabled) && (0U == g_enables));
        }
        test_run_owner();
        vehicle_t status = {0};

        if (E_OK != vehicle_runtime_status(&status)) { return (false); }
        return ((0U == g_enables) && (0U == g_submits) && (1U == g_s3_reads) &&
            (((7U == g_case) || (8U == g_case) || (9U == g_case) ||
              (10U == g_case)) ? (g_estops > 0U) :
                (VEHICLE_BLOCKED == status.state)) &&
            ((9U != g_case) || (g_exit && (VEHICLE_FAULT == status.state))));
    }
    vehicle_t vehicle = {0};
    vehicle_config_t cfg = test_config();
    vehicle_input_t input = test_input();
    if (82U == g_case)
    {
        vehicle_config_t config = test_config();
        vehicle_output_t result;
        if (!vehicle_init(&vehicle, &config)) { return (false); }
        vehicle.b_run_requested = true;
        input.navigation.kind = NAV_EVENT_BARCODE_VALID;
        input.navigation.command = NAV_COMMAND_LEFT;
        input.navigation.timestamp_us = input.raw_now_us;
        input.navigation_id = 1U;
        if (!vehicle_step(&vehicle, &input, &result) || !result.b_command ||
            (MOVE_PLAN_SPEED != result.command.kind) ||
            (0U != vehicle.navigation_id)) { return (false); }
        input.motion.active_id = result.command.id;
        input.motion.state = MOTION_RUN_ACTIVE;
        input.navigation.kind = NAV_EVENT_JUNCTION;
        input.navigation_id = 2U;
        input.pending_navigation_id = 2U;
        input.line_state = LINE_STATE_AWAITING_MANOEUVRE;
        if (!vehicle_step(&vehicle, &input, &result) || !result.b_stop ||
            result.b_command) { return (false); }
        input.motion.active_id = 0U;
        input.motion.terminal_id = vehicle.active_id;
        input.motion.terminal_result = MOTION_RUN_STOPPED;
        input.motion.state = MOTION_RUN_READY;
        if (!vehicle_step(&vehicle, &input, &result) ||
            (0U == result.acknowledge_id)) { return (false); }
        input.motion.terminal_id = 0U;
        return (vehicle_step(&vehicle, &input, &result) && result.b_command &&
            (MOVE_PLAN_LEFT == result.command.kind) &&
            (VEHICLE_MANOEUVRE == vehicle.state));
    }
    vehicle_output_t output;

    if (43U == g_case)
    {
        for (uint32_t cause = 0U; cause < 2U; ++cause)
        {
            if (!vehicle_init(&vehicle, &cfg)) { return (false); }
            vehicle.state = VEHICLE_MANOEUVRE;
            vehicle.active_id = 1U;
            vehicle.b_pending_command = true;
            vehicle.pending_command = (move_plan_command_t){0};
            vehicle.pending_command.id = 1U;
            vehicle.pending_command.issued_us = input.now_us;
            vehicle.pending_command.lifetime_us = cfg.command_ttl_us;
            input = test_input();
            if (0U == cause) { input.forward_cm = 1U; }
            else
            {
                input.remote.command_id = S1_CMD_PAUSE;
                input.remote.rx_timestamp_ms = 20U;
            }
            if (!vehicle_step(&vehicle, &input, &output) || !output.b_estop ||
                output.b_command) { return (false); }
        }
        return (true);
    }
    if (46U == g_case)
    {
        if (!vehicle_init(&vehicle, &cfg)) { return (false); }
        vehicle.state = VEHICLE_STOPPING;
        vehicle.active_id = 1U;
        vehicle.transition_us = 0U;
        input.motion.terminal_id = 1U;
        input.motion.terminal_result = MOTION_RUN_STOPPED;
        input.now_us = (uint64_t)cfg.transition_timeout_us + 1U;
        input.motion.measurement_timestamp_us = input.now_us;
        return (vehicle_step(&vehicle, &input, &output) && output.b_estop &&
            (0U == output.acknowledge_id));
    }
    if (47U == g_case)
    {
        if (!vehicle_init(&vehicle, &cfg)) { return (false); }
        vehicle.b_run_requested = true;
        vehicle.b_recovery_required = true;
        vehicle.state = VEHICLE_RECOVERY;
        vehicle.transition_us = input.now_us;
        input.remote.command_id = S1_CMD_PAUSE;
        input.remote.rx_timestamp_ms = 20U;
        if (!vehicle_step(&vehicle, &input, &output) ||
            (VEHICLE_PAUSED != vehicle.state)) { return (false); }
        input.remote.command_id = S1_CMD_RESUME;
        input.line_mode = LINE_MODE_PAUSED;
        if (!vehicle_step(&vehicle, &input, &output) || output.b_command ||
            !output.b_mode || (LINE_MODE_RECOVERY != output.mode) ||
            (VEHICLE_RECOVERY != vehicle.state)) { return (false); }
        input.remote.command_id = S1_CMD_NONE;
        input.line_mode = LINE_MODE_RECOVERY;
        input.navigation.kind = NAV_EVENT_LINE_FOUND;
        input.navigation.timestamp_us = input.raw_now_us;
        return (vehicle_step(&vehicle, &input, &output) &&
            !vehicle.b_recovery_required && output.b_mode &&
            (LINE_MODE_TRACKING == output.mode));
    }
    if (42U == g_case)
    {
        input.motion.active_id = 99U;
        input.motion.state = MOTION_RUN_ACTIVE;
        input.b_line_available = false;
        if (!vehicle_step(&vehicle, &input, &output) || !output.b_estop)
        { return (false); }
        if (!vehicle_init(&vehicle, &cfg)) { return (false); }
        return (vehicle_step(&vehicle, &input, &output) && output.b_estop &&
            !output.b_enable && !output.b_command);
    }
    if ((39U == g_case) || (40U == g_case))
    {
        vehicle_odometry_t odometry = {0};

        input.motion.state = MOTION_RUN_ACTIVE;
        input.motion.active_kind = MOVE_PLAN_SPEED;
        input.motion.target_mm_s[0] = 100;
        input.motion.target_mm_s[1] = 100;
        input.motion.speed_mm_s[0] = 100;
        input.motion.speed_mm_s[1] = 100;
        input.motion.position_um[0] = -10000;
        input.motion.position_um[1] = -10000;
        if (!vehicle_odometry(&odometry, &input, 500) ||
            odometry.b_tracking_forward) { return (false); }
        input.now_us += 10000U;
        input.raw_now_us += 10000U;
        input.motion.measurement_timestamp_us += 10000U;
        input.motion.position_um[0] += 1000;
        input.motion.position_um[1] += 1000;
        if (!vehicle_odometry(&odometry, &input, 500) ||
            !odometry.b_tracking_forward || (1000U != odometry.distance_um) ||
            (input.raw_now_us != odometry.raw_timestamp_us)) { return (false); }
        if (39U == g_case)
        {
            input.now_us += 10000U;
            input.raw_now_us += 10000U;
            return (!vehicle_odometry(&odometry, &input, 500) &&
                (1000U == odometry.distance_um));
        }
        input.motion.measurement_timestamp_us += 10000U;
        input.now_us += 10000U;
        input.raw_now_us += 10000U;
        input.motion.position_um[0] = INT64_MAX;
        input.motion.position_um[1] = INT64_MAX;
        return (vehicle_odometry(&odometry, &input, 500) &&
            !odometry.b_tracking_forward && (1000U == odometry.distance_um));
    }
    if (35U == g_case)
    {
        vehicle_t turn = {0};
        vehicle_output_t result;

        if (!vehicle_init(&turn, &cfg)) { return (false); }
        turn.state = VEHICLE_MANOEUVRE;
        turn.active_id = 1U;
        input.motion.terminal_id = 1U;
        input.motion.terminal_result = MOTION_RUN_CANCELLED;
        return (vehicle_step(&turn, &input, &result) && result.b_estop &&
            (0U == result.acknowledge_id));
    }
    if (36U == g_case)
    {
        if (!vehicle_init(&vehicle, &cfg)) { return (false); }
        vehicle.b_run_requested = true;
        input.b_line_valid = false;
        return (vehicle_step(&vehicle, &input, &output) && !output.b_command &&
            (0U == vehicle.next_id) && (0U == vehicle.active_id));
    }
    if (37U == g_case)
    {
        if (!vehicle_init(&vehicle, &cfg)) { return (false); }
        vehicle.b_line_found = true;
        input.navigation.kind = NAV_EVENT_JUNCTION;
        input.navigation.command = NAV_COMMAND_RIGHT;
        input.navigation.timestamp_us = input.raw_now_us;
        input.navigation_id = 2U;
        return (vehicle_step(&vehicle, &input, &output) &&
            !vehicle.b_line_found && (2U == vehicle.navigation_id));
    }
    if (32U == g_case)
    {
        cfg.b_calibrated = false;
        return (!vehicle_init(&vehicle, &cfg) &&
            vehicle_step(&vehicle, &input, &output) && !output.b_enable &&
            !output.b_command && (VEHICLE_UNCONFIGURED == vehicle.state));
    }
    if (!vehicle_init(&vehicle, &cfg)) { return (false); }
    input.remote.command_id = s1_codec_parse_command("START_RUN", 9U);
    input.remote.rx_timestamp_ms = 20U;
    if (16U == g_case)
    {
        vehicle.active_id = 1U;
        input.b_obstacle_available = false;
        return (vehicle_step(&vehicle, &input, &output) && output.b_estop);
    }
    if (20U == g_case)
    {
        s5_obstacle_plan_t plan;

        input.forward_cm = 20U;
        input.obstacle.closest_distance_cm = 20U;
        input.obstacle.bypass_direction = S5_BYPASS_LEFT_CANDIDATE;
        return ((S5_LOGIC_CLEARANCE_UNVERIFIED ==
            s5_build_obstacle_plan(&input.obstacle, &plan)) && !plan.executable &&
            vehicle_step(&vehicle, &input, &output) && !output.b_command &&
            (VEHICLE_OBSTACLE == vehicle.state));
    }
    if (21U == g_case)
    {
        input.line_time_us = input.now_us; /* Wrong clock must not look fresh. */
        return (vehicle_step(&vehicle, &input, &output) &&
            (0U != (vehicle.blockers & VEHICLE_BLOCK_LINE)) && !output.b_enable);
    }
    if (22U == g_case)
    {
        s1_codec_frame_t frame;

        return (vehicle_frame(&vehicle, &input, &frame) &&
            (2U == frame.schema_version) &&
            (0U == (frame.validity & S1_VALID_ENCODER)));
    }
    if (30U == g_case)
    {
        for (uint32_t invalid = 0U; invalid < 6U; ++invalid)
        {
            input = test_input();
            if (!vehicle_init(&vehicle, &cfg)) { return (false); }
            switch (invalid)
            {
                case 0U: input.line = (line_observation_t)255; break;
                case 1U: input.line_state = (line_state_t)255; break;
                case 2U: input.line_mode = (line_mode_t)255; break;
                case 3U: input.navigation.kind = (nav_event_kind_t)255; break;
                case 4U: input.remote.command_id = (s1_codec_command_id_t)255; break;
                /* Packed phase has three bits; 7 is representable/invalid. */
                default: input.imu.phase = 7U; break;
            }
            if (!vehicle_step(&vehicle, &input, &output) || !output.b_estop)
            { return (false); }
        }
        return (true);
    }
    if (33U == g_case)
    {
        imu_core_t imu;
        imu_core_config_t config = imu_core_defaults();
        imu_core_sample_t sample = {0};

        config.calibration_samples = 2U;
        sample.accel_g[2] = 1.0F;
        if (!imu_core_init(&imu, &config)) { return (false); }
        (void)imu_core_update(&imu, &sample);
        sample.time_ms = 10U;
        (void)imu_core_update(&imu, &sample);
        sample.time_ms = 20U;
        sample.accel_g[0] = 1.5F;
        sample.accel_g[1] = 1.5F;
        (void)imu_core_update(&imu, &sample);
        input.imu = imu.status;
        input.imu.time_ms = 1020U;
        return ((0U != (imu.status.events & IMU_EVENT_IMPACT)) &&
            vehicle_step(&vehicle, &input, &output) && output.b_estop);
    }
    if (34U == g_case)
    {
        input.remote.command_id = S1_CMD_NONE;
        input.motion.state = MOTION_RUN_CONFIGURED;
        return (vehicle_step(&vehicle, &input, &output) &&
            (VEHICLE_READY == vehicle.state) && !output.b_enable);
    }
    if ((17U == g_case) || (18U == g_case) || (19U == g_case))
    {
        nav_map_t map;

        nav_map_init(&map);
        if (!nav_map_apply(&map, "MAP A=LEFT")) { return (false); }
        input.navigation.command = nav_map_lookup(&map, 'A');
        input.navigation.kind = NAV_EVENT_JUNCTION;
        input.navigation.timestamp_us = input.raw_now_us;
        input.navigation_id = 10U;
        input.pending_navigation_id = 10U;
        input.line_state = LINE_STATE_AWAITING_MANOEUVRE;
        if (!vehicle_step(&vehicle, &input, &output) || !output.b_command ||
            (MOVE_PLAN_LEFT != output.command.kind)) { return (false); }
        if (17U == g_case) { return (VEHICLE_MANOEUVRE == vehicle.state); }
        if (19U == g_case)
        {
            input.b_estop = true;
            return (vehicle_step(&vehicle, &input, &output) && output.b_estop &&
                !output.b_command);
        }
        input.motion.terminal_id = output.command.id;
        input.motion.terminal_result = MOTION_RUN_DONE;
        if (!vehicle_step(&vehicle, &input, &output) ||
            (0U == output.acknowledge_id) ||
            (0U != output.navigation_done_id)) { return (false); }
        input.motion.terminal_id = 0U;
        input.motion.last_request_id = vehicle.pending_command.id;
        input.motion.last_request_issued_us = vehicle.pending_command.issued_us;
        input.motion.b_request_accepted = true;
        return (vehicle_step(&vehicle, &input, &output) &&
            (10U == output.navigation_done_id) && !output.b_command);
    }
    if (!vehicle_step(&vehicle, &input, &output) || !output.b_command)
    {
        return (false);
    }
    move_plan_command_t first = output.command;

    if (12U == g_case)
    {
        input.now_us += 10000U;
        return (vehicle_step(&vehicle, &input, &output) && output.b_command &&
            (first.id == output.command.id) &&
            (first.issued_us == output.command.issued_us));
    }
    if (13U == g_case)
    {
        input.motion.last_request_id = first.id;
        input.motion.last_request_issued_us = first.issued_us;
        input.motion.b_request_accepted = false;
        return (vehicle_step(&vehicle, &input, &output) && output.b_estop);
    }
    motion_run_t run;

    if (!test_ready(&run) || !motion_run_command(&run, &first, input.now_us))
    {
        return (false);
    }
    input.motion = run.status;
    input.now_us += 10000U;
    input.raw_now_us += 10000U;
    input.remote.command_id = S1_CMD_NONE;
    if (15U == g_case)
    {
        input.remote.command_id = S1_CMD_PAUSE;
        input.remote.rx_timestamp_ms = 30U;
        if (!vehicle_step(&vehicle, &input, &output) || !output.b_stop)
        { return (false); }
        input.motion.terminal_id = first.id;
        input.motion.terminal_result = MOTION_RUN_STOPPED;
        input.motion.state = MOTION_RUN_READY;
        input.motion.active_id = 0U;
        if (!vehicle_step(&vehicle, &input, &output) || output.b_stop ||
            (first.id != output.acknowledge_id)) { return (false); }
        input.motion.terminal_id = 0U;
        return (vehicle_step(&vehicle, &input, &output) &&
            (VEHICLE_PAUSED == vehicle.state) && !output.b_command);
    }
    input.black_mask = 1U;
    if (!line_classify(input.black_mask, &input.line) ||
        !vehicle_step(&vehicle, &input, &output) || !output.b_command)
    { return (false); }
    if ((first.id != output.command.id) ||
        (80 != output.command.left_mm_s) ||
        (100 != output.command.right_mm_s)) { return (false); }
    if (11U == g_case) { return (true); }
    if (14U == g_case)
    {
        uint64_t deadline = run.loop.last_tick_us;

        return (motion_run_command(&run, &output.command, input.now_us) &&
            (run.loop.last_tick_us == deadline) &&
            (run.status.active_id == first.id));
    }
    return (false);
}

static void test_sink (void * p_context, uint32_t events, uint32_t levels)
{
    ++g_callbacks;
    if (NULL != p_context)
    {
        g_ok = enc_capture_feed(p_context, events, levels) && g_ok;
    }
}
static void test_fault (void) { ++g_faults; }
static bool test_bank (void)
{
    gpio_bank_t bank;
    gpio_bank_io_t io = {test_port_read, test_port_write, test_port_core,
        test_port_enter, test_port_leave, NULL, test_fault};
    /* Test installation uses the same typed function through this wrapper. */
    io.p_install = test_bank_install;
    if (!gpio_bank_init(&bank, &io, ENC_CAPTURE_MASK | (1UL << 6U)))
    { return (false); }
    if (23U == g_case)
    {
        if (!gpio_bank_reserve(&bank, ENC_CAPTURE_MASK, test_sink, NULL))
        { return (false); }
        g_core = 0U;
        return (!gpio_bank_enable(&bank, ENC_CAPTURE_MASK) &&
            !gpio_bank_release(&bank, ENC_CAPTURE_MASK));
    }
    if (29U == g_case)
    {
        g_install_ok = false;
        return (!gpio_bank_reserve(&bank, ENC_CAPTURE_MASK, test_sink, NULL) &&
            (0U == bank.clients[0].mask) && (0U == bank.clients[1].mask));
    }
    enc_capture_t capture;
    wheel_loop_config_t cfg = test_motion_config();

    if (!enc_capture_init(&capture, &cfg.geometry, 1U) ||
        !enc_capture_feed(&capture, ENC_CAPTURE_MASK, 0U) ||
        !gpio_bank_reserve(&bank, ENC_CAPTURE_MASK, test_sink, &capture) ||
        gpio_bank_reserve(&bank, 1UL << 7U, test_sink, NULL)) { return (false); }
    if (!gpio_bank_enable(&bank, ENC_CAPTURE_MASK)) { return (false); }
    uint32_t edge = (27U == g_case) ? 12U : 8U;
    g_regs[0xFCU / 4U] = edge << 16U; /* GP28, group3 nibble4. */
    g_levels = 1UL << 28U;
    if (41U == g_case)
    {
        g_regs[0xF0U / 4U] |= 8UL << 20U;
        g_regs[0x130U / 4U] |= 8UL << 20U; /* Illicit unregistered owner. */
        gpio_bank_irq(&bank);
        return ((1U == g_faults) && (0U == g_callbacks) &&
            (0U != (g_regs[0xF0U / 4U] & (8UL << 20U))));
    }
    if (24U == g_case)
    {
        g_regs[0xF0U / 4U] |= 8UL << 20U; /* Unowned GP5 edge. */
        gpio_bank_irq(&bank);
        return ((0U != (g_regs[0xF0U / 4U] & (8UL << 20U))) &&
            (0U == g_regs[0xFCU / 4U]));
    }
    if (26U == g_case)
    {
        if (!gpio_bank_release(&bank, ENC_CAPTURE_MASK)) { return (false); }
        gpio_bank_irq(&bank);
        return ((0U == g_callbacks) &&
            !gpio_bank_release(&bank, ENC_CAPTURE_MASK));
    }
    if (28U == g_case)
    {
        if (!gpio_bank_reserve(&bank, 1UL << 6U, test_sink, NULL) ||
            !gpio_bank_enable(&bank, 1UL << 6U)) { return (false); }
        g_regs[0xF0U / 4U] |= 8UL << 24U;
    }
    gpio_bank_irq(&bank);
    if ((27U == g_case) || (38U == g_case))
    {
        return ((1U == g_faults) && (0U == g_callbacks) &&
            (0 == capture.decoder[0].count));
    }
    return ((1 == capture.decoder[0].count) &&
        (0 == capture.decoder[1].count) && (1U == g_reads) &&
        (g_callbacks == ((28U == g_case) ? 2U : 1U)));
}

static void test_task (INT code, void * p_context)
{
    (void)code;
    (void)p_context;
}

static bool test_bank_install (void) { return (g_install_ok); }
/*** end of file ***/


static bool route_cycle (vehicle_t * p_vehicle, vehicle_input_t * p_input,
    vehicle_output_t * p_output)
{
    p_input->now_us += 10000U;
    p_input->raw_now_us += 10000U;
    p_input->motion.measurement_timestamp_us = p_input->now_us;
    p_input->line_time_us = p_input->raw_now_us;
    p_input->obstacle_time_us = p_input->raw_now_us;
    p_input->obstacle_started_us = p_input->raw_now_us;
    p_input->imu.time_ms = (uint32_t)(p_input->raw_now_us / 1000U);
    return (vehicle_step(p_vehicle,p_input,p_output));
}

static bool test_route (void)
{
    vehicle_config_t config = test_config();
    /* HOST fixtures; neither dimensions nor corridor coverage are physical evidence. */
    config.route = (s5_route_config_t){50000,80000,10000,5000,10000,0,
        100000,200000,50000,200000,50,1000000U,20000000U,true};
    vehicle_t vehicle = {0};
    vehicle_input_t input = test_input();
    vehicle_output_t output = {0};
    const bool right = (66U == g_case);

    input.scan_id_us = 100000U;
    input.scan_completed_us = 1000000U;
    input.b_scan_held = true;
    input.obstacle.valid = true;
    input.obstacle.forward_distance_cm = 40U;
    input.obstacle.bypass_direction = right ?
        S5_BYPASS_RIGHT_CANDIDATE : S5_BYPASS_LEFT_CANDIDATE;
    input.clearance = (s5_clearance_t){
        {{-155000,155000,-155000,460000},
         {-155000,960000,150000,460000},
         {650000,1160000,-155000,460000}},
        input.scan_id_us,1010000U,40000000U,true,true};
    if (right)
    {
        for (size_t n = 0U; n < 3U; ++n)
        {
            int32_t old_min = input.clearance.corridor[n].y_min_um;
            input.clearance.corridor[n].y_min_um =
                -input.clearance.corridor[n].y_max_um;
            input.clearance.corridor[n].y_max_um = -old_min;
        }
    }
    if (79U == g_case)
    {
        config.route.speed_mm_s = 0;
        return (!vehicle_init(&vehicle,&config) && !vehicle.b_initialised);
    }
    if (!vehicle_init(&vehicle,&config)) { return (false); }
    vehicle.b_run_requested = true;
    vehicle.b_obstacle_pending = true;
    vehicle.state = VEHICLE_OBSTACLE;
    if (67U == g_case) { input.clearance.b_verified = false; }
    if (81U == g_case) { input.clearance.b_original_line_verified = false; }
    if (80U == g_case) { input.clearance.corridor[1].y_min_um = -200000; }
    input.line_mode = (68U == g_case) ? LINE_MODE_TRACKING : LINE_MODE_BYPASS;
    if (!route_cycle(&vehicle,&input,&output)) { return (false); }
    if ((67U == g_case) || (80U == g_case) || (81U == g_case))
    {
        return (!output.b_command && !vehicle.b_route_active &&
            (VEHICLE_OBSTACLE == vehicle.state));
    }
    if (68U == g_case)
    {
        if (!output.b_mode || output.b_command ||
            (LINE_MODE_BYPASS != output.mode)) { return (false); }
        input.line_mode = LINE_MODE_BYPASS;
        if (!route_cycle(&vehicle,&input,&output)) { return (false); }
    }
    if (!output.b_command || !vehicle.route.executable) { return (false); }
    for (uint32_t step = 0U; step < 8U; ++step)
    {
        if (0U != step)
        {
            input.line_mode = (7U == step) ? LINE_MODE_RECOVERY : LINE_MODE_BYPASS;
            if (7U == step)
            {
                input.line = LINE_OBSERVATION_ABSENT;
                input.black_mask = 0U;
                input.line_state = LINE_STATE_RECOVERY;
            }
            if ((77U == g_case) && (1U == step))
            {
                input.obstacle_time_us = input.raw_now_us; /* Fresh fall from an OLD trigger. */
                input.obstacle_started_us = vehicle.route_ready_raw_us - 1U;
                input.now_us += 1U; input.raw_now_us += 1U;
                if (!vehicle_step(&vehicle,&input,&output) || output.b_command)
                { return (false); }
                if (!route_cycle(&vehicle,&input,&output) || !output.b_command)
                { return (false); }
                return (true);
            }
            if (!route_cycle(&vehicle,&input,&output) || !output.b_command)
            { return (false); }
        }
        const move_plan_command_t command = output.command;
        move_plan_t plan = {0};
        wheel_loop_config_t motion = test_motion_config();
        const int64_t origin[2] = {0,0};
        int32_t speeds[2] = {0,0};
        bool arrived = false;

        if (!move_plan_begin(&plan,&command,&motion.profile,&motion.geometry,
            origin,input.now_us)) { return (false); }
        const int64_t goal[2] = {plan.goal_um[0],plan.goal_um[1]};
        if (!move_plan_step(&plan,&motion.profile,goal,speeds,&arrived) ||
            !arrived) { return (false); }
        if (0U == step)
        {
            const move_plan_kind_t expected = right ? MOVE_PLAN_RIGHT : MOVE_PLAN_LEFT;
            if (expected != command.kind) { return (false); }
        }
        input.motion.state = MOTION_RUN_ACTIVE;
        input.motion.active_id = command.id;
        input.motion.b_standstill = false;
        input.motion.last_request_id = command.id;
        input.motion.last_request_issued_us = command.issued_us;
        input.motion.b_request_accepted = true;
        if (69U == g_case) { input.now_us += config.route.step_timeout_us; }
        if (70U == g_case)
        {
            input.remote.command_id = S1_CMD_PAUSE;
            input.remote.rx_timestamp_ms = (uint32_t)(input.now_us / 1000U);
        }
        if (71U == g_case) { input.raw_now_us = vehicle.route.valid_until_us; }
        if (75U == g_case) { input.b_line_valid = false; }
        if (76U == g_case) { ++input.scan_id_us; }
        if (78U == g_case) { input.line_mode = LINE_MODE_PAUSED; }
        if (!route_cycle(&vehicle,&input,&output)) { return (false); }
        if ((g_case >= 69U && g_case <= 71U) ||
            (g_case >= 75U && g_case <= 76U) || (78U == g_case))
        {
            return (output.b_estop && !output.b_command &&
                (VEHICLE_FAULT == vehicle.state));
        }
        if (output.b_command) { return (false); } /* No finite refresh. */
        if (68U == g_case) { return (true); }
        if ((7U == step) && (72U != g_case))
        {
            if (73U == g_case)
            {
                input.navigation.kind = NAV_EVENT_LINE_FOUND;
                input.navigation.timestamp_us = vehicle.route_step_raw_us - 1U;
                if (!route_cycle(&vehicle,&input,&output) || output.b_stop ||
                    vehicle.b_route_found) { return (false); }
            }
            input.navigation.kind = NAV_EVENT_LINE_FOUND;
            input.navigation.timestamp_us = input.raw_now_us + 10000U;
            input.line_mode = LINE_MODE_PAUSED; /* Actual S3 found-event behavior. */
            input.line = LINE_OBSERVATION_CENTRED;
            input.black_mask = 3U;
            if (!route_cycle(&vehicle,&input,&output) ||
                !output.b_stop || !vehicle.b_route_found) { return (false); }
        }
        input.motion.terminal_id = command.id + ((74U == g_case) ? 1U : 0U);
        input.motion.terminal_result = (7U == step && 72U != g_case) ?
            MOTION_RUN_STOPPED : MOTION_RUN_DONE;
        input.motion.active_id = 0U;
        input.motion.state = MOTION_RUN_READY;
        input.motion.b_standstill = true;
        if (!route_cycle(&vehicle,&input,&output)) { return (false); }
        if ((74U == g_case) || (7U == step && 72U == g_case))
        {
            return (output.b_estop && (VEHICLE_FAULT == vehicle.state));
        }
        if ((command.id != output.acknowledge_id) || output.b_command)
        { return (false); }
        if (!route_cycle(&vehicle,&input,&output) ||
            (command.id != output.acknowledge_id) ||
            (vehicle.route_index != step)) { return (false); }
        input.motion.terminal_id = 0U;
        if (!route_cycle(&vehicle,&input,&output) || output.b_command)
        { return (false); }
    }
    if (!vehicle.b_route_finishing ||
        (vehicle.route.scan_id_us != output.obstacle_recovered_us))
    { return (false); }
    input.b_scan_held = false;
    if (!route_cycle(&vehicle,&input,&output) || !output.b_mode ||
        (LINE_MODE_TRACKING != output.mode)) { return (false); }
    input.line_mode = LINE_MODE_TRACKING;
    return (route_cycle(&vehicle,&input,&output) &&
        !vehicle.b_route_active && !vehicle.b_obstacle_pending &&
        !vehicle.b_route_finishing && (VEHICLE_READY == vehicle.state) &&
        !output.b_command);
}
/*** end of file ***/
