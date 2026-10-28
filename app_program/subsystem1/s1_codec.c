/**
 * @file s1_codec.c
 * @brief Buddy 1: pure message encoding and command parsing.
 *
 * NOTE: No kernel, lwIP or libc functions are used, so the same file is
 * compiled for the Pico and for the PC unit tests.
 */

#include "s1_codec.h"
#include "s1_bits.h"
#include <stddef.h>

#define DECIMAL_BASE 10u
#define U32_DIGITS_MAX 10u
/* Longest unsigned 64-bit decimal number: 20 digits. */
#define U64_DIGITS_MAX 20u
#define TENTHS_LIMIT 99999 /* +/- 9999.9: bounds the JSON length (NFR10). */
#define COMMAND_TOKEN_LENGTH 24u
#define LOWER_TO_UPPER_OFFSET ('a' - 'A')

typedef struct
{
    char *p_buffer;
    uint32_t capacity;
    uint32_t length;
    bool b_overflow;
} text_writer_t;

typedef struct
{
    const char *p_name;
    s1_codec_command_id_t command_id;
} command_entry_t;

/* NOTE: The canonical name of each command is listed before its aliases. */
static const command_entry_t g_command_table[] = {
    {"EMERGENCY_STOP", S1_CMD_EMERGENCY_STOP},
    {"START_RUN", S1_CMD_START_RUN},
    {"PAUSE", S1_CMD_PAUSE},
    {"RESUME", S1_CMD_RESUME},
    {"PAUSE_RUN", S1_CMD_PAUSE},
    {"RESUME_RUN", S1_CMD_RESUME}};

#define COMMAND_TABLE_COUNT \
    (sizeof(g_command_table) / sizeof(g_command_table[0]))

static void writer_start(text_writer_t *p_writer, char *p_buffer,
                         uint32_t capacity);
static void writer_char(text_writer_t *p_writer, char letter);
static void writer_text(text_writer_t *p_writer, const char *p_text);
static void writer_u32(text_writer_t *p_writer, uint32_t value);
static void writer_i64(text_writer_t *p_writer, int64_t value);
static void writer_u64(text_writer_t *p_writer, uint64_t value);
static uint32_t encode_v2(const s1_codec_frame_t *p_frame, uint32_t sequence,
                          char *p_buffer, uint32_t capacity);
static void writer_i32(text_writer_t *p_writer, int32_t value);
static void writer_tenths(text_writer_t *p_writer, int32_t value_x10);
static uint32_t writer_finish(text_writer_t *p_writer);
static char to_upper_ascii(char letter);
static bool is_blank(char letter);
static bool is_token_end(char letter);
static bool text_equal(const char *p_first, const char *p_second);

/* ---------------------------------------------------------------------- */
/* Public functions                                                       */
/* ---------------------------------------------------------------------- */

/* Encode one telemetry frame. Field names follow FR17. The extra sequence
 * number lets the laptop measure frame loss (NFR8). The worst-case length is
 * 243 bytes, so the JSON always fits NFR10 (256 bytes). */
uint32_t
s1_codec_encode_telemetry(const s1_codec_frame_t *p_frame, uint32_t sequence,
                          char *p_buffer, uint32_t capacity)
{
    text_writer_t writer = {0};

    if ((NULL == p_frame) || (NULL == p_buffer))
    {
        return 0u;
    }
    if (S1_SCHEMA_V2 == p_frame->schema_version)
    {
        return encode_v2(p_frame, sequence, p_buffer, capacity);
    }
    if ((0u != p_frame->schema_version) || (p_frame->encoder_count_left < 0) ||
        (p_frame->encoder_count_left > UINT32_MAX) ||
        (p_frame->encoder_count_right < 0) ||
        (p_frame->encoder_count_right > UINT32_MAX) ||
        (p_frame->motor_speed_left < INT16_MIN) ||
        (p_frame->motor_speed_left > INT16_MAX) ||
        (p_frame->motor_speed_right < INT16_MIN) ||
        (p_frame->motor_speed_right > INT16_MAX))
    {
        if (capacity > 0u)
        {
            p_buffer[0] = '\0';
        }
        return 0u;
    }
    writer_start(&writer, p_buffer, capacity);
    writer_text(&writer, "{\"seq\":");
    writer_u32(&writer, sequence);
    writer_text(&writer, ",\"timestamp\":");
    writer_u32(&writer, p_frame->timestamp_ms);
    writer_text(&writer, ",\"robot_state\":");
    writer_u32(&writer, p_frame->robot_state);
    writer_text(&writer, ",\"left_motor_speed\":");
    writer_i32(&writer, p_frame->motor_speed_left);
    writer_text(&writer, ",\"right_motor_speed\":");
    writer_i32(&writer, p_frame->motor_speed_right);
    writer_text(&writer, ",\"left_encoder_count\":");
    writer_u32(&writer, (uint32_t)p_frame->encoder_count_left);
    writer_text(&writer, ",\"right_encoder_count\":");
    writer_u32(&writer, (uint32_t)p_frame->encoder_count_right);
    writer_text(&writer, ",\"line_status\":");
    writer_u32(&writer, p_frame->line_status);
    writer_text(&writer, ",\"hump_peak\":");
    writer_tenths(&writer, p_frame->hump_peak_x10);
    writer_text(&writer, ",\"obstacle_distance\":");
    writer_tenths(&writer, p_frame->obstacle_distance_x10);
    writer_text(&writer, "}");
    return writer_finish(&writer);
}

