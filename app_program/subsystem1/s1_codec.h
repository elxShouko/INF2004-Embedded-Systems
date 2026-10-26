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
    S1_ROBOT_STATE_INITIALISING = 0,
    S1_ROBOT_STATE_READY = 1,
    S1_ROBOT_STATE_OPERATIONAL = 2,
    S1_ROBOT_STATE_PAUSED = 3,
    S1_ROBOT_STATE_SAFE_STOP = 4,
    S1_ROBOT_STATE_MISSION_COMPLETE = 5,
    S1_ROBOT_STATE_STOPPED = 6,
    S1_ROBOT_STATE_FAULT = 7
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

/** Indexes of s1_codec_frame_t.sensor_age_us (raw-source ages in us). */
typedef enum
{
    S1_AGE_LINE = 0,
    S1_AGE_OBSTACLE = 1,
    S1_AGE_IMU = 2
} s1_codec_age_index_t;

/** Value of s1_codec_frame_t.schema_version for the production JSON schema. */
#define S1_SCHEMA_V2 2u

/** Number of entries in s1_codec_frame_t.sensor_age_us. */
#define S1_AGE_COUNT 3u

/** One telemetry frame (FR17). Distances are in tenths of a centimetre. */
typedef struct
{
    uint32_t timestamp_ms; /**< Milliseconds since boot. */
    int64_t encoder_count_left;
    int64_t encoder_count_right;
    int32_t hump_peak_x10;
    int32_t obstacle_distance_x10;
    int32_t motor_speed_left;
    int32_t motor_speed_right;
    uint8_t robot_state; /**< An s1_codec_robot_state_t value. */
    uint8_t line_status; /**< Bit 0 left, bit 1 centre, bit 2 right. */
    /* Version 0 preserves professor/demo wire keys. Production uses version 2.
     * v2 counts are signed decimal STRINGS to preserve 64 bits in JSON clients.
     * t: kernel modulo-2^32 ms; et: full kernel us; eg: acquisition generation.
     * a: raw-source ages in us [line, obstacle, IMU], <=999999 when valid.
     * l: bit0 left front, bit1 right front, bit2 side; not left/centre/right.
     * Invalid observations encode null, never an invented zero measurement.
     */
    uint64_t encoder_timestamp_us;
    uint32_t encoder_generation;
    uint32_t sensor_age_us[S1_AGE_COUNT];
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
uint32_t s1_codec_encode_telemetry(const s1_codec_frame_t *p_frame,
                                   uint32_t sequence, char *p_buffer,
                                   uint32_t capacity);

/**
 * @brief Encode the heartbeat message (FR20).
 * @return Length in bytes, or 0 if it did not fit.
 */
uint32_t s1_codec_encode_heartbeat(uint32_t uptime_ms, char *p_buffer,
                                   uint32_t capacity);

/**
 * @brief Turn a received payload into a command (FR18).
 * @param p_payload Text, not necessarily NUL-terminated.
 * @param length    Number of bytes in p_payload.
 * @return The command, or S1_CMD_NONE if the text is not a known command.
 */
s1_codec_command_id_t s1_codec_parse_command(const char *p_payload,
                                             uint32_t length);

/** @brief Printable name of a command, or NONE. */
const char *s1_codec_command_name(s1_codec_command_id_t command_id);

/* ======================================================================= */
/* Wire format v3: minimum-bit binary
 * (docs/buddy1/MINIMUM_BIT_REPRESENTATION.md) */
/* (see the field table there). Little-endian, LSB-first. Fields are written
 * with masks and   */
/* shifts (s1_bits.h), never by casting a struct onto the buffer.            */
/* ======================================================================= */

/* Value of the 2-bit format field. */
#define S1_CODEC_BIN_FORMAT 1u

/* Telemetry frame size in bits. */
#define S1_CODEC_TELEMETRY_BIN_BITS 232u

/* Telemetry frame size in bytes. */
#define S1_CODEC_TELEMETRY_BIN_BYTES 29u

/* Heartbeat size in bytes. */
#define S1_CODEC_HEARTBEAT_BIN_BYTES 8u

/* Command size in bytes. */
#define S1_CODEC_COMMAND_BIN_BYTES 1u

/** Telemetry as the laptop sees it after decoding (also used by the tests). */
typedef struct
{
    uint16_t sequence;   /**< 12 bits, wraps at 4096. */
    uint8_t robot_state; /**< 0 to 7. */
    uint8_t validity;    /**< S1_VALID_* bits. */
    uint8_t fault_mask;  /**< S1_FAULT_* bits. */
    uint32_t timestamp_ms;
    int16_t speed_left; /**< -2048 to 2047 mm/s. */
    int16_t speed_right;
    int32_t encoder_left; /**< Signed 32-bit count (out-of-range counts are
                             rejected, not truncated). */
    int32_t encoder_right;
    uint8_t encoder_generation; /**< Low 8 bits. */
    uint16_t encoder_age_ms;    /**< 0 to 1023. */
    uint8_t line_status;        /**< 0 to 7. */
    uint16_t line_age_ms;       /**< 0 to 999. */
    uint16_t hump_x10;          /**< 0 to 4095. */
    uint16_t obstacle_x10;      /**< 20 to 4000 tenths of a cm. */
    uint16_t obstacle_age_ms;   /**< 0 to 999. */
    uint16_t imu_age_ms;        /**< 0 to 999. */
} s1_codec_bin_telemetry_t;

/**
 * @brief Pack a telemetry frame into exactly 29 bytes.
 * @param p_frame      Frame from the Vehicle Controller (unchanged struct).
 * @param sequence     Running frame counter; only the low 12 bits are sent.
 * @param fault_mask   S1_FAULT_* bits.
 * @param p_buffer     Output.
 * @param capacity     Must be >= S1_CODEC_TELEMETRY_BIN_BYTES.
 * @param p_out_of_range Optional. Receives how many field groups were out of
 *                     range and had their validity bit cleared (never clamped).
 * @return 29, or 0 if an argument is bad or robot_state is not 0 to 7.
 */
uint32_t s1_codec_encode_telemetry_bin(const s1_codec_frame_t *p_frame,
                                       uint32_t sequence, uint8_t fault_mask,
                                       uint8_t *p_buffer, uint32_t capacity,
                                       uint32_t *p_out_of_range);

/**
 * @brief Overwrite the sequence (low 12 bits) and the fault mask of an
 *        already packed telemetry frame. The Wi-Fi task calls this when the
 *        frame leaves the buffer, so both values are those of the moment of
 *        sending. Nothing else in the frame changes.
 * @return false if p_buffer is NULL or length is not 29.
 */
bool s1_codec_telemetry_bin_stamp(uint8_t *p_buffer, uint32_t length,
                                  uint32_t sequence, uint8_t fault_mask);

/** @brief Unpack a telemetry frame. false if length, format or reserved bits
 * are wrong. */
bool s1_codec_decode_telemetry_bin(const uint8_t *p_buffer, uint32_t length,
                                   s1_codec_bin_telemetry_t *p_out);

/** Result of the latest mapping update, reported in the heartbeat. */
typedef enum
{
    S1_MAP_RESULT_NONE = 0,     /**< No update seen since boot. */
    S1_MAP_RESULT_QUEUED = 1,   /**< s3_submit() returned E_OK: queued ONLY. */
    S1_MAP_RESULT_ACCEPTED = 2, /**< Buddy 3 replied accepted = 1. */
    S1_MAP_RESULT_REJECTED =
        3 /**< Bad text, queue full, timeout or S3 reply accepted = 0. */
} s1_codec_map_result_t;

/** Acknowledgement of the latest mapping update (2 bits + 8 bits + 8 bits). */
typedef struct
{
    uint8_t seq;    /**< Low 8 bits of the update counter (1, 2, ...). */
    uint8_t result; /**< An s1_codec_map_result_t value (0 to 3). */
    uint8_t
        revision; /**< Low 8 bits of Buddy 3's map revision (ACCEPTED only). */
} s1_codec_map_ack_t;

/**
 * @brief Pack the 8-byte heartbeat (FR20).
 * @param p_ack Latest mapping acknowledgement, or NULL for "none".
 * @return 8, or 0 on a bad argument (result above 3 included).
 */
uint32_t s1_codec_encode_heartbeat_bin(uint32_t uptime_ms,
                                       bool b_estop_latched,
                                       const s1_codec_map_ack_t *p_ack,
                                       uint8_t *p_buffer, uint32_t capacity);

/**
 * @brief Unpack a heartbeat. false if length, format or reserved bits are
 * wrong.
 * @param p_ack Optional (NULL allowed): receives the mapping acknowledgement.
 */
bool s1_codec_decode_heartbeat_bin(const uint8_t *p_buffer, uint32_t length,
                                   uint32_t *p_uptime_ms,
                                   bool *p_estop_latched,
                                   s1_codec_map_ack_t *p_ack);

/**
 * @brief Parse the 1-byte command.
 * @return The command, or S1_CMD_NONE if length != 1, the id is not 1 to 4,
 *         or any reserved bit (3 to 7) is set.
 */
s1_codec_command_id_t s1_codec_parse_command_bin(const uint8_t *p_payload,
                                                 uint32_t length);

/** @brief Pack a command into 1 byte (used by tests and the laptop sender). */
uint32_t s1_codec_encode_command_bin(s1_codec_command_id_t command_id,
                                     uint8_t *p_buffer, uint32_t capacity);

#endif /* S1_CODEC_H */

/*** end of file ***/