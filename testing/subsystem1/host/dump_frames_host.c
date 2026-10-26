/* Prints many encoded telemetry frames with the inputs that produced them, one
 * per line, for tools/s1_crosscheck.py (an independent Python oracle written
 * from the document, not from this code). Includes out-of-range inputs.
 *   F seq state validity fault ts_ms spdL spdR encL encR gen enc_ts_us line
 *     age_line_us age_obst_us age_imu_us hump obst hex(29 bytes) */
#include <inttypes.h>
#include <stdio.h>

#include "../../../app_program/subsystem1/s1_codec.h"

static uint32_t g_state = 2463534242u;
static uint32_t rnd(void)
{
    g_state ^= g_state << 13;
    g_state ^= g_state >> 17;
    g_state ^= g_state << 5;
    return g_state;
}
/* Mostly in range, sometimes a boundary, sometimes just out of range. */
static int64_t pick(int64_t lo, int64_t hi)
{
    uint32_t r = rnd() % 16u;
    if (r == 0u)
    {
        return lo;
    }
    if (r == 1u)
    {
        return hi;
    }
    if (r == 2u)
    {
        return lo - 1;
    }
    if (r == 3u)
    {
        return hi + 1;
    }
    return lo + (int64_t)(((uint64_t)rnd() << 16 ^ rnd()) % (uint64_t)(hi - lo + 1));
}

int main(void)
{
    uint32_t i, k;
    for (i = 0u; i < 4000u; i++)
    {
        s1_codec_frame_t f = {0};
        uint8_t b[S1_CODEC_TELEMETRY_BIN_BYTES];
        uint32_t seq = rnd();
        uint8_t fault = (uint8_t)rnd();
        f.timestamp_ms = (i % 7u == 0u) ? 0xFFFFFFFFu : (rnd() % 4000000000u);
        f.robot_state = (uint8_t)(rnd() % 8u);
        f.validity = (uint8_t)(rnd() % 64u);
        f.motor_speed_left = (int32_t)pick(-2048, 2047);
        f.motor_speed_right = (int32_t)pick(-2048, 2047);
        f.encoder_count_left = pick(-2147483648ll, 2147483647ll);
        f.encoder_count_right = pick(-2147483648ll, 2147483647ll);
        f.encoder_generation = (rnd() % 5u == 0u) ? 0u : (rnd() % 400u);
        k = (uint32_t)pick(0, 1023999);
        f.encoder_timestamp_us = ((uint64_t)f.timestamp_ms * 1000u > k) ? ((uint64_t)f.timestamp_ms * 1000u - k) : 0u;
        if (rnd() % 23u == 0u)
        {
            f.encoder_timestamp_us = (uint64_t)f.timestamp_ms * 1000u + 1u;
        } /* from the future */
        f.line_status = (uint8_t)pick(0, 7);
        f.sensor_age_us[0] = (uint32_t)pick(0, 999999);
        f.sensor_age_us[1] = (uint32_t)pick(0, 999999);
        f.sensor_age_us[2] = (uint32_t)pick(0, 999999);
        f.hump_peak_x10 = (int32_t)pick(0, 4095);
        f.obstacle_distance_x10 = (int32_t)pick(20, 4000);
        if (s1_codec_encode_telemetry_bin(&f, seq, fault, b, sizeof b, NULL) != 29u)
        {
            return 3;
        }
        printf("F %" PRIu32 " %u %u %u %" PRIu32 " %" PRId32 " %" PRId32 " %" PRId64 " %" PRId64 " %" PRIu32
               " %" PRIu64 " %u %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRId32 " %" PRId32 " ",
               seq, f.robot_state, f.validity, fault, f.timestamp_ms, f.motor_speed_left, f.motor_speed_right,
               f.encoder_count_left, f.encoder_count_right, f.encoder_generation, f.encoder_timestamp_us,
               f.line_status, f.sensor_age_us[0], f.sensor_age_us[1], f.sensor_age_us[2], f.hump_peak_x10,
               f.obstacle_distance_x10);
        for (k = 0u; k < 29u; k++)
        {
            printf("%02x", b[k]);
        }
        printf("\n");
    }
    return 0;
}