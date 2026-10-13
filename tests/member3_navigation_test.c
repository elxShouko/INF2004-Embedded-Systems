#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "line/barcode.h"
#include "line/line_tracking.h"
#include "line/navigation_map.h"

/* Independent golden bar/space assignments from USS-39 Table 2.
 * Five bar bits and four space bits are interleaved, not decoder hex values.
 */
static const char *g_bars[26] =
{
    "10001", "01001", "11000", "00101", "10100", "01100", "00011",
    "10010", "01010", "00110", "10001", "01001", "11000", "00101",
    "10100", "01100", "00011", "10010", "01010", "00110", "10001",
    "01001", "11000", "00101", "10100", "01100"
};

static void symbol(uint32_t *p_out, const char *p_bars, const char *p_spaces,
                   uint32_t scale)
{
    for (unsigned int i = 0U; i < 9U; ++i)
    {
        const char wide = ((i & 1U) == 0U) ? p_bars[i / 2U] : p_spaces[i / 2U];
        p_out[i] = ((wide == '1') ? 3U : 1U) * scale;
    }
}

static void frame(uint32_t *p_out, unsigned int letter, uint32_t scale)
{
    symbol(p_out, "00110", "1000", scale);
    p_out[9] = scale;
    symbol(p_out + 10U, g_bars[letter],
           (letter < 10U) ? "0010" : ((letter < 20U) ? "0001" : "1000"), scale);
    p_out[19] = scale;
    symbol(p_out + 20U, "00110", "1000", scale);
}

static void test_capture(const uint32_t *p_widths, char expected)
{
    barcode_capture_t capture;
    char result = '\0';
    uint64_t now = 0U;
    assert(barcode_capture_init(&capture, 1000U, 1000000U));
    assert(barcode_capture_update(&capture, now, false, &result) == BARCODE_RESULT_PENDING);
    now = 1000U;
    assert(barcode_capture_update(&capture, now, true, &result) == BARCODE_RESULT_PENDING);
    bool black = true;
    for (unsigned int i = 0U; i < BARCODE_RUN_COUNT; ++i)
    {
        now += p_widths[i];
        black = !black;
        assert(barcode_capture_update(&capture, now, black, &result) == BARCODE_RESULT_PENDING);
    }
    assert(barcode_capture_update(&capture, now + 1000U, false, &result) == BARCODE_RESULT_VALID);
    assert(result == expected);
    assert(barcode_capture_update(&capture, now + 2000U, false, &result) == BARCODE_RESULT_PENDING);
}

