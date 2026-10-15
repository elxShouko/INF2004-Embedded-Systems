/** @file imu_link.h Read-only adapter to Buddy 2's provisional motion API. */
#ifndef IMU_LINK_H
#define IMU_LINK_H

#include "imu_core.h"

/** Populate copied motion evidence; failure clears validity, never holds it.
 * Uses the producer's clock for age and derivatives, not the Pico epoch.
 * No commands, motor writes or calibration assumptions are made here.
 */
void imu_link_read (imu_core_sample_t * p_sample, uint32_t previous_ms,
                    float32_t wheel_limit_mm_s);

#endif /* IMU_LINK_H */

/* End of file. */

