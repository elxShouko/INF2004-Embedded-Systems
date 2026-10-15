/** @file buddy4_cases.c
 * Synthetic tests of CURRENT app_program/subsystem4 sources, not archives.
 * Runs serially on host or in the template's initial test task.
 */
#include "buddy4_cases.h"
#include "../../../app_program/subsystem4/imu_core.h"
#include "../../../app_program/subsystem4/drv_lsm.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static void report_case (const char *p_id, bool b_pass, const char *p_text);
static bool near_value (float32_t actual, float32_t expected);
static void ready_core (imu_core_t *p_core, imu_core_sample_t *p_sample);
static void step_pitch (imu_core_t *p_core, imu_core_sample_t *p_sample,
                        float32_t degrees);
static void complete_hump (imu_core_t *p_core,
                           imu_core_sample_t *p_sample);
static void test_calibration (void);
static void test_humps (void);
static void test_faults (void);
static void test_heading (void);
static void test_driver (void);
static void test_driver_failures (void);
static void test_boundaries (void);
static void test_motion_validity (void);
static void test_braking_regression (void);
static void bus_reset (void);
static bool fake_read (uint8_t address, uint8_t reg, uint8_t *p_data,
                       uint32_t length);
static bool fake_write (uint8_t address, uint8_t reg, uint8_t value);

static buddy4_cases_report_t g_p_report;
static uint32_t g_failures;
static uint8_t g_accel[256];
static uint8_t g_mag[256];
static bool g_b_io_failure;
static bool g_b_bad_readback;
static const drv_lsm_bus_t g_bus = {fake_read, fake_write};

uint32_t buddy4_cases_run (buddy4_cases_report_t p_report)
{
    g_p_report = p_report;
    g_failures = 0u;
    test_calibration();
    test_humps();
    test_faults();
    test_heading();
    test_driver();
    test_driver_failures();
    test_boundaries();
    test_motion_validity();
    test_braking_regression();
    return g_failures;
}

static void report_case (const char *p_id, bool b_pass, const char *p_text)
{
    if (!b_pass)
    {
        ++g_failures;
    }
    if (NULL != g_p_report)
    {
        g_p_report(p_id, b_pass, p_text);
    }
}

static bool near_value (float32_t actual, float32_t expected)
{
    return isfinite(actual) && (fabsf(actual - expected) < 0.01f);
}

static void ready_core (imu_core_t *p_core, imu_core_sample_t *p_sample)
{
    imu_core_config_t config = imu_core_defaults();

    memset(p_sample, 0, sizeof(*p_sample));
    config.calibration_samples = 2u;
    config.b_mag_calibrated = true;
    /* Synthetic regression envelope, NOT measured vehicle calibration. */
    config.b_height_calibrated = true;
    config.maximum_accel_mm_s2 = 100.0f;
    config.speed_band_mm_s = 5.0f;
    config.wheel_delta_mm_s = 5.0f;
    config.minimum_speed_mm_s = 1.0f;
    config.settle_ms = 20u;
    config.ascent_ms = 20u;
    config.level_ms = 20u;
    (void)imu_core_init(p_core, &config);
    p_sample->accel_g[2] = 1.0f;
    p_sample->time_ms = 20u;
    (void)imu_core_update(p_core, p_sample);
    p_sample->time_ms = 40u;
    (void)imu_core_update(p_core, p_sample);
    p_sample->b_motion_valid = true;
    p_sample->b_straight = true;
    p_sample->b_traction_trusted = true;
    p_sample->b_distance_valid = true;
    p_sample->distance_mm = 10.0f;
    step_pitch(p_core, p_sample, 0.0f);
    step_pitch(p_core, p_sample, 0.0f);
    p_sample->b_distance_valid = false;
}

static void step_pitch (imu_core_t *p_core, imu_core_sample_t *p_sample,
                        float32_t degrees)
{
    const float32_t radians = degrees / 57.2957795131f;

    p_sample->accel_g[0] = -sinf(radians);
    p_sample->accel_g[1] = 0.0f;
    p_sample->accel_g[2] = cosf(radians);
    p_sample->time_ms += 20u;
    p_sample->motion_time_ms = p_sample->time_ms;
    p_sample->speed_mm_s = p_sample->distance_mm * 50.0f;
    (void)imu_core_update(p_core, p_sample);
}

