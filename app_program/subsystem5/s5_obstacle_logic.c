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
#define S5_MIN_VALID_DISTANCE_CM	2U
#define S5_MAX_VALID_DISTANCE_CM	400U

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
				? S5_BYPASS_LEFT
				: S5_BYPASS_RIGHT;
	}
	else if (left_range_passes)
	{
		profile->bypass_direction = S5_BYPASS_LEFT;
	}
	else if (right_range_passes)
	{
		profile->bypass_direction = S5_BYPASS_RIGHT;
	}
	else
	{
		profile->bypass_direction = S5_BYPASS_NO_SAFE_ROUTE;
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

	if (profile == NULL)
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}
	memset(profile, 0, sizeof(*profile));

	if ((coarse_samples == NULL) || (fine_samples == NULL))
	{
		return S5_LOGIC_INVALID_ARGUMENT;
	}
	if (!s5_coarse_samples_are_valid(coarse_samples, coarse_count) ||
	    !s5_fine_samples_are_valid(fine_samples, fine_count))
	{
		return S5_LOGIC_INVALID_SCAN;
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
	s5_obstacle_action_t first_turn;
	s5_obstacle_action_t return_turn;

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

	if (profile->bypass_direction == S5_BYPASS_LEFT)
	{
		if ((profile->left_valid_samples < S5_MIN_VALID_SIDE_SAMPLES) ||
		    (profile->left_min_range_cm <
		     S5_DEV_MIN_RANGE_FOR_BYPASS_CM))
		{
			return S5_LOGIC_NO_SAFE_ROUTE;
		}
		first_turn = S5_ACTION_TURN_LEFT;
		return_turn = S5_ACTION_TURN_RIGHT;
	}
	else if (profile->bypass_direction == S5_BYPASS_RIGHT)
	{
		if ((profile->right_valid_samples < S5_MIN_VALID_SIDE_SAMPLES) ||
		    (profile->right_min_range_cm <
		     S5_DEV_MIN_RANGE_FOR_BYPASS_CM))
		{
			return S5_LOGIC_NO_SAFE_ROUTE;
		}
		first_turn = S5_ACTION_TURN_RIGHT;
		return_turn = S5_ACTION_TURN_LEFT;
	}
	else
	{
		return S5_LOGIC_NO_SAFE_ROUTE;
	}

	plan->bypass_direction = profile->bypass_direction;
	plan->actions[plan->action_count++] = S5_ACTION_STOP;
	if (profile->closest_distance_cm <= S5_DEV_REVERSE_DISTANCE_CM)
	{
		plan->actions[plan->action_count++] = S5_ACTION_REVERSE;
		plan->reverse_required = true;
	}
	plan->actions[plan->action_count++] = first_turn;
	plan->actions[plan->action_count++] = S5_ACTION_BYPASS;
	plan->actions[plan->action_count++] = return_turn;
	plan->actions[plan->action_count++] = S5_ACTION_SEARCH_LINE;
	plan->actions[plan->action_count++] = S5_ACTION_RESUME_LINE;

	return S5_LOGIC_OK;
}
