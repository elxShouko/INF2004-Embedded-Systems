#include "s5_obstacle_logic.h"

#include <string.h>

#define S5_SCAN_MIN_ANGLE_DEG		30U
#define S5_SCAN_MAX_ANGLE_DEG		150U
#define S5_SCAN_CENTRE_ANGLE_DEG	90U
#define S5_PROFILE_MAX_ANGLE_DIFFERENCE_DEG	\
	(S5_SCAN_CENTRE_ANGLE_DEG - S5_SCAN_MIN_ANGLE_DEG)
#define S5_SCAN_ANGLE_STEP_DEG		5U
#define S5_COARSE_SCAN_STEP_DEG		30U
#define S5_FINE_SCAN_HALF_SPAN_DEG	20U
#define S5_MIN_VALID_FINE_SAMPLES	3U
#define S5_MIN_VALID_SIDE_SAMPLES	2U
#define S5_MAX_SCAN_DURATION_MS		0x7fffffffU

/* Development defaults; tune only from repeatable hardware measurements. */
#define S5_DEV_MIN_RANGE_FOR_BYPASS_CM	25U
#define S5_DEV_REVERSE_DISTANCE_CM	10U
#define S5_DEV_PROFILE_EDGE_MARGIN_CM	15U

#define S5_Q15_ONE			32767U
#define S5_Q15_HALF			(S5_Q15_ONE / 2U)
#define S5_Q1000_HALF			500U
#define S5_Q1000_ONE			1000U

static const uint16_t s5_sine_q15[] =
{
	0U, 2856U, 5690U, 8481U, 11207U, 13848U, 16383U,
	18794U, 21062U, 23170U, 25101U, 26841U, 28377U
};

static const uint16_t s5_cosine_q15[] =
{
	32767U, 32642U, 32269U, 31650U, 30791U, 29697U, 28377U,
	26841U, 25101U, 23170U, 21062U, 18794U, 16384U
};

static const uint16_t s5_tangent_half_span_q1000[] =
{
	0U, 44U, 87U, 132U, 176U, 222U, 268U, 315U, 364U
};

static bool
s5_is_distance_valid(uint16_t distance_cm)
{
	return (distance_cm >= S5_MIN_VALID_DISTANCE_CM) &&
	       (distance_cm <= S5_MAX_VALID_DISTANCE_CM);
}

static bool
s5_coarse_samples_are_valid(
	const s5_scan_sample_t *samples,
	size_t sample_count)
{
	uint8_t expected_angle;

	if ((samples == NULL) ||
	    (sample_count != S5_COARSE_SCAN_COUNT))
	{
		return false;
	}

	for (size_t index = 0U; index < sample_count; index++)
	{
		expected_angle = (uint8_t)(
			S5_SCAN_MIN_ANGLE_DEG +
			(index * S5_COARSE_SCAN_STEP_DEG));
		if ((samples[index].angle_deg != expected_angle) ||
		    (samples[index].angle_deg > S5_SCAN_MAX_ANGLE_DEG) ||
		    (samples[index].valid &&
		     !s5_is_distance_valid(samples[index].distance_cm)))
		{
			return false;
		}
	}

	return true;
}

static bool
s5_fine_samples_are_valid(
	const s5_scan_sample_t *samples,
	size_t sample_count)
{
	int32_t angle_step;

	if ((samples == NULL) ||
	    (sample_count < S5_MIN_VALID_FINE_SAMPLES) ||
	    (sample_count > S5_FINE_SCAN_MAX_COUNT))
	{
		return false;
	}

	for (size_t index = 0U; index < sample_count; index++)
	{
		if ((samples[index].angle_deg < S5_SCAN_MIN_ANGLE_DEG) ||
		    (samples[index].angle_deg > S5_SCAN_MAX_ANGLE_DEG) ||
		    ((samples[index].angle_deg % S5_SCAN_ANGLE_STEP_DEG) != 0U) ||
		    (samples[index].valid &&
		     !s5_is_distance_valid(samples[index].distance_cm)))
		{
			return false;
		}

		if (index > 0U)
		{
			angle_step =
				(int32_t)samples[index].angle_deg -
				(int32_t)samples[index - 1U].angle_deg;
			if (angle_step != (int32_t)S5_SCAN_ANGLE_STEP_DEG)
			{
				return false;
			}
		}
	}

	return true;
}

