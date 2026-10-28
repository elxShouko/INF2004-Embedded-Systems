/** @file test_commission.c @brief Production bench logic, synthetic ports only. */
#include "subsystem2/bench_motion.h"
#include "subsystem2/enc_capture.h"
#include "subsystem1/s1_codec.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    uint32_t prepares;
    uint32_t stops;
    uint16_t level[2];
    bool b_fail_right;
} bench_fake_t;

static bool prepare (void * p_context, uint16_t period, uint32_t clock)
{
    bench_fake_t * p_fake = p_context;
    ++p_fake->prepares;
    return ((6250U == period) && (125000000U == clock));
}

static bool apply (void * p_context, drv_motor_wheel_t wheel,
    uint16_t a, uint16_t b)
{
    bench_fake_t * p_fake = p_context;
    if (p_fake->b_fail_right && (DRV_MOTOR_RIGHT == wheel) &&
        ((0U != a) || (0U != b))) { return (false); }
    p_fake->level[wheel] = (a > b) ? a : b;
    return (true);
}

static bool stop (void * p_context)
{
    bench_fake_t * p_fake = p_context;
    ++p_fake->stops;
    p_fake->level[0] = 0U;
    p_fake->level[1] = 0U;
    return (true);
}

static motion_bench_config_t config (void)
{
    /* Synthetic fixture only; these are never included in the robot profile. */
    motion_bench_config_t cfg = {0};
    cfg.motor = (drv_motor_config_t){125000000U, 20000U, 101U, 0.2F,
        {MOTION_TYPES_POLARITY_NORMAL, MOTION_TYPES_POLARITY_NORMAL},
        DRV_MOTOR_BRAKE, MOTION_CONFIG_CALIBRATION_CONFIRMED, true};
    cfg.effort_permille[0] = 100;
    cfg.effort_permille[1] = 100;
    cfg.pulse_us = 100000U;
    cfg.b_wheels_raised = true;
    cfg.b_power_authorised = true;
    return (cfg);
}

