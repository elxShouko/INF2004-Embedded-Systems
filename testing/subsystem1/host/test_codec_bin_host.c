/* Host tests for the binary wire format v3 in s1_codec. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../../app_program/subsystem1/s1_codec.h"
#include "../../../app_program/subsystem1/s1_bits.h"

static int g_checks = 0, g_failures = 0;
static void check(int ok, const char *name)
{
    g_checks++;
    if (!ok)
    {
        g_failures++;
        printf("  FAIL %s\n", name);
    }
}

static s1_codec_frame_t good_frame(void)
{
    s1_codec_frame_t f;
    memset(&f, 0, sizeof f);
    f.timestamp_ms = 123456u;
    f.encoder_count_left = 1234567;
    f.encoder_count_right = -765432;
    f.hump_peak_x10 = 120;
    f.obstacle_distance_x10 = 455;
    f.motor_speed_left = 300;
    f.motor_speed_right = -250;
    f.robot_state = 2;
    f.line_status = 5;
    f.encoder_timestamp_us = 123456000ull - 7500ull;
    f.encoder_generation = 0x1A5;
    f.sensor_age_us[0] = 12000;
    f.sensor_age_us[1] = 34000;
    f.sensor_age_us[2] = 999999;
    f.validity = S1_VALID_ALL;
    f.schema_version = 2;
    return f;
}

static void test_roundtrip(void)
{
    s1_codec_frame_t f = good_frame();
    uint8_t b[S1_CODEC_TELEMETRY_BIN_BYTES];
    uint32_t oor = 99u;
    s1_codec_bin_telemetry_t d;
    uint32_t n = s1_codec_encode_telemetry_bin(&f, 4097u, 0xA5u, b, sizeof b, &oor);
    check(n == 29u, "size 29");
    check(oor == 0u, "no out-of-range");
    check(s1_codec_decode_telemetry_bin(b, n, &d), "decode ok");
    check(d.sequence == 1u, "sequence wraps at 12 bits");
    check(d.robot_state == 2u, "state");
    check(d.validity == 63u, "valid mask");
    check(d.fault_mask == 0xA5u, "fault mask");
    check(d.timestamp_ms == 123456u, "time");
    check(d.speed_left == 300 && d.speed_right == -250, "speeds");
    check(d.encoder_left == 1234567 && d.encoder_right == -765432, "encoders");
    check(d.encoder_generation == 0xA5u, "generation low 8 bits");
    check(d.encoder_age_ms == 7u, "encoder age ms (7500us -> 7)");
    check(d.line_status == 5u && d.line_age_ms == 12u, "line");
    check(d.hump_x10 == 120u, "hump");
    check(d.obstacle_x10 == 455u && d.obstacle_age_ms == 34u, "obstacle");
    check(d.imu_age_ms == 999u, "imu age");
}

static void test_layout(void)
{
    /* Known positions from contract 6.2, independent of the encoder code. */
    s1_codec_frame_t f = good_frame();
    uint8_t b[29];
    uint32_t v = 0u;
    int32_t sv = 0;
    s1_codec_encode_telemetry_bin(&f, 7u, 0x12u, b, sizeof b, NULL);
    s1_bits_get(b, 29u, 0u, 2u, &v);
    check(v == 1u, "bit 0 format");
    s1_bits_get(b, 29u, 2u, 12u, &v);
    check(v == 7u, "bit 2 sequence");
    s1_bits_get(b, 29u, 23u, 8u, &v);
    check(v == 0x12u, "bit 23 fault");
    s1_bits_get(b, 29u, 31u, 32u, &v);
    check(v == 123456u, "bit 31 timestamp");
    s1_bits_get_signed(b, 29u, 75u, 12u, &sv);
    check(sv == -250, "bit 75 right speed");
    s1_bits_get(b, 29u, 194u, 12u, &v);
    check(v == 455u, "bit 194 obstacle");
    s1_bits_get(b, 29u, 226u, 6u, &v);
    check(v == 0u, "bit 226 reserved zero");
    check(b[0] == (uint8_t)(1u | ((7u & 0x3Fu) << 2)), "byte 0 hand-computed");
}

