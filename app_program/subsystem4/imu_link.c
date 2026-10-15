/** @file imu_link.c
 * Signed mean wheel speed -> approximate path increment over the IMU period.
 * Buddy 2 status is a coherent polling copy. Its kernel operating-time clock
 * is not the hardware timer epoch: compute age at source, add read latency.
 */
#include "imu_link.h"
#include "drv_imu.h"
#include "../subsystem2/motion_api.h"

#include <math.h>
#include <stddef.h>

/** Read without an S4 lock. Missing or stale feedback is invalid. */
void imu_link_read (imu_core_sample_t * p_sample, uint32_t previous_ms,
                    float32_t wheel_limit_mm_s)
{
    motion_run_status_t status = {0};
    uint64_t source_now_us = 0u;
    uint64_t age_us = 0u;
    uint32_t latency_ms = 0u;
    uint32_t elapsed_ms = 0u;
    const uint32_t started_ms = drv_imu_time_ms();

    if (NULL == p_sample)
    {
        return;
    }
    p_sample->b_motion_valid = false;
    p_sample->b_distance_valid = false;
    p_sample->b_straight = false;
    p_sample->speed_mm_s = 0.0f;
    p_sample->distance_mm = 0.0f;
    if ((MOTION_API_QUEUED != motion_api_status(&status)) ||
        (!motion_api_now(&source_now_us)) ||
        (!status.b_measurement_valid) ||
        (MOTION_RUN_ACTIVE != status.state) ||
        (MOTION_RUN_NONE != status.fault_result) ||
        (source_now_us < status.measurement_timestamp_us) ||
        (source_now_us < status.timestamp_us))
    {
        return;
    }
    age_us = source_now_us - status.measurement_timestamp_us;
    latency_ms = drv_imu_time_ms() - started_ms;
    elapsed_ms = p_sample->time_ms - previous_ms;
    if ((age_us > 40000u) || (age_us > status.measurement_max_age_us) ||
        ((source_now_us - status.timestamp_us) > 40000u) ||
        (latency_ms > 10u) || (elapsed_ms > 100u) || (0u == elapsed_ms) ||
        (status.speed_mm_s[0] < -1000) || (status.speed_mm_s[0] > 1000) ||
        (status.speed_mm_s[1] < -1000) || (status.speed_mm_s[1] > 1000))
    {
        return;
    }
    /* Age <=40000 us; ceil conversion is bounded. Source time wraps modulo. */
    p_sample->motion_age_ms = (uint32_t)((age_us + 999u) / 1000u) +
                              latency_ms;
    p_sample->motion_time_ms =
        (uint32_t)(status.measurement_timestamp_us / 1000u);
    /* Bounded signed wheel values are exactly representable as float32_t. */
    p_sample->speed_mm_s = ((float32_t)status.speed_mm_s[0] +
                           (float32_t)status.speed_mm_s[1]) * 0.5f;
    p_sample->wheel_delta_mm_s = fabsf((float32_t)status.speed_mm_s[0] -
                                      (float32_t)status.speed_mm_s[1]);
    /* Widen before subtraction: targets can span the signed 32-bit domain. */
    p_sample->b_straight =
        ((MOVE_PLAN_SPEED == status.active_kind) ||
         (MOVE_PLAN_TRAVEL == status.active_kind)) &&
        (status.speed_mm_s[0] > 0) && (status.speed_mm_s[1] > 0) &&
        (status.target_mm_s[0] > 0) && (status.target_mm_s[1] > 0) &&
        (p_sample->wheel_delta_mm_s <= wheel_limit_mm_s) &&
        (fabsf((float32_t)((int64_t)status.target_mm_s[0] -
                           (int64_t)status.target_mm_s[1])) <=
         wheel_limit_mm_s);
    p_sample->b_motion_valid =
        (p_sample->motion_age_ms <= IMU_MOTION_MAX_AGE_MS);
    p_sample->b_distance_valid = p_sample->b_motion_valid &&
                                (p_sample->speed_mm_s >= 0.0f);
    if (p_sample->b_distance_valid)
    {
        /* Elapsed <=100 ms is exact in float. Never take abs(reverse). */
        p_sample->distance_mm = p_sample->speed_mm_s *
                                (float32_t)elapsed_ms * 0.001f;
    }
}

/* End of file. */

