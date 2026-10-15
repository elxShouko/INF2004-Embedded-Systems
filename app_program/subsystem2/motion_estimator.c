/**
 * @file motion_estimator.c
 * @brief Checked integer arithmetic and transactional wheel odometry.
 */
#include "motion_estimator.h"
#include <stddef.h>

#define MOTION_ESTIMATOR_SPEED_SCALE (1000U)

static bool motion_estimator_add (int64_t first, int64_t second,
                                 int64_t * p_sum);
static uint64_t motion_estimator_magnitude (int64_t value);
static bool motion_estimator_product (int64_t count, uint32_t scale,
                                     int64_t * p_product);
static motion_estimator_result_t motion_estimator_time (
    motion_estimator_t const * p_estimator,
    motion_estimator_sample_t const * p_sample, uint64_t now_us);
static motion_estimator_result_t motion_estimator_calculate (
    motion_estimator_t * p_estimator,
    motion_estimator_sample_t const * p_sample);
static bool motion_estimator_speed (int64_t numerator, uint64_t denominator,
                                   motion_types_speed_mm_s_t * p_speed);

motion_estimator_result_t motion_estimator_init (
    motion_estimator_t * p_estimator, motion_config_t const * p_config,
    motion_estimator_wheel_t wheel, uint32_t max_elapsed_us)
{
    motion_estimator_result_t result = MOTION_ESTIMATOR_NULL;

    if (NULL != p_estimator)
    {
        *p_estimator = (motion_estimator_t){0};
        p_estimator->result = MOTION_ESTIMATOR_UNAVAILABLE;
        result = MOTION_ESTIMATOR_BAD_CONFIG;
        if ((MOTION_CONFIG_VALID == motion_config_validate(p_config)) &&
            ((MOTION_ESTIMATOR_LEFT == wheel) ||
             (MOTION_ESTIMATOR_RIGHT == wheel)) && (0U != max_elapsed_us))
        {
            motion_config_wheel_t const * p_wheel =
                (MOTION_ESTIMATOR_LEFT == wheel) ?
                &p_config->left : &p_config->right;

            p_estimator->counts_per_wheel_rev = p_wheel->counts_per_wheel_rev;
            p_estimator->circumference_um = p_wheel->wheel_circumference_um;
            p_estimator->max_elapsed_us = max_elapsed_us;
            p_estimator->b_initialised = true;
            result = MOTION_ESTIMATOR_OK;
        }
    }

    return (result);
}

motion_estimator_result_t motion_estimator_reset (
    motion_estimator_t * p_estimator)
{
    motion_estimator_result_t result = MOTION_ESTIMATOR_NULL;

    if (NULL != p_estimator)
    {
        result = MOTION_ESTIMATOR_UNAVAILABLE;
        if (p_estimator->b_initialised)
        {
            p_estimator->previous = (motion_estimator_sample_t){0};
            p_estimator->measurement = (motion_estimator_measurement_t){0};
            p_estimator->remainder = 0;
            p_estimator->b_have_baseline = false;
            p_estimator->b_reset_required = false;
            p_estimator->result = MOTION_ESTIMATOR_UNAVAILABLE;
            result = MOTION_ESTIMATOR_OK;
        }
    }

    return (result);
}

motion_estimator_result_t motion_estimator_update (
    motion_estimator_t * p_estimator,
    motion_estimator_sample_t const * p_sample, uint64_t now_us,
    motion_estimator_measurement_t * p_measurement)
{
    motion_estimator_result_t result = MOTION_ESTIMATOR_NULL;

    motion_estimator_measurement_t measurement = {0};
    if ((NULL == p_estimator) || (NULL == p_sample) ||
        (NULL == p_measurement))
    {
        result = MOTION_ESTIMATOR_NULL;
        if (NULL != p_estimator)
        {
            p_estimator->result = result;
            p_estimator->measurement.b_speed_valid = false;
            p_estimator->measurement.speed_mm_s = 0;
            measurement = p_estimator->measurement;
        }
    }
    else if (!p_estimator->b_initialised)
    {
        result = MOTION_ESTIMATOR_UNAVAILABLE;
    }
    else
    {
        result = motion_estimator_time(p_estimator, p_sample, now_us);
        if ((MOTION_ESTIMATOR_OK == result) &&
            (!p_estimator->b_have_baseline))
        {
            p_estimator->previous = *p_sample;
            p_estimator->b_have_baseline = true;
            p_estimator->measurement.timestamp_us = p_sample->timestamp_us;
            p_estimator->measurement.b_position_valid = true;
            result = MOTION_ESTIMATOR_BASELINE;
        }
        else if (MOTION_ESTIMATOR_OK == result)
        {
            result = motion_estimator_calculate(p_estimator, p_sample);
        }
        else
        {
            /* No baseline/position change on a rejected measurement. */
        }
        p_estimator->result = result;
        if ((MOTION_ESTIMATOR_RESET_REQUIRED == result) ||
            (p_estimator->b_have_baseline &&
             (p_sample->timestamp_us < p_estimator->previous.timestamp_us)) ||
            (p_estimator->b_have_baseline &&
             (p_sample->timestamp_us > p_estimator->previous.timestamp_us) &&
             ((p_sample->timestamp_us - p_estimator->previous.timestamp_us) >
              p_estimator->max_elapsed_us)))
        {
            p_estimator->b_reset_required = true;
        }
        if (MOTION_ESTIMATOR_OK != result)
        {
            p_estimator->measurement.b_speed_valid = false;
            p_estimator->measurement.speed_mm_s = 0;
        }
        measurement = p_estimator->measurement;
    }
    if (NULL != p_measurement)
    {
        *p_measurement = measurement;
    }

    return (result);
}