static bool
s5_sample_times_are_valid(
	const s5_scan_context_t *scan_context,
	const s5_scan_sample_t *coarse_samples,
	const s5_scan_sample_t *fine_samples,
	size_t fine_count,
	uint32_t scan_duration_ms)
{
	uint32_t previous_elapsed_ms;
	uint32_t sample_elapsed_ms;
	bool have_previous_sample;

	previous_elapsed_ms = 0U;
	have_previous_sample = false;

	for (size_t index = 0U; index < S5_COARSE_SCAN_COUNT; index++)
	{
		sample_elapsed_ms =
			coarse_samples[index].captured_at_ms -
			scan_context->started_at_ms;
		if (sample_elapsed_ms > scan_duration_ms ||
		    (have_previous_sample &&
		     (sample_elapsed_ms <= previous_elapsed_ms)))
		{
			return false;
		}
		previous_elapsed_ms = sample_elapsed_ms;
		have_previous_sample = true;
	}

	for (size_t index = 0U; index < fine_count; index++)
	{
		sample_elapsed_ms =
			fine_samples[index].captured_at_ms -
			scan_context->started_at_ms;
		if (sample_elapsed_ms > scan_duration_ms ||
		    (have_previous_sample &&
		     (sample_elapsed_ms <= previous_elapsed_ms)))
		{
			return false;
		}
		previous_elapsed_ms = sample_elapsed_ms;
		have_previous_sample = true;
	}

	return true;
}

static bool
s5_sample_ranges_are_consistent(
	const s5_scan_sample_t *samples,
	size_t sample_count,
	uint16_t max_adjacent_range_delta_cm)
{
	uint16_t previous_distance_cm;
	bool have_previous_valid_sample;

	previous_distance_cm = 0U;
	have_previous_valid_sample = false;

	for (size_t index = 0U; index < sample_count; index++)
	{
		uint16_t range_delta_cm;

		if (!samples[index].valid)
		{
			have_previous_valid_sample = false;
			continue;
		}

		if (have_previous_valid_sample)
		{
			range_delta_cm =
				(samples[index].distance_cm >= previous_distance_cm)
					? (uint16_t)(samples[index].distance_cm -
						     previous_distance_cm)
					: (uint16_t)(previous_distance_cm -
						     samples[index].distance_cm);
			if (range_delta_cm > max_adjacent_range_delta_cm)
			{
				return false;
			}
		}

		previous_distance_cm = samples[index].distance_cm;
		have_previous_valid_sample = true;
	}

	return true;
}

static s5_logic_status_t
s5_validate_scan_context(
	const s5_scan_context_t *scan_context,
	const s5_scan_sample_t *coarse_samples,
	const s5_scan_sample_t *fine_samples,
	size_t fine_count)
{
	uint32_t scan_duration_ms;
	uint32_t scan_age_ms;

	if (scan_context->cancelled)
	{
		return S5_LOGIC_SCAN_CANCELLED;
	}
	if ((scan_context->acquisition_status != S5_ACQUISITION_OK) ||
	    !scan_context->servo_settled)
	{
		return S5_LOGIC_ACQUISITION_FAILED;
	}
	if (!scan_context->complete)
	{
		return S5_LOGIC_INCOMPLETE_SCAN;
	}
	if (!scan_context->vehicle_stationary)
	{
		return S5_LOGIC_SCAN_NOT_STATIONARY;
	}
	if ((scan_context->max_duration_ms == 0U) ||
	    (scan_context->max_duration_ms > S5_MAX_SCAN_DURATION_MS) ||
	    (scan_context->max_age_ms == 0U) ||
	    (scan_context->max_age_ms > S5_MAX_SCAN_DURATION_MS) ||
	    (scan_context->max_adjacent_range_delta_cm == 0U))
	{
		return S5_LOGIC_INVALID_SCAN;
	}

	scan_duration_ms =
		scan_context->completed_at_ms - scan_context->started_at_ms;
	scan_age_ms =
		scan_context->evaluated_at_ms - scan_context->completed_at_ms;
	if ((scan_duration_ms > scan_context->max_duration_ms) ||
	    (scan_age_ms > scan_context->max_age_ms) ||
	    !s5_sample_times_are_valid(
		    scan_context,
		    coarse_samples,
		    fine_samples,
		    fine_count,
		    scan_duration_ms))
	{
		return S5_LOGIC_STALE_SCAN;
	}

	if (!s5_sample_ranges_are_consistent(
		    coarse_samples,
		    S5_COARSE_SCAN_COUNT,
		    scan_context->max_adjacent_range_delta_cm) ||
	    !s5_sample_ranges_are_consistent(
		    fine_samples,
		    fine_count,
		    scan_context->max_adjacent_range_delta_cm))
	{
		return S5_LOGIC_INCONSISTENT_SCAN;
	}

	return S5_LOGIC_OK;
}

