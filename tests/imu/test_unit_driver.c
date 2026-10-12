/**
 * @file    test_unit_driver.c
 * @brief   White-box unit tests: LSM303DLHC driver against the register-level
 *          simulator (decoding, configuration, FIFO, errors, retries,
 *          recovery).
 */

#include <string.h>

#include "imu/imu_config.h"
#include "imu/imu_lsm303dlhc.h"
#include "imu/lsm303dlhc_regs.h"
#include "test_util.h"

static lsm303_config_t drv_cfg(void)
{
    lsm303_config_t c;

    c.accel_odr_code  = LSM303_ACCEL_ODR_200HZ;
    c.accel_fs_code   = LSM303_ACCEL_FS_4G;
    c.b_accel_fifo    = true;
    c.mag_odr_code    = LSM303_MAG_ODR_75HZ;
    c.mag_gain_code   = LSM303_MAG_GAIN_1_9;
    c.b_mag_id_check  = true;
    c.i2c_baud_hz     = 400000U;
    c.i2c_timeout_us  = 2000U;
    c.i2c_max_retries = 2U;
    return (c);
}

static void t_decode_accel(void)
{
    TEST_ASSERT_EQ(lsm303_decode_accel_axis(0x00U, 0x00U), 0);
    TEST_ASSERT_EQ(lsm303_decode_accel_axis(0x10U, 0x00U), 1);
    TEST_ASSERT_EQ(lsm303_decode_accel_axis(0xF0U, 0x7FU), 2047);
    TEST_ASSERT_EQ(lsm303_decode_accel_axis(0x00U, 0x80U), -2048);
    TEST_ASSERT_EQ(lsm303_decode_accel_axis(0xF0U, 0xFFU), -1);
    TEST_ASSERT_EQ(lsm303_decode_accel_axis(0x0FU, 0x00U), 0); /* LSBs */
    TEST_ASSERT_EQ(lsm303_decode_accel_axis(0xE0U, 0xFFU), -2);
}

static void t_decode_mag(void)
{
    TEST_ASSERT_EQ(lsm303_decode_mag_axis(0x01U, 0x02U), 258);
    TEST_ASSERT_EQ(lsm303_decode_mag_axis(0xF0U, 0x00U), -4096);
    TEST_ASSERT_EQ(lsm303_decode_mag_axis(0xFFU, 0xFFU), -1);
    TEST_ASSERT_EQ(lsm303_decode_mag_axis(0x07U, 0xFFU), 2047);
}

static void t_scale_tables(void)
{
    TEST_ASSERT_NEAR(lsm303_accel_sensitivity_g(LSM303_ACCEL_FS_2G), 0.001, 1e-9);
    TEST_ASSERT_NEAR(lsm303_accel_sensitivity_g(LSM303_ACCEL_FS_4G), 0.002, 1e-9);
    TEST_ASSERT_NEAR(lsm303_accel_sensitivity_g(LSM303_ACCEL_FS_8G), 0.004, 1e-9);
    TEST_ASSERT_NEAR(lsm303_accel_sensitivity_g(LSM303_ACCEL_FS_16G), 0.012, 1e-9);
    TEST_ASSERT_NEAR(lsm303_accel_sensitivity_g(9U), 0.0, 1e-9);
    TEST_ASSERT_NEAR(lsm303_mag_lsb_per_gauss(LSM303_MAG_GAIN_1_3, false), 1100.0, 1e-3);
    TEST_ASSERT_NEAR(lsm303_mag_lsb_per_gauss(LSM303_MAG_GAIN_1_3, true), 980.0, 1e-3);
    TEST_ASSERT_NEAR(lsm303_mag_lsb_per_gauss(LSM303_MAG_GAIN_8_1, false), 230.0, 1e-3);
    TEST_ASSERT_NEAR(lsm303_mag_lsb_per_gauss(LSM303_MAG_GAIN_8_1, true), 205.0, 1e-3);
    TEST_ASSERT_NEAR(lsm303_mag_lsb_per_gauss(0U, false), 0.0, 1e-9);
    TEST_ASSERT_NEAR(lsm303_mag_lsb_per_gauss(8U, false), 0.0, 1e-9);
}