/* Encode the heartbeat message. */
uint32_t
s1_codec_encode_heartbeat(uint32_t uptime_ms, char *p_buffer,
                          uint32_t capacity)
{
    text_writer_t writer = {0};

    if (NULL == p_buffer)
    {
        return 0u;
    }
    writer_start(&writer, p_buffer, capacity);
    writer_text(&writer, "{\"status\":\"ALIVE\",\"uptime_ms\":");
    writer_u32(&writer, uptime_ms);
    writer_text(&writer, "}");
    return writer_finish(&writer);
}

/* Turn a payload such as " start_run" into a command identifier. */
s1_codec_command_id_t
s1_codec_parse_command(const char *p_payload, uint32_t length)
{
    char token[COMMAND_TOKEN_LENGTH] = {0};
    uint32_t position = 0u;
    uint32_t token_length = 0u;
    uint32_t entry_number = 0u;
    s1_codec_command_id_t command_id = S1_CMD_NONE;

    if (NULL == p_payload)
    {
        return S1_CMD_NONE;
    }
    while ((position < length) && is_blank(p_payload[position]))
    {
        position++;
    }
    while ((position < length) && (!is_token_end(p_payload[position])) &&
           (token_length < (COMMAND_TOKEN_LENGTH - 1u)))
    {
        token[token_length] = to_upper_ascii(p_payload[position]);
        token_length++;
        position++;
    }

    for (entry_number = 0u; entry_number < COMMAND_TABLE_COUNT; entry_number++)
    {
        if (text_equal(token, g_command_table[entry_number].p_name))
        {
            command_id = g_command_table[entry_number].command_id;
            break;
        }
    }
    return command_id;
}

/* Return the canonical name of a command, or NONE if it has no name. */
const char *
s1_codec_command_name(s1_codec_command_id_t command_id)
{
    const char *p_name = "NONE";
    uint32_t entry_number = 0u;

    for (entry_number = 0u; entry_number < COMMAND_TABLE_COUNT; entry_number++)
    {
        if (command_id == g_command_table[entry_number].command_id)
        {
            p_name = g_command_table[entry_number].p_name;
            break;
        }
    }
    return p_name;
}

/* ---------------------------------------------------------------------- */
/* Private functions                                                      */
/* ---------------------------------------------------------------------- */

/* Prepare a writer on a buffer and make it an empty string. */
static void
writer_start(text_writer_t *p_writer, char *p_buffer, uint32_t capacity)
{
    p_writer->p_buffer = p_buffer;
    p_writer->capacity = capacity;
    p_writer->length = 0u;
    p_writer->b_overflow = (0u == capacity);
    if (0u != capacity)
    {
        p_buffer[0] = '\0';
    }
}

/* Append one character, keeping room for the terminator. */
static void
writer_char(text_writer_t *p_writer, char letter)
{
    if ((!p_writer->b_overflow) &&
        ((p_writer->length + 1u) < p_writer->capacity))
    {
        p_writer->p_buffer[p_writer->length] = letter;
        p_writer->length++;
        p_writer->p_buffer[p_writer->length] = '\0';
    }
    else
    {
        p_writer->b_overflow = true;
    }
}

/* Append a string. */
static void
writer_text(text_writer_t *p_writer, const char *p_text)
{
    const char *p_cursor = p_text;

    while ('\0' != *p_cursor)
    {
        writer_char(p_writer, *p_cursor);
        p_cursor++;
    }
}

/* Append an unsigned number in decimal. */
static void
writer_u32(text_writer_t *p_writer, uint32_t value)
{
    char digits[U32_DIGITS_MAX] = {0};
    uint32_t count = 0u;
    uint32_t remaining = value;

    do
    {
        digits[count] = (char)('0' + (remaining % DECIMAL_BASE));
        remaining = remaining / DECIMAL_BASE;
        count++;
    } while (remaining > 0u);

    while (count > 0u)
    {
        count--;
        writer_char(p_writer, digits[count]);
    }
}

/* Append a signed number in decimal. */
static void
writer_i32(text_writer_t *p_writer, int32_t value)
{
    uint32_t magnitude = (uint32_t)value;

    if (value < 0)
    {
        writer_char(p_writer, '-');
        magnitude = 0u - magnitude; /* Two's complement magnitude. */
    }
    writer_u32(p_writer, magnitude);
}

/* Append a value given in tenths as text such as -12.5. The value is
 * limited to +/- 9999.9 so the JSON length stays bounded. */
static void
writer_tenths(text_writer_t *p_writer, int32_t value_x10)
{
    int32_t limited = value_x10;
    uint32_t magnitude = 0u;

    if (limited > TENTHS_LIMIT)
    {
        limited = TENTHS_LIMIT;
    }
    else if (limited < -TENTHS_LIMIT)
    {
        limited = -TENTHS_LIMIT;
    }
    else
    {
        limited = value_x10;
    }

    if (limited < 0)
    {
        writer_char(p_writer, '-');
        magnitude = 0u - (uint32_t)limited;
    }
    else
    {
        magnitude = (uint32_t)limited;
    }
    writer_u32(p_writer, magnitude / DECIMAL_BASE);
    writer_char(p_writer, '.');
    writer_u32(p_writer, magnitude % DECIMAL_BASE);
}

/* Return the length, or 0 (and an empty string) if anything did not fit. */
static uint32_t
writer_finish(text_writer_t *p_writer)
{
    uint32_t length = p_writer->length;

    if (p_writer->b_overflow)
    {
        length = 0u;
        if (0u != p_writer->capacity)
        {
            p_writer->p_buffer[0] = '\0';
        }
    }
    return length;
}