motion_estimator_result_t motion_estimator_get (
    motion_estimator_t const * p_estimator, uint64_t now_us,
    motion_estimator_measurement_t * p_measurement)
{
    motion_estimator_result_t result = MOTION_ESTIMATOR_NULL;

    motion_estimator_measurement_t measurement = {0};
    if ((NULL == p_estimator) || (NULL == p_measurement))
    {
        result = MOTION_ESTIMATOR_NULL;
    }
    else if ((!p_estimator->b_initialised) || (!p_estimator->b_have_baseline))
    {
        result = MOTION_ESTIMATOR_UNAVAILABLE;
    }
    else
    {
        measurement = p_estimator->measurement;
        result = p_estimator->result;
        if (now_us < measurement.timestamp_us)
        {
            result = MOTION_ESTIMATOR_BAD_TIME;
        }
        else if ((now_us - measurement.timestamp_us) >
                 p_estimator->max_elapsed_us)
        {
            result = MOTION_ESTIMATOR_STALE;
        }
        else
        {
            /* Last result and validity remain visible while fresh. */
        }
        if (MOTION_ESTIMATOR_OK != result)
        {
            measurement.b_speed_valid = false;
            measurement.speed_mm_s = 0;
        }
    }
    if (NULL != p_measurement)
    {
        *p_measurement = measurement;
    }

    return (result);
}

static bool motion_estimator_add (int64_t first, int64_t second,
                                 int64_t * p_sum)
{
    bool b_valid = !(((second > 0) && (first > (INT64_MAX - second))) ||
                     ((second < 0) && (first < (INT64_MIN - second))));

    if (b_valid)
    {
        *p_sum = first + second;
    }

    return (b_valid);
}

static uint64_t motion_estimator_magnitude (int64_t value)
{
    /* Never negate INT64_MIN. Each cast converts a nonnegative value. */
    uint64_t magnitude = (value < 0) ?
        (uint64_t)(-(value + 1)) + 1U : (uint64_t)value;

    return (magnitude);
}

static bool motion_estimator_product (int64_t count, uint32_t scale,
                                     int64_t * p_product)
{
    uint64_t magnitude = motion_estimator_magnitude(count);
    /* INT64_MAX and its successor both fit uint64_t. */
    uint64_t limit = (uint64_t)INT64_MAX + ((count < 0) ? 1U : 0U);
    bool b_valid = (0U != scale) && (magnitude <= (limit / scale));

    if (b_valid)
    {
        magnitude *= scale;
        if (magnitude > (uint64_t)INT64_MAX)
        {
            *p_product = INT64_MIN; /* Only the negative limit can be here. */
        }
        else
        {
            /* Bound above guarantees the conversion fits int64_t. */
            *p_product = (count < 0) ? -(int64_t)magnitude :
                                      (int64_t)magnitude;
        }
    }

    return (b_valid);
}

