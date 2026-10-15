#include "s5_obstacle_runtime.h"

#include <string.h>

static bool
s5_runtime_config_is_valid(const s5_runtime_config_t *config)
{
	return (config != NULL) &&
	       (config->forward_sample_max_age_ms > 0U) &&
	       (config->forward_sample_max_age_ms <= S5_RUNTIME_MAX_TIMEOUT_MS) &&
	       (config->acknowledgement_max_age_ms > 0U) &&
	       (config->acknowledgement_max_age_ms <=
		S5_RUNTIME_MAX_TIMEOUT_MS) &&
	       (config->stop_timeout_ms > 0U) &&
	       (config->stop_timeout_ms <= S5_RUNTIME_MAX_TIMEOUT_MS) &&
	       (config->scan_timeout_ms > 0U) &&
	       (config->scan_timeout_ms <= S5_RUNTIME_MAX_TIMEOUT_MS) &&
	       (config->bypass_timeout_ms > 0U) &&
	       (config->bypass_timeout_ms <= S5_RUNTIME_MAX_TIMEOUT_MS) &&
	       (config->line_timeout_ms > 0U) &&
	       (config->line_timeout_ms <= S5_RUNTIME_MAX_TIMEOUT_MS) &&
	       (config->obstacle_threshold_cm >= S5_MIN_VALID_DISTANCE_CM) &&
	       (config->obstacle_threshold_cm <= S5_MAX_VALID_DISTANCE_CM) &&
	       (config->confirmation_count > 0U);
}

static bool
s5_runtime_forward_is_fresh(
	const s5_runtime_t *runtime,
	const s5_runtime_input_t *input)
{
	uint32_t age_ms;

	if (!input->forward_valid ||
	    (input->forward_distance_cm < S5_MIN_VALID_DISTANCE_CM) ||
	    (input->forward_distance_cm > S5_MAX_VALID_DISTANCE_CM))
	{
		return false;
	}

	age_ms = input->now_ms - input->forward_sample_time_ms;
	return (age_ms <= runtime->config.forward_sample_max_age_ms);
}

static bool
s5_runtime_timestamp_is_fresh_after(
	uint32_t now_ms,
	uint32_t timestamp_ms,
	uint32_t reference_ms,
	uint32_t max_age_ms,
	uint32_t max_elapsed_ms)
{
	uint32_t elapsed_since_reference_ms;
	uint32_t age_ms;

	elapsed_since_reference_ms = timestamp_ms - reference_ms;
	age_ms = now_ms - timestamp_ms;
	return (elapsed_since_reference_ms > 0U) &&
	       (elapsed_since_reference_ms < max_elapsed_ms) &&
	       (age_ms <= max_age_ms);
}

static void
s5_runtime_set_fault(
	s5_runtime_t *runtime,
	s5_runtime_fault_t fault,
	s5_runtime_events_t *events)
{
	runtime->status.state = S5_RUNTIME_FAULT;
	runtime->status.fault = fault;
	runtime->status.busy = false;
	runtime->status.scan_busy = false;
	if ((fault == S5_RUNTIME_FAULT_INVALID_INPUT) ||
	    (fault == S5_RUNTIME_FAULT_FORWARD_SENSOR))
	{
		runtime->status.forward_valid = false;
	}
	events->request_stop = true;
	events->request_fault = true;
}

static bool
s5_runtime_timeout_elapsed(
	const s5_runtime_t *runtime,
	uint32_t now_ms,
	uint32_t timeout_ms)
{
	return ((uint32_t)(now_ms - runtime->state_started_at_ms) >=
		timeout_ms);
}

