#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* These C-only task fakes perform nonlocal exits, not Windows exception
 * unwinding. Use MinGW's documented non-SEH setjmp mode; production ARM
 * code never includes this host harness or macro.
 */
#if defined(__MINGW32__)
#define __USE_MINGW_SETJMP_NON_SEH 1
#endif
#include <setjmp.h>
#include "../../../app_program/subsystem5/s5_acquire.h"
#if defined(S5_LIVE_HOST_TEST)
#include "../../../app_program/subsystem5/s5_live.h"
#include "../../../app_program/subsystem2/motion_api.h"
#endif

#include "../../../app_program/subsystem5/s5_obstacle_runtime.h"

static unsigned int s5_test_failures;

static void
check_result(
	const char *test_id,
	bool condition,
	const char *description)
{
	if (condition)
	{
		(void)printf("[TEST] %s PASS  %s\n", test_id, description);
	}
	else
	{
		s5_test_failures++;
		(void)printf("[TEST] %s FAIL  %s\n", test_id, description);
	}
}

static s5_runtime_config_t make_config(void);
static uint64_t g_test_now;
static uint64_t g_trigger_us;
static bool g_trigger_seen;
static bool g_trigger_too_soon;
static uint32_t g_open_count;
static uint32_t g_echo_width = 580U;
static uint32_t g_close_count;
static bool g_no_echo;
static bool g_echo_after_clock;
static bool g_open_fail;
static bool g_servo_fail;
static uint8_t g_last_angle;
static bool port_open (void * p, s5_acquire_config_t const * c)
{ (void)p; (void)c; ++g_open_count; return (!g_open_fail); }
static bool port_servo (void * p, uint8_t angle)
{ (void)p; g_last_angle = angle; return (!g_servo_fail); }
static bool port_trigger (void * p)
{
    (void)p;
    if (g_trigger_seen && (g_test_now - g_trigger_us <= 60000U))
    { g_trigger_too_soon = true; }
    g_trigger_seen = true; g_trigger_us = g_test_now; return (true);
}
static int32_t port_echo (void * p, uint32_t * width, uint64_t * capture)
{
    (void)p;
    *width = g_echo_width; *capture = g_trigger_us + g_echo_width;
    return ((!g_no_echo && ((g_test_now >= *capture) || g_echo_after_clock)) ? 1 : 0);
}
static void port_close (void * p) { (void)p; ++g_close_count; }
static s5_acquire_config_t acquire_config (void)
{
    const s5_acquire_config_t c = {61000U,24000U,10000U,2000000U,
        20000U,1000U,2000U,58U,400U,15U,true,true};
    return (c); /* HOST fixture only; no measured calibration claim. */
}
static s5_acquire_io_t acquire_io (void)
{
    const s5_acquire_io_t io = {NULL,port_open,port_servo,port_trigger,
        port_echo,port_close};
    return (io);
}
static bool acquire_step (s5_acquire_t * owner, uint64_t time,
    bool scan, bool stationary, bool cancel)
{
    g_test_now = time;
    return (s5_acquire_step(owner,time,scan,stationary,cancel));
}
static void test_hold (void);
static void test_acquisition (void)
{
    s5_acquire_t owner = {0};
    s5_acquire_config_t config = acquire_config();
    s5_acquire_io_t io = acquire_io();
    bool result;

    config.b_calibrated = false;
    result = !s5_acquire_open(&owner,&config,&io,0U) && (0U == g_open_count);
    check_result("S5-U48",result,"unconfirmed calibration cannot open outputs");
    config = acquire_config();
    result = s5_acquire_open(&owner,&config,&io,0U) &&
        !s5_acquire_open(&owner,&config,&io,0U) && (1U == g_open_count);
    check_result("S5-U49",result,"single open and duplicate lifecycle rejection");
    g_echo_after_clock = true; /* ISR completes after the task's clock read. */
    result = acquire_step(&owner,10000U,false,false,false) &&
        !owner.b_forward_valid &&
        acquire_step(&owner,20000U,false,false,false) && owner.b_forward_valid &&
        (10U == owner.forward_cm) && (10580U == owner.forward_us);
    g_echo_after_clock = false;
    check_result("S5-U50",result,"capture after clock read defers, then preserves range/time");
    result = acquire_step(&owner,30000U,true,false,false) && !owner.b_scan;
    check_result("S5-U51",result,"moving vehicle cannot begin scan");
    result = acquire_step(&owner,40000U,true,true,false);
    for (uint64_t time = 50000U; result && !owner.b_scan_complete &&
        (time < 1900000U); time += 10000U)
    {
        result = acquire_step(&owner,time,false,true,false);
    }
    result = result && owner.b_scan_complete && owner.scan.complete &&
        (owner.fine_count > 0U) && (owner.fine_count <= S5_FINE_SCAN_MAX_COUNT) &&
        (90U == g_last_angle) && !owner.b_scan && !g_trigger_too_soon;
    check_result("S5-U52",result,"bounded coarse/fine scan returns sensor forward");
    s5_acquire_close(&owner); s5_acquire_close(&owner);
    check_result("S5-U53",1U == g_close_count,"close is idempotent");
    owner = (s5_acquire_t){0}; g_trigger_seen = false; g_no_echo = true; g_test_now = 0U;
    result = s5_acquire_open(&owner,&config,&io,0U) &&
        acquire_step(&owner,10000U,false,true,false) &&
        !acquire_step(&owner,34000U,false,true,false) &&
        !owner.b_forward_valid && !owner.b_open;
    check_result("S5-U54",result,"echo deadline fails closed exactly at boundary");
    owner = (s5_acquire_t){0}; g_no_echo = false; g_test_now = 0U;
    result = s5_acquire_open(&owner,&config,&io,0U) &&
        acquire_step(&owner,1000U,true,true,false) &&
        !acquire_step(&owner,2000U,false,false,false) && !owner.b_open;
    check_result("S5-U55",result,"motion during scan closes acquisition");
    owner = (s5_acquire_t){0};
    result = s5_acquire_open(&owner,&config,&io,1000U) &&
        !acquire_step(&owner,999U,false,true,false);
    check_result("S5-U56",result,"clock regression fails closed");
    owner = (s5_acquire_t){0};
    result = s5_acquire_open(&owner,&config,&io,1000U) &&
        !acquire_step(&owner,2000U,false,true,true) && !owner.b_open &&
        (S5_ACQUISITION_ABORTED == owner.scan.acquisition_status);
    check_result("S5-U57",result,"explicit cancellation disables outputs");
}

