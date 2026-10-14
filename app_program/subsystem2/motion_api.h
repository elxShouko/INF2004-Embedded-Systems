/** @file motion_api.h
 * @brief PROVISIONAL Buddy 2 interface; Vehicle Controller owns arbitration.
 */
#ifndef MOTION_API_H
#define MOTION_API_H
#include "motion_bus.h"
typedef enum
{
    MOTION_API_QUEUED = 0, MOTION_API_BUSY, MOTION_API_INVALID,
    MOTION_API_UNAVAILABLE, MOTION_API_ERROR
} motion_api_result_t;
/* Copies all request data; callback contexts must remain alive. Configuration
 * and explicit enable are separate. QUEUED is intake, not physical acceptance;
 * status/terminal result confirms owner processing. No unmeasured defaults.
 * All task-context APIs use polling mutex acquisition: BUSY means retry.
 * For status(), QUEUED means the coherent copy succeeded. last_request_id,
 * last_request_issued_us and b_request_accepted confirm command processing,
 * including refresh rejection. Only one caller (Vehicle Controller) arbitrates
 * commands. Refresh before expiration with the SAME ID and a newer issued_us;
 * acknowledge terminal_id before submitting a higher ID. No ID wrap permitted.
 * now() uses kernel monotonic operating time (10 ms resolution here). Status
 * timestamp_us is publication time; measurement_timestamp_us is sample time.
 * Configuration is copied once before enable; active replacement is rejected.
 */
motion_api_result_t motion_api_configure (wheel_loop_config_t const * p_config,
    drv_encoder_port_t const * p_encoder_port);
motion_api_result_t motion_api_enable (void);
motion_api_result_t motion_api_submit (move_plan_command_t const * p_command);
motion_api_result_t motion_api_stop (void);
motion_api_result_t motion_api_cancel (uint32_t id);
motion_api_result_t motion_api_ack (uint32_t terminal_id);
motion_api_result_t motion_api_status (motion_run_status_t * p_status);
bool motion_api_now (uint64_t * p_now_us);
/* Task OR handler: latched dedicated safety flag + wake, NO command mutex or
 * mailbox. No runtime reset: emergency/fault recovery is unpowered/reboot only.
 * Return ERROR if wake/safety posting fails; caller must use system-level
 * independent power removal policy, not assume a physical stop occurred.
 */
motion_api_result_t motion_api_estop (void);
#endif /* MOTION_API_H */

/*** end of file ***/