static bool
s5_runtime_copy_scan(
	s5_runtime_t *runtime,
	const s5_runtime_input_t *input)
{
	s5_obstacle_profile_t profile;
	s5_obstacle_plan_t plan;
	s5_logic_status_t result;
	uint32_t since_scan_request_ms;
	uint32_t since_scan_evaluation_ms;
	uint32_t since_scan_completion_ms;

	if ((input->scan_context == NULL) ||
	    (input->coarse_samples == NULL) ||
	    (input->fine_samples == NULL) ||
	    (input->coarse_count != S5_COARSE_SCAN_COUNT) ||
	    (input->fine_count > S5_FINE_SCAN_MAX_COUNT))
	{
		runtime->status.scan_status = S5_LOGIC_INVALID_ARGUMENT;
		return false;
	}

	runtime->status.scan_context = *input->scan_context;
	runtime->status.acquisition_status =
		input->scan_context->acquisition_status;
	since_scan_request_ms =
		runtime->status.scan_context.started_at_ms -
		runtime->status.scan_started_at_ms;
	since_scan_evaluation_ms =
		input->now_ms - runtime->status.scan_context.evaluated_at_ms;
	since_scan_completion_ms =
		input->now_ms - runtime->status.scan_context.completed_at_ms;
	if ((since_scan_request_ms > S5_RUNTIME_MAX_TIMEOUT_MS) ||
	    (since_scan_evaluation_ms > S5_RUNTIME_MAX_TIMEOUT_MS) ||
	    (since_scan_evaluation_ms >
	     runtime->status.scan_context.max_age_ms) ||
	    (since_scan_completion_ms > S5_RUNTIME_MAX_TIMEOUT_MS) ||
	    (since_scan_completion_ms >
	     runtime->status.scan_context.max_age_ms))
	{
		runtime->status.scan_status = S5_LOGIC_STALE_SCAN;
		runtime->status.scan_complete = true;
		return false;
	}

	memcpy(
		runtime->status.coarse_samples,
		input->coarse_samples,
		sizeof(runtime->status.coarse_samples));
	memcpy(
		runtime->status.fine_samples,
		input->fine_samples,
		input->fine_count * sizeof(runtime->status.fine_samples[0]));
	runtime->status.coarse_count = input->coarse_count;
	runtime->status.fine_count = input->fine_count;
	runtime->status.scan_completed_at_ms =
		input->scan_context->completed_at_ms;

	result = s5_build_obstacle_profile(
		&runtime->status.scan_context,
		runtime->status.coarse_samples,
		runtime->status.coarse_count,
		runtime->status.fine_samples,
		runtime->status.fine_count,
		&profile);
	runtime->status.scan_status = result;
	runtime->status.scan_complete = true;
	runtime->status.scan_valid = (result == S5_LOGIC_OK);
	if (result != S5_LOGIC_OK)
	{
		return false;
	}

	runtime->status.profile = profile;
	result = s5_build_obstacle_plan(&profile, &plan);
	runtime->status.scan_status = result;
	runtime->status.plan = plan;

	return (result == S5_LOGIC_OK) &&
	       plan.executable &&
	       (plan.action_count > 0U) &&
	       ((plan.bypass_direction == S5_BYPASS_LEFT_CANDIDATE) ||
		(plan.bypass_direction == S5_BYPASS_RIGHT_CANDIDATE));
}

s5_logic_status_t
s5_runtime_init(
	s5_runtime_t *runtime,
	const s5_runtime_config_t *config)
{
	if ((runtime == NULL) || !s5_runtime_config_is_valid(config))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	memset(runtime, 0, sizeof(*runtime));
	runtime->config = *config;
	runtime->status.state = S5_RUNTIME_MONITOR;
	runtime->status.scan_status = S5_LOGIC_INVALID_SCAN;
	runtime->initialised = true;
	return S5_LOGIC_OK;
}