/* Convert an ASCII lower-case letter to upper case. Others pass unchanged. */
static char
to_upper_ascii(char letter)
{
    char result = letter;

    if ((letter >= 'a') && (letter <= 'z'))
    {
        result = letter - LOWER_TO_UPPER_OFFSET;
    }
    return result;
}

/* True for characters skipped before a command word: blanks and quotes. */
static bool
is_blank(char letter)
{
    return ((' ' == letter) || ('\t' == letter) || ('\r' == letter) ||
            ('\n' == letter) || ('"' == letter));
}

/* True for the characters that end a command word. */
static bool
is_token_end(char letter)
{
    return (('\0' == letter) || is_blank(letter));
}

/* Compare two NUL-terminated strings. */
static bool
text_equal(const char *p_first, const char *p_second)
{
    const char *p_left = p_first;
    const char *p_right = p_second;

    while (('\0' != *p_left) && (*p_left == *p_right))
    {
        p_left++;
        p_right++;
    }
    return (*p_left == *p_right);
}

/* Limits and field positions of the JSON version 2 (debug) encoder. */
#define V2_LINE_MASK_MAX 7u
#define V2_OBSTACLE_MIN_X10 20
#define V2_OBSTACLE_MAX_X10 4000
#define V2_AGE_MAX_US 999999u
#define V2_FIELD_COUNT 7u
#define V2_FIELD_SPEED_RIGHT 1u
#define V2_FIELD_ENCODER_RIGHT 3u
#define V2_FIELD_LINE 4u
#define V2_FIELD_HUMP 5u

static const char *const g_v2_names[V2_FIELD_COUNT] = {
    ",\"ml\":", ",\"mr\":", ",\"el\":", ",\"er\":",
    ",\"l\":", ",\"h\":", ",\"o\":"};

static const uint8_t g_v2_masks[V2_FIELD_COUNT] = {
    S1_VALID_SPEED, S1_VALID_SPEED, S1_VALID_ENCODER, S1_VALID_ENCODER,
    S1_VALID_LINE, S1_VALID_HUMP, S1_VALID_OBSTACLE};

static const uint8_t g_v2_age_masks[S1_AGE_COUNT] = {
    S1_VALID_LINE, S1_VALID_OBSTACLE, S1_VALID_IMU};

/* True if the frame's flags and values can be written as version 2. */
static bool
v2_frame_ok(const s1_codec_frame_t *p_frame)
{
    const uint8_t valid = p_frame->validity;

    return !(
        (p_frame->robot_state > S1_ROBOT_STATE_FAULT) ||
        (0u != (valid & (uint8_t)~S1_VALID_ALL)) ||
        (((valid & S1_VALID_LINE) != 0u) &&
         (p_frame->line_status > V2_LINE_MASK_MAX)) ||
        (((valid & S1_VALID_HUMP) != 0u) &&
         ((p_frame->hump_peak_x10 < 0) ||
          (p_frame->hump_peak_x10 > TENTHS_LIMIT))) ||
        (((valid & S1_VALID_OBSTACLE) != 0u) &&
         ((p_frame->obstacle_distance_x10 < V2_OBSTACLE_MIN_X10) ||
          (p_frame->obstacle_distance_x10 > V2_OBSTACLE_MAX_X10))) ||
        (((valid & S1_VALID_HUMP) != 0u) && ((valid & S1_VALID_IMU) == 0u)) ||
        (((valid & S1_VALID_ENCODER) != 0u) &&
         (0u == p_frame->encoder_generation)));
}

/* Write one value of the seven (speeds, counts, line, hump, obstacle). */
static void
v2_write_field(text_writer_t *p_writer, const s1_codec_frame_t *p_frame,
               uint32_t field)
{
    writer_text(p_writer, g_v2_names[field]);
    if (0u == (p_frame->validity & g_v2_masks[field]))
    {
        writer_text(p_writer, "null");
    }
    else if (field <= V2_FIELD_SPEED_RIGHT)
    {
        writer_i32(p_writer, (0u == field) ? p_frame->motor_speed_left
                                           : p_frame->motor_speed_right);
    }
    else if (field <= V2_FIELD_ENCODER_RIGHT)
    {
        writer_char(p_writer, '\"');
        writer_i64(p_writer, (V2_FIELD_ENCODER_RIGHT == field)
                                 ? p_frame->encoder_count_right
                                 : p_frame->encoder_count_left);
        writer_char(p_writer, '\"');
    }
    else if (V2_FIELD_LINE == field)
    {
        writer_u32(p_writer, p_frame->line_status);
    }
    else
    {
        writer_tenths(p_writer, (V2_FIELD_HUMP == field)
                                    ? p_frame->hump_peak_x10
                                    : p_frame->obstacle_distance_x10);
    }
}

/* Write the three source ages. False if a valid age is above the limit. */
static bool
v2_write_ages(text_writer_t *p_writer, const s1_codec_frame_t *p_frame)
{
    bool b_ok = true;
    uint32_t source = 0u;

    for (source = 0u; b_ok && (source < S1_AGE_COUNT); ++source)
    {
        if (source > 0u)
        {
            writer_char(p_writer, ',');
        }
        if (0u == (p_frame->validity & g_v2_age_masks[source]))
        {
            writer_text(p_writer, "null");
        }
        else if (p_frame->sensor_age_us[source] > V2_AGE_MAX_US)
        {
            b_ok = false;
        }
        else
        {
            writer_u32(p_writer, p_frame->sensor_age_us[source]);
        }
    }
    return b_ok;
}

/* Compact versioned production schema keeps complete int64 counts plus source
 * metadata under the project's 256-byte payload limit. See the contract. */
