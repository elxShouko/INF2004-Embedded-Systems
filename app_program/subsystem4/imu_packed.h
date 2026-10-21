/** @file imu_packed.h
 * Stable Buddy 4 inter-task representation for bounded status fields.
 * Numeric measurements remain float32 until calibrated ranges are agreed.
 */
#ifndef IMU_PACKED_H
#define IMU_PACKED_H

#include "imu_core.h"

#define IMU_PACK_PHASE_SHIFT          0u
#define IMU_PACK_PHASE_MASK           (0x07u << IMU_PACK_PHASE_SHIFT)
#define IMU_PACK_MOTION_SHIFT         3u
#define IMU_PACK_MOTION_MASK          (0x07u << IMU_PACK_MOTION_SHIFT)
#define IMU_PACK_HEADING_VALID_SHIFT  6u
#define IMU_PACK_HEIGHT_VALID_SHIFT   7u
#define IMU_PACK_HIGHEST_VALID_SHIFT  8u
#define IMU_PACK_PROVISIONAL_SHIFT    9u
#define IMU_PACK_EVENTS_SHIFT         10u
#define IMU_PACK_EVENTS_MASK          (0x01FFu << IMU_PACK_EVENTS_SHIFT)
#define IMU_PACK_REASONS_SHIFT        19u
#define IMU_PACK_REASONS_MASK         (0x00FFu << IMU_PACK_REASONS_SHIFT)
#define IMU_PACK_USED_MASK            0x07FFFFFFu
#define IMU_PACKED_STATUS_BYTES       32u
#define IMU_STATUS_PRE_TRIBUNAL_HOST_BYTES 52u

typedef struct
{
    uint32_t  time_ms;
    float32_t pitch_deg;
    float32_t roll_deg;
    float32_t heading_deg;
    float32_t turn_dps;
    float32_t hump_peak_mm;
    float32_t highest_hump_mm;
    uint32_t  fields;
} imu_packed_status_t;

bool imu_packed_encode (const imu_core_status_t *p_status,
                        imu_packed_status_t *p_packed);
bool imu_packed_decode (const imu_packed_status_t *p_packed,
                        imu_core_status_t *p_status);

#endif /* IMU_PACKED_H */
