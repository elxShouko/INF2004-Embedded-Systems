/**
 * @file motion_foundation_test.c
 * @brief Pure synthetic tests. All parameters below are TEST FIXTURES ONLY.
 *
 * S2-U13..U42 are reserved here; official testing/ logs are out of scope.
 * No fixture is a robot calibration value or a physical stability claim.
 */
#include "motion_foundation_test.h"

#if defined(MOTION_CONFIG_HOST_TEST) || APP_TEST
#include <stddef.h>
#include <float.h>
#include <math.h>
#if defined(MOTION_CONFIG_HOST_TEST)
#include <stdio.h>
#else
/* Translation-unit-only standard size_t, matching the S2/S3 test adapters. */
#define PROHIBIT_DEF_SIZE_T 1
#include "subsystem2.h"
#endif

#include "motion_quad.h"
#include "motion_estimator.h"
#include "motion_pid.h"

#define MOTION_FOUNDATION_GROUPS (30U)
#define MOTION_FOUNDATION_TOLERANCE (0.00001F)
#define MOTION_FOUNDATION_TRIALS (2U)
#define MOTION_FOUNDATION_SATURATION_STEPS (10U)
#define MOTION_FOUNDATION_WALK_STEPS (1000U)

static motion_config_t motion_foundation_config(void);
static motion_pid_config_t motion_foundation_pid_cfg(void);
static bool motion_foundation_near(float first, float second);
static uint32_t motion_foundation_report(bool b_pass, char const * p_id,
                                         char const * p_description);
static bool motion_foundation_quad_cycles(void);
static bool motion_foundation_quad_invalid(void);
static bool motion_foundation_quad_args(void);
static bool motion_foundation_quad_limits(void);
static bool motion_foundation_quad_pair(void);
static bool motion_foundation_quad_table(void);
static bool motion_foundation_est_distance(void);
static bool motion_foundation_est_pair(void);
static bool motion_foundation_est_rounding(void);
static bool motion_foundation_est_walk(void);
static bool motion_foundation_est_time(void);
static bool motion_foundation_est_limits(void);
static bool motion_foundation_est_extremes(void);
static bool motion_foundation_est_reset(void);
static bool motion_foundation_est_config(void);
static bool motion_foundation_pid_response(void);
static bool motion_foundation_pid_integral(void);
static bool motion_foundation_pid_windup(void);
static bool motion_foundation_pid_deriv(void);
static bool motion_foundation_pid_reset(void);
static bool motion_foundation_pid_config(void);
static bool motion_foundation_pid_limits(void);
static bool motion_foundation_pid_pair(void);
static bool motion_foundation_snapshots(void);
static bool motion_foundation_pipeline(void);

static bool motion_foundation_pid_headroom(void);
static bool motion_foundation_pid_crossing(void);
static bool motion_foundation_pid_unwind(void);
static bool motion_foundation_est_null_cache(void);
static bool motion_foundation_est_speed_edges(void);
static bool motion_foundation_pid_bad_bounds(void);
static bool motion_foundation_pid_nonfinite(void);

uint32_t motion_foundation_test(void)
{
    uint32_t failed = 0U;

    failed += motion_foundation_report(motion_foundation_quad_cycles(),
        "S2-U13", "quadrature directions, reversal and repeated states");
    failed += motion_foundation_report(motion_foundation_quad_invalid(),
        "S2-U14", "invalid jumps resync; diagnostics saturate");
    failed += motion_foundation_report(motion_foundation_quad_args(),
        "S2-U15", "decoder init, null, polarity and state validation");
    failed += motion_foundation_report(motion_foundation_quad_limits(),
        "S2-U16", "signed count limits latch until explicit reset");
    failed += motion_foundation_report(motion_foundation_quad_pair(),
        "S2-U17", "independent decoders apply polarity exactly once");
    failed += motion_foundation_report(motion_foundation_quad_table(),
        "S2-U18", "all sixteen state transitions match the convention");
    failed += motion_foundation_report(motion_foundation_est_distance(),
        "S2-U19", "distance, signed speed and actual variable elapsed time");
    failed += motion_foundation_report(motion_foundation_est_pair(),
        "S2-U20", "independent left/right wheel scales");
    failed += motion_foundation_report(motion_foundation_est_rounding() &&
        motion_foundation_est_walk(),
        "S2-U21", "fractional distance, reversal and repeatability");
    failed += motion_foundation_report(motion_foundation_est_time(),
        "S2-U22", "zero/backward/future/stale timing and fresh zero speed");
    failed += motion_foundation_report(motion_foundation_est_limits() &&
        motion_foundation_est_extremes(),
        "S2-U23", "count, product, position and speed overflow rejection");
    failed += motion_foundation_report(motion_foundation_est_reset(),
        "S2-U24", "source generation and explicit odometry baseline reset");
    failed += motion_foundation_report(motion_foundation_est_config(),
        "S2-U25", "invalid/unverified calibration and null estimation");
    failed += motion_foundation_report(motion_foundation_pid_response(),
        "S2-U26", "zero error and signed proportional response");
    failed += motion_foundation_report(motion_foundation_pid_integral(),
        "S2-U27", "integral limits, variable elapsed time and reversal");
    failed += motion_foundation_report(motion_foundation_pid_windup(),
        "S2-U28", "output saturation, anti-windup and recovery");
    failed += motion_foundation_report(motion_foundation_pid_deriv(),
        "S2-U29", "derivative on measurement avoids target kick");
    failed += motion_foundation_report(motion_foundation_pid_reset(),
        "S2-U30", "reset, idle, stop and invalid timing clear dynamics");
    failed += motion_foundation_report(motion_foundation_pid_config(),
        "S2-U31", "invalid gains, bounds, confirmation and null arguments");
    failed += motion_foundation_report(motion_foundation_pid_limits(),
        "S2-U32", "nonfinite arithmetic disables output safely");
    failed += motion_foundation_report(motion_foundation_pid_pair(),
        "S2-U33", "independent PID state for both wheels");
    failed += motion_foundation_report(motion_foundation_snapshots(),
        "S2-U34", "caller edits do not alter copied calibration");
    failed += motion_foundation_report(motion_foundation_pipeline(),
        "S2-U35", "decoder to estimator to PID preserves vehicle sign");
    failed += motion_foundation_report(motion_foundation_pid_crossing(),
        "S2-U36", "pure-I large steps reach both output limits repeatedly");
    failed += motion_foundation_report(motion_foundation_pid_unwind(),
        "S2-U37", "integral crosses saturation and recovers from overspeed");
    failed += motion_foundation_report(motion_foundation_est_null_cache(),
        "S2-U38", "null updates invalidate cached speed and retain baseline");
    failed += motion_foundation_report(motion_foundation_est_speed_edges(),
        "S2-U39", "nonzero fractional speeds and exact timing boundaries");
    failed += motion_foundation_report(motion_foundation_pid_bad_bounds(),
        "S2-U40", "all PID bounds reject nonfinite and invalid values");
    failed += motion_foundation_report(motion_foundation_pid_nonfinite(),
        "S2-U41", "derivative, integral and output overflow disable PID");
    failed += motion_foundation_report(motion_foundation_pid_headroom(),
        "S2-U42", "asymmetric output headroom includes P/D and I bounds");
#if defined(MOTION_CONFIG_HOST_TEST)
    printf("[TEST] subsystem2 M1B-M1D SUMMARY %u passed, %u failed\n",
           MOTION_FOUNDATION_GROUPS - failed, failed);
#else
    /* T-Monitor's supplied API uses UB*, so adapt the constant format only. */
    tm_printf((UB*)"[TEST] subsystem2 M1B-M1D SUMMARY %u passed, %u failed\n",
              MOTION_FOUNDATION_GROUPS - failed, failed);
#endif

    return (failed);
}

