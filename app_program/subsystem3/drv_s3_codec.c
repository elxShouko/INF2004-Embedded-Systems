/** @file drv_s3_codec.c
 * @brief Checked compact queue frames, with no compiler bit-field wire ABI.
 */

#include "drv_s3_codec.h"

#include <stddef.h>
#include <string.h>

/** @brief Append an unsigned field to a pre-sized private frame. */
static void put (uint8_t * p_bytes, uint32_t * p_bit, uint64_t value,
                  uint32_t width)
{
    for (uint32_t offset = 0U; offset < width; ++offset)
    {
        const uint32_t bit = *p_bit + offset;
        p_bytes[bit / 8U] = (uint8_t)(
            (uint32_t)p_bytes[bit / 8U] |
            (uint32_t)(((value >> offset) & 1U) << (bit % 8U)));
    }
    *p_bit += width;
}

/** @brief Read a field only after validating the complete frame length. */
static uint64_t get (const uint8_t * p_bytes, uint32_t * p_bit,
                      uint32_t width)
{
    uint64_t value = 0U;
    for (uint32_t offset = 0U; offset < width; ++offset)
    {
        const uint32_t bit = *p_bit + offset;
        value |= (uint64_t)(((uint32_t)p_bytes[bit / 8U] >>
                             (bit % 8U)) & 1U)
                 << offset;
    }
    *p_bit += width;
    return value;
}

/** @brief Reject surplus bytes and noncanonical unused high bits. */
static bool finished (const uint8_t * p_bytes, uint32_t size, uint32_t bit)
{
    const uint32_t bytes = (bit + 7U) / 8U;
    return (size == bytes) &&
           ((0U == (bit % 8U)) ||
            (0U == ((uint32_t)p_bytes[bytes - 1U] >> (bit % 8U))));
}

/** @brief Return the exact request length for a three-bit tag. */
static uint32_t input_size (uint32_t kind)
{
    static const uint8_t sizes[8] = {79U, 5U, 18U, 21U, 9U, 5U, 5U, 5U};
    return sizes[kind & 7U];
}

/** @brief Encode configuration without narrowing any accepted numeric range. */
static void config_put (const s3_config_t * p_config, uint8_t * p_bytes,
                         uint32_t * p_bit)
{
    const bool b_masks = (p_config->calibration.black_high_mask <= 7U) &&
                         (p_config->calibration.valid_mask <= 7U);
    put(p_bytes, p_bit, p_config->calibration.black_high_mask & 7U, 3U);
    put(p_bytes, p_bit, b_masks ? p_config->calibration.valid_mask : 0U, 3U);
    put(p_bytes, p_bit,
        (p_config->period_ms <= 100U) ? p_config->period_ms : 127U, 7U);
    put(p_bytes, p_bit, p_config->line_stable_us, 32U);
    put(p_bytes, p_bit, p_config->line_lost_us, 32U);
    put(p_bytes, p_bit, p_config->line_found_us, 32U);
    put(p_bytes, p_bit, p_config->recovery_timeout_us, 32U);
    put(p_bytes, p_bit, p_config->barcode_quiet_us, 32U);
    put(p_bytes, p_bit, p_config->barcode_timeout_us, 32U);
    put(p_bytes, p_bit, p_config->max_sample_gap_us, 32U);
    put(p_bytes, p_bit, p_config->odometry_stale_us, 32U);
    put(p_bytes, p_bit, p_config->barcode_rearm_us, 32U);
    put(p_bytes, p_bit, p_config->junction_enabled, 1U);
    put(p_bytes, p_bit, p_config->junction.max_barcode_bar_um, 32U);
    put(p_bytes, p_bit, p_config->junction.min_barcode_gap_um, 32U);
    put(p_bytes, p_bit, p_config->junction.min_span_um, 32U);
    put(p_bytes, p_bit, p_config->junction.max_span_um, 32U);
    put(p_bytes, p_bit, p_config->junction.clear_span_um, 32U);
    put(p_bytes, p_bit, p_config->junction.max_sample_step_um, 32U);
    put(p_bytes, p_bit, p_config->junction.max_sample_gap_us, 32U);
    put(p_bytes, p_bit, p_config->junction.min_black_us, 32U);
    put(p_bytes, p_bit, p_config->junction.max_crossing_us, 32U);
}

