#ifndef S5_OBSTACLE_RUNTIME_H
#define S5_OBSTACLE_RUNTIME_H

#include "s5_obstacle_logic.h"

#define S5_RUNTIME_MAX_TIMEOUT_MS	0x7fffffffU

typedef enum
{
	S5_RUNTIME_MONITOR = 0,
	S5_RUNTIME_WAIT_STOP,
	S5_RUNTIME_SCANNING,
	S5_RUNTIME_WAIT_BYPASS,
	S5_RUNTIME_LINE_RECOVERY,
	S5_RUNTIME_FAULT
} s5_runtime_state_t;

typedef enum
{
	S5_RUNTIME_FAULT_NONE = 0,
	S5_RUNTIME_FAULT_INVALID_INPUT,
	S5_RUNTIME_FAULT_FORWARD_SENSOR,
	S5_RUNTIME_FAULT_STOP_TIMEOUT,
	S5_RUNTIME_FAULT_MOVED_DURING_SCAN,
	S5_RUNTIME_FAULT_SCAN_CANCELLED,
	S5_RUNTIME_FAULT_SCAN_TIMEOUT,
	S5_RUNTIME_FAULT_SCAN_INVALID,
	S5_RUNTIME_FAULT_NO_CLEARANCE,
	S5_RUNTIME_FAULT_BYPASS_TIMEOUT,
	S5_RUNTIME_FAULT_LINE_TIMEOUT
} s5_runtime_fault_t;

typedef struct
{
	uint32_t forward_sample_max_age_ms;
	uint32_t stop_timeout_ms;
	uint32_t scan_timeout_ms;
	uint32_t bypass_timeout_ms;
	uint32_t line_timeout_ms;
	uint16_t obstacle_threshold_cm;
	uint8_t confirmation_count;
} s5_runtime_config_t;

typedef struct
{
	/* Runtime and scan timestamps use one wrapping monotonic millisecond clock. */
	uint32_t now_ms;
	uint32_t forward_sample_time_ms;
	uint16_t forward_distance_cm;
	bool forward_valid;
	bool vehicle_stopped;
	bool scan_busy;
	bool scan_complete;
	bool scan_cancelled;
	s5_acquisition_status_t acquisition_status;
	const s5_scan_context_t *scan_context;
	const s5_scan_sample_t *coarse_samples;
	size_t coarse_count;
	const s5_scan_sample_t *fine_samples;
	size_t fine_count;
	bool bypass_complete;
	bool line_found;
} s5_runtime_input_t;

typedef struct
{
	s5_runtime_state_t state;
	s5_runtime_fault_t fault;
	uint32_t updated_at_ms;
	uint32_t forward_sample_time_ms;
	uint32_t scan_started_at_ms;
	uint32_t scan_completed_at_ms;
	uint16_t forward_distance_cm;
	s5_logic_status_t scan_status;
	s5_acquisition_status_t acquisition_status;
	s5_scan_context_t scan_context;
	s5_scan_sample_t coarse_samples[S5_COARSE_SCAN_COUNT];
	s5_scan_sample_t fine_samples[S5_FINE_SCAN_MAX_COUNT];
	size_t coarse_count;
	size_t fine_count;
	s5_obstacle_profile_t profile;
	s5_obstacle_plan_t plan;
	bool forward_valid;
	bool busy;
	bool scan_busy;
	bool scan_complete;
	bool scan_valid;
} s5_runtime_status_t;

typedef struct
{
	bool request_stop;
	bool request_scan;
	bool request_bypass;
	bool request_line_recovery;
	bool request_fault;
} s5_runtime_events_t;

typedef struct
{
	s5_runtime_config_t config;
	s5_runtime_status_t status;
	uint32_t state_started_at_ms;
	uint32_t last_forward_sample_time_ms;
	uint8_t obstacle_confirmations;
	bool have_update;
	bool have_forward_sample;
	bool initialised;
} s5_runtime_t;

/*
 * Configuration values are mandatory and have no calibrated defaults.
 * Call step/get_status only from the single owning task; cross-task readers
 * require a separately agreed RTOS publication mechanism.
 */
s5_logic_status_t s5_runtime_init(
	s5_runtime_t *runtime,
	const s5_runtime_config_t *config);

s5_logic_status_t s5_runtime_step(
	s5_runtime_t *runtime,
	const s5_runtime_input_t *input,
	s5_runtime_events_t *events);

s5_logic_status_t s5_runtime_get_status(
	const s5_runtime_t *runtime,
	s5_runtime_status_t *status);

#endif /* S5_OBSTACLE_RUNTIME_H */
