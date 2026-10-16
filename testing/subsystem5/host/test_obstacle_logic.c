#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../../../app_program/subsystem5/s5_obstacle_logic.h"

static unsigned int s5_test_failures;
static void test_route_planner (void);

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
		coarse_samples[index].captured_at_ms =
			100U + (uint32_t)(index * 100U);
		coarse_samples[index].valid = true;
	}

	for (size_t index = 0U; index < S5_FINE_SCAN_MAX_COUNT; index++)
	{
		fine_samples[index].angle_deg =
			(uint8_t)(70U + (index * 5U));
		fine_samples[index].distance_cm = fine_distances[index];
		fine_samples[index].captured_at_ms =
			600U + (uint32_t)(index * 100U);
		fine_samples[index].valid = true;
	}
}

static void
set_valid_scan_context(s5_scan_context_t *scan_context)
{
	scan_context->started_at_ms = 100U;
	scan_context->completed_at_ms = 1500U;
	scan_context->evaluated_at_ms = 1600U;
	scan_context->max_duration_ms = 2000U;
	scan_context->max_age_ms = 500U;
	scan_context->max_adjacent_range_delta_cm = 100U;
	scan_context->acquisition_status = S5_ACQUISITION_OK;
	scan_context->complete = true;
	scan_context->cancelled = false;
	scan_context->servo_settled = true;
	scan_context->vehicle_stationary = true;
}