static motion_config_t motion_foundation_config(void)
{
    motion_config_t config = {0};

    config.left = (motion_config_wheel_t)
        {100U, 200000U, MOTION_TYPES_POLARITY_NORMAL,
         MOTION_TYPES_POLARITY_NORMAL};
    config.right = (motion_config_wheel_t)
        {120U, 210000U, MOTION_TYPES_POLARITY_REVERSED,
         MOTION_TYPES_POLARITY_REVERSED};
    config.track_width_um = 150000U;
    config.max_wheel_speed_mm_s = 500;
    config.calibration = MOTION_CONFIG_CALIBRATION_CONFIRMED;

    return (config);
}

static motion_pid_config_t motion_foundation_pid_cfg(void)
{
    motion_pid_config_t config = {0};

    config.proportional_gain = 0.001F;
    config.output_min = -1.0F;
    config.output_max = 1.0F;
    config.integral_min = -0.5F;
    config.integral_max = 0.5F;
    config.max_elapsed_us = 1000000U;
    config.calibration = MOTION_CONFIG_CALIBRATION_CONFIRMED;

    return (config);
}

static bool motion_foundation_near(float first, float second)
{
    float difference = first - second;

    return ((0 != isfinite(difference)) &&
            (difference >= -MOTION_FOUNDATION_TOLERANCE) &&
            (difference <= MOTION_FOUNDATION_TOLERANCE));
}

static uint32_t motion_foundation_report(bool b_pass, char const * p_id,
                                         char const * p_description)
{
#if defined(MOTION_CONFIG_HOST_TEST)
    printf("[TEST] %s %s  %s\n", p_id, b_pass ? "PASS" : "FAIL",
           p_description);
#else
    /* Required constant-format adaptation to the supplied T-Monitor API. */
    tm_printf((UB*)"[TEST] %s %s  %s\n", p_id,
              b_pass ? "PASS" : "FAIL", p_description);
#endif

    return (b_pass ? 0U : 1U);
}

static bool motion_foundation_quad_cycles(void)
{
    static uint8_t const forward[] = {0U, 1U, 3U, 2U, 0U};
    static uint8_t const reverse[] = {2U, 3U, 1U, 0U};
    motion_quad_t decoder = {0};
    bool b_pass = (MOTION_QUAD_OK ==
        motion_quad_init(&decoder, MOTION_TYPES_POLARITY_NORMAL));

    for (size_t index = 0U; index < sizeof(forward); ++index)
    {
        motion_quad_result_t expected = (0U == index) ?
            MOTION_QUAD_BASELINE : MOTION_QUAD_OK;

        b_pass = (expected == motion_quad_update(&decoder, forward[index])) &&
                 b_pass;
    }
    b_pass = (4 == decoder.count) && b_pass;
    b_pass = (MOTION_QUAD_REPEATED == motion_quad_update(&decoder, 0U)) &&
             (4 == decoder.count) && b_pass;
    for (size_t index = 0U; index < sizeof(reverse); ++index)
    {
        b_pass = (MOTION_QUAD_OK ==
                  motion_quad_update(&decoder, reverse[index])) && b_pass;
    }
    b_pass = (0 == decoder.count) && b_pass;
    b_pass = (MOTION_QUAD_OK == motion_quad_update(&decoder, 2U)) &&
             (-1 == decoder.count) && b_pass;

    return (b_pass);
}

static bool motion_foundation_quad_invalid(void)
{
    motion_quad_t decoder = {0};
    bool b_pass = (MOTION_QUAD_OK ==
        motion_quad_init(&decoder, MOTION_TYPES_POLARITY_NORMAL));

    b_pass = (MOTION_QUAD_BASELINE == motion_quad_update(&decoder, 0U)) &&
             b_pass;
    b_pass = (MOTION_QUAD_INVALID_TRANSITION ==
              motion_quad_update(&decoder, 3U)) &&
             (0 == decoder.count) && (1U == decoder.invalid_transitions) &&
             b_pass;
    b_pass = (MOTION_QUAD_OK == motion_quad_update(&decoder, 2U)) &&
             (1 == decoder.count) && b_pass;
    decoder.invalid_transitions = UINT32_MAX;
    b_pass = (MOTION_QUAD_INVALID_TRANSITION ==
              motion_quad_update(&decoder, 1U)) &&
             (UINT32_MAX == decoder.invalid_transitions) && b_pass;

    return (b_pass);
}

static bool motion_foundation_quad_args(void)
{
    motion_quad_t decoder = {0};
    bool b_pass = (MOTION_QUAD_UNINITIALISED ==
                   motion_quad_update(&decoder, 0U));

    b_pass = (MOTION_QUAD_NULL == motion_quad_update(NULL, 0U)) && b_pass;
    b_pass = (MOTION_QUAD_NULL ==
              motion_quad_init(NULL, MOTION_TYPES_POLARITY_NORMAL)) && b_pass;
    b_pass = (MOTION_QUAD_BAD_POLARITY ==
              motion_quad_init(&decoder, MOTION_TYPES_POLARITY_UNKNOWN)) &&
             !decoder.b_initialised && b_pass;
    b_pass = (MOTION_QUAD_OK ==
              motion_quad_init(&decoder, MOTION_TYPES_POLARITY_NORMAL)) &&
             b_pass;
    b_pass = (MOTION_QUAD_BAD_STATE == motion_quad_update(&decoder, 4U)) &&
             !decoder.b_have_state && b_pass;
    b_pass = (MOTION_QUAD_BASELINE == motion_quad_update(&decoder, 3U)) &&
             (0 == decoder.count) && b_pass;
    decoder.previous_ab = UINT8_MAX;
    b_pass = (MOTION_QUAD_BAD_STATE == motion_quad_update(&decoder, 0U)) &&
             b_pass;

    return (b_pass);
}