s5_logic_status_t
s5_runtime_step(
	s5_runtime_t *runtime,
	const s5_runtime_input_t *input,
	s5_runtime_events_t *events)
{
	uint32_t update_interval_ms;

	if ((runtime == NULL) || (events == NULL))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	memset(events, 0, sizeof(*events));
	if (!runtime->initialised)
	{
		events->request_stop = true;
		events->request_fault = true;
		return S5_LOGIC_INVALID_ARGUMENT;
	}
	if (input == NULL)
	{
		s5_runtime_set_fault(
			runtime,
			S5_RUNTIME_FAULT_INVALID_INPUT,
			events);
		return S5_LOGIC_INVALID_ARGUMENT;
	}
	if (runtime->have_update)
	{
		update_interval_ms =
			input->now_ms - runtime->status.updated_at_ms;
		if (update_interval_ms > S5_RUNTIME_MAX_TIMEOUT_MS)
		{
			runtime->status.updated_at_ms = input->now_ms;
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_INVALID_INPUT,
				events);
			return S5_LOGIC_STALE_SCAN;
		}
	}
	runtime->status.updated_at_ms = input->now_ms;
	runtime->have_update = true;

	if (runtime->status.state == S5_RUNTIME_FAULT)
	{
		events->request_stop = true;
		events->request_fault = true;
		return S5_LOGIC_OK;
	}

	if ((runtime->status.state == S5_RUNTIME_MONITOR) ||
	    (runtime->status.state == S5_RUNTIME_WAIT_BYPASS) ||
	    (runtime->status.state == S5_RUNTIME_LINE_RECOVERY))
	{
		if (!s5_runtime_forward_is_fresh(runtime, input))
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_FORWARD_SENSOR,
				events);
			return S5_LOGIC_OK;
		}

		runtime->status.forward_distance_cm =
			input->forward_distance_cm;
		runtime->status.forward_sample_time_ms =
			input->forward_sample_time_ms;
		runtime->status.forward_valid = true;
		if (runtime->have_forward_sample &&
		    (input->forward_sample_time_ms !=
		     runtime->last_forward_sample_time_ms) &&
		    ((input->forward_sample_time_ms -
		      runtime->last_forward_sample_time_ms) >
		     S5_RUNTIME_MAX_TIMEOUT_MS))
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_FORWARD_SENSOR,
				events);
			return S5_LOGIC_OK;
		}
	}

	switch (runtime->status.state)
	{
	case S5_RUNTIME_MONITOR:
		if (!runtime->have_forward_sample ||
		    (input->forward_sample_time_ms !=
		     runtime->last_forward_sample_time_ms))
		{
			runtime->last_forward_sample_time_ms =
				input->forward_sample_time_ms;
			runtime->have_forward_sample = true;

			if (input->forward_distance_cm <=
			    runtime->config.obstacle_threshold_cm)
			{
				if (runtime->obstacle_confirmations <
				    runtime->config.confirmation_count)
				{
					runtime->obstacle_confirmations++;
				}
			}
			else
			{
				runtime->obstacle_confirmations = 0U;
			}
		}

		if (runtime->have_forward_sample &&
		    (runtime->obstacle_confirmations >=
		     runtime->config.confirmation_count))
		{
			runtime->status.state = S5_RUNTIME_WAIT_STOP;
			runtime->status.busy = true;
			runtime->state_started_at_ms = input->now_ms;
			runtime->status.scan_complete = false;
			runtime->status.scan_busy = false;
			runtime->status.scan_valid = false;
			runtime->status.scan_started_at_ms = 0U;
			runtime->status.scan_completed_at_ms = 0U;
			runtime->status.acquisition_status = S5_ACQUISITION_OK;
			runtime->status.fault = S5_RUNTIME_FAULT_NONE;
			runtime->status.scan_status = S5_LOGIC_INVALID_SCAN;
			memset(&runtime->status.scan_context, 0,
			       sizeof(runtime->status.scan_context));
			memset(runtime->status.coarse_samples, 0,
			       sizeof(runtime->status.coarse_samples));
			memset(runtime->status.fine_samples, 0,
			       sizeof(runtime->status.fine_samples));
			memset(&runtime->status.profile, 0,
			       sizeof(runtime->status.profile));
			memset(&runtime->status.plan, 0,
			       sizeof(runtime->status.plan));
			runtime->status.coarse_count = 0U;
			runtime->status.fine_count = 0U;
			runtime->status.stop_ack_time_ms = 0U;
			runtime->status.bypass_completed_at_ms = 0U;
			runtime->status.line_found_at_ms = 0U;
			events->request_stop = true;
		}
		break;

	case S5_RUNTIME_WAIT_STOP:
		events->request_stop = true;
		if (input->vehicle_stopped &&
		    s5_runtime_timestamp_is_fresh_after(
			    input->now_ms,
			    input->vehicle_stopped_sample_time_ms,
			    runtime->state_started_at_ms,
			    runtime->config.acknowledgement_max_age_ms,
			    runtime->config.stop_timeout_ms))
		{
			runtime->status.stop_ack_time_ms =
				input->vehicle_stopped_sample_time_ms;
			runtime->status.state = S5_RUNTIME_SCANNING;
			runtime->state_started_at_ms = input->now_ms;
			runtime->status.scan_started_at_ms = input->now_ms;
			events->request_scan = true;
		}
		else if (s5_runtime_timeout_elapsed(
				 runtime,
				 input->now_ms,
				 runtime->config.stop_timeout_ms))
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_STOP_TIMEOUT,
				events);
		}
		break;

	case S5_RUNTIME_SCANNING:
		events->request_stop = true;
		events->request_scan =
			!input->scan_busy && !input->scan_complete;
		runtime->status.scan_busy = input->scan_busy;
		if (s5_runtime_timeout_elapsed(
			    runtime,
			    input->now_ms,
			    runtime->config.scan_timeout_ms))
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_SCAN_TIMEOUT,
				events);
		}
		else if (!s5_runtime_timestamp_is_fresh_after(
			    input->now_ms,
			    input->vehicle_stopped_sample_time_ms,
			    runtime->state_started_at_ms,
			    runtime->config.acknowledgement_max_age_ms,
			    runtime->config.scan_timeout_ms))
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_STOP_STATUS_STALE,
				events);
		}
		else if (!input->vehicle_stopped)
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_MOVED_DURING_SCAN,
				events);
		}
		else if (input->scan_cancelled)
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_SCAN_CANCELLED,
				events);
		}
		else if (input->scan_complete)
		{
			runtime->status.scan_busy = false;
			runtime->status.scan_complete = true;
			runtime->status.acquisition_status =
				input->acquisition_status;
			if (input->acquisition_status != S5_ACQUISITION_OK)
			{
				runtime->status.scan_status =
					S5_LOGIC_ACQUISITION_FAILED;
				runtime->status.scan_complete = true;
				s5_runtime_set_fault(
					runtime,
					S5_RUNTIME_FAULT_SCAN_INVALID,
					events);
			}
			else if (!s5_runtime_copy_scan(runtime, input))
			{
				s5_runtime_set_fault(
					runtime,
					runtime->status.scan_valid
						? S5_RUNTIME_FAULT_NO_CLEARANCE
						: S5_RUNTIME_FAULT_SCAN_INVALID,
					events);
			}
			else
			{
				runtime->status.state = S5_RUNTIME_WAIT_BYPASS;
				runtime->state_started_at_ms = input->now_ms;
				runtime->status.busy = true;
				events->request_bypass = true;
			}
		}
		break;

	case S5_RUNTIME_WAIT_BYPASS:
		if (input->forward_sample_time_ms !=
		    runtime->last_forward_sample_time_ms)
		{
			runtime->last_forward_sample_time_ms =
				input->forward_sample_time_ms;
			runtime->have_forward_sample = true;
		}
		if (input->forward_distance_cm <=
		    runtime->config.obstacle_threshold_cm)
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_FORWARD_SENSOR,
				events);
		}
		else if (input->bypass_complete &&
			 s5_runtime_timestamp_is_fresh_after(
				 input->now_ms,
				 input->bypass_complete_time_ms,
				 runtime->state_started_at_ms,
				 runtime->config.acknowledgement_max_age_ms,
				 runtime->config.bypass_timeout_ms))
		{
			runtime->status.bypass_completed_at_ms =
				input->bypass_complete_time_ms;
			runtime->status.state = S5_RUNTIME_LINE_RECOVERY;
			runtime->state_started_at_ms = input->now_ms;
			events->request_line_recovery = true;
		}
		else if (s5_runtime_timeout_elapsed(
				 runtime,
				 input->now_ms,
				 runtime->config.bypass_timeout_ms))
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_BYPASS_TIMEOUT,
				events);
		}
		else
		{
			events->request_bypass = true;
		}
		break;

	case S5_RUNTIME_LINE_RECOVERY:
		if (input->forward_sample_time_ms !=
		    runtime->last_forward_sample_time_ms)
		{
			runtime->last_forward_sample_time_ms =
				input->forward_sample_time_ms;
			runtime->have_forward_sample = true;
		}
		if (input->forward_distance_cm <=
		    runtime->config.obstacle_threshold_cm)
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_FORWARD_SENSOR,
				events);
		}
		else if (input->line_found &&
			 s5_runtime_timestamp_is_fresh_after(
				 input->now_ms,
				 input->line_found_time_ms,
				 runtime->state_started_at_ms,
				 runtime->config.acknowledgement_max_age_ms,
				 runtime->config.line_timeout_ms))
		{
			runtime->status.line_found_at_ms =
				input->line_found_time_ms;
			runtime->status.state = S5_RUNTIME_MONITOR;
			runtime->status.busy = false;
			runtime->state_started_at_ms = input->now_ms;
			runtime->obstacle_confirmations = 0U;
			runtime->have_forward_sample = false;
		}
		else if (s5_runtime_timeout_elapsed(
				 runtime,
				 input->now_ms,
				 runtime->config.line_timeout_ms))
		{
			s5_runtime_set_fault(
				runtime,
				S5_RUNTIME_FAULT_LINE_TIMEOUT,
				events);
		}
		else
		{
			events->request_line_recovery = true;
		}
		break;

	case S5_RUNTIME_FAULT:
	default:
		s5_runtime_set_fault(
			runtime,
			S5_RUNTIME_FAULT_INVALID_INPUT,
			events);
		return S5_LOGIC_INVALID_SCAN;
	}

	return S5_LOGIC_OK;
}

s5_logic_status_t
s5_runtime_get_status(
	const s5_runtime_t *runtime,
	s5_runtime_status_t *status)
{
	if ((runtime == NULL) || (status == NULL) ||
	    !runtime->initialised)
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	*status = runtime->status;
	return S5_LOGIC_OK;
}
