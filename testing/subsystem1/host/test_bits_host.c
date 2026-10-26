/* Host (PC) unit tests for s1_bits.h and s1_fault.h. Run with run_host_tests.sh.
 * An independent bit-by-bit reference (no shared code with s1_bits.h) is the
 * oracle, so the chunked implementation is checked against something simpler. */
#include <stdio.h>
#include <string.h>

#include "../../../app_program/subsystem1/s1_bits.h"
#include "../../../app_program/subsystem1/s1_fault.h"

#define BUFFER_BYTES 16u
#define RANDOM_TRIALS 200000u

static int g_checks = 0;
static int g_failures = 0;

static void
check(int b_ok, const char *p_name)
{
    g_checks++;
    if (!b_ok)
    {
        g_failures++;
        printf("  FAIL %s\n", p_name);
    }
}

/* Reference: one bit at a time. */
static void
ref_put(uint8_t *p_buffer, uint32_t start, uint32_t width, uint32_t value)
{
    uint32_t bit = 0u;

    for (bit = 0u; bit < width; bit++)
    {
        uint32_t position = start + bit;
        uint8_t mask = (uint8_t)(1u << (position % 8u));

        if (0u != ((value >> bit) & 1u))
        {
            p_buffer[position / 8u] = (uint8_t)(p_buffer[position / 8u] | mask);
        }
        else
        {
            p_buffer[position / 8u] = (uint8_t)(p_buffer[position / 8u] & ~mask);
        }
    }
}

static uint32_t
ref_get(const uint8_t *p_buffer, uint32_t start, uint32_t width)
{
    uint32_t value = 0u;
    uint32_t bit = 0u;

    for (bit = 0u; bit < width; bit++)
    {
        uint32_t position = start + bit;

        if (0u != ((p_buffer[position / 8u] >> (position % 8u)) & 1u))
        {
            value |= (1u << bit);
        }
    }
    return value;
}

static uint32_t g_random = 0x12345678u;

static uint32_t
next_random(void)
{
    g_random ^= g_random << 13;
    g_random ^= g_random >> 17;
    g_random ^= g_random << 5;
    return g_random;
}

static void
test_known_patterns(void)
{
    uint8_t buffer[BUFFER_BYTES];
    uint32_t value = 0u;

    /* 3 bits holding 0b101 starting at bit 7 cross a byte boundary:
     * bit 7 of byte 0 gets the low bit (1), bits 0 and 1 of byte 1 get 0, 1. */
    memset(buffer, 0, sizeof(buffer));
    check(s1_bits_put(buffer, sizeof(buffer), 7u, 3u, 5u), "put crossing a byte");
    check((0x80u == buffer[0]) && (0x02u == buffer[1]), "crossing field lands on bits 7, 8, 9");

    memset(buffer, 0, sizeof(buffer));
    check(s1_bits_put(buffer, sizeof(buffer), 0u, 32u, 0x04030201u), "put 32 bits aligned");
    check((0x01u == buffer[0]) && (0x02u == buffer[1]) && (0x03u == buffer[2]) &&
              (0x04u == buffer[3]),
          "32-bit field is little-endian");

    memset(buffer, 0, sizeof(buffer));
    check(s1_bits_put(buffer, sizeof(buffer), 5u, 32u, 0xFFFFFFFFu), "put 32 bits unaligned");
    check((0xE0u == buffer[0]) && (0xFFu == buffer[1]) && (0xFFu == buffer[2]) &&
              (0xFFu == buffer[3]) && (0x1Fu == buffer[4]),
          "unaligned 32-bit field spans five bytes");
    check(s1_bits_get(buffer, sizeof(buffer), 5u, 32u, &value) && (0xFFFFFFFFu == value),
          "unaligned 32-bit field reads back");
}

