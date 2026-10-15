/** @file drv_lsm.h
 * LSM303DLHC register driver. Bus callbacks return false on short/error I/O.
 * One task owns the bus. Output is committed only after a complete sample.
 */
#ifndef DRV_LSM_H
#define DRV_LSM_H

#include "imu_core.h"

#define DRV_LSM_ACCEL_ADDR 0x19u
#define DRV_LSM_MAG_ADDR   0x1Eu

typedef bool (*drv_lsm_read_t) (uint8_t address, uint8_t reg,
                               uint8_t *p_data, uint32_t length);
typedef bool (*drv_lsm_write_t) (uint8_t address, uint8_t reg,
                                uint8_t value);

typedef struct
{
    drv_lsm_read_t  p_read;
    drv_lsm_write_t p_write;
} drv_lsm_bus_t;

typedef enum
{
    DRV_LSM_OK,
    DRV_LSM_NOT_READY,
    DRV_LSM_IO_ERROR,
    DRV_LSM_INVALID
} drv_lsm_result_t;

bool drv_lsm_init (const drv_lsm_bus_t *p_bus);
drv_lsm_result_t drv_lsm_read (const drv_lsm_bus_t *p_bus,
                              imu_core_sample_t *p_sample);

#endif /* DRV_LSM_H */

