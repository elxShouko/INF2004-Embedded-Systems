/** @file test_hump_wire.c
 * INT-86: controller must not truncate an out-of-range IMU height into
 * Buddy 1's 12-bit binary telemetry range.
 */
#include "vehicle.h"

#include <math.h>
#include <stdio.h>

static bool check_height (float height_mm, bool b_expected_valid,
                          int32_t expected_mm)
{
    vehicle_t vehicle = {0};
    vehicle_input_t input = {0};
    s1_codec_frame_t frame = {0};

    vehicle.state = VEHICLE_READY;
    input.now_us = 20000u;
    input.raw_now_us = 1020000u;
    input.b_imu_available = true;
    input.imu.phase = IMU_MONITORING;
    input.imu.time_ms = 1020u;
    input.imu.b_highest_valid = true;
    input.imu.highest_hump_mm = height_mm;
    if (!vehicle_frame(&vehicle, &input, &frame) ||
        (0u == (frame.validity & S1_VALID_IMU)))
    {
        return false;
    }
    if (b_expected_valid)
    {
        return (0u != (frame.validity & S1_VALID_HUMP)) &&
               (expected_mm == frame.hump_peak_x10);
    }
    return (0u == (frame.validity & S1_VALID_HUMP));
}

int main (void)
{
    const bool b_pass = check_height(0.0f, true, 0) &&
                        check_height(4095.0f, true, 4095) &&
                        check_height(4095.9f, false, 0) &&
                        check_height(4096.0f, false, 0) &&
                        check_height(-1.0f, false, 0) &&
                        check_height(NAN, false, 0) &&
                        check_height(INFINITY, false, 0);

    (void)printf("[TEST] INT-86 %s hump wire range\n",
                 b_pass ? "PASS" : "FAIL");
    return b_pass ? 0 : 1;
}

/* End of file. */
