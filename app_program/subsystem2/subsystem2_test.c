/*
 *	subsystem2_test.c
 *	On-target tests for subsystem 2. Built with: make TEST=1
 *  MOTION_CONFIG_HOST_TEST selects only the pure M1A host test runner.
 *	Record results in testing/subsystem2/TEST_LOG.md.
 */

#if defined(MOTION_CONFIG_HOST_TEST)
#include <stdio.h>

/* Host-only adapter: keep the template's report shape without RTOS stubs. */
#define TEST_INIT() uint32_t test_pass = 0U; uint32_t test_fail = 0U
#define TEST_CHECK(id, condition, description) \
    do \
    { \
        bool b_pass = (condition); \
        test_pass += b_pass ? 1U : 0U; \
        test_fail += b_pass ? 0U : 1U; \
        printf("[TEST] %s %s  %s\n", (id), \
               b_pass ? "PASS" : "FAIL", (description)); \
    } while (0)
#define TEST_SUMMARY(name) \
    printf("[TEST] %s SUMMARY %u passed, %u failed\n", \
           (name), (unsigned int)test_pass, (unsigned int)test_fail)
#else
/* This translation unit uses standard integer headers. Keep the compiler's
 * size_t instead of the port's optional typedef, as the S3 adapter does.
 * This is local to the test source; no shared configuration is changed.
 */
#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include "subsystem2.h"
#include "../test_report.h"
#endif

#include "motion_config.h"
#include "motion_foundation_test.h"

#if defined(MOTION_CONFIG_HOST_TEST) || APP_TEST

/* Synthetic fixtures ONLY; these are not robot calibration defaults.
 * The test IDs below are reserved here pending approval to update the
 * out-of-scope testing/subsystem2/TEST_LOG.md. No hardware is actuated.
 */
static motion_config_t motion_config_test_fixture(void)
{
    motion_config_t config = {0};

    config.left.counts_per_wheel_rev = 100U;
    config.left.wheel_circumference_um = 200000U;
    config.left.motor_polarity = MOTION_TYPES_POLARITY_NORMAL;
    config.left.encoder_polarity = MOTION_TYPES_POLARITY_REVERSED;
    config.right.counts_per_wheel_rev = 120U;
    config.right.wheel_circumference_um = 210000U;
    config.right.motor_polarity = MOTION_TYPES_POLARITY_REVERSED;
    config.right.encoder_polarity = MOTION_TYPES_POLARITY_NORMAL;
    config.track_width_um = 150000U;
    config.max_wheel_speed_mm_s = 250;
    config.calibration = MOTION_CONFIG_CALIBRATION_CONFIRMED;

    return (config);
}

static bool motion_config_test_reset(void)
{
    motion_config_t config = motion_config_test_fixture();
    bool b_reset = motion_config_init(&config);

    return (b_reset &&
            (MOTION_CONFIG_UNCONFIGURED == motion_config_validate(&config)) &&
            (0U == config.left.counts_per_wheel_rev) &&
            (0U == config.left.wheel_circumference_um) &&
            (MOTION_TYPES_POLARITY_UNKNOWN == config.left.motor_polarity) &&
            (MOTION_TYPES_POLARITY_UNKNOWN == config.left.encoder_polarity) &&
            (0U == config.right.counts_per_wheel_rev) &&
            (0U == config.right.wheel_circumference_um) &&
            (MOTION_TYPES_POLARITY_UNKNOWN == config.right.motor_polarity) &&
            (MOTION_TYPES_POLARITY_UNKNOWN == config.right.encoder_polarity) &&
            (0U == config.track_width_um) &&
            (0 == config.max_wheel_speed_mm_s) &&
            (MOTION_CONFIG_CALIBRATION_UNCONFIGURED == config.calibration));
}

static bool motion_config_test_wheels(void)
{
    motion_config_t config = motion_config_test_fixture();
    bool b_pass = true;

    config.left.counts_per_wheel_rev = 0U;
    b_pass = b_pass && (MOTION_CONFIG_BAD_LEFT_WHEEL ==
                       motion_config_validate(&config));
    config = motion_config_test_fixture();
    config.left.wheel_circumference_um = 0U;
    b_pass = b_pass && (MOTION_CONFIG_BAD_LEFT_WHEEL ==
                       motion_config_validate(&config));
    config = motion_config_test_fixture();
    config.right.counts_per_wheel_rev = 0U;
    b_pass = b_pass && (MOTION_CONFIG_BAD_RIGHT_WHEEL ==
                       motion_config_validate(&config));
    config = motion_config_test_fixture();
    config.right.wheel_circumference_um = 0U;
    b_pass = b_pass && (MOTION_CONFIG_BAD_RIGHT_WHEEL ==
                       motion_config_validate(&config));

    return (b_pass);
}