static bool test (uint32_t id)
{
    if (90U == id)
    {
        uint8_t bytes[6] = {0xA1U, 0x78U, 0x56U, 0x34U, 0x12U, 0U};
        uint32_t session = 0U;
        for (uint8_t code = 0xA1U; code <= 0xA5U; ++code)
        {
            bytes[0] = code;
            if ((s1_codec_command_id_t)(S1_CMD_BENCH_ENCODERS + code - 0xA1U) !=
                s1_codec_parse_robot(bytes, 5U, &session)) { return (false); }
            if (0x12345678U != session) { return (false); }
        }
        return ((S1_CMD_NONE == s1_codec_parse_robot(bytes, 4U, &session)) &&
            (S1_CMD_NONE == s1_codec_parse_robot(bytes, 6U, &session)) &&
            (S1_CMD_NONE == s1_codec_parse_robot(NULL, 5U, &session)) &&
            (S1_CMD_NONE == s1_codec_parse_robot(bytes, 5U, NULL)));
    }
    if (91U == id)
    {
        uint8_t bytes[5] = {0xA1U, 0U, 0U, 0U, 0U};
        uint32_t session = 123U;
        if ((S1_CMD_NONE != s1_codec_parse_robot(bytes, 5U, &session)) ||
            (123U != session)) { return (false); }
        bytes[0] = 0xA6U; bytes[1] = 1U;
        if (S1_CMD_NONE != s1_codec_parse_robot(bytes, 5U, &session))
        { return (false); }
        for (uint8_t code = 1U; code <= 4U; ++code)
        {
            if ((s1_codec_command_id_t)code !=
                s1_codec_parse_command_bin(&code, 1U)) { return (false); }
        }
        return (true);
    }
    if (92U == id)
    {
        enc_capture_t capture = {0};
        enc_capture_sample_t sample = {0};
        if (enc_capture_init_raw(&capture, 0U) ||
            !enc_capture_init_raw(&capture, 1U) ||
            !enc_capture_feed(&capture, ENC_CAPTURE_MASK, 0U)) { return (false); }
        uint32_t levels[] = {1UL << 28U, (1UL << 28U) | (1UL << 7U),
            1UL << 7U, 0U};
        for (uint32_t index = 0U; index < 4U; ++index)
        {
            if (!enc_capture_feed(&capture, ENC_CAPTURE_LEFT_MASK, levels[index]))
            { return (false); }
        }
        return (enc_capture_get(&capture, 10000U, &sample) &&
            (4 == sample.sample[0].count) && (0 == sample.sample[1].count) &&
            !enc_capture_feed(&capture, ENC_CAPTURE_LEFT_MASK,
                (1UL << 7U) | (1UL << 28U)) &&
            !enc_capture_get(&capture, 20000U, &sample));
    }
    motion_bench_t bench = {0};
    bench_fake_t fake = {0};
    drv_motor_io_t io = {&fake, prepare, apply, stop};
    motion_bench_config_t cfg = config();
    if (93U == id)
    {
        cfg = (motion_bench_config_t){0};
        return (motion_bench_init(&bench, &cfg, &io) &&
            !motion_bench_arm(&bench, 0U, true, true) &&
            !motion_bench_pulse(&bench, 0U, true, true) &&
            (0U == fake.prepares) && (0U == fake.stops));
    }
    if (94U == id) { cfg.b_power_authorised = false; }
    if (95U == id) { cfg.b_wheels_raised = false; }
    if (96U == id) { cfg.pulse_us = MOTION_BENCH_MAX_PULSE_US + 1U; }
    if (!motion_bench_init(&bench, &cfg, &io)) { return (false); }
    if ((id >= 94U) && (id <= 96U))
    {
        return (!motion_bench_arm(&bench, 0U, true, true) &&
            (0U == fake.prepares));
    }
    if (97U == id)
    {
        return (!motion_bench_arm(&bench, 0U, false, true) &&
            !motion_bench_arm(&bench, 0U, true, false) &&
            (0U == fake.prepares));
    }
    if (!motion_bench_arm(&bench, 10000U, true, true)) { return (false); }
    if (98U == id)
    {
        motion_bench_tick(&bench, 10000U + MOTION_BENCH_ARM_US,
            true, true, false, false);
        return (!bench.status.b_armed && bench.status.b_used &&
            !motion_bench_pulse(&bench, 5020000U, true, true));
    }
    fake.b_fail_right = (101U == id);
    bool b_pulse = motion_bench_pulse(&bench, 20000U, true, true);
    if (101U == id)
    {
        return (!b_pulse && bench.status.b_fault && (0U == fake.level[0]) &&
            (0U == fake.level[1]) && (fake.stops > 0U));
    }
    if (!b_pulse || (0U == fake.level[0])) { return (false); }
    if (99U == id)
    {
        motion_bench_tick(&bench, 119999U, true, true, false, false);
        if (!bench.status.b_running) { return (false); }
        motion_bench_tick(&bench, 120000U, true, true, false, false);
    }
    else if (100U == id)
    {
        motion_bench_tick(&bench, 30000U, true, false, false, false);
        if (!bench.status.b_fault) { return (false); }
    }
    else { return (false); }
    return (!bench.status.b_running && !bench.status.b_armed &&
        (0U == fake.level[0]) && (0U == fake.level[1]) &&
        !motion_bench_arm(&bench, 130000U, true, true) &&
        !motion_bench_pulse(&bench, 130000U, true, true));
}

int main (int argc, char * argv[])
{
    if (2 != argc) { return (2); }
    uint32_t id = (uint32_t)strtoul(argv[1], NULL, 10);
    bool b_ok = test(id);
    (void)printf("[TEST] INT-%u %s production commissioning policy\n",
        id, b_ok ? "PASS" : "FAIL");
    return (b_ok ? 0 : 1);
}

/*** end of file ***/