static uint16_t
s5_project_distance(
	uint16_t distance_cm,
	uint16_t trig_q15)
{
	uint32_t product;

	product = (uint32_t)distance_cm * (uint32_t)trig_q15;
	return (uint16_t)((product + S5_Q15_HALF) / S5_Q15_ONE);
}

static bool
s5_profile_coordinates(
	uint8_t angle_deg,
	uint16_t distance_cm,
	int16_t *lateral_offset_cm,
	uint16_t *forward_distance_cm)
{
	uint8_t angle_difference;
	size_t table_index;
	uint16_t lateral_magnitude_cm;

	if ((lateral_offset_cm == NULL) ||
	    (forward_distance_cm == NULL) ||
	    (angle_deg < S5_SCAN_MIN_ANGLE_DEG) ||
	    (angle_deg > S5_SCAN_MAX_ANGLE_DEG))
	{
		return false;
	}

	angle_difference = (angle_deg >= S5_SCAN_CENTRE_ANGLE_DEG)
		? (uint8_t)(angle_deg - S5_SCAN_CENTRE_ANGLE_DEG)
		: (uint8_t)(S5_SCAN_CENTRE_ANGLE_DEG - angle_deg);

	if ((angle_difference > S5_PROFILE_MAX_ANGLE_DIFFERENCE_DEG) ||
	    ((angle_difference % S5_SCAN_ANGLE_STEP_DEG) != 0U))
	{
		return false;
	}

	table_index = angle_difference / S5_SCAN_ANGLE_STEP_DEG;
	lateral_magnitude_cm = s5_project_distance(
		distance_cm, s5_sine_q15[table_index]);

	if (angle_deg < S5_SCAN_CENTRE_ANGLE_DEG)
	{
		*lateral_offset_cm = -(int16_t)lateral_magnitude_cm;
	}
	else
	{
		*lateral_offset_cm = (int16_t)lateral_magnitude_cm;
	}

	*forward_distance_cm = s5_project_distance(
		distance_cm, s5_cosine_q15[table_index]);
	return true;
}

static void
s5_set_bypass_direction(s5_obstacle_profile_t *profile)
{
	bool left_range_passes;
	bool right_range_passes;

	left_range_passes =
		(profile->left_valid_samples >= S5_MIN_VALID_SIDE_SAMPLES) &&
		(profile->left_min_range_cm >= S5_DEV_MIN_RANGE_FOR_BYPASS_CM);
	right_range_passes =
		(profile->right_valid_samples >= S5_MIN_VALID_SIDE_SAMPLES) &&
		(profile->right_min_range_cm >= S5_DEV_MIN_RANGE_FOR_BYPASS_CM);

	if (left_range_passes && right_range_passes)
	{
		profile->bypass_direction =
			(profile->left_min_range_cm >= profile->right_min_range_cm)
				? S5_BYPASS_LEFT_CANDIDATE
				: S5_BYPASS_RIGHT_CANDIDATE;
	}
	else if (left_range_passes)
	{
		profile->bypass_direction = S5_BYPASS_LEFT_CANDIDATE;
	}
	else if (right_range_passes)
	{
		profile->bypass_direction = S5_BYPASS_RIGHT_CANDIDATE;
	}
	else
	{
		profile->bypass_direction = S5_BYPASS_NO_CANDIDATE;
	}
}

