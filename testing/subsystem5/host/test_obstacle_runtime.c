#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
main(void)
{
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

	(void)printf("[TEST] subsystem5 runtime failures: %u\n",
		     s5_test_failures);
	return (s5_test_failures == 0U) ? 0 : 1;
}