static bool motion_foundation_quad_limits(void)
{
    motion_quad_t decoder = {0};
    bool b_pass = (MOTION_QUAD_OK ==
        motion_quad_init(&decoder, MOTION_TYPES_POLARITY_NORMAL));

    b_pass = (MOTION_QUAD_BASELINE == motion_quad_update(&decoder, 0U)) &&
             b_pass;
    decoder.count = INT64_MAX - 1;
    b_pass = (MOTION_QUAD_OK == motion_quad_update(&decoder, 1U)) &&
             (INT64_MAX == decoder.count) && b_pass;
    b_pass = (MOTION_QUAD_COUNT_LIMIT == motion_quad_update(&decoder, 3U)) &&
             (INT64_MAX == decoder.count) && b_pass;
    b_pass = (MOTION_QUAD_COUNT_LIMIT == motion_quad_update(&decoder, 1U)) &&
             b_pass;
    b_pass = (MOTION_QUAD_OK ==
              motion_quad_init(&decoder, MOTION_TYPES_POLARITY_NORMAL)) &&
             (0 == decoder.count) && b_pass;
    b_pass = (MOTION_QUAD_BASELINE == motion_quad_update(&decoder, 0U)) &&
             b_pass;
    decoder.count = INT64_MIN + 1;
    b_pass = (MOTION_QUAD_OK == motion_quad_update(&decoder, 2U)) &&
             (INT64_MIN == decoder.count) && b_pass;
    b_pass = (MOTION_QUAD_COUNT_LIMIT == motion_quad_update(&decoder, 3U)) &&
             (INT64_MIN == decoder.count) && b_pass;

    return (b_pass);
}

static bool motion_foundation_quad_pair(void)
{
    motion_quad_t left = {0};
    motion_quad_t right = {0};
    static uint8_t const sequence[] = {0U, 1U, 3U, 2U, 0U};
    bool b_pass = (MOTION_QUAD_OK ==
        motion_quad_init(&left, MOTION_TYPES_POLARITY_NORMAL));

    b_pass = (MOTION_QUAD_OK ==
              motion_quad_init(&right, MOTION_TYPES_POLARITY_REVERSED)) &&
             b_pass;
    for (size_t index = 0U; index < sizeof(sequence); ++index)
    {
        motion_quad_result_t expected = (0U == index) ?
            MOTION_QUAD_BASELINE : MOTION_QUAD_OK;

        b_pass = (expected == motion_quad_update(&left, sequence[index])) &&
                 b_pass;
        b_pass = (expected == motion_quad_update(&right, sequence[index])) &&
                 b_pass;
    }

    return ((4 == left.count) && (-4 == right.count) && b_pass);
}

static bool motion_foundation_quad_table(void)
{
    static int8_t const expected[] =
        {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
    bool b_pass = true;

    for (size_t index = 0U; index < sizeof(expected); ++index)
    {
        motion_quad_t decoder = {0};
        /* index <= 15, so both two-bit states fit uint8_t. */
        uint8_t previous = (uint8_t)(index / 4U);
        uint8_t next = (uint8_t)(index % 4U);
        motion_quad_result_t result = MOTION_QUAD_OK;

        b_pass = (MOTION_QUAD_OK ==
            motion_quad_init(&decoder, MOTION_TYPES_POLARITY_NORMAL)) &&
            b_pass;
        b_pass = (MOTION_QUAD_BASELINE ==
                  motion_quad_update(&decoder, previous)) && b_pass;
        result = motion_quad_update(&decoder, next);
        if (previous == next)
        {
            b_pass = (MOTION_QUAD_REPEATED == result) && b_pass;
        }
        else if (0 == expected[index])
        {
            b_pass = (MOTION_QUAD_INVALID_TRANSITION == result) && b_pass;
        }
        else
        {
            b_pass = (MOTION_QUAD_OK == result) && b_pass;
        }
        b_pass = (decoder.count == expected[index]) && b_pass;
    }

    return (b_pass);
}

static bool motion_foundation_est_distance(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_measurement_t measurement = {0};
    motion_estimator_sample_t sample = {1000, 0U, 1U};
    bool b_pass = (MOTION_ESTIMATOR_OK ==
        motion_estimator_init(&estimator, &config, MOTION_ESTIMATOR_LEFT,
                              1000000U));

    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        measurement.b_position_valid && !measurement.b_speed_valid && b_pass;
    sample.count += 50;
    sample.timestamp_us = 250000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (100000 == measurement.position_um) &&
        (400 == measurement.speed_mm_s) && b_pass;
    sample.count -= 50;
    sample.timestamp_us = 500000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (0 == measurement.position_um) &&
        (-400 == measurement.speed_mm_s) && b_pass;
    sample.count -= 25;
    sample.timestamp_us = 1000000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (-50000 == measurement.position_um) &&
        (-100 == measurement.speed_mm_s) && b_pass;

    return (b_pass);
}

static bool motion_foundation_est_pair(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t left = {0};
    motion_estimator_t right = {0};
    motion_estimator_sample_t sample = {0, 0U, 0U};
    motion_estimator_measurement_t measurement = {0};
    bool b_pass = (MOTION_ESTIMATOR_OK ==
        motion_estimator_init(&left, &config, MOTION_ESTIMATOR_LEFT, 1000000U));

    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&right, &config,
        MOTION_ESTIMATOR_RIGHT, 1000000U)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&left,
        &sample, 0U, &measurement)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&right,
        &sample, 0U, &measurement)) && b_pass;
    sample.count = 100;
    sample.timestamp_us = 1000000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&left, &sample,
        sample.timestamp_us, &measurement)) &&
        (200000 == measurement.position_um) &&
        (200 == measurement.speed_mm_s) && b_pass;
    sample.count = 120;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&right, &sample,
        sample.timestamp_us, &measurement)) &&
        (210000 == measurement.position_um) &&
        (210 == measurement.speed_mm_s) && b_pass;

    return (b_pass);
}

