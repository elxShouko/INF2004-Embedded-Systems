/** @file vehicle.h @brief Single-owner mission policy; no hardware calls. */
#ifndef VEHICLE_H
#define VEHICLE_H
#include "subsystem2/motion_run.h"
#include "subsystem1/s1_codec.h"
#include "subsystem3/drv_line_tracking.h"
#include "subsystem3/line/navigation_types.h"
#include "subsystem4/imu_core.h"
#include "subsystem5/s5_obstacle_logic.h"

typedef enum
{
    VEHICLE_UNCONFIGURED = 0, VEHICLE_BLOCKED, VEHICLE_READY,
    VEHICLE_FOLLOW, VEHICLE_STOPPING, VEHICLE_MANOEUVRE,
    VEHICLE_RECOVERY, VEHICLE_PAUSED, VEHICLE_OBSTACLE, VEHICLE_FAULT,
    VEHICLE_BYPASS
} vehicle_state_t;

#define VEHICLE_BLOCK_LINE (1U)
#define VEHICLE_BLOCK_IMU (2U)
#define VEHICLE_BLOCK_OBSTACLE (4U)
#define VEHICLE_BLOCK_MOTION (8U)
#define VEHICLE_BLOCK_POWER (16U)

typedef struct
{
    uint32_t sensor_age_us;
    uint32_t command_ttl_us;
    uint32_t transition_timeout_us;
    uint32_t remote_age_ms;
    int32_t forward_mm_s;
    int32_t correction_mm_s;
    int32_t turn_mm_s;
    int32_t straight_um;
    uint16_t obstacle_stop_cm;
    bool b_calibrated;
    bool b_power_authorised;
    bool b_uturn_right;
    s5_route_config_t route; /* Optional; all zero means disabled. */
} vehicle_config_t;

typedef struct
{
    uint64_t now_us; /* micro T-Kernel operating time, full 64 bits. */
    uint64_t raw_now_us; /* RP2040 timer: NEVER equated with now_us. */
    motion_run_status_t motion;
    s1_codec_command_t remote;
    nav_event_t navigation;
    uint32_t navigation_id;
    uint32_t pending_navigation_id;
    uint64_t line_time_us;
    line_observation_t line;
    line_state_t line_state;
    line_mode_t line_mode;
    uint8_t black_mask;
    imu_core_status_t imu;
    s5_obstacle_profile_t obstacle; /* Scan diagnostics, not forward readiness. */
    s5_clearance_t clearance;
    uint64_t scan_id_us;
    uint64_t scan_completed_us;
    bool b_scan_held;
    uint16_t forward_cm;
    bool b_forward_valid;
    uint64_t obstacle_started_us;
    uint64_t obstacle_time_us; /* Raw timer domain, checked by provider. */
    int64_t encoder_count[2]; /* Actual counts, never reconstructed distance. */
    bool b_motion_available;
    bool b_line_available;
    bool b_line_calibrated;
    bool b_line_valid;
    bool b_imu_available;
    bool b_obstacle_available;
    bool b_counts_valid;
    bool b_link_up;
    bool b_estop;
    bool b_provider_fault;
} vehicle_input_t;

typedef struct
{
    move_plan_command_t command;
    uint32_t acknowledge_id;
    uint32_t navigation_done_id;
    uint64_t obstacle_recovered_us;
    line_mode_t mode;
    bool b_command;
    bool b_enable;
    bool b_stop;
    bool b_estop;
    bool b_mode;
} vehicle_output_t;

typedef struct
{
    vehicle_config_t config;
    vehicle_state_t state;
    uint32_t blockers;
    uint32_t next_id;
    uint32_t active_id;
    uint32_t navigation_id;
    uint32_t last_navigation_id;
    nav_command_t navigation;
    uint64_t transition_us;
    uint64_t last_now_us;
    bool b_initialised;
    bool b_run_requested;
    bool b_pause_requested;
    bool b_turn_completed;
    bool b_recovery_required;
    bool b_pending_command;
    bool b_line_found;
    move_plan_command_t pending_command;
    s5_obstacle_plan_t route;
    uint64_t route_started_us;
    uint64_t route_step_raw_us;
    uint64_t route_ready_raw_us;
    uint8_t route_index;
    bool b_obstacle_pending;
    bool b_route_active;
    bool b_route_terminal;
    bool b_route_found;
    bool b_route_finishing;
} vehicle_t;

/* Single owner. Relative forward path, not encoder counts or absolute pose. */
typedef struct
{
    int64_t previous_um[2];
    uint64_t previous_us;
    uint64_t distance_um;
    uint64_t raw_timestamp_us;
    uint8_t half_um; /* Carry the average of two odd wheel increments. */
    bool b_baseline;
    bool b_previous_forward;
    bool b_tracking_forward;
} vehicle_odometry_t;

bool vehicle_odometry (vehicle_odometry_t * p_odometry,
    vehicle_input_t const * p_input, int32_t maximum_mm_s);
bool vehicle_init (vehicle_t * p_vehicle, vehicle_config_t const * p_config);
bool vehicle_step (vehicle_t * p_vehicle, vehicle_input_t const * p_input,
    vehicle_output_t * p_output);
/* Version 2 copied telemetry: independently valid fields, missing data null. */
bool vehicle_frame (vehicle_t const * p_vehicle,
    vehicle_input_t const * p_input, s1_codec_frame_t * p_frame);
#endif /* VEHICLE_H */
/*** end of file ***/