static bool motion_config_test_polarity(void)
{
    motion_config_t config = motion_config_test_fixture();
    bool b_pass = true;

    config.left.motor_polarity = MOTION_TYPES_POLARITY_UNKNOWN;
    b_pass = b_pass && (MOTION_CONFIG_BAD_LEFT_WHEEL ==
                       motion_config_validate(&config));
    config = motion_config_test_fixture();
    config.left.encoder_polarity = (motion_types_polarity_t)99;
    b_pass = b_pass && (MOTION_CONFIG_BAD_LEFT_WHEEL ==
                       motion_config_validate(&config));
    config = motion_config_test_fixture();
    config.right.motor_polarity = (motion_types_polarity_t)99;
    b_pass = b_pass && (MOTION_CONFIG_BAD_RIGHT_WHEEL ==
                       motion_config_validate(&config));
    config = motion_config_test_fixture();
    config.right.encoder_polarity = MOTION_TYPES_POLARITY_UNKNOWN;
    b_pass = b_pass && (MOTION_CONFIG_BAD_RIGHT_WHEEL ==
                       motion_config_validate(&config));

    return (b_pass);
}

static bool motion_config_test_limits(void)
{
    motion_config_t config = motion_config_test_fixture();
    bool b_pass = true;

    config.track_width_um = 0U;
    b_pass = b_pass && (MOTION_CONFIG_BAD_TRACK_WIDTH ==
                       motion_config_validate(&config));
    config = motion_config_test_fixture();
    config.max_wheel_speed_mm_s = 0;
    b_pass = b_pass && (MOTION_CONFIG_BAD_SPEED_LIMIT ==
                       motion_config_validate(&config));
    config.max_wheel_speed_mm_s = -1;
    b_pass = b_pass && (MOTION_CONFIG_BAD_SPEED_LIMIT ==
                       motion_config_validate(&config));
    config.max_wheel_speed_mm_s = INT32_MIN;
    b_pass = b_pass && (MOTION_CONFIG_BAD_SPEED_LIMIT ==
                       motion_config_validate(&config));

    return (b_pass);
}

static bool motion_config_test_evidence(void)
{
    motion_config_t config = motion_config_test_fixture();
    bool b_pass = true;

    config.calibration = MOTION_CONFIG_CALIBRATION_UNVERIFIED;
    b_pass = b_pass && (MOTION_CONFIG_UNVERIFIED ==
                       motion_config_validate(&config));
    config.calibration = (motion_config_calibration_t)99;
    b_pass = b_pass && (MOTION_CONFIG_BAD_CALIBRATION ==
                       motion_config_validate(&config));
    config.calibration = MOTION_CONFIG_CALIBRATION_UNCONFIGURED;
    b_pass = b_pass && (MOTION_CONFIG_UNCONFIGURED ==
                       motion_config_validate(&config));
    config.calibration = MOTION_CONFIG_CALIBRATION_CONFIRMED;
    b_pass = b_pass && (MOTION_CONFIG_VALID ==
                       motion_config_validate(&config));

    return (b_pass);
}

static bool motion_config_test_bounds(void)
{
    motion_config_t config = motion_config_test_fixture();
    bool b_pass = true;

    config.left.counts_per_wheel_rev = 1U;
    config.right.counts_per_wheel_rev = UINT32_MAX;
    config.left.wheel_circumference_um = UINT32_MAX;
    config.right.wheel_circumference_um = 1U;
    config.track_width_um = UINT32_MAX;
    config.max_wheel_speed_mm_s = INT32_MAX;
    b_pass = (MOTION_CONFIG_VALID == motion_config_validate(&config));
    config.track_width_um = 1U;
    config.max_wheel_speed_mm_s = 1;
    b_pass = b_pass && (MOTION_CONFIG_VALID ==
                       motion_config_validate(&config));

    return (b_pass);
}

