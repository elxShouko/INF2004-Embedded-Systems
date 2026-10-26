/* Message sizes before (JSON) and after (binary), measured with the real
 * encoders. Build and run on the PC: the output does not depend on the CPU. */
#include <stdio.h>
#include <string.h>

#include "../../../app_program/subsystem1/s1_codec.h"

static void typical(s1_codec_frame_t *f)
{
    memset(f, 0, sizeof *f);
    f->timestamp_ms = 123456u;
    f->robot_state = S1_ROBOT_STATE_OPERATIONAL;
    f->line_status = 2u;
    f->motor_speed_left = 220;
    f->motor_speed_right = 215;
    f->encoder_count_left = 48210;
    f->encoder_count_right = 48007;
    f->encoder_generation = 1234u;
    f->encoder_timestamp_us = 123450000u;
    f->hump_peak_x10 = 35;
    f->obstacle_distance_x10 = 850;
    f->sensor_age_us[0] = 4000u;
    f->sensor_age_us[1] = 12000u;
    f->sensor_age_us[2] = 8000u;
    f->validity = S1_VALID_ALL;
    f->schema_version = 2u;
}

int main(void)
{
    s1_codec_frame_t f;
    char json[S1_CODEC_TELEMETRY_JSON_CAPACITY];
    uint8_t bin[S1_CODEC_TELEMETRY_BIN_BYTES];
    uint32_t jt, jw, jh, jc;
    char hb[S1_CODEC_HEARTBEAT_JSON_CAPACITY];

    typical(&f);
    jt = s1_codec_encode_telemetry(&f, 1234u, json, sizeof json);
    printf("telemetry JSON, typical frame (v2)      : %3u bytes\n", jt);
    /* Worst case: every digit position used. */
    f.timestamp_ms = 0xFFFFFFFFu;
    f.encoder_count_left = -9223372036854775807ll - 1;
    f.encoder_count_right = -9223372036854775807ll - 1;
    f.encoder_timestamp_us = 0xFFFFFFFFFFFFFFFFull;
    f.encoder_generation = 0xFFFFFFFFu;
    f.motor_speed_left = -2048;
    f.motor_speed_right = -2048;
    f.hump_peak_x10 = 4095;
    f.obstacle_distance_x10 = 4000;
    f.sensor_age_us[0] = f.sensor_age_us[1] = f.sensor_age_us[2] = 999999u;
    jw = s1_codec_encode_telemetry(&f, 0xFFFFFFFFu, json, sizeof json);
    printf("telemetry JSON, worst-case digits       : %3u bytes (NFR10 limit 256; 0 = did not fit)\n", jw);
    typical(&f);
    printf("telemetry binary (every frame)          : %3u bytes\n",
           s1_codec_encode_telemetry_bin(&f, 1234u, 0u, bin, sizeof bin, NULL));
    jh = s1_codec_encode_heartbeat(1234567u, hb, sizeof hb);
    printf("heartbeat JSON                          : %3u bytes\n", jh);
    printf("heartbeat binary                        : %3u bytes\n", (unsigned)S1_CODEC_HEARTBEAT_BIN_BYTES);
    jc = (uint32_t)strlen("EMERGENCY_STOP");
    printf("command JSON/text EMERGENCY_STOP        : %3u bytes\n", jc);
    printf("command binary                          : %3u byte\n", (unsigned)S1_CODEC_COMMAND_BIN_BYTES);
    printf("telemetry saving vs typical JSON        : %.0f %%\n", 100.0 - 100.0 * 29.0 / (double)jt);
    printf("MQTT PUBLISH on robot/telemetry (fixed header 2 + topic 2+15 + payload):\n");
    printf("  JSON typical %u bytes, binary %u bytes, per second at 10 Hz: %u vs %u\n", 2u + 17u + jt, 2u + 17u + 29u,
           (2u + 17u + jt) * 10u, (2u + 17u + 29u) * 10u);
    return 0;
}