static bool motion_foundation_est_rounding(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_measurement_t measurement = {0};
    static int64_t const counts[] = {0, 1, 2, 3, 2, 1, 0, -1, -2, -3, 0};
    static int64_t const positions[] = {0, 0, 1, 2, 1, 0, 0, 0, -1, -2, 0};
    bool b_pass = true;

    config.left.counts_per_wheel_rev = 3U;
    config.left.wheel_circumference_um = 2U;
    for (uint32_t trial = 0U; trial < MOTION_FOUNDATION_TRIALS; ++trial)
    {
        b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
            &config, MOTION_ESTIMATOR_LEFT, 1000000U)) && b_pass;
        for (size_t index = 0U;
             index < (sizeof(counts) / sizeof(counts[0])); ++index)
        {
            motion_estimator_sample_t sample =
                {counts[index], index * 100000U, 0U};
            motion_estimator_result_t expected = (0U == index) ?
                MOTION_ESTIMATOR_BASELINE : MOTION_ESTIMATOR_OK;

            b_pass = (expected == motion_estimator_update(&estimator, &sample,
                sample.timestamp_us, &measurement)) &&
                (positions[index] == measurement.position_um) && b_pass;
        }
    }

    return (b_pass);
}

static bool motion_foundation_est_walk(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {0};
    motion_estimator_measurement_t measurement = {0};
    uint32_t random_state = 17U;
    bool b_pass = true;

    config.left.counts_per_wheel_rev = 3U;
    config.left.wheel_circumference_um = 2U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator, &config,
        MOTION_ESTIMATOR_LEFT, 1000000U)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && b_pass;
    for (uint32_t index = 0U; index < MOTION_FOUNDATION_WALK_STEPS; ++index)
    {
        /* Deterministic synthetic walk; unsigned PRNG wrapping is defined. */
        random_state = random_state * 1664525U + 1013904223U;
        /* Remainder is 0..14, so conversion to signed int32 is exact. */
        int32_t delta = (int32_t)(random_state % 15U) - 7;
        /* 0..15 fits int64_t; subsequent interval arithmetic is signed. */
        int64_t interval_factor = random_state % 16U;
        int64_t elapsed = 10000 + interval_factor * 10000;
        int64_t expected_speed = (delta * 2000) / (3 * elapsed);

        sample.count += delta;
        /* Elapsed is positive and <=160000, so unsigned conversion is exact. */
        sample.timestamp_us += (uint64_t)elapsed;
        b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
            &sample, sample.timestamp_us, &measurement)) &&
            (measurement.position_um == ((sample.count * 2) / 3)) &&
            (measurement.speed_mm_s == expected_speed) && b_pass;
    }

    return (b_pass);
}

static bool motion_foundation_est_time(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {0, 100U, 0U};
    motion_estimator_measurement_t measurement = {0};
    bool b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000U));

    b_pass = (MOTION_ESTIMATOR_BAD_TIME == motion_estimator_update(&estimator,
        &sample, 99U, &measurement)) && !measurement.b_speed_valid && b_pass;
    b_pass = (MOTION_ESTIMATOR_STALE == motion_estimator_update(&estimator,
        &sample, 1101U, &measurement)) && !estimator.b_have_baseline && b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 100U, &measurement)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_BAD_TIME == motion_estimator_update(&estimator,
        &sample, 100U, &measurement)) && b_pass;
    sample.timestamp_us = 99U;
    b_pass = (MOTION_ESTIMATOR_BAD_TIME == motion_estimator_update(&estimator,
        &sample, 100U, &measurement)) && b_pass;
    sample.timestamp_us = 100U;
    b_pass = (MOTION_ESTIMATOR_RESET_REQUIRED == motion_estimator_update(
        &estimator, &sample, 100U, &measurement)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_reset(&estimator)) &&
             b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 100U, &measurement)) && b_pass;
    sample.timestamp_us = 1101U;
    b_pass = (MOTION_ESTIMATOR_STALE == motion_estimator_update(&estimator,
        &sample, 1101U, &measurement)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_reset(&estimator)) &&
             b_pass;
    sample.timestamp_us = 100U;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 100U, &measurement)) && b_pass;
    sample.timestamp_us = 1100U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, 1100U, &measurement)) && (0 == measurement.speed_mm_s) &&
        measurement.b_speed_valid && b_pass;
    b_pass = (MOTION_ESTIMATOR_STALE == motion_estimator_get(&estimator,
        2101U, &measurement)) && !measurement.b_speed_valid && b_pass;
    b_pass = (MOTION_ESTIMATOR_BAD_TIME == motion_estimator_get(&estimator,
        1099U, &measurement)) && b_pass;

    return (b_pass);
}

static bool motion_foundation_est_limits(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {INT64_MIN, 0U, 0U};
    motion_estimator_measurement_t measurement = {0};
    bool b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000000U));

    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && b_pass;
    sample.count = INT64_MIN + 1;
    sample.timestamp_us = 1000000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (2000 == measurement.position_um) && b_pass;
    sample.count = INT64_MAX;
    sample.timestamp_us += 1000000U;
    b_pass = (MOTION_ESTIMATOR_OVERFLOW == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (2000 == measurement.position_um) && !measurement.b_speed_valid &&
        (INT64_MIN + 1 == estimator.previous.count) && b_pass;

    config.left.counts_per_wheel_rev = 1U;
    config.left.wheel_circumference_um = 1000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000000U)) && b_pass;
    sample = (motion_estimator_sample_t){0, 0U, 0U};
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && b_pass;
    sample.count = (int64_t)INT32_MAX + 1; /* Widen before adding. */
    sample.timestamp_us = 1000000U;
    b_pass = (MOTION_ESTIMATOR_OVERFLOW == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;
    sample.count = INT32_MIN;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (INT32_MIN == measurement.speed_mm_s) && b_pass;

    config.left.wheel_circumference_um = 2U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000000U)) && b_pass;
    sample = (motion_estimator_sample_t){0, 0U, 0U};
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && b_pass;
    sample.count = INT64_MAX;
    sample.timestamp_us = 1000000U;
    b_pass = (MOTION_ESTIMATOR_OVERFLOW == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;
    estimator.measurement.position_um = INT64_MAX;
    sample.count = 1;
    b_pass = (MOTION_ESTIMATOR_OVERFLOW == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (INT64_MAX == measurement.position_um) && b_pass;

    return (b_pass);
}

static bool motion_foundation_est_extremes(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {0, UINT64_MAX - 1U, 0U};
    motion_estimator_measurement_t measurement = {0};
    bool b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000000U));

    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;
    sample.count = 1;
    sample.timestamp_us = UINT64_MAX;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;
    sample.timestamp_us = 0U;
    b_pass = (MOTION_ESTIMATOR_BAD_TIME == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && estimator.b_reset_required && b_pass;

    config.left.counts_per_wheel_rev = 1U;
    config.left.wheel_circumference_um = 1U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, UINT32_MAX)) && b_pass;
    sample = (motion_estimator_sample_t){0};
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && b_pass;
    sample.count = INT64_MIN;
    sample.timestamp_us = 1000000U;
    b_pass = (MOTION_ESTIMATOR_OVERFLOW == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;
    estimator.measurement.position_um = INT64_MIN;
    sample.count = -1;
    b_pass = (MOTION_ESTIMATOR_OVERFLOW == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;

    config.left.counts_per_wheel_rev = UINT32_MAX;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, UINT32_MAX)) && b_pass;
    sample = (motion_estimator_sample_t){0};
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && b_pass;
    sample.count = 1;
    sample.timestamp_us = UINT32_MAX;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (0 == measurement.speed_mm_s) && b_pass;
    sample.count = INT64_MAX;
    sample.timestamp_us += UINT32_MAX;
    b_pass = (MOTION_ESTIMATOR_OVERFLOW == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;

    return (b_pass);
}

