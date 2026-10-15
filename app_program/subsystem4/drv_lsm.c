/** @file drv_lsm.c
 * ST LSM303DLHC DocID018771 Rev 2, sections 5-7 and table 3.
 * 100 Hz, high resolution, +/-2 g, BDU; magnetic 75 Hz, +/-1.3 gauss.
 * These rates/ranges are development selections, not hardware validation.
 */
#include "drv_lsm.h"

#include <stddef.h>

#define LSM_CTRL1_A 0x20u
#define LSM_CTRL4_A 0x23u
#define LSM_STATUS_A 0x27u
#define LSM_OUTPUT_A 0xA8u
#define LSM_STATUS_M 0x09u
#define LSM_OUTPUT_M 0x03u
#define LSM_ID_M 0x0Au
#define LSM_RAW_BYTES 6u

static int32_t decode_word (uint8_t low, uint8_t high);
static bool write_checked (const drv_lsm_bus_t *p_bus, uint8_t address,
                           uint8_t reg, uint8_t value);
static drv_lsm_result_t read_magnetic (const drv_lsm_bus_t *p_bus,
                                      imu_core_sample_t *p_sample);

bool drv_lsm_init (const drv_lsm_bus_t *p_bus)
{
    uint8_t identity[3] = {0u};

    if ((NULL == p_bus) || (NULL == p_bus->p_read) ||
        (NULL == p_bus->p_write))
    {
        return false;
    }
    if (!p_bus->p_read(DRV_LSM_MAG_ADDR, LSM_ID_M, identity, 3u))
    {
        return false;
    }
    if ((0x48u != identity[0]) || (0x34u != identity[1]) ||
        (0x33u != identity[2]))
    {
        return false;
    }
    /* No accelerometer WHO_AM_I is specified for this exact part. */
    return write_checked(p_bus, DRV_LSM_ACCEL_ADDR, LSM_CTRL1_A, 0x00u) &&
           write_checked(p_bus, DRV_LSM_ACCEL_ADDR, 0x21u, 0x00u) &&
           write_checked(p_bus, DRV_LSM_ACCEL_ADDR, LSM_CTRL4_A, 0x88u) &&
           write_checked(p_bus, DRV_LSM_ACCEL_ADDR, 0x24u, 0x00u) &&
           write_checked(p_bus, DRV_LSM_ACCEL_ADDR, LSM_CTRL1_A, 0x57u) &&
           write_checked(p_bus, DRV_LSM_MAG_ADDR, 0x00u, 0x18u) &&
           write_checked(p_bus, DRV_LSM_MAG_ADDR, 0x01u, 0x20u) &&
           write_checked(p_bus, DRV_LSM_MAG_ADDR, 0x02u, 0x00u);
}

drv_lsm_result_t drv_lsm_read (const drv_lsm_bus_t *p_bus,
                              imu_core_sample_t *p_sample)
{
    imu_core_sample_t sample = {0};
    drv_lsm_result_t  result = DRV_LSM_INVALID;
    uint8_t           status = 0u;
    uint8_t           raw[LSM_RAW_BYTES] = {0u};

    if ((NULL == p_bus) || (NULL == p_bus->p_read) || (NULL == p_sample))
    {
        return DRV_LSM_INVALID;
    }
    if (!p_bus->p_read(DRV_LSM_ACCEL_ADDR, LSM_STATUS_A, &status, 1u))
    {
        return DRV_LSM_IO_ERROR;
    }
    if (0u == (status & 0x08u))
    {
        return DRV_LSM_NOT_READY;
    }
    /* 100 Hz sensor/50 Hz poll intentionally drops intermediate samples.
     * ZYXOR is therefore expected. BDU protects the current axis words. */
    if (!p_bus->p_read(DRV_LSM_ACCEL_ADDR, LSM_OUTPUT_A, raw,
                       LSM_RAW_BYTES))
    {
        return DRV_LSM_IO_ERROR;
    }
    for (uint32_t axis = 0u; axis < 3u; ++axis)
    {
        const int32_t counts = decode_word(raw[axis * 2u],
                                          raw[(axis * 2u) + 1u]) / 16;

        if ((counts <= -2000) || (counts >= 2000))
        {
            return DRV_LSM_INVALID;
        }
        /* HR +/-2 g: 1 mg/LSB; signed 12-bit values fit float exactly. */
        sample.accel_g[axis] = (float32_t)counts * 0.001f;
    }
    result = read_magnetic(p_bus, &sample);
    if (DRV_LSM_OK == result)
    {
        *p_sample = sample;
    }
    return result;
}

static int32_t decode_word (uint8_t low, uint8_t high)
{
    const uint32_t word = ((uint32_t)high << 8u) | (uint32_t)low;
    int32_t        value = (int32_t)word; /* 0..65535 fits int32_t. */

    if (word >= 0x8000u)
    {
        value -= 65536;
    }
    return value;
}

static bool write_checked (const drv_lsm_bus_t *p_bus, uint8_t address,
                           uint8_t reg, uint8_t value)
{
    uint8_t observed = 0u;

    return p_bus->p_write(address, reg, value) &&
           p_bus->p_read(address, reg, &observed, 1u) &&
           (value == observed);
}

static drv_lsm_result_t read_magnetic (const drv_lsm_bus_t *p_bus,
                                      imu_core_sample_t *p_sample)
{
    const uint32_t order[3] = {0u, 4u, 2u}; /* Register order X,Z,Y. */
    uint8_t        raw[LSM_RAW_BYTES] = {0u};
    uint8_t        status = 0u;

    if (!p_bus->p_read(DRV_LSM_MAG_ADDR, LSM_STATUS_M, &status, 1u))
    {
        return DRV_LSM_IO_ERROR;
    }
    if (0u == (status & 0x01u))
    {
        return DRV_LSM_OK; /* Acceleration usable; no stale magnetic data. */
    }
    if (!p_bus->p_read(DRV_LSM_MAG_ADDR, LSM_OUTPUT_M, raw, LSM_RAW_BYTES))
    {
        return DRV_LSM_IO_ERROR;
    }
    for (uint32_t axis = 0u; axis < 3u; ++axis)
    {
        const uint32_t offset = order[axis];
        const int32_t counts = decode_word(raw[offset + 1u], raw[offset]);
        const float32_t sensitivity = (2u == axis) ? 980.0f : 1100.0f;

        if ((counts < -2048) || (counts > 2047))
        {
            return DRV_LSM_OK; /* Overflow/invalid heading, not stale yaw. */
        }
        /* Counts <= 2048 in magnitude, exactly representable. */
        p_sample->mag_gauss[axis] = (float32_t)counts / sensitivity;
    }
    p_sample->b_mag_valid = true;
    return DRV_LSM_OK;
}

/* End of file. */