static uint32_t
encode_v2(const s1_codec_frame_t *p_frame, uint32_t sequence, char *p_buffer,
          uint32_t capacity)
{
    text_writer_t writer = {0};
    uint32_t field = 0u;

    writer_start(&writer, p_buffer, capacity);
    if (!v2_frame_ok(p_frame))
    {
        return 0u;
    }
    writer_text(&writer, "{\"z\":2,\"q\":");
    writer_u32(&writer, sequence);
    writer_text(&writer, ",\"t\":");
    writer_u32(&writer, p_frame->timestamp_ms);
    writer_text(&writer, ",\"s\":");
    writer_u32(&writer, p_frame->robot_state);
    for (field = 0u; field < V2_FIELD_COUNT; ++field)
    {
        v2_write_field(&writer, p_frame, field);
    }
    writer_text(&writer, ",\"v\":");
    writer_u32(&writer, p_frame->validity);
    writer_text(&writer, ",\"et\":");
    if (0u != (p_frame->validity & S1_VALID_ENCODER))
    {
        writer_char(&writer, '\"');
        writer_u64(&writer, p_frame->encoder_timestamp_us);
        writer_char(&writer, '\"');
        writer_text(&writer, ",\"eg\":");
        writer_u32(&writer, p_frame->encoder_generation);
    }
    else
    {
        writer_text(&writer, "null,\"eg\":null");
    }
    writer_text(&writer, ",\"a\":[");
    if (!v2_write_ages(&writer, p_frame))
    {
        if (capacity > 0u)
        {
            p_buffer[0] = '\0';
        }
        return 0u;
    }
    writer_text(&writer, "]}");
    return writer_finish(&writer);
}

static void
writer_u64(text_writer_t *p_writer, uint64_t value)
{
    char digits[U64_DIGITS_MAX];
    uint32_t count = 0u;

    do
    {
        digits[count++] = (char)('0' + (value % DECIMAL_BASE));
        value /= DECIMAL_BASE;
    } while (value > 0u);
    while (count > 0u)
    {
        writer_char(p_writer, digits[--count]);
    }
}

static void
writer_i64(text_writer_t *p_writer, int64_t value)
{
    uint64_t magnitude = (uint64_t)value;

    if (value < 0)
    {
        writer_char(p_writer, '-');
        magnitude = 0u - magnitude; /* Includes INT64_MIN without signed UB. */
    }
    writer_u64(p_writer, magnitude);
}

/* ---------------------------------------------------------------------- */
/* Binary wire format v3 (docs/buddy1/MINIMUM_BIT_REPRESENTATION.md)      */
/* ---------------------------------------------------------------------- */

/* Start bit and width of each telemetry field. */
#define TB_FORMAT_START 0u
#define TB_FORMAT_WIDTH 2u
#define TB_SEQ_START 2u
#define TB_SEQ_WIDTH 12u
#define TB_STATE_START 14u
#define TB_STATE_WIDTH 3u
#define TB_VALID_START 17u
#define TB_VALID_WIDTH 6u
#define TB_FAULT_START 23u
#define TB_FAULT_WIDTH 8u
#define TB_TIME_START 31u
#define TB_TIME_WIDTH 32u
#define TB_SPEED_L_START 63u
#define TB_SPEED_R_START 75u
#define TB_SPEED_WIDTH 12u
#define TB_ENC_L_START 87u
#define TB_ENC_R_START 119u
#define TB_ENC_WIDTH 32u
#define TB_GEN_START 151u
#define TB_GEN_WIDTH 8u
#define TB_ENC_AGE_START 159u
#define TB_ENC_AGE_WIDTH 10u
#define TB_LINE_START 169u
#define TB_LINE_WIDTH 3u
#define TB_LINE_AGE_START 172u
#define TB_LINE_AGE_WIDTH 10u
#define TB_HUMP_START 182u
#define TB_HUMP_WIDTH 12u
#define TB_OBST_START 194u
#define TB_OBST_WIDTH 12u
#define TB_OBST_AGE_START 206u
#define TB_OBST_AGE_WIDTH 10u
#define TB_IMU_AGE_START 216u
#define TB_IMU_AGE_WIDTH 10u
#define TB_RESERVED_START 226u
#define TB_RESERVED_WIDTH 6u

/* Heartbeat fields. */
#define HB_FORMAT_START 0u
#define HB_FORMAT_WIDTH 2u
#define HB_ESTOP_START 2u
#define HB_ESTOP_WIDTH 1u
#define HB_MAP_RESULT_START 3u
#define HB_MAP_RESULT_WIDTH 2u
#define HB_RESERVED_A_START 5u
#define HB_RESERVED_A_WIDTH 3u
#define HB_UPTIME_START 8u
#define HB_UPTIME_WIDTH 32u
#define HB_MAP_SEQ_START 40u
#define HB_MAP_SEQ_WIDTH 8u
#define HB_MAP_REV_START 48u
#define HB_MAP_REV_WIDTH 8u
#define HB_RESERVED_B_START 56u
#define HB_RESERVED_B_WIDTH 8u

/* Command byte: bits 0 to 2 the id, bits 3 to 7 reserved (zero). */
#define CMD_ID_START 0u
#define CMD_ID_WIDTH 3u
#define CMD_RESERVED_START 3u
#define CMD_RESERVED_WIDTH 5u

