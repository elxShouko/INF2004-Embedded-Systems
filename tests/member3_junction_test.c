#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "line/junction.h"
#include "line/line_tracking.h"

/* Simulation geometry only; these are NOT calibrated robot defaults. */
static const junction_config_t g_config =
{
    .max_barcode_bar_um = 9000U,
    .min_barcode_gap_um = 2500U,
    .min_span_um = 12000U,
    .max_span_um = 22000U,
    .clear_span_um = 2000U,
    .max_sample_step_um = 500U,
    .max_sample_gap_us = 10000U,
    .min_black_us = 1000U,
    .max_crossing_us = 1000000U
};

static unsigned int traverse(uint32_t stroke_um, uint32_t time_per_step_us,
                             bool lose_line, bool tracking, bool start_inside)
{
    junction_detector_t detector;
    nav_event_t event;
    assert(junction_init(&detector, &g_config));
    unsigned int detections = 0U;
    for (uint64_t distance = 0U; distance <= 60000U; distance += 250U)
    {
        const uint64_t begin = start_inside ? 0U : 10000U;
        const bool side = (distance >= begin) && (distance < begin + stroke_um);
        junction_sample_t sample =
        {
            .timestamp_us = (distance / 250U) * time_per_step_us,
            .forward_distance_um = distance,
            .black_mask = (uint8_t)(3U | (side ? 4U : 0U)),
            .tracking_forward = tracking
        };
        if (lose_line && (distance == 18000U))
        {
            sample.black_mask = 4U;
        }
        const junction_result_t result = junction_update(&detector, &sample, &event);
        assert(result != JUNCTION_RESULT_INVALID);
        if (result == JUNCTION_RESULT_DETECTED)
        {
            ++detections;
            assert(event.kind == NAV_EVENT_JUNCTION);
            assert(event.command == NAV_COMMAND_NONE);
            assert(distance >= begin + stroke_um + g_config.clear_span_um);
        }
    }
    return detections;
}

int main(void)
{
    /* Model the photo: front points +/-6 mm are inside an 18 mm main line.
     * The side point outside that line crosses an 18 mm transverse stroke.
     */
    assert(traverse(18000U, 500U, false, true, false) == 1U);
    assert(traverse(18000U, 2000U, false, true, false) == 1U);
    assert(traverse(0U, 1000U, false, true, false) == 0U); /* Straight line */
    assert(traverse(9000U, 1000U, false, true, false) == 0U); /* Wide barcode bar */
    assert(traverse(250U, 1000U, false, true, false) == 0U); /* Black noise */
    assert(traverse(40000U, 1000U, false, true, false) == 0U); /* Wide patch */
    assert(traverse(18000U, 1000U, true, true, false) == 0U);
    assert(traverse(18000U, 1000U, false, false, false) == 0U); /* Bypass/pause */
    assert(traverse(18000U, 1000U, false, true, true) == 0U); /* Partial start */

    junction_detector_t detector;
    nav_event_t event;
    junction_sample_t sample = {0U, 0U, 3U, true};
    assert(junction_init(&detector, &g_config));
    assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_PENDING);
    /* A complete array of alternating barcode bars must not become a junction. */
    unsigned int detections = 0U;
    for (uint64_t distance = 250U; distance < 150000U; distance += 250U)
    {
        sample.timestamp_us += 1000U;
        sample.forward_distance_um = distance;
        sample.black_mask = (uint8_t)(3U |
            (((distance >= 10000U) && ((distance - 10000U) % 12000U < 9000U)) ? 4U : 0U));
        if (junction_update(&detector, &sample, &event) == JUNCTION_RESULT_DETECTED)
        {
            ++detections;
        }
    }
    assert(detections == 0U);

    /* No motion: a stationary side-black sensor cannot confirm a crossing. */
    assert(junction_init(&detector, &g_config));
    sample = (junction_sample_t){0U, 0U, 3U, true};
    assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_PENDING);
    for (unsigned int i = 1U; i < 200U; ++i)
    {
        sample.timestamp_us = i * 1000U;
        sample.black_mask = 7U;
        assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_PENDING);
    }
    /* Gaps, jumps, reversed odometry and illegal masks discard evidence. */
    sample.timestamp_us += 20000U;
    assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_INVALID);
    sample.timestamp_us += 1000U;
    sample.forward_distance_um += 501U;
    assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_INVALID);
    sample.timestamp_us += 1000U;
    sample.forward_distance_um -= 1U;
    assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_INVALID);
    sample.timestamp_us -= 1U;
    assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_INVALID);
    sample.timestamp_us += 1000U;
    sample.black_mask = 8U;
    assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_INVALID);
    junction_config_t invalid = g_config;
    invalid.min_span_um = invalid.max_barcode_bar_um;
    assert(!junction_init(&detector, &invalid));
    assert(junction_update(&detector, &sample, &event) == JUNCTION_RESULT_INVALID);
    invalid = g_config;
    invalid.min_barcode_gap_um = invalid.max_sample_step_um;
    assert(!junction_init(&detector, &invalid));

    /* Front-pair filtering ignores side barcode flicker and short line glitches. */
    line_tracker_t tracker;
    line_observation_t observation;
    assert(line_tracker_init(&tracker, 1000U));
    assert(!line_tracker_update(&tracker, 0U, 3U, &observation));
    assert(line_tracker_update(&tracker, 1000U, 7U, &observation));
    assert(observation == LINE_OBSERVATION_CENTRED);
    assert(line_tracker_update(&tracker, 1100U, 4U, &observation));
    assert(observation == LINE_OBSERVATION_CENTRED);
    assert(line_tracker_update(&tracker, 1200U, 3U, &observation));
    assert(observation == LINE_OBSERVATION_CENTRED);
    assert(line_tracker_update(&tracker, 2000U, 1U, &observation));
    assert(line_tracker_update(&tracker, 3000U, 5U, &observation));
    assert(observation == LINE_OBSERVATION_CORRECT_LEFT);
    assert(!line_tracker_update(&tracker, 2999U, 5U, &observation));
    puts("Member 3 junction tests passed: crossing geometry, barcodes, noise, timing and line filtering.");
    return 0;
}