static void complete_hump (imu_core_t *p_core,
                           imu_core_sample_t *p_sample)
{
    step_pitch(p_core, p_sample, 0.0f);
    step_pitch(p_core, p_sample, 0.0f);
    step_pitch(p_core, p_sample, 0.0f);
    step_pitch(p_core, p_sample, 30.0f);
    step_pitch(p_core, p_sample, 30.0f);
    step_pitch(p_core, p_sample, 0.0f);
    step_pitch(p_core, p_sample, -30.0f);
    step_pitch(p_core, p_sample, -30.0f);
    step_pitch(p_core, p_sample, 0.0f);
    step_pitch(p_core, p_sample, 0.0f);
}

static void test_calibration (void)
{
    imu_core_t core = {0};
    imu_core_sample_t sample = {0};
    imu_core_config_t config = imu_core_defaults();

    config.ascend_deg = NAN;
    report_case("S4-U116", (!imu_core_init(&core, &config)) &&
                 (!imu_core_init(NULL, &config)) &&
                 (!imu_core_init(&core, NULL)), "invalid config rejected");
    config = imu_core_defaults();
    config.calibration_samples = 2u;
    (void)imu_core_init(&core, &config);
    sample.accel_g[2] = 1.0f;
    sample.time_ms = 20u;
    (void)imu_core_update(&core, &sample);
    sample.time_ms = 40u;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U117", (IMU_MONITORING == core.status.phase) &&
                 (IMU_EVENT_CALIBRATED == core.status.events),
                 "stationary calibration completes");
    config = imu_core_defaults();
    config.calibration_samples = 2u;
    (void)imu_core_init(&core, &config);
    sample.time_ms = 20u;
    (void)imu_core_update(&core, &sample);
    sample.accel_g[0] = 0.2f;
    sample.time_ms += 20u;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U118", (0u == core.count) &&
                 (IMU_CALIBRATING == core.status.phase),
                 "motion resets calibration window");
    ready_core(&core, &sample);
    step_pitch(&core, &sample, 30.0f);
    report_case("S4-U119", near_value(core.status.pitch_deg, 30.0f),
                 "body-axis pitch is degrees");
    ready_core(&core, &sample);
    sample.accel_g[1] = 0.5f;
    sample.accel_g[2] = 0.8660254f;
    sample.time_ms += 20u;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U120", near_value(core.status.roll_deg, 30.0f),
                 "body-axis roll is degrees");
}

static void test_humps (void)
{
    imu_core_t core = {0};
    imu_core_sample_t sample = {0};
    bool b_states = false;

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    step_pitch(&core, &sample, 30.0f);
    b_states = (IMU_ASCENDING == core.status.phase);
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    b_states = b_states && (IMU_PEAK == core.status.phase);
    report_case("S4-U162", core.status.b_highest_valid &&
                 near_value(core.status.highest_hump_mm, 10.0f) &&
                 (0u != (core.status.events & IMU_EVENT_PEAK)),
                 "Week 6 highest estimate is published at peak");
    step_pitch(&core, &sample, -30.0f);
    b_states = b_states && (IMU_DESCENDING == core.status.phase);
    step_pitch(&core, &sample, -30.0f);
    step_pitch(&core, &sample, 0.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U121", b_states &&
                 (IMU_MONITORING == core.status.phase) &&
                 core.status.b_height_valid &&
                 near_value(core.status.hump_peak_mm, 10.0f),
                 "ascent/peak/descent and distance-integrated height");
    sample.distance_mm = 5.0f;
    complete_hump(&core, &sample);
    report_case("S4-U122", near_value(core.status.highest_hump_mm, 10.0f),
                 "smaller completed hump retains session maximum");
    sample.distance_mm = 20.0f;
    complete_hump(&core, &sample);
    report_case("S4-U123", near_value(core.status.highest_hump_mm, 20.0f),
                 "larger completed hump updates session maximum");
    ready_core(&core, &sample);
    complete_hump(&core, &sample);
    report_case("S4-U124", (!core.status.b_height_valid) &&
                 (!core.status.b_highest_valid),
                 "pitch alone is not reported as measured height");
    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    step_pitch(&core, &sample, 30.0f);
    sample.b_distance_valid = false;
    step_pitch(&core, &sample, 20.0f);
    sample.b_distance_valid = true;
    step_pitch(&core, &sample, 0.0f);
    step_pitch(&core, &sample, -30.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U125", !core.status.b_height_valid,
                 "missing distance invalidates entire current hump");
    ready_core(&core, &sample);
    core.config.maximum_hump_ms = 101u;
    step_pitch(&core, &sample, 30.0f);
    for (uint32_t count = 0u; count < 6u; ++count)
    {
        step_pitch(&core, &sample, 30.0f);
    }
    report_case("S4-U126", IMU_FAULT == core.status.phase,
                 "unfinished hump times out to a latched fault");
}