static void
test_rejections(void)
{
    uint8_t buffer[BUFFER_BYTES];
    uint8_t before[BUFFER_BYTES];
    uint32_t value = 77u;

    memset(buffer, 0xA5, sizeof(buffer));
    memcpy(before, buffer, sizeof(buffer));

    check(!s1_bits_put(buffer, sizeof(buffer), 0u, 3u, 8u), "value too large for 3 bits rejected");
    check(!s1_bits_put(buffer, sizeof(buffer), 0u, 0u, 0u), "width 0 rejected");
    check(!s1_bits_put(buffer, sizeof(buffer), 0u, 33u, 0u), "width 33 rejected");
    check(!s1_bits_put(buffer, sizeof(buffer), (BUFFER_BYTES * 8u) - 2u, 3u, 1u),
          "field running past the end rejected");
    check(!s1_bits_put(buffer, sizeof(buffer), BUFFER_BYTES * 8u, 1u, 1u), "start at the end rejected");
    check(!s1_bits_put(NULL, sizeof(buffer), 0u, 1u, 1u), "null buffer rejected");
    check(!s1_bits_put(buffer, S1_BITS_MAX_BYTES + 1u, 0u, 1u, 1u), "huge length rejected");
    check(0 == memcmp(buffer, before, sizeof(buffer)), "a rejected write leaves the buffer untouched");

    check(!s1_bits_get(buffer, sizeof(buffer), (BUFFER_BYTES * 8u) - 2u, 3u, &value) &&
              (77u == value),
          "read past the end rejected, output untouched");
    check(!s1_bits_get(buffer, sizeof(buffer), 0u, 0u, &value), "read of width 0 rejected");
    check(!s1_bits_get(buffer, sizeof(buffer), 0u, 1u, NULL), "read into null rejected");
}

static void
test_every_width_and_offset(void)
{
    uint8_t buffer[BUFFER_BYTES];
    uint8_t expect[BUFFER_BYTES];
    uint32_t width = 0u;
    uint32_t start = 0u;
    uint32_t value = 0u;
    uint32_t got = 0u;
    uint32_t pat = 0u;
    int b_ok = 1;
    const uint32_t patterns[] = {0u, 0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 1u, 0x80000000u};

    for (width = 1u; width <= 32u; width++)
    {
        for (start = 0u; (start + width) <= (BUFFER_BYTES * 8u); start++)
        {
            for (pat = 0u; pat < (sizeof(patterns) / sizeof(patterns[0])); pat++)
            {
                value = patterns[pat] & s1_bits_mask(width);
                memset(buffer, 0xA5, sizeof(buffer));
                memcpy(expect, buffer, sizeof(buffer));
                ref_put(expect, start, width, value);
                b_ok = b_ok && s1_bits_put(buffer, sizeof(buffer), start, width, value) &&
                       (0 == memcmp(buffer, expect, sizeof(buffer))) &&
                       s1_bits_get(buffer, sizeof(buffer), start, width, &got) && (got == value);
            }
        }
    }
    check(b_ok, "every width 1..32 at every offset matches the reference; neighbours untouched");
}

static void
test_random_against_reference(void)
{
    uint8_t buffer[BUFFER_BYTES];
    uint8_t expect[BUFFER_BYTES];
    uint32_t trial = 0u;
    uint32_t width = 0u;
    uint32_t start = 0u;
    uint32_t value = 0u;
    uint32_t got = 0u;
    int b_ok = 1;
    unsigned index = 0u;

    for (index = 0u; index < BUFFER_BYTES; index++)
    {
        buffer[index] = (uint8_t)next_random();
    }
    memcpy(expect, buffer, sizeof(buffer));
    for (trial = 0u; trial < RANDOM_TRIALS; trial++)
    {
        width = 1u + (next_random() % 32u);
        start = next_random() % ((BUFFER_BYTES * 8u) - width + 1u);
        value = next_random() & s1_bits_mask(width);
        ref_put(expect, start, width, value);
        b_ok = b_ok && s1_bits_put(buffer, sizeof(buffer), start, width, value) &&
               (0 == memcmp(buffer, expect, sizeof(buffer))) &&
               s1_bits_get(buffer, sizeof(buffer), start, width, &got) &&
               (got == value) && (got == ref_get(expect, start, width));
    }
    check(b_ok, "200000 random writes and reads match the reference");
}