/* Value limits, in the units of the wire format. */
#define SPEED_MIN (-2048)
#define SPEED_MAX 2047
#define OBSTACLE_MIN_X10 20
#define OBSTACLE_MAX_X10 4000
#define HUMP_MAX_X10 4095
#define LINE_MASK_MAX 7u
#define STATE_MAX 7u
#define AGE_LINE_MAX_MS 999u
#define AGE_ENC_MAX_MS 1023u
#define US_PER_MS 1000u

/* One field: where it is and whether it is two's complement. */
typedef struct
{
    uint16_t start;
    uint8_t width;
    bool b_signed;
} bin_field_t;

/* Indexes of the telemetry fields in g_tb_table and in the value array. */
typedef enum
{
    TB_F_FORMAT = 0,
    TB_F_SEQ,
    TB_F_STATE,
    TB_F_VALID,
    TB_F_FAULT,
    TB_F_TIME,
    TB_F_SPEED_L,
    TB_F_SPEED_R,
    TB_F_ENC_L,
    TB_F_ENC_R,
    TB_F_GEN,
    TB_F_ENC_AGE,
    TB_F_LINE,
    TB_F_LINE_AGE,
    TB_F_HUMP,
    TB_F_OBST,
    TB_F_OBST_AGE,
    TB_F_IMU_AGE,
    TB_F_RESERVED,
    TB_F_COUNT
} tb_field_t;

/* Indexes of the heartbeat fields. */
typedef enum
{
    HB_F_FORMAT = 0,
    HB_F_ESTOP,
    HB_F_MAP_RESULT,
    HB_F_RESERVED_A,
    HB_F_UPTIME,
    HB_F_MAP_SEQ,
    HB_F_MAP_REV,
    HB_F_RESERVED_B,
    HB_F_COUNT
} hb_field_t;

static const bin_field_t g_tb_table[TB_F_COUNT] = {
    [TB_F_FORMAT] = {TB_FORMAT_START, TB_FORMAT_WIDTH, false},
    [TB_F_SEQ] = {TB_SEQ_START, TB_SEQ_WIDTH, false},
    [TB_F_STATE] = {TB_STATE_START, TB_STATE_WIDTH, false},
    [TB_F_VALID] = {TB_VALID_START, TB_VALID_WIDTH, false},
    [TB_F_FAULT] = {TB_FAULT_START, TB_FAULT_WIDTH, false},
    [TB_F_TIME] = {TB_TIME_START, TB_TIME_WIDTH, false},
    [TB_F_SPEED_L] = {TB_SPEED_L_START, TB_SPEED_WIDTH, true},
    [TB_F_SPEED_R] = {TB_SPEED_R_START, TB_SPEED_WIDTH, true},
    [TB_F_ENC_L] = {TB_ENC_L_START, TB_ENC_WIDTH, true},
    [TB_F_ENC_R] = {TB_ENC_R_START, TB_ENC_WIDTH, true},
    [TB_F_GEN] = {TB_GEN_START, TB_GEN_WIDTH, false},
    [TB_F_ENC_AGE] = {TB_ENC_AGE_START, TB_ENC_AGE_WIDTH, false},
    [TB_F_LINE] = {TB_LINE_START, TB_LINE_WIDTH, false},
    [TB_F_LINE_AGE] = {TB_LINE_AGE_START, TB_LINE_AGE_WIDTH, false},
    [TB_F_HUMP] = {TB_HUMP_START, TB_HUMP_WIDTH, false},
    [TB_F_OBST] = {TB_OBST_START, TB_OBST_WIDTH, false},
    [TB_F_OBST_AGE] = {TB_OBST_AGE_START, TB_OBST_AGE_WIDTH, false},
    [TB_F_IMU_AGE] = {TB_IMU_AGE_START, TB_IMU_AGE_WIDTH, false},
    [TB_F_RESERVED] = {TB_RESERVED_START, TB_RESERVED_WIDTH, false}};

static const bin_field_t g_hb_table[HB_F_COUNT] = {
    [HB_F_FORMAT] = {HB_FORMAT_START, HB_FORMAT_WIDTH, false},
    [HB_F_ESTOP] = {HB_ESTOP_START, HB_ESTOP_WIDTH, false},
    [HB_F_MAP_RESULT] = {HB_MAP_RESULT_START, HB_MAP_RESULT_WIDTH, false},
    [HB_F_RESERVED_A] = {HB_RESERVED_A_START, HB_RESERVED_A_WIDTH, false},
    [HB_F_UPTIME] = {HB_UPTIME_START, HB_UPTIME_WIDTH, false},
    [HB_F_MAP_SEQ] = {HB_MAP_SEQ_START, HB_MAP_SEQ_WIDTH, false},
    [HB_F_MAP_REV] = {HB_MAP_REV_START, HB_MAP_REV_WIDTH, false},
    [HB_F_RESERVED_B] = {HB_RESERVED_B_START, HB_RESERVED_B_WIDTH, false}};

/* Write every field of a table. Signed values are held as their two's
 * complement bit pattern in a uint32_t. Nothing is written if any value does
 * not fit its width. */
static bool
bin_put_all(uint8_t *p_buffer, uint32_t length, const bin_field_t *p_table,
            const uint32_t *p_values, uint32_t count)
{
    bool b_ok = true;
    uint32_t index = 0u;

    for (index = 0u; b_ok && (index < count); index++)
    {
        if (p_table[index].b_signed)
        {
            b_ok = s1_bits_put_signed(p_buffer, length, p_table[index].start,
                                      p_table[index].width,
                                      (int32_t)p_values[index]);
        }
        else
        {
            b_ok = s1_bits_put(p_buffer, length, p_table[index].start,
                               p_table[index].width, p_values[index]);
        }
    }
    return b_ok;
}