static void t_init_registers(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();

    sim_reset();
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    TEST_ASSERT(dev.b_ready);
    TEST_ASSERT_EQ(sim_accel_reg(LSM303_REG_CTRL_REG1_A), 0x67);
    TEST_ASSERT_EQ(sim_accel_reg(LSM303_REG_CTRL_REG4_A), 0x98);
    TEST_ASSERT_EQ(sim_accel_reg(LSM303_REG_CTRL_REG5_A), 0x40);
    TEST_ASSERT_EQ(sim_accel_reg(LSM303_REG_FIFO_CTRL_REG_A), 0x80);
    TEST_ASSERT_EQ(sim_mag_reg(LSM303_REG_CRA_REG_M), 0x18);
    TEST_ASSERT_EQ(sim_mag_reg(LSM303_REG_CRB_REG_M), 0x40);
    TEST_ASSERT_EQ(sim_mag_reg(LSM303_REG_MR_REG_M), 0x00);
    TEST_ASSERT_NEAR(dev.accel_full_scale_g, 4.0, 1e-6);
}

static void t_init_failures(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();

    sim_reset();
    sim_cfg()->b_accel_absent = true;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_ERR_I2C);
    TEST_ASSERT(!dev.b_ready);

    sim_reset();
    sim_cfg()->b_mag_absent = true;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_ERR_I2C);

    sim_reset();
    sim_cfg()->b_wrong_id = true;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_ERR_ID);

    sim_reset();
    sim_cfg()->b_wrong_id = true;
    c.b_mag_id_check = false;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    c.b_mag_id_check = true;

    sim_reset();
    sim_cfg()->b_ignore_writes = true;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_ERR_VERIFY);

    sim_reset();
    sim_cfg()->b_bus_stuck = true;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_ERR_TIMEOUT);

    TEST_ASSERT_EQ(lsm303_init(NULL, &c), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(lsm303_init(&dev, NULL), IMU_ERR_PARAM);
    c.accel_fs_code = 7U;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_ERR_PARAM);
    c = drv_cfg();
    c.mag_gain_code = 0U;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_ERR_PARAM);
    c = drv_cfg();
    c.accel_odr_code = 0U;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_ERR_PARAM);
}

static void t_fifo_read(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      buf[32];
    uint8_t         n = 0U;
    bool            b_ovr = true;
    imu_vec3_t      g;

    sim_reset();
    sim_cfg()->const_accel.x = 0.25f;
    sim_cfg()->const_accel.y = -0.5f;
    sim_cfg()->const_accel.z = 1.0f;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    sim_advance_ms(20U);
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 32U, &n, &b_ovr), IMU_OK);
    TEST_ASSERT(n >= 4U && n <= 5U);
    TEST_ASSERT(!b_ovr);
    lsm303_accel_to_g(&dev, &buf[0], &g);
    TEST_ASSERT_NEAR(g.x, 0.25, 0.002);
    TEST_ASSERT_NEAR(g.y, -0.5, 0.002);
    TEST_ASSERT_NEAR(g.z, 1.0, 0.002);
    TEST_ASSERT_EQ(buf[0].z, 500);

    /* Nothing new without time passing. */
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 32U, &n, &b_ovr), IMU_OK);
    TEST_ASSERT_EQ(n, 0);

    /* Bounded by the caller's buffer: the rest stays queued. */
    sim_advance_ms(40U);
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 3U, &n, &b_ovr), IMU_OK);
    TEST_ASSERT_EQ(n, 3);
    TEST_ASSERT(sim_fifo_level() >= 5U);
}

static void t_fifo_overrun(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      buf[32];
    uint8_t         n = 0U;
    bool            b_ovr = false;

    sim_reset();
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    sim_advance_ms(300U);   /* 60 samples > 32 */
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 32U, &n, &b_ovr), IMU_OK);
    TEST_ASSERT_EQ(n, 32);
    TEST_ASSERT(b_ovr);
    TEST_ASSERT_EQ(dev.stats.fifo_overruns, 1);
}

