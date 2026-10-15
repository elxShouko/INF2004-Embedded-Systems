#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../../../app_program/subsystem5/s5_obstacle_logic.h"

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

static void
fill_normal_scan(
	s5_scan_sample_t *coarse_samples,
	s5_scan_sample_t *fine_samples)
{
	static const uint16_t coarse_distances[] =
	{
		100U, 80U, 50U, 40U, 60U
	};
	static const uint16_t fine_distances[] =
	{
		60U, 50U, 32U, 29U, 30U, 31U, 32U, 50U, 60U
	};

	for (size_t index = 0U; index < S5_COARSE_SCAN_COUNT; index++)
	{
		coarse_samples[index].angle_deg =
			(uint8_t)(30U + (index * 30U));
		coarse_samples[index].distance_cm = coarse_distances[index];
		coarse_samples[index].valid = true;
	}

	for (size_t index = 0U; index < S5_FINE_SCAN_MAX_COUNT; index++)
	{
		fine_samples[index].angle_deg =
			(uint8_t)(70U + (index * 5U));
		fine_samples[index].distance_cm = fine_distances[index];
		fine_samples[index].valid = true;
	}
}

int
main(void)
{
	uint8_t coarse_angles[S5_COARSE_SCAN_COUNT];
	uint8_t fine_angles[S5_FINE_SCAN_MAX_COUNT];
	size_t fine_angle_count;
	s5_scan_sample_t coarse_samples[S5_COARSE_SCAN_COUNT];
	s5_scan_sample_t fine_samples[S5_FINE_SCAN_MAX_COUNT];
	s5_obstacle_profile_t profile;
	s5_obstacle_plan_t plan;
	s5_logic_status_t status;
	bool angle_lists_correct;
	bool reverse_threshold_correct;
	bool off_axis_profiles_correct;

	status = s5_make_coarse_scan_angles(
		coarse_angles, S5_COARSE_SCAN_COUNT);
	angle_lists_correct =
		(status == S5_LOGIC_OK) &&
		(coarse_angles[0] == 30U) &&
		(coarse_angles[1] == 60U) &&
		(coarse_angles[2] == 90U) &&
		(coarse_angles[3] == 120U) &&
		(coarse_angles[4] == 150U);
	status = s5_make_coarse_scan_angles(coarse_angles, 4U);
	angle_lists_correct =
		angle_lists_correct &&
		(status == S5_LOGIC_INVALID_ARGUMENT);

	fine_angle_count = 0U;
	status = s5_make_fine_scan_angles(
		60U, fine_angles, 4U, &fine_angle_count);
	angle_lists_correct =
		angle_lists_correct &&
		(status == S5_LOGIC_INVALID_ARGUMENT) &&
		(fine_angle_count == 0U);

	fine_angle_count = 0U;
	status = s5_make_fine_scan_angles(
		60U, fine_angles, S5_FINE_SCAN_MAX_COUNT, &fine_angle_count);
	angle_lists_correct =
		angle_lists_correct &&
		(status == S5_LOGIC_OK) &&
		(fine_angle_count == S5_FINE_SCAN_MAX_COUNT) &&
		(fine_angles[0] == 40U) &&
		(fine_angles[8] == 80U);

	fine_angle_count = 0U;
	status = s5_make_fine_scan_angles(
		150U, fine_angles, S5_FINE_SCAN_MAX_COUNT, &fine_angle_count);
	angle_lists_correct =
		angle_lists_correct &&
		(status == S5_LOGIC_OK) &&
		(fine_angle_count == 5U) &&
		(fine_angles[0] == 130U) &&
		(fine_angles[4] == 150U);
	check_result("S5-U02", angle_lists_correct,
		     "coarse and fine angles are bounded and edge-clamped");

	fill_normal_scan(coarse_samples, fine_samples);
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U03",
		     (status == S5_LOGIC_OK) &&
		     profile.valid &&
		     (profile.closest_distance_cm == 29U) &&
		     (profile.closest_angle_deg == 85U) &&
		     (profile.lateral_offset_cm == -3) &&
		     (profile.forward_distance_cm == 29U) &&
		     (profile.estimated_width_cm == 10U) &&
		     (profile.left_min_range_cm == 80U) &&
		     (profile.right_min_range_cm == 40U),
		     "profile reports nearest point, geometry, width and side ranges");

	{
		static const uint8_t centres[] = {30U, 60U, 120U, 150U};
		static const int16_t expected_lateral_cm[] = {-43, -25, 25, 43};
		static const uint16_t expected_forward_cm[] = {25U, 43U, 43U, 25U};

		off_axis_profiles_correct = true;
		fill_normal_scan(coarse_samples, fine_samples);
		for (size_t centre_index = 0U;
		     centre_index < (sizeof(centres) / sizeof(centres[0]));
		     centre_index++)
		{
			status = s5_make_fine_scan_angles(
				centres[centre_index],
				fine_angles,
				S5_FINE_SCAN_MAX_COUNT,
				&fine_angle_count);
			if (status != S5_LOGIC_OK)
			{
				off_axis_profiles_correct = false;
				break;
			}

			for (size_t sample_index = 0U;
			     sample_index < fine_angle_count;
			     sample_index++)
			{
				fine_samples[sample_index].angle_deg =
					fine_angles[sample_index];
				fine_samples[sample_index].distance_cm =
					(fine_angles[sample_index] ==
					 centres[centre_index])
						? 50U
						: 60U;
				fine_samples[sample_index].valid = true;
			}

			status = s5_build_obstacle_profile(
				coarse_samples,
				S5_COARSE_SCAN_COUNT,
				fine_samples,
				fine_angle_count,
				&profile);
			if ((status != S5_LOGIC_OK) ||
			    (profile.closest_angle_deg != centres[centre_index]) ||
			    (profile.lateral_offset_cm !=
			     expected_lateral_cm[centre_index]) ||
			    (profile.forward_distance_cm !=
			     expected_forward_cm[centre_index]))
			{
				off_axis_profiles_correct = false;
				break;
			}
		}
	}
	check_result("S5-U11", off_axis_profiles_correct,
		     "profiles off-axis scans centred at coarse directions");

	fill_normal_scan(coarse_samples, fine_samples);
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	check_result("S5-U04",
		     (status == S5_LOGIC_OK) &&
		     (plan.bypass_direction == S5_BYPASS_LEFT) &&
		     (plan.action_count == 6U) &&
		     (plan.actions[0] == S5_ACTION_STOP) &&
		     (plan.actions[1] == S5_ACTION_TURN_LEFT) &&
		     (plan.actions[2] == S5_ACTION_BYPASS) &&
		     (plan.actions[3] == S5_ACTION_TURN_RIGHT) &&
		     (plan.actions[4] == S5_ACTION_SEARCH_LINE) &&
		     (plan.actions[5] == S5_ACTION_RESUME_LINE) &&
		     !plan.reverse_required,
		     "normal plan uses the larger validated side range");

	{
		static const uint16_t close_distances[] =
		{
			20U, 15U, 10U, 9U, 10U, 15U, 20U, 30U, 40U
		};
		for (size_t index = 0U; index < S5_FINE_SCAN_MAX_COUNT; index++)
		{
			fine_samples[index].distance_cm = close_distances[index];
		}
	}
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	reverse_threshold_correct =
		(status == S5_LOGIC_OK) &&
		(profile.closest_distance_cm == 9U) &&
		plan.reverse_required &&
		(plan.actions[0] == S5_ACTION_STOP) &&
		(plan.actions[1] == S5_ACTION_REVERSE) &&
		(plan.actions[2] == S5_ACTION_TURN_LEFT) &&
		(plan.action_count == 7U);

	{
		static const uint16_t threshold_distances[] =
		{
			20U, 15U, 12U, 10U, 12U, 15U, 20U, 30U, 40U
		};
		for (size_t index = 0U; index < S5_FINE_SCAN_MAX_COUNT; index++)
		{
			fine_samples[index].distance_cm =
				threshold_distances[index];
		}
	}
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	reverse_threshold_correct =
		reverse_threshold_correct &&
		(status == S5_LOGIC_OK) &&
		(profile.closest_distance_cm == 10U) &&
		plan.reverse_required;

	for (size_t index = 0U; index < S5_FINE_SCAN_MAX_COUNT; index++)
	{
		fine_samples[index].distance_cm++;
	}
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	reverse_threshold_correct =
		reverse_threshold_correct &&
		(status == S5_LOGIC_OK) &&
		(profile.closest_distance_cm == 11U) &&
		!plan.reverse_required &&
		(plan.actions[1] == S5_ACTION_TURN_LEFT) &&
		(plan.action_count == 6U);
	check_result("S5-U05", reverse_threshold_correct,
		     "reverse is included at and below the development threshold");

	fill_normal_scan(coarse_samples, fine_samples);
	coarse_samples[0].distance_cm = 20U;
	coarse_samples[1].distance_cm = 30U;
	coarse_samples[3].distance_cm = 10U;
	coarse_samples[4].distance_cm = 20U;
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	if (status == S5_LOGIC_OK)
	{
		status = s5_build_obstacle_plan(&profile, &plan);
	}
	check_result("S5-U06",
		     (status == S5_LOGIC_NO_SAFE_ROUTE) &&
		     (plan.action_count == 0U),
		     "no side meets the development range threshold");

	fill_normal_scan(coarse_samples, fine_samples);
	fine_samples[4].angle_deg = 95U;
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U07",
		     (status == S5_LOGIC_INVALID_SCAN) &&
		     !profile.valid &&
		     (profile.closest_distance_cm == 0U),
		     "malformed scan is rejected and output is cleared");

	fill_normal_scan(coarse_samples, fine_samples);
	coarse_samples[0].distance_cm = 40U;
	coarse_samples[1].distance_cm = 50U;
	coarse_samples[3].distance_cm = 90U;
	coarse_samples[4].distance_cm = 70U;
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	check_result("S5-U08",
		     (status == S5_LOGIC_OK) &&
		     (plan.bypass_direction == S5_BYPASS_RIGHT) &&
		     (plan.actions[1] == S5_ACTION_TURN_RIGHT) &&
		     (plan.actions[3] == S5_ACTION_TURN_LEFT),
		     "right-side bypass selects the corresponding turn sequence");

	fill_normal_scan(coarse_samples, fine_samples);
	fine_samples[0].valid = false;
	fine_samples[1].valid = false;
	fine_samples[2].valid = false;
	fine_samples[3].valid = false;
	fine_samples[4].valid = false;
	fine_samples[5].valid = false;
	fine_samples[6].valid = false;
	status = s5_build_obstacle_profile(
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U09",
		     (status == S5_LOGIC_INSUFFICIENT_DATA) &&
		     !profile.valid &&
		     (profile.closest_distance_cm == 0U),
		     "too few valid fine readings cannot produce a profile");

	(void)memset(&profile, 0, sizeof(profile));
	status = s5_build_obstacle_plan(&profile, &plan);
	check_result("S5-U10",
		     (status == S5_LOGIC_INVALID_ARGUMENT) &&
		     (plan.action_count == 0U),
		     "a profile that was not successfully built cannot create a plan");

	(void)printf("[TEST] subsystem5 host failures: %u\n", s5_test_failures);
	return (s5_test_failures == 0U) ? 0 : 1;
}