static void test_faults (void)
{
    imu_core_t core = {0};
    imu_core_sample_t sample = {0};

    ready_core(&core, &sample);
    sample.accel_g[2] = 1.9f;
    sample.time_ms += 20u;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U127", (IMU_FAULT == core.status.phase) &&
                 (0u != (core.status.events & IMU_EVENT_IMPACT)),
                 "impact latches fault");
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U128", IMU_FAULT == core.status.phase,
                 "good data cannot silently clear fault");
    ready_core(&core, &sample);
    (void)imu_core_update(&core, &sample);
    report_case("S4-U129", IMU_FAULT == core.status.phase,
                 "duplicate timestamp rejected");
    ready_core(&core, &sample);
    core.status.time_ms = UINT32_MAX - 9u;
    sample.time_ms = 10u;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U130", IMU_MONITORING == core.status.phase,
                 "monotonic timestamp wraps correctly");
    ready_core(&core, &sample);
    sample.time_ms += 101u;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U131", IMU_FAULT == core.status.phase,
                 "stale samples rejected");
    ready_core(&core, &sample);
    sample.accel_g[0] = NAN;
    sample.time_ms += 20u;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U132", IMU_FAULT == core.status.phase,
                 "NaN cannot enter processing");
    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = -1.0f;
    sample.time_ms += 20u;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U133", IMU_FAULT == core.status.phase,
                 "unsupported reverse distance rejected");
    ready_core(&core, &sample);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U134", IMU_MOTION_UNKNOWN == core.status.motion,
                 "gravity alone does not prove vehicle is stationary");
}

static void test_heading (void)
{
    imu_core_t core = {0};
    imu_core_sample_t sample = {0};

    ready_core(&core, &sample);
    sample.b_mag_valid = true;
    sample.mag_gauss[0] = 0.5f;
    core.config.b_mag_calibrated = false;
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U135", !core.status.b_heading_valid,
                 "uncalibrated magnetometer does not claim valid heading");
    core.config.b_mag_calibrated = true;
    sample.mag_gauss[0] = -0.49992385f;
    sample.mag_gauss[1] = -0.0087262f;
    step_pitch(&core, &sample, 0.0f);
    sample.mag_gauss[1] = 0.0087262f;
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U136", core.status.b_heading_valid &&
                 (fabsf(core.status.turn_dps - 100.0f) < 0.1f) &&
                 (0u != (core.status.events & IMU_EVENT_TURN)),
                 "heading wrap uses shortest turn, no fictitious gyro");
    sample.b_mag_valid = false;
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U137", (!core.status.b_heading_valid) &&
                 near_value(core.status.turn_dps, 0.0f),
                 "missing magnetic sample clears heading validity");
}

static void bus_reset (void)
{
    memset(g_accel, 0, sizeof(g_accel));
    memset(g_mag, 0, sizeof(g_mag));
    g_b_io_failure = false;
    g_b_bad_readback = false;
    g_mag[0x0A] = 0x48u;
    g_mag[0x0B] = 0x34u;
    g_mag[0x0C] = 0x33u;
    g_accel[0x27] = 0x08u;
    g_accel[0x2C] = 0x80u; /* +1000 << 4 = 0x3E80. */
    g_accel[0x2D] = 0x3Eu;
}

static bool fake_read (uint8_t address, uint8_t reg, uint8_t *p_data,
                       uint32_t length)
{
    const uint8_t *p_registers = (DRV_LSM_ACCEL_ADDR == address) ?
                                 g_accel : g_mag;
    const uint32_t start = reg & 0x7Fu;

    if (g_b_io_failure || ((start + length) > 256u))
    {
        return false;
    }
    memcpy(p_data, &p_registers[start], length);
    return true;
}

static bool fake_write (uint8_t address, uint8_t reg, uint8_t value)
{
    uint8_t *p_registers = (DRV_LSM_ACCEL_ADDR == address) ? g_accel : g_mag;

    if (g_b_io_failure)
    {
        return false;
    }
    p_registers[reg] = g_b_bad_readback ? (uint8_t)(value ^ 1u) : value;
    return true;
}