#if defined(S5_LIVE_HOST_TEST)
static T_CTSK g_task;
static ID g_next_id;
static uint8_t g_kind[16];
static uint32_t g_create_fail;
static uint32_t g_creates;
static uint32_t g_releases;
static uint32_t g_cycles;
static uint32_t g_live_case;
static bool g_recovery_checked;
static uint32_t g_limit;
static uint32_t g_fault_bits;
static bool g_bad_resource;
static bool g_start_fail;
static bool g_busy;
static jmp_buf g_end;
static ID create_object (uint8_t kind)
{
    if (++g_creates == g_create_fail) { return (E_SYS); }
    ID id = ++g_next_id;
    g_kind[id] = kind; return (id);
}
static ER delete_object (ID id, uint8_t kind)
{
    if ((id <= 0) || (id >= 16) || (g_kind[id] != kind))
    {
        g_bad_resource = true; return (E_SYS);
    }
    g_kind[id] = 0U; ++g_releases; return (E_OK);
}
ID tk_cre_mtx (T_CMTX const * p) { (void)p; return create_object(1U); }
ID tk_cre_flg (T_CFLG const * p) { (void)p; return create_object(2U); }
ID tk_cre_tsk (T_CTSK const * p) { g_task = *p; return create_object(3U); }
ER tk_del_mtx (ID id) { return delete_object(id,1U); }
ER tk_del_flg (ID id) { return delete_object(id,2U); }
ER tk_del_tsk (ID id) { return delete_object(id,3U); }
ER tk_loc_mtx (ID id, TMO timeout)
{
    if ((g_kind[id] != 1U) || (TMO_POL != timeout)) { g_bad_resource = true; }
    return (g_busy ? E_TMOUT : E_OK);
}
ER tk_unl_mtx (ID id)
{ if (g_kind[id] != 1U) { g_bad_resource = true; } return (E_OK); }
ER tk_set_flg (ID id, UINT bits)
{
    if (g_kind[id] != 2U) { g_bad_resource = true; }
    g_fault_bits |= bits; return (E_OK);
}
ER tk_wai_flg (ID id, UINT bits, UINT mode, UINT * p, TMO timeout)
{
    if ((g_kind[id] != 2U) || (TMO_POL != timeout) || (TWF_ORW != mode))
    { g_bad_resource = true; }
    *p = g_fault_bits & bits; return ((*p > 0U) ? E_OK : E_TMOUT);
}
ER tk_sta_tsk (ID id, INT code)
{
    (void)code;
    if (g_kind[id] != 3U) { g_bad_resource = true; }
    if (g_start_fail) { g_start_fail = false; return (E_SYS); }
    return (E_OK);
}
ER tk_dly_tsk (TMO delay)
{
    if (10 != delay) { g_bad_resource = true; }
    g_test_now += 10000U;
    ++g_cycles;
    if ((86U == g_live_case) && (150U == g_cycles))
    {
        s5_live_status_t snapshot = {0};
        g_recovery_checked = (E_OK == s5_live_status(&snapshot)) &&
            (S5_RUNTIME_HOLD == snapshot.policy.state) &&
            snapshot.policy.scan_valid && (0U != snapshot.scan_id_us) &&
            (E_OBJ == s5_live_recovered(snapshot.scan_id_us + 1U)) &&
            (E_OK == s5_live_recovered(snapshot.scan_id_us)) &&
            (E_OK == s5_live_recovered(snapshot.scan_id_us));
        g_echo_width = 5800U; /* New real engine samples clear the obstacle. */
    }
    if (g_cycles >= g_limit) { longjmp(g_end,1); }
    return (E_OK);
}
void tk_ext_tsk (void) { longjmp(g_end,1); }
bool drv_s5_now (uint64_t * p) { *p = g_test_now; return (true); }
bool drv_s5_bind (drv_s5_t * driver, drv_encoder_port_t const * irq,
    s5_acquire_io_t * io)
{ (void)driver; (void)irq; *io = acquire_io(); return (true); }
motion_api_result_t motion_api_status (motion_run_status_t * p)
{
    *p = (motion_run_status_t){0};
    if (86U == g_live_case)
    {
        p->state = MOTION_RUN_READY;
        p->b_measurement_valid = true;
        p->b_standstill = true;
    }
    return (MOTION_API_QUEUED);
}
static bool reserve_irq (void * p, uint32_t mask, drv_encoder_sink_t sink,
    void * context)
{ (void)p; (void)mask; (void)sink; (void)context; return (true); }
static bool enable_irq (void * p,uint32_t mask)
{ (void)p; (void)mask; return (true); }
static void release_irq (void * p,uint32_t mask) { (void)p; (void)mask; }
static int test_live (uint32_t number)
{
    bool result;
    g_live_case = number;
    if (58U == number)
    {
        g_create_fail = 2U;
        result = (E_SYS == s5_live_init()) && (1U == g_releases);
        g_create_fail = g_creates + 3U;
        result = result && (E_SYS == s5_live_init()) && (3U == g_releases);
        check_result("S5-U58",result && !g_bad_resource,
            "partial resource cleanup preserves identities and original error");
    }
    else
    {
        s5_live_config_t config = {0};
        drv_encoder_port_t irq = {NULL,reserve_irq,enable_irq,release_irq};
        s5_live_status_t status = {0};

        config.acquisition = acquire_config();
        config.policy = make_config();
        config.policy.forward_sample_max_age_ms = 200U;
        result = (E_OK == s5_live_init()) && (E_OBJ == s5_live_init());
        if (59U != number) { result = result &&
            (E_OK == s5_live_configure(&config,&irq)); }
        if (62U == number)
        {
            g_start_fail = true;
            result = result && (E_SYS == s5_live_start());
        }
        result = result && (E_OK == s5_live_start()) &&
            (E_OBJ == s5_live_start()) &&
            (E_OBJ == s5_live_configure(&config,&irq));
        g_limit = (86U == number) ? 180U : ((61U == number) ? 6U : 4U);
        g_no_echo = (61U == number);
        if (!result || (NULL == g_task.task)) { return 1; }
        if (0 == setjmp(g_end)) { g_task.task(0,NULL); }
        result = result && (E_OK == s5_live_status(&status));
        if (86U == number)
        {
            result = result && g_recovery_checked &&
                (S5_RUNTIME_MONITOR == status.policy.state) &&
                !status.policy.scan_complete && !status.policy.scan_valid &&
                status.b_forward_valid && (100U == status.forward_cm);
        }
        else if (59U == number) { result = result && !status.b_configured &&
            !status.b_forward_valid && (0U == g_open_count); }
        else if (61U == number) { result = result && status.b_fault &&
            !status.b_forward_valid && (E_OK == s5_live_fault()) &&
            (S5_RUNTIME_FAULT == status.policy.state) &&
            (S5_RUNTIME_FAULT_FORWARD_SENSOR == status.policy.fault) &&
            (S5_ACQUISITION_ECHO_TIMEOUT == status.policy.acquisition_status) &&
            (1U == g_close_count); }
        else { result = result && status.b_forward_valid &&
            (10580U == status.forward_timestamp_us) &&
            (10U == status.forward_cm); }
        if (63U == number)
        {
            g_test_now = status.forward_timestamp_us + status.max_age_us + 1U;
            result = result && (E_OK == s5_live_status(&status)) &&
                !status.b_forward_valid;
            g_busy = true; status.forward_cm = 123U;
            result = result && (E_TMOUT == s5_live_status(&status)) &&
                (123U == status.forward_cm);
        }
        char id[20]; (void)snprintf(id,sizeof(id),"S5-U%u",number);
        check_result(id,result && !g_bad_resource,
            "real live owner: startup, timestamp, latch, bounded status or retry");
    }
    return (0U == s5_test_failures) ? 0 : 1;
}
#endif