static void expect_cleared(s1_codec_frame_t f, uint32_t bit, const char *name)
{
    uint8_t b[29];
    s1_codec_bin_telemetry_t d;
    uint32_t oor = 0u;
    check(s1_codec_encode_telemetry_bin(&f, 0u, 0u, b, sizeof b, &oor) == 29u, name);
    check(s1_codec_decode_telemetry_bin(b, 29u, &d), name);
    check((d.validity & bit) == 0u, name);
    check((d.validity & (S1_VALID_ALL & ~bit)) == (S1_VALID_ALL & ~bit), "others kept");
    check(oor == 1u, "one group counted");
}

static void test_out_of_range(void)
{
    s1_codec_frame_t f;
    f = good_frame();
    f.motor_speed_left = 2048;
    expect_cleared(f, S1_VALID_SPEED, "speed +2048");
    f = good_frame();
    f.motor_speed_right = -2049;
    expect_cleared(f, S1_VALID_SPEED, "speed -2049");
    f = good_frame();
    f.encoder_generation = 0;
    expect_cleared(f, S1_VALID_ENCODER, "gen 0");
    f = good_frame();
    f.encoder_timestamp_us = 0;
    expect_cleared(f, S1_VALID_ENCODER, "enc age > 1023");
    f = good_frame();
    f.encoder_timestamp_us = 999999999999ull;
    expect_cleared(f, S1_VALID_ENCODER, "enc in future");
    f = good_frame();
    f.line_status = 8;
    expect_cleared(f, S1_VALID_LINE, "line 8");
    f = good_frame();
    f.sensor_age_us[0] = 1000000;
    expect_cleared(f, S1_VALID_LINE, "line age 1000ms");
    f = good_frame();
    f.hump_peak_x10 = 4096;
    expect_cleared(f, S1_VALID_HUMP, "hump 4096");
    f = good_frame();
    f.hump_peak_x10 = -1;
    expect_cleared(f, S1_VALID_HUMP, "hump -1");
    f = good_frame();
    f.obstacle_distance_x10 = 19;
    expect_cleared(f, S1_VALID_OBSTACLE, "obstacle 19");
    f = good_frame();
    f.obstacle_distance_x10 = 4001;
    expect_cleared(f, S1_VALID_OBSTACLE, "obstacle 4001");
    f = good_frame();
    f.sensor_age_us[2] = 1000000;
    expect_cleared(f, S1_VALID_IMU, "imu age");
}

static void test_boundaries(void)
{
    s1_codec_frame_t f = good_frame();
    uint8_t b[29];
    s1_codec_bin_telemetry_t d;
    uint32_t oor = 9u;
    f.motor_speed_left = -2048;
    f.motor_speed_right = 2047;
    f.hump_peak_x10 = 4095;
    f.obstacle_distance_x10 = 20;
    f.line_status = 7;
    f.sensor_age_us[0] = 999999;
    f.encoder_timestamp_us = 123456000ull - 1023999ull;
    f.encoder_count_left = 2147483647ll;
    f.encoder_count_right = -2147483647ll - 1;
    check(s1_codec_encode_telemetry_bin(&f, 0u, 0xFFu, b, sizeof b, &oor) == 29u, "boundary encode");
    check(oor == 0u, "boundaries all valid");
    check(s1_codec_decode_telemetry_bin(b, 29u, &d), "boundary decode");
    check(d.speed_left == -2048 && d.speed_right == 2047, "speed limits");
    check(d.hump_x10 == 4095 && d.obstacle_x10 == 20 && d.line_status == 7, "field limits");
    check(d.encoder_age_ms == 1023u, "enc age 1023");
    check(d.encoder_left == 2147483647 && d.encoder_right == -2147483647 - 1, "int32 limits");
    /* A count beyond signed 32 bits is REJECTED, never truncated. */
    f.encoder_count_left = 0x100000005ll;
    check(s1_codec_encode_telemetry_bin(&f, 0u, 0u, b, sizeof b, &oor) == 29u && oor == 1u,
          "count above int32: encoded, one group rejected");
    s1_codec_decode_telemetry_bin(b, 29u, &d);
    check((d.validity & S1_VALID_ENCODER) == 0u && d.encoder_left == 0 && d.encoder_right == 0 &&
              d.encoder_generation == 0u,
          "count above int32: encoder group invalid, no fake small value");
    f.encoder_count_left = 2147483647ll + 1;
    s1_codec_encode_telemetry_bin(&f, 0u, 0u, b, sizeof b, &oor);
    check(oor == 1u, "INT32_MAX + 1 rejected");
    f.encoder_count_left = 0;
    f.encoder_count_right = -2147483647ll - 2;
    s1_codec_encode_telemetry_bin(&f, 0u, 0u, b, sizeof b, &oor);
    check(oor == 1u, "INT32_MIN - 1 rejected");
}