static void test_driver (void)
{
    imu_core_sample_t sample = {0};

    bus_reset();
    report_case("S4-U138", drv_lsm_init(&g_bus) &&
                 (0x57u == g_accel[0x20]) &&
                 (0x88u == g_accel[0x23]) && (0x18u == g_mag[0x00]) &&
                 (0x20u == g_mag[0x01]) && (0u == g_mag[0x02]),
                 "datasheet register profile and readback");
    g_accel[0x28] = 0x80u; /* -1000 << 4 encoded as 0xC180. */
    g_accel[0x29] = 0xC1u;
    report_case("S4-U139", (DRV_LSM_OK == drv_lsm_read(&g_bus, &sample)) &&
                 near_value(sample.accel_g[0], -1.0f) &&
                 near_value(sample.accel_g[2], 1.0f),
                 "signed little-endian acceleration in g");
    g_mag[0x09] = 1u;
    g_mag[0x03] = 0x04u; /* X = 1100. */
    g_mag[0x04] = 0x4Cu;
    g_mag[0x05] = 0x03u; /* Z = 980. */
    g_mag[0x06] = 0xD4u;
    g_mag[0x07] = 0xFBu; /* Y = -1100. */
    g_mag[0x08] = 0xB4u;
    report_case("S4-U140", (DRV_LSM_OK == drv_lsm_read(&g_bus, &sample)) &&
                 sample.b_mag_valid &&
                 near_value(sample.mag_gauss[0], 1.0f) &&
                 near_value(sample.mag_gauss[1], -1.0f) &&
                 near_value(sample.mag_gauss[2], 1.0f),
                 "magnetic X,Z,Y ordering and unequal axis gains");
    g_mag[0x09] = 0u;
    report_case("S4-U141", (DRV_LSM_OK == drv_lsm_read(&g_bus, &sample)) &&
                 (!sample.b_mag_valid), "no stale magnetic sample reuse");
    g_accel[0x27] = 0u;
    report_case("S4-U142", DRV_LSM_NOT_READY ==
                 drv_lsm_read(&g_bus, &sample),
                 "no stale acceleration sample reuse");
}

static void test_driver_failures (void)
{
    imu_core_sample_t sample = {0};

    bus_reset();
    g_mag[0x0A] = 0u;
    report_case("S4-U143", !drv_lsm_init(&g_bus),
                 "wrong sensor identity rejected");
    bus_reset();
    g_b_bad_readback = true;
    report_case("S4-U144", !drv_lsm_init(&g_bus),
                 "configuration readback mismatch rejected");
    bus_reset();
    g_b_io_failure = true;
    sample.time_ms = 999u;
    report_case("S4-U145", (!drv_lsm_init(&g_bus)) &&
                 (DRV_LSM_IO_ERROR == drv_lsm_read(&g_bus, &sample)) &&
                 (999u == sample.time_ms),
                 "bus failure propagates without partial sample commit");
    bus_reset();
    g_mag[0x09] = 1u;
    g_mag[0x03] = 0xF0u;
    report_case("S4-U146", (DRV_LSM_OK == drv_lsm_read(&g_bus, &sample)) &&
                 (!sample.b_mag_valid), "magnetic overflow invalidates yaw");
    g_accel[0x28] = 0xF0u;
    g_accel[0x29] = 0x7Fu;
    report_case("S4-U147", DRV_LSM_INVALID ==
                 drv_lsm_read(&g_bus, &sample),
                 "acceleration saturation cannot look like normal tilt");
    report_case("S4-U148", (!drv_lsm_init(NULL)) &&
                 (DRV_LSM_INVALID == drv_lsm_read(NULL, &sample)) &&
                 (DRV_LSM_INVALID == drv_lsm_read(&g_bus, NULL)),
                 "invalid driver arguments rejected");
}