static bool motion_foundation_est_reset(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {100, 100U, 1U};
    motion_estimator_measurement_t measurement = {0};
    bool b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000000U));

    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 100U, &measurement)) && b_pass;
    sample.count = 110;
    sample.timestamp_us = 1000100U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;
    sample.count = 0;
    sample.generation = 2U;
    sample.timestamp_us += 100U;
    b_pass = (MOTION_ESTIMATOR_RESET_REQUIRED == motion_estimator_update(
        &estimator, &sample, sample.timestamp_us, &measurement)) &&
        (20000 == measurement.position_um) && b_pass;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_reset(&estimator)) &&
        (0 == estimator.measurement.position_um) && b_pass;
    sample.timestamp_us = 0U;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && !measurement.b_speed_valid && b_pass;

    return (b_pass);
}

static bool motion_foundation_est_config(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {0};
    motion_estimator_measurement_t measurement = {0};
    bool b_pass = (MOTION_ESTIMATOR_UNAVAILABLE ==
        motion_estimator_update(&estimator, &sample, 0U, &measurement));

    b_pass = (MOTION_ESTIMATOR_NULL == motion_estimator_init(NULL, &config,
        MOTION_ESTIMATOR_LEFT, 1000U)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_BAD_CONFIG == motion_estimator_init(&estimator,
        NULL, MOTION_ESTIMATOR_LEFT, 1000U)) && b_pass;
    config.calibration = MOTION_CONFIG_CALIBRATION_UNVERIFIED;
    b_pass = (MOTION_ESTIMATOR_BAD_CONFIG == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000U)) && b_pass;
    config = motion_foundation_config();
    config.left.counts_per_wheel_rev = 0U;
    b_pass = (MOTION_ESTIMATOR_BAD_CONFIG == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000U)) && b_pass;
    config = motion_foundation_config();
    b_pass = (MOTION_ESTIMATOR_BAD_CONFIG == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 0U)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_NULL == motion_estimator_update(NULL, &sample,
        0U, &measurement)) && !measurement.b_position_valid && b_pass;
    b_pass = (MOTION_ESTIMATOR_NULL == motion_estimator_update(&estimator,
        NULL, 0U, &measurement)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_NULL == motion_estimator_update(&estimator,
        &sample, 0U, NULL)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_NULL == motion_estimator_get(NULL, 0U,
        &measurement)) && b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_response(void)
{
    motion_pid_config_t config = motion_foundation_pid_cfg();
    motion_pid_t controller = {0};
    motion_pid_value_t effort = 0.0F;
    bool b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config));

    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 100,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.0F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.1F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, -100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, -0.1F) &&
        b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_integral(void)
{
    motion_pid_config_t config = motion_foundation_pid_cfg();
    motion_pid_t controller = {0};
    motion_pid_value_t effort = 0.0F;
    bool b_pass = true;

    config.proportional_gain = 0.0F;
    config.integral_gain = 0.001F;
    config.integral_min = -0.02F;
    config.integral_max = 0.02F;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.01F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
        200000U, true, &effort)) && motion_foundation_near(effort, 0.02F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, -100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, -0.01F) &&
        b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_windup(void)
{
    motion_pid_config_t config = motion_foundation_pid_cfg();
    motion_pid_t controller = {0};
    motion_pid_value_t effort = 0.0F;
    bool b_pass = true;

    config.proportional_gain = 0.1F;
    config.integral_gain = 0.01F;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) && b_pass;
    for (uint32_t trial = 0U;
         trial < MOTION_FOUNDATION_SATURATION_STEPS; ++trial)
    {
        b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
            100000U, true, &effort)) && motion_foundation_near(effort, 1.0F) &&
            motion_foundation_near(controller.integral, 0.0F) && b_pass;
    }
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 99,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.101F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, -100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, -1.0F) &&
        motion_foundation_near(controller.integral, 0.0F) && b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_deriv(void)
{
    motion_pid_config_t config = motion_foundation_pid_cfg();
    motion_pid_t controller = {0};
    motion_pid_value_t effort = 0.0F;
    bool b_pass = true;

    config.proportional_gain = 0.0F;
    config.derivative_gain = 0.01F;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.0F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 200, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.0F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 200, 10,
        100000U, true, &effort)) && motion_foundation_near(effort, -1.0F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_reset(&controller)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 200, 10,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.0F) &&
        b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_reset(void)
{
    motion_pid_config_t config = motion_foundation_pid_cfg();
    motion_pid_t controller = {0};
    motion_pid_value_t effort = 1.0F;
    bool b_pass = (MOTION_PID_UNINITIALISED == motion_pid_update(&controller,
        100, 0, 100000U, true, &effort)) &&
        motion_foundation_near(effort, 0.0F);

    config.integral_gain = 0.01F;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && b_pass;
    b_pass = (MOTION_PID_IDLE == motion_pid_update(&controller, 0, 100,
        0U, true, &effort)) && motion_foundation_near(effort, 0.0F) &&
        motion_foundation_near(controller.integral, 0.0F) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && b_pass;
    b_pass = (MOTION_PID_IDLE == motion_pid_update(&controller, 100, 0,
        0U, false, &effort)) && !controller.b_have_measurement && b_pass;
    b_pass = (MOTION_PID_BAD_TIME == motion_pid_update(&controller, 100, 0,
        0U, true, &effort)) && motion_foundation_near(effort, 0.0F) && b_pass;
    b_pass = (MOTION_PID_BAD_TIME == motion_pid_update(&controller, 100, 0,
        config.max_elapsed_us + 1U, true, &effort)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_reset(&controller)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.2F) &&
        b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller,
        &controller.config)) && !controller.b_have_measurement &&
        motion_foundation_near(controller.integral, 0.0F) && b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_config(void)
{
    motion_pid_config_t config = motion_foundation_pid_cfg();
    motion_pid_t controller = {0};
    motion_pid_value_t effort = 1.0F;
    bool b_pass = (MOTION_PID_NULL == motion_pid_validate(NULL));
    static float const invalid_gains[] = {-1.0F, NAN, INFINITY};

    for (size_t index = 0U;
         index < (sizeof(invalid_gains) / sizeof(invalid_gains[0])); ++index)
    {
        config = motion_foundation_pid_cfg();
        config.proportional_gain = invalid_gains[index];
        b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) &&
                 b_pass;
        config = motion_foundation_pid_cfg();
        config.integral_gain = invalid_gains[index];
        b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) &&
                 b_pass;
        config = motion_foundation_pid_cfg();
        config.derivative_gain = invalid_gains[index];
        b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) &&
                 b_pass;
    }
    config = motion_foundation_pid_cfg();
    config.output_min = 0.1F;
    b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) && b_pass;
    config = motion_foundation_pid_cfg();
    config.output_max = 1.1F;
    b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) && b_pass;
    config = motion_foundation_pid_cfg();
    config.integral_min = 0.1F;
    b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) && b_pass;
    config = motion_foundation_pid_cfg();
    config.max_elapsed_us = 0U;
    b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_init(&controller, &config)) &&
             !controller.b_initialised && b_pass;
    config = motion_foundation_pid_cfg();
    config.calibration = MOTION_CONFIG_CALIBRATION_UNVERIFIED;
    b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) && b_pass;
    b_pass = (MOTION_PID_NULL == motion_pid_update(NULL, 100, 0, 100U, true,
        &effort)) && motion_foundation_near(effort, 0.0F) && b_pass;
    b_pass = (MOTION_PID_NULL == motion_pid_update(&controller, 100, 0, 100U,
        true, NULL)) && b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_limits(void)
{
    motion_pid_config_t config = motion_foundation_pid_cfg();
    motion_pid_t controller = {0};
    motion_pid_value_t effort = 1.0F;
    bool b_pass = true;

    config.proportional_gain = FLT_MAX;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) && b_pass;
    b_pass = (MOTION_PID_ARITHMETIC == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.0F) &&
        !controller.b_initialised && b_pass;
    config = motion_foundation_pid_cfg();
    config.proportional_gain = 0.0F;
    config.integral_gain = FLT_MAX;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) && b_pass;
    b_pass = (MOTION_PID_ARITHMETIC == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.0F) &&
        b_pass;
    config = motion_foundation_pid_cfg();
    config.proportional_gain = 0.0F;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, INT32_MIN,
        INT32_MAX, 1U, true, &effort)) &&
        motion_foundation_near(effort, 0.0F) && b_pass;
    controller.integral = NAN;
    b_pass = (MOTION_PID_ARITHMETIC == motion_pid_update(&controller, 100, 0,
        100U, true, &effort)) && motion_foundation_near(effort, 0.0F) && b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_pair(void)
{
    motion_pid_config_t config = motion_foundation_pid_cfg();
    motion_pid_t left = {0};
    motion_pid_t right = {0};
    motion_pid_value_t effort = 0.0F;
    bool b_pass = true;

    config.proportional_gain = 0.0F;
    config.integral_gain = 0.001F;
    b_pass = (MOTION_PID_OK == motion_pid_init(&left, &config)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_init(&right, &config)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&left, 100, 0, 100000U,
        true, &effort)) && motion_foundation_near(effort, 0.01F) &&
        !right.b_have_measurement && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&right, -100, 0, 100000U,
        true, &effort)) && motion_foundation_near(effort, -0.01F) &&
        motion_foundation_near(left.integral, 0.01F) && b_pass;

    return (b_pass);
}