/* Read every field of a table (signed fields sign-extended). */
static bool
bin_get_all(const uint8_t *p_buffer, uint32_t length,
            const bin_field_t *p_table, uint32_t *p_values, uint32_t count)
{
    bool b_ok = true;
    uint32_t index = 0u;
    int32_t value = 0;

    for (index = 0u; b_ok && (index < count); index++)
    {
        if (p_table[index].b_signed)
        {
            b_ok = s1_bits_get_signed(p_buffer, length, p_table[index].start,
                                      p_table[index].width, &value);
            p_values[index] = (uint32_t)value;
        }
        else
        {
            b_ok = s1_bits_get(p_buffer, length, p_table[index].start,
                               p_table[index].width, &p_values[index]);
        }
    }
    return b_ok;
}

/* Convert an age in microseconds to whole ms; false if it does not fit. */
static bool
age_ms_from_us(uint64_t age_us, uint32_t max_ms, uint32_t *p_ms)
{
    uint64_t ms = age_us / US_PER_MS;

    if (ms > (uint64_t)max_ms)
    {
        return false;
    }
    *p_ms = (uint32_t)ms;
    return true;
}

/* The six value groups. Each returns true and fills its fields if the data
 * fits; false leaves the fields 0 and the caller clears the validity bit. */
static bool
group_speed(const s1_codec_frame_t *p_frame, uint32_t *p_values)
{
    bool b_fits = (p_frame->motor_speed_left >= SPEED_MIN) &&
                  (p_frame->motor_speed_left <= SPEED_MAX) &&
                  (p_frame->motor_speed_right >= SPEED_MIN) &&
                  (p_frame->motor_speed_right <= SPEED_MAX);

    if (b_fits)
    {
        p_values[TB_F_SPEED_L] = (uint32_t)p_frame->motor_speed_left;
        p_values[TB_F_SPEED_R] = (uint32_t)p_frame->motor_speed_right;
    }
    return b_fits;
}

static bool
group_encoder(const s1_codec_frame_t *p_frame, uint32_t *p_values)
{
    const uint64_t now_us = (uint64_t)p_frame->timestamp_ms * US_PER_MS;
    uint32_t age_ms = 0u;
    /* A count outside signed 32 bits is rejected, never truncated: a
     * truncated count would look like a genuine small number. */
    bool b_fits = (0u != p_frame->encoder_generation) &&
                  (p_frame->encoder_count_left >= (int64_t)INT32_MIN) &&
                  (p_frame->encoder_count_left <= (int64_t)INT32_MAX) &&
                  (p_frame->encoder_count_right >= (int64_t)INT32_MIN) &&
                  (p_frame->encoder_count_right <= (int64_t)INT32_MAX) &&
                  (p_frame->encoder_timestamp_us <= now_us) &&
                  age_ms_from_us(now_us - p_frame->encoder_timestamp_us,
                                 AGE_ENC_MAX_MS, &age_ms);

    if (b_fits)
    {
        p_values[TB_F_ENC_L] = (uint32_t)(int32_t)p_frame->encoder_count_left;
        p_values[TB_F_ENC_R] =
            (uint32_t)(int32_t)p_frame->encoder_count_right;
        p_values[TB_F_GEN] =
            p_frame->encoder_generation & s1_bits_mask(TB_GEN_WIDTH);
        p_values[TB_F_ENC_AGE] = age_ms;
    }
    return b_fits;
}

static bool
group_line(const s1_codec_frame_t *p_frame, uint32_t *p_values)
{
    uint32_t age_ms = 0u;
    bool b_fits = (p_frame->line_status <= LINE_MASK_MAX) &&
                  age_ms_from_us(p_frame->sensor_age_us[S1_AGE_LINE],
                                 AGE_LINE_MAX_MS, &age_ms);

    if (b_fits)
    {
        p_values[TB_F_LINE] = p_frame->line_status;
        p_values[TB_F_LINE_AGE] = age_ms;
    }
    return b_fits;
}

static bool
group_hump(const s1_codec_frame_t *p_frame, uint32_t *p_values)
{
    bool b_fits = (p_frame->hump_peak_x10 >= 0) &&
                  (p_frame->hump_peak_x10 <= HUMP_MAX_X10);

    if (b_fits)
    {
        p_values[TB_F_HUMP] = (uint32_t)p_frame->hump_peak_x10;
    }
    return b_fits;
}

static bool
group_obstacle(const s1_codec_frame_t *p_frame, uint32_t *p_values)
{
    uint32_t age_ms = 0u;
    bool b_fits = (p_frame->obstacle_distance_x10 >= OBSTACLE_MIN_X10) &&
                  (p_frame->obstacle_distance_x10 <= OBSTACLE_MAX_X10) &&
                  age_ms_from_us(p_frame->sensor_age_us[S1_AGE_OBSTACLE],
                                 AGE_LINE_MAX_MS, &age_ms);

    if (b_fits)
    {
        p_values[TB_F_OBST] = (uint32_t)p_frame->obstacle_distance_x10;
        p_values[TB_F_OBST_AGE] = age_ms;
    }
    return b_fits;
}

static bool
group_imu(const s1_codec_frame_t *p_frame, uint32_t *p_values)
{
    uint32_t age_ms = 0u;
    bool b_fits = age_ms_from_us(p_frame->sensor_age_us[S1_AGE_IMU],
                                 AGE_LINE_MAX_MS, &age_ms);

    if (b_fits)
    {
        p_values[TB_F_IMU_AGE] = age_ms;
    }
    return b_fits;
}

/* Fill the value groups the frame claims valid. A group that does not fit is
 * dropped: its validity bit is cleared and it is counted, never clamped. */