static void test_boundaries (void)
{
    imu_core_t core = {0};
    imu_core_sample_t sample = {0};
    bool b_decode_correct = true;

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    sample.accel_g[0] = -0.65f;
    sample.accel_g[2] = 1.125833f;
    sample.time_ms += 20u;
    (void)imu_core_update(&core, &sample);
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U163", (!core.status.b_height_valid) &&
                 (!core.status.b_highest_valid),
                 "dynamic acceleration at ascent entry invalidates height");
    ready_core(&core, &sample);
    sample.time_ms += core.config.maximum_gap_ms;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U164", IMU_MONITORING == core.status.phase,
                 "maximum allowed sample gap is inclusive");
    report_case("S4-U165", (IMU_EVENT_FAULT ==
                 imu_core_update(NULL, &sample)) &&
                 (0u != (imu_core_update(&core, NULL) & IMU_EVENT_FAULT)),
                 "null processing arguments fail safely");
    bus_reset();
    for (int32_t counts = -2048; counts <= 2047; ++counts)
    {
        const int32_t signed_word = counts * 16;
        /* Explicit unsigned encoding avoids shifting a negative number. */
        const uint32_t word = (uint32_t)((signed_word < 0) ?
                               (signed_word + 65536) : signed_word);
        drv_lsm_result_t result = DRV_LSM_INVALID;

        g_accel[0x28] = (uint8_t)(word & 0xFFu);
        g_accel[0x29] = (uint8_t)(word >> 8u);
        result = drv_lsm_read(&g_bus, &sample);
        if ((counts <= -2000) || (counts >= 2000))
        {
            b_decode_correct = b_decode_correct &&
                                (DRV_LSM_INVALID == result);
        }
        else
        {
            /* Every tested signed 12-bit value is exactly representable. */
            b_decode_correct = b_decode_correct && (DRV_LSM_OK == result) &&
                near_value(sample.accel_g[0], (float32_t)counts * 0.001f);
        }
    }
    report_case("S4-U166", b_decode_correct,
                 "all 4096 signed HR encodings and saturation boundaries");
}

/** Old core accepted this flat 0.2 g deceleration as an 11.188 mm hump. */
static void test_braking_regression (void)
{
    imu_core_t core = {0};
    imu_core_sample_t sample = {0};
    imu_core_config_t config = imu_core_defaults();
    float32_t speed = 500.0f;
    const float32_t delta = 0.2f * 9806.65f * 0.020f;

    config.b_height_calibrated = true; /* Synthetic envelope only. */
    config.maximum_accel_mm_s2 = 100.0f;
    config.speed_band_mm_s = 5.0f;
    config.wheel_delta_mm_s = 5.0f;
    config.minimum_speed_mm_s = 1.0f;
    config.settle_ms = 20u;
    config.ascent_ms = 20u;
    config.level_ms = 20u;
    (void)imu_core_init(&core, &config);
    sample.accel_g[2] = 1.0f;
    for (uint32_t count = 1u; count <= 100u; ++count)
    {
        sample.time_ms = count * 20u;
        (void)imu_core_update(&core, &sample);
    }
    sample.b_motion_valid = true;
    sample.b_straight = true;
    sample.b_traction_trusted = true;
    sample.b_distance_valid = true;
    sample.speed_mm_s = speed;
    sample.distance_mm = 10.0f;
    step_pitch(&core, &sample, 0.0f);
    for (uint32_t count = 0u; count < 10u; ++count)
    {
        const float32_t next = speed - delta;

        sample.time_ms += 20u;
        sample.motion_time_ms = sample.time_ms;
        sample.speed_mm_s = next;
        sample.distance_mm = (speed + next) * 0.5f * 0.020f;
        sample.accel_g[0] = -0.2f;
        sample.accel_g[2] = 1.0f;
        (void)imu_core_update(&core, &sample);
        speed = next;
    }
    sample.time_ms += 20u;
    sample.motion_time_ms = sample.time_ms;
    sample.accel_g[0] = 0.0f;
    sample.distance_mm = speed * 0.020f;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U176", (IMU_PEAK == core.status.phase) &&
                 (!core.status.b_highest_valid) &&
                 (!core.status.b_height_valid) &&
                 (0u != (core.status.height_reasons & IMU_HEIGHT_DYNAMIC)),
                 "flat 0.2 g braking cannot create a valid peak");
}

