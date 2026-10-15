/** @file imu_runtime.h
 * Buddy 4 provider API; shared controller currently reads status/fault.
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
/** Controller assertion that wheel traction is suitable for path integration.
 * False revokes any previous assertion, including between IMU samples.
 * Default is false. No current controller code asserts true. Caller must
 * revoke on slip, unavailable feedback or mode change. Equal wheel speeds
 * alone do not prove traction. S4 checks signed Buddy 2 status independently.
 */
ER imu_runtime_set_motion_trust (bool b_trusted);
/** Deprecated scalar input: insufficient to prove straight, nonslip motion.
 * b_valid=false revokes trust; b_valid=true returns E_PAR and revokes trust.
 */
ER imu_runtime_set_speed (float32_t speed_mm_s, uint32_t time_ms,
                          bool b_valid);

#endif /* IMU_RUNTIME_H */

