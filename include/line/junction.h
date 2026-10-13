#ifndef JUNCTION_H
#define JUNCTION_H

#include <stdbool.h>
#include <stdint.h>
#include "line/navigation_types.h"

typedef struct
{
    /* Measured geometric bounds; all distances are micrometres.
     * min_span_um MUST exceed the widest barcode black bar.
     */
    uint32_t max_barcode_bar_um;
    uint32_t min_barcode_gap_um;
    uint32_t min_span_um;
    uint32_t max_span_um;
    uint32_t clear_span_um;
    uint32_t max_sample_step_um;
    uint32_t max_sample_gap_us;
    uint32_t min_black_us;
    uint32_t max_crossing_us;
} junction_config_t;

typedef struct
{
    uint64_t timestamp_us;
    /* Monotonic cumulative forward distance supplied by Member 2.
     * Do not use absolute wheel path while reversing or turning.
     */
    uint64_t forward_distance_um;
    uint8_t black_mask; /* bit 0 left, bit 1 right, bit 2 side */
    bool tracking_forward; /* false during turns, bypass, pause, calibration */
} junction_sample_t;

typedef enum
{
    JUNCTION_RESULT_PENDING = 0,
    JUNCTION_RESULT_DETECTED,
    JUNCTION_RESULT_INVALID
} junction_result_t;

typedef struct
{
    junction_config_t config;
    uint64_t last_us;
    uint64_t last_distance_um;
    uint64_t clear_start_um;
    uint64_t black_start_um;
    uint64_t black_start_us;
    uint64_t first_white_um;
    uint64_t first_white_us;
    uint8_t phase;
    bool started;
} junction_detector_t;

bool junction_init(junction_detector_t *p_detector,
                   const junction_config_t *p_config);
/* Emits NAV_EVENT_JUNCTION once AFTER a wide black side stroke is followed
 * by stable white clearance while both front sensors retain the main line.
 * No motor action or crossing-topology classification is implied.
 * p_event is written only for DETECTED. All calls belong to one owner task.
 */
junction_result_t junction_update(junction_detector_t *p_detector,
                                  const junction_sample_t *p_sample,
                                  nav_event_t *p_event);

#endif