static void
test_signed(void)
{
    uint8_t buffer[BUFFER_BYTES];
    uint32_t width = 0u;
    int32_t value = 0;
    int32_t got = 0;
    int32_t low = 0;
    int32_t high = 0;
    int b_ok = 1;
    const int32_t probes[] = {0, 1, -1, 2, -2};
    unsigned index = 0u;

    for (width = 2u; width <= 32u; width++)
    {
        if (width < 32u)
        {
            low = -(int32_t)(1u << (width - 1u));
            high = (int32_t)((1u << (width - 1u)) - 1u);
        }
        else
        {
            low = INT32_MIN;
            high = INT32_MAX;
        }
        memset(buffer, 0x5A, sizeof(buffer));
        b_ok = b_ok && s1_bits_put_signed(buffer, sizeof(buffer), 3u, width, low) &&
               s1_bits_get_signed(buffer, sizeof(buffer), 3u, width, &got) && (got == low);
        b_ok = b_ok && s1_bits_put_signed(buffer, sizeof(buffer), 3u, width, high) &&
               s1_bits_get_signed(buffer, sizeof(buffer), 3u, width, &got) && (got == high);
        for (index = 0u; index < (sizeof(probes) / sizeof(probes[0])); index++)
        {
            value = probes[index];
            if (!s1_bits_fit_signed(value, width))
            {
                continue; /* 2 and -2 do not fit in a 2-bit field. */
            }
            b_ok = b_ok && s1_bits_put_signed(buffer, sizeof(buffer), 3u, width, value) &&
                   s1_bits_get_signed(buffer, sizeof(buffer), 3u, width, &got) && (got == value);
        }
        if (width < 32u)
        {
            memset(buffer, 0x5A, sizeof(buffer));
            b_ok = b_ok && !s1_bits_put_signed(buffer, sizeof(buffer), 3u, width, high + 1) &&
                   !s1_bits_put_signed(buffer, sizeof(buffer), 3u, width, low - 1) &&
                   (0x5Au == buffer[0]);
        }
    }
    check(b_ok, "signed fields of width 2..32 round-trip at the limits; one past either limit rejected");

    /* The 12-bit speed field of the telemetry frame. */
    memset(buffer, 0, sizeof(buffer));
    check(s1_bits_put_signed(buffer, sizeof(buffer), 63u, 12u, -2048) &&
              s1_bits_get_signed(buffer, sizeof(buffer), 63u, 12u, &got) && (-2048 == got),
          "speed field -2048");
    check(s1_bits_put_signed(buffer, sizeof(buffer), 63u, 12u, 2047) &&
              s1_bits_get_signed(buffer, sizeof(buffer), 63u, 12u, &got) && (2047 == got),
          "speed field +2047");
    check(!s1_bits_put_signed(buffer, sizeof(buffer), 63u, 12u, 2048), "speed field +2048 rejected");
    check(!s1_bits_put_signed(buffer, sizeof(buffer), 63u, 12u, -2049), "speed field -2049 rejected");
}

static void
test_fit(void)
{
    check(s1_bits_fit_unsigned(7u, 3u) && !s1_bits_fit_unsigned(8u, 3u), "3-bit unsigned limit");
    check(s1_bits_fit_unsigned(0xFFFFFFFFu, 32u), "32-bit unsigned accepts every value");
    check(!s1_bits_fit_unsigned(0u, 0u) && !s1_bits_fit_unsigned(0u, 33u), "unsigned width limits");
    check(s1_bits_fit_signed(-4, 3u) && s1_bits_fit_signed(3, 3u) &&
              !s1_bits_fit_signed(4, 3u) && !s1_bits_fit_signed(-5, 3u),
          "3-bit signed limits");
    check(s1_bits_fit_signed(INT32_MIN, 32u) && s1_bits_fit_signed(INT32_MAX, 32u),
          "32-bit signed accepts every value");
    check(!s1_bits_fit_signed(0, 0u) && !s1_bits_fit_signed(0, 33u), "signed width limits");
}

static void
test_fault_bits(void)
{
    const uint32_t bits[] = {S1_FAULT_MOTOR_STALL, S1_FAULT_ENCODER, S1_FAULT_IMU, S1_FAULT_NO_ECHO,
                             S1_FAULT_WIFI_LOSS, S1_FAULT_ESTOP_LATCHED, S1_FAULT_LINE_SENSOR,
                             S1_FAULT_BARCODE};
    uint32_t seen = 0u;
    unsigned index = 0u;
    int b_ok = 1;

    for (index = 0u; index < (sizeof(bits) / sizeof(bits[0])); index++)
    {
        b_ok = b_ok && (bits[index] == (1u << index)) && (0u == (seen & bits[index]));
        seen |= bits[index];
    }
    check(b_ok, "fault bits are single, distinct and in the documented order (bit 0 stall .. bit 7 barcode)");
    check((S1_FAULT_ALL == seen) && (S1_FAULT_BITS == (sizeof(bits) / sizeof(bits[0]))),
          "the eight fault bits cover exactly the 8-bit mask");
    check(S1_FAULT_OWNED_BY_S1 == 0x30u, "Buddy 1 owns bits 4 and 5");
}

int main(void)
{
    test_known_patterns();
    test_rejections();
    test_every_width_and_offset();
    test_random_against_reference();
    test_signed();
    test_fit();
    test_fault_bits();
    printf("\n%d checks, %d failed\n", g_checks, g_failures);
    return (0 == g_failures) ? 0 : 1;
}