static s5_runtime_config_t
make_config(void)
{
	s5_runtime_config_t config;

	config.forward_sample_max_age_ms = 50U;
	config.stop_timeout_ms = 100U;
	config.scan_timeout_ms = 2000U;
	config.bypass_timeout_ms = 1000U;
	config.line_timeout_ms = 1000U;
	config.obstacle_threshold_cm = 25U;
	config.confirmation_count = 2U;
	return config;
}

static void
make_input(
	s5_runtime_input_t *input,
	uint32_t now_ms,
	uint32_t sample_time_ms,
	uint16_t distance_cm)
{
	memset(input, 0, sizeof(*input));
	input->now_ms = now_ms;
	input->forward_sample_time_ms = sample_time_ms;
	input->forward_distance_cm = distance_cm;
	input->forward_valid = true;
}

static bool
init_runtime(s5_runtime_t *runtime)
{
	s5_runtime_config_t config;

	config = make_config();
	return (s5_runtime_init(runtime, &config) == S5_LOGIC_OK);
}

static bool
trigger_stop(s5_runtime_t *runtime, s5_runtime_input_t *input)
{
	s5_runtime_events_t events;

	make_input(input, 100U, 100U, 20U);
	if ((S5_LOGIC_OK != s5_runtime_step(runtime, input, &events)) ||
	    events.request_stop ||
	    (runtime->status.state != S5_RUNTIME_MONITOR))
	{
		return false;
	}

	make_input(input, 110U, 110U, 20U);
	if ((S5_LOGIC_OK != s5_runtime_step(runtime, input, &events)) ||
	    !events.request_stop ||
	    (runtime->status.state != S5_RUNTIME_WAIT_STOP))
	{
		return false;
	}
	return true;
}

