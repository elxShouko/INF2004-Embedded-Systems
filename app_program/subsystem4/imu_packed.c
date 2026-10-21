/** @file imu_packed.c Explicit masks avoid compiler-dependent queue layouts. */
#include "imu_packed.h"

#include <stddef.h>

typedef char imu_packed_status_size_check[
    (IMU_PACKED_STATUS_BYTES == sizeof(imu_packed_status_t)) ? 1 : -1];
typedef char imu_core_status_size_check[
    (IMU_PACKED_STATUS_BYTES == sizeof(imu_core_status_t)) ? 1 : -1];

static uint32_t pack_flag (bool b_value, uint32_t shift);
static bool packed_valid (const imu_packed_status_t *p_packed);

bool imu_packed_encode (const imu_core_status_t *p_status,
                        imu_packed_status_t *p_packed)
{
    uint32_t fields = 0u;

    if ((NULL == p_status) || (NULL == p_packed) ||
        (p_status->phase > IMU_FAULT) ||
        (p_status->motion > IMU_IMPACT))
    {
        return false;
    }
    fields = ((uint32_t)p_status->phase << IMU_PACK_PHASE_SHIFT) |
             ((uint32_t)p_status->motion << IMU_PACK_MOTION_SHIFT) |
             pack_flag(p_status->b_heading_valid,
                       IMU_PACK_HEADING_VALID_SHIFT) |
             pack_flag(p_status->b_height_valid,
                       IMU_PACK_HEIGHT_VALID_SHIFT) |
             pack_flag(p_status->b_highest_valid,
                       IMU_PACK_HIGHEST_VALID_SHIFT) |
             pack_flag(p_status->b_peak_provisional,
                       IMU_PACK_PROVISIONAL_SHIFT) |
             ((uint32_t)p_status->events << IMU_PACK_EVENTS_SHIFT) |
             ((uint32_t)p_status->height_reasons << IMU_PACK_REASONS_SHIFT);
    p_packed->time_ms = p_status->time_ms;
    p_packed->pitch_deg = p_status->pitch_deg;
    p_packed->roll_deg = p_status->roll_deg;
    p_packed->heading_deg = p_status->heading_deg;
    p_packed->turn_dps = p_status->turn_dps;
    p_packed->hump_peak_mm = p_status->hump_peak_mm;
    p_packed->highest_hump_mm = p_status->highest_hump_mm;
    p_packed->fields = fields;
    return true;
}

bool imu_packed_decode (const imu_packed_status_t *p_packed,
                        imu_core_status_t *p_status)
{
    imu_core_status_t decoded = {0};

    if ((NULL == p_status) || (!packed_valid(p_packed)))
    {
        return false;
    }
    decoded.time_ms = p_packed->time_ms;
    decoded.pitch_deg = p_packed->pitch_deg;
    decoded.roll_deg = p_packed->roll_deg;
    decoded.heading_deg = p_packed->heading_deg;
    decoded.turn_dps = p_packed->turn_dps;
    decoded.hump_peak_mm = p_packed->hump_peak_mm;
    decoded.highest_hump_mm = p_packed->highest_hump_mm;
    decoded.phase = (uint32_t)((p_packed->fields & IMU_PACK_PHASE_MASK) >>
                               IMU_PACK_PHASE_SHIFT);
    decoded.motion = (uint32_t)((p_packed->fields & IMU_PACK_MOTION_MASK) >>
                                IMU_PACK_MOTION_SHIFT);
    decoded.b_heading_valid = 0u != (p_packed->fields &
                                    (1u << IMU_PACK_HEADING_VALID_SHIFT));
    decoded.b_height_valid = 0u != (p_packed->fields &
                                   (1u << IMU_PACK_HEIGHT_VALID_SHIFT));
    decoded.b_highest_valid = 0u != (p_packed->fields &
                                    (1u << IMU_PACK_HIGHEST_VALID_SHIFT));
    decoded.b_peak_provisional = 0u != (p_packed->fields &
                                       (1u << IMU_PACK_PROVISIONAL_SHIFT));
    decoded.events = (uint32_t)((p_packed->fields & IMU_PACK_EVENTS_MASK) >>
                                IMU_PACK_EVENTS_SHIFT);
    decoded.height_reasons = (uint32_t)((p_packed->fields &
        IMU_PACK_REASONS_MASK) >> IMU_PACK_REASONS_SHIFT);
    *p_status = decoded;
    return true;
}

static uint32_t pack_flag (bool b_value, uint32_t shift)
{
    return b_value ? (1u << shift) : 0u;
}

static bool packed_valid (const imu_packed_status_t *p_packed)
{
    if ((NULL == p_packed) ||
        (0u != (p_packed->fields & ~IMU_PACK_USED_MASK)))
    {
        return false;
    }
    return (((p_packed->fields & IMU_PACK_PHASE_MASK) >>
             IMU_PACK_PHASE_SHIFT) <= (uint32_t)IMU_FAULT) &&
           (((p_packed->fields & IMU_PACK_MOTION_MASK) >>
             IMU_PACK_MOTION_SHIFT) <= (uint32_t)IMU_IMPACT);
}

/* End of file. */