static void test_reject(void)
{
    s1_codec_frame_t f = good_frame();
    uint8_t b[32];
    s1_codec_bin_telemetry_t d;
    f.robot_state = 8;
    check(s1_codec_encode_telemetry_bin(&f, 0u, 0u, b, sizeof b, NULL) == 0u, "state 8 not sent");
    f = good_frame();
    check(s1_codec_encode_telemetry_bin(&f, 0u, 0u, b, 28u, NULL) == 0u, "small buffer");
    check(s1_codec_encode_telemetry_bin(NULL, 0u, 0u, b, 32u, NULL) == 0u, "null frame");
    check(s1_codec_encode_telemetry_bin(&f, 0u, 0u, NULL, 32u, NULL) == 0u, "null buffer");
    s1_codec_encode_telemetry_bin(&f, 0u, 0u, b, sizeof b, NULL);
    check(!s1_codec_decode_telemetry_bin(b, 28u, &d), "length 28 rejected");
    check(!s1_codec_decode_telemetry_bin(b, 30u, &d), "length 30 rejected");
    b[0] &= (uint8_t)~3u;
    check(!s1_codec_decode_telemetry_bin(b, 29u, &d), "format 0 rejected");
    s1_codec_encode_telemetry_bin(&f, 0u, 0u, b, sizeof b, NULL);
    b[28] |= 0x04u; /* bit 226 reserved */
    check(!s1_codec_decode_telemetry_bin(b, 29u, &d), "reserved bit rejected");
}

static void test_stamp(void)
{
    s1_codec_frame_t f = good_frame();
    uint8_t a[29], b[29];
    s1_codec_bin_telemetry_t d;
    unsigned i, diff = 0u;
    s1_codec_encode_telemetry_bin(&f, 0u, 0u, a, sizeof a, NULL);
    memcpy(b, a, sizeof a);
    check(s1_codec_telemetry_bin_stamp(b, 29u, 4099u, 0xC3u), "stamp ok");
    check(s1_codec_decode_telemetry_bin(b, 29u, &d) && d.sequence == 3u && d.fault_mask == 0xC3u, "stamp sets sequence (12 bits) and fault");
    s1_codec_encode_telemetry_bin(&f, 3u, 0xC3u, a, sizeof a, NULL);
    for (i = 0u; i < 29u; i++)
    {
        if (a[i] != b[i])
        {
            diff++;
        }
    }
    check(diff == 0u, "stamped frame equals a frame encoded with the same values");
    check(!s1_codec_telemetry_bin_stamp(b, 28u, 0u, 0u), "stamp wrong length");
    check(!s1_codec_telemetry_bin_stamp(NULL, 29u, 0u, 0u), "stamp NULL");
}