int
main(void)
{
	uint8_t coarse_angles[S5_COARSE_SCAN_COUNT];
	uint8_t fine_angles[S5_FINE_SCAN_MAX_COUNT];
	size_t fine_angle_count;
	s5_scan_context_t scan_context;
	s5_scan_sample_t coarse_samples[S5_COARSE_SCAN_COUNT];
	s5_scan_sample_t fine_samples[S5_FINE_SCAN_MAX_COUNT];
	s5_obstacle_profile_t profile;
	s5_obstacle_plan_t plan;
	s5_logic_status_t status;
	bool angle_lists_correct;
	bool reverse_threshold_correct;
	bool off_axis_profiles_correct;
	bool scan_fault_checks_correct;

	set_valid_scan_context(&scan_context);

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
		&scan_context,
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
				&scan_context,
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
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	check_result("S5-U04",
		     (status == S5_LOGIC_CLEARANCE_UNVERIFIED) &&
		     (profile.bypass_direction ==
		      S5_BYPASS_LEFT_CANDIDATE) &&
		     (plan.bypass_direction == S5_BYPASS_LEFT_CANDIDATE) &&
		     (plan.action_count == 0U) &&
		     !plan.executable,
		     "left range candidate is not emitted as an executable route");

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
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	reverse_threshold_correct =
		(status == S5_LOGIC_CLEARANCE_UNVERIFIED) &&
		(profile.closest_distance_cm == 9U) &&
		plan.reverse_candidate &&
		(plan.action_count == 0U) &&
		!plan.executable;

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
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	reverse_threshold_correct =
		reverse_threshold_correct &&
		(status == S5_LOGIC_CLEARANCE_UNVERIFIED) &&
		(profile.closest_distance_cm == 10U) &&
		plan.reverse_candidate &&
		(plan.action_count == 0U) &&
		!plan.executable;

	for (size_t index = 0U; index < S5_FINE_SCAN_MAX_COUNT; index++)
	{
		fine_samples[index].distance_cm++;
	}
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	reverse_threshold_correct =
		reverse_threshold_correct &&
		(status == S5_LOGIC_CLEARANCE_UNVERIFIED) &&
		(profile.closest_distance_cm == 11U) &&
		!plan.reverse_candidate &&
		(plan.action_count == 0U) &&
		!plan.executable;
	check_result("S5-U05", reverse_threshold_correct,
		     "reverse remains an advisory candidate, never an action");

	fill_normal_scan(coarse_samples, fine_samples);
	coarse_samples[0].distance_cm = 20U;
	coarse_samples[1].distance_cm = 30U;
	coarse_samples[3].distance_cm = 10U;
	coarse_samples[4].distance_cm = 20U;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	if (status == S5_LOGIC_OK)
	{
		status = s5_build_obstacle_plan(&profile, &plan);
	}
	check_result("S5-U06",
		     (status == S5_LOGIC_CLEARANCE_UNVERIFIED) &&
		     (profile.bypass_direction == S5_BYPASS_NO_CANDIDATE) &&
		     (plan.action_count == 0U) &&
		     !plan.executable,
		     "missing range candidates are not misreported as proven unsafe");

	fill_normal_scan(coarse_samples, fine_samples);
	fine_samples[4].angle_deg = 95U;
	status = s5_build_obstacle_profile(
		&scan_context,
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
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	status = (status == S5_LOGIC_OK)
		? s5_build_obstacle_plan(&profile, &plan)
		: status;
	check_result("S5-U08",
		     (status == S5_LOGIC_CLEARANCE_UNVERIFIED) &&
		     (plan.bypass_direction ==
		      S5_BYPASS_RIGHT_CANDIDATE) &&
		     (plan.action_count == 0U) &&
		     !plan.executable,
		     "right range candidate is not emitted as an executable route");

	fill_normal_scan(coarse_samples, fine_samples);
	fine_samples[0].valid = false;
	fine_samples[1].valid = false;
	fine_samples[2].valid = false;
	fine_samples[3].valid = false;
	fine_samples[4].valid = false;
	fine_samples[5].valid = false;
	fine_samples[6].valid = false;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U09",
		     (status == S5_LOGIC_INSUFFICIENT_DATA) &&
		     !profile.valid &&
		     (profile.closest_distance_cm == 0U),
		     "too few valid fine readings cannot produce a profile");

	fill_normal_scan(coarse_samples, fine_samples);
	set_valid_scan_context(&scan_context);
	scan_context.cancelled = true;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U12",
		     (status == S5_LOGIC_SCAN_CANCELLED) &&
		     !profile.valid,
		     "cancelled scan is rejected and profile cleared");

	set_valid_scan_context(&scan_context);
	scan_context.acquisition_status = S5_ACQUISITION_ECHO_TIMEOUT;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	scan_fault_checks_correct =
		(status == S5_LOGIC_ACQUISITION_FAILED) &&
		!profile.valid;
	set_valid_scan_context(&scan_context);
	scan_context.servo_settled = false;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	scan_fault_checks_correct =
		scan_fault_checks_correct &&
		(status == S5_LOGIC_ACQUISITION_FAILED) &&
		!profile.valid;
	check_result("S5-U13", scan_fault_checks_correct,
		     "echo timeout or unsettled servo rejects scan");

	set_valid_scan_context(&scan_context);
	scan_context.complete = false;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	scan_fault_checks_correct =
		(status == S5_LOGIC_INCOMPLETE_SCAN) &&
		!profile.valid;
	set_valid_scan_context(&scan_context);
	scan_context.vehicle_stationary = false;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	scan_fault_checks_correct =
		scan_fault_checks_correct &&
		(status == S5_LOGIC_SCAN_NOT_STATIONARY) &&
		!profile.valid;
	check_result("S5-U14", scan_fault_checks_correct,
		     "partial or moving-vehicle scan is rejected");

	set_valid_scan_context(&scan_context);
	scan_context.max_duration_ms = 1000U;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U15",
		     (status == S5_LOGIC_STALE_SCAN) &&
		     !profile.valid,
		     "scan exceeding supplied maximum duration is rejected");

	set_valid_scan_context(&scan_context);
	fine_samples[0].captured_at_ms = coarse_samples[
		S5_COARSE_SCAN_COUNT - 1U].captured_at_ms;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U16",
		     (status == S5_LOGIC_STALE_SCAN) &&
		     !profile.valid,
		     "non-monotonic sample times are rejected");

	fill_normal_scan(coarse_samples, fine_samples);
	set_valid_scan_context(&scan_context);
	fine_samples[4].distance_cm = 200U;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U17",
		     (status == S5_LOGIC_INCONSISTENT_SCAN) &&
		     !profile.valid,
		     "range jumps over supplied consistency limit are rejected");

	fill_normal_scan(coarse_samples, fine_samples);
	set_valid_scan_context(&scan_context);
	scan_context.evaluated_at_ms =
		scan_context.completed_at_ms + scan_context.max_age_ms + 1U;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U18",
		     (status == S5_LOGIC_STALE_SCAN) &&
		     !profile.valid,
		     "scan older than configured maximum age is rejected");

	fill_normal_scan(coarse_samples, fine_samples);
	set_valid_scan_context(&scan_context);
	fine_samples[S5_FINE_SCAN_MAX_COUNT - 1U].captured_at_ms =
		scan_context.completed_at_ms + 1U;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U19",
		     (status == S5_LOGIC_STALE_SCAN) &&
		     !profile.valid,
		     "late sample beyond scan completion is rejected");

	{
		static const s5_acquisition_status_t acquisition_failures[] =
		{
			S5_ACQUISITION_TRIGGER_FAILED,
			S5_ACQUISITION_ECHO_TIMEOUT,
			S5_ACQUISITION_ECHO_STUCK_HIGH,
			S5_ACQUISITION_ECHO_STUCK_LOW,
			S5_ACQUISITION_SERVO_INIT_FAILED,
			S5_ACQUISITION_SERVO_POSITION_FAILED
		};

		scan_fault_checks_correct = true;
		for (size_t index = 0U;
		     index < (sizeof(acquisition_failures) /
			      sizeof(acquisition_failures[0]));
		     index++)
		{
			set_valid_scan_context(&scan_context);
			scan_context.acquisition_status =
				acquisition_failures[index];
			status = s5_build_obstacle_profile(
				&scan_context,
				coarse_samples,
				S5_COARSE_SCAN_COUNT,
				fine_samples,
				S5_FINE_SCAN_MAX_COUNT,
				&profile);
			if ((status != S5_LOGIC_ACQUISITION_FAILED) ||
			    profile.valid)
			{
				scan_fault_checks_correct = false;
				break;
			}
		}
	}
	check_result("S5-U20", scan_fault_checks_correct,
		     "trigger, echo, and servo faults reject profile creation");

	fill_normal_scan(coarse_samples, fine_samples);
	set_valid_scan_context(&scan_context);
	scan_context.max_adjacent_range_delta_cm = 0U;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U21",
		     (status == S5_LOGIC_INVALID_SCAN) &&
		     !profile.valid,
		     "missing scan-validation configuration is rejected");

	fill_normal_scan(coarse_samples, fine_samples);
	set_valid_scan_context(&scan_context);
	fine_samples[4].distance_cm = 401U;
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U22",
		     (status == S5_LOGIC_INVALID_SCAN) &&
		     !profile.valid,
		     "out-of-range measurement is rejected");

	fill_normal_scan(coarse_samples, fine_samples);
	set_valid_scan_context(&scan_context);
	scan_context.started_at_ms = UINT32_MAX - 500U;
	scan_context.completed_at_ms =
		scan_context.started_at_ms + 1400U;
	scan_context.evaluated_at_ms =
		scan_context.completed_at_ms + 100U;
	for (size_t index = 0U; index < S5_COARSE_SCAN_COUNT; index++)
	{
		uint32_t sample_elapsed_ms;

		sample_elapsed_ms = coarse_samples[index].captured_at_ms - 100U;
		coarse_samples[index].captured_at_ms =
			scan_context.started_at_ms + sample_elapsed_ms;
	}
	for (size_t index = 0U; index < S5_FINE_SCAN_MAX_COUNT; index++)
	{
		uint32_t sample_elapsed_ms;

		sample_elapsed_ms = fine_samples[index].captured_at_ms - 100U;
		fine_samples[index].captured_at_ms =
			scan_context.started_at_ms + sample_elapsed_ms;
	}
	status = s5_build_obstacle_profile(
		&scan_context,
		coarse_samples, S5_COARSE_SCAN_COUNT,
		fine_samples, S5_FINE_SCAN_MAX_COUNT, &profile);
	check_result("S5-U23",
		     (status == S5_LOGIC_OK) && profile.valid,
		     "valid scan across millisecond-counter wrap is accepted");

	fill_normal_scan(coarse_samples, fine_samples);
	set_valid_scan_context(&scan_context);
	(void)memset(&profile, 0, sizeof(profile));
	status = s5_build_obstacle_plan(&profile, &plan);
	check_result("S5-U10",
		     (status == S5_LOGIC_INVALID_ARGUMENT) &&
		     (plan.action_count == 0U) &&
		     !plan.executable,
		     "a profile that was not successfully built cannot create a plan");

    test_route_planner();
	(void)printf("[TEST] subsystem5 host failures: %u\n", s5_test_failures);
	return (s5_test_failures == 0U) ? 0 : 1;
}