static void
make_valid_scan(
	s5_runtime_input_t *input,
	s5_scan_context_t *context,
	s5_scan_sample_t *coarse,
	s5_scan_sample_t *fine)
{
	static const uint16_t coarse_distances[S5_COARSE_SCAN_COUNT] =
	{
		100U, 80U, 50U, 40U, 60U
	};
	static const uint16_t fine_distances[S5_FINE_SCAN_MAX_COUNT] =
	{
		60U, 50U, 32U, 29U, 30U, 31U, 32U, 50U, 60U
	};

	*context = (s5_scan_context_t){
		.started_at_ms = 120U,
		.completed_at_ms = 1520U,
		.evaluated_at_ms = 1620U,
		.max_duration_ms = 2000U,
		.max_age_ms = 500U,
		.max_adjacent_range_delta_cm = 100U,
		.acquisition_status = S5_ACQUISITION_OK,
		.complete = true,
		.cancelled = false,
		.servo_settled = true,
		.vehicle_stationary = true
	};

	for (size_t index = 0U; index < S5_COARSE_SCAN_COUNT; index++)
	{
		coarse[index] = (s5_scan_sample_t){
			.angle_deg = (uint8_t)(30U + (index * 30U)),
			.distance_cm = coarse_distances[index],
			.captured_at_ms = 200U + (uint32_t)(index * 100U),
			.valid = true
		};
	}
	for (size_t index = 0U; index < S5_FINE_SCAN_MAX_COUNT; index++)
	{
		fine[index] = (s5_scan_sample_t){
			.angle_deg = (uint8_t)(70U + (index * 5U)),
			.distance_cm = fine_distances[index],
			.captured_at_ms = 700U + (uint32_t)(index * 100U),
			.valid = true
		};
	}

	input->scan_context = context;
	input->coarse_samples = coarse;
	input->coarse_count = S5_COARSE_SCAN_COUNT;
	input->fine_samples = fine;
	input->fine_count = S5_FINE_SCAN_MAX_COUNT;
	input->acquisition_status = S5_ACQUISITION_OK;
	input->scan_busy = false;
	input->scan_complete = true;
}

