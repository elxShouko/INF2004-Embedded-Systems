/**
 * @file motion_estimator.h
 * @brief Pure per-wheel estimation from vehicle-oriented decoder counts.
 */
#ifndef MOTION_ESTIMATOR_H
#define MOTION_ESTIMATOR_H

#include "motion_config.h"

typedef enum
{
    MOTION_ESTIMATOR_LEFT = 0,
    MOTION_ESTIMATOR_RIGHT
} motion_estimator_wheel_t;

typedef enum
{
    MOTION_ESTIMATOR_OK = 0,
    MOTION_ESTIMATOR_BASELINE,
    MOTION_ESTIMATOR_UNAVAILABLE,
    MOTION_ESTIMATOR_NULL,
    MOTION_ESTIMATOR_BAD_CONFIG,
    MOTION_ESTIMATOR_BAD_TIME,
    MOTION_ESTIMATOR_STALE,
    MOTION_ESTIMATOR_RESET_REQUIRED,
    MOTION_ESTIMATOR_OVERFLOW
} motion_estimator_result_t;

typedef struct
{
    motion_types_encoder_count_t count;
    uint64_t timestamp_us;
    /* Change on any source-count reset. Signed counts must NOT wrap. */
    uint32_t generation;
} motion_estimator_sample_t;

typedef struct
{
    motion_types_distance_um_t position_um;
    motion_types_speed_mm_s_t speed_mm_s;
    uint64_t timestamp_us;
    bool b_position_valid;
    bool b_speed_valid;
} motion_estimator_measurement_t;

/* Owner-only state; treat copied calibration as immutable until re-init. */
typedef struct
{
    uint32_t counts_per_wheel_rev;
    uint32_t circumference_um;
    uint32_t max_elapsed_us;
    motion_estimator_sample_t previous;
    motion_estimator_measurement_t measurement;
    int64_t remainder;
    motion_estimator_result_t result;
    bool b_initialised;
    bool b_have_baseline;
    bool b_reset_required;
} motion_estimator_t;

/* Requires M1A VALID and an explicit nonzero application timing limit.
 * Copies the selected wheel's scale. No calibration/timing is guessed.
 * Encoder polarity is NOT applied again; input counts are vehicle-oriented.
 * Caller must reject unhealthy/COUNT_LIMIT decoder results; a held faulty
 * count must not be mistaken for a valid stationary wheel measurement.
 */
motion_estimator_result_t motion_estimator_init (
    motion_estimator_t * p_estimator, motion_config_t const * p_config,
    motion_estimator_wheel_t wheel, uint32_t max_elapsed_us);

/* New odometry origin: clear position, fractional remainder and baseline;
 * keep calibration. Next update captures count/time/generation, with no speed.
 */
motion_estimator_result_t motion_estimator_reset (
    motion_estimator_t * p_estimator);

/* Timestamp must increase strictly; now_us must not precede the sample.
 * Both sample age and interval must be <= max_elapsed_us. A source reset,
 * timestamp wrap/backward jump or excessive gap latches RESET_REQUIRED after
 * its initial diagnostic and needs explicit reset. Duplicate timestamps are
 * rejected but do not latch. Output must not alias instance or sample state.
 * Bad updates do not advance position/baseline and invalidate speed.
 * With a nonnull object, a null sample or output records NULL and clears
 * cached speed validity/value, preserving historical position, timestamp,
 * fractional remainder and accepted baseline. A later valid sample recovers
 * using that baseline under the same timing/generation/reset rules.
 * A null object is rejected; any nonnull output is cleared.
 * Distance retains fractional micrometres between updates, truncating the
 * accumulated position toward zero. Speed is unfiltered and truncates toward
 * zero in mm/s. Intermediate overflow is rejected even if a later division
 * could make the mathematical result fit. No signed wrapping is inferred.
 */
motion_estimator_result_t motion_estimator_update (
    motion_estimator_t * p_estimator,
    motion_estimator_sample_t const * p_sample, uint64_t now_us,
    motion_estimator_measurement_t * p_measurement);

/* Copy latest measurement with freshness checked at the consumer's time.
 * A stale/error result may retain last position, but never a valid speed.
 */
motion_estimator_result_t motion_estimator_get (
    motion_estimator_t const * p_estimator, uint64_t now_us,
    motion_estimator_measurement_t * p_measurement);

#endif /* MOTION_ESTIMATOR_H */

/*** end of file ***/

