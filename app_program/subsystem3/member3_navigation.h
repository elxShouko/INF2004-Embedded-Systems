/** @file member3_navigation.h
 * @brief Member 3 member3 navigation module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#ifndef MEMBER3_NAVIGATION_H
#define MEMBER3_NAVIGATION_H

#include "../app.h"
#include "drv_ir_input.h"
#include "drv_line_tracking.h"
#include "drv_barcode.h"
#include "drv_junction.h"
#include "drv_navigation_map.h"

#define S3_PRI_NAVIGATION 17 /* Existing template's allocated S3 band. */
#define S3_MAPPING_TEXT_MAX 384U

typedef struct
{
    ir_calibration_t  calibration;
    uint32_t          period_ms;
    uint32_t          line_stable_us;
    uint32_t          line_lost_us;
    uint32_t          line_found_us;
    uint32_t          recovery_timeout_us;
    uint32_t          barcode_quiet_us;
    uint32_t          barcode_timeout_us;
    uint32_t          max_sample_gap_us;
    uint32_t          odometry_stale_us;
    uint32_t          barcode_rearm_us;
    bool              junction_enabled;
    junction_config_t junction;
} s3_config_t;

typedef enum
{
    S3_INPUT_CONFIG = 0,
    S3_INPUT_MODE,
    S3_INPUT_MAPPING,
    S3_INPUT_ODOMETRY,
    S3_INPUT_MANOEUVRE_DONE
} s3_input_kind_t;

/** @brief Describe one timestamped forward-odometry observation. */
typedef struct
{
    uint64_t timestamp_us;
    uint64_t forward_distance_um;
    bool     tracking_forward;
} s3_odometry_t;

/** @brief Preserve the existing tagged input payload layout. */
typedef union
{
    s3_config_t   config;
    line_mode_t   mode;
    char          mapping[S3_MAPPING_TEXT_MAX];
    s3_odometry_t odometry;
    uint32_t      manoeuvre_event_id;
} s3_input_payload_t;

typedef struct
{
    s3_input_kind_t kind;
    uint32_t request_id; /* Sender's correlation token, echoed in replies. */

    s3_input_payload_t value;
} s3_input_t;

typedef enum
{
    S3_OUTPUT_NAVIGATION = 0,
    S3_OUTPUT_LINE,
    S3_OUTPUT_REPLY
} s3_output_kind_t;

typedef enum
{
    S3_FAULT_NONE = 0,
    S3_FAULT_SAMPLE_GAP,
    S3_FAULT_RECOVERY_TIMEOUT,
    S3_FAULT_OUTPUT_FULL,
    S3_FAULT_DRIVER,
    S3_FAULT_KERNEL
} s3_fault_t;

typedef struct
{
    uint32_t           event_id;
    s3_output_kind_t   kind;
    nav_event_t        navigation;
    line_observation_t line;
    uint32_t           map_revision;
    uint32_t           request_id;
    bool               accepted;
    s3_fault_t         fault;
} s3_output_t;

/** @brief Coherent owner snapshot; successful copying is not motion readiness.
 * raw.timestamp_us is acquisition time from s3_clock_us(), never read time.
 * black_mask is the unfiltered normalized mask for raw when calibrated and
 * fault-free; line is the last stable filtered front-pair observation.
 * line_valid means a filtered value exists, NOT that it matches current raw.
 * Check calibration, sample age, mask/line agreement, mode/state, fault and
 * pending_manoeuvre_event_id together before permitting ordinary tracking.
 * Repeated reads do not renew sensing. On a fault, retained sensor/line fields
 * are diagnostic only. Failed publication leaves an older snapshot to age out.
 */
typedef struct
{
    ir_sample_t        raw;
    uint8_t            black_mask;
    bool               calibrated;
    bool               line_valid;
    line_observation_t line;
    line_state_t       state;
    line_mode_t        mode;
    s3_fault_t         fault;
    uint32_t           map_revision;
    uint32_t           pending_manoeuvre_event_id;
    uint32_t           output_drops;
    nav_command_t      mapping[NAV_ALPHABET_SIZE];
} s3_status_t;

/* Created by S3 init BEFORE app_main starts any subsystem tasks. */
IMPORT volatile ID mbf_s3_input;
IMPORT volatile ID mbf_s3_output;
IMPORT volatile ID mtx_s3_status;
IMPORT volatile ID tid_s3_navigation;

/* Task-context APIs. All processing/configuration is serialized by S3 task.
 * E_OK here means queued; use S3_OUTPUT_REPLY for actual validation result.
 * Controller should drain output frequently and poll status for latched fault.
 * Line events are changes, not motion keepalives. A controller must validate
 * calibrated/line_valid, fault, mode/state AND the age of raw.timestamp_us
 * before refreshing Buddy 2 motion. A fresh raw sample alone does not make
 * a retained filtered observation suitable for steering. See the proposed
 * handoff in docs/buddy3/production-timing-and-handoff.md. No controller or
 * direct motor connection is implemented by these provider APIs.
 */
/** @brief Copy one request into the bounded owner-task input queue. */
ER s3_submit (const s3_input_t * p_input, TMO timeout);
/** @brief Receive and validate one fixed-size owner-task output. */
ER s3_receive (s3_output_t * p_output, TMO timeout);
/** @brief Poll once for a copied status; task context only, no retry loop.
 * E_OK: coherent copy (may be old); E_TMOUT: mutex busy, no wait performed;
 * E_PAR: NULL output; E_OBJ: objects unavailable. Other kernel errors pass
 * through, including unlock errors. On EVERY error output remains unchanged.
 * Treat failure as unavailable feedback; never renew motion from that buffer.
 * No kernel object wait is requested; target execution latency still requires
 * measurement under SMP/preemption. Reads never modify acquisition timestamps.
 */
ER s3_get_status (s3_status_t * p_status);
/** @brief Create navigation objects before any subsystem task starts. */
ER s3_navigation_init (void);
/** @brief Create and start the single navigation owner task. */
ER s3_navigation_start (void);
/** @brief Serialize requests and sensor processing on the owner task. */
void s3_navigation_task (INT stacd, void * p_exinf);

#endif

/*** End of file. ***/

