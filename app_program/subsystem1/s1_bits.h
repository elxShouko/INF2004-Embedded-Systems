/**
 * @file s1_bits.h
 * @brief Buddy 1: read and write bit fields of 1 to 32 bits in a byte array.
 *
 * Bit 0 is the least significant bit of byte 0, bit 8 the least significant
 * bit of byte 1, and so on. A field of N bits that starts at bit S uses bits S
 * to S+N-1 and keeps its least significant bit in the lowest numbered one
 * (little-endian, LSB-first). See docs/buddy1/TELEMETRY_COMMAND_CONTRACT.md,
 * section 6.
 *
 * NOTE: Every access is one byte at a time, with masks and shifts. The
 * Cortex-M0+ faults on an unaligned word access and fields cross byte
 * boundaries, so there is no cast to a wider type and no raw struct bit-field.
 * This header has no kernel, lwIP or libc dependency, so it is also tested on
 * a PC.
 *
 * NOTE: These functions write plain memory and are NOT safe for two writers
 * of the same buffer. Build a frame in a local array, then hand it over.
 */

#ifndef S1_BITS_H
#define S1_BITS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define S1_BITS_MAX_WIDTH 32u
#define S1_BITS_ALL_ONES 0xFFFFFFFFu
#define S1_BITS_PER_BYTE 8u
#define S1_BITS_BYTE_MASK 7u
#define S1_BITS_BYTE_SHIFT 3u
#define S1_BITS_MAX_BYTES 0x1FFFFFFFu /* Keeps length * 8 inside 32 bits. */

/** @brief Mask with the lowest `width` bits set (width 1 to 32). */
static inline uint32_t
s1_bits_mask(uint32_t width)
{
    return (width >= S1_BITS_MAX_WIDTH) ? S1_BITS_ALL_ONES
                                        : ((1u << width) - 1u);
}

/** @brief True if `value` needs no more than `width` bits. */
static inline bool
s1_bits_fit_unsigned(uint32_t value, uint32_t width)
{
    return (width >= 1u) && (width <= S1_BITS_MAX_WIDTH) &&
           (0u == (value & ~s1_bits_mask(width)));
}

/** @brief True if `value` fits in `width` bits as a two's complement number. */
static inline bool
s1_bits_fit_signed(int32_t value, uint32_t width)
{
    bool b_fits = false;
    int32_t limit = 0;

    if ((width >= 1u) && (width < S1_BITS_MAX_WIDTH))
    {
        limit = (int32_t)(1u << (width - 1u));
        b_fits = (value >= -limit) && (value < limit);
    }
    else if (S1_BITS_MAX_WIDTH == width)
    {
        b_fits = true;
    }
    else
    {
        b_fits = false;
    }
    return b_fits;
}

/** @brief True if the field lies inside a buffer of `length` bytes. */
static inline bool
s1_bits_in_range(uint32_t length, uint32_t start, uint32_t width)
{
    bool b_ok = false;
    uint32_t total = 0u;

    if ((width >= 1u) && (width <= S1_BITS_MAX_WIDTH) &&
        (length <= S1_BITS_MAX_BYTES))
    {
        total = length * S1_BITS_PER_BYTE;
        b_ok = (start <= total) && (width <= (total - start));
    }
    return b_ok;
}

/**
 * @brief Write an unsigned field. Bits outside the field are not changed.
 * @return false, with the buffer untouched, if the field is outside the
 * buffer, the width is not 1 to 32, or the value needs more than `width` bits.
 */
static inline bool
s1_bits_put(uint8_t *p_buffer, uint32_t length, uint32_t start,
            uint32_t width, uint32_t value)
{
    bool b_ok = (NULL != p_buffer) && s1_bits_in_range(length, start, width) &&
                s1_bits_fit_unsigned(value, width);
    uint32_t position = start;
    uint32_t remaining = width;
    uint32_t rest = value;
    uint32_t index = 0u;
    uint32_t offset = 0u;
    uint32_t count = 0u;
    uint32_t mask = 0u;

    while (b_ok && (remaining > 0u))
    {
        index = position >> S1_BITS_BYTE_SHIFT;
        offset = position & S1_BITS_BYTE_MASK;
        count = S1_BITS_PER_BYTE - offset;
        if (count > remaining)
        {
            count = remaining;
        }
        mask = ((1u << count) - 1u) << offset;
        p_buffer[index] = (uint8_t)(((uint32_t)p_buffer[index] & ~mask) |
                                    ((rest << offset) & mask));
        rest >>= count;
        position += count;
        remaining -= count;
    }
    return b_ok;
}

/**
 * @brief Read an unsigned field.
 * @return false, with *p_value untouched, if the field is outside the buffer
 * or the width is not 1 to 32.
 */
static inline bool
s1_bits_get(const uint8_t *p_buffer, uint32_t length, uint32_t start,
            uint32_t width, uint32_t *p_value)
{
    bool b_ok = (NULL != p_buffer) && (NULL != p_value) &&
                s1_bits_in_range(length, start, width);
    uint32_t position = start;
    uint32_t remaining = width;
    uint32_t result = 0u;
    uint32_t got = 0u;
    uint32_t index = 0u;
    uint32_t offset = 0u;
    uint32_t count = 0u;
    uint32_t chunk = 0u;

    while (b_ok && (remaining > 0u))
    {
        index = position >> S1_BITS_BYTE_SHIFT;
        offset = position & S1_BITS_BYTE_MASK;
        count = S1_BITS_PER_BYTE - offset;
        if (count > remaining)
        {
            count = remaining;
        }
        chunk = ((uint32_t)p_buffer[index] >> offset) & ((1u << count) - 1u);
        result |= chunk << got;
        got += count;
        position += count;
        remaining -= count;
    }
    if (b_ok)
    {
        *p_value = result;
    }
    return b_ok;
}

/**
 * @brief Write a two's complement field.
 * @return false, with the buffer untouched, if the value does not fit.
 */
static inline bool
s1_bits_put_signed(uint8_t *p_buffer, uint32_t length, uint32_t start,
                   uint32_t width, int32_t value)
{
    return s1_bits_fit_signed(value, width) &&
           s1_bits_put(p_buffer, length, start, width,
                       ((uint32_t)value) & s1_bits_mask(width));
}

/** @brief Read a two's complement field and sign-extend it. */
static inline bool
s1_bits_get_signed(const uint8_t *p_buffer, uint32_t length, uint32_t start,
                   uint32_t width, int32_t *p_value)
{
    uint32_t raw = 0u;
    uint32_t sign = 0u;
    bool b_ok =
        (NULL != p_value) && s1_bits_get(p_buffer, length, start, width, &raw);

    if (b_ok)
    {
        sign = 1u << (width - 1u);
        if (0u == (raw & sign))
        {
            *p_value = (int32_t)raw;
        }
        else
        {
            *p_value = -(int32_t)((~raw) & s1_bits_mask(width)) - 1;
        }
    }
    return b_ok;
}

#endif /* S1_BITS_H */

/*** end of file ***/