static void t_single_mode(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      buf[4];
    uint8_t         n = 0U;
    bool            b_ovr = false;

    c.b_accel_fifo = false;
    sim_reset();
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    TEST_ASSERT_EQ(sim_accel_reg(LSM303_REG_CTRL_REG5_A), 0x00);
    sim_advance_ms(20U);
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 4U, &n, &b_ovr), IMU_OK);
    TEST_ASSERT_EQ(n, 1);
    TEST_ASSERT(b_ovr);           /* several samples were overwritten */
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 4U, &n, &b_ovr), IMU_OK);
    TEST_ASSERT_EQ(n, 0);
}

static void t_mag_order_and_scale(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      raw;
    imu_vec3_t      m;
    bool            b_ovf = true;

    sim_reset();
    sim_cfg()->const_mag.x = 0.10f;
    sim_cfg()->const_mag.y = -0.20f;
    sim_cfg()->const_mag.z = 0.30f;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, &raw, &b_ovf), IMU_OK);
    TEST_ASSERT(!b_ovf);
    lsm303_mag_to_gauss(&dev, &raw, &m);
    TEST_ASSERT_NEAR(m.x, 0.10, 0.002);
    TEST_ASSERT_NEAR(m.y, -0.20, 0.002);
    TEST_ASSERT_NEAR(m.z, 0.30, 0.002);
    TEST_ASSERT_EQ(raw.z, 228);   /* 0.30 G * 760 LSB/G (Z gain differs) */
}

static void t_mag_overflow(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      raw;
    bool            b_ovf = false;

    sim_reset();
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    sim_cfg()->b_mag_overflow = true;
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, &raw, &b_ovf), IMU_OK);
    TEST_ASSERT(b_ovf);
    TEST_ASSERT_EQ(dev.stats.mag_overflows, 1);

    /* A field above the selected range also produces the marker. */
    sim_cfg()->b_mag_overflow = false;
    sim_cfg()->const_mag.x = 3.0f;
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, &raw, &b_ovf), IMU_OK);
    TEST_ASSERT(b_ovf);
}

static void t_retries(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      raw;
    bool            b_ovf;

    sim_reset();
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);

    sim_cfg()->fail_next = 2U;   /* = max retries: still succeeds */
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, &raw, &b_ovf), IMU_OK);
    TEST_ASSERT_EQ(dev.stats.retries, 2);
    TEST_ASSERT_EQ(dev.stats.failed_transfers, 0);
    TEST_ASSERT_EQ(dev.stats.nacks, 2);

    sim_cfg()->fail_next   = 3U;  /* exceeds retries: reported */
    sim_cfg()->fail_status = IMU_HAL_TIMEOUT;
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, &raw, &b_ovf), IMU_ERR_TIMEOUT);
    TEST_ASSERT_EQ(dev.stats.failed_transfers, 1);
    TEST_ASSERT_EQ(dev.stats.timeouts, 3);

    /* Retries are bounded: exactly 1 + max_retries attempts. */
    TEST_ASSERT_EQ(dev.stats.retries, 4);
}

static void t_recover(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      raw;
    bool            b_ovf;

    sim_reset();
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    sim_cfg()->b_bus_stuck = true;
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, &raw, &b_ovf), IMU_ERR_TIMEOUT);
    TEST_ASSERT_EQ(lsm303_recover(&dev), IMU_OK);
    TEST_ASSERT_EQ(sim_cfg()->recover_calls, 1);
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, &raw, &b_ovf), IMU_OK);
    TEST_ASSERT_EQ(dev.stats.recoveries, 1);

    sim_cfg()->b_bus_stuck = true;
    sim_cfg()->b_stuck_survives_recovery = true;
    TEST_ASSERT(IMU_OK != lsm303_recover(&dev));
    TEST_ASSERT(!dev.b_ready);
    TEST_ASSERT_EQ(lsm303_recover(NULL), IMU_ERR_PARAM);
}