static motion_estimator_result_t motion_estimator_time (
    motion_estimator_t const * p_estimator,
    motion_estimator_sample_t const * p_sample, uint64_t now_us)
{
    motion_estimator_result_t result = MOTION_ESTIMATOR_OK;

    if (p_estimator->b_reset_required)
    {
        result = MOTION_ESTIMATOR_RESET_REQUIRED;
    }
    else if ((0U == p_estimator->counts_per_wheel_rev) ||
        (0U == p_estimator->circumference_um) ||
        (0U == p_estimator->max_elapsed_us))
    {
        result = MOTION_ESTIMATOR_BAD_CONFIG;
    }
    else if (now_us < p_sample->timestamp_us)
    {
        result = MOTION_ESTIMATOR_BAD_TIME;
    }
    else if ((now_us - p_sample->timestamp_us) > p_estimator->max_elapsed_us)
    {
        result = MOTION_ESTIMATOR_STALE;
    }
    else if (p_estimator->b_have_baseline &&
             (p_sample->generation != p_estimator->previous.generation))
    {
        result = MOTION_ESTIMATOR_RESET_REQUIRED;
    }
    else if (p_estimator->b_have_baseline &&
             (p_sample->timestamp_us <= p_estimator->previous.timestamp_us))
    {
        result = MOTION_ESTIMATOR_BAD_TIME;
    }
    else if (p_estimator->b_have_baseline &&
             ((p_sample->timestamp_us - p_estimator->previous.timestamp_us) >
              p_estimator->max_elapsed_us))
    {
        result = MOTION_ESTIMATOR_STALE;
    }
    else
    {
        /* Timing and source generation are consistent. */
    }

    return (result);
}

static bool motion_estimator_speed (int64_t numerator, uint64_t denominator,
                                   motion_types_speed_mm_s_t * p_speed)
{
    uint64_t magnitude = motion_estimator_magnitude(numerator);
    bool b_valid = (0U != denominator) &&
        (magnitude <= (UINT64_MAX / MOTION_ESTIMATOR_SPEED_SCALE));

    if (b_valid)
    {
        magnitude = (magnitude * MOTION_ESTIMATOR_SPEED_SCALE) / denominator;
        /* Reverse permits the asymmetric INT32_MIN magnitude. */
        uint64_t limit = (uint64_t)INT32_MAX + ((numerator < 0) ? 1U : 0U);

        b_valid = (magnitude <= limit);
        if (b_valid)
        {
            if (magnitude > (uint64_t)INT32_MAX)
            {
                *p_speed = INT32_MIN;
            }
            else
            {
                /* Bounds guarantee this cast and subsequent negation fit. */
                *p_speed = (numerator < 0) ? -(int32_t)magnitude :
                                            (int32_t)magnitude;
            }
        }
    }

    return (b_valid);
}

static motion_estimator_result_t motion_estimator_calculate (
    motion_estimator_t * p_estimator,
    motion_estimator_sample_t const * p_sample)
{
    motion_estimator_result_t result = MOTION_ESTIMATOR_OVERFLOW;
    int64_t previous = p_estimator->previous.count;
    int64_t current = p_sample->count;
    bool b_delta_valid = !(((previous < 0) &&
                           (current > (INT64_MAX + previous))) ||
                          ((previous > 0) &&
                           (current < (INT64_MIN + previous))));
    int64_t numerator = 0;
    int64_t total = 0;
    int64_t position = 0;
    motion_types_speed_mm_s_t speed = 0;
    uint64_t elapsed = p_sample->timestamp_us -
                       p_estimator->previous.timestamp_us;
    /* Timing validation bounds elapsed to uint32_t; this product fits u64. */
    uint64_t denominator = elapsed * p_estimator->counts_per_wheel_rev;
    /* Every uint32_t value is representable in int64_t. Keep subsequent
     * signed position/remainder arithmetic consistently signed.
     */
    int64_t counts_per_rev = p_estimator->counts_per_wheel_rev;

    if (b_delta_valid &&
        (motion_estimator_product(current - previous,
                                 p_estimator->circumference_um, &numerator)) &&
        (motion_estimator_add(numerator, p_estimator->remainder, &total)) &&
        (motion_estimator_add(p_estimator->measurement.position_um,
                            total / counts_per_rev,
                            &position)) &&
        (motion_estimator_speed(numerator, denominator, &speed)))
    {
        int64_t remainder = total % counts_per_rev;

        /* Canonical truncation of TOTAL position across direction changes. */
        if ((position > 0) && (remainder < 0))
        {
            --position;
            remainder += counts_per_rev;
        }
        else if ((position < 0) && (remainder > 0))
        {
            ++position;
            remainder -= counts_per_rev;
        }
        else
        {
            /* Whole and fractional position already have consistent signs. */
        }
        p_estimator->previous = *p_sample;
        p_estimator->remainder = remainder;
        p_estimator->measurement.position_um = position;
        p_estimator->measurement.speed_mm_s = speed;
        p_estimator->measurement.timestamp_us = p_sample->timestamp_us;
        p_estimator->measurement.b_position_valid = true;
        p_estimator->measurement.b_speed_valid = true;
        result = MOTION_ESTIMATOR_OK;
    }

    return (result);
}

/*** end of file ***/

