/**
 * @file s1_codec.c
 * @brief Buddy 1: pure message encoding and command parsing.
 *
 * NOTE: No kernel, lwIP or libc functions are used, so the same file is
 * compiled for the Pico and for the PC unit tests.
 */

#include "s1_codec.h"

#define DECIMAL_BASE 10u
#define U32_DIGITS_MAX 10u
#define TENTHS_LIMIT 99999 /* +/- 9999.9: bounds the JSON length (NFR10). */
#define COMMAND_TOKEN_LENGTH 24u
#define LOWER_TO_UPPER_OFFSET ('a' - 'A')

typedef struct
{
    char *   p_buffer;
    uint32_t capacity;
    uint32_t length;
    bool     b_overflow;
} text_writer_t;

typedef struct
{
    const char *          p_name;
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

#define COMMAND_TABLE_COUNT                                                    \
    (sizeof(g_command_table) / sizeof(g_command_table[0]))

static void     writer_start (text_writer_t * p_writer, char * p_buffer,
                              uint32_t capacity);
static void     writer_char (text_writer_t * p_writer, char letter);
static void     writer_text (text_writer_t * p_writer, const char * p_text);
static void     writer_u32 (text_writer_t * p_writer, uint32_t value);
static void     writer_i32 (text_writer_t * p_writer, int32_t value);
static void     writer_tenths (text_writer_t * p_writer, int32_t value_x10);
static uint32_t writer_finish (text_writer_t * p_writer);
static char     to_upper_ascii (char letter);
static bool     is_blank (char letter);
static bool     is_token_end (char letter);
static bool     text_equal (const char * p_first, const char * p_second);

/* ---------------------------------------------------------------------- */
/* Public functions                                                       */
/* ---------------------------------------------------------------------- */

/* Encode one telemetry frame. Field names follow FR17. The extra sequence
 * number lets the laptop measure frame loss (NFR8). The worst-case length is
 * 243 bytes, so the JSON always fits NFR10 (256 bytes). */
uint32_t
s1_codec_encode_telemetry (const s1_codec_frame_t * p_frame, uint32_t sequence,
                           char * p_buffer, uint32_t capacity)
{
    text_writer_t writer = {0};

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
    writer_u32(&writer, p_frame->encoder_count_left);
    writer_text(&writer, ",\"right_encoder_count\":");
    writer_u32(&writer, p_frame->encoder_count_right);
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
s1_codec_encode_heartbeat (uint32_t uptime_ms, char * p_buffer,
                           uint32_t capacity)
{
    text_writer_t writer = {0};

    writer_start(&writer, p_buffer, capacity);
    writer_text(&writer, "{\"status\":\"ALIVE\",\"uptime_ms\":");
    writer_u32(&writer, uptime_ms);
    writer_text(&writer, "}");
    return writer_finish(&writer);
}

/* Turn a payload such as " start_run" into a command identifier. */
s1_codec_command_id_t
s1_codec_parse_command (const char * p_payload, uint32_t length)
{
    char                  token[COMMAND_TOKEN_LENGTH] = {0};
    uint32_t              position                    = 0u;
    uint32_t              token_length                = 0u;
    uint32_t              entry_number                = 0u;
    s1_codec_command_id_t command_id                  = S1_CMD_NONE;

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
s1_codec_command_name (s1_codec_command_id_t command_id)
{
    const char * p_name       = "NONE";
    uint32_t     entry_number = 0u;

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
writer_start (text_writer_t * p_writer, char * p_buffer, uint32_t capacity)
{
    p_writer->p_buffer   = p_buffer;
    p_writer->capacity   = capacity;
    p_writer->length     = 0u;
    p_writer->b_overflow = (0u == capacity);
    if (0u != capacity)
    {
        p_buffer[0] = '\0';
    }
}

/* Append one character, keeping room for the terminator. */
static void
writer_char (text_writer_t * p_writer, char letter)
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
writer_text (text_writer_t * p_writer, const char * p_text)
{
    const char * p_cursor = p_text;

    while ('\0' != *p_cursor)
    {
        writer_char(p_writer, *p_cursor);
        p_cursor++;
    }
}

/* Append an unsigned number in decimal. */
static void
writer_u32 (text_writer_t * p_writer, uint32_t value)
{
    char     digits[U32_DIGITS_MAX] = {0};
    uint32_t count                  = 0u;
    uint32_t remaining              = value;

    do
    {
        digits[count] = '0' + (remaining % DECIMAL_BASE);
        remaining     = remaining / DECIMAL_BASE;
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
writer_i32 (text_writer_t * p_writer, int32_t value)
{
    uint32_t magnitude = value;

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
writer_tenths (text_writer_t * p_writer, int32_t value_x10)
{
    int32_t  limited   = value_x10;
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
        magnitude = 0u - (uint32_t) limited;
    }
    else
    {
        magnitude = limited;
    }
    writer_u32(p_writer, magnitude / DECIMAL_BASE);
    writer_char(p_writer, '.');
    writer_u32(p_writer, magnitude % DECIMAL_BASE);
}

/* Return the length, or 0 (and an empty string) if anything did not fit. */
static uint32_t
writer_finish (text_writer_t * p_writer)
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
to_upper_ascii (char letter)
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
is_blank (char letter)
{
    return ((' ' == letter) || ('\t' == letter) || ('\r' == letter) ||
            ('\n' == letter) || ('"' == letter));
}

/* True for the characters that end a command word. */
static bool
is_token_end (char letter)
{
    return (('\0' == letter) || is_blank(letter));
}

/* Compare two NUL-terminated strings. */
static bool
text_equal (const char * p_first, const char * p_second)
{
    const char * p_left  = p_first;
    const char * p_right = p_second;

    while (('\0' != *p_left) && (*p_left == *p_right))
    {
        p_left++;
        p_right++;
    }
    return (*p_left == *p_right);
}

/*** end of file ***/
