/* Host (PC) unit tests for s1_codec. Build and run with run_host_tests.sh. */
#include <stdio.h>
#include <string.h>

#include "../../../app_program/subsystem1/s1_codec.h"

static int g_checks = 0;
static int g_failures = 0;

static void
check (int b_ok, const char * p_name)
{
    g_checks++;
    if (!b_ok)
    {
        g_failures++;
        printf ("  FAIL %s\n", p_name);
    }
}

static void
test_telemetry (void)
{
    char             json[S1_CODEC_TELEMETRY_JSON_CAPACITY];
    s1_codec_frame_t frame = { 123456u, 1234567u, 1234560u, 12, 455, 120, -120, 2u, 2u, 0u, 0u, {0u,0u,0u}, 0u, 0u};
    const char *     keys[] = { "seq", "timestamp", "robot_state", "left_motor_speed",
                                "right_motor_speed", "left_encoder_count",
                                "right_encoder_count", "line_status", "hump_peak",
                                "obstacle_distance" };
    char             needle[40];
    unsigned         k;
    uint32_t         length = s1_codec_encode_telemetry (&frame, 7u, json, sizeof (json));

    printf ("T-codec-1 typical: %s\n", json);
    check (length == strlen (json) && length > 0u, "length matches");
    for (k = 0u; k < (sizeof (keys) / sizeof (keys[0])); k++)
    {
        snprintf (needle, sizeof (needle), "\"%s\":", keys[k]);
        check (strstr (json, needle) != NULL, keys[k]);
    }
    check (strstr (json, "\"hump_peak\":1.2,") != NULL, "tenths 12 -> 1.2");
    check (strstr (json, "\"obstacle_distance\":45.5}") != NULL, "tenths 455 -> 45.5");
    check (strstr (json, "\"right_motor_speed\":-120,") != NULL, "negative speed");
}

static void
test_worst_case (void)
{
    char             json[S1_CODEC_TELEMETRY_JSON_CAPACITY];
    s1_codec_frame_t frame = { 4294967295u, 4294967295u, 4294967295u, -2000000000,
                               -2000000000, -32768, -32768, 255u, 255u, 0u, 0u, {0u,0u,0u}, 0u, 0u};
    uint32_t         length = s1_codec_encode_telemetry (&frame, 4294967295u, json, sizeof (json));

    printf ("T-codec-2 worst case (%u bytes): %s\n", (unsigned) length, json);
    check (length > 0u, "worst case fits");
    check (length <= 255u, "NFR10: worst case under 256 bytes");
    check (strstr (json, "\"hump_peak\":-9999.9,") != NULL, "clamped to -9999.9");
}

static void
test_overflow (void)
{
    char             small[40];
    s1_codec_frame_t frame = { 1u, 2u, 3u, 4, 5, 6, 7, 1u, 1u, 0u, 0u, {0u,0u,0u}, 0u, 0u};

    check (s1_codec_encode_telemetry (&frame, 1u, small, sizeof (small)) == 0u,
           "too small buffer -> 0, never truncated JSON");
    check (small[0] == '\0', "buffer is an empty string after overflow");
    check (s1_codec_encode_telemetry (&frame, 1u, small, 0u) == 0u, "zero capacity");
}

static void
test_heartbeat (void)
{
    char json[S1_CODEC_HEARTBEAT_JSON_CAPACITY];

    s1_codec_encode_heartbeat (74829u, json, sizeof (json));
    printf ("T-codec-3 heartbeat: %s\n", json);
    check (strcmp (json, "{\"status\":\"ALIVE\",\"uptime_ms\":74829}") == 0, "FR20 format");
    check (s1_codec_encode_heartbeat (4294967295u, json, sizeof (json)) > 0u, "max uptime fits");
}