static void test_route_planner (void)
{
    const s5_route_config_t config = {50000,80000,10000,5000,10000,0,
        100000,200000,50000,200000,50,1000000U,20000000U,true};
    const s5_clearance_t evidence = {
        {{-155000,155000,-155000,460000},
         {-155000,960000,150000,460000},
         {650000,1160000,-155000,460000}},
        100000U,1000000U,40000000U,true,true};
    s5_obstacle_profile_t profile = {0};
    s5_obstacle_plan_t plan = {0};

    profile.valid = true;
    profile.forward_distance_cm = 40U;
    profile.bypass_direction = S5_BYPASS_LEFT_CANDIDATE;
    check_result("S5-U73",S5_LOGIC_OK == s5_route_plan(&profile,&config,
        &evidence,100000U,1000000U,1010000U,&plan) &&
        plan.executable && (8U == plan.action_count) &&
        (305000 == plan.distance_um[1]) &&
        (805000 == plan.distance_um[3]) &&
        (200000 == plan.distance_um[7]) &&
        (S5_ACTION_TURN_LEFT == plan.actions[0]),
        "configured left geometry and exact um conversion");
    s5_clearance_t right = evidence;
    for (size_t n = 0U; n < 3U; ++n)
    {
        int32_t min = right.corridor[n].y_min_um;
        right.corridor[n].y_min_um = -right.corridor[n].y_max_um;
        right.corridor[n].y_max_um = -min;
    }
    profile.bypass_direction = S5_BYPASS_RIGHT_CANDIDATE;
    check_result("S5-U74",S5_LOGIC_OK == s5_route_plan(&profile,&config,
        &right,100000U,1000000U,1010000U,&plan) &&
        (S5_ACTION_TURN_RIGHT == plan.actions[0]) &&
        (S5_ACTION_TURN_LEFT == plan.actions[2]),
        "right route mirrors turns and clearance");
    profile.bypass_direction = S5_BYPASS_LEFT_CANDIDATE;
    s5_clearance_t invalid = evidence;
    invalid.b_verified = false;
    check_result("S5-U75",S5_LOGIC_CLEARANCE_UNVERIFIED ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,
            1010000U,&plan) && !plan.executable && (0U == plan.action_count),
        "unverified evidence clears previously executable output");
    invalid = evidence; invalid.scan_id_us++;
    check_result("S5-U76",S5_LOGIC_CLEARANCE_UNVERIFIED ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,1010000U,&plan),
        "different scan frame rejected");
    invalid = evidence; invalid.observed_us = 999999U;
    check_result("S5-U77",S5_LOGIC_CLEARANCE_UNVERIFIED ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,1010000U,&plan),
        "evidence predating completed scan rejected");
    invalid = evidence; invalid.observed_us = 1010001U;
    check_result("S5-U78",S5_LOGIC_CLEARANCE_UNVERIFIED ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,1010000U,&plan),
        "future evidence rejected");
    invalid = evidence; invalid.valid_until_us = 21009999U;
    check_result("S5-U79",S5_LOGIC_CLEARANCE_UNVERIFIED ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,1010000U,&plan),
        "certificate must cover entire route deadline");
    invalid = evidence; invalid.corridor[0].x_max_um--;
    check_result("S5-U80",S5_LOGIC_CLEARANCE_UNVERIFIED ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,1010000U,&plan),
        "one micrometre swept corridor undercoverage rejects");
    invalid = evidence; invalid.corridor[1].y_min_um = -200000;
    check_result("S5-U81",S5_LOGIC_CLEARANCE_UNVERIFIED ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,1010000U,&plan),
        "claimed free box contradicting bounded obstacle rejects");
    invalid = evidence; invalid.b_original_line_verified = false;
    check_result("S5-U87",S5_LOGIC_CLEARANCE_UNVERIFIED ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,1010000U,&plan),
        "free space without original-line identity cannot authorize recovery");
    profile.lateral_offset_cm = 30; /* Profile positive RIGHT -> route y negative. */
    invalid = evidence;
    invalid.corridor[0] = (s5_clear_box_t){-155000,450000,-155000,760000};
    invalid.corridor[1] = (s5_clear_box_t){-155000,960000,450000,760000};
    invalid.corridor[2] = (s5_clear_box_t){650000,1160000,-155000,760000};
    check_result("S5-U88",S5_LOGIC_OK ==
        s5_route_plan(&profile,&config,&invalid,100000U,1000000U,1010000U,&plan) &&
        (605000 == plan.distance_um[1]),"legacy lateral sign maps into robot frame");
    profile.lateral_offset_cm = 0;
    s5_route_config_t bad = config;
    bad.stopping_um = 0;
    check_result("S5-U82",!s5_route_validate(&bad) &&
        (S5_LOGIC_CLEARANCE_UNVERIFIED == s5_route_plan(&profile,&config,
            &evidence,100000U,1000000U,UINT64_MAX,&plan)) &&
        !plan.executable, "missing stopping calibration and clock overflow reject");
}
/*** end of file ***/