s5_logic_status_t
s5_make_coarse_scan_angles(
	uint8_t *angles,
	size_t angle_capacity)
{
	if ((angles == NULL) ||
	    (angle_capacity < S5_COARSE_SCAN_COUNT))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	for (size_t index = 0U; index < S5_COARSE_SCAN_COUNT; index++)
	{
		angles[index] = (uint8_t)(
			S5_SCAN_MIN_ANGLE_DEG +
			(index * S5_COARSE_SCAN_STEP_DEG));
	}

	return S5_LOGIC_OK;
}

s5_logic_status_t
s5_make_fine_scan_angles(
	uint8_t centre_angle_deg,
	uint8_t *angles,
	size_t angle_capacity,
	size_t *angle_count)
{
	uint8_t start_angle;
	uint8_t end_angle;
	size_t required_count;

	if (angle_count == NULL)
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	*angle_count = 0U;
	if (angles == NULL)
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	if ((centre_angle_deg < S5_SCAN_MIN_ANGLE_DEG) ||
	    (centre_angle_deg > S5_SCAN_MAX_ANGLE_DEG) ||
	    ((centre_angle_deg % S5_SCAN_ANGLE_STEP_DEG) != 0U))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	start_angle = (centre_angle_deg > S5_FINE_SCAN_HALF_SPAN_DEG)
		? (uint8_t)(centre_angle_deg - S5_FINE_SCAN_HALF_SPAN_DEG)
		: S5_SCAN_MIN_ANGLE_DEG;
	if (start_angle < S5_SCAN_MIN_ANGLE_DEG)
	{
		start_angle = S5_SCAN_MIN_ANGLE_DEG;
	}

	end_angle = (uint8_t)(centre_angle_deg + S5_FINE_SCAN_HALF_SPAN_DEG);
	if (end_angle > S5_SCAN_MAX_ANGLE_DEG)
	{
		end_angle = S5_SCAN_MAX_ANGLE_DEG;
	}

	required_count =
		((size_t)(end_angle - start_angle) / S5_SCAN_ANGLE_STEP_DEG) + 1U;
	if (angle_capacity < required_count)
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	for (size_t index = 0U; index < required_count; index++)
	{
		angles[index] = (uint8_t)(
			start_angle + (index * S5_SCAN_ANGLE_STEP_DEG));
	}

	*angle_count = required_count;
	return S5_LOGIC_OK;
}