static void test_heartbeat_command(void)
{
    uint8_t b[8];
    uint32_t up = 0u;
    bool es = false;
    int c;
    s1_codec_map_ack_t ack = {0u, 0u, 0u};
    s1_codec_map_ack_t got = {9u, 9u, 9u};
    check(s1_codec_encode_heartbeat_bin(0xDEADBEEFu, true, NULL, b, sizeof b) == 8u, "hb size");
    check(s1_codec_decode_heartbeat_bin(b, 8u, &up, &es, &got) && up == 0xDEADBEEFu && es &&
              got.seq == 0u && got.result == 0u && got.revision == 0u,
          "hb roundtrip, no map ack");
    check(b[0] == 5u, "hb byte0 = format 1 | estop<<2");
    s1_codec_encode_heartbeat_bin(1u, false, NULL, b, sizeof b);
    check(s1_codec_decode_heartbeat_bin(b, 8u, &up, &es, NULL) && !es && up == 1u, "hb no estop, NULL ack allowed");
    check(!s1_codec_decode_heartbeat_bin(b, 5u, &up, &es, NULL), "hb old 5-byte length refused");
    check(!s1_codec_decode_heartbeat_bin(b, 7u, &up, &es, NULL), "hb length 7");
    b[0] |= 0x20u;
    check(!s1_codec_decode_heartbeat_bin(b, 8u, &up, &es, NULL), "hb reserved bits 5-7");
    s1_codec_encode_heartbeat_bin(1u, false, NULL, b, sizeof b);
    b[7] = 1u;
    check(!s1_codec_decode_heartbeat_bin(b, 8u, &up, &es, NULL), "hb reserved byte 7");
    check(s1_codec_encode_heartbeat_bin(1u, false, NULL, b, 7u) == 0u, "hb small buffer");
    /* Map acknowledgement: every result, boundary seq and revision. */
    for (c = 0; c < 4; c++)
    {
        ack.seq = (uint8_t)(c == 3 ? 255 : 17 + c);
        ack.result = (uint8_t)c;
        ack.revision = (uint8_t)(c == 3 ? 255 : 40 + c);
        check(8u == s1_codec_encode_heartbeat_bin(123456u, (c & 1) != 0, &ack, b, sizeof b) &&
                  s1_codec_decode_heartbeat_bin(b, 8u, &up, &es, &got) && up == 123456u &&
                  es == ((c & 1) != 0) && got.seq == ack.seq && got.result == ack.result &&
                  got.revision == ack.revision,
              "hb map ack roundtrip");
    }
    ack.result = 4u;
    check(0u == s1_codec_encode_heartbeat_bin(1u, false, &ack, b, sizeof b), "hb map result 4 refused");
    /* Independent golden vector, written by hand from the layout:
     * format 1, estop 1, map result 2 (ACCEPTED) -> byte0 = 1 | 1<<2 | 2<<3 = 0x15;
     * uptime 0x01020304 little-endian; seq 5; revision 7; byte 7 = 0. */
    ack.seq = 5u;
    ack.result = 2u;
    ack.revision = 7u;
    s1_codec_encode_heartbeat_bin(0x01020304u, true, &ack, b, sizeof b);
    check(b[0] == 0x15u && b[1] == 0x04u && b[2] == 0x03u && b[3] == 0x02u && b[4] == 0x01u &&
              b[5] == 5u && b[6] == 7u && b[7] == 0u,
          "hb golden bytes 15 04 03 02 01 05 07 00");

    for (c = 0; c < 256; c++)
    {
        uint8_t p = (uint8_t)c;
        s1_codec_command_id_t id = s1_codec_parse_command_bin(&p, 1u);
        s1_codec_command_id_t want = (c >= 1 && c <= 4) ? (s1_codec_command_id_t)c : S1_CMD_NONE;
        if (id != want)
        {
            check(0, "command byte table");
            printf("   byte %d\n", c);
        }
    }
    check(1, "all 256 command bytes checked");
    {
        uint8_t two[2] = {1u, 0u};
        check(s1_codec_parse_command_bin(two, 2u) == S1_CMD_NONE, "cmd length 2");
        check(s1_codec_parse_command_bin(two, 0u) == S1_CMD_NONE, "cmd length 0");
        check(s1_codec_parse_command_bin(NULL, 1u) == S1_CMD_NONE, "cmd null");
    }
    {
        uint8_t p = 0;
        int id;
        for (id = 1; id <= 4; id++)
        {
            check(s1_codec_encode_command_bin((s1_codec_command_id_t)id, &p, 1u) == 1u &&
                      s1_codec_parse_command_bin(&p, 1u) == (s1_codec_command_id_t)id,
                  "cmd roundtrip");
        }
        check(s1_codec_encode_command_bin(S1_CMD_NONE, &p, 1u) == 0u, "encode NONE refused");
    }
}

