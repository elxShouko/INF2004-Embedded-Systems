/** @file imu_runtime.h
 * Proposed Buddy 4 provider API; no mission-controller consumer exists yet.
 * Task context only. No network or motor operations are performed here.
 */
#ifndef IMU_RUNTIME_H
#define IMU_RUNTIME_H

#include "../app.h"
#include "imu_core.h"

typedef struct
{
    imu_core_status_t sensing;
    uint32_t          dropped_events;
    bool              b_enabled;
} imu_runtime_status_t;

ER imu_runtime_init (void);
ER imu_runtime_start (void);
ER imu_runtime_get_status (imu_runtime_status_t *p_status);
/** Copies a queued transition; caller owns the returned value. Nonblocking. */
ER imu_runtime_get_event (imu_core_status_t *p_event);
/** Latched, independent of the telemetry queue; never clears on read.
 * The controller must also monitor sample freshness and API errors.
 * This is NOT a motor-stop implementation. Only reboot clears the latch.
 */
ER imu_runtime_wait_fault (TMO timeout);
/** Proposed input from Buddy 2/controller; clock is drv_imu_time_ms().
 * Speed must be calibrated, forward, <=1000 mm/s, and <=40 ms old.
 * False validity explicitly revokes the previous input.
 */
ER imu_runtime_set_speed (float32_t speed_mm_s, uint32_t time_ms,
                          bool b_valid);

#endif /* IMU_RUNTIME_H */