int
main(int argc, char * argv[])
{
#if defined(S5_LIVE_HOST_TEST)
    if (2 == argc) { return test_live((uint32_t)strtoul(argv[1],NULL,10)); }
#else
    (void)argc; (void)argv;
#endif

	s5_runtime_t runtime;
	s5_runtime_input_t input;
	s5_runtime_events_t events;
	s5_runtime_config_t config;
	bool result;

	config = make_config();
	config.stop_timeout_ms = 0U;
	check_result("S5-U24",
		     s5_runtime_init(&runtime, &config) ==
			     S5_LOGIC_INVALID_ARGUMENT,
		     "runtime rejects missing timeout configuration");

	result = init_runtime(&runtime);
	make_input(&input, 100U, 100U, 20U);
	if (result)
	{
		input.forward_valid = false;
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_stop &&
			 events.request_fault &&
			 (runtime.status.fault ==
			  S5_RUNTIME_FAULT_FORWARD_SENSOR);
	}
	check_result("S5-U25", result,
		     "invalid forward sample requests stop and latches fault");

	result = init_runtime(&runtime);
	if (result)
	{
		make_input(&input, 100U, 100U, 20U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 !events.request_stop &&
			 (runtime.status.state == S5_RUNTIME_MONITOR);
		make_input(&input, 105U, 100U, 20U);
		result = result &&
			 (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 !events.request_stop &&
			 (runtime.status.state == S5_RUNTIME_MONITOR);
		make_input(&input, 110U, 110U, 20U);
		result = result &&
			 (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_stop &&
			 (runtime.status.state == S5_RUNTIME_WAIT_STOP);
	}
	check_result("S5-U26", result,
		     "distinct confirmed proximity samples request a stop");

	result = init_runtime(&runtime);
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_scan &&
				 (runtime.status.state == S5_RUNTIME_SCANNING);
		}
	}
	check_result("S5-U27", result,
		     "a full scan is requested only after stopped acknowledgement");

	result = init_runtime(&runtime);
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 210U, 210U, 20U);
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 events.request_fault &&
				 (runtime.status.fault ==
				  S5_RUNTIME_FAULT_STOP_TIMEOUT);
		}
	}
	check_result("S5-U28", result,
		     "missing stopped acknowledgement fails closed on timeout");

	result = init_runtime(&runtime);
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_scan;
			make_input(&input, 130U, 130U, 20U);
			input.vehicle_stopped = true;
			input.scan_cancelled = true;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 events.request_fault &&
				 (runtime.status.fault ==
				  S5_RUNTIME_FAULT_SCAN_CANCELLED);
		}
	}
	check_result("S5-U29", result,
		     "cancelled scan requests stop and latches fault");

	result = init_runtime(&runtime);
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_scan;
			make_input(&input, 130U, 130U, 20U);
			input.vehicle_stopped = false;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 events.request_fault &&
				 (runtime.status.fault ==
				  S5_RUNTIME_FAULT_MOVED_DURING_SCAN);
		}
	}
	check_result("S5-U30", result,
		     "vehicle movement during a full scan fails closed");

	result = init_runtime(&runtime);
	if (result)
	{
		config = make_config();
		config.scan_timeout_ms = 1000U;
		result = (S5_LOGIC_OK ==
			  s5_runtime_init(&runtime, &config));
	}
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_scan;
			make_input(&input, 1120U, 1120U, 20U);
			input.vehicle_stopped = true;
			input.scan_busy = true;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 events.request_fault &&
				 (runtime.status.fault ==
				  S5_RUNTIME_FAULT_SCAN_TIMEOUT);
		}
	}
	check_result("S5-U31", result,
		     "unfinished full scan fails closed at configured timeout");

	result = init_runtime(&runtime);
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_scan;
			make_input(&input, 130U, 130U, 20U);
			input.vehicle_stopped = true;
			input.scan_complete = true;
			input.acquisition_status =
				S5_ACQUISITION_ECHO_TIMEOUT;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 events.request_fault &&
				 (runtime.status.scan_status ==
				  S5_LOGIC_ACQUISITION_FAILED);
		}
	}
	check_result("S5-U32", result,
		     "echo acquisition failure cannot produce a profile or route");

	result = init_runtime(&runtime);
	if (result)
	{
		s5_runtime_status_t status;
		s5_scan_context_t context;
		s5_scan_sample_t coarse[S5_COARSE_SCAN_COUNT];
		s5_scan_sample_t fine[S5_FINE_SCAN_MAX_COUNT];

		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_scan;
			make_input(&input, 1620U, 1620U, 20U);
			input.vehicle_stopped = true;
			make_valid_scan(&input, &context, coarse, fine);
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 events.request_fault &&
				 !events.request_bypass &&
				 (runtime.status.fault ==
				  S5_RUNTIME_FAULT_NO_CLEARANCE) &&
				 runtime.status.scan_valid &&
				 !runtime.status.plan.executable;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_get_status(&runtime, &status)) &&
				 status.scan_complete &&
				 status.scan_valid &&
				 (status.coarse_count == S5_COARSE_SCAN_COUNT) &&
				 (status.fine_count == S5_FINE_SCAN_MAX_COUNT) &&
				 (status.coarse_samples[0].distance_cm == 100U) &&
				 (status.scan_completed_at_ms == 1520U);
		}
	}
	check_result("S5-U33", result,
		     "valid scan without measured clearance never emits bypass");

	result = init_runtime(&runtime);
	if (result)
	{
		s5_runtime_status_t status;

		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			(void)s5_runtime_step(&runtime, &input, &events);
			make_input(&input, 130U, 130U, 20U);
			input.vehicle_stopped = true;
			input.scan_complete = true;
			input.acquisition_status =
				S5_ACQUISITION_SERVO_POSITION_FAILED;
			(void)s5_runtime_step(&runtime, &input, &events);
			result = (S5_LOGIC_OK ==
				  s5_runtime_get_status(&runtime, &status)) &&
				 status.scan_complete &&
				 !status.scan_valid &&
				 (status.fault ==
				  S5_RUNTIME_FAULT_SCAN_INVALID) &&
				 (status.scan_status ==
				  S5_LOGIC_ACQUISITION_FAILED);
		}
	}
	check_result("S5-U34", result,
		     "status snapshot copies scan completion and acquisition fault");

	result = init_runtime(&runtime);
	make_input(&input, 100U, 100U, 20U);
	if (result)
	{
		input.forward_valid = false;
		(void)s5_runtime_step(&runtime, &input, &events);
		make_input(&input, 110U, 110U, 20U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_stop &&
			 events.request_fault &&
			 (runtime.status.state == S5_RUNTIME_FAULT);
	}
	check_result("S5-U35", result,
		     "latched fault keeps requesting stop and fault handling");

	result = init_runtime(&runtime);
	if (result)
	{
		result = (S5_LOGIC_INVALID_ARGUMENT ==
			  s5_runtime_step(&runtime, NULL, &events)) &&
			 events.request_stop &&
			 events.request_fault &&
			 (runtime.status.fault ==
			  S5_RUNTIME_FAULT_INVALID_INPUT);
	}
	check_result("S5-U36", result,
		     "invalid runtime input requests a safe stop and latches fault");

	result = init_runtime(&runtime);
	make_input(&input, 151U, 100U, 20U);
	if (result)
	{
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_stop &&
			 events.request_fault &&
			 (runtime.status.fault ==
			  S5_RUNTIME_FAULT_FORWARD_SENSOR);
	}
	check_result("S5-U37", result,
		     "stale forward sample cannot maintain motion monitoring");

	result = init_runtime(&runtime);
	if (result)
	{
		make_input(&input, 100U, 100U, 100U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events));
		make_input(&input, 99U, 99U, 100U);
		result = result &&
			 (S5_LOGIC_STALE_SCAN ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_stop &&
			 events.request_fault &&
			 (runtime.status.fault ==
			  S5_RUNTIME_FAULT_INVALID_INPUT);
	}
	check_result("S5-U38", result,
		     "backwards runtime clock fails closed");

	result = init_runtime(&runtime);
	if (result)
	{
		make_input(&input, UINT32_MAX - 5U, UINT32_MAX - 5U, 100U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events));
		make_input(&input, 4U, 4U, 100U);
		result = result &&
			 (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 (runtime.status.state == S5_RUNTIME_MONITOR) &&
			 !events.request_fault;
	}
	check_result("S5-U39", result,
		     "valid runtime timestamp wrap is accepted");

	result = init_runtime(&runtime);
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_scan;
			make_input(&input, 200U, 200U, 20U);
			input.vehicle_stopped = true;
			input.scan_busy = true;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 (runtime.status.state == S5_RUNTIME_SCANNING) &&
				 runtime.status.scan_busy &&
				 !events.request_fault;
			make_input(&input, 2120U, 2120U, 20U);
			input.vehicle_stopped = true;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 events.request_fault &&
				 (runtime.status.fault ==
				  S5_RUNTIME_FAULT_SCAN_TIMEOUT);
		}
	}
	check_result("S5-U40", result,
		     "scan startup grace is bounded by the full-scan timeout");

	result = init_runtime(&runtime);
	if (result)
	{
		make_input(&input, 10U, 10U, 100U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events));
		runtime.status.state = S5_RUNTIME_WAIT_BYPASS;
		runtime.status.busy = true;
		runtime.state_started_at_ms = 10U;
		make_input(&input, 20U, 20U, 100U);
		input.bypass_complete = true;
		result = result &&
			 (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_line_recovery &&
			 (runtime.status.state == S5_RUNTIME_LINE_RECOVERY);
	}
	check_result("S5-U41", result,
		     "bypass completion requests line recovery");

	result = init_runtime(&runtime);
	if (result)
	{
		make_input(&input, 10U, 10U, 100U);
		(void)s5_runtime_step(&runtime, &input, &events);
		runtime.status.state = S5_RUNTIME_WAIT_BYPASS;
		runtime.status.busy = true;
		runtime.state_started_at_ms = 10U;
		make_input(&input, 1010U, 1010U, 100U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_stop &&
			 events.request_fault &&
			 (runtime.status.fault ==
			  S5_RUNTIME_FAULT_BYPASS_TIMEOUT);
	}
	check_result("S5-U42", result,
		     "bypass acknowledgement timeout fails closed");

	result = init_runtime(&runtime);
	if (result)
	{
		make_input(&input, 10U, 10U, 100U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events));
		runtime.status.state = S5_RUNTIME_LINE_RECOVERY;
		runtime.status.busy = true;
		runtime.state_started_at_ms = 10U;
		make_input(&input, 20U, 20U, 100U);
		input.line_found = true;
		result = result &&
			 (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 (runtime.status.state == S5_RUNTIME_MONITOR) &&
			 !runtime.status.busy &&
			 !events.request_fault;
	}
	check_result("S5-U43", result,
		     "LINE_FOUND returns the policy to forward monitoring");

	result = init_runtime(&runtime);
	if (result)
	{
		make_input(&input, 10U, 10U, 100U);
		(void)s5_runtime_step(&runtime, &input, &events);
		runtime.status.state = S5_RUNTIME_LINE_RECOVERY;
		runtime.status.busy = true;
		runtime.state_started_at_ms = 10U;
		make_input(&input, 1010U, 1010U, 100U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_stop &&
			 events.request_fault &&
			 (runtime.status.fault ==
			  S5_RUNTIME_FAULT_LINE_TIMEOUT);
	}
	check_result("S5-U44", result,
		     "line reacquisition timeout fails closed");

	result = init_runtime(&runtime);
	if (result)
	{
		make_input(&input, 10U, 10U, 100U);
		(void)s5_runtime_step(&runtime, &input, &events);
		runtime.status.state = S5_RUNTIME_WAIT_BYPASS;
		runtime.status.busy = true;
		runtime.state_started_at_ms = 10U;
		make_input(&input, 20U, 20U, 20U);
		result = (S5_LOGIC_OK ==
			  s5_runtime_step(&runtime, &input, &events)) &&
			 events.request_stop &&
			 events.request_fault &&
			 (runtime.status.fault ==
			  S5_RUNTIME_FAULT_FORWARD_SENSOR);
	}
	check_result("S5-U45", result,
		     "renewed forward obstacle during bypass fails closed");

	result = init_runtime(&runtime);
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 !events.request_scan &&
				 (runtime.status.state == S5_RUNTIME_WAIT_STOP);
			make_input(&input, 125U, 125U, 20U);
			input.vehicle_stopped = true;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 events.request_scan &&
				 (runtime.status.state == S5_RUNTIME_SCANNING);
		}
	}
	check_result("S5-U46", result,
		     "stop request remains asserted until stopped acknowledgement");

	result = init_runtime(&runtime);
	if (result)
	{
		result = trigger_stop(&runtime, &input);
		if (result)
		{
			make_input(&input, 120U, 120U, 20U);
			input.vehicle_stopped = true;
			result = (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_scan;
			make_input(&input, 125U, 125U, 20U);
			input.vehicle_stopped = true;
			input.scan_busy = true;
			result = result &&
				 (S5_LOGIC_OK ==
				  s5_runtime_step(&runtime, &input, &events)) &&
				 events.request_stop &&
				 !events.request_fault &&
				 (runtime.status.state == S5_RUNTIME_SCANNING);
		}
	}
	check_result("S5-U47", result,
		     "stop request remains asserted throughout stationary scanning");

	test_acquisition();
    test_hold();
	(void)printf("[TEST] subsystem5 runtime failures: %u\n",
		     s5_test_failures);
	return (s5_test_failures == 0U) ? 0 : 1;
}


