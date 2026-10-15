/** @file drv_ir_input.h
 * @brief Member 3 drv ir input module.
 * @note See BARR_COMPLIANCE.md for compatibility exceptions under review.
 */

#ifndef IR_INPUT_H
#define IR_INPUT_H

#include <stdbool.h>
#include <stdint.h>

/* User-confirmed: line 1 = car's left, line 2 = car's right. */
typedef enum
{
    IR_CHANNEL_LINE_1 = 0,
    IR_CHANNEL_LINE_2,
    IR_CHANNEL_BARCODE,
    IR_CHANNEL_COUNT
} ir_channel_t;

typedef struct
{
    uint64_t timestamp_us;
    uint8_t  levels; /* Bit n is the raw DOUT level of channel n. */
} ir_sample_t;

typedef struct
{
    uint32_t samples;
    uint32_t high_count[IR_CHANNEL_COUNT];
} ir_observations_t;

typedef struct
{
    uint8_t black_high_mask;
    uint8_t valid_mask;
} ir_calibration_t;

/* The caller explicitly supplies the surface under ALL three sensors. */
/** @brief Accumulate a raw reference sample without counter overflow. */
bool ir_observe (ir_observations_t * p_observations,
                 const ir_sample_t * p_sample);
/* Accept only stable, opposite black/white observations for each channel. */
/** @brief Validate opposite stable black and white reference readings. */
bool ir_calibrate (const ir_observations_t * p_black,
                   const ir_observations_t * p_white, uint32_t minimum_samples,
                   ir_calibration_t * p_calibration);
/** @brief Convert calibrated GPIO levels into a black-surface mask. */
bool ir_normalize (const ir_sample_t *      p_sample,
                   const ir_calibration_t * p_calibration,
                   uint8_t *                p_black_mask);

typedef struct
{
    uint64_t candidate_since_us;
    uint64_t last_timestamp_us;
    uint32_t stable_us;
    uint8_t  candidate;
    uint8_t  stable;
    bool     started;
    bool     valid;
} ir_filter_t;

/* No assumed filtering duration: the caller must supply a measured value. */
/** @brief Reset stability filtering using a caller-selected interval. */
bool ir_filter_init (ir_filter_t * p_filter, uint32_t stable_us);
/** @brief Retain the last stable pattern until a new pattern settles. */
bool ir_filter_update (ir_filter_t * p_filter, const ir_sample_t * p_sample,
                       ir_sample_t * p_filtered);

#endif

/*** End of file. ***/