s5_logic_status_t
s5_build_obstacle_profile(
	const s5_scan_context_t *scan_context,
	const s5_scan_sample_t *coarse_samples,
	size_t coarse_count,
	const s5_scan_sample_t *fine_samples,
	size_t fine_count,
	s5_obstacle_profile_t *profile)
{
	uint32_t left_minimum_cm;
	uint32_t right_minimum_cm;
	uint32_t closest_distance_cm;
	uint32_t width_cm;
	uint32_t width_span_deg;
	uint32_t tangent_index;
	size_t closest_index;
	size_t first_edge_index;
	size_t last_edge_index;
	uint8_t valid_fine_samples;
	s5_logic_status_t context_status;

	if (profile == NULL)
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}
	memset(profile, 0, sizeof(*profile));

	if ((scan_context == NULL) ||
	    (coarse_samples == NULL) ||
	    (fine_samples == NULL))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}
	if (!s5_coarse_samples_are_valid(coarse_samples, coarse_count) ||
	    !s5_fine_samples_are_valid(fine_samples, fine_count))
	{
		return S5_LOGIC_INVALID_SCAN;
	}
	context_status = s5_validate_scan_context(
		scan_context,
		coarse_samples,
		fine_samples,
		fine_count);
	if (context_status != S5_LOGIC_OK)
	{
		return context_status;
	}

	left_minimum_cm = UINT32_MAX;
	right_minimum_cm = UINT32_MAX;
	closest_distance_cm = UINT32_MAX;
	closest_index = 0U;
	valid_fine_samples = 0U;

	for (size_t index = 0U; index < coarse_count; index++)
	{
		if (!coarse_samples[index].valid)
		{
			continue;
		}

		if (coarse_samples[index].angle_deg < S5_SCAN_CENTRE_ANGLE_DEG)
		{
			profile->left_valid_samples++;
			if (coarse_samples[index].distance_cm < left_minimum_cm)
			{
				left_minimum_cm = coarse_samples[index].distance_cm;
			}
		}
		else if (coarse_samples[index].angle_deg >
			 S5_SCAN_CENTRE_ANGLE_DEG)
		{
			profile->right_valid_samples++;
			if (coarse_samples[index].distance_cm < right_minimum_cm)
			{
				right_minimum_cm = coarse_samples[index].distance_cm;
			}
		}
	}

	for (size_t index = 0U; index < fine_count; index++)
	{
		if (!fine_samples[index].valid)
		{
			continue;
		}

		valid_fine_samples++;
		if (fine_samples[index].distance_cm < closest_distance_cm)
		{
			closest_distance_cm = fine_samples[index].distance_cm;
			closest_index = index;
		}
	}

	if (valid_fine_samples < S5_MIN_VALID_FINE_SAMPLES)
	{
		memset(profile, 0, sizeof(*profile));
		return S5_LOGIC_INSUFFICIENT_DATA;
	}

	profile->closest_distance_cm = (uint16_t)closest_distance_cm;
	profile->closest_angle_deg = fine_samples[closest_index].angle_deg;
	if (!s5_profile_coordinates(
		    profile->closest_angle_deg,
		    profile->closest_distance_cm,
		    &profile->lateral_offset_cm,
		    &profile->forward_distance_cm))
	{
		memset(profile, 0, sizeof(*profile));
		return S5_LOGIC_INVALID_SCAN;
	}

	if (profile->left_valid_samples >= S5_MIN_VALID_SIDE_SAMPLES)
	{
		profile->left_min_range_cm = (uint16_t)left_minimum_cm;
	}
	if (profile->right_valid_samples >= S5_MIN_VALID_SIDE_SAMPLES)
	{
		profile->right_min_range_cm = (uint16_t)right_minimum_cm;
	}

	first_edge_index = closest_index;
	last_edge_index = closest_index;
	while (first_edge_index > 0U)
	{
		size_t candidate_index;

		candidate_index = first_edge_index - 1U;
		if (!fine_samples[candidate_index].valid ||
		    ((uint32_t)fine_samples[candidate_index].distance_cm >
		     (closest_distance_cm + S5_DEV_PROFILE_EDGE_MARGIN_CM)))
		{
			break;
		}
		first_edge_index = candidate_index;
	}
	while ((last_edge_index + 1U) < fine_count)
	{
		size_t candidate_index;

		candidate_index = last_edge_index + 1U;
		if (!fine_samples[candidate_index].valid ||
		    ((uint32_t)fine_samples[candidate_index].distance_cm >
		     (closest_distance_cm + S5_DEV_PROFILE_EDGE_MARGIN_CM)))
		{
			break;
		}
		last_edge_index = candidate_index;
	}

	width_span_deg =
		(uint32_t)(fine_samples[last_edge_index].angle_deg -
			   fine_samples[first_edge_index].angle_deg);
	tangent_index = width_span_deg / S5_SCAN_ANGLE_STEP_DEG;
	if ((tangent_index < (sizeof(s5_tangent_half_span_q1000) /
			      sizeof(s5_tangent_half_span_q1000[0]))) &&
	    ((width_span_deg % S5_SCAN_ANGLE_STEP_DEG) == 0U))
	{
		width_cm =
			(2U * closest_distance_cm *
			 s5_tangent_half_span_q1000[tangent_index] +
			 S5_Q1000_HALF) /
			S5_Q1000_ONE;
		profile->estimated_width_cm = (uint16_t)width_cm;
	}

	s5_set_bypass_direction(profile);
	profile->valid = true;
	return S5_LOGIC_OK;
}