static void test_motion_validity (void)
{
    imu_core_t core = {0};
    imu_core_sample_t sample = {0};
    bool b_peak = false;

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    sample.speed_mm_s = 500.0f;
    step_pitch(&core, &sample, 0.0f);
    sample.accel_g[0] = 0.2f;
    sample.accel_g[2] = 1.0f;
    sample.speed_mm_s = 460.0f;
    sample.time_ms += 20u;
    sample.motion_time_ms = sample.time_ms;
    (void)imu_core_update(&core, &sample);
    report_case("S4-U177", (IMU_MONITORING == core.status.phase) &&
                 (!core.status.b_highest_valid) &&
                 (0u != (core.status.height_reasons & IMU_HEIGHT_DYNAMIC)),
                 "flat acceleration is classified as dynamic");
    sample.accel_g[0] = -0.2f;
    sample.speed_mm_s = 420.0f;
    sample.time_ms += 20u;
    sample.motion_time_ms = sample.time_ms;
    (void)imu_core_update(&core, &sample);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U178", (!core.status.b_highest_valid) &&
                 (!core.status.b_height_valid),
                 "acceleration then braking leaves maximum invalid");

    ready_core(&core, &sample);
    core.config.ascent_ms = 40u;
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    step_pitch(&core, &sample, 30.0f);
    b_peak = (IMU_ASCENDING == core.status.phase);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U179", b_peak && (IMU_PEAK == core.status.phase) &&
                 (!core.status.b_highest_valid) &&
                 (0u != (core.status.height_reasons &
                         IMU_HEIGHT_PERSISTENCE)),
                 "one-sample pitch spike fails ascent dwell");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    step_pitch(&core, &sample, 30.0f);
    sample.motion_age_ms = 41u;
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U180", (IMU_PEAK == core.status.phase) &&
                 (!core.status.b_highest_valid) &&
                 (0u != (core.status.height_reasons & IMU_HEIGHT_MOTION)),
                 "stale motion invalidates an entire candidate");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    step_pitch(&core, &sample, 30.0f);
    sample.b_motion_valid = false;
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U181", (IMU_PEAK == core.status.phase) &&
                 (!core.status.b_highest_valid),
                 "unavailable motion cannot preserve candidate validity");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.speed_mm_s = -50.0f;
    sample.distance_mm = 0.0f;
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U182", (IMU_PEAK == core.status.phase) &&
                 (!core.status.b_highest_valid) &&
                 (0u != (core.status.height_reasons & IMU_HEIGHT_REVERSE)),
                 "reverse speed never counts as forward hump travel");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    sample.b_straight = false;
    sample.wheel_delta_mm_s = 100.0f;
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U183", (IMU_PEAK == core.status.phase) &&
                 (!core.status.b_highest_valid) &&
                 (0u != (core.status.height_reasons & IMU_HEIGHT_TURN)),
                 "in-place turn cannot create a height maximum");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    b_peak = core.status.b_highest_valid &&
             core.status.b_peak_provisional;
    sample.motion_epoch += 1u;
    step_pitch(&core, &sample, -3.0f);
    report_case("S4-U184", b_peak && (!core.status.b_highest_valid) &&
                 (!core.status.b_peak_provisional) &&
                 (0u != (core.status.events & IMU_EVENT_HEIGHT_INVALID)),
                 "revocation retracts a provisional peak");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    complete_hump(&core, &sample);
    b_peak = core.status.b_highest_valid &&
             near_value(core.status.highest_hump_mm, 10.0f);
    sample.b_motion_valid = false;
    complete_hump(&core, &sample);
    report_case("S4-U185", b_peak && core.status.b_highest_valid &&
                 near_value(core.status.highest_hump_mm, 10.0f),
                 "bad later candidate preserves completed maximum");
    sample.b_motion_valid = true;
    sample.distance_mm = 20.0f;
    complete_hump(&core, &sample);
    report_case("S4-U186", core.status.b_highest_valid &&
                 near_value(core.status.highest_hump_mm, 20.0f),
                 "valid hump after rejected candidate updates maximum");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    step_pitch(&core, &sample, -3.0f);
    b_peak = (IMU_DESCENDING == core.status.phase);
    step_pitch(&core, &sample, 0.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U187", b_peak &&
                 (IMU_MONITORING == core.status.phase) &&
                 core.status.b_highest_valid &&
                 (!core.status.b_peak_provisional),
                 "shallow asymmetric descent closes at persistent level");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    sample.b_traction_trusted = false;
    complete_hump(&core, &sample);
    report_case("S4-U188", (!core.status.b_highest_valid) &&
                 (0u != (core.status.height_reasons & IMU_HEIGHT_TRACTION)),
                 "untrusted traction cannot validate height");

    ready_core(&core, &sample);
    sample.b_distance_valid = true;
    sample.distance_mm = 10.0f;
    sample.speed_mm_s = 500.0f;
    step_pitch(&core, &sample, 0.0f);
    sample.distance_mm = 10.2f; /* step_pitch derives 510 mm/s. */
    step_pitch(&core, &sample, 30.0f);
    step_pitch(&core, &sample, 0.0f);
    report_case("S4-U189", (!core.status.b_highest_valid) &&
                 (0u != (core.status.height_reasons & IMU_HEIGHT_DYNAMIC)),
                 "speed drift outside measured band rejects peak");
}

/* End of file. */