static bool motion_foundation_snapshots(void)
{
    motion_config_t config = motion_foundation_config();
    motion_pid_config_t gains = motion_foundation_pid_cfg();
    motion_pid_t controller = {0};
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {0};
    motion_estimator_measurement_t measurement = {0};
    motion_pid_value_t effort = 0.0F;
    bool b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &gains));

    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator, &config,
        MOTION_ESTIMATOR_LEFT, 1000000U)) && b_pass;
    gains.proportional_gain = NAN;
    config.left.wheel_circumference_um = 0U;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, 100, 0,
        100000U, true, &effort)) && motion_foundation_near(effort, 0.1F) &&
        b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && b_pass;
    sample.count = 100;
    sample.timestamp_us = 1000000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (200000 == measurement.position_um) && b_pass;

    return (b_pass);
}

static bool motion_foundation_pipeline(void)
{
    motion_config_t config = motion_foundation_config();
    motion_pid_config_t gains = motion_foundation_pid_cfg();
    motion_quad_t decoder = {0};
    motion_estimator_t estimator = {0};
    motion_pid_t controller = {0};
    motion_estimator_sample_t sample = {0};
    motion_estimator_measurement_t measurement = {0};
    motion_pid_value_t effort = 0.0F;
    static uint8_t const sequence[] = {1U, 3U, 2U, 0U};
    bool b_pass = (MOTION_QUAD_OK ==
        motion_quad_init(&decoder, config.right.encoder_polarity));

    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator, &config,
        MOTION_ESTIMATOR_RIGHT, 1000000U)) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &gains)) && b_pass;
    b_pass = (MOTION_QUAD_BASELINE == motion_quad_update(&decoder, 0U)) &&
             b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 0U, &measurement)) && b_pass;
    for (size_t index = 0U; index < sizeof(sequence); ++index)
    {
        b_pass = (MOTION_QUAD_OK ==
                  motion_quad_update(&decoder, sequence[index])) && b_pass;
    }
    sample.count = decoder.count;
    sample.timestamp_us = 1000000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (-7000 == measurement.position_um) &&
        (-7 == measurement.speed_mm_s) && b_pass;
    b_pass = (MOTION_PID_OK == motion_pid_update(&controller, -107,
        measurement.speed_mm_s, 100000U, true, &effort)) &&
        motion_foundation_near(effort, -0.1F) && b_pass;

    return (b_pass);
}


