/** @file imu_guard.h Gravity-tilt trust from independent motion evidence. */
#ifndef IMU_GUARD_H
#define IMU_GUARD_H

#include "imu_core.h"

/** Check a calibrated envelope; false keeps all height claims disabled. */
bool imu_guard_config (const imu_core_config_t * p_config);
/** Owner-only update; returns rejection bits, no acceleration correction. */
uint32_t imu_guard_check (imu_core_t * p_core,
                          const imu_core_sample_t * p_sample,
                          bool b_gravity_like);

#endif /* IMU_GUARD_H */

/* End of file. */