static void test_hold (void)
{
    s5_runtime_t runtime = {0};
    s5_runtime_input_t input = {0};
    s5_runtime_events_t events = {0};
    s5_scan_context_t context = {0};
    s5_scan_sample_t coarse[S5_COARSE_SCAN_COUNT] = {{0}};
    s5_scan_sample_t fine[S5_FINE_SCAN_MAX_COUNT] = {{0}};
    bool result = init_runtime(&runtime);

    runtime.status.state = S5_RUNTIME_SCANNING;
    runtime.status.scan_started_at_ms = 120U;
    runtime.state_started_at_ms = 120U;
    make_input(&input,1620U,1620U,100U);
    make_valid_scan(&input,&context,coarse,fine);
    input.vehicle_stopped = true;
    input.scan_complete = true;
    input.controller_owned = true;
    result = result && (S5_LOGIC_OK == s5_runtime_step(&runtime,&input,&events)) &&
        (S5_RUNTIME_HOLD == runtime.status.state) &&
        runtime.status.scan_valid && !runtime.status.plan.executable &&
        events.request_stop && !events.request_fault;
    check_result("S5-U83",result,
        "controller-owned valid scan holds real profile without invented clearance");
    make_input(&input,1630U,1630U,100U);
    input.controller_owned = true;
    input.line_found = true;
    result = (S5_LOGIC_OK == s5_runtime_step(&runtime,&input,&events)) &&
        (S5_RUNTIME_MONITOR == runtime.status.state) &&
        !runtime.status.scan_valid && !runtime.status.scan_complete &&
        !runtime.status.busy;
    check_result("S5-U84",result,
        "correlated recovery input rearms monitoring without stale scan");
    runtime.status.state = S5_RUNTIME_HOLD;
    runtime.status.scan_completed_at_ms = 1600U;
    make_input(&input,1800U,1600U,100U);
    input.controller_owned = true;
    result = (S5_LOGIC_OK == s5_runtime_step(&runtime,&input,&events)) &&
        (S5_RUNTIME_FAULT == runtime.status.state) && events.request_fault;
    check_result("S5-U85",result,"held scan still faults on missing forward sensing");
}
/*** end of file ***/
