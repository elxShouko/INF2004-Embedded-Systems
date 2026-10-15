/** @file drv_imu.h
 * Pico SDK transport. Only imu_runtime_task may call open/read.
 * The read-only timestamp helper is available to speed-input producers.
 */
#ifndef DRV_IMU_H
#define DRV_IMU_H

#include "drv_lsm.h"

bool drv_imu_open (void);
drv_lsm_result_t drv_imu_read (imu_core_sample_t *p_sample);
uint32_t drv_imu_time_ms (void);

#endif /* DRV_IMU_H */