static uint32_t
select_groups(const s1_codec_frame_t *p_frame, uint32_t *p_values,
              uint32_t *p_cleared)
{
    uint32_t valid = (uint32_t)p_frame->validity & S1_VALID_ALL;
    uint32_t cleared = 0u;

    if ((0u != (valid & S1_VALID_SPEED)) && !group_speed(p_frame, p_values))
    {
        valid &= ~S1_VALID_SPEED;
        cleared++;
    }
    if ((0u != (valid & S1_VALID_ENCODER)) && !group_encoder(p_frame, p_values))
    {
        valid &= ~S1_VALID_ENCODER;
        cleared++;
    }
    if ((0u != (valid & S1_VALID_LINE)) && !group_line(p_frame, p_values))
    {
        valid &= ~S1_VALID_LINE;
        cleared++;
    }
    if ((0u != (valid & S1_VALID_HUMP)) && !group_hump(p_frame, p_values))
    {
        valid &= ~S1_VALID_HUMP;
        cleared++;
    }
    if ((0u != (valid & S1_VALID_OBSTACLE)) &&
        !group_obstacle(p_frame, p_values))
    {
        valid &= ~S1_VALID_OBSTACLE;
        cleared++;
    }
    if ((0u != (valid & S1_VALID_IMU)) && !group_imu(p_frame, p_values))
    {
        valid &= ~S1_VALID_IMU;
        cleared++;
    }
    *p_cleared = cleared;
    return valid;
}

bool s1_codec_telemetry_bin_stamp(uint8_t *p_buffer, uint32_t length,
                                  uint32_t sequence, uint8_t fault_mask)
{
    return (NULL != p_buffer) && (S1_CODEC_TELEMETRY_BIN_BYTES == length) &&
           s1_bits_put(p_buffer, length, TB_SEQ_START, TB_SEQ_WIDTH,
                       sequence & s1_bits_mask(TB_SEQ_WIDTH)) &&
           s1_bits_put(p_buffer, length, TB_FAULT_START, TB_FAULT_WIDTH,
                       fault_mask);
}

uint32_t
s1_codec_encode_telemetry_bin(const s1_codec_frame_t *p_frame,
                              uint32_t sequence, uint8_t fault_mask,
                              uint8_t *p_buffer, uint32_t capacity,
                              uint32_t *p_out_of_range)
{
    uint32_t values[TB_F_COUNT] = {0u};
    uint32_t cleared = 0u;
    uint32_t index = 0u;
    bool b_ok = false;

    if (NULL != p_out_of_range)
    {
        *p_out_of_range = 0u;
    }
    if ((NULL == p_frame) || (NULL == p_buffer) ||
        (capacity < S1_CODEC_TELEMETRY_BIN_BYTES) ||
        (p_frame->robot_state > STATE_MAX))
    {
        return 0u;
    }
    values[TB_F_FORMAT] = S1_CODEC_BIN_FORMAT;
    values[TB_F_SEQ] = sequence & s1_bits_mask(TB_SEQ_WIDTH);
    values[TB_F_STATE] = p_frame->robot_state;
    values[TB_F_FAULT] = fault_mask;
    values[TB_F_TIME] = p_frame->timestamp_ms;
    values[TB_F_VALID] = select_groups(p_frame, values, &cleared);

    for (index = 0u; index < S1_CODEC_TELEMETRY_BIN_BYTES; index++)
    {
        p_buffer[index] = 0u;
    }
    b_ok = bin_put_all(p_buffer, S1_CODEC_TELEMETRY_BIN_BYTES, g_tb_table,
                       values, TB_F_COUNT);
    if (b_ok && (NULL != p_out_of_range))
    {
        *p_out_of_range = cleared;
    }
    return b_ok ? S1_CODEC_TELEMETRY_BIN_BYTES : 0u;
}

bool s1_codec_decode_telemetry_bin(const uint8_t *p_buffer, uint32_t length,
                                   s1_codec_bin_telemetry_t *p_out)
{
    uint32_t values[TB_F_COUNT] = {0u};
    bool b_ok = (NULL != p_buffer) && (NULL != p_out) &&
                (S1_CODEC_TELEMETRY_BIN_BYTES == length);

    b_ok = b_ok &&
           bin_get_all(p_buffer, length, g_tb_table, values, TB_F_COUNT) &&
           (S1_CODEC_BIN_FORMAT == values[TB_F_FORMAT]) &&
           (0u == values[TB_F_RESERVED]);
    if (b_ok)
    {
        p_out->sequence = (uint16_t)values[TB_F_SEQ];
        p_out->robot_state = (uint8_t)values[TB_F_STATE];
        p_out->validity = (uint8_t)values[TB_F_VALID];
        p_out->fault_mask = (uint8_t)values[TB_F_FAULT];
        p_out->timestamp_ms = values[TB_F_TIME];
        p_out->speed_left = (int16_t)(int32_t)values[TB_F_SPEED_L];
        p_out->speed_right = (int16_t)(int32_t)values[TB_F_SPEED_R];
        p_out->encoder_left = (int32_t)values[TB_F_ENC_L];
        p_out->encoder_right = (int32_t)values[TB_F_ENC_R];
        p_out->encoder_generation = (uint8_t)values[TB_F_GEN];
        p_out->encoder_age_ms = (uint16_t)values[TB_F_ENC_AGE];
        p_out->line_status = (uint8_t)values[TB_F_LINE];
        p_out->line_age_ms = (uint16_t)values[TB_F_LINE_AGE];
        p_out->hump_x10 = (uint16_t)values[TB_F_HUMP];
        p_out->obstacle_x10 = (uint16_t)values[TB_F_OBST];
        p_out->obstacle_age_ms = (uint16_t)values[TB_F_OBST_AGE];
        p_out->imu_age_ms = (uint16_t)values[TB_F_IMU_AGE];
    }
    return b_ok;
}

