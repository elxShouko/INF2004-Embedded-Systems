/** @file member3_data.h
 * @brief S3 application values, independent of kernel object declarations.
 */

#ifndef MEMBER3_DATA_H
#define MEMBER3_DATA_H

#include "drv_ir_input.h"
#include "drv_line_tracking.h"
#include "drv_barcode.h"
#include "drv_junction.h"
#include "drv_navigation_map.h"

#define S3_PRI_NAVIGATION 17 /* Existing template's allocated S3 band. */

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
    uint8_t           junction_enabled : 1;
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

/** @brief Describe one forward-odometry acquisition, not a publication.
 * timestamp_us uses S3's RP2040 raw timer domain. The controller must translate
 * S2's kernel measurement time while preserving source age; never use S2's
 * publication time or refresh time on read/retry. Distance is cumulative
 * forward micrometres, not signed wheel position or speed in mm/s. Repeated
 * timestamps require identical distance AND tracking_forward values.
 */
typedef struct
{
    uint64_t timestamp_us;
    uint64_t forward_distance_um;
    uint8_t  tracking_forward : 1;
} s3_odometry_t;

/** @brief Store one typed request; the codec defines the queue bytes. */
typedef union
{
    s3_config_t   config;
    line_mode_t   mode;
    nav_update_t  mapping;
    s3_odometry_t odometry;
    /* S3 navigation event ID, distinct from the S2 motion command ID.
     * Controller sends DONE only after matching S2 success and required
     * stop/standstill. S3 cannot establish physical completion from this ID.
     */
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
    S3_FAULT_SAMPLE_GAP = 1U,
    S3_FAULT_RECOVERY_TIMEOUT = 2U,
    S3_FAULT_OUTPUT_FULL = 4U,
    S3_FAULT_DRIVER = 8U,
    S3_FAULT_KERNEL = 16U
} s3_fault_t;

typedef struct
{
    uint32_t           event_id;
    uint8_t            kind : 2;
    nav_event_t        navigation;
    uint8_t            line : 2;
    uint32_t           map_revision;
    uint32_t           request_id;
    uint8_t            accepted : 1;
    uint8_t            fault : 5;
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
    uint8_t            black_mask : 3;
    uint8_t            calibrated : 1;
    uint8_t            line_valid : 1;
    uint8_t            line : 2;
    uint8_t            state : 3;
    uint8_t            mode : 2;
    uint8_t            fault : 5;
    uint32_t           map_revision;
    uint32_t           pending_manoeuvre_event_id;
    uint32_t           output_drops;
    uint8_t            mapping[NAV_ACTION_BYTES]; /* Use nav_action_get(). */
} s3_status_t;

#endif

/*** End of file. ***/

