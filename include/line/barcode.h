#ifndef BARCODE_H
#define BARCODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BARCODE_RUN_COUNT 29U

typedef enum
{
    BARCODE_RESULT_PENDING = 0,
    BARCODE_RESULT_VALID,
    BARCODE_RESULT_INVALID
} barcode_result_t;

/* Decode exactly one Code 39 uppercase letter, framed by two '*'.
 * Widths alternate black/white, starting and ending black. Either direction.
 * Units can be distance or time; traversal must preserve width ratios.
 * Quiet zones must be checked by the caller (capture below does so).
 */
bool barcode_decode(const uint32_t *p_widths, size_t count, char *p_letter);

typedef struct
{
    uint32_t widths[BARCODE_RUN_COUNT];
    uint64_t edge_us;
    uint64_t frame_start_us;
    uint64_t last_us;
    uint32_t quiet_us;
    uint32_t timeout_us;
    uint8_t count;
    bool started;
    bool black;
    bool armed;
    bool capturing;
} barcode_capture_t;

/* Timing is caller-supplied, measured for the supported speed range.
 * quiet_us must exceed all internal gaps and represent a full quiet zone.
 */
bool barcode_capture_init(barcode_capture_t *p_capture,
                          uint32_t quiet_us, uint32_t timeout_us);
barcode_result_t barcode_capture_update(barcode_capture_t *p_capture,
                                        uint64_t timestamp_us, bool black,
                                        char *p_letter);

#endif