uint32_t
s1_codec_encode_heartbeat_bin(uint32_t uptime_ms, bool b_estop_latched,
                              const s1_codec_map_ack_t *p_ack,
                              uint8_t *p_buffer, uint32_t capacity)
{
    const s1_codec_map_ack_t none = {0u, 0u, 0u};
    const s1_codec_map_ack_t *p_use = (NULL != p_ack) ? p_ack : &none;
    uint32_t values[HB_F_COUNT] = {0u};
    uint32_t index = 0u;
    bool b_ok = (NULL != p_buffer) &&
                (capacity >= S1_CODEC_HEARTBEAT_BIN_BYTES) &&
                (p_use->result <= (uint8_t)S1_MAP_RESULT_REJECTED);

    if (b_ok)
    {
        values[HB_F_FORMAT] = S1_CODEC_BIN_FORMAT;
        values[HB_F_ESTOP] = b_estop_latched ? 1u : 0u;
        values[HB_F_MAP_RESULT] = p_use->result;
        values[HB_F_UPTIME] = uptime_ms;
        values[HB_F_MAP_SEQ] = p_use->seq;
        values[HB_F_MAP_REV] = p_use->revision;
        for (index = 0u; index < S1_CODEC_HEARTBEAT_BIN_BYTES; index++)
        {
            p_buffer[index] = 0u;
        }
        b_ok = bin_put_all(p_buffer, S1_CODEC_HEARTBEAT_BIN_BYTES, g_hb_table,
                           values, HB_F_COUNT);
    }
    return b_ok ? S1_CODEC_HEARTBEAT_BIN_BYTES : 0u;
}

bool s1_codec_decode_heartbeat_bin(const uint8_t *p_buffer, uint32_t length,
                                   uint32_t *p_uptime_ms, bool *p_estop_latched,
                                   s1_codec_map_ack_t *p_ack)
{
    uint32_t values[HB_F_COUNT] = {0u};
    bool b_ok = (NULL != p_buffer) && (NULL != p_uptime_ms) &&
                (NULL != p_estop_latched) &&
                (S1_CODEC_HEARTBEAT_BIN_BYTES == length);

    b_ok = b_ok &&
           bin_get_all(p_buffer, length, g_hb_table, values, HB_F_COUNT) &&
           (S1_CODEC_BIN_FORMAT == values[HB_F_FORMAT]) &&
           (0u == values[HB_F_RESERVED_A]) && (0u == values[HB_F_RESERVED_B]);
    if (b_ok)
    {
        *p_uptime_ms = values[HB_F_UPTIME];
        *p_estop_latched = (0u != values[HB_F_ESTOP]);
        if (NULL != p_ack)
        {
            p_ack->seq = (uint8_t)values[HB_F_MAP_SEQ];
            p_ack->result = (uint8_t)values[HB_F_MAP_RESULT];
            p_ack->revision = (uint8_t)values[HB_F_MAP_REV];
        }
    }
    return b_ok;
}

s1_codec_command_id_t
s1_codec_parse_command_bin(const uint8_t *p_payload, uint32_t length)
{
    uint32_t id = 0u;
    uint32_t reserved = 0u;
    bool b_ok =
        (NULL != p_payload) && (S1_CODEC_COMMAND_BIN_BYTES == length) &&
        s1_bits_get(p_payload, length, CMD_ID_START, CMD_ID_WIDTH, &id) &&
        s1_bits_get(p_payload, length, CMD_RESERVED_START, CMD_RESERVED_WIDTH,
                    &reserved);

    b_ok = b_ok && (0u == reserved) &&
           (id >= (uint32_t)S1_CMD_EMERGENCY_STOP) &&
           (id <= (uint32_t)S1_CMD_RESUME);
    return b_ok ? (s1_codec_command_id_t)id : S1_CMD_NONE;
}

#if APP_ROBOT
s1_codec_command_id_t s1_codec_parse_robot (uint8_t const * p_bytes,
    uint32_t length, uint32_t * p_session)
{
    if ((NULL == p_bytes) || (NULL == p_session) || (5U != length) ||
        (p_bytes[0] < 0xA1U) || (p_bytes[0] > 0xA5U))
    {
        return (S1_CMD_NONE);
    }
    uint32_t session = 0U;
    for (uint32_t index = 0U; index < 4U; ++index)
    {
        session |= (uint32_t)p_bytes[index + 1U] << (8U * index);
    }
    if (0U == session) { return (S1_CMD_NONE); }
    *p_session = session;
    return ((s1_codec_command_id_t)(S1_CMD_BENCH_ENCODERS +
        (p_bytes[0] - 0xA1U)));
}
#endif

uint32_t
s1_codec_encode_command_bin(s1_codec_command_id_t command_id,
                            uint8_t *p_buffer, uint32_t capacity)
{
    uint32_t length = 0u;

    if ((NULL != p_buffer) && (capacity >= S1_CODEC_COMMAND_BIN_BYTES) &&
        (command_id >= S1_CMD_EMERGENCY_STOP) && (command_id <= S1_CMD_RESUME))
    {
        p_buffer[0] = (uint8_t)command_id;
        length = S1_CODEC_COMMAND_BIN_BYTES;
    }
    return length;
}

/*** end of file ***/