int main(void)
{
    nav_map_t map;
    nav_map_init(&map);
    assert(nav_map_lookup(&map, 'A') == NAV_COMMAND_STRAIGHT);
    assert(nav_map_lookup(&map, 'B') == NAV_COMMAND_LEFT);
    assert(nav_map_lookup(&map, 'C') == NAV_COMMAND_RIGHT);
    assert(nav_map_lookup(&map, 'D') == NAV_COMMAND_U_TURN);
    assert(nav_map_lookup(&map, 'Z') == NAV_COMMAND_NONE);
    assert(nav_map_apply(&map, "MAP A=FORWARD Z=UTURN W=LEFT F=RIGHT B=NONE"));
    assert(map.revision == 1U);
    assert(nav_map_lookup(&map, 'W') == NAV_COMMAND_LEFT);
    assert(nav_map_lookup(&map, 'F') == NAV_COMMAND_RIGHT);
    assert(nav_map_lookup(&map, 'Z') == NAV_COMMAND_U_TURN);
    assert(nav_map_lookup(&map, 'B') == NAV_COMMAND_NONE);
    const nav_map_t snapshot = map;
    const char *bad[] = {"MAP A=LEFT Z=BACKWARD", "MAP A=LEFT A=RIGHT",
                        "MAP a=LEFT", "MAP [=LEFT", "MAP ", "MAP A=LEFT junk",
                        "MAP A=LEFT\n", "MAP A=LEFTB=RIGHT", "MA", ""};
    for (unsigned int i = 0U; i < sizeof(bad) / sizeof(bad[0]); ++i)
    {
        assert(!nav_map_apply(&map, bad[i]));
        assert(memcmp(&map, &snapshot, sizeof(map)) == 0);
    }
    assert(!nav_map_apply(NULL, "MAP A=LEFT"));
    assert(!nav_map_apply(&map, NULL));
    map.revision = UINT32_MAX;
    assert(!nav_map_apply(&map, "MAP A=LEFT"));

    line_observation_t observation;
    const line_observation_t expected[] = {LINE_OBSERVATION_ABSENT,
        LINE_OBSERVATION_CORRECT_LEFT, LINE_OBSERVATION_CORRECT_RIGHT,
        LINE_OBSERVATION_CENTRED};
    for (unsigned int i = 0U; i < 8U; ++i)
    {
        assert(line_classify((uint8_t)i, &observation));
        assert(observation == expected[i & 3U]);
    }
    assert(!line_classify(8U, &observation));

    uint32_t widths[BARCODE_RUN_COUNT];
    uint32_t reverse[BARCODE_RUN_COUNT];
    char letter;
    for (unsigned int index = 0U; index < 26U; ++index)
    {
        for (uint32_t scale = 10U; scale <= 1000U; scale *= 10U)
        {
            frame(widths, index, scale);
            assert(barcode_decode(widths, BARCODE_RUN_COUNT, &letter));
            assert(letter == (char)('A' + index));
            for (unsigned int i = 0U; i < BARCODE_RUN_COUNT; ++i)
            {
                reverse[i] = widths[BARCODE_RUN_COUNT - 1U - i];
            }
            assert(barcode_decode(reverse, BARCODE_RUN_COUNT, &letter));
            assert(letter == (char)('A' + index));
        }
        frame(widths, index, 100U);
        test_capture(widths, (char)('A' + index));
        for (unsigned int i = 0U; i < BARCODE_RUN_COUNT; ++i)
        {
            reverse[i] = widths[BARCODE_RUN_COUNT - 1U - i];
        }
        test_capture(reverse, (char)('A' + index));
    }
    /* Actual raster widths extracted from supplied PDF pages 1 and 3. */
    const uint32_t pdf_a[29] = {60,178,60,58,179,59,179,59,60,59,179,59,60,59,60,177,60,59,179,59,60,178,60,58,179,59,179,59,60};
    const uint32_t pdf_z[29] = {60,178,60,58,179,59,179,59,60,59,60,178,179,58,179,59,60,59,60,59,60,178,60,58,179,59,179,59,60};
    assert(barcode_decode(pdf_a, 29U, &letter) && (letter == 'A'));
    assert(barcode_decode(pdf_z, 29U, &letter) && (letter == 'Z'));
    frame(widths, 0U, 100U);
    assert(!barcode_decode(widths, 28U, &letter));
    widths[0] = 0U;
    assert(!barcode_decode(widths, 29U, &letter));
    frame(widths, 0U, 100U);
    widths[9] = 10000U;
    assert(!barcode_decode(widths, 29U, &letter));
    for (unsigned int i = 0U; i < 29U; ++i)
    {
        widths[i] = 100U;
    }
    assert(!barcode_decode(widths, 29U, &letter));

    barcode_capture_t capture;
    assert(barcode_capture_init(&capture, 1000U, 2000U));
    assert(barcode_capture_update(&capture, 0U, false, &letter) == BARCODE_RESULT_PENDING);
    assert(barcode_capture_update(&capture, 1000U, true, &letter) == BARCODE_RESULT_PENDING);
    assert(barcode_capture_update(&capture, 3000U, true, &letter) == BARCODE_RESULT_INVALID);
    assert(barcode_capture_update(&capture, 2999U, true, &letter) == BARCODE_RESULT_INVALID);
    assert(!barcode_capture_init(&capture, 0U, 1000U));

    /* No leading quiet zone: ignore partial scans. */
    assert(barcode_capture_init(&capture, 1000U, 1000000U));
    assert(barcode_capture_update(&capture, 0U, false, &letter) == BARCODE_RESULT_PENDING);
    assert(barcode_capture_update(&capture, 500U, true, &letter) == BARCODE_RESULT_PENDING);
    assert(!capture.capturing);

    /* Incomplete framed input ends as INVALID, never a navigation letter. */
    assert(barcode_capture_init(&capture, 1000U, 1000000U));
    assert(barcode_capture_update(&capture, 0U, false, &letter) == BARCODE_RESULT_PENDING);
    assert(barcode_capture_update(&capture, 1000U, true, &letter) == BARCODE_RESULT_PENDING);
    assert(barcode_capture_update(&capture, 1100U, false, &letter) == BARCODE_RESULT_PENDING);
    assert(barcode_capture_update(&capture, 2100U, false, &letter) == BARCODE_RESULT_INVALID);

    /* More transitions than a single-letter frame must not overflow storage. */
    assert(barcode_capture_init(&capture, 1000U, 1000000U));
    assert(barcode_capture_update(&capture, 0U, false, &letter) == BARCODE_RESULT_PENDING);
    assert(barcode_capture_update(&capture, 1000U, true, &letter) == BARCODE_RESULT_PENDING);
    bool level = true;
    for (unsigned int i = 0U; i < 30U; ++i)
    {
        level = !level;
        const barcode_result_t status = barcode_capture_update(&capture,
            1100U + i * 100U, level, &letter);
        assert(status == ((i == 29U) ? BARCODE_RESULT_INVALID : BARCODE_RESULT_PENDING));
    }
    assert(!capture.capturing);

    nav_map_init(&map);
    assert(nav_map_apply(&map, "MAP A=LEFT"));
    assert(barcode_decode(pdf_a, 29U, &letter));
    assert(nav_map_lookup(&map, letter) == NAV_COMMAND_LEFT);
    puts("Member 3 navigation tests passed: A-Z both directions, capture, mapping, line rules.");
    return 0;
}