/* Compare fields rather than padding bytes, whose values need not be stable
 * across structure copies. All configuration fields are checked explicitly.
 */
static bool motion_config_test_equal(motion_config_t const * p_before,
                                     motion_config_t const * p_after)
{
    return ((p_before->left.counts_per_wheel_rev ==
             p_after->left.counts_per_wheel_rev) &&
            (p_before->left.wheel_circumference_um ==
             p_after->left.wheel_circumference_um) &&
            (p_before->left.motor_polarity == p_after->left.motor_polarity) &&
            (p_before->left.encoder_polarity ==
             p_after->left.encoder_polarity) &&
            (p_before->right.counts_per_wheel_rev ==
             p_after->right.counts_per_wheel_rev) &&
            (p_before->right.wheel_circumference_um ==
             p_after->right.wheel_circumference_um) &&
            (p_before->right.motor_polarity == p_after->right.motor_polarity) &&
            (p_before->right.encoder_polarity ==
             p_after->right.encoder_polarity) &&
            (p_before->track_width_um == p_after->track_width_um) &&
            (p_before->max_wheel_speed_mm_s == p_after->max_wheel_speed_mm_s) &&
            (p_before->calibration == p_after->calibration));
}

static bool motion_config_test_unchanged(void)
{
    static motion_config_result_t const expected[] =
    {
        MOTION_CONFIG_VALID,
        MOTION_CONFIG_UNCONFIGURED,
        MOTION_CONFIG_BAD_CALIBRATION,
        MOTION_CONFIG_BAD_LEFT_WHEEL,
        MOTION_CONFIG_BAD_RIGHT_WHEEL,
        MOTION_CONFIG_BAD_TRACK_WIDTH,
        MOTION_CONFIG_BAD_SPEED_LIMIT,
        MOTION_CONFIG_UNVERIFIED
    };
    bool b_pass = true;

    for (size_t index = 0U;
         index < (sizeof(expected) / sizeof(expected[0])); ++index)
    {
        motion_config_t config = motion_config_test_fixture();

        switch (expected[index])
        {
            case MOTION_CONFIG_VALID:
                /* The complete synthetic fixture already selects this. */
            break;
            case MOTION_CONFIG_UNCONFIGURED:
                config = (motion_config_t){0};
            break;
            case MOTION_CONFIG_BAD_CALIBRATION:
                /* 3 fits the enum's integer type but is not an enumerator. */
                config.calibration = (motion_config_calibration_t)3;
            break;
            case MOTION_CONFIG_BAD_LEFT_WHEEL:
                config.left.counts_per_wheel_rev = 0U;
            break;
            case MOTION_CONFIG_BAD_RIGHT_WHEEL:
                config.right.wheel_circumference_um = 0U;
            break;
            case MOTION_CONFIG_BAD_TRACK_WIDTH:
                config.track_width_um = 0U;
            break;
            case MOTION_CONFIG_BAD_SPEED_LIMIT:
                config.max_wheel_speed_mm_s = -1;
            break;
            case MOTION_CONFIG_UNVERIFIED:
                config.calibration = MOTION_CONFIG_CALIBRATION_UNVERIFIED;
            break;
            default:
                b_pass = false;
            break;
        }

        motion_config_t before = config;
        motion_config_result_t result = motion_config_validate(&config);
        bool b_unchanged = motion_config_test_equal(&before, &config);

        b_pass = (b_unchanged && (expected[index] == result)) && b_pass;
    }

    return (b_pass);
}