static void test_random(void)
{
    int i;
    srand(12345);
    for (i = 0; i < 20000; i++)
    {
        s1_codec_frame_t f = good_frame();
        uint8_t b[29];
        s1_codec_bin_telemetry_t d;
        uint32_t oor;
        f.motor_speed_left = (rand() % 4096) - 2048;
        f.motor_speed_right = (rand() % 4096) - 2048;
        f.encoder_count_left = (int64_t)(int32_t)(((uint32_t)rand() << 16) ^ (uint32_t)rand());
        f.encoder_count_right = (int64_t)(int32_t)(((uint32_t)rand() << 16) ^ (uint32_t)rand());
        f.hump_peak_x10 = rand() % 4096;
        f.obstacle_distance_x10 = 20 + rand() % 3981;
        f.line_status = (uint8_t)(rand() % 8);
        f.robot_state = (uint8_t)(rand() % 8);
        f.timestamp_ms = ((uint32_t)rand() << 16) ^ (uint32_t)rand();
        f.encoder_timestamp_us = (uint64_t)f.timestamp_ms * 1000u - (uint64_t)(rand() % 1024) * 1000u;
        if (f.timestamp_ms < 1024u)
        {
            f.timestamp_ms += 2048u;
            f.encoder_timestamp_us = (uint64_t)f.timestamp_ms * 1000u;
        }
        f.sensor_age_us[0] = (uint32_t)(rand() % 1000) * 1000u;
        f.sensor_age_us[1] = (uint32_t)(rand() % 1000) * 1000u;
        f.sensor_age_us[2] = (uint32_t)(rand() % 1000) * 1000u;
        f.validity = (uint8_t)(rand() & 63);
        f.encoder_generation = 1u + (uint32_t)(rand() % 1000);
        if (s1_codec_encode_telemetry_bin(&f, (uint32_t)i, (uint8_t)rand(), b, sizeof b, &oor) != 29u || oor != 0u || !s1_codec_decode_telemetry_bin(b, 29u, &d) || ((f.validity & S1_VALID_SPEED) && (d.speed_left != f.motor_speed_left || d.speed_right != f.motor_speed_right)) || ((f.validity & S1_VALID_ENCODER) && (d.encoder_left != (int32_t)f.encoder_count_left || d.encoder_right != (int32_t)f.encoder_count_right)) || ((f.validity & S1_VALID_HUMP) && d.hump_x10 != f.hump_peak_x10) || ((f.validity & S1_VALID_OBSTACLE) && (d.obstacle_x10 != f.obstacle_distance_x10 || d.obstacle_age_ms != f.sensor_age_us[1] / 1000u)) || ((f.validity & S1_VALID_LINE) && (d.line_status != f.line_status || d.line_age_ms != f.sensor_age_us[0] / 1000u)) || ((f.validity & S1_VALID_IMU) && d.imu_age_ms != f.sensor_age_us[2] / 1000u) || d.robot_state != f.robot_state || d.timestamp_ms != f.timestamp_ms || d.validity != f.validity || d.sequence != (i & 0xFFF))
        {
            check(0, "random round trip");
            printf("   trial %d\n", i);
            return;
        }
    }
    check(1, "20000 random round trips");
}

int main(void)
{
    printf("T-codec-bin tests\n");
    test_roundtrip();
    test_layout();
    test_out_of_range();
    test_boundaries();
    test_reject();
    test_stamp();
    test_heartbeat_command();
    test_random();
    printf("codec_bin: %d checks, %d failed\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}