s5_logic_status_t
s5_build_obstacle_plan(
	const s5_obstacle_profile_t *profile,
	s5_obstacle_plan_t *plan)
{
	if ((profile == NULL) || (plan == NULL))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}
	memset(plan, 0, sizeof(*plan));

	if (!profile->valid ||
	    (profile->closest_distance_cm <
		    S5_MIN_VALID_DISTANCE_CM ||
	    profile->closest_distance_cm >
		    S5_MAX_VALID_DISTANCE_CM))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	if ((profile->bypass_direction != S5_BYPASS_LEFT_CANDIDATE) &&
	    (profile->bypass_direction != S5_BYPASS_RIGHT_CANDIDATE) &&
	    (profile->bypass_direction != S5_BYPASS_NO_CANDIDATE))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}

	plan->bypass_direction = profile->bypass_direction;
	plan->reverse_candidate =
		(profile->closest_distance_cm <= S5_DEV_REVERSE_DISTANCE_CM);

	/*
	 * Range samples do not prove a collision-free vehicle path. Do not
	 * produce executable actions until geometry and motion contracts exist.
	 */
	return S5_LOGIC_CLEARANCE_UNVERIFIED;
}


/* Conservative circle uses L1 half-extents, avoiding floating point/underbound. */
bool s5_route_validate (s5_route_config_t const * p_config)
{
    if (NULL == p_config) { return (false); }
    const int32_t values[] = {p_config->half_length_um,
        p_config->half_width_um,p_config->clearance_um,
        p_config->tracking_error_um,p_config->stopping_um,
        p_config->obstacle_half_width_um,p_config->obstacle_depth_um,
        p_config->lateral_extra_um,p_config->search_um};

    for (size_t index = 0U; index < sizeof(values)/sizeof(values[0]); ++index)
    {
        if ((values[index] <= 0) || (values[index] > 1000000))
        {
            return (false);
        }
    }
    return (p_config->b_calibrated &&
        (p_config->sensor_forward_um >= 0) &&
        (p_config->sensor_forward_um <= 1000000) &&
        (p_config->speed_mm_s > 0) && (p_config->speed_mm_s <= 1000) &&
        (p_config->step_timeout_us > 0U) &&
        (p_config->route_timeout_us > p_config->step_timeout_us) &&
        (p_config->route_timeout_us <= 60000000U));
}

