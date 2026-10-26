/**
 * @file s1_codec.h
 * @brief Buddy 1: pure message encoding and command parsing.
 *
 * NOTE: This module has no kernel, lwIP or libc dependency. Everything in it
 * can therefore be tested on a PC as well as on the Pico (testing/subsystem1).
 *
 * ASSUMPTION: Floating-point values are exchanged as fixed-point tenths
 * (value x10). The image is built soft-float and has no float printf, so
 * fixed point is cheaper and its JSON length is exactly bounded (NFR10).
 */

#ifndef S1_CODEC_H
#define S1_CODEC_H

#include <stdbool.h>
#include <stdint.h>

/** Size of the buffer that must be passed to the telemetry encoder. */
#define S1_CODEC_TELEMETRY_JSON_CAPACITY 256u

/** Size that is enough for any heartbeat message. */
#define S1_CODEC_HEARTBEAT_JSON_CAPACITY 48u
#define S1_CODEC_MAPPING_MAX_BYTES 47u /* Existing MQTT RX buffer is 48. */

/** Values for s1_codec_frame_t.robot_state (Section 5 statechart). */
typedef enum
{
    S1_ROBOT_STATE_INITIALISING     = 0,
    S1_ROBOT_STATE_READY            = 1,
    S1_ROBOT_STATE_OPERATIONAL      = 2,
    S1_ROBOT_STATE_PAUSED           = 3,
    S1_ROBOT_STATE_SAFE_STOP        = 4,
    S1_ROBOT_STATE_MISSION_COMPLETE = 5,
    S1_ROBOT_STATE_STOPPED          = 6,
    S1_ROBOT_STATE_FAULT            = 7
} s1_codec_robot_state_t;

/** Remote override commands for the Vehicle Controller (FR18). */
typedef enum
{
    S1_CMD_NONE = 0,
    S1_CMD_EMERGENCY_STOP,
    S1_CMD_START_RUN,
    S1_CMD_PAUSE,
    S1_CMD_RESUME,
    S1_CMD_MAPPING
} s1_codec_command_id_t;

/** One telemetry frame (FR17). Distances are in tenths of a centimetre. */
typedef struct
{
    uint32_t timestamp_ms; /**< Milliseconds since boot. */
    int64_t encoder_count_left;
    int64_t encoder_count_right;
    int32_t  hump_peak_x10;
    int32_t  obstacle_distance_x10;
    int32_t  motor_speed_left;
    int32_t  motor_speed_right;
    uint8_t  robot_state; /**< An s1_codec_robot_state_t value. */
    uint8_t  line_status; /**< Bit 0 left, bit 1 centre, bit 2 right. */
    /* Version 0 preserves professor/demo wire keys. Production uses version 2.
     * v2 counts are signed decimal STRINGS to preserve 64 bits in JSON clients.
     * t: kernel modulo-2^32 ms; et: full kernel us; eg: acquisition generation.
     * a: raw-source ages in us [line, obstacle, IMU], <=999999 when valid.
     * l: bit0 left front, bit1 right front, bit2 side; not left/centre/right.
     * Invalid observations encode null, never an invented zero measurement.
     */
    uint64_t encoder_timestamp_us;
    uint32_t encoder_generation;
    uint32_t sensor_age_us[3];
    uint8_t validity;
    uint8_t schema_version;
} s1_codec_frame_t;

#define S1_VALID_SPEED 1u
#define S1_VALID_ENCODER 2u
#define S1_VALID_LINE 4u
#define S1_VALID_HUMP 8u
#define S1_VALID_OBSTACLE 16u
#define S1_VALID_IMU 32u
#define S1_VALID_ALL 63u

/** One remote command as delivered to the Vehicle Controller. */
typedef struct
{
    s1_codec_command_id_t command_id;
    uint32_t              rx_timestamp_ms; /**< Arrival at Buddy 1. */
    /* Mapping slots: LEFT, RIGHT, FORWARD, UTURN. Copied, never pointers. */
    char                  mapping_letters[4];
} s1_codec_command_t;

/** @brief Parse a bounded MAP payload; preserve output on any error. */
bool s1_codec_parse_mapping (const char * p_payload, uint32_t length,
                             s1_codec_command_t * p_command);
/** @brief Validate four distinct uppercase letters in a mapping command. */
bool s1_codec_mapping_valid (const s1_codec_command_t * p_command);

/**
 * @brief Encode a telemetry frame as a JSON object (FR17).
 * @param p_frame  Frame to encode.
 * @param sequence Running counter, lets the laptop measure frame loss (NFR8).
 * @param p_buffer Output text, always NUL-terminated.
 * @param capacity Size of p_buffer. Use S1_CODEC_TELEMETRY_JSON_CAPACITY.
 * @return Length in bytes, or 0 if it did not fit (never a truncated object).
 */
uint32_t s1_codec_encode_telemetry (const s1_codec_frame_t * p_frame,
                                    uint32_t sequence, char * p_buffer,
                                    uint32_t capacity);

/**
 * @brief Encode the heartbeat message (FR20).
 * @return Length in bytes, or 0 if it did not fit.
 */
uint32_t s1_codec_encode_heartbeat (uint32_t uptime_ms, char * p_buffer,
                                    uint32_t capacity);

/**
 * @brief Turn a received payload into a command (FR18).
 * @param p_payload Text, not necessarily NUL-terminated.
 * @param length    Number of bytes in p_payload.
 * @return The command, or S1_CMD_NONE if the text is not a known command.
 */
s1_codec_command_id_t s1_codec_parse_command (const char * p_payload,
                                              uint32_t     length);

/** @brief Printable name of a command, or NONE. */
const char * s1_codec_command_name (s1_codec_command_id_t command_id);

#endif /* S1_CODEC_H */

/*** end of file ***/
