/**
 * @file    lsm303dlhc_regs.h
 * @brief   ST LSM303DLHC register map and bit definitions.
 *
 * Source: ST LSM303DLHC datasheet (DocID018771), cross-checked against the
 * Adafruit_LSM303DLHC and Pololu lsm303-arduino reference drivers.
 *
 * Notes that matter for a correct driver:
 *   - Accelerometer and magnetometer are two separate I2C slaves.
 *   - Accelerometer outputs are little-endian (with CTRL_REG4_A.BLE = 0),
 *     12-bit left-justified in high-resolution mode (shift right by 4).
 *   - Accelerometer multi-byte reads need bit 7 of the sub-address set
 *     (auto-increment).
 *   - Magnetometer outputs are BIG-endian and ordered X, Z, Y (not X, Y, Z).
 *     The magnetometer address pointer auto-increments without a flag.
 *   - A magnetometer output of -4096 (0xF000) indicates ADC overflow.
 *   - The accelerometer of the DLHC has NO documented WHO_AM_I register;
 *     identity is checked through the magnetometer identification registers
 *     IRA/IRB/IRC ('H', '4', '3') plus a configuration read-back.
 */

#ifndef LSM303DLHC_REGS_H
#define LSM303DLHC_REGS_H

/* -------------------- I2C 7-bit slave addresses ----------------------- */
#define LSM303_ACCEL_ADDR            (0x19U)   /* 0011001b */
#define LSM303_MAG_ADDR              (0x1EU)   /* 0011110b */

/* ---------------------- Accelerometer registers ----------------------- */
#define LSM303_REG_CTRL_REG1_A       (0x20U)
#define LSM303_REG_CTRL_REG2_A       (0x21U)
#define LSM303_REG_CTRL_REG3_A       (0x22U)
#define LSM303_REG_CTRL_REG4_A       (0x23U)
#define LSM303_REG_CTRL_REG5_A       (0x24U)
#define LSM303_REG_CTRL_REG6_A       (0x25U)
#define LSM303_REG_STATUS_REG_A      (0x27U)
#define LSM303_REG_OUT_X_L_A         (0x28U)
#define LSM303_REG_FIFO_CTRL_REG_A   (0x2EU)
#define LSM303_REG_FIFO_SRC_REG_A    (0x2FU)

/** Sub-address MSB = auto-increment for accelerometer multi-byte access. */
#define LSM303_ACCEL_AUTO_INC        (0x80U)

/* CTRL_REG1_A: ODR[7:4] LPen[3] Zen[2] Yen[1] Xen[0] */
#define LSM303_CTRL1_ODR_SHIFT       (4U)
#define LSM303_CTRL1_XYZ_EN          (0x07U)

/** Accelerometer output data-rate codes (normal / high-resolution mode). */
#define LSM303_ACCEL_ODR_1HZ         (0x01U)
#define LSM303_ACCEL_ODR_10HZ        (0x02U)
#define LSM303_ACCEL_ODR_25HZ        (0x03U)
#define LSM303_ACCEL_ODR_50HZ        (0x04U)
#define LSM303_ACCEL_ODR_100HZ       (0x05U)
#define LSM303_ACCEL_ODR_200HZ       (0x06U)
#define LSM303_ACCEL_ODR_400HZ       (0x07U)

/* CTRL_REG4_A: BDU[7] BLE[6] FS[5:4] HR[3] */
#define LSM303_CTRL4_BDU             (0x80U)
#define LSM303_CTRL4_FS_SHIFT        (4U)
#define LSM303_CTRL4_HR              (0x08U)

/** Accelerometer full-scale codes. */
#define LSM303_ACCEL_FS_2G           (0x00U)
#define LSM303_ACCEL_FS_4G           (0x01U)
#define LSM303_ACCEL_FS_8G           (0x02U)
#define LSM303_ACCEL_FS_16G          (0x03U)

/* CTRL_REG5_A: BOOT[7] FIFO_EN[6] */
#define LSM303_CTRL5_FIFO_EN         (0x40U)

/* STATUS_REG_A */
#define LSM303_STATUS_ZYXDA          (0x08U)
#define LSM303_STATUS_ZYXOR          (0x80U)

/* FIFO_CTRL_REG_A: FM[7:6] */
#define LSM303_FIFO_MODE_BYPASS      (0x00U)
#define LSM303_FIFO_MODE_STREAM      (0x80U)

/* FIFO_SRC_REG_A: WTM[7] OVRN[6] EMPTY[5] FSS[4:0] */
#define LSM303_FIFO_SRC_OVRN         (0x40U)
#define LSM303_FIFO_SRC_EMPTY        (0x20U)
#define LSM303_FIFO_SRC_FSS_MASK     (0x1FU)
#define LSM303_FIFO_DEPTH            (32U)

/* ----------------------- Magnetometer registers ----------------------- */
#define LSM303_REG_CRA_REG_M         (0x00U)
#define LSM303_REG_CRB_REG_M         (0x01U)
#define LSM303_REG_MR_REG_M          (0x02U)
/* Output order: X_H X_L Z_H Z_L Y_H Y_L (big-endian, X/Z/Y!) */
#define LSM303_REG_OUT_X_H_M         (0x03U)
#define LSM303_REG_SR_REG_M          (0x09U)
#define LSM303_REG_IRA_REG_M         (0x0AU)

#define LSM303_IRA_VALUE             (0x48U)   /* 'H' */
#define LSM303_IRB_VALUE             (0x34U)   /* '4' */
#define LSM303_IRC_VALUE             (0x33U)   /* '3' */

/* CRA_REG_M: TEMP_EN[7] DO[4:2] */
#define LSM303_CRA_DO_SHIFT          (2U)
#define LSM303_MAG_ODR_15HZ          (0x04U)
#define LSM303_MAG_ODR_30HZ          (0x05U)
#define LSM303_MAG_ODR_75HZ          (0x06U)
#define LSM303_MAG_ODR_220HZ         (0x07U)

/* CRB_REG_M: GN[7:5] */
#define LSM303_CRB_GN_SHIFT          (5U)
#define LSM303_MAG_GAIN_1_3          (0x01U)   /* +/-1.3 gauss */
#define LSM303_MAG_GAIN_1_9          (0x02U)   /* +/-1.9 gauss */
#define LSM303_MAG_GAIN_2_5          (0x03U)
#define LSM303_MAG_GAIN_4_0          (0x04U)
#define LSM303_MAG_GAIN_4_7          (0x05U)
#define LSM303_MAG_GAIN_5_6          (0x06U)
#define LSM303_MAG_GAIN_8_1          (0x07U)

/* MR_REG_M: MD[1:0] */
#define LSM303_MR_CONTINUOUS         (0x00U)
#define LSM303_MR_SLEEP              (0x03U)

/* SR_REG_M */
#define LSM303_SR_DRDY               (0x01U)

/** Magnetometer ADC overflow marker. */
#define LSM303_MAG_OVERFLOW_VALUE    (-4096)

#endif /* LSM303DLHC_REGS_H */