static bool motion_config_test_enums(void)
{
    static int32_t const invalid[] = {-1, 3, 127, INT32_MAX};
    bool b_pass = true;

    for (size_t index = 0U;
         index < (sizeof(invalid) / sizeof(invalid[0])); ++index)
    {
        motion_config_t config = motion_config_test_fixture();
        /* Inject non-enumerator values intentionally. Negative/out-of-range
         * conversion can depend on the enum's compatible integer type.
         * Only rejection is asserted, not a signed representation or enum
         * width. No invalid enum value indexes an array.
         */
        motion_types_polarity_t polarity =
            (motion_types_polarity_t)invalid[index];
        motion_config_calibration_t calibration =
            (motion_config_calibration_t)invalid[index];

        config.left.motor_polarity = polarity;
        b_pass = (MOTION_CONFIG_BAD_LEFT_WHEEL ==
                  motion_config_validate(&config)) && b_pass;
        config = motion_config_test_fixture();
        config.left.encoder_polarity = polarity;
        b_pass = (MOTION_CONFIG_BAD_LEFT_WHEEL ==
                  motion_config_validate(&config)) && b_pass;
        config = motion_config_test_fixture();
        config.right.motor_polarity = polarity;
        b_pass = (MOTION_CONFIG_BAD_RIGHT_WHEEL ==
                  motion_config_validate(&config)) && b_pass;
        config = motion_config_test_fixture();
        config.right.encoder_polarity = polarity;
        b_pass = (MOTION_CONFIG_BAD_RIGHT_WHEEL ==
                  motion_config_validate(&config)) && b_pass;
        config = motion_config_test_fixture();
        config.calibration = calibration;
        b_pass = (MOTION_CONFIG_BAD_CALIBRATION ==
                  motion_config_validate(&config)) && b_pass;
    }

    return (b_pass);
}

static bool motion_config_tests(void)
{
    motion_config_t zero = {0};
    motion_config_t config = motion_config_test_fixture();
    TEST_INIT();

    TEST_CHECK("S2-U03", motion_config_test_reset() &&
               (MOTION_CONFIG_UNCONFIGURED == motion_config_validate(&zero)),
               "reset and zero configuration remain unconfigured");
    TEST_CHECK("S2-U04", !motion_config_init(NULL) &&
               (MOTION_CONFIG_NULL == motion_config_validate(NULL)),
               "null arguments are rejected");
    TEST_CHECK("S2-U05", motion_config_test_wheels(),
               "both wheels require nonzero resolution and circumference");
    TEST_CHECK("S2-U06", motion_config_test_polarity(),
               "unknown and invalid polarities are rejected");
    TEST_CHECK("S2-U07", motion_config_test_limits(),
               "zero track and nonpositive speed limits are rejected");
    TEST_CHECK("S2-U08", motion_config_test_evidence(),
               "calibration evidence and enum states are checked");
    TEST_CHECK("S2-U09", MOTION_CONFIG_VALID ==
               motion_config_validate(&config),
               "asymmetric wheels and independent polarities are supported");
    TEST_CHECK("S2-U10", motion_config_test_bounds(),
               "representation boundaries impose no invented calibration");
    TEST_CHECK("S2-U11", motion_config_test_unchanged(),
               "validation preserves every field on all nonnull paths");
    TEST_CHECK("S2-U12", motion_config_test_enums(),
               "unusual invalid enum values are rejected in every field");
    TEST_SUMMARY("subsystem2 M1A");

#if defined(MOTION_CONFIG_HOST_TEST)
    return (0U == test_fail);
#else
    return (0 == test_fail_);
#endif
}

#if defined(MOTION_CONFIG_HOST_TEST)
int main(void)
{
    bool b_config_pass = motion_config_tests();
    uint32_t foundation_failures = motion_foundation_test();

    return ((b_config_pass && (0U == foundation_failures)) ? 0 : 1);
}
#else

EXPORT void subsystem2_test(void)
{
	T_RSEM	rsem;
	T_RMPF	rmpf;
	UW	seq;
	INT	v, bad = 0;

	TEST_INIT();

	/* S2-U01: every sample value is inside the documented 0..100 range */
	for (seq = 0; seq < 40; seq++) {
		v = s2_make_value(seq);
		if (v < 0 || v > 100) bad++;
	}
	TEST_CHECK("S2-U01", bad == 0, "sample values stay within 0..100");

	/* S2-U02: flow control never over- or under-counts */
	TEST_CHECK("S2-U02",
		   tk_ref_sem(sem_slots, &rsem) == E_OK &&
		   rsem.semcnt >= 0 && rsem.semcnt <= S2_POOL_COUNT &&
		   tk_ref_mpf(mpf_samples, &rmpf) == E_OK &&
		   rmpf.frbcnt <= S2_POOL_COUNT,
		   "free slots and free blocks within pool size");

	TEST_SUMMARY("subsystem2");

    (void)motion_config_tests();
    /* Report all pure foundation failures; no motor actuation is involved. */
    (void)motion_foundation_test();
}

#endif /* MOTION_CONFIG_HOST_TEST */
#endif /* MOTION_CONFIG_HOST_TEST || APP_TEST */
