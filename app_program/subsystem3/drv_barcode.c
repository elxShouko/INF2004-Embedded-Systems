/** @file drv_barcode.c
 * @brief Member 3 drv barcode module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#include "drv_barcode.h"

#include <string.h>

/* Code 39 Table 2: bits 8..0 are the nine alternating bar/space elements.
 * 1 denotes wide. Source: USS-39, docs/buddy3/member3.md.
 */
#define BARCODE_ALPHABET_COUNT 26U
#define BARCODE_ELEMENTS_PER_SYMBOL 9U
#define BARCODE_SYMBOLS_PER_FRAME 3U
#define BARCODE_GAPS_PER_FRAME 2U

#define CODE39_STAR_PATTERN 0x094U

/* 26 nine-bit patterns = 234 bits; high six bits of the final byte are zero. */
static const uint8_t g_letter_patterns[30] = {
    0x09U, 0x93U, 0x20U, 0xcdU, 0x80U, 0x11U, 0x4bU, 0x03U, 0x86U, 0x4cU,
    0x38U, 0x0cU, 0x1cU, 0x22U, 0x74U, 0x82U, 0x44U, 0x29U, 0x07U, 0x0cU,
    0x1aU, 0xb1U, 0x10U, 0x38U, 0x18U, 0xf0U, 0x48U, 0x90U, 0xa1U, 0x01U};

static bool classify_symbol (const uint32_t * p_widths, uint16_t * p_pattern,
                             uint32_t * p_narrow);
static bool decode_forward (const uint32_t * p_widths, char * p_letter);

static barcode_result_t capture_transition (barcode_capture_t * p_capture,
                                            uint64_t            timestamp_us,
                                            uint64_t duration, bool black,
                                            barcode_result_t result);

/** @brief Read one nine-bit pattern without assuming host byte ordering. */
static uint16_t letter_pattern (uint32_t index)
{
    const uint32_t bit = index * 9U;
    const uint32_t byte = bit / 8U;
    const uint32_t pair = (uint32_t)g_letter_patterns[byte] |
                         ((uint32_t)g_letter_patterns[byte + 1U] << 8U);
    return (uint16_t)((pair >> (bit % 8U)) & 0x1ffU);
}

/**
 * @brief Decode one framed uppercase letter in either scan direction.
 * @note No numerical conversion is needed; run count is unsigned.
 */
bool barcode_decode (const uint32_t * p_widths, uint32_t count, char * p_letter)
{
    if ((NULL == p_widths) || (NULL == p_letter) ||
        (count != BARCODE_RUN_COUNT))
    {
        return false;
    }

    if (decode_forward(p_widths, p_letter))
    {
        return true;
    }

    uint32_t reversed[BARCODE_RUN_COUNT];

    for (uint32_t element_index = 0U; element_index < BARCODE_RUN_COUNT;
         ++element_index)
    {
        reversed[element_index] =
            p_widths[BARCODE_RUN_COUNT - 1U - element_index];
    }

    return decode_forward(reversed, p_letter);
}

/**
 * @brief Initialize bounded capture with caller-measured timing.
 */
bool barcode_capture_init (barcode_capture_t * p_capture, uint32_t quiet_us,
                           uint32_t timeout_us)
{
    if ((NULL == p_capture) || (0U == quiet_us) || (timeout_us <= quiet_us))
    {
        return false;
    }

    memset(p_capture, 0, sizeof(*p_capture));
    p_capture->quiet_us   = quiet_us;
    p_capture->timeout_us = timeout_us;

    return true;
}

/**
 * @brief Capture an edge or finish a quiet-zone-delimited scan.
 * @note The duration is checked against UINT32_MAX before narrowing to
 * uint32_t.
 */
barcode_result_t barcode_capture_update (barcode_capture_t * p_capture,
                                         uint64_t timestamp_us, bool black,
                                         char * p_letter)
{
    if ((NULL == p_capture) || (NULL == p_letter) ||
        (0U == p_capture->quiet_us) ||
        ((p_capture->started) && (timestamp_us < p_capture->last_us)))
    {
        return BARCODE_RESULT_INVALID;
    }

    if (!p_capture->started)
    {
        p_capture->started = true;
        p_capture->edge_us = timestamp_us;
        p_capture->black   = black;
    }

    p_capture->last_us        = timestamp_us;
    const uint64_t   duration = timestamp_us - p_capture->edge_us;
    barcode_result_t result   = BARCODE_RESULT_PENDING;

    if ((p_capture->capturing) &&
        ((timestamp_us - p_capture->frame_start_us) >= p_capture->timeout_us))
    {
        p_capture->capturing = false;
        p_capture->armed     = false;
        p_capture->count     = 0U;
        /* A timed-out white run must not immediately rearm this scan. */
        p_capture->edge_us = timestamp_us;
        p_capture->black   = black;

        return BARCODE_RESULT_INVALID;
    }

    if ((!p_capture->black) && (duration >= p_capture->quiet_us))
    {
        if (p_capture->capturing)
        {
            result =
                barcode_decode(p_capture->widths, p_capture->count, p_letter)
                    ? BARCODE_RESULT_VALID
                    : BARCODE_RESULT_INVALID;
            p_capture->capturing = false;
        }

        p_capture->armed = true;
    }

    if (black != p_capture->black)
    {
        result = capture_transition(p_capture, timestamp_us, duration, black,
                                    result);
    }

    return result;
}

