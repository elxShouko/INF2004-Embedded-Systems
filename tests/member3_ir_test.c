#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "line/ir_input.h"
#include "line/navigation_types.h"

int main(void)
{
    ir_observations_t black = {0};
    ir_observations_t white = {0};
    ir_calibration_t calibration;
    ir_sample_t sample = {0U, 5U};
    uint8_t mask = 99U;

    /* Each channel may have its own electrical polarity. */
    for (unsigned int index = 0U; index < 4U; ++index)
    {
        sample.levels = 5U;
        assert(ir_observe(&black, &sample));
        sample.levels = 2U;
        assert(ir_observe(&white, &sample));
    }
    assert(ir_calibrate(&black, &white, 4U, &calibration));
    assert(calibration.black_high_mask == 5U);
    for (unsigned int levels = 0U; levels < 8U; ++levels)
    {
        sample.levels = (uint8_t)levels;
        assert(ir_normalize(&sample, &calibration, &mask));
        assert(mask == ((~(levels ^ 5U)) & 7U));
    }
    assert(!ir_calibrate(&black, &white, 5U, &calibration));
    assert(!ir_normalize(&sample, &calibration, &mask));
    assert(!ir_calibrate(&black, &white, 0U, &calibration));
    assert(!ir_calibrate(&black, &black, 4U, &calibration));
    white.high_count[0] = 1U; /* Noisy reference must not be accepted. */
    assert(!ir_calibrate(&black, &white, 4U, &calibration));
    assert(calibration.valid_mask == 6U);
    assert(!ir_normalize(&sample, &calibration, &mask));

    ir_filter_t filter;
    ir_sample_t filtered;
    assert(ir_filter_init(&filter, 10U));
    sample = (ir_sample_t){100U, 0U};
    assert(!ir_filter_update(&filter, &sample, &filtered));
    sample.timestamp_us = 110U;
    assert(ir_filter_update(&filter, &sample, &filtered));
    assert(filtered.levels == 0U);
    sample = (ir_sample_t){111U, 1U};
    assert(ir_filter_update(&filter, &sample, &filtered));
    assert(filtered.levels == 0U);
    sample = (ir_sample_t){115U, 0U}; /* Short glitch is rejected. */
    assert(ir_filter_update(&filter, &sample, &filtered));
    sample = (ir_sample_t){120U, 1U};
    assert(ir_filter_update(&filter, &sample, &filtered));
    sample.timestamp_us = 130U;
    assert(ir_filter_update(&filter, &sample, &filtered));
    assert(filtered.levels == 1U);
    sample.timestamp_us = 129U; /* Reject backwards time. */
    assert(!ir_filter_update(&filter, &sample, &filtered));
    assert(filter.last_timestamp_us == 130U);
    assert(ir_filter_init(&filter, 0U));
    assert(ir_filter_update(&filter, &sample, &filtered));
    assert(filtered.levels == sample.levels);
    sample.levels = 8U;
    assert(!ir_filter_update(&filter, &sample, &filtered));
    assert(!ir_observe(&black, &sample));
    assert(!ir_observe(NULL, &sample));
    assert(!ir_filter_init(NULL, 10U));
    assert(!ir_calibrate(NULL, NULL, 1U, NULL));
    black.samples = UINT32_MAX;
    sample.levels = 0U;
    assert(!ir_observe(&black, &sample));

    puts("Member 3 IR tests passed.");
    return 0;
}