static bool motion_foundation_pid_crossing(void)
{
    bool b_pass = true;

    for (int32_t direction = -1; direction <= 1; direction += 2)
    {
        motion_pid_config_t config = motion_foundation_pid_cfg();
        motion_pid_t controller = {0};
        motion_pid_value_t effort = 0.0F;
        float sign = (float)direction;

        config.proportional_gain = 0.0F;
        config.integral_gain = 1.0F;
        config.integral_min = -10.0F;
        config.integral_max = 10.0F;
        b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) &&
                 b_pass;
        for (uint32_t index = 0U; index < 10U; ++index)
        {
            b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
                direction * 100, 0, 100000U, true, &effort)) &&
                motion_foundation_near(effort, sign) &&
                motion_foundation_near(controller.integral, sign) && b_pass;
        }
        /* Same target sign: error opposes the saturated integral. */
        b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
            direction * 100, direction * 105, 100000U, true, &effort)) &&
            motion_foundation_near(effort, sign * 0.5F) && b_pass;
        b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
            direction * 100, direction * 105, 100000U, true, &effort)) &&
            motion_foundation_near(effort, 0.0F) && b_pass;
    }

    return (b_pass);
}

static bool motion_foundation_pid_unwind(void)
{
    bool b_pass = true;

    for (int32_t direction = -1; direction <= 1; direction += 2)
    {
        motion_pid_config_t config = motion_foundation_pid_cfg();
        motion_pid_t controller = {0};
        motion_pid_value_t effort = 0.0F;
        float sign = (float)direction;

        config.proportional_gain = 0.0F;
        config.integral_gain = 1.0F;
        config.integral_min = -10.0F;
        config.integral_max = 10.0F;
        b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) &&
                 b_pass;
        b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
            direction * 5, 0, 100000U, true, &effort)) &&
            motion_foundation_near(effort, sign * 0.5F) && b_pass;
        for (uint32_t index = 0U; index < 10U; ++index)
        {
            b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
                direction * 5, direction * 105, 100000U, true, &effort)) &&
                motion_foundation_near(effort, -sign) &&
                motion_foundation_near(controller.integral, -sign) && b_pass;
        }
        b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
            direction * 5, 0, 100000U, true, &effort)) &&
            motion_foundation_near(effort, sign * -0.5F) && b_pass;
        b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
            direction * 5, 0, 100000U, true, &effort)) &&
            motion_foundation_near(effort, 0.0F) && b_pass;
    }

    return (b_pass);
}

static bool motion_foundation_est_null_cache(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {100, 1000U, 7U};
    motion_estimator_measurement_t measurement = {0};
    bool b_pass = true;

    config.left.counts_per_wheel_rev = 3U;
    config.left.wheel_circumference_um = 1000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 1000000U)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) && b_pass;
    for (uint32_t index = 0U; index < 2U; ++index)
    {
        sample.count += 1;
        sample.timestamp_us += 200000U;
        b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
            &sample, sample.timestamp_us, &measurement)) &&
            measurement.b_speed_valid && (1 == measurement.speed_mm_s) &&
            b_pass;
        motion_estimator_t before = estimator;
        motion_estimator_result_t result = (0U == index) ?
            motion_estimator_update(&estimator, NULL, sample.timestamp_us,
                                    &measurement) :
            motion_estimator_update(&estimator, &sample, sample.timestamp_us,
                                    NULL);

        b_pass = (MOTION_ESTIMATOR_NULL == result) &&
            (MOTION_ESTIMATOR_NULL == estimator.result) &&
            !estimator.measurement.b_speed_valid &&
            (0 == estimator.measurement.speed_mm_s) &&
            (before.measurement.position_um ==
             estimator.measurement.position_um) &&
            (before.measurement.timestamp_us ==
             estimator.measurement.timestamp_us) &&
            (before.measurement.b_position_valid ==
             estimator.measurement.b_position_valid) &&
            (before.previous.count == estimator.previous.count) &&
            (before.previous.timestamp_us == estimator.previous.timestamp_us) &&
            (before.previous.generation == estimator.previous.generation) &&
            (before.remainder == estimator.remainder) &&
            estimator.b_have_baseline && !estimator.b_reset_required && b_pass;
        b_pass = (MOTION_ESTIMATOR_NULL == motion_estimator_get(&estimator,
            sample.timestamp_us, &measurement)) &&
            measurement.b_position_valid && !measurement.b_speed_valid &&
            (before.measurement.position_um == measurement.position_um) &&
            (0 == measurement.speed_mm_s) && b_pass;
    }
    /* Recover from the last ACCEPTED count/time, including its remainder. */
    sample.count += 1;
    sample.timestamp_us += 200000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (1000 == measurement.position_um) && (1 == measurement.speed_mm_s) &&
        measurement.b_position_valid && measurement.b_speed_valid &&
        (sample.timestamp_us == measurement.timestamp_us) && b_pass;
    b_pass = (MOTION_ESTIMATOR_BAD_TIME == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        !measurement.b_speed_valid && !estimator.b_reset_required && b_pass;
    b_pass = (MOTION_ESTIMATOR_NULL == motion_estimator_update(NULL,
        &sample, sample.timestamp_us, &measurement)) &&
        !measurement.b_position_valid && !measurement.b_speed_valid && b_pass;

    return (b_pass);
}

static bool motion_foundation_est_speed_edges(void)
{
    motion_config_t config = motion_foundation_config();
    motion_estimator_t estimator = {0};
    motion_estimator_sample_t sample = {0, 1000U, 0U};
    motion_estimator_measurement_t measurement = {0};
    bool b_pass = true;

    config.left.counts_per_wheel_rev = 3U;
    config.left.wheel_circumference_um = 1000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_init(&estimator,
        &config, MOTION_ESTIMATOR_LEFT, 200000U)) && b_pass;
    b_pass = (MOTION_ESTIMATOR_BASELINE == motion_estimator_update(&estimator,
        &sample, 1000U, &measurement)) && measurement.b_position_valid &&
        !measurement.b_speed_valid && (1000U == measurement.timestamp_us) &&
        b_pass;
    sample.count = 1;
    sample.timestamp_us += 200000U;
    /* 1000/3 um in 0.2 s = +1.666... mm/s, truncated to +1.
     * Both interval and sample age equal the permitted maximum here.
     */
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us + 200000U, &measurement)) &&
        (333 == measurement.position_um) && (1 == measurement.speed_mm_s) &&
        measurement.b_position_valid && measurement.b_speed_valid &&
        (sample.timestamp_us == measurement.timestamp_us) && b_pass;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_get(&estimator,
        sample.timestamp_us + 200000U, &measurement)) &&
        measurement.b_speed_valid && b_pass;
    b_pass = (MOTION_ESTIMATOR_STALE == motion_estimator_get(&estimator,
        sample.timestamp_us + 200001U, &measurement)) &&
        !measurement.b_speed_valid && measurement.b_position_valid &&
        (333 == measurement.position_um) && (0 == measurement.speed_mm_s) &&
        (sample.timestamp_us == measurement.timestamp_us) && b_pass;
    sample.count = 0;
    sample.timestamp_us += 200000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (0 == measurement.position_um) && (-1 == measurement.speed_mm_s) &&
        measurement.b_position_valid && measurement.b_speed_valid && b_pass;
    sample.count = -1;
    sample.timestamp_us += 100000U;
    /* -3.333... mm/s truncates to -3, not -4. */
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (-333 == measurement.position_um) && (-3 == measurement.speed_mm_s) &&
        measurement.b_speed_valid && b_pass;
    sample.count = 0;
    sample.timestamp_us += 100000U;
    b_pass = (MOTION_ESTIMATOR_OK == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        (0 == measurement.position_um) && (3 == measurement.speed_mm_s) &&
        measurement.b_speed_valid && b_pass;
    sample.timestamp_us += 200001U;
    b_pass = (MOTION_ESTIMATOR_STALE == motion_estimator_update(&estimator,
        &sample, sample.timestamp_us, &measurement)) &&
        !measurement.b_speed_valid && estimator.b_reset_required && b_pass;
    b_pass = (MOTION_ESTIMATOR_RESET_REQUIRED == motion_estimator_update(
        &estimator, &sample, sample.timestamp_us, &measurement)) && b_pass;

    return (b_pass);
}