static void t_not_ready_and_null(void)
{
    lsm303_t   dev;
    imu_raw3_t buf[2];
    uint8_t    n;
    bool       b;

    (void)memset(&dev, 0, sizeof(dev));
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 2U, &n, &b), IMU_ERR_NOT_READY);
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, buf, &b), IMU_ERR_NOT_READY);
    TEST_ASSERT_EQ(lsm303_read_accel(NULL, buf, 2U, &n, &b), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, NULL, 2U, &n, &b), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 0U, &n, &b), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(lsm303_read_mag(&dev, NULL, &b), IMU_ERR_PARAM);
    lsm303_accel_to_g(NULL, NULL, NULL);      /* must not crash */
    lsm303_mag_to_gauss(NULL, NULL, NULL);
    lsm303_shutdown(NULL);
}

static void t_signed_and_clipping(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      buf[8];
    uint8_t         n;
    bool            b;
    imu_vec3_t      g;

    sim_reset();
    sim_cfg()->const_accel.x = -1.5f;
    sim_cfg()->const_accel.y = 5.0f;     /* beyond +/-4 g */
    sim_cfg()->const_accel.z = -0.25f;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    sim_advance_ms(10U);
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 8U, &n, &b), IMU_OK);
    TEST_ASSERT(n > 0U);
    lsm303_accel_to_g(&dev, &buf[0], &g);
    TEST_ASSERT_NEAR(g.x, -1.5, 0.002);
    TEST_ASSERT_NEAR(g.y, 4.094, 0.002);   /* saturates at +2047 LSB */
    TEST_ASSERT_NEAR(g.z, -0.25, 0.002);
}

static void t_shutdown(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();

    sim_reset();
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    lsm303_shutdown(&dev);
    TEST_ASSERT_EQ(sim_mag_reg(LSM303_REG_MR_REG_M), LSM303_MR_SLEEP);
    TEST_ASSERT_EQ(sim_accel_reg(LSM303_REG_CTRL_REG1_A), 0);
    TEST_ASSERT(!dev.b_ready);
}

static void t_other_fs_and_odr(void)
{
    lsm303_t        dev;
    lsm303_config_t c = drv_cfg();
    imu_raw3_t      buf[32];
    uint8_t         n;
    bool            b;
    imu_vec3_t      g;

    sim_reset();
    c.accel_fs_code  = LSM303_ACCEL_FS_16G;
    c.accel_odr_code = LSM303_ACCEL_ODR_100HZ;
    c.mag_gain_code  = LSM303_MAG_GAIN_8_1;
    sim_cfg()->const_accel.z = 9.0f;
    TEST_ASSERT_EQ(lsm303_init(&dev, &c), IMU_OK);
    sim_advance_ms(100U);
    TEST_ASSERT_EQ(lsm303_read_accel(&dev, buf, 32U, &n, &b), IMU_OK);
    TEST_ASSERT(n >= 10U && n <= 11U);
    lsm303_accel_to_g(&dev, &buf[0], &g);
    TEST_ASSERT_NEAR(g.z, 9.0, 0.012);
}

void suite_unit_driver(void)
{
    test_section("UNIT (white-box): LSM303DLHC driver");
    RUN_TEST(t_decode_accel);
    RUN_TEST(t_decode_mag);
    RUN_TEST(t_scale_tables);
    RUN_TEST(t_init_registers);
    RUN_TEST(t_init_failures);
    RUN_TEST(t_fifo_read);
    RUN_TEST(t_fifo_overrun);
    RUN_TEST(t_single_mode);
    RUN_TEST(t_mag_order_and_scale);
    RUN_TEST(t_mag_overflow);
    RUN_TEST(t_retries);
    RUN_TEST(t_recover);
    RUN_TEST(t_not_ready_and_null);
    RUN_TEST(t_signed_and_clipping);
    RUN_TEST(t_shutdown);
    RUN_TEST(t_other_fs_and_odr);
}