static void
test_commands (void)
{
    check (s1_codec_parse_command ("EMERGENCY_STOP", 14u) == S1_CMD_EMERGENCY_STOP, "ESTOP");
    check (s1_codec_parse_command (" start_run\n", 11u) == S1_CMD_START_RUN, "lowercase, blanks");
    check (s1_codec_parse_command ("\"PAUSE\"", 7u) == S1_CMD_PAUSE, "quoted");
    check (s1_codec_parse_command ("RESUME", 6u) == S1_CMD_RESUME, "resume");
    check (s1_codec_parse_command ("PAUSE_RUN", 9u) == S1_CMD_PAUSE, "alias");
    check (s1_codec_parse_command ("SELF_DESTRUCT", 13u) == S1_CMD_NONE, "unknown");
    check (s1_codec_parse_command ("", 0u) == S1_CMD_NONE, "empty");
    check (s1_codec_parse_command ("EMERGENCY_STOPX", 15u) == S1_CMD_NONE, "no prefix match");
    check (s1_codec_parse_command ("EMERGENCY_STOP", 9u) == S1_CMD_NONE, "length is respected");
    check (s1_codec_parse_command ("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 36u) == S1_CMD_NONE, "overlong");
    check (strcmp (s1_codec_command_name (S1_CMD_PAUSE), "PAUSE") == 0, "name canonical");
}

int
main (void)
{
    test_telemetry ();
    test_worst_case ();
    test_overflow ();
    test_heartbeat ();
    test_commands ();
    {
        s1_codec_frame_t frame = {0};
        char json[S1_CODEC_TELEMETRY_JSON_CAPACITY];

        frame.schema_version = 2u;
        frame.validity = S1_VALID_ALL;
        frame.timestamp_ms = UINT32_MAX;
        frame.encoder_count_left = INT64_MIN;
        frame.encoder_count_right = INT64_MAX;
        frame.encoder_timestamp_us = UINT64_MAX;
        frame.encoder_generation = UINT32_MAX;
        frame.motor_speed_left = INT32_MIN;
        frame.motor_speed_right = INT32_MAX;
        frame.robot_state = S1_ROBOT_STATE_FAULT;
        frame.line_status = 7u;
        frame.hump_peak_x10 = 99999;
        frame.obstacle_distance_x10 = 4000;
        frame.sensor_age_us[0] = 999999u;
        frame.sensor_age_us[1] = 999999u;
        frame.sensor_age_us[2] = 999999u;
        uint32_t length = s1_codec_encode_telemetry(&frame,UINT32_MAX,
            json,sizeof(json));
        check(length > 0u && length < 256u,"v2 worst-case payload bound");
        check(strstr(json,"\"el\":\"-9223372036854775808\"") != NULL,
            "v2 negative full precision");
        check(strstr(json,"\"er\":\"9223372036854775807\"") != NULL,
            "v2 positive full precision");
        check(strstr(json,"18446744073709551615") != NULL,
            "v2 full source timestamp");
        uint32_t maximum = 0u;
        bool bounded = true;

        frame.encoder_count_right = INT64_MIN;
        frame.motor_speed_right = INT32_MIN;
        for (uint8_t mask = 0u; mask <= S1_VALID_ALL; ++mask)
        {
            frame.validity = mask;
            length = s1_codec_encode_telemetry(&frame,UINT32_MAX,json,sizeof(json));
            if ((0u != (mask & S1_VALID_HUMP)) &&
                (0u == (mask & S1_VALID_IMU)))
            {
                bounded = bounded && (0u == length);
            }
            else
            {
                bounded = bounded && (length > 0u) && (length < 256u);
                if (length > maximum) { maximum = length; }
            }
        }
        (void)printf("T-codec-v2 maximum across validity masks: %u bytes\n",
            (unsigned)maximum);
        check(bounded && (239u == maximum),
            "v2 full negative widths and every validity/null combination bounded");
        frame.validity = 0u;
        check(s1_codec_encode_telemetry(&frame,0u,json,sizeof(json)) > 0u &&
            strstr(json,"\"el\":null") != NULL &&
            strstr(json,"\"h\":null") != NULL,"v2 unknown values null");
        frame.validity = S1_VALID_LINE;
        frame.sensor_age_us[0] = 1000000u;
        check(s1_codec_encode_telemetry(&frame,0u,json,sizeof(json)) == 0u &&
            json[0] == '\0',"v2 invalid numeric age rejected");
        check(s1_codec_encode_telemetry(NULL,0u,json,sizeof(json)) == 0u,
            "null frame rejected");
    }
    printf ("\n%d checks, %d failed\n", g_checks, g_failures);
    return (0 == g_failures) ? 0 : 1;
}