static bool motion_foundation_pid_bad_bounds(void)
{
    static float const invalid[] = {NAN, INFINITY, -INFINITY};
    bool b_pass = true;

    for (size_t index = 0U; index < 3U; ++index)
    {
        for (uint32_t field = 0U; field < 4U; ++field)
        {
            motion_pid_config_t config = motion_foundation_pid_cfg();
            motion_pid_t controller = {0};
            motion_pid_value_t effort = 1.0F;

            switch (field)
            {
                case 0U:
                    config.output_min = invalid[index];
                break;
                case 1U:
                    config.output_max = invalid[index];
                break;
                case 2U:
                    config.integral_min = invalid[index];
                break;
                default:
                    config.integral_max = invalid[index];
                break;
            }
            b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) &&
                (MOTION_PID_BAD_CONFIG == motion_pid_init(&controller,
                 &config)) && !controller.b_initialised && b_pass;
            b_pass = (MOTION_PID_UNINITIALISED == motion_pid_update(&controller,
                100, 0, 100000U, true, &effort)) &&
                motion_foundation_near(effort, 0.0F) && b_pass;
        }
    }
    for (uint32_t field = 0U; field < 7U; ++field)
    {
        motion_pid_config_t config = motion_foundation_pid_cfg();

        switch (field)
        {
            case 0U:
                config.output_min = -1.1F;
            break;
            case 1U:
                config.output_min = 0.1F;
            break;
            case 2U:
                config.output_max = -0.1F;
            break;
            case 3U:
                config.output_max = 1.1F;
            break;
            case 4U:
                config.output_min = 0.0F;
                config.output_max = 0.0F;
            break;
            case 5U:
                config.integral_min = 0.1F;
            break;
            default:
                config.integral_max = -0.1F;
            break;
        }
        b_pass = (MOTION_PID_BAD_CONFIG == motion_pid_validate(&config)) &&
                 b_pass;
    }

    return (b_pass);
}

static bool motion_foundation_pid_nonfinite(void)
{
    bool b_pass = true;

    for (uint32_t trial = 0U; trial < 3U; ++trial)
    {
        motion_pid_config_t config = motion_foundation_pid_cfg();
        motion_pid_t controller = {0};
        motion_pid_value_t effort = 1.0F;

        config.proportional_gain = 0.0F;
        config.integral_min = -FLT_MAX;
        config.integral_max = FLT_MAX;
        if (0U == trial)
        {
            config.derivative_gain = FLT_MAX;
        }
        else if (1U == trial)
        {
            config.integral_gain = FLT_MAX;
        }
        else
        {
            config.proportional_gain = FLT_MAX;
        }
        b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) &&
                 b_pass;
        if (0U == trial)
        {
            b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
                1, 0, 1U, true, &effort)) && b_pass;
        }
        else
        {
            /* Test injection of valid finite state to overflow the next sum. */
            controller.integral = FLT_MAX;
        }
        b_pass = (MOTION_PID_ARITHMETIC == motion_pid_update(&controller,
            1, (0U == trial) ? 10 : 0, 1000000U, true, &effort)) &&
            !controller.b_initialised && !controller.b_have_measurement &&
            motion_foundation_near(effort, 0.0F) &&
            motion_foundation_near(controller.integral, 0.0F) && b_pass;
        b_pass = (MOTION_PID_UNINITIALISED == motion_pid_update(&controller,
            1, 0, 100000U, true, &effort)) &&
            motion_foundation_near(effort, 0.0F) && b_pass;
    }

    return (b_pass);
}


static bool motion_foundation_pid_headroom(void)
{
    bool b_pass = true;

    for (uint32_t trial = 0U; trial < 4U; ++trial)
    {
        motion_pid_config_t config = motion_foundation_pid_cfg();
        motion_pid_t controller = {0};
        motion_pid_value_t effort = 0.0F;
        static float const expected_effort[] = {0.75F, -0.25F, 0.45F, 0.75F};
        static float const expected_integral[] = {0.5F, 0.0F, 0.2F, 0.5F};
        int32_t target = (1U == trial) ? -100 : 100;
        int32_t measured = (3U == trial) ? 95 : ((1U == trial) ? -5 : 5);
        int32_t previous = (3U == trial) ? measured :
            measured + ((1U == trial) ? -5 : 5);

        config.proportional_gain = (3U == trial) ? 0.05F : 0.0F;
        config.derivative_gain = 0.01F;
        config.integral_gain = 1.0F;
        config.output_min = -0.25F;
        config.output_max = 0.75F;
        config.integral_min = -10.0F;
        config.integral_max = (2U == trial) ? 0.2F : 10.0F;
        b_pass = (MOTION_PID_OK == motion_pid_init(&controller, &config)) &&
                 b_pass;
        /* Establish D history with zero error; no integral is accumulated. */
        b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
            previous, previous, 1000000U, true, &effort)) && b_pass;
        /* D is +0.25/-0.25; final case has P=0.25 and D=0. */
        b_pass = (MOTION_PID_OK == motion_pid_update(&controller,
            target, measured, 200000U, true, &effort)) &&
            motion_foundation_near(effort, expected_effort[trial]) &&
            motion_foundation_near(controller.integral,
                                   expected_integral[trial]) && b_pass;
    }

    return (b_pass);
}

#endif /* MOTION_CONFIG_HOST_TEST || APP_TEST */

/*** end of file ***/

