#ifndef S5_OBSTACLE_LOGIC_H
#define S5_OBSTACLE_LOGIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define S5_COARSE_SCAN_COUNT	5U
#define S5_FINE_SCAN_MAX_COUNT	9U
#define S5_ACTION_PLAN_MAX	7U
#define S5_MIN_VALID_DISTANCE_CM	2U
#define S5_MAX_VALID_DISTANCE_CM	400U

typedef enum
{
	S5_LOGIC_OK = 0,
	S5_LOGIC_INVALID_ARGUMENT,
	S5_LOGIC_INVALID_SCAN,
	S5_LOGIC_INSUFFICIENT_DATA,
	S5_LOGIC_SCAN_CANCELLED,
	S5_LOGIC_ACQUISITION_FAILED,
	S5_LOGIC_INCOMPLETE_SCAN,
	S5_LOGIC_SCAN_NOT_STATIONARY,
	S5_LOGIC_STALE_SCAN,
	S5_LOGIC_INCONSISTENT_SCAN,
	S5_LOGIC_CLEARANCE_UNVERIFIED
} s5_logic_status_t;

typedef enum
{
	S5_BYPASS_NONE = 0,
	S5_BYPASS_LEFT_CANDIDATE,
	S5_BYPASS_RIGHT_CANDIDATE,
	S5_BYPASS_NO_CANDIDATE
} s5_bypass_direction_t;

typedef enum
{
	S5_ACTION_NONE = 0,
	S5_ACTION_STOP,
	S5_ACTION_REVERSE,
	S5_ACTION_TURN_LEFT,
	S5_ACTION_TURN_RIGHT,
	S5_ACTION_BYPASS,
	S5_ACTION_SEARCH_LINE,
	S5_ACTION_RESUME_LINE
} s5_obstacle_action_t;

typedef enum
{
	S5_ACQUISITION_OK = 0,
	S5_ACQUISITION_TRIGGER_FAILED,
	S5_ACQUISITION_ECHO_TIMEOUT,
	S5_ACQUISITION_ECHO_STUCK_HIGH,
	S5_ACQUISITION_ECHO_STUCK_LOW,
	S5_ACQUISITION_SERVO_INIT_FAILED,
	S5_ACQUISITION_SERVO_POSITION_FAILED
} s5_acquisition_status_t;

typedef struct
{
	uint8_t angle_deg;
	uint16_t distance_cm;
	uint32_t captured_at_ms;
	bool valid;
} s5_scan_sample_t;

typedef struct
{
	uint32_t started_at_ms;
	uint32_t completed_at_ms;
	uint32_t evaluated_at_ms;
	uint32_t max_duration_ms;
	uint32_t max_age_ms;
	uint16_t max_adjacent_range_delta_cm;
	s5_acquisition_status_t acquisition_status;
	bool complete;
	bool cancelled;
	bool servo_settled;
	bool vehicle_stationary;
} s5_scan_context_t;

typedef struct
{
	bool valid;
	uint16_t closest_distance_cm;
	uint8_t closest_angle_deg;
	int16_t lateral_offset_cm;
	uint16_t forward_distance_cm;
	/* Approximate angular-span estimate; not calibrated physical width. */
	uint16_t estimated_width_cm;
	/* Minimum sampled range on each side, not geometric clearance. */
	uint16_t left_min_range_cm;
	uint16_t right_min_range_cm;
	uint8_t left_valid_samples;
	uint8_t right_valid_samples;
	/* Range-only proposal; this is not a clearance or safety decision. */
	s5_bypass_direction_t bypass_direction;
} s5_obstacle_profile_t;

typedef struct
{
	s5_obstacle_action_t actions[S5_ACTION_PLAN_MAX];
	uint8_t action_count;
	/* Candidate metadata only; action_count remains zero until approved. */
	s5_bypass_direction_t bypass_direction;
	bool reverse_candidate;
	bool executable;
} s5_obstacle_plan_t;

/*
 * Development assumptions: 90 degrees is forward; smaller angles are left.
 * Range-based direction candidates are not a physical clearance proof.
 */
s5_logic_status_t s5_make_coarse_scan_angles(
	uint8_t *angles,
	size_t angle_capacity);

s5_logic_status_t s5_make_fine_scan_angles(
	uint8_t centre_angle_deg,
	uint8_t *angles,
	size_t angle_capacity,
	size_t *angle_count);

s5_logic_status_t s5_build_obstacle_profile(
	const s5_scan_context_t *scan_context,
	const s5_scan_sample_t *coarse_samples,
	size_t coarse_count,
	const s5_scan_sample_t *fine_samples,
	size_t fine_count,
	s5_obstacle_profile_t *profile);

s5_logic_status_t s5_build_obstacle_plan(
	const s5_obstacle_profile_t *profile,
	s5_obstacle_plan_t *plan);

#endif /* S5_OBSTACLE_LOGIC_H */