/** @brief Restore settings, including invalid values for owner rejection. */
static void config_get (const uint8_t * p_bytes, uint32_t * p_bit,
                         s3_config_t * p_config)
{
    p_config->calibration.black_high_mask = (uint8_t)get(p_bytes, p_bit, 3U);
    p_config->calibration.valid_mask = (uint8_t)get(p_bytes, p_bit, 3U);
    p_config->period_ms = (uint32_t)get(p_bytes, p_bit, 7U);
    p_config->line_stable_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->line_lost_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->line_found_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->recovery_timeout_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->barcode_quiet_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->barcode_timeout_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->max_sample_gap_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->odometry_stale_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->barcode_rearm_us = (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction_enabled = (0U != get(p_bytes, p_bit, 1U));
    p_config->junction.max_barcode_bar_um =
        (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction.min_barcode_gap_um =
        (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction.min_span_um =
        (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction.max_span_um =
        (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction.clear_span_um =
        (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction.max_sample_step_um =
        (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction.max_sample_gap_us =
        (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction.min_black_us =
        (uint32_t)get(p_bytes, p_bit, 32U);
    p_config->junction.max_crossing_us =
        (uint32_t)get(p_bytes, p_bit, 32U);
}

/** @brief Encode a binary partial map; invalid ingress queues a rejection. */
static void mapping_put (const nav_update_t * p_update, uint8_t * p_bytes,
                          uint32_t * p_bit)
{
    const bool b_valid = nav_update_valid(p_update);
    put(p_bytes, p_bit, b_valid ? p_update->selected : 0U, 26U);
    for (uint32_t index = 0U; index < NAV_ALPHABET_SIZE; ++index)
    {
        put(p_bytes, p_bit,
            b_valid ? (uint32_t)nav_action_get(p_update->actions, index) : 0U,
            3U);
    }
}

/** @brief Restore an exact partial map, retaining zero-selection rejection. */
static bool mapping_get (const uint8_t * p_bytes, uint32_t * p_bit,
                          nav_update_t * p_update)
{
    p_update->selected = (uint32_t)get(p_bytes, p_bit, 26U);
    for (uint32_t index = 0U; index < NAV_ALPHABET_SIZE; ++index)
    {
        const uint32_t action = (uint32_t)get(p_bytes, p_bit, 3U);
        if (!nav_action_set(p_update->actions, index, (nav_command_t)action))
        {
            return false;
        }
    }
    if (0U == p_update->selected)
    {
        const uint8_t empty[NAV_ACTION_BYTES] = {0};
        return (0 == memcmp(empty, p_update->actions, sizeof(empty)));
    }
    return nav_update_valid(p_update);
}

/** @brief Encode a request and retain owner-task validation replies. */
uint32_t s3_input_encode (const s3_input_t * p_input, uint8_t * p_bytes)
{
    if ((NULL == p_input) || (NULL == p_bytes))
    {
        return 0U;
    }
    memset(p_bytes, 0, S3_INPUT_WIRE_MAX);
    uint32_t bit = 0U;
    const uint32_t kind = ((uint32_t)p_input->kind <= 4U)
                              ? (uint32_t)p_input->kind : 7U;
    put(p_bytes, &bit, kind, 3U);
    put(p_bytes, &bit, p_input->request_id, 32U);
    switch (kind)
    {
        case S3_INPUT_CONFIG:
            config_put(&p_input->value.config, p_bytes, &bit);
        break;
        case S3_INPUT_MODE:
            /* Reserved tag 7 queues invalid input without truncation. */
            if ((uint32_t)p_input->value.mode > 3U)
            {
                p_bytes[0] |= 7U;
                return input_size(7U);
            }
            put(p_bytes, &bit, (uint32_t)p_input->value.mode, 2U);
        break;
        case S3_INPUT_MAPPING:
            mapping_put(&p_input->value.mapping, p_bytes, &bit);
        break;
        case S3_INPUT_ODOMETRY:
            put(p_bytes, &bit, p_input->value.odometry.timestamp_us, 64U);
            put(p_bytes, &bit, p_input->value.odometry.forward_distance_um,
                64U);
            put(p_bytes, &bit, p_input->value.odometry.tracking_forward, 1U);
        break;
        case S3_INPUT_MANOEUVRE_DONE:
            put(p_bytes, &bit, p_input->value.manoeuvre_event_id, 32U);
        break;
        default:
        break;
    }
    return (bit + 7U) / 8U;
}

/** @brief Decode exact-size frames, preserving caller data on error. */
bool s3_input_decode (const uint8_t * p_bytes, uint32_t size,
                      s3_input_t * p_input)
{
    if ((NULL == p_bytes) || (NULL == p_input) || (0U == size) ||
        (size != input_size(p_bytes[0] & 7U)))
    {
        return false;
    }
    if (((p_bytes[0] & 7U) == 5U) || ((p_bytes[0] & 7U) == 6U))
    {
        return false;
    }
    s3_input_t candidate = {0};
    uint32_t bit = 0U;
    candidate.kind = (s3_input_kind_t)get(p_bytes, &bit, 3U);
    candidate.request_id = (uint32_t)get(p_bytes, &bit, 32U);
    switch (candidate.kind)
    {
        case S3_INPUT_CONFIG:
            config_get(p_bytes, &bit, &candidate.value.config);
        break;
        case S3_INPUT_MODE:
            candidate.value.mode = (line_mode_t)get(p_bytes, &bit, 2U);
        break;
        case S3_INPUT_MAPPING:
            if (!mapping_get(p_bytes, &bit, &candidate.value.mapping))
            {
                return false;
            }
        break;
        case S3_INPUT_ODOMETRY:
            candidate.value.odometry.timestamp_us = get(p_bytes, &bit, 64U);
            candidate.value.odometry.forward_distance_um =
                get(p_bytes, &bit, 64U);
            candidate.value.odometry.tracking_forward =
                (0U != get(p_bytes, &bit, 1U));
        break;
        case S3_INPUT_MANOEUVRE_DONE:
            candidate.value.manoeuvre_event_id =
                (uint32_t)get(p_bytes, &bit, 32U);
        break;
        default:
            /* Unknown application requests still receive a negative reply. */
        break;
    }
    if (!finished(p_bytes, size, bit))
    {
        return false;
    }
    *p_input = candidate;
    return true;
}

/** @brief Validate semantic enum ranges before building an output frame. */
static bool output_valid (const s3_output_t * p_output)
{
    return (p_output->kind <= S3_OUTPUT_REPLY) &&
           (p_output->navigation.black_mask <= 7U) &&
           (p_output->navigation.kind <= NAV_EVENT_FAULT) &&
           (p_output->navigation.command <= NAV_COMMAND_U_TURN) &&
           (p_output->navigation.state <= LINE_STATE_FAULT_REPORTED) &&
           ((p_output->navigation.barcode_character == '\0') ||
            ((p_output->navigation.barcode_character >= 'A') &&
             (p_output->navigation.barcode_character <= 'Z')));
}

/** @brief Encode copied output fields, omitting other output-kind payloads. */
uint32_t s3_output_encode (const s3_output_t * p_output, uint8_t * p_bytes)
{
    if ((NULL == p_output) || (NULL == p_bytes) || !output_valid(p_output))
    {
        return 0U;
    }
    memset(p_bytes, 0, S3_OUTPUT_WIRE_MAX);
    uint32_t bit = 0U;
    put(p_bytes, &bit, p_output->kind, 2U);
    put(p_bytes, &bit, p_output->event_id, 32U);
    put(p_bytes, &bit, p_output->map_revision, 32U);
    put(p_bytes, &bit, p_output->fault, 5U);
    if (S3_OUTPUT_REPLY == p_output->kind)
    {
        put(p_bytes, &bit, p_output->request_id, 32U);
        put(p_bytes, &bit, p_output->accepted, 1U);
    }
    else
    {
        put(p_bytes, &bit, p_output->navigation.timestamp_us, 64U);
        put(p_bytes, &bit, p_output->navigation.black_mask, 3U);
        if (S3_OUTPUT_LINE == p_output->kind)
        {
            put(p_bytes, &bit, p_output->line, 2U);
        }
        else
        {
            put(p_bytes, &bit, p_output->navigation.kind, 3U);
            put(p_bytes, &bit, p_output->navigation.command, 3U);
            put(p_bytes, &bit, p_output->navigation.state, 3U);
            const char letter = p_output->navigation.barcode_character;
            put(p_bytes, &bit, (letter == '\0') ? 26U :
                (uint32_t)(letter - 'A'), 5U);
        }
    }
    return (bit + 7U) / 8U;
}

/** @brief Decode outputs after checking their kind-specific byte length. */
bool s3_output_decode (const uint8_t * p_bytes, uint32_t size,
                       s3_output_t * p_output)
{
    static const uint8_t sizes[4] = {19U, 18U, 13U, 0U};
    if ((NULL == p_bytes) || (NULL == p_output) || (0U == size) ||
        (size != sizes[p_bytes[0] & 3U]))
    {
        return false;
    }
    s3_output_t candidate = {0};
    uint32_t bit = 0U;
    candidate.kind = (uint8_t)(get(p_bytes, &bit, 2U) & 3U);
    candidate.event_id = (uint32_t)get(p_bytes, &bit, 32U);
    candidate.map_revision = (uint32_t)get(p_bytes, &bit, 32U);
    candidate.fault = (uint8_t)(get(p_bytes, &bit, 5U) & 31U);
    if (S3_OUTPUT_REPLY == candidate.kind)
    {
        candidate.request_id = (uint32_t)get(p_bytes, &bit, 32U);
        candidate.accepted = (uint8_t)(get(p_bytes, &bit, 1U) & 1U);
    }
    else
    {
        candidate.navigation.timestamp_us = get(p_bytes, &bit, 64U);
        candidate.navigation.black_mask =
            (uint8_t)(get(p_bytes, &bit, 3U) & 7U);
        if (S3_OUTPUT_LINE == candidate.kind)
        {
            candidate.line = (uint8_t)(get(p_bytes, &bit, 2U) & 3U);
        }
        else
        {
            candidate.navigation.kind =
                (uint8_t)(get(p_bytes, &bit, 3U) & 7U);
            candidate.navigation.command =
                (uint8_t)(get(p_bytes, &bit, 3U) & 7U);
            candidate.navigation.state =
                (uint8_t)(get(p_bytes, &bit, 3U) & 7U);
            const uint32_t letter = (uint32_t)get(p_bytes, &bit, 5U);
            if (letter > 26U)
            {
                return false;
            }
            candidate.navigation.barcode_character =
                (26U == letter) ? '\0' : (char)('A' + letter);
        }
    }
    if (!finished(p_bytes, size, bit) || !output_valid(&candidate))
    {
        return false;
    }
    *p_output = candidate;
    return true;
}

/*** End of file. ***/