/**
 * @brief Classify a Code 39 symbol using separated width groups.
 * @note Widening to uint64_t is lossless. The pattern uses nine bits, so
 * uint16_t holds every generated value.
 */
static bool classify_symbol (const uint32_t * p_widths, uint16_t * p_pattern,
                             uint32_t * p_narrow)
{
    uint32_t sorted[9];
    memcpy(sorted, p_widths, sizeof(sorted));

    for (uint32_t element_index = 1U;
         element_index < BARCODE_ELEMENTS_PER_SYMBOL; ++element_index)
    {
        const uint32_t value        = sorted[element_index];
        unsigned int   insert_index = element_index;

        while ((insert_index > 0U) && (sorted[insert_index - 1U] > value))
        {
            sorted[insert_index] = sorted[insert_index - 1U];
            --insert_index;
        }

        sorted[insert_index] = value;
    }

    /* Conservative separation check, not a substitute for hardware tuning.
     * Narrow/wide groups each span at most 1.5x; separation at least 1.5x;
     * widest element at most 4x narrow minimum. Use 64-bit arithmetic.
     */

    if ((0U == sorted[0]) ||
        ((uint64_t)sorted[5] * 2U > (uint64_t)sorted[0] * 3U) ||
        ((uint64_t)sorted[8] * 2U > (uint64_t)sorted[6] * 3U) ||
        ((uint64_t)sorted[6] * 2U < (uint64_t)sorted[5] * 3U) ||
        ((uint64_t)sorted[8] > (uint64_t)sorted[0] * 4U))
    {
        return false;
    }

    uint16_t pattern = 0U;

    for (uint32_t element_index = 0U;
         element_index < BARCODE_ELEMENTS_PER_SYMBOL; ++element_index)
    {
        pattern =
            (uint16_t)(((uint32_t)pattern << 1U) |
                       ((p_widths[element_index] >= sorted[6]) ? 1U : 0U));
    }

    *p_pattern = pattern;
    *p_narrow  = sorted[2];

    return true;
}

/**
 * @brief Validate framing and decode an uppercase forward symbol.
 * @note The table index is below 26, so the ASCII uppercase result fits char on
 * RP2040.
 */
static bool decode_forward (const uint32_t * p_widths, char * p_letter)
{
    uint16_t patterns[3] = {0};
    uint32_t narrow[3] = {0};

    for (uint32_t element_index = 0U; element_index < BARCODE_SYMBOLS_PER_FRAME;
         ++element_index)
    {
        if (!classify_symbol(p_widths + (element_index * 10U),
                             &patterns[element_index], &narrow[element_index]))
        {
            return false;
        }
    }

    if ((patterns[0] != CODE39_STAR_PATTERN) ||
        (patterns[2] != CODE39_STAR_PATTERN))
    {
        return false;
    }

    for (uint32_t element_index = 0U; element_index < BARCODE_GAPS_PER_FRAME;
         ++element_index)
    {
        const uint32_t gap = p_widths[(element_index * 10U) + 9U];

        if (((uint64_t)gap * 2U < narrow[element_index]) ||
            ((uint64_t)gap > (uint64_t)narrow[element_index] * 3U) ||
            ((uint64_t)narrow[element_index + 1U] * 2U <
             narrow[element_index]) ||
            ((uint64_t)narrow[element_index + 1U] >
             (uint64_t)narrow[element_index] * 2U))
        {
            return false;
        }
    }

    for (uint32_t element_index = 0U; element_index < BARCODE_ALPHABET_COUNT;
         ++element_index)
    {
        if (patterns[1] == letter_pattern(element_index))
        {
            *p_letter = (char)('A' + element_index);

            return true;
        }
    }

    return false;
}

/**
 * @brief Store an edge or reject capture overflow without nested handlers.
 * @note A narrowed duration was checked against UINT32_MAX first.
 */
static barcode_result_t capture_transition (barcode_capture_t * p_capture,
                                            uint64_t            timestamp_us,
                                            uint64_t duration, bool black,
                                            barcode_result_t result)
{
    if (p_capture->capturing)
    {
        if ((0U == duration) || (duration > UINT32_MAX) ||
            (p_capture->count >= BARCODE_RUN_COUNT))
        {
            p_capture->capturing = false;
            p_capture->armed     = false;
            result               = BARCODE_RESULT_INVALID;
        }
        else
        {
            p_capture->widths[p_capture->count++] = (uint32_t)duration;
        }
    }
    else if (black && (p_capture->armed))
    {
        p_capture->count          = 0U;
        p_capture->capturing      = true;
        p_capture->frame_start_us = timestamp_us;
        p_capture->armed          = false;
    }
    else
    {
        /* Wait for a valid leading quiet zone before starting capture. */
    }

    p_capture->black   = black;
    p_capture->edge_us = timestamp_us;

    return result;
}

/*** End of file. ***/