s5_logic_status_t s5_route_plan (s5_obstacle_profile_t const * p_profile,
    s5_route_config_t const * p_config, s5_clearance_t const * p_clearance,
    uint64_t scan_id_us, uint64_t completed_us, uint64_t now_us,
    s5_obstacle_plan_t * p_plan)
{
    if (NULL == p_plan) { return (S5_LOGIC_INVALID_ARGUMENT); }
    *p_plan = (s5_obstacle_plan_t){0};
    if ((NULL == p_profile) || !p_profile->valid ||
        !s5_route_validate(p_config) || (NULL == p_clearance))
    {
        return (S5_LOGIC_INVALID_ARGUMENT);
    }
    if (!p_clearance->b_verified || !p_clearance->b_original_line_verified ||
        (0U == scan_id_us) ||
        (scan_id_us != p_clearance->scan_id_us) ||
        (completed_us < scan_id_us) ||
        (p_clearance->observed_us < completed_us) ||
        (p_clearance->observed_us > now_us) ||
        (now_us > UINT64_MAX - p_config->route_timeout_us) ||
        (p_clearance->valid_until_us < now_us + p_config->route_timeout_us))
    {
        return (S5_LOGIC_CLEARANCE_UNVERIFIED);
    }
    if (((p_profile->bypass_direction != S5_BYPASS_LEFT_CANDIDATE) &&
         (p_profile->bypass_direction != S5_BYPASS_RIGHT_CANDIDATE)) ||
        (p_profile->forward_distance_cm < S5_MIN_VALID_DISTANCE_CM) ||
        (p_profile->forward_distance_cm > S5_MAX_VALID_DISTANCE_CM) ||
        (p_profile->lateral_offset_cm < -400) ||
        (p_profile->lateral_offset_cm > 400))
    {
        return (S5_LOGIC_INVALID_SCAN);
    }
    const int64_t radius = (int64_t)p_config->half_length_um +
        p_config->half_width_um + p_config->clearance_um +
        p_config->tracking_error_um + p_config->stopping_um;
    int64_t lateral = (int64_t)p_profile->lateral_offset_cm * 10000;

    if (lateral < 0) { lateral = -lateral; }
    lateral += p_config->obstacle_half_width_um + radius +
        p_config->lateral_extra_um;
    const int64_t forward = (int64_t)p_profile->forward_distance_cm * 10000 +
        p_config->sensor_forward_um + p_config->obstacle_depth_um + radius +
        p_config->lateral_extra_um;
    const bool b_left = p_profile->bypass_direction == S5_BYPASS_LEFT_CANDIDATE;
    const int64_t y_min = b_left ? 0 : -lateral;
    const int64_t y_max = b_left ? lateral : 0;
    /* One certificate must contain each ENTIRE swept corridor, including
     * in-place rotations, braking/pose error, and the bounded forward search.
     */
    const int64_t bounds[3][4] = {
        {-radius,radius,y_min-radius,y_max+radius},
        {-radius,forward+radius,(b_left ? lateral : -lateral)-radius,
            (b_left ? lateral : -lateral)+radius},
        {forward-radius,forward+p_config->search_um+radius,
            y_min-radius,y_max+radius}};
    for (size_t index = 0U; index < 3U; ++index)
    {
        s5_clear_box_t const * p_box = &p_clearance->corridor[index];

        const int64_t obstacle_x = (int64_t)p_profile->forward_distance_cm *
            10000 + p_config->sensor_forward_um;
        /* Legacy profile is positive RIGHT; route coordinates are positive LEFT. */
        const int64_t obstacle_y = -(int64_t)p_profile->lateral_offset_cm * 10000;
        const bool b_overlap =
            ((int64_t)p_box->x_max_um >= obstacle_x) &&
            ((int64_t)p_box->x_min_um <= obstacle_x + p_config->obstacle_depth_um) &&
            ((int64_t)p_box->y_max_um >= obstacle_y - p_config->obstacle_half_width_um) &&
            ((int64_t)p_box->y_min_um <= obstacle_y + p_config->obstacle_half_width_um);

        if (b_overlap || (p_box->x_min_um >= p_box->x_max_um) ||
            (p_box->y_min_um >= p_box->y_max_um) ||
            ((int64_t)p_box->x_min_um > bounds[index][0]) ||
            ((int64_t)p_box->x_max_um < bounds[index][1]) ||
            ((int64_t)p_box->y_min_um > bounds[index][2]) ||
            ((int64_t)p_box->y_max_um < bounds[index][3]))
        {
            return (S5_LOGIC_CLEARANCE_UNVERIFIED);
        }
    }
    /* Configuration/profile bounds keep distances below INT32_MAX. */
    const s5_obstacle_action_t outward = b_left ?
        S5_ACTION_TURN_LEFT : S5_ACTION_TURN_RIGHT;
    const s5_obstacle_action_t inward = b_left ?
        S5_ACTION_TURN_RIGHT : S5_ACTION_TURN_LEFT;
    *p_plan = (s5_obstacle_plan_t){0};
    p_plan->actions[0] = outward;
    p_plan->actions[1] = S5_ACTION_BYPASS;
    p_plan->actions[2] = inward;
    p_plan->actions[3] = S5_ACTION_BYPASS;
    p_plan->actions[4] = inward;
    p_plan->actions[5] = S5_ACTION_BYPASS;
    p_plan->actions[6] = outward;
    p_plan->actions[7] = S5_ACTION_SEARCH_LINE;
    p_plan->distance_um[1] = (int32_t)lateral;
    p_plan->distance_um[3] = (int32_t)forward;
    p_plan->distance_um[5] = (int32_t)lateral;
    p_plan->distance_um[7] = p_config->search_um;
    p_plan->speed_mm_s = p_config->speed_mm_s;
    p_plan->scan_id_us = scan_id_us;
    p_plan->valid_until_us = p_clearance->valid_until_us;
    p_plan->action_count = S5_ACTION_PLAN_MAX;
    p_plan->bypass_direction = p_profile->bypass_direction;
    p_plan->executable = true;
    return (S5_LOGIC_OK);
}
/*** end of file ***/
