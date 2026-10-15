# Buddy 4 — Full Source Listing (Part 4)

Historical snapshot, preserved verbatim below, including obsolete `tests/imu/`
paths. Use [the current test registry](../../testing/subsystem4/TEST_LOG.md)
for the consolidated test layout and commands. This lists the older standalone
implementation, not integrated production code in `app_program/subsystem4/`.
Every Buddy 4 source file, header, test and tool in branch `buddy4-imu`, in dependency order. The CMake additions are shown first.

## `CMakeLists.txt` (appended block only)

```cmake
# ---------------------------------------------------------------------------
# Buddy 4 - IMU-based motion and terrain monitoring
# LSM303DLHC on Robo Pico Grove 4: I2C0, SDA = GP16, SCL = GP17
# ---------------------------------------------------------------------------
# INTERFACE libraries (Pico SDK idiom): sources are compiled into whichever
# executable links them, so no duplicate hardware_* objects are produced.
#
#   buddy4_imu       driver + algorithms + state machine (no RTOS)
#   buddy4_imu_rtos  + the uT-Kernel IMU / Terrain Task wrapper; only defined
#                    when UTKERNEL_INCLUDE_DIRS points at the course
#                    uT-Kernel headers (tk/tkernel.h). The team firmware then
#                    uses:  target_link_libraries(<firmware> buddy4_imu_rtos)
set(UTKERNEL_INCLUDE_DIRS "" CACHE STRING
    "uT-Kernel 3.0 include directories (dirs containing tk/tkernel.h, config.h)")
set(UTKERNEL_DEFINITIONS "" CACHE STRING
    "uT-Kernel target definitions, e.g. _PICO_RP2040_")
option(BUDDY4_RTOS_COMPILE_CHECK
       "Compile imu_rtos.c as an object library (API check without linking)"
       OFF)

add_library(buddy4_imu INTERFACE)
target_sources(buddy4_imu INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_params.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_math.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_hal_pico.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_lsm303dlhc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_calibration.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_processing.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_hump.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_events.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_terrain.c
)
target_include_directories(buddy4_imu INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(buddy4_imu INTERFACE
    pico_stdlib
    hardware_i2c
    hardware_gpio
    hardware_timer
    m
)

if (UTKERNEL_INCLUDE_DIRS)
    add_library(buddy4_imu_rtos INTERFACE)
    target_sources(buddy4_imu_rtos INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_rtos.c)
    target_include_directories(buddy4_imu_rtos INTERFACE
        ${UTKERNEL_INCLUDE_DIRS})
    # The TRON uT-Kernel headers typedef size_t themselves unless told not
    # to; required when they are mixed with newlib / Pico SDK headers.
    target_compile_definitions(buddy4_imu_rtos INTERFACE
        PROHIBIT_DEF_SIZE_T ${UTKERNEL_DEFINITIONS})
    target_link_libraries(buddy4_imu_rtos INTERFACE buddy4_imu)

    if (BUDDY4_RTOS_COMPILE_CHECK)
        add_library(buddy4_imu_rtos_check OBJECT
            ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_rtos.c)
        target_include_directories(buddy4_imu_rtos_check PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/include ${UTKERNEL_INCLUDE_DIRS})
        target_compile_definitions(buddy4_imu_rtos_check PRIVATE
            PROHIBIT_DEF_SIZE_T ${UTKERNEL_DEFINITIONS})
        target_compile_options(buddy4_imu_rtos_check PRIVATE -Wall -Wextra)
    endif()
endif()

# Stand-alone bring-up / calibration logger (own main(), no uT-Kernel).
# Does not touch src/main.c or any other buddy's code.
add_executable(buddy4_imu_bringup
    tools/buddy4_bringup/bringup_main.c
)
target_link_libraries(buddy4_imu_bringup buddy4_imu pico_stdlib)
target_compile_options(buddy4_imu_bringup PRIVATE -Wall -Wextra)
pico_enable_stdio_usb(buddy4_imu_bringup 1)
pico_enable_stdio_uart(buddy4_imu_bringup 0)
pico_add_extra_outputs(buddy4_imu_bringup)

# Host-side unit / black-box / robustness tests: see tests/imu/Makefile
# (they do not use the Pico SDK and are not part of this build).
```

## `include/imu/imu_types.h`

```c
/**
 * @file    imu_types.h
 * @brief   Buddy 4 (IMU / Terrain) - common types shared by all IMU modules.
 *
 * This header is RTOS-independent and hardware-independent so that the
 * algorithmic modules can be compiled and tested on a host PC.
 *
 * Units used throughout the Buddy 4 subsystem:
 *   - acceleration (specific force): g   (1 g = 9.80665 m/s^2)
 *   - magnetic field:                gauss
 *   - angles:                        degrees
 *   - angular rate:                  degrees per second
 *   - time:                          milliseconds since boot (uint32_t,
 *                                    wraps after ~49.7 days; all interval
 *                                    arithmetic uses unsigned subtraction)
 *
 * Vehicle frame (after axis mapping and mounting correction):
 *   +X = forward, +Y = left, +Z = up   (right-handed)
 *   pitch > 0 : nose up
 *   roll  > 0 : left side up (vehicle rolled towards its right)
 *   heading   : compass heading of +X, 0..360 deg, clockwise seen from
 *               above, so a LEFT turn DECREASES heading.
 */

#ifndef IMU_TYPES_H
#define IMU_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/** Error codes returned by Buddy 4 functions (0 = success). */
typedef enum
{
    IMU_OK                =   0,
    IMU_ERR_PARAM         =  -1,  /**< NULL pointer / out-of-range input  */
    IMU_ERR_I2C           =  -2,  /**< I2C NACK or bus error              */
    IMU_ERR_TIMEOUT       =  -3,  /**< I2C transfer timed out             */
    IMU_ERR_ID            =  -4,  /**< Device identity check failed       */
    IMU_ERR_NOT_READY     =  -5,  /**< Module not initialised             */
    IMU_ERR_INVALID_DATA  =  -6,  /**< Implausible sensor data            */
    IMU_ERR_CAL_MOTION    =  -7,  /**< Calibration: vehicle not still     */
    IMU_ERR_CAL_RANGE     =  -8,  /**< Calibration: result out of range   */
    IMU_ERR_CAL_SAMPLES   =  -9,  /**< Calibration: too few valid samples */
    IMU_ERR_BUSY          = -10,  /**< Shared data temporarily locked     */
    IMU_ERR_STATE         = -11,  /**< Request not allowed in this state  */
    IMU_ERR_QUEUE_FULL    = -12,  /**< Bounded queue full                 */
    IMU_ERR_NO_DATA       = -13,  /**< No new sample available            */
    IMU_ERR_CAL_COVERAGE  = -14,  /**< Mag calibration: rotation too small*/
    IMU_ERR_RTOS          = -15,  /**< RTOS object creation / call failed */
    IMU_ERR_VERIFY        = -16   /**< Register read-back mismatch       */
} imu_err_t;

/** Floating-point 3-axis vector. */
typedef struct
{
    float x;
    float y;
    float z;
} imu_vec3_t;

/** Raw signed 3-axis sensor sample (sensor frame, sensor counts). */
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} imu_raw3_t;

/** Confidence attached to measurements that are partly inferred. */
typedef enum
{
    IMU_CONF_NONE = 0,   /**< Not available                             */
    IMU_CONF_LOW,        /**< Inferred / uncalibrated / degraded        */
    IMU_CONF_MEDIUM,     /**< Measured, but with documented limitations */
    IMU_CONF_HIGH        /**< Measured and cross-checked                */
} imu_confidence_t;

/** Severity attached to events. */
typedef enum
{
    IMU_SEV_INFO = 0,
    IMU_SEV_MINOR,
    IMU_SEV_CRITICAL
} imu_severity_t;

/**
 * Motion classes. STATIONARY / STEADY_MOTION / ACCELERATING / TURNING_* /
 * CLIMBING_HUMP / DESCENDING_HUMP / SUDDEN_IMPACT come from the briefing.
 * Added (justified in docs Part 3):
 *   UNKNOWN       - no valid data (start-up, fault)
 *   STEADY_MOTION - no significant acceleration and vibration present:
 *                   an accelerometer cannot tell constant velocity from
 *                   standing still with motors running, so this class is
 *                   used instead of guessing STATIONARY.
 *   DECELERATING  - braking is physically distinct and must not be shown
 *                   as ACCELERATING or confused with an impact.
 */
typedef enum
{
    IMU_MOTION_UNKNOWN = 0,
    IMU_MOTION_STATIONARY,
    IMU_MOTION_STEADY_MOTION,
    IMU_MOTION_ACCELERATING,
    IMU_MOTION_DECELERATING,
    IMU_MOTION_TURNING_LEFT,
    IMU_MOTION_TURNING_RIGHT,
    IMU_MOTION_CLIMBING_HUMP,
    IMU_MOTION_DESCENDING_HUMP,
    IMU_MOTION_SUDDEN_IMPACT,
    IMU_MOTION_COUNT
} imu_motion_class_t;

/** Heading quality. */
typedef enum
{
    IMU_HEADING_INVALID = 0,     /**< no usable magnetometer data           */
    IMU_HEADING_UNCALIBRATED,    /**< raw field, hard-iron not removed      */
    IMU_HEADING_DISTURBED,       /**< |B| differs from calibrated reference */
    IMU_HEADING_GOOD
} imu_heading_quality_t;

/** Turning direction. */
typedef enum
{
    IMU_TURN_NONE = 0,
    IMU_TURN_LEFT,
    IMU_TURN_RIGHT
} imu_turn_dir_t;

/** Hump-processing phase (Design Review 6.4 HUMP PROCESSING region). */
typedef enum
{
    IMU_HUMP_MONITORING = 0,
    IMU_HUMP_ASCENT,
    IMU_HUMP_PEAK,
    IMU_HUMP_DESCENT
} imu_hump_phase_t;

/** Buddy 4 subsystem states (Design Review Section 6.4). */
typedef enum
{
    IMU_STATE_INITIALISATION = 0,
    IMU_STATE_CALIBRATION,
    IMU_STATE_MONITORING,
    IMU_STATE_HUMP_ASCENT,
    IMU_STATE_HUMP_PEAK,
    IMU_STATE_HUMP_DESCENT,
    IMU_STATE_TILT_EVENT,       /**< transient (run-to-completion)   */
    IMU_STATE_TURN_EVENT,       /**< transient (run-to-completion)   */
    IMU_STATE_IMPACT_EVENT,     /**< transient: classify impact      */
    IMU_STATE_FAULT_HANDLING,   /**< transient: report collision     */
    IMU_STATE_SENSOR_FAULT,     /**< transient: handle IMU error     */
    IMU_STATE_FAULT_SAFE,
    IMU_STATE_RECOVERY,
    IMU_STATE_STOPPED,
    IMU_STATE_COUNT
} imu_state_t;

/** Reason for being in a fault state. */
typedef enum
{
    IMU_FAULT_NONE = 0,
    IMU_FAULT_INIT_FAILED,
    IMU_FAULT_CAL_FAILED,
    IMU_FAULT_I2C,
    IMU_FAULT_STALE_DATA,
    IMU_FAULT_INVALID_DATA,
    IMU_FAULT_FROZEN_DATA,
    IMU_FAULT_COLLISION
} imu_fault_t;

/**
 * Optional motion hint supplied by Buddy 2 (encoders) through the Mission
 * Controller. Buddy 4 never reads Buddy 2's hardware; it only consumes this
 * structure if somebody provides it.
 */
typedef struct
{
    bool     b_valid;            /**< false = no encoder information      */
    bool     b_accel_valid;      /**< accel_mm_s2 field is meaningful     */
    int32_t  speed_mm_s;         /**< signed forward speed (+ = forward)  */
    int32_t  accel_mm_s2;        /**< signed forward acceleration         */
    uint32_t timestamp_ms;       /**< time the hint was produced          */
} imu_motion_hint_t;

#endif /* IMU_TYPES_H */
```

## `include/imu/lsm303dlhc_regs.h`

```c
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
```

## `include/imu/imu_config.h`

```c
/**
 * @file    imu_config.h
 * @brief   Buddy 4 configuration: hardware constants, default tuning values
 *          and the run-time parameter structure.
 *
 * !!! IMPORTANT !!!
 * Every threshold, filter cut-off and timing value marked "UNVALIDATED" is an
 * initial DEVELOPMENT DEFAULT chosen from first-principles estimates for a
 * small (~150 mm wheelbase) two-wheel robot car travelling at 0.1-0.5 m/s.
 * None of these values has been validated on the team's vehicle. They must be
 * replaced with values derived from the recorded measurements described in
 * docs/buddy4/README.md (Part 7) before they are relied upon.
 * (Design Review Sections 8.2 / 8.3: "Deferred / Needs validation".)
 *
 * The values can be overridden either at compile time (-D on the compiler
 * command line, because every macro is wrapped in #ifndef) or at run time
 * through imu_params_t.
 */

#ifndef IMU_CONFIG_H
#define IMU_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_types.h"
#include "imu/lsm303dlhc_regs.h"

/* ===================================================================== */
/*  Hardware allocation (Design Review Section 2) - CONFIRMED BY DOCUMENT */
/* ===================================================================== */

/** Robo Pico Grove 4: GP16 = I2C0 SDA, GP17 = I2C0 SCL (RP2040 GPIO mux). */
#ifndef IMU_I2C_SDA_GPIO
#define IMU_I2C_SDA_GPIO                (16U)
#endif
#ifndef IMU_I2C_SCL_GPIO
#define IMU_I2C_SCL_GPIO                (17U)
#endif
/** 0 = i2c0, 1 = i2c1. GP16/GP17 can only be I2C0 on the RP2040. */
#ifndef IMU_I2C_INSTANCE
#define IMU_I2C_INSTANCE                (0U)
#endif
/**
 * Enable the RP2040 internal pull-ups (~50-80 kOhm). Most LSM303DLHC breakout
 * boards already carry 4.7-10 kOhm pull-ups; the internal ones are only a
 * weak backup. Verify the breakout with a multimeter.
 */
#ifndef IMU_I2C_INTERNAL_PULLUPS
#define IMU_I2C_INTERNAL_PULLUPS        (1)
#endif

/* ===================================================================== */
/*  Static sizing (compile time only)                                     */
/* ===================================================================== */
#define IMU_HEADING_HIST_LEN            (16U)  /**< heading history slots  */
#define IMU_EVENT_CRITICAL_PENDING      (4U)   /**< critical retry slots   */
#ifndef IMU_STATE_TRACE_LEN
#define IMU_STATE_TRACE_LEN             (16U)  /**< state transition trace */
#endif
#define IMU_ACCEL_BATCH_MAX             (LSM303_FIFO_DEPTH)
#define IMU_MAX_SAMPLE_PERIOD_MS        (100U)
#define IMU_MIN_SAMPLE_PERIOD_MS        (5U)

/* ===================================================================== */
/*  Default (UNVALIDATED) tuning values                                   */
/* ===================================================================== */

/* --- Sampling ----------------------------------------------------------
 * 20 ms (50 Hz): two ticks of the 10 ms uT-Kernel system timer used by the
 * TRON RP2040 port. Must be an integer multiple of the kernel tick. */
#ifndef IMU_CFG_SAMPLE_PERIOD_MS
#define IMU_CFG_SAMPLE_PERIOD_MS        (20U)
#endif

/* --- I2C ---------------------------------------------------------------- */
#ifndef IMU_CFG_I2C_BAUD_HZ
#define IMU_CFG_I2C_BAUD_HZ             (400000U)  /* LSM303DLHC fast-mode */
#endif
#ifndef IMU_CFG_I2C_TIMEOUT_US
#define IMU_CFG_I2C_TIMEOUT_US          (2000U)    /* per transfer bound   */
#endif
#ifndef IMU_CFG_I2C_MAX_RETRIES
#define IMU_CFG_I2C_MAX_RETRIES         (2U)
#endif

/* --- Fault handling (UNVALIDATED) ---------------------------------------- */
#define IMU_CFG_FAULT_CONSEC_CYCLES     (5U)     /* 100 ms of failures     */
#define IMU_CFG_FROZEN_CYCLES           (50U)    /* 1 s identical samples  */
#define IMU_CFG_STALE_DATA_MS           (200U)
#define IMU_CFG_ACCEL_PLAUS_MIN_G       (0.20f)
#define IMU_CFG_INIT_RETRY_MS           (200U)
#define IMU_CFG_INIT_MAX_ATTEMPTS       (5U)
#define IMU_CFG_RECOVERY_INTERVAL_MS    (200U)
#define IMU_CFG_RECOVERY_INTERVAL_MAX_MS (2000U)
#define IMU_CFG_RECOVERY_MAX_ATTEMPTS   (10U)
#define IMU_CFG_RECOVERY_VERIFY_SAMPLES (10U)
#define IMU_CFG_COLLISION_SETTLE_MS     (500U)

/* --- Calibration (UNVALIDATED) -------------------------------------------- */
#define IMU_CFG_CAL_SAMPLES             (100U)   /* 2 s at 50 Hz           */
#define IMU_CFG_CAL_MAX_NOISE_G         (0.02f)  /* per-axis std dev       */
#define IMU_CFG_CAL_G_MIN               (0.85f)
#define IMU_CFG_CAL_G_MAX               (1.15f)
#define IMU_CFG_CAL_MAX_MOUNT_TILT_DEG  (30.0f)
#define IMU_CFG_CAL_MAX_ATTEMPTS        (5U)
#define IMU_CFG_CAL_RETRY_MS            (500U)
#define IMU_CFG_MAG_CAL_MIN_SPAN_GAUSS  (0.30f)
#define IMU_CFG_MAG_CAL_MIN_SAMPLES     (50U)

/* --- Filtering (UNVALIDATED) ---------------------------------------------- */
#define IMU_CFG_ATTITUDE_LPF_HZ         (8.0f)
#define IMU_CFG_ATTITUDE_VALID_TOL_G    (0.15f)
#define IMU_CFG_HEADING_LPF_HZ          (3.0f)
#define IMU_CFG_MAG_FIELD_TOL_FRAC      (0.25f)
#define IMU_CFG_HEADING_MAX_JUMP_DEG    (45.0f)
#define IMU_CFG_HEADING_OUTLIER_LIMIT   (3U)
#define IMU_CFG_IMPACT_LPF_HZ           (2.0f)
#define IMU_CFG_VIB_LPF_HZ              (1.0f)

/* --- Tilt (UNVALIDATED) --------------------------------------------------- */
#define IMU_CFG_TILT_THRESHOLD_DEG      (15.0f)
#define IMU_CFG_TILT_HYSTERESIS_DEG     (3.0f)
#define IMU_CFG_TILT_MIN_DURATION_MS    (300U)
#define IMU_CFG_TILT_CRITICAL_DEG       (35.0f)

/* --- Turning (UNVALIDATED) ------------------------------------------------ */
#define IMU_CFG_TURN_RATE_WINDOW_MS     (200U)
#define IMU_CFG_TURN_ENTER_DPS          (25.0f)
#define IMU_CFG_TURN_EXIT_DPS           (12.0f)
#define IMU_CFG_TURN_MIN_DURATION_MS    (200U)
#define IMU_CFG_TURN_EXIT_DURATION_MS   (200U)
#define IMU_CFG_HEADING_CHANGE_DEG      (20.0f)
#define IMU_CFG_TURN_ALLOW_UNCAL_MAG    (true)

/* --- Hump detection (UNVALIDATED) ----------------------------------------- */
#define IMU_CFG_HUMP_ASCENT_ENTER_DEG   (3.0f)
#define IMU_CFG_HUMP_ASCENT_MIN_MS      (60U)
#define IMU_CFG_HUMP_PEAK_DROP_DEG      (1.0f)
#define IMU_CFG_HUMP_DESCENT_ENTER_DEG  (2.5f)   /* magnitude, nose-down */
#define IMU_CFG_HUMP_DESCENT_MIN_MS     (40U)
#define IMU_CFG_HUMP_LEVEL_BAND_DEG     (1.5f)
#define IMU_CFG_HUMP_LEVEL_MIN_MS       (100U)
#define IMU_CFG_HUMP_MIN_PEAK_DEG       (3.5f)
#define IMU_CFG_HUMP_MIN_DESCENT_DEG    (2.0f)
#define IMU_CFG_HUMP_ASCENT_MAX_MS      (2000U)
#define IMU_CFG_HUMP_MIN_EVENT_MS       (150U)
/* Max time between the crest and the start of the nose-down phase: front
 * wheels land, rear wheels must then reach the hump. ~1.3 * L / v_min
 * (L = 0.14 m, v_min = 0.12 m/s). Distance-based (1.5 L) with a hint. */
#define IMU_CFG_HUMP_CREST_MAX_MS       (1500U)
#define IMU_CFG_HUMP_MAX_EVENT_MS       (4000U)
#define IMU_CFG_HUMP_REARM_LEVEL_MS     (150U)
#define IMU_CFG_HUMP_MIN_GAP_MS         (300U)
#define IMU_CFG_HUMP_IMPACT_THR_SCALE   (1.5f)
#define IMU_CFG_HUMP_REQUIRE_MOVING     (true)   /* only used with a hint */
#define IMU_CFG_HUMP_MAX_HINT_ACCEL_G   (0.10f)  /* only used with a hint */
/** Wheelbase (axle-to-axle / wheel-to-caster contact distance).
 *  PLACEHOLDER - MEASURE THE REAL VEHICLE. */
#define IMU_CFG_WHEELBASE_MM            (140.0f)
#define IMU_CFG_HUMP_HEIGHT_GAIN        (1.0f)
#define IMU_CFG_HUMP_HEIGHT_OFFSET_MM   (0.0f)
#define IMU_CFG_IMU_POS_FRAC            (0.5f)   /* IMU dist from rear / L */

/* --- Impact (UNVALIDATED) ------------------------------------------------- */
#define IMU_CFG_IMPACT_MINOR_G          (1.2f)
#define IMU_CFG_IMPACT_CRITICAL_G       (2.0f)
#define IMU_CFG_IMPACT_CRITICAL_ANY_G   (3.0f)
#define IMU_CFG_IMPACT_HORIZ_RATIO      (0.6f)
#define IMU_CFG_IMPACT_MAX_DURATION_MS  (60U)
#define IMU_CFG_IMPACT_COOLDOWN_MS      (1000U)
#define IMU_CFG_IMPACT_HOLD_MS          (500U)
#define IMU_CFG_IMPACT_WARMUP_MS        (500U)
#define IMU_CFG_IMPACT_MINOR_FAULT      (false)

/* --- Motion classification (UNVALIDATED) ---------------------------------- */
#define IMU_CFG_ACCEL_ENTER_G           (0.06f)
#define IMU_CFG_ACCEL_EXIT_G            (0.03f)
#define IMU_CFG_CLASS_HOLD_MS           (150U)
#define IMU_CFG_STATIONARY_SPEED_MM_S   (10)
#define IMU_CFG_STATIONARY_VIB_G        (0.015f)
#define IMU_CFG_HINT_MAX_AGE_MS         (200U)
#define IMU_CFG_HINT_MAX_SPEED_MM_S     (3000)   /* plausibility bound */
#define IMU_CFG_HINT_MAX_ACCEL_MM_S2    (20000)  /* plausibility bound */

/* --- Event rate limiting -------------------------------------------------- */
#define IMU_CFG_MOTION_EVT_MIN_MS       (250U)
#define IMU_CFG_TILT_EVT_MIN_MS         (250U)
#define IMU_CFG_TURN_EVT_MIN_MS         (100U)

/* ===================================================================== */
/*  Enumerations used by the parameter structure                          */
/* ===================================================================== */

/** Vehicle-to-sensor axis selector. */
typedef enum
{
    IMU_AXIS_X = 0,
    IMU_AXIS_Y = 1,
    IMU_AXIS_Z = 2
} imu_axis_t;

/** Physical height estimation method (see docs Part 2 for the physics). */
typedef enum
{
    IMU_HEIGHT_METHOD_NONE = 0,   /**< report only the IMU peak metric     */
    IMU_HEIGHT_METHOD_GEOMETRIC,  /**< h = L * sin(peak pitch)             */
    IMU_HEIGHT_METHOD_INTEGRATED  /**< h = max integral(v sin(pitch)) / k  */
} imu_height_method_t;

/** Metric used to rank humps and pick the mission maximum. */
typedef enum
{
    IMU_RANK_BY_PEAK_PITCH = 0,   /**< directly measured (default)         */
    IMU_RANK_BY_HEIGHT_EST        /**< estimated height (needs calibration)*/
} imu_rank_metric_t;

/* ===================================================================== */
/*  Run-time parameter structure                                          */
/* ===================================================================== */

/** All tunables in one structure so that tests and the Mission Controller
 *  can change them without recompiling. Populate with imu_params_default()
 *  and validate with imu_params_validate(). */
typedef struct
{
    /* Sampling / bus */
    uint32_t sample_period_ms;
    uint32_t i2c_baud_hz;
    uint32_t i2c_timeout_us;
    uint8_t  i2c_max_retries;
    uint8_t  accel_odr_code;
    uint8_t  accel_fs_code;
    bool     b_accel_fifo;
    uint8_t  mag_odr_code;
    uint8_t  mag_gain_code;
    bool     b_mag_id_check;

    /* Sensor-to-vehicle axis map: vehicle axis k = sign[k] * sensor[axis[k]]
     * k: 0 = forward, 1 = left, 2 = up. Must form a right-handed frame. */
    uint8_t  axis_map[3];
    int8_t   axis_sign[3];

    /* Faults */
    uint32_t fault_consec_cycles;
    uint32_t frozen_cycles;
    uint32_t stale_data_ms;
    float    accel_plaus_min_g;
    uint32_t init_retry_ms;
    uint32_t init_max_attempts;
    uint32_t recovery_interval_ms;
    uint32_t recovery_interval_max_ms;
    uint32_t recovery_max_attempts;
    uint32_t recovery_verify_samples;
    uint32_t collision_settle_ms;

    /* Calibration */
    uint32_t cal_samples;
    float    cal_max_noise_g;
    float    cal_g_min;
    float    cal_g_max;
    float    cal_max_mount_tilt_deg;
    uint32_t cal_max_attempts;
    uint32_t cal_retry_ms;
    float    mag_cal_min_span_gauss;
    uint32_t mag_cal_min_samples;

    /* Stored magnetometer calibration (from the offline procedure) */
    bool       b_mag_cal_valid;
    imu_vec3_t mag_hard_iron_gauss;
    imu_vec3_t mag_soft_iron_scale;
    float      mag_ref_field_gauss;     /* 0 = take from calibration */

    /* Optional stored accelerometer bias from a 6-position calibration */
    imu_vec3_t accel_bias_g;

    /* Filtering */
    float    attitude_lpf_hz;
    float    attitude_valid_tol_g;
    float    heading_lpf_hz;
    float    mag_field_tol_frac;
    float    heading_max_jump_deg;
    uint32_t heading_outlier_limit;
    float    impact_lpf_hz;
    float    vib_lpf_hz;

    /* Tilt */
    float    tilt_threshold_deg;
    float    tilt_hysteresis_deg;
    uint32_t tilt_min_duration_ms;
    float    tilt_critical_deg;

    /* Turning */
    uint32_t turn_rate_window_ms;
    float    turn_enter_dps;
    float    turn_exit_dps;
    uint32_t turn_min_duration_ms;
    uint32_t turn_exit_duration_ms;
    float    heading_change_deg;
    bool     b_turn_allow_uncal_mag;

    /* Hump */
    float    hump_ascent_enter_deg;
    uint32_t hump_ascent_min_ms;
    float    hump_peak_drop_deg;
    float    hump_descent_enter_deg;
    uint32_t hump_descent_min_ms;
    float    hump_level_band_deg;
    uint32_t hump_level_min_ms;
    float    hump_min_peak_deg;
    float    hump_min_descent_deg;
    uint32_t hump_ascent_max_ms;
    uint32_t hump_min_event_ms;
    uint32_t hump_crest_max_ms;
    uint32_t hump_max_event_ms;
    uint32_t hump_rearm_level_ms;
    uint32_t hump_min_gap_ms;
    float    hump_impact_thr_scale;
    bool     b_hump_require_moving;
    float    hump_max_hint_accel_g;
    float    wheelbase_mm;
    float    hump_height_gain;
    float    hump_height_offset_mm;
    float    imu_pos_frac;
    bool     b_hump_height_calibrated;
    imu_height_method_t height_method;
    imu_rank_metric_t   rank_metric;

    /* Impact */
    float    impact_minor_g;
    float    impact_critical_g;
    float    impact_critical_any_g;
    float    impact_horiz_ratio;
    uint32_t impact_max_duration_ms;
    uint32_t impact_cooldown_ms;
    uint32_t impact_hold_ms;
    uint32_t impact_warmup_ms;
    bool     b_impact_minor_enters_fault;

    /* Motion classification */
    float    accel_enter_g;
    float    accel_exit_g;
    uint32_t class_hold_ms;
    int32_t  stationary_speed_mm_s;
    float    stationary_vib_g;
    uint32_t hint_max_age_ms;

    /* Event rate limiting */
    uint32_t motion_evt_min_ms;
    uint32_t tilt_evt_min_ms;
    uint32_t turn_evt_min_ms;
} imu_params_t;

/**
 * @brief  Fill @p p_params with the compile-time defaults above.
 * @param  p_params  destination (must not be NULL)
 */
void imu_params_default(imu_params_t * p_params);

/**
 * @brief  Range-check a parameter set (axis map handedness, ordering of
 *         hysteresis thresholds, non-zero periods, finite values...).
 * @return IMU_OK or IMU_ERR_PARAM.
 */
imu_err_t imu_params_validate(imu_params_t const * p_params);

/** @return accelerometer ODR in Hz for an LSM303 ODR code (0 if unknown). */
uint32_t imu_params_accel_odr_hz(uint8_t odr_code);

#endif /* IMU_CONFIG_H */
```

## `src/imu/imu_params.c`

```c
/**
 * @file    imu_params.c
 * @brief   Buddy 4 run-time parameter defaults and validation.
 */

#include <math.h>
#include <stddef.h>

#include "imu/imu_config.h"

/* Upper bounds used only for sanity checking (not tuning values). */
#define PARAM_MAX_ANGLE_DEG        (90.0f)
#define PARAM_MAX_G                (16.0f)
#define PARAM_MAX_TIME_MS          (60000U)
#define PARAM_MAX_LPF_HZ           (100.0f)
#define PARAM_MAX_WHEELBASE_MM     (2000.0f)
#define PARAM_MAX_SAMPLES          (2000U)

static bool param_finite_pos(float value)
{
    return ((0 != isfinite(value)) && (value > 0.0f));
}

static bool param_finite_range(float value, float lo, float hi)
{
    return ((0 != isfinite(value)) && (value >= lo) && (value <= hi));
}

static bool param_time_ok(uint32_t value)
{
    return ((value > 0U) && (value <= PARAM_MAX_TIME_MS));
}

uint32_t imu_params_accel_odr_hz(uint8_t odr_code)
{
    uint32_t hz;

    switch (odr_code)
    {
        case LSM303_ACCEL_ODR_1HZ:   hz = 1U;   break;
        case LSM303_ACCEL_ODR_10HZ:  hz = 10U;  break;
        case LSM303_ACCEL_ODR_25HZ:  hz = 25U;  break;
        case LSM303_ACCEL_ODR_50HZ:  hz = 50U;  break;
        case LSM303_ACCEL_ODR_100HZ: hz = 100U; break;
        case LSM303_ACCEL_ODR_200HZ: hz = 200U; break;
        case LSM303_ACCEL_ODR_400HZ: hz = 400U; break;
        default:                     hz = 0U;   break;
    }

    return (hz);
}

void imu_params_default(imu_params_t * p_params)
{
    if (NULL == p_params)
    {
        return;
    }

    p_params->sample_period_ms   = IMU_CFG_SAMPLE_PERIOD_MS;
    p_params->i2c_baud_hz        = IMU_CFG_I2C_BAUD_HZ;
    p_params->i2c_timeout_us     = IMU_CFG_I2C_TIMEOUT_US;
    p_params->i2c_max_retries    = (uint8_t)IMU_CFG_I2C_MAX_RETRIES;
    p_params->accel_odr_code     = LSM303_ACCEL_ODR_200HZ;
    p_params->accel_fs_code      = LSM303_ACCEL_FS_4G;
    p_params->b_accel_fifo       = true;
    p_params->mag_odr_code       = LSM303_MAG_ODR_75HZ;
    p_params->mag_gain_code      = LSM303_MAG_GAIN_1_9;
    p_params->b_mag_id_check     = true;

    /* Default: sensor X forward, Y left, Z up. CHANGE AFTER AXIS TEST. */
    p_params->axis_map[0]  = (uint8_t)IMU_AXIS_X;
    p_params->axis_map[1]  = (uint8_t)IMU_AXIS_Y;
    p_params->axis_map[2]  = (uint8_t)IMU_AXIS_Z;
    p_params->axis_sign[0] = 1;
    p_params->axis_sign[1] = 1;
    p_params->axis_sign[2] = 1;

    p_params->fault_consec_cycles      = IMU_CFG_FAULT_CONSEC_CYCLES;
    p_params->frozen_cycles            = IMU_CFG_FROZEN_CYCLES;
    p_params->stale_data_ms            = IMU_CFG_STALE_DATA_MS;
    p_params->accel_plaus_min_g        = IMU_CFG_ACCEL_PLAUS_MIN_G;
    p_params->init_retry_ms            = IMU_CFG_INIT_RETRY_MS;
    p_params->init_max_attempts        = IMU_CFG_INIT_MAX_ATTEMPTS;
    p_params->recovery_interval_ms     = IMU_CFG_RECOVERY_INTERVAL_MS;
    p_params->recovery_interval_max_ms = IMU_CFG_RECOVERY_INTERVAL_MAX_MS;
    p_params->recovery_max_attempts    = IMU_CFG_RECOVERY_MAX_ATTEMPTS;
    p_params->recovery_verify_samples  = IMU_CFG_RECOVERY_VERIFY_SAMPLES;
    p_params->collision_settle_ms      = IMU_CFG_COLLISION_SETTLE_MS;

    p_params->cal_samples            = IMU_CFG_CAL_SAMPLES;
    p_params->cal_max_noise_g        = IMU_CFG_CAL_MAX_NOISE_G;
    p_params->cal_g_min              = IMU_CFG_CAL_G_MIN;
    p_params->cal_g_max              = IMU_CFG_CAL_G_MAX;
    p_params->cal_max_mount_tilt_deg = IMU_CFG_CAL_MAX_MOUNT_TILT_DEG;
    p_params->cal_max_attempts       = IMU_CFG_CAL_MAX_ATTEMPTS;
    p_params->cal_retry_ms           = IMU_CFG_CAL_RETRY_MS;
    p_params->mag_cal_min_span_gauss = IMU_CFG_MAG_CAL_MIN_SPAN_GAUSS;
    p_params->mag_cal_min_samples    = IMU_CFG_MAG_CAL_MIN_SAMPLES;

    p_params->b_mag_cal_valid       = false;
    p_params->mag_hard_iron_gauss.x = 0.0f;
    p_params->mag_hard_iron_gauss.y = 0.0f;
    p_params->mag_hard_iron_gauss.z = 0.0f;
    p_params->mag_soft_iron_scale.x = 1.0f;
    p_params->mag_soft_iron_scale.y = 1.0f;
    p_params->mag_soft_iron_scale.z = 1.0f;
    p_params->mag_ref_field_gauss   = 0.0f;

    p_params->accel_bias_g.x = 0.0f;
    p_params->accel_bias_g.y = 0.0f;
    p_params->accel_bias_g.z = 0.0f;

    p_params->attitude_lpf_hz       = IMU_CFG_ATTITUDE_LPF_HZ;
    p_params->attitude_valid_tol_g  = IMU_CFG_ATTITUDE_VALID_TOL_G;
    p_params->heading_lpf_hz        = IMU_CFG_HEADING_LPF_HZ;
    p_params->mag_field_tol_frac    = IMU_CFG_MAG_FIELD_TOL_FRAC;
    p_params->heading_max_jump_deg  = IMU_CFG_HEADING_MAX_JUMP_DEG;
    p_params->heading_outlier_limit = IMU_CFG_HEADING_OUTLIER_LIMIT;
    p_params->impact_lpf_hz         = IMU_CFG_IMPACT_LPF_HZ;
    p_params->vib_lpf_hz            = IMU_CFG_VIB_LPF_HZ;

    p_params->tilt_threshold_deg   = IMU_CFG_TILT_THRESHOLD_DEG;
    p_params->tilt_hysteresis_deg  = IMU_CFG_TILT_HYSTERESIS_DEG;
    p_params->tilt_min_duration_ms = IMU_CFG_TILT_MIN_DURATION_MS;
    p_params->tilt_critical_deg    = IMU_CFG_TILT_CRITICAL_DEG;

    p_params->turn_rate_window_ms    = IMU_CFG_TURN_RATE_WINDOW_MS;
    p_params->turn_enter_dps         = IMU_CFG_TURN_ENTER_DPS;
    p_params->turn_exit_dps          = IMU_CFG_TURN_EXIT_DPS;
    p_params->turn_min_duration_ms   = IMU_CFG_TURN_MIN_DURATION_MS;
    p_params->turn_exit_duration_ms  = IMU_CFG_TURN_EXIT_DURATION_MS;
    p_params->heading_change_deg     = IMU_CFG_HEADING_CHANGE_DEG;
    p_params->b_turn_allow_uncal_mag = IMU_CFG_TURN_ALLOW_UNCAL_MAG;

    p_params->hump_ascent_enter_deg    = IMU_CFG_HUMP_ASCENT_ENTER_DEG;
    p_params->hump_ascent_min_ms       = IMU_CFG_HUMP_ASCENT_MIN_MS;
    p_params->hump_peak_drop_deg       = IMU_CFG_HUMP_PEAK_DROP_DEG;
    p_params->hump_descent_enter_deg   = IMU_CFG_HUMP_DESCENT_ENTER_DEG;
    p_params->hump_descent_min_ms      = IMU_CFG_HUMP_DESCENT_MIN_MS;
    p_params->hump_level_band_deg      = IMU_CFG_HUMP_LEVEL_BAND_DEG;
    p_params->hump_level_min_ms        = IMU_CFG_HUMP_LEVEL_MIN_MS;
    p_params->hump_min_peak_deg        = IMU_CFG_HUMP_MIN_PEAK_DEG;
    p_params->hump_min_descent_deg     = IMU_CFG_HUMP_MIN_DESCENT_DEG;
    p_params->hump_ascent_max_ms       = IMU_CFG_HUMP_ASCENT_MAX_MS;
    p_params->hump_min_event_ms        = IMU_CFG_HUMP_MIN_EVENT_MS;
    p_params->hump_crest_max_ms        = IMU_CFG_HUMP_CREST_MAX_MS;
    p_params->hump_max_event_ms        = IMU_CFG_HUMP_MAX_EVENT_MS;
    p_params->hump_rearm_level_ms      = IMU_CFG_HUMP_REARM_LEVEL_MS;
    p_params->hump_min_gap_ms          = IMU_CFG_HUMP_MIN_GAP_MS;
    p_params->hump_impact_thr_scale    = IMU_CFG_HUMP_IMPACT_THR_SCALE;
    p_params->b_hump_require_moving    = IMU_CFG_HUMP_REQUIRE_MOVING;
    p_params->hump_max_hint_accel_g    = IMU_CFG_HUMP_MAX_HINT_ACCEL_G;
    p_params->wheelbase_mm             = IMU_CFG_WHEELBASE_MM;
    p_params->hump_height_gain         = IMU_CFG_HUMP_HEIGHT_GAIN;
    p_params->hump_height_offset_mm    = IMU_CFG_HUMP_HEIGHT_OFFSET_MM;
    p_params->imu_pos_frac             = IMU_CFG_IMU_POS_FRAC;
    p_params->b_hump_height_calibrated = false;
    p_params->height_method            = IMU_HEIGHT_METHOD_GEOMETRIC;
    p_params->rank_metric              = IMU_RANK_BY_PEAK_PITCH;

    p_params->impact_minor_g              = IMU_CFG_IMPACT_MINOR_G;
    p_params->impact_critical_g           = IMU_CFG_IMPACT_CRITICAL_G;
    p_params->impact_critical_any_g       = IMU_CFG_IMPACT_CRITICAL_ANY_G;
    p_params->impact_horiz_ratio          = IMU_CFG_IMPACT_HORIZ_RATIO;
    p_params->impact_max_duration_ms      = IMU_CFG_IMPACT_MAX_DURATION_MS;
    p_params->impact_cooldown_ms          = IMU_CFG_IMPACT_COOLDOWN_MS;
    p_params->impact_hold_ms              = IMU_CFG_IMPACT_HOLD_MS;
    p_params->impact_warmup_ms            = IMU_CFG_IMPACT_WARMUP_MS;
    p_params->b_impact_minor_enters_fault = IMU_CFG_IMPACT_MINOR_FAULT;

    p_params->accel_enter_g          = IMU_CFG_ACCEL_ENTER_G;
    p_params->accel_exit_g           = IMU_CFG_ACCEL_EXIT_G;
    p_params->class_hold_ms          = IMU_CFG_CLASS_HOLD_MS;
    p_params->stationary_speed_mm_s  = IMU_CFG_STATIONARY_SPEED_MM_S;
    p_params->stationary_vib_g       = IMU_CFG_STATIONARY_VIB_G;
    p_params->hint_max_age_ms        = IMU_CFG_HINT_MAX_AGE_MS;

    p_params->motion_evt_min_ms = IMU_CFG_MOTION_EVT_MIN_MS;
    p_params->tilt_evt_min_ms   = IMU_CFG_TILT_EVT_MIN_MS;
    p_params->turn_evt_min_ms   = IMU_CFG_TURN_EVT_MIN_MS;
}

/** Check that the axis map is a permutation and right-handed (det = +1). */
static bool params_axis_map_ok(imu_params_t const * p_params)
{
    float   m[3][3] = { { 0.0f } };
    float   det;
    uint8_t used = 0U;
    uint8_t k;

    for (k = 0U; k < 3U; k++)
    {
        uint8_t axis = p_params->axis_map[k];
        int8_t  sign = p_params->axis_sign[k];

        if ((axis > (uint8_t)IMU_AXIS_Z) || ((1 != sign) && (-1 != sign)))
        {
            return (false);
        }
        used |= (uint8_t)(1U << axis);
        m[k][axis] = (float)sign;
    }

    if (0x07U != used)
    {
        return (false);   /* not a permutation */
    }

    det = (m[0][0] * ((m[1][1] * m[2][2]) - (m[1][2] * m[2][1])))
        - (m[0][1] * ((m[1][0] * m[2][2]) - (m[1][2] * m[2][0])))
        + (m[0][2] * ((m[1][0] * m[2][1]) - (m[1][1] * m[2][0])));

    /* det = -1 would mirror the world: left/right turns would swap. */
    return (det > 0.0f);
}

imu_err_t imu_params_validate(imu_params_t const * p_params)
{
    bool b_ok = true;

    if (NULL == p_params)
    {
        return (IMU_ERR_PARAM);
    }

    b_ok = b_ok && (p_params->sample_period_ms >= IMU_MIN_SAMPLE_PERIOD_MS);
    b_ok = b_ok && (p_params->sample_period_ms <= IMU_MAX_SAMPLE_PERIOD_MS);
    b_ok = b_ok && (p_params->i2c_baud_hz >= 10000U);
    b_ok = b_ok && (p_params->i2c_baud_hz <= 400000U);
    b_ok = b_ok && (p_params->i2c_timeout_us >= 100U);
    b_ok = b_ok && (p_params->i2c_timeout_us <= 20000U);
    b_ok = b_ok && (p_params->i2c_max_retries <= 5U);
    b_ok = b_ok && (0U != imu_params_accel_odr_hz(p_params->accel_odr_code));
    b_ok = b_ok && (p_params->accel_fs_code <= LSM303_ACCEL_FS_16G);
    b_ok = b_ok && (p_params->mag_odr_code <= LSM303_MAG_ODR_220HZ);
    b_ok = b_ok && (p_params->mag_gain_code >= LSM303_MAG_GAIN_1_3);
    b_ok = b_ok && (p_params->mag_gain_code <= LSM303_MAG_GAIN_8_1);
    b_ok = b_ok && params_axis_map_ok(p_params);

    b_ok = b_ok && (p_params->fault_consec_cycles > 0U);
    b_ok = b_ok && (p_params->frozen_cycles > 1U);
    b_ok = b_ok && param_time_ok(p_params->stale_data_ms);
    b_ok = b_ok && param_finite_range(p_params->accel_plaus_min_g,
                                      0.0f, 0.9f);
    b_ok = b_ok && param_time_ok(p_params->init_retry_ms);
    b_ok = b_ok && (p_params->init_max_attempts > 0U);
    b_ok = b_ok && param_time_ok(p_params->recovery_interval_ms);
    b_ok = b_ok && (p_params->recovery_interval_max_ms
                    >= p_params->recovery_interval_ms);
    b_ok = b_ok && param_time_ok(p_params->recovery_interval_max_ms);
    b_ok = b_ok && (p_params->recovery_max_attempts > 0U);
    b_ok = b_ok && (p_params->recovery_verify_samples > 0U);
    b_ok = b_ok && param_time_ok(p_params->collision_settle_ms);

    b_ok = b_ok && (p_params->cal_samples >= 10U);
    b_ok = b_ok && (p_params->cal_samples <= PARAM_MAX_SAMPLES);
    b_ok = b_ok && param_finite_pos(p_params->cal_max_noise_g);
    b_ok = b_ok && param_finite_range(p_params->cal_g_min, 0.1f, 1.0f);
    b_ok = b_ok && param_finite_range(p_params->cal_g_max, 1.0f, 2.0f);
    b_ok = b_ok && param_finite_range(p_params->cal_max_mount_tilt_deg,
                                      0.0f, PARAM_MAX_ANGLE_DEG);
    b_ok = b_ok && (p_params->cal_max_attempts > 0U);
    b_ok = b_ok && param_time_ok(p_params->cal_retry_ms);
    b_ok = b_ok && param_finite_pos(p_params->mag_cal_min_span_gauss);
    b_ok = b_ok && (p_params->mag_cal_min_samples >= 10U);

    b_ok = b_ok && param_finite_range(p_params->mag_soft_iron_scale.x,
                                      0.2f, 5.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_soft_iron_scale.y,
                                      0.2f, 5.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_soft_iron_scale.z,
                                      0.2f, 5.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_hard_iron_gauss.x,
                                      -8.0f, 8.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_hard_iron_gauss.y,
                                      -8.0f, 8.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_hard_iron_gauss.z,
                                      -8.0f, 8.0f);
    b_ok = b_ok && param_finite_range(p_params->mag_ref_field_gauss,
                                      0.0f, 8.0f);
    b_ok = b_ok && param_finite_range(p_params->accel_bias_g.x, -0.5f, 0.5f);
    b_ok = b_ok && param_finite_range(p_params->accel_bias_g.y, -0.5f, 0.5f);
    b_ok = b_ok && param_finite_range(p_params->accel_bias_g.z, -0.5f, 0.5f);

    b_ok = b_ok && param_finite_range(p_params->attitude_lpf_hz,
                                      0.05f, PARAM_MAX_LPF_HZ);
    b_ok = b_ok && param_finite_range(p_params->attitude_valid_tol_g,
                                      0.01f, 1.0f);
    b_ok = b_ok && param_finite_range(p_params->heading_lpf_hz,
                                      0.05f, PARAM_MAX_LPF_HZ);
    b_ok = b_ok && param_finite_range(p_params->mag_field_tol_frac,
                                      0.01f, 1.0f);
    b_ok = b_ok && param_finite_range(p_params->heading_max_jump_deg,
                                      1.0f, 180.0f);
    b_ok = b_ok && param_finite_range(p_params->impact_lpf_hz,
                                      0.05f, PARAM_MAX_LPF_HZ);
    b_ok = b_ok && param_finite_range(p_params->vib_lpf_hz,
                                      0.05f, PARAM_MAX_LPF_HZ);

    b_ok = b_ok && param_finite_range(p_params->tilt_threshold_deg,
                                      1.0f, PARAM_MAX_ANGLE_DEG);
    b_ok = b_ok && param_finite_range(p_params->tilt_hysteresis_deg, 0.0f,
                                      p_params->tilt_threshold_deg);
    b_ok = b_ok && param_time_ok(p_params->tilt_min_duration_ms);
    b_ok = b_ok && param_finite_range(p_params->tilt_critical_deg,
                                      p_params->tilt_threshold_deg,
                                      PARAM_MAX_ANGLE_DEG);

    b_ok = b_ok && param_time_ok(p_params->turn_rate_window_ms);
    b_ok = b_ok && (p_params->turn_rate_window_ms
                    >= (2U * p_params->sample_period_ms));
    b_ok = b_ok && (p_params->turn_rate_window_ms
                    < ((IMU_HEADING_HIST_LEN - 1U)
                       * p_params->sample_period_ms));
    b_ok = b_ok && param_finite_pos(p_params->turn_enter_dps);
    b_ok = b_ok && param_finite_range(p_params->turn_exit_dps, 0.0f,
                                      p_params->turn_enter_dps);
    b_ok = b_ok && param_time_ok(p_params->turn_min_duration_ms);
    b_ok = b_ok && param_time_ok(p_params->turn_exit_duration_ms);
    b_ok = b_ok && param_finite_range(p_params->heading_change_deg,
                                      1.0f, 180.0f);

    b_ok = b_ok && param_finite_range(p_params->hump_level_band_deg,
                                      0.1f, 20.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_ascent_enter_deg,
                                      p_params->hump_level_band_deg, 45.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_descent_enter_deg,
                                      p_params->hump_level_band_deg, 45.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_peak_drop_deg,
                                      0.1f, 20.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_min_peak_deg,
                                      p_params->hump_ascent_enter_deg, 45.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_min_descent_deg,
                                      0.0f, 45.0f);
    b_ok = b_ok && param_time_ok(p_params->hump_ascent_min_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_descent_min_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_level_min_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_ascent_max_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_min_event_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_crest_max_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_max_event_ms);
    b_ok = b_ok && (p_params->hump_max_event_ms > p_params->hump_min_event_ms);
    b_ok = b_ok && param_time_ok(p_params->hump_rearm_level_ms);
    b_ok = b_ok && (p_params->hump_min_gap_ms <= PARAM_MAX_TIME_MS);
    b_ok = b_ok && param_finite_range(p_params->hump_impact_thr_scale,
                                      1.0f, 10.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_max_hint_accel_g,
                                      0.0f, 2.0f);
    b_ok = b_ok && param_finite_range(p_params->wheelbase_mm,
                                      10.0f, PARAM_MAX_WHEELBASE_MM);
    b_ok = b_ok && param_finite_range(p_params->hump_height_gain,
                                      0.1f, 10.0f);
    b_ok = b_ok && param_finite_range(p_params->hump_height_offset_mm,
                                      -100.0f, 100.0f);
    b_ok = b_ok && param_finite_range(p_params->imu_pos_frac, 0.05f, 1.5f);
    b_ok = b_ok && (p_params->height_method <= IMU_HEIGHT_METHOD_INTEGRATED);
    b_ok = b_ok && (p_params->rank_metric <= IMU_RANK_BY_HEIGHT_EST);

    b_ok = b_ok && param_finite_range(p_params->impact_minor_g,
                                      0.1f, PARAM_MAX_G);
    b_ok = b_ok && param_finite_range(p_params->impact_critical_g,
                                      p_params->impact_minor_g, PARAM_MAX_G);
    b_ok = b_ok && param_finite_range(p_params->impact_critical_any_g,
                                      p_params->impact_critical_g,
                                      PARAM_MAX_G);
    b_ok = b_ok && param_finite_range(p_params->impact_horiz_ratio,
                                      0.0f, 1.0f);
    b_ok = b_ok && param_time_ok(p_params->impact_max_duration_ms);
    b_ok = b_ok && param_time_ok(p_params->impact_cooldown_ms);
    b_ok = b_ok && param_time_ok(p_params->impact_hold_ms);
    b_ok = b_ok && (p_params->impact_warmup_ms <= PARAM_MAX_TIME_MS);

    b_ok = b_ok && param_finite_range(p_params->accel_enter_g, 0.001f, 2.0f);
    b_ok = b_ok && param_finite_range(p_params->accel_exit_g, 0.0f,
                                      p_params->accel_enter_g);
    b_ok = b_ok && param_time_ok(p_params->class_hold_ms);
    b_ok = b_ok && (p_params->stationary_speed_mm_s >= 0);
    b_ok = b_ok && param_finite_range(p_params->stationary_vib_g, 0.0f, 1.0f);
    b_ok = b_ok && param_time_ok(p_params->hint_max_age_ms);

    b_ok = b_ok && (p_params->motion_evt_min_ms <= PARAM_MAX_TIME_MS);
    b_ok = b_ok && (p_params->tilt_evt_min_ms <= PARAM_MAX_TIME_MS);
    b_ok = b_ok && (p_params->turn_evt_min_ms <= PARAM_MAX_TIME_MS);

    return (b_ok ? IMU_OK : IMU_ERR_PARAM);
}
```

## `include/imu/imu_hal.h`

```c
/**
 * @file    imu_hal.h
 * @brief   Buddy 4 hardware-abstraction interface (I2C bus + time base).
 *
 * The LSM303DLHC driver only talks to the hardware through these functions.
 * Two implementations exist:
 *   - src/imu/imu_hal_pico.c : Raspberry Pi Pico SDK (hardware_i2c), target
 *   - tests/imu/sim_lsm303.c : register-level LSM303DLHC simulator, host
 * The implementation is selected at link time (no function pointers in the
 * hot path, no RTOS dependency).
 *
 * All functions are bounded in time: no call may block longer than the
 * timeout it is given (bus recovery: < 200 us).
 */

#ifndef IMU_HAL_H
#define IMU_HAL_H

#include <stddef.h>
#include <stdint.h>

/** Result of a HAL bus operation. */
typedef enum
{
    IMU_HAL_OK = 0,
    IMU_HAL_NACK,        /**< address or data not acknowledged */
    IMU_HAL_TIMEOUT,     /**< transfer did not finish in time  */
    IMU_HAL_ERROR        /**< any other bus / parameter error  */
} imu_hal_status_t;

/**
 * @brief  Initialise the I2C peripheral and the SDA/SCL pins.
 * @param  baud_hz  requested SCL frequency.
 */
imu_hal_status_t imu_hal_i2c_init(uint32_t baud_hz);

/** @brief Release the I2C peripheral and return the pins to GPIO inputs. */
void imu_hal_i2c_deinit(void);

/**
 * @brief  Write @p len bytes to 7-bit slave @p addr7 (STOP at the end).
 */
imu_hal_status_t imu_hal_i2c_write(uint8_t addr7, uint8_t const * p_data,
                                   size_t len, uint32_t timeout_us);

/**
 * @brief  Write @p tx_len bytes (no STOP, repeated START) then read
 *         @p rx_len bytes.
 */
imu_hal_status_t imu_hal_i2c_write_read(uint8_t addr7,
                                        uint8_t const * p_tx, size_t tx_len,
                                        uint8_t * p_rx, size_t rx_len,
                                        uint32_t timeout_us);

/**
 * @brief  I2C bus-clear procedure (NXP UM10204 section 3.1.16): with the
 *         peripheral detached, clock SCL up to 9 times until a slave that
 *         holds SDA low releases it, then generate a STOP.
 */
void imu_hal_i2c_bus_recover(void);

/** @return milliseconds since boot (wraps at 2^32). */
uint32_t imu_hal_time_ms(void);

/** @return microseconds since boot, low 32 bits (for execution timing). */
uint32_t imu_hal_time_us(void);

#endif /* IMU_HAL_H */
```

## `src/imu/imu_hal_pico.c`

```c
/**
 * @file    imu_hal_pico.c
 * @brief   Buddy 4 HAL implementation on the Raspberry Pi Pico C SDK.
 *
 * Uses hardware_i2c on I2C0, GP16 (SDA) / GP17 (SCL) = Robo Pico Grove 4,
 * as allocated in Design Review Section 2. No other GPIO is touched.
 *
 * Timing notes (RTOS integration):
 *   - i2c_write_timeout_us()/i2c_read_timeout_us() poll the peripheral in the
 *     calling task. They are preemptible by higher-priority uT-Kernel tasks
 *     and bounded by the timeout argument, so they never block the Motion
 *     Control / Line tasks. A typical 6-byte read at 400 kHz takes ~200 us.
 *   - Bus recovery busy-waits at most 10 x 2 x 5 us = 100 us.
 *   - sleep_ms()/sleep_us() are deliberately NOT used: under uT-Kernel the
 *     task must delay with tk_dly_tsk()/tk_slp_tsk() instead.
 */

#include "imu/imu_hal.h"
#include "imu/imu_config.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/error.h"
#include "hardware/timer.h"
#include "pico/time.h"

#if (IMU_I2C_INSTANCE == 0U)
#define IMU_I2C_PORT        (i2c0)
#else
#define IMU_I2C_PORT        (i2c1)
#endif

#define HAL_RECOVER_CLOCKS     (9U)
#define HAL_RECOVER_HALF_US    (5U)    /* 100 kHz bit-banged clock */

static uint32_t g_hal_baud_hz = IMU_CFG_I2C_BAUD_HZ;

static imu_hal_status_t hal_map_result(int result, size_t expected)
{
    imu_hal_status_t status;

    if ((result >= 0) && ((size_t)result == expected))
    {
        status = IMU_HAL_OK;
    }
    else if (PICO_ERROR_TIMEOUT == result)
    {
        status = IMU_HAL_TIMEOUT;
    }
    else if (PICO_ERROR_GENERIC == result)
    {
        status = IMU_HAL_NACK;   /* SDK reports address/data NACK this way */
    }
    else
    {
        status = IMU_HAL_ERROR;
    }

    return (status);
}

imu_hal_status_t imu_hal_i2c_init(uint32_t baud_hz)
{
    uint actual;

    g_hal_baud_hz = baud_hz;
    actual = i2c_init(IMU_I2C_PORT, baud_hz);

    gpio_set_function(IMU_I2C_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(IMU_I2C_SCL_GPIO, GPIO_FUNC_I2C);
#if (IMU_I2C_INTERNAL_PULLUPS != 0)
    gpio_pull_up(IMU_I2C_SDA_GPIO);
    gpio_pull_up(IMU_I2C_SCL_GPIO);
#endif

    return ((0U == actual) ? IMU_HAL_ERROR : IMU_HAL_OK);
}

void imu_hal_i2c_deinit(void)
{
    i2c_deinit(IMU_I2C_PORT);
    gpio_set_function(IMU_I2C_SDA_GPIO, GPIO_FUNC_SIO);
    gpio_set_function(IMU_I2C_SCL_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(IMU_I2C_SDA_GPIO, GPIO_IN);
    gpio_set_dir(IMU_I2C_SCL_GPIO, GPIO_IN);
}

imu_hal_status_t imu_hal_i2c_write(uint8_t addr7, uint8_t const * p_data,
                                   size_t len, uint32_t timeout_us)
{
    int result;

    if ((NULL == p_data) || (0U == len))
    {
        return (IMU_HAL_ERROR);
    }

    result = i2c_write_timeout_us(IMU_I2C_PORT, addr7, p_data, len,
                                  false, timeout_us);

    return (hal_map_result(result, len));
}

imu_hal_status_t imu_hal_i2c_write_read(uint8_t addr7,
                                        uint8_t const * p_tx, size_t tx_len,
                                        uint8_t * p_rx, size_t rx_len,
                                        uint32_t timeout_us)
{
    int              result;
    imu_hal_status_t status;

    if ((NULL == p_tx) || (NULL == p_rx) || (0U == tx_len) || (0U == rx_len))
    {
        return (IMU_HAL_ERROR);
    }

    /* nostop = true -> repeated START before the read phase. */
    result = i2c_write_timeout_us(IMU_I2C_PORT, addr7, p_tx, tx_len,
                                  true, timeout_us);
    status = hal_map_result(result, tx_len);

    if (IMU_HAL_OK == status)
    {
        result = i2c_read_timeout_us(IMU_I2C_PORT, addr7, p_rx, rx_len,
                                     false, timeout_us);
        status = hal_map_result(result, rx_len);
    }

    return (status);
}

void imu_hal_i2c_bus_recover(void)
{
    uint32_t pulse;

    i2c_deinit(IMU_I2C_PORT);

    /* Release both lines (open-drain emulation: input = released). */
    gpio_init(IMU_I2C_SDA_GPIO);
    gpio_init(IMU_I2C_SCL_GPIO);
    gpio_pull_up(IMU_I2C_SDA_GPIO);
    gpio_pull_up(IMU_I2C_SCL_GPIO);
    gpio_set_dir(IMU_I2C_SDA_GPIO, GPIO_IN);
    gpio_set_dir(IMU_I2C_SCL_GPIO, GPIO_IN);
    gpio_put(IMU_I2C_SCL_GPIO, false);
    gpio_put(IMU_I2C_SDA_GPIO, false);
    busy_wait_us_32(HAL_RECOVER_HALF_US);

    for (pulse = 0U; pulse < HAL_RECOVER_CLOCKS; pulse++)
    {
        if (gpio_get(IMU_I2C_SDA_GPIO))
        {
            break;   /* slave released SDA */
        }
        gpio_set_dir(IMU_I2C_SCL_GPIO, GPIO_OUT);      /* SCL low  */
        busy_wait_us_32(HAL_RECOVER_HALF_US);
        gpio_set_dir(IMU_I2C_SCL_GPIO, GPIO_IN);       /* SCL high */
        busy_wait_us_32(HAL_RECOVER_HALF_US);
    }

    /* STOP condition: SDA low -> high while SCL is high. */
    gpio_set_dir(IMU_I2C_SDA_GPIO, GPIO_OUT);
    busy_wait_us_32(HAL_RECOVER_HALF_US);
    gpio_set_dir(IMU_I2C_SDA_GPIO, GPIO_IN);
    busy_wait_us_32(HAL_RECOVER_HALF_US);

    (void)imu_hal_i2c_init(g_hal_baud_hz);
}

uint32_t imu_hal_time_ms(void)
{
    return (to_ms_since_boot(get_absolute_time()));
}

uint32_t imu_hal_time_us(void)
{
    return (time_us_32());
}
```

## `include/imu/imu_lsm303dlhc.h`

```c
/**
 * @file    imu_lsm303dlhc.h
 * @brief   Low-level LSM303DLHC accelerometer + magnetometer driver.
 *
 * Responsibilities: register configuration, identity verification, data
 * acquisition (FIFO or single sample), byte-order/sign decoding, unit
 * conversion, bounded retries, error accounting and bus/device recovery.
 *
 * NOT responsible for: calibration, filtering, motion/terrain logic, RTOS.
 * The LSM303DLHC has NO gyroscope; this driver exposes no angular-rate data.
 */

#ifndef IMU_LSM303DLHC_H
#define IMU_LSM303DLHC_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_hal.h"
#include "imu/imu_types.h"

/** Driver configuration (a subset of imu_params_t). */
typedef struct
{
    uint8_t  accel_odr_code;
    uint8_t  accel_fs_code;
    bool     b_accel_fifo;
    uint8_t  mag_odr_code;
    uint8_t  mag_gain_code;
    bool     b_mag_id_check;
    uint32_t i2c_baud_hz;
    uint32_t i2c_timeout_us;
    uint8_t  i2c_max_retries;
} lsm303_config_t;

/** Driver error / health counters (monotonic, wrap at 2^32). */
typedef struct
{
    uint32_t transfers;
    uint32_t failed_transfers;     /**< after all retries          */
    uint32_t retries;              /**< individual retry attempts  */
    uint32_t nacks;
    uint32_t timeouts;
    uint32_t fifo_overruns;
    uint32_t accel_overruns;       /**< STATUS_REG_A.ZYXOR         */
    uint32_t mag_overflows;
    uint32_t recoveries;
    imu_hal_status_t last_status;
} lsm303_stats_t;

/** Driver instance. */
typedef struct
{
    lsm303_config_t cfg;
    lsm303_stats_t  stats;
    float           accel_g_per_lsb;    /* after the >>4 justification */
    float           mag_xy_gauss_per_lsb;
    float           mag_z_gauss_per_lsb;
    float           accel_full_scale_g;
    bool            b_ready;
} lsm303_t;

/**
 * @brief  Initialise the I2C bus, verify identity and configure both
 *         sensors. Configuration registers are read back and compared.
 * @return IMU_OK, IMU_ERR_PARAM, IMU_ERR_I2C, IMU_ERR_TIMEOUT, IMU_ERR_ID,
 *         IMU_ERR_VERIFY.
 */
imu_err_t lsm303_init(lsm303_t * p_dev, lsm303_config_t const * p_cfg);

/**
 * @brief  Read all available accelerometer samples.
 *
 * FIFO mode: reads FIFO_SRC_REG_A and then every stored sample (max
 * @p max_samples); oldest first. Single mode: checks STATUS_REG_A.ZYXDA and
 * reads one sample.
 *
 * @param[out] p_samples    raw samples (12-bit, already right-justified)
 * @param[out] p_count      number of samples written (0 if none new)
 * @param[out] p_b_overrun  true if samples were lost (FIFO/ZYXOR overrun)
 * @return IMU_OK (count may be 0), IMU_ERR_I2C, IMU_ERR_TIMEOUT, ...
 */
imu_err_t lsm303_read_accel(lsm303_t * p_dev, imu_raw3_t * p_samples,
                            uint8_t max_samples, uint8_t * p_count,
                            bool * p_b_overrun);

/**
 * @brief  Read one magnetometer sample (big-endian X, Z, Y register order
 *         is reordered to X, Y, Z here).
 * @param[out] p_b_overflow  true if any axis reported ADC overflow (-4096)
 */
imu_err_t lsm303_read_mag(lsm303_t * p_dev, imu_raw3_t * p_raw,
                          bool * p_b_overflow);

/** @brief  Raw accelerometer counts -> g (sensor frame). */
void lsm303_accel_to_g(lsm303_t const * p_dev, imu_raw3_t const * p_raw,
                       imu_vec3_t * p_out);

/** @brief  Raw magnetometer counts -> gauss (sensor frame). */
void lsm303_mag_to_gauss(lsm303_t const * p_dev, imu_raw3_t const * p_raw,
                         imu_vec3_t * p_out);

/**
 * @brief  Recovery: bus-clear, re-initialise the peripheral, re-verify and
 *         re-configure the device. Bounded in time.
 */
imu_err_t lsm303_recover(lsm303_t * p_dev);

/** @brief  Put the magnetometer to sleep and release the bus. */
void lsm303_shutdown(lsm303_t * p_dev);

/* ---- Pure helpers, exported for unit testing ------------------------- */

/** Little-endian, left-justified 12-bit accelerometer word -> counts. */
int16_t lsm303_decode_accel_axis(uint8_t lo, uint8_t hi);

/** Big-endian 16-bit magnetometer word -> counts. */
int16_t lsm303_decode_mag_axis(uint8_t hi, uint8_t lo);

/** @return mg per LSB in high-resolution mode, 0 for an invalid code. */
float lsm303_accel_sensitivity_g(uint8_t fs_code);

/** @return LSB/gauss for X/Y (b_z = false) or Z, 0 for an invalid code. */
float lsm303_mag_lsb_per_gauss(uint8_t gain_code, bool b_z);

/** @return imu_err_t equivalent of a HAL status. */
imu_err_t lsm303_map_hal(imu_hal_status_t status);

#endif /* IMU_LSM303DLHC_H */
```

## `src/imu/imu_lsm303dlhc.c`

```c
/**
 * @file    imu_lsm303dlhc.c
 * @brief   Low-level LSM303DLHC driver (see imu_lsm303dlhc.h).
 */

#include <stddef.h>
#include <string.h>

#include "imu/imu_lsm303dlhc.h"
#include "imu/lsm303dlhc_regs.h"

#define ACCEL_SAMPLE_BYTES     (6U)
#define MAG_SAMPLE_BYTES       (6U)
#define MAG_ID_BYTES           (3U)
#define ACCEL_JUSTIFY_DIV      (16)     /* 12-bit left-justified -> >> 4 */

/* ------------------------------------------------------------------------ */
/*  Pure helpers                                                             */
/* ------------------------------------------------------------------------ */

int16_t lsm303_decode_accel_axis(uint8_t lo, uint8_t hi)
{
    int32_t word = (int32_t)(int16_t)(uint16_t)(((uint16_t)hi << 8) | lo);

    /* Portable arithmetic right shift by 4 (floor division by 16). */
    if (word < 0)
    {
        word = (word - (ACCEL_JUSTIFY_DIV - 1)) / ACCEL_JUSTIFY_DIV;
    }
    else
    {
        word = word / ACCEL_JUSTIFY_DIV;
    }

    return ((int16_t)word);
}

int16_t lsm303_decode_mag_axis(uint8_t hi, uint8_t lo)
{
    return ((int16_t)(uint16_t)(((uint16_t)hi << 8) | lo));
}

float lsm303_accel_sensitivity_g(uint8_t fs_code)
{
    float sens;

    /* Datasheet Table 3, high-resolution (12-bit) mode. */
    switch (fs_code)
    {
        case LSM303_ACCEL_FS_2G:  sens = 0.001f; break;
        case LSM303_ACCEL_FS_4G:  sens = 0.002f; break;
        case LSM303_ACCEL_FS_8G:  sens = 0.004f; break;
        case LSM303_ACCEL_FS_16G: sens = 0.012f; break;
        default:                  sens = 0.0f;   break;
    }

    return (sens);
}

static float lsm303_accel_full_scale(uint8_t fs_code)
{
    float fs;

    switch (fs_code)
    {
        case LSM303_ACCEL_FS_2G:  fs = 2.0f;  break;
        case LSM303_ACCEL_FS_4G:  fs = 4.0f;  break;
        case LSM303_ACCEL_FS_8G:  fs = 8.0f;  break;
        case LSM303_ACCEL_FS_16G: fs = 16.0f; break;
        default:                  fs = 0.0f;  break;
    }

    return (fs);
}

float lsm303_mag_lsb_per_gauss(uint8_t gain_code, bool b_z)
{
    /* Datasheet Table 3 / Table 75 (GN bits), X/Y and Z gains differ. */
    static float const k_xy[8] = { 0.0f, 1100.0f, 855.0f, 670.0f,
                                   450.0f, 400.0f, 330.0f, 230.0f };
    static float const k_z[8]  = { 0.0f, 980.0f, 760.0f, 600.0f,
                                   400.0f, 355.0f, 295.0f, 205.0f };
    float lsb = 0.0f;

    if (gain_code < 8U)
    {
        lsb = b_z ? k_z[gain_code] : k_xy[gain_code];
    }

    return (lsb);
}

imu_err_t lsm303_map_hal(imu_hal_status_t status)
{
    imu_err_t err;

    switch (status)
    {
        case IMU_HAL_OK:      err = IMU_OK;          break;
        case IMU_HAL_TIMEOUT: err = IMU_ERR_TIMEOUT; break;
        default:              err = IMU_ERR_I2C;     break;
    }

    return (err);
}

/* ------------------------------------------------------------------------ */
/*  Bus access with bounded retries                                          */
/* ------------------------------------------------------------------------ */

static void lsm303_account(lsm303_t * p_dev, imu_hal_status_t status)
{
    p_dev->stats.last_status = status;

    if (IMU_HAL_NACK == status)
    {
        p_dev->stats.nacks++;
    }
    else if (IMU_HAL_TIMEOUT == status)
    {
        p_dev->stats.timeouts++;
    }
    else
    {
        /* OK or generic error: nothing extra to count */
    }
}

static imu_err_t lsm303_read_regs(lsm303_t * p_dev, uint8_t addr7,
                                  uint8_t reg, uint8_t * p_buf, size_t len)
{
    imu_hal_status_t status = IMU_HAL_ERROR;
    uint8_t          attempt;

    p_dev->stats.transfers++;

    for (attempt = 0U; attempt <= p_dev->cfg.i2c_max_retries; attempt++)
    {
        if (attempt > 0U)
        {
            p_dev->stats.retries++;
        }

        status = imu_hal_i2c_write_read(addr7, &reg, 1U, p_buf, len,
                                        p_dev->cfg.i2c_timeout_us);
        lsm303_account(p_dev, status);

        if (IMU_HAL_OK == status)
        {
            break;
        }
    }

    if (IMU_HAL_OK != status)
    {
        p_dev->stats.failed_transfers++;
    }

    return (lsm303_map_hal(status));
}

static imu_err_t lsm303_write_reg(lsm303_t * p_dev, uint8_t addr7,
                                  uint8_t reg, uint8_t value)
{
    uint8_t const    buf[2] = { reg, value };
    imu_hal_status_t status = IMU_HAL_ERROR;
    uint8_t          attempt;

    p_dev->stats.transfers++;

    for (attempt = 0U; attempt <= p_dev->cfg.i2c_max_retries; attempt++)
    {
        if (attempt > 0U)
        {
            p_dev->stats.retries++;
        }

        status = imu_hal_i2c_write(addr7, buf, sizeof(buf),
                                   p_dev->cfg.i2c_timeout_us);
        lsm303_account(p_dev, status);

        if (IMU_HAL_OK == status)
        {
            break;
        }
    }

    if (IMU_HAL_OK != status)
    {
        p_dev->stats.failed_transfers++;
    }

    return (lsm303_map_hal(status));
}

/** Write a register and read it back (catches wrong device / bad wiring). */
static imu_err_t lsm303_write_verify(lsm303_t * p_dev, uint8_t addr7,
                                     uint8_t reg, uint8_t value)
{
    uint8_t   readback = 0U;
    imu_err_t err      = lsm303_write_reg(p_dev, addr7, reg, value);

    if (IMU_OK == err)
    {
        err = lsm303_read_regs(p_dev, addr7, reg, &readback, 1U);
    }

    if ((IMU_OK == err) && (readback != value))
    {
        err = IMU_ERR_VERIFY;
    }

    return (err);
}

/* ------------------------------------------------------------------------ */
/*  Configuration                                                            */
/* ------------------------------------------------------------------------ */

static imu_err_t lsm303_configure(lsm303_t * p_dev)
{
    uint8_t   id[MAG_ID_BYTES] = { 0U, 0U, 0U };
    uint8_t   ctrl1;
    uint8_t   ctrl4;
    imu_err_t err;

    /* 1. Magnetometer identity ('H','4','3'); auto-increment is implicit. */
    err = lsm303_read_regs(p_dev, LSM303_MAG_ADDR, LSM303_REG_IRA_REG_M,
                           id, sizeof(id));
    if ((IMU_OK == err) && p_dev->cfg.b_mag_id_check)
    {
        if ((LSM303_IRA_VALUE != id[0]) || (LSM303_IRB_VALUE != id[1])
            || (LSM303_IRC_VALUE != id[2]))
        {
            err = IMU_ERR_ID;
        }
    }

    /* 2. Accelerometer: ODR + all axes, BDU + FS + HR, FIFO. Read-back of
     *    CTRL_REG1_A/CTRL_REG4_A doubles as the accelerometer presence check
     *    (the DLHC accelerometer has no documented WHO_AM_I). */
    ctrl1 = (uint8_t)((uint8_t)(p_dev->cfg.accel_odr_code
                                << LSM303_CTRL1_ODR_SHIFT)
                      | LSM303_CTRL1_XYZ_EN);
    ctrl4 = (uint8_t)(LSM303_CTRL4_BDU
                      | (uint8_t)(p_dev->cfg.accel_fs_code
                                  << LSM303_CTRL4_FS_SHIFT)
                      | LSM303_CTRL4_HR);

    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_ACCEL_ADDR,
                                  LSM303_REG_CTRL_REG1_A, ctrl1);
    }
    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_ACCEL_ADDR,
                                  LSM303_REG_CTRL_REG4_A, ctrl4);
    }
    if (IMU_OK == err)
    {
        /* Bypass first: clears any stale FIFO content. */
        err = lsm303_write_reg(p_dev, LSM303_ACCEL_ADDR,
                               LSM303_REG_FIFO_CTRL_REG_A,
                               LSM303_FIFO_MODE_BYPASS);
    }
    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_ACCEL_ADDR,
                                  LSM303_REG_CTRL_REG5_A,
                                  p_dev->cfg.b_accel_fifo
                                      ? LSM303_CTRL5_FIFO_EN : 0U);
    }
    if ((IMU_OK == err) && p_dev->cfg.b_accel_fifo)
    {
        err = lsm303_write_verify(p_dev, LSM303_ACCEL_ADDR,
                                  LSM303_REG_FIFO_CTRL_REG_A,
                                  LSM303_FIFO_MODE_STREAM);
    }

    /* 3. Magnetometer: ODR, gain, continuous conversion. */
    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_MAG_ADDR,
                                  LSM303_REG_CRA_REG_M,
                                  (uint8_t)(p_dev->cfg.mag_odr_code
                                            << LSM303_CRA_DO_SHIFT));
    }
    if (IMU_OK == err)
    {
        err = lsm303_write_verify(p_dev, LSM303_MAG_ADDR,
                                  LSM303_REG_CRB_REG_M,
                                  (uint8_t)(p_dev->cfg.mag_gain_code
                                            << LSM303_CRB_GN_SHIFT));
    }
    if (IMU_OK == err)
    {
        err = lsm303_write_reg(p_dev, LSM303_MAG_ADDR, LSM303_REG_MR_REG_M,
                               LSM303_MR_CONTINUOUS);
    }

    return (err);
}

imu_err_t lsm303_init(lsm303_t * p_dev, lsm303_config_t const * p_cfg)
{
    imu_err_t err;

    if ((NULL == p_dev) || (NULL == p_cfg))
    {
        return (IMU_ERR_PARAM);
    }

    if ((0.0f >= lsm303_accel_sensitivity_g(p_cfg->accel_fs_code))
        || (0.0f >= lsm303_mag_lsb_per_gauss(p_cfg->mag_gain_code, false))
        || (p_cfg->accel_odr_code > LSM303_ACCEL_ODR_400HZ)
        || (0U == p_cfg->accel_odr_code)
        || (p_cfg->mag_odr_code > LSM303_MAG_ODR_220HZ))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_dev, 0, sizeof(*p_dev));
    p_dev->cfg = *p_cfg;
    p_dev->accel_g_per_lsb      =
        lsm303_accel_sensitivity_g(p_cfg->accel_fs_code);
    p_dev->accel_full_scale_g   = lsm303_accel_full_scale(p_cfg->accel_fs_code);
    p_dev->mag_xy_gauss_per_lsb =
        1.0f / lsm303_mag_lsb_per_gauss(p_cfg->mag_gain_code, false);
    p_dev->mag_z_gauss_per_lsb  =
        1.0f / lsm303_mag_lsb_per_gauss(p_cfg->mag_gain_code, true);

    err = lsm303_map_hal(imu_hal_i2c_init(p_cfg->i2c_baud_hz));

    if (IMU_OK == err)
    {
        err = lsm303_configure(p_dev);
    }

    p_dev->b_ready = (IMU_OK == err);

    return (err);
}

imu_err_t lsm303_recover(lsm303_t * p_dev)
{
    imu_err_t err;

    if (NULL == p_dev)
    {
        return (IMU_ERR_PARAM);
    }

    p_dev->stats.recoveries++;
    p_dev->b_ready = false;

    imu_hal_i2c_bus_recover();
    err = lsm303_map_hal(imu_hal_i2c_init(p_dev->cfg.i2c_baud_hz));

    if (IMU_OK == err)
    {
        err = lsm303_configure(p_dev);
    }

    p_dev->b_ready = (IMU_OK == err);

    return (err);
}

void lsm303_shutdown(lsm303_t * p_dev)
{
    if (NULL == p_dev)
    {
        return;
    }

    if (p_dev->b_ready)
    {
        (void)lsm303_write_reg(p_dev, LSM303_MAG_ADDR, LSM303_REG_MR_REG_M,
                               LSM303_MR_SLEEP);
        (void)lsm303_write_reg(p_dev, LSM303_ACCEL_ADDR,
                               LSM303_REG_CTRL_REG1_A, 0U); /* power-down */
    }

    p_dev->b_ready = false;
    imu_hal_i2c_deinit();
}

/* ------------------------------------------------------------------------ */
/*  Data acquisition                                                         */
/* ------------------------------------------------------------------------ */

static imu_err_t lsm303_read_one_accel(lsm303_t * p_dev, imu_raw3_t * p_out)
{
    uint8_t   buf[ACCEL_SAMPLE_BYTES];
    imu_err_t err;

    err = lsm303_read_regs(p_dev, LSM303_ACCEL_ADDR,
                           (uint8_t)(LSM303_REG_OUT_X_L_A
                                     | LSM303_ACCEL_AUTO_INC),
                           buf, sizeof(buf));
    if (IMU_OK == err)
    {
        p_out->x = lsm303_decode_accel_axis(buf[0], buf[1]);
        p_out->y = lsm303_decode_accel_axis(buf[2], buf[3]);
        p_out->z = lsm303_decode_accel_axis(buf[4], buf[5]);
    }

    return (err);
}

imu_err_t lsm303_read_accel(lsm303_t * p_dev, imu_raw3_t * p_samples,
                            uint8_t max_samples, uint8_t * p_count,
                            bool * p_b_overrun)
{
    uint8_t   reg   = 0U;
    uint8_t   avail = 0U;
    uint8_t   i;
    imu_err_t err;

    if ((NULL == p_dev) || (NULL == p_samples) || (NULL == p_count)
        || (NULL == p_b_overrun) || (0U == max_samples))
    {
        return (IMU_ERR_PARAM);
    }

    *p_count     = 0U;
    *p_b_overrun = false;

    if (!p_dev->b_ready)
    {
        return (IMU_ERR_NOT_READY);
    }

    if (p_dev->cfg.b_accel_fifo)
    {
        err = lsm303_read_regs(p_dev, LSM303_ACCEL_ADDR,
                               LSM303_REG_FIFO_SRC_REG_A, &reg, 1U);
        if (IMU_OK == err)
        {
            if (0U != (reg & LSM303_FIFO_SRC_OVRN))
            {
                /* FIFO full: 32 samples stored, older ones were lost. */
                avail = (uint8_t)LSM303_FIFO_DEPTH;
                *p_b_overrun = true;
                p_dev->stats.fifo_overruns++;
            }
            else if (0U != (reg & LSM303_FIFO_SRC_EMPTY))
            {
                avail = 0U;
            }
            else
            {
                avail = (uint8_t)(reg & LSM303_FIFO_SRC_FSS_MASK);
            }
        }
    }
    else
    {
        err = lsm303_read_regs(p_dev, LSM303_ACCEL_ADDR,
                               LSM303_REG_STATUS_REG_A, &reg, 1U);
        if (IMU_OK == err)
        {
            avail = (0U != (reg & LSM303_STATUS_ZYXDA)) ? 1U : 0U;
            if (0U != (reg & LSM303_STATUS_ZYXOR))
            {
                *p_b_overrun = true;
                p_dev->stats.accel_overruns++;
            }
        }
    }

    if (avail > max_samples)
    {
        avail = max_samples;   /* the rest stays in the FIFO */
    }

    for (i = 0U; (IMU_OK == err) && (i < avail); i++)
    {
        err = lsm303_read_one_accel(p_dev, &p_samples[i]);
        if (IMU_OK == err)
        {
            *p_count = (uint8_t)(i + 1U);
        }
    }

    return (err);
}

imu_err_t lsm303_read_mag(lsm303_t * p_dev, imu_raw3_t * p_raw,
                          bool * p_b_overflow)
{
    uint8_t   buf[MAG_SAMPLE_BYTES];
    imu_err_t err;

    if ((NULL == p_dev) || (NULL == p_raw) || (NULL == p_b_overflow))
    {
        return (IMU_ERR_PARAM);
    }

    *p_b_overflow = false;

    if (!p_dev->b_ready)
    {
        return (IMU_ERR_NOT_READY);
    }

    err = lsm303_read_regs(p_dev, LSM303_MAG_ADDR, LSM303_REG_OUT_X_H_M,
                           buf, sizeof(buf));
    if (IMU_OK == err)
    {
        /* Register order is X, Z, Y (big-endian). */
        p_raw->x = lsm303_decode_mag_axis(buf[0], buf[1]);
        p_raw->z = lsm303_decode_mag_axis(buf[2], buf[3]);
        p_raw->y = lsm303_decode_mag_axis(buf[4], buf[5]);

        if ((LSM303_MAG_OVERFLOW_VALUE == p_raw->x)
            || (LSM303_MAG_OVERFLOW_VALUE == p_raw->y)
            || (LSM303_MAG_OVERFLOW_VALUE == p_raw->z))
        {
            *p_b_overflow = true;
            p_dev->stats.mag_overflows++;
        }
    }

    return (err);
}

void lsm303_accel_to_g(lsm303_t const * p_dev, imu_raw3_t const * p_raw,
                       imu_vec3_t * p_out)
{
    if ((NULL == p_dev) || (NULL == p_raw) || (NULL == p_out))
    {
        return;
    }

    p_out->x = (float)p_raw->x * p_dev->accel_g_per_lsb;
    p_out->y = (float)p_raw->y * p_dev->accel_g_per_lsb;
    p_out->z = (float)p_raw->z * p_dev->accel_g_per_lsb;
}

void lsm303_mag_to_gauss(lsm303_t const * p_dev, imu_raw3_t const * p_raw,
                         imu_vec3_t * p_out)
{
    if ((NULL == p_dev) || (NULL == p_raw) || (NULL == p_out))
    {
        return;
    }

    p_out->x = (float)p_raw->x * p_dev->mag_xy_gauss_per_lsb;
    p_out->y = (float)p_raw->y * p_dev->mag_xy_gauss_per_lsb;
    p_out->z = (float)p_raw->z * p_dev->mag_z_gauss_per_lsb;
}
```

## `include/imu/imu_math.h`

```c
/**
 * @file    imu_math.h
 * @brief   Small, allocation-free math helpers for the Buddy 4 subsystem
 *          (vector algebra, frame mapping, angle wrapping, IIR filters).
 */

#ifndef IMU_MATH_H
#define IMU_MATH_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_types.h"

#define IMU_PI_F            (3.14159265358979f)
#define IMU_RAD_TO_DEG      (57.2957795130823f)
#define IMU_DEG_TO_RAD      (0.0174532925199433f)

/** 3x3 rotation matrix, row-major. */
typedef struct
{
    float m[3][3];
} imu_mat3_t;

float imu_vec_norm(imu_vec3_t const * p_v);
float imu_vec_dot(imu_vec3_t const * p_a, imu_vec3_t const * p_b);
void  imu_vec_cross(imu_vec3_t const * p_a, imu_vec3_t const * p_b,
                    imu_vec3_t * p_out);
void  imu_vec_sub(imu_vec3_t const * p_a, imu_vec3_t const * p_b,
                  imu_vec3_t * p_out);
void  imu_vec_scale(imu_vec3_t const * p_a, float k, imu_vec3_t * p_out);
bool  imu_vec_is_finite(imu_vec3_t const * p_v);

/** @return false if |v| is too small to normalise (output unchanged). */
bool  imu_vec_normalise(imu_vec3_t const * p_v, imu_vec3_t * p_out);

void  imu_mat_identity(imu_mat3_t * p_m);
void  imu_mat_apply(imu_mat3_t const * p_m, imu_vec3_t const * p_v,
                    imu_vec3_t * p_out);

/**
 * @brief  Rotation that maps unit vector @p p_from onto +Z (Rodrigues).
 * @return false if @p p_from is (anti)parallel to -Z or not finite.
 */
bool  imu_mat_align_to_z(imu_vec3_t const * p_from, imu_mat3_t * p_m);

/**
 * @brief  Sensor frame -> vehicle frame using an axis permutation + signs.
 *         vehicle[k] = sign[k] * sensor[axis[k]].
 */
void  imu_frame_map(uint8_t const axis[3], int8_t const sign[3],
                    imu_vec3_t const * p_sensor, imu_vec3_t * p_vehicle);

/** Wrap an angle to [0, 360). */
float imu_wrap_360(float deg);

/** Wrap an angle difference to (-180, 180]. */
float imu_wrap_180(float deg);

/** First-order IIR coefficient for cut-off @p fc_hz at sample time @p dt_s. */
float imu_lpf_alpha(float fc_hz, float dt_s);

/** y += alpha * (x - y). */
float imu_lpf_step(float y, float x, float alpha);

/** Clamp helper. */
float imu_clampf(float x, float lo, float hi);

#endif /* IMU_MATH_H */
```

## `src/imu/imu_math.c`

```c
/**
 * @file    imu_math.c
 * @brief   Math helpers for Buddy 4 (see imu_math.h).
 */

#include <math.h>
#include <stddef.h>

#include "imu/imu_math.h"

#define MATH_MIN_NORM      (1.0e-6f)
#define MATH_PARALLEL_EPS  (1.0e-7f)

float imu_vec_norm(imu_vec3_t const * p_v)
{
    return (sqrtf((p_v->x * p_v->x) + (p_v->y * p_v->y)
                  + (p_v->z * p_v->z)));
}

float imu_vec_dot(imu_vec3_t const * p_a, imu_vec3_t const * p_b)
{
    return ((p_a->x * p_b->x) + (p_a->y * p_b->y) + (p_a->z * p_b->z));
}

void imu_vec_cross(imu_vec3_t const * p_a, imu_vec3_t const * p_b,
                   imu_vec3_t * p_out)
{
    imu_vec3_t r;

    r.x = (p_a->y * p_b->z) - (p_a->z * p_b->y);
    r.y = (p_a->z * p_b->x) - (p_a->x * p_b->z);
    r.z = (p_a->x * p_b->y) - (p_a->y * p_b->x);
    *p_out = r;
}

void imu_vec_sub(imu_vec3_t const * p_a, imu_vec3_t const * p_b,
                 imu_vec3_t * p_out)
{
    p_out->x = p_a->x - p_b->x;
    p_out->y = p_a->y - p_b->y;
    p_out->z = p_a->z - p_b->z;
}

void imu_vec_scale(imu_vec3_t const * p_a, float k, imu_vec3_t * p_out)
{
    p_out->x = p_a->x * k;
    p_out->y = p_a->y * k;
    p_out->z = p_a->z * k;
}

bool imu_vec_is_finite(imu_vec3_t const * p_v)
{
    return ((NULL != p_v) && (0 != isfinite(p_v->x))
            && (0 != isfinite(p_v->y)) && (0 != isfinite(p_v->z)));
}

bool imu_vec_normalise(imu_vec3_t const * p_v, imu_vec3_t * p_out)
{
    float n;

    if (!imu_vec_is_finite(p_v))
    {
        return (false);
    }

    n = imu_vec_norm(p_v);
    if ((0 == isfinite(n)) || (n < MATH_MIN_NORM))
    {
        return (false);
    }

    imu_vec_scale(p_v, 1.0f / n, p_out);
    return (true);
}

void imu_mat_identity(imu_mat3_t * p_m)
{
    uint8_t r;
    uint8_t c;

    for (r = 0U; r < 3U; r++)
    {
        for (c = 0U; c < 3U; c++)
        {
            p_m->m[r][c] = (r == c) ? 1.0f : 0.0f;
        }
    }
}

void imu_mat_apply(imu_mat3_t const * p_m, imu_vec3_t const * p_v,
                   imu_vec3_t * p_out)
{
    imu_vec3_t r;

    r.x = (p_m->m[0][0] * p_v->x) + (p_m->m[0][1] * p_v->y)
        + (p_m->m[0][2] * p_v->z);
    r.y = (p_m->m[1][0] * p_v->x) + (p_m->m[1][1] * p_v->y)
        + (p_m->m[1][2] * p_v->z);
    r.z = (p_m->m[2][0] * p_v->x) + (p_m->m[2][1] * p_v->y)
        + (p_m->m[2][2] * p_v->z);
    *p_out = r;
}

bool imu_mat_align_to_z(imu_vec3_t const * p_from, imu_mat3_t * p_m)
{
    static imu_vec3_t const k_z = { 0.0f, 0.0f, 1.0f };
    imu_vec3_t u;
    imu_vec3_t v;
    float      c;
    float      k;
    float      vx[3][3];
    uint8_t    r;
    uint8_t    col;
    uint8_t    i;

    if (!imu_vec_normalise(p_from, &u))
    {
        return (false);
    }

    c = imu_vec_dot(&u, &k_z);
    if (c > (1.0f - MATH_PARALLEL_EPS))
    {
        imu_mat_identity(p_m);
        return (true);
    }
    if (c < (-1.0f + MATH_PARALLEL_EPS))
    {
        return (false);   /* upside-down: rotation axis undefined */
    }

    imu_vec_cross(&u, &k_z, &v);
    k = 1.0f / (1.0f + c);   /* (1 - c) / s^2 with s^2 = 1 - c^2 */

    /* Skew-symmetric matrix [v]x */
    vx[0][0] = 0.0f;  vx[0][1] = -v.z;  vx[0][2] = v.y;
    vx[1][0] = v.z;   vx[1][1] = 0.0f;  vx[1][2] = -v.x;
    vx[2][0] = -v.y;  vx[2][1] = v.x;   vx[2][2] = 0.0f;

    /* R = I + [v]x + [v]x^2 * k */
    for (r = 0U; r < 3U; r++)
    {
        for (col = 0U; col < 3U; col++)
        {
            float sq = 0.0f;

            for (i = 0U; i < 3U; i++)
            {
                sq += vx[r][i] * vx[i][col];
            }
            p_m->m[r][col] = ((r == col) ? 1.0f : 0.0f) + vx[r][col]
                           + (sq * k);
        }
    }

    return (true);
}

void imu_frame_map(uint8_t const axis[3], int8_t const sign[3],
                   imu_vec3_t const * p_sensor, imu_vec3_t * p_vehicle)
{
    float const s[3] = { p_sensor->x, p_sensor->y, p_sensor->z };
    float       v[3];
    uint8_t     k;

    for (k = 0U; k < 3U; k++)
    {
        uint8_t a = (axis[k] < 3U) ? axis[k] : k;

        v[k] = (float)sign[k] * s[a];
    }

    p_vehicle->x = v[0];
    p_vehicle->y = v[1];
    p_vehicle->z = v[2];
}

float imu_wrap_360(float deg)
{
    float r = fmodf(deg, 360.0f);

    if (r < 0.0f)
    {
        r += 360.0f;
    }
    if (r >= 360.0f)
    {
        r = 0.0f;
    }

    return (r);
}

float imu_wrap_180(float deg)
{
    float r = fmodf(deg, 360.0f);

    if (r > 180.0f)
    {
        r -= 360.0f;
    }
    else if (r <= -180.0f)
    {
        r += 360.0f;
    }
    else
    {
        /* already in range */
    }

    return (r);
}

float imu_lpf_alpha(float fc_hz, float dt_s)
{
    float rc;
    float alpha = 1.0f;

    if ((fc_hz > 0.0f) && (dt_s > 0.0f))
    {
        rc    = 1.0f / (2.0f * IMU_PI_F * fc_hz);
        alpha = dt_s / (rc + dt_s);
    }

    return (alpha);
}

float imu_lpf_step(float y, float x, float alpha)
{
    return (y + (alpha * (x - y)));
}

float imu_clampf(float x, float lo, float hi)
{
    float r = x;

    if (r < lo)
    {
        r = lo;
    }
    if (r > hi)
    {
        r = hi;
    }

    return (r);
}
```

## `include/imu/imu_calibration.h`

```c
/**
 * @file    imu_calibration.h
 * @brief   Buddy 4 sensor calibration.
 *
 * 1. Stationary accelerometer calibration (run at power-up, vehicle still on
 *    level ground, motors OFF):
 *      - mean specific force  -> gravity / mounting-orientation reference
 *      - |mean|               -> 1 g reference (removes scale/bias error along
 *                                gravity, i.e. vertical offset compensation)
 *      - per-axis std dev     -> noise estimate (used to scale thresholds)
 *      - levelling rotation   -> maps the measured gravity onto vehicle +Z,
 *                                so the sensor does not have to be perfectly
 *                                aligned with the chassis.
 *    LIMITATION: with a single orientation, a constant sensor bias is
 *    indistinguishable from a small mounting tilt. Both are removed together
 *    (which is exactly what tilt/hump detection needs). A per-axis bias from
 *    an optional 6-position bench calibration can be supplied in
 *    imu_params_t.accel_bias_g.
 *
 * 2. Magnetometer hard-/soft-iron calibration (min/max ellipse fit in the
 *    horizontal plane). Samples are collected while the vehicle spins slowly
 *    in place WITH MOTORS RUNNING (so motor magnets and typical motor current
 *    fields are included). Only the two horizontal sensor axes can be
 *    calibrated from a planar rotation; the vertical-axis offset is taken
 *    from imu_params_t (default 0) - heading only depends weakly on it after
 *    tilt compensation at small tilt angles.
 */

#ifndef IMU_CALIBRATION_H
#define IMU_CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_config.h"
#include "imu/imu_math.h"
#include "imu/imu_types.h"

/** Result of the stationary accelerometer calibration. */
typedef struct
{
    bool        b_valid;
    uint32_t    samples;
    imu_vec3_t  gravity_mean_g;     /**< axis-mapped, bias-removed mean    */
    float       g_ref;              /**< |gravity_mean_g| (should be ~1)   */
    float       mount_tilt_deg;     /**< angle between gravity and +Z      */
    float       mount_pitch_deg;    /**< informative                       */
    float       mount_roll_deg;     /**< informative                       */
    imu_mat3_t  level_rot;          /**< maps gravity_mean_g onto +Z       */
    imu_vec3_t  noise_std_g;        /**< per vehicle axis                  */
    float       noise_max_std_g;    /**< max of the three                  */
} imu_accel_cal_t;

/** Running statistics for the stationary calibration (Welford). */
typedef struct
{
    uint32_t   n;
    imu_vec3_t mean;
    imu_vec3_t m2;
} imu_accel_cal_acc_t;

/** Magnetometer calibration result (sensor frame, gauss). */
typedef struct
{
    bool       b_valid;
    uint32_t   samples;
    imu_vec3_t hard_iron_gauss;
    imu_vec3_t soft_iron_scale;
    imu_vec3_t span_gauss;
    float      ref_field_gauss;     /**< expected |B| after correction     */
} imu_mag_cal_t;

/** Running min/max accumulator for magnetometer calibration. */
typedef struct
{
    uint32_t   n;
    imu_vec3_t min;
    imu_vec3_t max;
    imu_vec3_t sum;
} imu_mag_cal_acc_t;

/* ---- accelerometer ---------------------------------------------------- */
void      imu_cal_accel_begin(imu_accel_cal_acc_t * p_acc);

/** @param p_vehicle_g  axis-mapped, bias-removed sample (not levelled). */
void      imu_cal_accel_add(imu_accel_cal_acc_t * p_acc,
                            imu_vec3_t const * p_vehicle_g);

/**
 * @return IMU_OK, IMU_ERR_CAL_SAMPLES (too few), IMU_ERR_CAL_MOTION (noise
 *         above tolerance: vehicle moving or being handled), IMU_ERR_CAL_RANGE
 *         (|g| or mounting tilt out of range), IMU_ERR_PARAM.
 */
imu_err_t imu_cal_accel_finish(imu_accel_cal_acc_t const * p_acc,
                               imu_params_t const * p_params,
                               imu_accel_cal_t * p_out);

/* ---- magnetometer ----------------------------------------------------- */
void      imu_cal_mag_begin(imu_mag_cal_acc_t * p_acc);
void      imu_cal_mag_add(imu_mag_cal_acc_t * p_acc,
                          imu_vec3_t const * p_sensor_gauss);

/**
 * @return IMU_OK, IMU_ERR_CAL_SAMPLES, IMU_ERR_CAL_COVERAGE (vehicle did not
 *         rotate enough / ellipse implausible), IMU_ERR_PARAM.
 */
imu_err_t imu_cal_mag_finish(imu_mag_cal_acc_t const * p_acc,
                             imu_params_t const * p_params,
                             imu_mag_cal_t * p_out);

/** Load a stored magnetometer calibration from the parameter block. */
void      imu_cal_mag_from_params(imu_params_t const * p_params,
                                  imu_mag_cal_t * p_out);

/** Apply hard-/soft-iron correction (sensor frame). */
void      imu_cal_mag_apply(imu_mag_cal_t const * p_cal,
                            imu_vec3_t const * p_raw_gauss,
                            imu_vec3_t * p_out);

#endif /* IMU_CALIBRATION_H */
```

## `src/imu/imu_calibration.c`

```c
/**
 * @file    imu_calibration.c
 * @brief   Buddy 4 accelerometer and magnetometer calibration.
 */

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "imu/imu_calibration.h"

#define CAL_MIN_SAMPLES       (10U)
#define CAL_MAX_ELLIPSE_RATIO (2.0f)

static float vec_get(imu_vec3_t const * p_v, uint8_t axis)
{
    float r;

    switch (axis)
    {
        case 0U:  r = p_v->x; break;
        case 1U:  r = p_v->y; break;
        default:  r = p_v->z; break;
    }

    return (r);
}

static void vec_set(imu_vec3_t * p_v, uint8_t axis, float value)
{
    switch (axis)
    {
        case 0U:  p_v->x = value; break;
        case 1U:  p_v->y = value; break;
        default:  p_v->z = value; break;
    }
}

/* ------------------------------------------------------------------------ */
/*  Accelerometer                                                            */
/* ------------------------------------------------------------------------ */

void imu_cal_accel_begin(imu_accel_cal_acc_t * p_acc)
{
    if (NULL != p_acc)
    {
        (void)memset(p_acc, 0, sizeof(*p_acc));
    }
}

static void welford(float * p_mean, float * p_m2, float x, uint32_t n)
{
    float delta = x - *p_mean;

    *p_mean += delta / (float)n;
    *p_m2   += delta * (x - *p_mean);
}

void imu_cal_accel_add(imu_accel_cal_acc_t * p_acc,
                       imu_vec3_t const * p_vehicle_g)
{
    if ((NULL == p_acc) || !imu_vec_is_finite(p_vehicle_g))
    {
        return;   /* non-finite samples are ignored, not accumulated */
    }

    p_acc->n++;
    welford(&p_acc->mean.x, &p_acc->m2.x, p_vehicle_g->x, p_acc->n);
    welford(&p_acc->mean.y, &p_acc->m2.y, p_vehicle_g->y, p_acc->n);
    welford(&p_acc->mean.z, &p_acc->m2.z, p_vehicle_g->z, p_acc->n);
}

imu_err_t imu_cal_accel_finish(imu_accel_cal_acc_t const * p_acc,
                               imu_params_t const * p_params,
                               imu_accel_cal_t * p_out)
{
    imu_vec3_t const z_axis = { 0.0f, 0.0f, 1.0f };
    imu_vec3_t       u;
    float            denom;
    float            g;
    float            c;

    if ((NULL == p_acc) || (NULL == p_params) || (NULL == p_out))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    imu_mat_identity(&p_out->level_rot);
    p_out->samples = p_acc->n;

    if (p_acc->n < CAL_MIN_SAMPLES)
    {
        return (IMU_ERR_CAL_SAMPLES);
    }

    denom = (float)(p_acc->n - 1U);
    p_out->noise_std_g.x = sqrtf(p_acc->m2.x / denom);
    p_out->noise_std_g.y = sqrtf(p_acc->m2.y / denom);
    p_out->noise_std_g.z = sqrtf(p_acc->m2.z / denom);
    p_out->noise_max_std_g = fmaxf(p_out->noise_std_g.x,
                                   fmaxf(p_out->noise_std_g.y,
                                         p_out->noise_std_g.z));
    p_out->gravity_mean_g = p_acc->mean;

    if ((0 == isfinite(p_out->noise_max_std_g))
        || (p_out->noise_max_std_g > p_params->cal_max_noise_g))
    {
        return (IMU_ERR_CAL_MOTION);
    }

    g = imu_vec_norm(&p_acc->mean);
    p_out->g_ref = g;
    if ((0 == isfinite(g)) || (g < p_params->cal_g_min)
        || (g > p_params->cal_g_max))
    {
        return (IMU_ERR_CAL_RANGE);
    }

    (void)imu_vec_normalise(&p_acc->mean, &u);
    c = imu_clampf(imu_vec_dot(&u, &z_axis), -1.0f, 1.0f);
    p_out->mount_tilt_deg  = acosf(c) * IMU_RAD_TO_DEG;
    p_out->mount_pitch_deg = atan2f(u.x, sqrtf((u.y * u.y) + (u.z * u.z)))
                           * IMU_RAD_TO_DEG;
    p_out->mount_roll_deg  = atan2f(u.y, u.z) * IMU_RAD_TO_DEG;

    if (p_out->mount_tilt_deg > p_params->cal_max_mount_tilt_deg)
    {
        return (IMU_ERR_CAL_RANGE);   /* wrong axis map or IMU upside-down */
    }

    if (!imu_mat_align_to_z(&u, &p_out->level_rot))
    {
        return (IMU_ERR_CAL_RANGE);
    }

    p_out->b_valid = true;
    return (IMU_OK);
}

/* ------------------------------------------------------------------------ */
/*  Magnetometer                                                             */
/* ------------------------------------------------------------------------ */

void imu_cal_mag_begin(imu_mag_cal_acc_t * p_acc)
{
    if (NULL != p_acc)
    {
        (void)memset(p_acc, 0, sizeof(*p_acc));
    }
}

void imu_cal_mag_add(imu_mag_cal_acc_t * p_acc,
                     imu_vec3_t const * p_sensor_gauss)
{
    if ((NULL == p_acc) || !imu_vec_is_finite(p_sensor_gauss))
    {
        return;
    }

    if (0U == p_acc->n)
    {
        p_acc->min = *p_sensor_gauss;
        p_acc->max = *p_sensor_gauss;
    }
    else
    {
        p_acc->min.x = fminf(p_acc->min.x, p_sensor_gauss->x);
        p_acc->min.y = fminf(p_acc->min.y, p_sensor_gauss->y);
        p_acc->min.z = fminf(p_acc->min.z, p_sensor_gauss->z);
        p_acc->max.x = fmaxf(p_acc->max.x, p_sensor_gauss->x);
        p_acc->max.y = fmaxf(p_acc->max.y, p_sensor_gauss->y);
        p_acc->max.z = fmaxf(p_acc->max.z, p_sensor_gauss->z);
    }

    p_acc->sum.x += p_sensor_gauss->x;
    p_acc->sum.y += p_sensor_gauss->y;
    p_acc->sum.z += p_sensor_gauss->z;
    p_acc->n++;
}

imu_err_t imu_cal_mag_finish(imu_mag_cal_acc_t const * p_acc,
                             imu_params_t const * p_params,
                             imu_mag_cal_t * p_out)
{
    uint8_t up_axis;
    uint8_t h[2];
    uint8_t k;
    uint8_t a;
    float   r[2];
    float   r_avg;
    float   v_mean;
    float   v_off;

    if ((NULL == p_acc) || (NULL == p_params) || (NULL == p_out))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    p_out->soft_iron_scale.x = 1.0f;
    p_out->soft_iron_scale.y = 1.0f;
    p_out->soft_iron_scale.z = 1.0f;
    p_out->samples = p_acc->n;

    if (p_acc->n < p_params->mag_cal_min_samples)
    {
        return (IMU_ERR_CAL_SAMPLES);
    }

    /* The two sensor axes that are horizontal on the vehicle. */
    up_axis = (p_params->axis_map[2] < 3U) ? p_params->axis_map[2] : 2U;
    k = 0U;
    for (a = 0U; a < 3U; a++)
    {
        if ((a != up_axis) && (k < 2U))
        {
            h[k] = a;
            k++;
        }
    }

    imu_vec_sub(&p_acc->max, &p_acc->min, &p_out->span_gauss);

    for (k = 0U; k < 2U; k++)
    {
        float lo   = vec_get(&p_acc->min, h[k]);
        float hi   = vec_get(&p_acc->max, h[k]);

        r[k] = 0.5f * (hi - lo);
        vec_set(&p_out->hard_iron_gauss, h[k], 0.5f * (hi + lo));

        if ((hi - lo) < p_params->mag_cal_min_span_gauss)
        {
            return (IMU_ERR_CAL_COVERAGE);
        }
    }

    if (((r[0] / r[1]) > CAL_MAX_ELLIPSE_RATIO)
        || ((r[1] / r[0]) > CAL_MAX_ELLIPSE_RATIO))
    {
        return (IMU_ERR_CAL_COVERAGE);   /* distorted / partial rotation */
    }

    r_avg = 0.5f * (r[0] + r[1]);
    vec_set(&p_out->soft_iron_scale, h[0], r_avg / r[0]);
    vec_set(&p_out->soft_iron_scale, h[1], r_avg / r[1]);

    /* Vertical axis: cannot be observed by a planar spin. */
    v_off = vec_get(&p_params->mag_hard_iron_gauss, up_axis);
    vec_set(&p_out->hard_iron_gauss, up_axis, v_off);
    vec_set(&p_out->soft_iron_scale, up_axis,
            vec_get(&p_params->mag_soft_iron_scale, up_axis));

    v_mean = (vec_get(&p_acc->sum, up_axis) / (float)p_acc->n) - v_off;
    p_out->ref_field_gauss = sqrtf((r_avg * r_avg) + (v_mean * v_mean));

    if ((0 == isfinite(p_out->ref_field_gauss))
        || (p_out->ref_field_gauss <= 0.0f))
    {
        return (IMU_ERR_CAL_COVERAGE);
    }

    p_out->b_valid = true;
    return (IMU_OK);
}

void imu_cal_mag_from_params(imu_params_t const * p_params,
                             imu_mag_cal_t * p_out)
{
    if ((NULL == p_params) || (NULL == p_out))
    {
        return;
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    p_out->b_valid         = p_params->b_mag_cal_valid;
    p_out->hard_iron_gauss = p_params->mag_hard_iron_gauss;
    p_out->soft_iron_scale = p_params->mag_soft_iron_scale;
    p_out->ref_field_gauss = p_params->mag_ref_field_gauss;
}

void imu_cal_mag_apply(imu_mag_cal_t const * p_cal,
                       imu_vec3_t const * p_raw_gauss,
                       imu_vec3_t * p_out)
{
    imu_vec3_t d;

    if ((NULL == p_cal) || (NULL == p_raw_gauss) || (NULL == p_out))
    {
        return;
    }

    imu_vec_sub(p_raw_gauss, &p_cal->hard_iron_gauss, &d);
    p_out->x = d.x * p_cal->soft_iron_scale.x;
    p_out->y = d.y * p_cal->soft_iron_scale.y;
    p_out->z = d.z * p_cal->soft_iron_scale.z;
}
```

## `include/imu/imu_processing.h`

```c
/**
 * @file    imu_processing.h
 * @brief   Buddy 4 signal processing: frame mapping, attitude (pitch / roll /
 *          tilt), tilt-compensated magnetic heading, heading-derived turn
 *          rate, turning detection, tilt detection, impact detection and
 *          motion classification.
 *
 * Hardware-independent and RTOS-independent: works on physical units only.
 *
 * What the LSM303DLHC can and cannot provide (see docs Part 2):
 *   - It measures SPECIFIC FORCE (gravity + linear acceleration) and the
 *     magnetic field. It has no gyroscope.
 *   - Pitch/roll are derived from the gravity direction. During linear
 *     acceleration a: pitch error ~ asin(a/g) (0.05 g -> ~2.9 deg). The
 *     estimate is flagged unreliable when |f| departs from 1 g and can be
 *     corrected with the optional Buddy 2 acceleration hint.
 *   - "Turn rate" is the time derivative of the magnetic heading over a
 *     window (default 200 ms). It is NOT a gyroscope angular velocity: it is
 *     band-limited, delayed by ~window/2 and corrupted by magnetic
 *     disturbances (motors, steel), which are detected and flagged.
 */

#ifndef IMU_PROCESSING_H
#define IMU_PROCESSING_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_calibration.h"
#include "imu/imu_config.h"
#include "imu/imu_math.h"
#include "imu/imu_types.h"

/** Phase of a reported tilt / turn event. */
typedef enum
{
    IMU_PHASE_START = 0,
    IMU_PHASE_END,
    IMU_PHASE_HEADING_CHANGE,
    IMU_PHASE_ESCALATE
} imu_event_phase_t;

/** Impact-detector result (one per detected pulse). */
typedef struct
{
    bool           b_detected;
    imu_severity_t severity;
    float          peak_g;           /**< |high-pass accel| at the peak   */
    float          horiz_g;          /**< horizontal component at peak    */
    float          vert_g;           /**< vertical component at peak      */
    uint32_t       t_ms;             /**< time of the peak                */
    uint32_t       duration_ms;      /**< time above threshold            */
    uint32_t       suppressed;       /**< pulses suppressed by cool-down  */
} imu_impact_t;

/** Per-cycle processing output. */
typedef struct
{
    /* Attitude */
    float      pitch_deg;            /**< relative to calibrated level    */
    float      roll_deg;
    float      tilt_deg;
    bool       b_attitude_reliable;  /**< |f| close to 1 g                */
    imu_vec3_t accel_lp_g;           /**< levelled, low-pass               */
    imu_vec3_t accel_cycle_g;        /**< levelled, cycle average          */
    float      fwd_accel_g;          /**< forward linear accel estimate    */
    bool       b_fwd_from_hint;      /**< fwd_accel_g came from encoders   */
    float      vert_accel_g;         /**< cycle vertical dynamic accel     */
    float      vibration_g;          /**< RMS of high-passed |accel|       */

    /* Heading / turning */
    float                 heading_deg;
    imu_heading_quality_t heading_quality;
    float                 mag_field_gauss;
    float                 turn_rate_dps;   /**< heading-derived, see note */
    bool                  b_turn_rate_valid;
    imu_turn_dir_t        turn_dir;        /**< current debounced state   */

    /* Events produced during this step */
    bool              b_tilt_event;
    imu_event_phase_t tilt_phase;
    imu_severity_t    tilt_severity;

    bool              b_turn_event;
    imu_event_phase_t turn_phase;
    imu_turn_dir_t    turn_event_dir;
    float             turn_event_delta_deg;  /**< heading change of turn  */
    uint32_t          turn_event_duration_ms;
    bool              b_turn_event_unreliable;

    /* Motion classification (filled by imu_proc_classify) */
    imu_motion_class_t motion;
    imu_motion_class_t motion_prev;
    imu_confidence_t   motion_conf;
    bool               b_motion_changed;
} imu_proc_out_t;

/** Processing context (all state is static-size, no allocation). */
typedef struct
{
    imu_params_t const * p_params;
    imu_accel_cal_t      accel_cal;
    imu_mag_cal_t        mag_cal;

    /* Timing */
    bool     b_seeded;
    uint32_t last_t_ms;

    /* Attitude */
    imu_vec3_t accel_lp;
    imu_vec3_t up_last;
    float      pitch_deg;
    float      roll_deg;
    float      tilt_deg;
    bool       b_attitude_reliable;

    /* High-rate impact path */
    bool         b_hf_seeded;
    uint32_t     hf_seed_t_ms;
    imu_vec3_t   hf_lp;
    float        hf_alpha;
    float        vib_var;
    float        vib_alpha;
    bool         b_imp_active;
    bool         b_imp_sustained;
    uint32_t     imp_start_ms;
    uint32_t     imp_last_above_ms;
    imu_impact_t imp_peak;
    uint32_t     imp_cooldown_until;
    bool         b_imp_cooldown;
    imu_severity_t imp_last_sev;
    uint32_t     imp_suppressed;
    uint32_t     imp_rejected_long;
    uint32_t     imp_last_report_ms;
    bool         b_imp_has_report;

    /* Heading */
    bool     b_heading_seeded;
    float    heading_f_deg;
    float    heading_unwrapped;
    uint32_t heading_outliers;
    uint32_t hist_t[IMU_HEADING_HIST_LEN];
    float    hist_h[IMU_HEADING_HIST_LEN];
    uint8_t  hist_head;
    uint8_t  hist_count;
    float    heading_ref_unwrapped;
    float    turn_rate_dps;
    bool     b_turn_rate_valid;
    imu_heading_quality_t heading_quality;
    float    mag_field;

    /* Turn detector */
    imu_turn_dir_t turn_state;
    imu_turn_dir_t turn_cand_dir;
    uint32_t       turn_cand_start;
    bool           b_turn_cand;
    float          turn_cand_unw;
    float          turn_start_unw;
    uint32_t       turn_start_ms;
    uint32_t       turn_exit_start;
    bool           b_turn_exit_timing;

    /* Tilt detector */
    bool     b_tilted;
    bool     b_tilt_critical_sent;
    bool     b_tilt_cand;
    uint32_t tilt_cand_start;

    /* Motion classifier */
    imu_motion_class_t motion;
    imu_motion_class_t motion_cand;
    uint32_t           motion_cand_start;
    imu_confidence_t   motion_conf;
    float              fwd_accel_g;
    bool               b_fwd_from_hint;
    uint32_t           impact_hold_until;
    bool               b_impact_hold;

    /* Optional encoder hint (Buddy 2 via Mission Controller) */
    imu_motion_hint_t hint;
    float             hint_accel_g;
    bool              b_hint_accel_valid;
    uint32_t          hints_rejected;
} imu_proc_t;

/** Initialise; @p p_params must stay valid for the life of @p p_proc. */
imu_err_t imu_proc_init(imu_proc_t * p_proc, imu_params_t const * p_params);

/** Install calibration results (either may be NULL to keep current). */
void imu_proc_set_calibration(imu_proc_t * p_proc,
                              imu_accel_cal_t const * p_accel,
                              imu_mag_cal_t const * p_mag);

/** Clear all filters and detectors (after a fault / time discontinuity). */
void imu_proc_reset_dynamic(imu_proc_t * p_proc);

/** Sensor-frame accel (g) -> vehicle axes, stored bias removed, NOT levelled
 *  (used to feed the stationary calibration). */
void imu_proc_map_accel(imu_proc_t const * p_proc,
                        imu_vec3_t const * p_sensor_g,
                        imu_vec3_t * p_vehicle_g);

/** Sensor-frame accel (g) -> levelled vehicle frame. */
void imu_proc_level_accel(imu_proc_t const * p_proc,
                          imu_vec3_t const * p_sensor_g,
                          imu_vec3_t * p_level_g);

/**
 * @brief  High-rate impact path, called for EVERY accelerometer sample
 *         (all FIFO entries), oldest first.
 * @param  b_hump_active  raises the threshold while crossing a hump.
 * @param  p_out          b_detected = true when a pulse was classified.
 */
void imu_proc_hf_accel(imu_proc_t * p_proc, imu_vec3_t const * p_sensor_g,
                       uint32_t t_ms, bool b_hump_active,
                       imu_impact_t * p_out);

/**
 * @brief  Main periodic step (attitude, heading, turn and tilt detection).
 * @param  p_accel_sensor_g  cycle-average accel, sensor frame (not NULL)
 * @param  p_mag_sensor_gauss  magnetometer sample, sensor frame, or NULL if
 *                             unavailable/overflowed this cycle.
 * @param  b_hump_active  tilt events from pitch are suppressed in a hump.
 */
void imu_proc_step(imu_proc_t * p_proc, imu_vec3_t const * p_accel_sensor_g,
                   imu_vec3_t const * p_mag_sensor_gauss, uint32_t now_ms,
                   bool b_hump_active, imu_proc_out_t * p_out);

/**
 * @brief  Motion classification; call after imu_proc_step and the hump
 *         detector in the same cycle.
 */
void imu_proc_classify(imu_proc_t * p_proc, uint32_t now_ms,
                       imu_hump_phase_t hump_phase, bool b_new_impact,
                       imu_proc_out_t * p_out);

/**
 * @brief  Supply / refresh the optional Buddy 2 motion hint.
 * @return IMU_OK, or IMU_ERR_PARAM if the hint is implausible (rejected).
 */
imu_err_t imu_proc_set_hint(imu_proc_t * p_proc,
                            imu_motion_hint_t const * p_hint);

/** @return true if a hint younger than hint_max_age_ms is available. */
bool imu_proc_hint_fresh(imu_proc_t const * p_proc, uint32_t now_ms);

/** Name helpers for logs / telemetry (never NULL). */
char const * imu_motion_name(imu_motion_class_t motion);
char const * imu_heading_quality_name(imu_heading_quality_t quality);

#endif /* IMU_PROCESSING_H */
```

## `src/imu/imu_processing.c`

```c
/**
 * @file    imu_processing.c
 * @brief   Buddy 4 signal processing (see imu_processing.h for the model).
 */

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "imu/imu_processing.h"

#define PROC_STANDARD_GRAVITY_MM_S2  (9806.65f)
#define PROC_TIME_GAP_RESET_MS       (1000U)   /* discontinuity -> re-seed */
#define PROC_RELEASE_FRACTION        (0.7f)    /* impact hysteresis        */
#define PROC_UNWRAP_REBASE_DEG       (3600.0f)
#define PROC_MIN_FIELD_GAUSS         (0.05f)
#define PROC_MAX_FIELD_GAUSS         (4.0f)
#define PROC_MIN_EAST_NORM           (0.02f)
#define PROC_HINT_DIFF_MIN_MS        (20U)
#define PROC_HINT_DIFF_MAX_MS        (500U)
#define PROC_HINT_DIFF_ALPHA         (0.5f)
#define PROC_NOISE_VIB_FACTOR        (3.0f)

/* ------------------------------------------------------------------------ */
/*  Init / calibration                                                      */
/* ------------------------------------------------------------------------ */

static float proc_odr_period_s(imu_params_t const * p_params)
{
    uint32_t hz = imu_params_accel_odr_hz(p_params->accel_odr_code);

    return ((hz > 0U) ? (1.0f / (float)hz) : 0.01f);
}

imu_err_t imu_proc_init(imu_proc_t * p_proc, imu_params_t const * p_params)
{
    if ((NULL == p_proc) || (NULL == p_params))
    {
        return (IMU_ERR_PARAM);
    }

    if (IMU_OK != imu_params_validate(p_params))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_proc, 0, sizeof(*p_proc));
    p_proc->p_params = p_params;

    imu_mat_identity(&p_proc->accel_cal.level_rot);
    p_proc->accel_cal.g_ref = 1.0f;
    p_proc->accel_cal.b_valid = false;
    imu_cal_mag_from_params(p_params, &p_proc->mag_cal);

    imu_proc_reset_dynamic(p_proc);

    return (IMU_OK);
}

void imu_proc_set_calibration(imu_proc_t * p_proc,
                              imu_accel_cal_t const * p_accel,
                              imu_mag_cal_t const * p_mag)
{
    if (NULL == p_proc)
    {
        return;
    }

    if (NULL != p_accel)
    {
        p_proc->accel_cal = *p_accel;
    }
    if (NULL != p_mag)
    {
        p_proc->mag_cal = *p_mag;
    }

    imu_proc_reset_dynamic(p_proc);
}

void imu_proc_reset_dynamic(imu_proc_t * p_proc)
{
    imu_params_t const * p_params;
    float                odr_dt;

    if ((NULL == p_proc) || (NULL == p_proc->p_params))
    {
        return;
    }

    p_params = p_proc->p_params;
    odr_dt   = proc_odr_period_s(p_params);

    p_proc->b_seeded            = false;
    p_proc->b_hf_seeded         = false;
    p_proc->hf_alpha            = imu_lpf_alpha(p_params->impact_lpf_hz,
                                                odr_dt);
    p_proc->vib_alpha           = imu_lpf_alpha(p_params->vib_lpf_hz, odr_dt);
    p_proc->vib_var             = 0.0f;
    p_proc->b_imp_active        = false;
    p_proc->b_imp_sustained     = false;
    p_proc->b_imp_cooldown      = false;
    p_proc->imp_suppressed      = 0U;
    p_proc->up_last.x           = 0.0f;
    p_proc->up_last.y           = 0.0f;
    p_proc->up_last.z           = 1.0f;
    p_proc->pitch_deg           = 0.0f;
    p_proc->roll_deg            = 0.0f;
    p_proc->tilt_deg            = 0.0f;
    p_proc->b_attitude_reliable = false;

    p_proc->b_heading_seeded    = false;
    p_proc->heading_outliers    = 0U;
    p_proc->hist_head           = 0U;
    p_proc->hist_count          = 0U;
    p_proc->b_turn_rate_valid   = false;
    p_proc->turn_rate_dps       = 0.0f;
    p_proc->heading_quality     = IMU_HEADING_INVALID;

    p_proc->turn_state          = IMU_TURN_NONE;
    p_proc->b_turn_cand         = false;
    p_proc->b_turn_exit_timing  = false;

    p_proc->b_tilted            = false;
    p_proc->b_tilt_critical_sent = false;
    p_proc->b_tilt_cand         = false;

    p_proc->motion              = IMU_MOTION_UNKNOWN;
    p_proc->motion_cand         = IMU_MOTION_UNKNOWN;
    p_proc->motion_conf         = IMU_CONF_NONE;
    p_proc->b_impact_hold       = false;
}

void imu_proc_map_accel(imu_proc_t const * p_proc,
                        imu_vec3_t const * p_sensor_g,
                        imu_vec3_t * p_vehicle_g)
{
    imu_vec3_t corrected;

    imu_vec_sub(p_sensor_g, &p_proc->p_params->accel_bias_g, &corrected);
    imu_frame_map(p_proc->p_params->axis_map, p_proc->p_params->axis_sign,
                  &corrected, p_vehicle_g);
}

void imu_proc_level_accel(imu_proc_t const * p_proc,
                          imu_vec3_t const * p_sensor_g,
                          imu_vec3_t * p_level_g)
{
    imu_vec3_t v;

    imu_proc_map_accel(p_proc, p_sensor_g, &v);
    imu_mat_apply(&p_proc->accel_cal.level_rot, &v, p_level_g);
}

/* ------------------------------------------------------------------------ */
/*  High-rate impact detection                                              */
/* ------------------------------------------------------------------------ */

static imu_severity_t proc_impact_severity(imu_params_t const * p_params,
                                           imu_impact_t const * p_peak)
{
    imu_severity_t sev   = IMU_SEV_MINOR;
    float          ratio = (p_peak->peak_g > 0.0f)
                           ? (p_peak->horiz_g / p_peak->peak_g) : 0.0f;

    /* CRITICAL: very large in any direction, or large and mostly
     * horizontal (collision-like). Otherwise MINOR: bump, kerb, hump
     * edge, hand knock. */
    if ((p_peak->peak_g >= p_params->impact_critical_any_g)
        || ((p_peak->peak_g >= p_params->impact_critical_g)
            && (ratio >= p_params->impact_horiz_ratio)))
    {
        sev = IMU_SEV_CRITICAL;
    }

    return (sev);
}

void imu_proc_hf_accel(imu_proc_t * p_proc, imu_vec3_t const * p_sensor_g,
                       uint32_t t_ms, bool b_hump_active,
                       imu_impact_t * p_out)
{
    imu_params_t const * p_params;
    imu_vec3_t           a;
    imu_vec3_t           hp;
    float                mag;
    float                thr;
    float                release;

    if ((NULL == p_proc) || (NULL == p_sensor_g) || (NULL == p_out))
    {
        return;
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    if (!imu_vec_is_finite(p_sensor_g))
    {
        return;
    }

    p_params = p_proc->p_params;
    imu_proc_level_accel(p_proc, p_sensor_g, &a);

    if (!p_proc->b_hf_seeded)
    {
        p_proc->hf_lp        = a;
        p_proc->b_hf_seeded  = true;
        p_proc->hf_seed_t_ms = t_ms;
        return;
    }

    imu_vec_sub(&a, &p_proc->hf_lp, &hp);
    p_proc->hf_lp.x = imu_lpf_step(p_proc->hf_lp.x, a.x, p_proc->hf_alpha);
    p_proc->hf_lp.y = imu_lpf_step(p_proc->hf_lp.y, a.y, p_proc->hf_alpha);
    p_proc->hf_lp.z = imu_lpf_step(p_proc->hf_lp.z, a.z, p_proc->hf_alpha);

    mag = imu_vec_norm(&hp);
    p_proc->vib_var = imu_lpf_step(p_proc->vib_var, mag * mag,
                                   p_proc->vib_alpha);

    if ((t_ms - p_proc->hf_seed_t_ms) < p_params->impact_warmup_ms)
    {
        return;   /* filter settling: no detection */
    }

    thr = p_params->impact_minor_g;
    if (b_hump_active)
    {
        thr *= p_params->hump_impact_thr_scale;
    }
    release = thr * PROC_RELEASE_FRACTION;

    if (p_proc->b_imp_sustained)
    {
        /* Long excursion rejected: wait until it has clearly ended. */
        if (mag < release)
        {
            p_proc->b_imp_sustained = false;
        }
        return;
    }

    if (!p_proc->b_imp_active)
    {
        if (mag > thr)
        {
            p_proc->b_imp_active       = true;
            p_proc->imp_start_ms       = t_ms;
            p_proc->imp_last_above_ms  = t_ms;
            p_proc->imp_peak.peak_g    = mag;
            p_proc->imp_peak.horiz_g   = sqrtf((hp.x * hp.x) + (hp.y * hp.y));
            p_proc->imp_peak.vert_g    = fabsf(hp.z);
            p_proc->imp_peak.t_ms      = t_ms;
        }
        return;
    }

    if (mag > p_proc->imp_peak.peak_g)
    {
        p_proc->imp_peak.peak_g  = mag;
        p_proc->imp_peak.horiz_g = sqrtf((hp.x * hp.x) + (hp.y * hp.y));
        p_proc->imp_peak.vert_g  = fabsf(hp.z);
        p_proc->imp_peak.t_ms    = t_ms;
    }

    if (mag > release)
    {
        p_proc->imp_last_above_ms = t_ms;
    }

    if ((t_ms - p_proc->imp_start_ms) > p_params->impact_max_duration_ms)
    {
        /* Too long for an impact: sustained manoeuvre / vibration. */
        p_proc->b_imp_active    = false;
        p_proc->b_imp_sustained = true;
        p_proc->imp_rejected_long++;
        return;
    }

    if (mag <= release)
    {
        imu_severity_t sev;
        bool           b_report = true;

        p_proc->b_imp_active = false;
        sev = proc_impact_severity(p_params, &p_proc->imp_peak);

        if (p_proc->b_imp_cooldown
            && ((t_ms - p_proc->imp_last_report_ms)
                < p_params->impact_cooldown_ms))
        {
            /* Within cool-down: only an escalation is reported. */
            if (sev <= p_proc->imp_last_sev)
            {
                b_report = false;
                p_proc->imp_suppressed++;
            }
        }

        if (b_report)
        {
            *p_out = p_proc->imp_peak;
            p_out->b_detected  = true;
            p_out->severity    = sev;
            p_out->duration_ms = p_proc->imp_last_above_ms
                               - p_proc->imp_start_ms;
            p_out->suppressed  = p_proc->imp_suppressed;

            p_proc->imp_suppressed     = 0U;
            p_proc->imp_last_report_ms = t_ms;
            p_proc->imp_last_sev       = sev;
            p_proc->b_imp_cooldown     = true;
        }
    }
}

/* ------------------------------------------------------------------------ */
/*  Heading                                                                 */
/* ------------------------------------------------------------------------ */

static void proc_hist_clear(imu_proc_t * p_proc)
{
    p_proc->hist_head         = 0U;
    p_proc->hist_count        = 0U;
    p_proc->b_turn_rate_valid = false;
}

static void proc_hist_push(imu_proc_t * p_proc, uint32_t t_ms, float h)
{
    p_proc->hist_t[p_proc->hist_head] = t_ms;
    p_proc->hist_h[p_proc->hist_head] = h;
    p_proc->hist_head = (uint8_t)((p_proc->hist_head + 1U)
                                  % IMU_HEADING_HIST_LEN);
    if (p_proc->hist_count < IMU_HEADING_HIST_LEN)
    {
        p_proc->hist_count++;
    }
}

/** Rate = d(unwrapped heading)/dt over >= turn_rate_window_ms. */
static void proc_turn_rate(imu_proc_t * p_proc, uint32_t now_ms)
{
    uint8_t i;
    uint8_t idx;

    p_proc->b_turn_rate_valid = false;

    for (i = 1U; i < p_proc->hist_count; i++)
    {
        uint32_t age;

        idx = (uint8_t)((p_proc->hist_head + IMU_HEADING_HIST_LEN - 1U - i)
                        % IMU_HEADING_HIST_LEN);
        age = now_ms - p_proc->hist_t[idx];

        if (age >= p_proc->p_params->turn_rate_window_ms)
        {
            p_proc->turn_rate_dps = (p_proc->heading_unwrapped
                                     - p_proc->hist_h[idx])
                                  / ((float)age * 0.001f);
            p_proc->b_turn_rate_valid = true;
            break;
        }
    }
}

static void proc_unwrap_rebase(imu_proc_t * p_proc)
{
    float   shift;
    uint8_t i;

    if (fabsf(p_proc->heading_unwrapped) < PROC_UNWRAP_REBASE_DEG)
    {
        return;
    }

    shift = 360.0f * floorf(p_proc->heading_unwrapped / 360.0f);
    p_proc->heading_unwrapped     -= shift;
    p_proc->heading_ref_unwrapped -= shift;
    p_proc->turn_start_unw        -= shift;
    p_proc->turn_cand_unw         -= shift;
    for (i = 0U; i < IMU_HEADING_HIST_LEN; i++)
    {
        p_proc->hist_h[i] -= shift;
    }
}

/**
 * Tilt-compensated heading by vector algebra (convention-independent):
 *   up = unit gravity reaction (vehicle frame)
 *   E  = B x up (magnetic east),  N = up x E (magnetic north)
 *   heading of +X = atan2(E.x, N.x), clockwise from north.
 */
static bool proc_heading_raw(imu_vec3_t const * p_b, imu_vec3_t const * p_up,
                             float * p_heading)
{
    imu_vec3_t e;
    imu_vec3_t e_n;
    imu_vec3_t n;

    imu_vec_cross(p_b, p_up, &e);
    if ((imu_vec_norm(&e) < PROC_MIN_EAST_NORM)
        || !imu_vec_normalise(&e, &e_n))
    {
        return (false);   /* field almost vertical: heading undefined */
    }

    imu_vec_cross(p_up, &e_n, &n);
    *p_heading = imu_wrap_360(atan2f(e_n.x, n.x) * IMU_RAD_TO_DEG);

    return (0 != isfinite(*p_heading));
}

static void proc_heading_step(imu_proc_t * p_proc,
                              imu_vec3_t const * p_mag_sensor,
                              uint32_t now_ms, float dt_s)
{
    imu_params_t const * p_params = p_proc->p_params;
    imu_vec3_t           m_cal;
    imu_vec3_t           m_veh;
    imu_vec3_t           m_lev;
    float                raw;
    float                delta;
    float                alpha;

    if ((NULL == p_mag_sensor) || !imu_vec_is_finite(p_mag_sensor))
    {
        p_proc->heading_quality = IMU_HEADING_INVALID;
        proc_hist_clear(p_proc);
        return;
    }

    if (p_proc->mag_cal.b_valid)
    {
        imu_cal_mag_apply(&p_proc->mag_cal, p_mag_sensor, &m_cal);
    }
    else
    {
        m_cal = *p_mag_sensor;
    }

    imu_frame_map(p_params->axis_map, p_params->axis_sign, &m_cal, &m_veh);
    imu_mat_apply(&p_proc->accel_cal.level_rot, &m_veh, &m_lev);
    p_proc->mag_field = imu_vec_norm(&m_lev);

    if ((p_proc->mag_field < PROC_MIN_FIELD_GAUSS)
        || (p_proc->mag_field > PROC_MAX_FIELD_GAUSS)
        || !proc_heading_raw(&m_lev, &p_proc->up_last, &raw))
    {
        p_proc->heading_quality = IMU_HEADING_INVALID;
        proc_hist_clear(p_proc);
        return;
    }

    if (!p_proc->mag_cal.b_valid)
    {
        p_proc->heading_quality = IMU_HEADING_UNCALIBRATED;
    }
    else if ((p_proc->mag_cal.ref_field_gauss > 0.0f)
             && (fabsf(p_proc->mag_field - p_proc->mag_cal.ref_field_gauss)
                 > (p_params->mag_field_tol_frac
                    * p_proc->mag_cal.ref_field_gauss)))
    {
        p_proc->heading_quality = IMU_HEADING_DISTURBED;
    }
    else
    {
        p_proc->heading_quality = IMU_HEADING_GOOD;
    }

    if (IMU_HEADING_DISTURBED == p_proc->heading_quality)
    {
        /* Do not integrate a corrupted heading into the turn-rate history. */
        proc_hist_clear(p_proc);
        return;
    }

    if (!p_proc->b_heading_seeded)
    {
        p_proc->heading_f_deg         = raw;
        p_proc->heading_unwrapped     = raw;
        p_proc->heading_ref_unwrapped = raw;
        p_proc->b_heading_seeded      = true;
        p_proc->heading_outliers      = 0U;
        proc_hist_clear(p_proc);
        proc_hist_push(p_proc, now_ms, p_proc->heading_unwrapped);
        return;
    }

    delta = imu_wrap_180(raw - p_proc->heading_f_deg);

    if (fabsf(delta) > p_params->heading_max_jump_deg)
    {
        p_proc->heading_outliers++;
        if (p_proc->heading_outliers <= p_params->heading_outlier_limit)
        {
            return;   /* reject a single implausible jump */
        }
        /* Persistent: accept the new heading, restart the rate history. */
        p_proc->heading_f_deg = raw;
        p_proc->heading_unwrapped += delta;
        p_proc->heading_outliers = 0U;
        proc_hist_clear(p_proc);
        proc_hist_push(p_proc, now_ms, p_proc->heading_unwrapped);
        return;
    }

    p_proc->heading_outliers = 0U;
    alpha = imu_lpf_alpha(p_params->heading_lpf_hz, dt_s);
    p_proc->heading_f_deg     = imu_wrap_360(p_proc->heading_f_deg
                                             + (alpha * delta));
    p_proc->heading_unwrapped += alpha * delta;
    proc_unwrap_rebase(p_proc);
    proc_hist_push(p_proc, now_ms, p_proc->heading_unwrapped);
    proc_turn_rate(p_proc, now_ms);
}

static bool proc_heading_usable(imu_proc_t const * p_proc)
{
    return ((IMU_HEADING_GOOD == p_proc->heading_quality)
            || ((IMU_HEADING_UNCALIBRATED == p_proc->heading_quality)
                && p_proc->p_params->b_turn_allow_uncal_mag));
}

/* ------------------------------------------------------------------------ */
/*  Turning detector                                                        */
/* ------------------------------------------------------------------------ */

static void proc_turn_end(imu_proc_t * p_proc, uint32_t now_ms,
                          bool b_unreliable, imu_proc_out_t * p_out)
{
    p_out->b_turn_event            = true;
    p_out->turn_phase              = IMU_PHASE_END;
    p_out->turn_event_dir          = p_proc->turn_state;
    p_out->turn_event_delta_deg    = p_proc->heading_unwrapped
                                   - p_proc->turn_start_unw;
    p_out->turn_event_duration_ms  = now_ms - p_proc->turn_start_ms;
    p_out->b_turn_event_unreliable = b_unreliable;

    p_proc->turn_state            = IMU_TURN_NONE;
    p_proc->b_turn_exit_timing    = false;
    p_proc->b_turn_cand           = false;
    p_proc->heading_ref_unwrapped = p_proc->heading_unwrapped;
}

static void proc_turn_step(imu_proc_t * p_proc, uint32_t now_ms,
                           imu_proc_out_t * p_out)
{
    imu_params_t const * p_params = p_proc->p_params;
    bool                 b_ok     = proc_heading_usable(p_proc)
                                    && p_proc->b_turn_rate_valid;
    float                rate     = p_proc->turn_rate_dps;
    imu_turn_dir_t       dir      = (rate < 0.0f) ? IMU_TURN_LEFT
                                                  : IMU_TURN_RIGHT;

    if (IMU_TURN_NONE == p_proc->turn_state)
    {
        if (b_ok && (fabsf(rate) > p_params->turn_enter_dps))
        {
            if (!p_proc->b_turn_cand || (p_proc->turn_cand_dir != dir))
            {
                p_proc->b_turn_cand     = true;
                p_proc->turn_cand_dir   = dir;
                p_proc->turn_cand_start = now_ms;
                /* The turn began roughly one window earlier. */
                p_proc->turn_cand_unw   = p_proc->heading_unwrapped
                    - (rate * (float)p_params->turn_rate_window_ms * 0.001f);
            }
            else if ((now_ms - p_proc->turn_cand_start)
                     >= p_params->turn_min_duration_ms)
            {
                p_proc->turn_state         = dir;
                p_proc->turn_start_unw     = p_proc->turn_cand_unw;
                p_proc->turn_start_ms      = p_proc->turn_cand_start;
                p_proc->b_turn_cand        = false;
                p_proc->b_turn_exit_timing = false;

                p_out->b_turn_event            = true;
                p_out->turn_phase              = IMU_PHASE_START;
                p_out->turn_event_dir          = dir;
                p_out->turn_event_delta_deg    = p_proc->heading_unwrapped
                                               - p_proc->turn_start_unw;
                p_out->turn_event_duration_ms  = now_ms
                                               - p_proc->turn_start_ms;
                p_out->b_turn_event_unreliable = false;
            }
            else
            {
                /* keep timing the candidate */
            }
        }
        else
        {
            p_proc->b_turn_cand = false;

            /* Slow heading drift / gentle curve monitoring. */
            if (b_ok && p_proc->b_heading_seeded
                && (fabsf(p_proc->heading_unwrapped
                          - p_proc->heading_ref_unwrapped)
                    >= p_params->heading_change_deg))
            {
                p_out->b_turn_event            = true;
                p_out->turn_phase              = IMU_PHASE_HEADING_CHANGE;
                p_out->turn_event_delta_deg    = p_proc->heading_unwrapped
                                     - p_proc->heading_ref_unwrapped;
                p_out->turn_event_dir          =
                    (p_out->turn_event_delta_deg < 0.0f) ? IMU_TURN_LEFT
                                                         : IMU_TURN_RIGHT;
                p_out->turn_event_duration_ms  = 0U;
                p_out->b_turn_event_unreliable = false;
                p_proc->heading_ref_unwrapped  = p_proc->heading_unwrapped;
            }
        }
        return;
    }

    /* Currently turning: look for the end (low rate, reversal or loss). */
    if (!b_ok || (fabsf(rate) < p_params->turn_exit_dps)
        || (dir != p_proc->turn_state))
    {
        if (!p_proc->b_turn_exit_timing)
        {
            p_proc->b_turn_exit_timing = true;
            p_proc->turn_exit_start    = now_ms;
        }
        else if ((now_ms - p_proc->turn_exit_start)
                 >= p_params->turn_exit_duration_ms)
        {
            proc_turn_end(p_proc, now_ms, !b_ok, p_out);
        }
        else
        {
            /* keep timing the exit */
        }
    }
    else
    {
        p_proc->b_turn_exit_timing = false;
    }
}

/* ------------------------------------------------------------------------ */
/*  Tilt detector                                                           */
/* ------------------------------------------------------------------------ */

static void proc_tilt_step(imu_proc_t * p_proc, uint32_t now_ms,
                           bool b_hump_active, imu_proc_out_t * p_out)
{
    imu_params_t const * p_params = p_proc->p_params;
    float                tilt     = p_proc->tilt_deg;
    bool                 b_crit;

    if (!p_proc->b_attitude_reliable)
    {
        p_proc->b_tilt_cand = false;   /* gravity direction not trustworthy */
        return;
    }

    /* While crossing a hump the pitch excursion is expected: use roll only
     * unless the total tilt is already critical. */
    if (b_hump_active && (tilt < p_params->tilt_critical_deg))
    {
        tilt = fabsf(p_proc->roll_deg);
    }
    b_crit = (tilt >= p_params->tilt_critical_deg);

    if (!p_proc->b_tilted)
    {
        if (tilt > p_params->tilt_threshold_deg)
        {
            if (!p_proc->b_tilt_cand)
            {
                p_proc->b_tilt_cand     = true;
                p_proc->tilt_cand_start = now_ms;
            }
            if (((now_ms - p_proc->tilt_cand_start)
                 >= p_params->tilt_min_duration_ms) || b_crit)
            {
                p_proc->b_tilted             = true;
                p_proc->b_tilt_cand          = false;
                p_proc->b_tilt_critical_sent = b_crit;
                p_out->b_tilt_event  = true;
                p_out->tilt_phase    = IMU_PHASE_START;
                p_out->tilt_severity = b_crit ? IMU_SEV_CRITICAL
                                              : IMU_SEV_MINOR;
            }
        }
        else
        {
            p_proc->b_tilt_cand = false;
        }
        return;
    }

    if (b_crit && !p_proc->b_tilt_critical_sent)
    {
        p_proc->b_tilt_critical_sent = true;
        p_out->b_tilt_event  = true;
        p_out->tilt_phase    = IMU_PHASE_ESCALATE;
        p_out->tilt_severity = IMU_SEV_CRITICAL;
        return;
    }

    if (tilt < (p_params->tilt_threshold_deg - p_params->tilt_hysteresis_deg))
    {
        if (!p_proc->b_tilt_cand)
        {
            p_proc->b_tilt_cand     = true;
            p_proc->tilt_cand_start = now_ms;
        }
        else if ((now_ms - p_proc->tilt_cand_start)
                 >= p_params->tilt_min_duration_ms)
        {
            p_proc->b_tilted             = false;
            p_proc->b_tilt_cand          = false;
            p_proc->b_tilt_critical_sent = false;
            p_out->b_tilt_event  = true;
            p_out->tilt_phase    = IMU_PHASE_END;
            p_out->tilt_severity = IMU_SEV_INFO;
        }
        else
        {
            /* keep timing */
        }
    }
    else
    {
        p_proc->b_tilt_cand = false;
    }
}

/* ------------------------------------------------------------------------ */
/*  Main step                                                               */
/* ------------------------------------------------------------------------ */

void imu_proc_step(imu_proc_t * p_proc, imu_vec3_t const * p_accel_sensor_g,
                   imu_vec3_t const * p_mag_sensor_gauss, uint32_t now_ms,
                   bool b_hump_active, imu_proc_out_t * p_out)
{
    imu_params_t const * p_params;
    imu_vec3_t           a;
    imu_vec3_t           f;
    float                dt_s;
    float                norm;
    float                alpha;

    if ((NULL == p_proc) || (NULL == p_accel_sensor_g) || (NULL == p_out))
    {
        return;
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    p_out->motion      = p_proc->motion;
    p_out->motion_prev = p_proc->motion;
    p_out->motion_conf = p_proc->motion_conf;

    if (!imu_vec_is_finite(p_accel_sensor_g))
    {
        return;
    }

    p_params = p_proc->p_params;

    /* Time base: tolerate jitter, re-seed after a discontinuity (pause,
     * fault, or time moving backwards - unsigned difference is huge). */
    if (p_proc->b_seeded
        && ((now_ms - p_proc->last_t_ms) > PROC_TIME_GAP_RESET_MS))
    {
        imu_proc_reset_dynamic(p_proc);
    }

    if (p_proc->b_seeded && (now_ms != p_proc->last_t_ms))
    {
        dt_s = (float)(now_ms - p_proc->last_t_ms) * 0.001f;
    }
    else
    {
        dt_s = (float)p_params->sample_period_ms * 0.001f;
    }

    imu_proc_level_accel(p_proc, p_accel_sensor_g, &a);
    p_out->accel_cycle_g = a;

    if (!p_proc->b_seeded)
    {
        p_proc->accel_lp = a;
        p_proc->b_seeded = true;
    }
    else
    {
        alpha = imu_lpf_alpha(p_params->attitude_lpf_hz, dt_s);
        p_proc->accel_lp.x = imu_lpf_step(p_proc->accel_lp.x, a.x, alpha);
        p_proc->accel_lp.y = imu_lpf_step(p_proc->accel_lp.y, a.y, alpha);
        p_proc->accel_lp.z = imu_lpf_step(p_proc->accel_lp.z, a.z, alpha);
    }
    p_proc->last_t_ms = now_ms;

    /* Forward-acceleration compensation from the encoder hint (if any). */
    f = p_proc->accel_lp;
    p_proc->b_fwd_from_hint = false;
    if (imu_proc_hint_fresh(p_proc, now_ms) && p_proc->b_hint_accel_valid)
    {
        f.x -= p_proc->hint_accel_g;
        p_proc->fwd_accel_g     = p_proc->hint_accel_g;
        p_proc->b_fwd_from_hint = true;
    }
    else
    {
        /* Without encoders: valid only on level ground (not in a hump). */
        p_proc->fwd_accel_g = p_proc->accel_lp.x;
    }

    norm = imu_vec_norm(&f);
    p_proc->b_attitude_reliable =
        (fabsf(norm - p_proc->accel_cal.g_ref)
         <= p_params->attitude_valid_tol_g);

    p_proc->pitch_deg = atan2f(f.x, sqrtf((f.y * f.y) + (f.z * f.z)))
                      * IMU_RAD_TO_DEG;
    p_proc->roll_deg  = atan2f(f.y, f.z) * IMU_RAD_TO_DEG;
    p_proc->tilt_deg  = atan2f(sqrtf((f.x * f.x) + (f.y * f.y)), f.z)
                      * IMU_RAD_TO_DEG;

    if (p_proc->b_attitude_reliable)
    {
        (void)imu_vec_normalise(&f, &p_proc->up_last);
    }

    proc_heading_step(p_proc, p_mag_sensor_gauss, now_ms, dt_s);
    proc_turn_step(p_proc, now_ms, p_out);
    proc_tilt_step(p_proc, now_ms, b_hump_active, p_out);

    p_out->pitch_deg           = p_proc->pitch_deg;
    p_out->roll_deg            = p_proc->roll_deg;
    p_out->tilt_deg            = p_proc->tilt_deg;
    p_out->b_attitude_reliable = p_proc->b_attitude_reliable;
    p_out->accel_lp_g          = p_proc->accel_lp;
    p_out->fwd_accel_g         = p_proc->fwd_accel_g;
    p_out->b_fwd_from_hint     = p_proc->b_fwd_from_hint;
    p_out->vert_accel_g        = a.z - p_proc->accel_cal.g_ref;
    p_out->vibration_g         = sqrtf(fmaxf(p_proc->vib_var, 0.0f));
    p_out->heading_deg         = p_proc->heading_f_deg;
    p_out->heading_quality     = p_proc->heading_quality;
    p_out->mag_field_gauss     = p_proc->mag_field;
    p_out->turn_rate_dps       = p_proc->b_turn_rate_valid
                                 ? p_proc->turn_rate_dps : 0.0f;
    p_out->b_turn_rate_valid   = p_proc->b_turn_rate_valid
                                 && proc_heading_usable(p_proc);
    p_out->turn_dir            = p_proc->turn_state;
}

/* ------------------------------------------------------------------------ */
/*  Motion classification                                                   */
/* ------------------------------------------------------------------------ */

void imu_proc_classify(imu_proc_t * p_proc, uint32_t now_ms,
                       imu_hump_phase_t hump_phase, bool b_new_impact,
                       imu_proc_out_t * p_out)
{
    imu_params_t const * p_params;
    imu_motion_class_t   cand;
    imu_confidence_t     conf;
    bool                 b_immediate = true;
    bool                 b_hint;
    float                acc_thr;
    float                dec_thr;
    float                vib_thr;

    if ((NULL == p_proc) || (NULL == p_out))
    {
        return;
    }

    p_params = p_proc->p_params;
    b_hint   = imu_proc_hint_fresh(p_proc, now_ms);
    p_out->motion_prev = p_proc->motion;

    if (b_new_impact)
    {
        p_proc->b_impact_hold     = true;
        p_proc->impact_hold_until = now_ms;   /* start of hold window */
    }
    if (p_proc->b_impact_hold
        && ((now_ms - p_proc->impact_hold_until) >= p_params->impact_hold_ms))
    {
        p_proc->b_impact_hold = false;
    }

    if (p_proc->b_impact_hold)
    {
        cand = IMU_MOTION_SUDDEN_IMPACT;
        conf = IMU_CONF_MEDIUM;
    }
    else if ((IMU_HUMP_ASCENT == hump_phase) || (IMU_HUMP_PEAK == hump_phase))
    {
        cand = IMU_MOTION_CLIMBING_HUMP;
        conf = IMU_CONF_MEDIUM;
    }
    else if (IMU_HUMP_DESCENT == hump_phase)
    {
        cand = IMU_MOTION_DESCENDING_HUMP;
        conf = IMU_CONF_MEDIUM;
    }
    else if (IMU_TURN_LEFT == p_proc->turn_state)
    {
        cand = IMU_MOTION_TURNING_LEFT;
        conf = (IMU_HEADING_GOOD == p_proc->heading_quality)
               ? IMU_CONF_MEDIUM : IMU_CONF_LOW;
    }
    else if (IMU_TURN_RIGHT == p_proc->turn_state)
    {
        cand = IMU_MOTION_TURNING_RIGHT;
        conf = (IMU_HEADING_GOOD == p_proc->heading_quality)
               ? IMU_CONF_MEDIUM : IMU_CONF_LOW;
    }
    else
    {
        b_immediate = false;
        acc_thr = (IMU_MOTION_ACCELERATING == p_proc->motion)
                  ? p_params->accel_exit_g : p_params->accel_enter_g;
        dec_thr = (IMU_MOTION_DECELERATING == p_proc->motion)
                  ? p_params->accel_exit_g : p_params->accel_enter_g;
        vib_thr = fmaxf(p_params->stationary_vib_g,
                        PROC_NOISE_VIB_FACTOR
                        * p_proc->accel_cal.noise_max_std_g);

        if (p_proc->fwd_accel_g > acc_thr)
        {
            cand = IMU_MOTION_ACCELERATING;
            conf = p_proc->b_fwd_from_hint ? IMU_CONF_HIGH : IMU_CONF_MEDIUM;
        }
        else if (p_proc->fwd_accel_g < -dec_thr)
        {
            cand = IMU_MOTION_DECELERATING;
            conf = p_proc->b_fwd_from_hint ? IMU_CONF_HIGH : IMU_CONF_MEDIUM;
        }
        else if (b_hint)
        {
            int32_t speed = p_proc->hint.speed_mm_s;

            cand = ((speed <= p_params->stationary_speed_mm_s)
                    && (speed >= -p_params->stationary_speed_mm_s))
                   ? IMU_MOTION_STATIONARY : IMU_MOTION_STEADY_MOTION;
            conf = IMU_CONF_HIGH;
        }
        else if (sqrtf(fmaxf(p_proc->vib_var, 0.0f)) < vib_thr)
        {
            /* No vibration at all: very probably standing still (motors
             * off). Inferred, therefore LOW confidence. */
            cand = IMU_MOTION_STATIONARY;
            conf = IMU_CONF_LOW;
        }
        else
        {
            cand = IMU_MOTION_STEADY_MOTION;
            conf = IMU_CONF_LOW;
        }
    }

    if (cand == p_proc->motion)
    {
        p_proc->motion_cand = cand;
        p_proc->motion_conf = conf;
    }
    else if (b_immediate || (IMU_MOTION_UNKNOWN == p_proc->motion))
    {
        p_proc->motion      = cand;
        p_proc->motion_cand = cand;
        p_proc->motion_conf = conf;
    }
    else if (cand != p_proc->motion_cand)
    {
        p_proc->motion_cand       = cand;
        p_proc->motion_cand_start = now_ms;
    }
    else if ((now_ms - p_proc->motion_cand_start) >= p_params->class_hold_ms)
    {
        p_proc->motion      = cand;
        p_proc->motion_conf = conf;
    }
    else
    {
        /* candidate not yet stable: keep the current class */
    }

    p_out->motion           = p_proc->motion;
    p_out->motion_conf      = p_proc->motion_conf;
    p_out->b_motion_changed = (p_out->motion != p_out->motion_prev);
}

/* ------------------------------------------------------------------------ */
/*  Encoder hint                                                            */
/* ------------------------------------------------------------------------ */

imu_err_t imu_proc_set_hint(imu_proc_t * p_proc,
                            imu_motion_hint_t const * p_hint)
{
    imu_motion_hint_t prev;

    if ((NULL == p_proc) || (NULL == p_hint))
    {
        return (IMU_ERR_PARAM);
    }

    prev = p_proc->hint;

    if (!p_hint->b_valid)
    {
        p_proc->hint.b_valid      = false;
        p_proc->b_hint_accel_valid = false;
        return (IMU_OK);
    }

    if ((p_hint->speed_mm_s > IMU_CFG_HINT_MAX_SPEED_MM_S)
        || (p_hint->speed_mm_s < -IMU_CFG_HINT_MAX_SPEED_MM_S)
        || (p_hint->b_accel_valid
            && ((p_hint->accel_mm_s2 > IMU_CFG_HINT_MAX_ACCEL_MM_S2)
                || (p_hint->accel_mm_s2 < -IMU_CFG_HINT_MAX_ACCEL_MM_S2))))
    {
        p_proc->hints_rejected++;
        return (IMU_ERR_PARAM);   /* spoofed / corrupted: ignore */
    }

    if (p_hint->b_accel_valid)
    {
        p_proc->hint_accel_g = (float)p_hint->accel_mm_s2
                             / PROC_STANDARD_GRAVITY_MM_S2;
        p_proc->b_hint_accel_valid = true;
    }
    else if (prev.b_valid
             && ((p_hint->timestamp_ms - prev.timestamp_ms)
                 >= PROC_HINT_DIFF_MIN_MS)
             && ((p_hint->timestamp_ms - prev.timestamp_ms)
                 <= PROC_HINT_DIFF_MAX_MS))
    {
        float dv = (float)(p_hint->speed_mm_s - prev.speed_mm_s);
        float dt = (float)(p_hint->timestamp_ms - prev.timestamp_ms) * 0.001f;
        float a  = (dv / dt) / PROC_STANDARD_GRAVITY_MM_S2;

        p_proc->hint_accel_g = p_proc->b_hint_accel_valid
            ? imu_lpf_step(p_proc->hint_accel_g, a, PROC_HINT_DIFF_ALPHA)
            : a;
        p_proc->b_hint_accel_valid = true;
    }
    else
    {
        p_proc->b_hint_accel_valid = false;
    }

    p_proc->hint = *p_hint;
    return (IMU_OK);
}

bool imu_proc_hint_fresh(imu_proc_t const * p_proc, uint32_t now_ms)
{
    return ((NULL != p_proc) && p_proc->hint.b_valid
            && ((now_ms - p_proc->hint.timestamp_ms)
                <= p_proc->p_params->hint_max_age_ms));
}

/* ------------------------------------------------------------------------ */
/*  Names                                                                   */
/* ------------------------------------------------------------------------ */

char const * imu_motion_name(imu_motion_class_t motion)
{
    static char const * const k_names[IMU_MOTION_COUNT] =
    {
        "UNKNOWN", "STATIONARY", "STEADY_MOTION", "ACCELERATING",
        "DECELERATING", "TURNING_LEFT", "TURNING_RIGHT", "CLIMBING_HUMP",
        "DESCENDING_HUMP", "SUDDEN_IMPACT"
    };

    return (((uint32_t)motion < (uint32_t)IMU_MOTION_COUNT)
            ? k_names[motion] : "INVALID");
}

char const * imu_heading_quality_name(imu_heading_quality_t quality)
{
    char const * p_name;

    switch (quality)
    {
        case IMU_HEADING_INVALID:      p_name = "INVALID";      break;
        case IMU_HEADING_UNCALIBRATED: p_name = "UNCALIBRATED"; break;
        case IMU_HEADING_DISTURBED:    p_name = "DISTURBED";    break;
        case IMU_HEADING_GOOD:         p_name = "GOOD";         break;
        default:                       p_name = "INVALID";      break;
    }

    return (p_name);
}
```

## `include/imu/imu_hump.h`

```c
/**
 * @file    imu_hump.h
 * @brief   Buddy 4 hump detection and peak measurement.
 *
 * Detection model (docs Part 3 has the full derivation):
 *   A rigid car of wheelbase L crossing a hump pitches nose-UP while the front
 *   wheels climb, reaches a maximum pitch near the moment the front wheels
 *   reach the crest, and pitches nose-DOWN while the front wheels descend
 *   (and/or the rear wheels climb), before returning to level. The detector
 *   therefore requires the complete signature
 *
 *       level (armed) -> sustained +pitch (ASCENT) -> pitch maximum (PEAK)
 *       -> sustained -pitch (DESCENT) -> level again (complete)
 *
 *   inside a bounded time window. Isolated spikes fail the minimum-duration
 *   tests; a ramp/slope fails the "no descent" / "ascent too long" tests;
 *   a single acceleration step (pitch artefact from linear acceleration)
 *   fails the descent test; double counting is prevented by the re-arm
 *   condition (level for hump_rearm_level_ms and hump_min_gap_ms after the
 *   previous hump).
 *
 * Measurement:
 *   - PRIMARY (directly measured): peak nose-up pitch in degrees relative to
 *     the calibrated level attitude. Used to rank humps by default.
 *   - SECONDARY (estimate, explicit assumptions): physical height in mm.
 *       GEOMETRIC : h = L * sin(theta_peak)   valid if hump length <~ L
 *       INTEGRATED: h = max(integral(v*sin(theta) dt)) / imu_pos_frac
 *                   needs the Buddy 2 speed hint during the whole ascent.
 *     followed by an empirical linear correction (gain, offset) that must be
 *     fitted from humps of known height. Without that fit the estimate is
 *     reported with IMU_CONF_LOW.
 */

#ifndef IMU_HUMP_H
#define IMU_HUMP_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_config.h"
#include "imu/imu_types.h"

/** Validity of a hump measurement. */
typedef enum
{
    IMU_HUMP_VALID = 0,
    IMU_HUMP_PROVISIONAL,     /**< at the peak, traversal not complete */
    IMU_HUMP_REJ_LOW_PEAK,
    IMU_HUMP_REJ_NO_DESCENT,
    IMU_HUMP_REJ_TOO_SHORT,
    IMU_HUMP_REJ_TOO_LONG,
    IMU_HUMP_REJ_SLOPE,
    IMU_HUMP_REJ_ABORTED
} imu_hump_validity_t;

/** One hump measurement. */
typedef struct
{
    uint32_t            index;            /**< 1..n valid humps, 0 if not */
    imu_hump_validity_t validity;
    uint32_t            t_start_ms;
    uint32_t            t_peak_ms;
    uint32_t            t_end_ms;
    uint32_t            ascent_ms;
    uint32_t            total_ms;
    float               peak_pitch_deg;   /**< PRIMARY metric (measured)  */
    float               min_pitch_deg;    /**< most nose-down (negative)  */
    float               peak_vert_up_g;   /**< max upward dynamic accel   */
    float               peak_vert_down_g; /**< max downward (negative)    */
    float               height_est_mm;    /**< estimate, see conf/method  */
    imu_confidence_t    height_conf;
    imu_height_method_t height_method;
    float               rise_mm;          /**< integrated rise (if speed) */
    bool                b_rise_valid;
    float               length_est_mm;    /**< v * duration (if speed)    */
    bool                b_length_valid;
    bool                b_attitude_degraded;
    float               rank_value;       /**< value used for the maximum */
} imu_hump_meas_t;

/** Mission statistics. */
typedef struct
{
    uint32_t        valid_count;
    uint32_t        rejected_count;
    bool            b_has_last;
    imu_hump_meas_t last;           /**< most recently completed valid hump */
    bool            b_has_max;
    imu_hump_meas_t max;            /**< highest valid hump of the mission  */
} imu_hump_stats_t;

/** Inputs for one detector step. */
typedef struct
{
    float pitch_deg;              /**< filtered, levelled (+ hint-compensated)*/
    float vert_accel_g;           /**< cycle vertical dynamic acceleration  */
    bool  b_attitude_reliable;
    bool  b_speed_valid;          /**< fresh Buddy 2 speed hint available   */
    float speed_mm_s;
    bool  b_hint_accel_valid;
    float hint_accel_g;
} imu_hump_input_t;

/** Outputs of one detector step. */
typedef struct
{
    imu_hump_phase_t phase;
    bool             b_detected;   /**< ASCENT confirmed (HUMP_DETECTED)   */
    bool             b_peak;       /**< PEAK identified (HUMP_PEAK)        */
    bool             b_completed;  /**< traversal finished, VALID          */
    bool             b_rejected;   /**< candidate discarded                */
    bool             b_new_max;    /**< completed hump is the new maximum  */
    imu_hump_meas_t  meas;         /**< provisional or final measurement   */
} imu_hump_out_t;

/** Detector instance. */
typedef struct
{
    imu_params_t const * p_params;
    imu_hump_phase_t     phase;
    imu_hump_stats_t     stats;
    imu_hump_meas_t      cur;

    bool     b_armed;
    bool     b_level_timing;
    uint32_t level_start_ms;
    bool     b_has_last_end;
    uint32_t last_end_ms;

    bool     b_cand;
    uint32_t cand_start_ms;
    float    cand_peak_deg;
    uint32_t cand_peak_ms;
    bool     b_desc_cand;
    uint32_t desc_cand_start_ms;
    bool     b_lvl_cand;
    uint32_t lvl_cand_start_ms;

    uint32_t last_t_ms;
    float    rise_mm;
    bool     b_rise_ok;
    float    dist_mm;
    float    dist_at_peak_mm;
} imu_hump_t;

imu_err_t imu_hump_init(imu_hump_t * p_hump, imu_params_t const * p_params);

/** Clear run statistics (new mission / START_RUN). Phase is reset too. */
void imu_hump_reset_mission(imu_hump_t * p_hump);

/** Abort an in-progress hump (impact, fault). Counted as rejected. */
void imu_hump_abort(imu_hump_t * p_hump, uint32_t now_ms);

/** One detector step (call once per sampling period). */
void imu_hump_step(imu_hump_t * p_hump, uint32_t now_ms,
                   imu_hump_input_t const * p_in, imu_hump_out_t * p_out);

/** Height estimate for a given measurement (pure function, exported for
 *  testing and offline re-evaluation of logged peaks). */
void imu_hump_estimate_height(imu_params_t const * p_params,
                              imu_hump_meas_t * p_meas);

char const * imu_hump_validity_name(imu_hump_validity_t validity);

#endif /* IMU_HUMP_H */
```

## `src/imu/imu_hump.c`

```c
/**
 * @file    imu_hump.c
 * @brief   Buddy 4 hump detector (see imu_hump.h for the model).
 */

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "imu/imu_hump.h"
#include "imu/imu_math.h"

#define HUMP_RANK_EPS            (1.0e-3f)
#define HUMP_LONG_FACTOR         (1.5f)   /* hump longer than 1.5 L       */
#define HUMP_MAX_STEP_MS         (200U)   /* integration step bound       */
#define HUMP_CREST_MAX_WHEELBASES (1.5f)  /* crest->descent distance bound*/

static void hump_to_monitoring(imu_hump_t * p_hump, uint32_t now_ms)
{
    p_hump->phase          = IMU_HUMP_MONITORING;
    p_hump->b_armed        = false;
    p_hump->b_level_timing = false;
    p_hump->b_cand         = false;
    p_hump->b_desc_cand    = false;
    p_hump->b_lvl_cand     = false;
    p_hump->b_has_last_end = true;
    p_hump->last_end_ms    = now_ms;
}

imu_err_t imu_hump_init(imu_hump_t * p_hump, imu_params_t const * p_params)
{
    if ((NULL == p_hump) || (NULL == p_params))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_hump, 0, sizeof(*p_hump));
    p_hump->p_params = p_params;
    p_hump->phase    = IMU_HUMP_MONITORING;

    return (IMU_OK);
}

void imu_hump_reset_mission(imu_hump_t * p_hump)
{
    if (NULL == p_hump)
    {
        return;
    }

    (void)memset(&p_hump->stats, 0, sizeof(p_hump->stats));
    (void)memset(&p_hump->cur, 0, sizeof(p_hump->cur));
    p_hump->phase          = IMU_HUMP_MONITORING;
    p_hump->b_armed        = false;
    p_hump->b_level_timing = false;
    p_hump->b_cand         = false;
    p_hump->b_desc_cand    = false;
    p_hump->b_lvl_cand     = false;
    p_hump->b_has_last_end = false;
}

void imu_hump_abort(imu_hump_t * p_hump, uint32_t now_ms)
{
    if (NULL == p_hump)
    {
        return;
    }

    if (IMU_HUMP_MONITORING != p_hump->phase)
    {
        p_hump->stats.rejected_count++;
        p_hump->cur.validity = IMU_HUMP_REJ_ABORTED;
    }

    hump_to_monitoring(p_hump, now_ms);
}

void imu_hump_estimate_height(imu_params_t const * p_params,
                              imu_hump_meas_t * p_meas)
{
    imu_height_method_t method;
    float               h = 0.0f;
    imu_confidence_t    conf;

    if ((NULL == p_params) || (NULL == p_meas))
    {
        return;
    }

    method = p_params->height_method;
    if ((IMU_HEIGHT_METHOD_INTEGRATED == method) && !p_meas->b_rise_valid)
    {
        method = IMU_HEIGHT_METHOD_GEOMETRIC;   /* no speed: fall back */
    }

    conf = p_params->b_hump_height_calibrated ? IMU_CONF_MEDIUM
                                              : IMU_CONF_LOW;

    switch (method)
    {
        case IMU_HEIGHT_METHOD_GEOMETRIC:
            h = p_params->wheelbase_mm
              * sinf(fmaxf(p_meas->peak_pitch_deg, 0.0f) * IMU_DEG_TO_RAD);
            if (p_meas->b_length_valid
                && (p_meas->length_est_mm
                    > (HUMP_LONG_FACTOR * p_params->wheelbase_mm)))
            {
                conf = IMU_CONF_LOW;   /* long hump: pitch = slope only */
            }
            break;

        case IMU_HEIGHT_METHOD_INTEGRATED:
            h = p_meas->rise_mm / p_params->imu_pos_frac;
            break;

        default:
            conf = IMU_CONF_NONE;
            break;
    }

    if (IMU_CONF_NONE != conf)
    {
        h = (h * p_params->hump_height_gain) + p_params->hump_height_offset_mm;
        if ((0 == isfinite(h)) || (h < 0.0f))
        {
            h = 0.0f;
        }
        if (p_meas->b_attitude_degraded)
        {
            conf = IMU_CONF_LOW;
        }
    }

    p_meas->height_est_mm = h;
    p_meas->height_conf   = conf;
    p_meas->height_method = method;
}

static void hump_track(imu_hump_t * p_hump, uint32_t now_ms,
                       imu_hump_input_t const * p_in)
{
    imu_hump_meas_t * p_m = &p_hump->cur;
    uint32_t          dt_ms = now_ms - p_hump->last_t_ms;
    float             dt_s;

    if (dt_ms > HUMP_MAX_STEP_MS)
    {
        dt_ms = HUMP_MAX_STEP_MS;
    }
    dt_s = (float)dt_ms * 0.001f;

    if (p_in->vert_accel_g > p_m->peak_vert_up_g)
    {
        p_m->peak_vert_up_g = p_in->vert_accel_g;
    }
    if (p_in->vert_accel_g < p_m->peak_vert_down_g)
    {
        p_m->peak_vert_down_g = p_in->vert_accel_g;
    }
    if (!p_in->b_attitude_reliable)
    {
        p_m->b_attitude_degraded = true;
    }

    if (p_in->b_speed_valid)
    {
        p_hump->dist_mm += fabsf(p_in->speed_mm_s) * dt_s;
        if (IMU_HUMP_DESCENT != p_hump->phase)
        {
            p_hump->rise_mm += p_in->speed_mm_s
                             * sinf(p_in->pitch_deg * IMU_DEG_TO_RAD) * dt_s;
            if (p_hump->rise_mm > p_m->rise_mm)
            {
                p_m->rise_mm = p_hump->rise_mm;
            }
        }
    }
    else
    {
        p_hump->b_rise_ok = false;
    }
}

static void hump_finish(imu_hump_t * p_hump, uint32_t now_ms,
                        imu_hump_validity_t validity, imu_hump_out_t * p_out)
{
    imu_params_t const * p_params = p_hump->p_params;
    imu_hump_meas_t *    p_m      = &p_hump->cur;

    p_m->t_end_ms       = now_ms;
    p_m->total_ms       = now_ms - p_m->t_start_ms;
    p_m->b_rise_valid   = p_hump->b_rise_ok;
    p_m->b_length_valid = p_hump->b_rise_ok;
    p_m->length_est_mm  = p_hump->b_rise_ok ? p_hump->dist_mm : 0.0f;

    if (IMU_HUMP_VALID == validity)
    {
        if (p_m->peak_pitch_deg < p_params->hump_min_peak_deg)
        {
            validity = IMU_HUMP_REJ_LOW_PEAK;
        }
        else if (-p_m->min_pitch_deg < p_params->hump_min_descent_deg)
        {
            validity = IMU_HUMP_REJ_NO_DESCENT;
        }
        else if (p_m->total_ms < p_params->hump_min_event_ms)
        {
            validity = IMU_HUMP_REJ_TOO_SHORT;
        }
        else
        {
            /* complete, credible traversal */
        }
    }

    p_m->validity = validity;
    imu_hump_estimate_height(p_params, p_m);
    p_m->rank_value = ((IMU_RANK_BY_HEIGHT_EST == p_params->rank_metric)
                       && (IMU_CONF_NONE != p_m->height_conf))
                      ? p_m->height_est_mm : p_m->peak_pitch_deg;

    if (IMU_HUMP_VALID == validity)
    {
        p_hump->stats.valid_count++;
        p_m->index = p_hump->stats.valid_count;
        p_hump->stats.last       = *p_m;
        p_hump->stats.b_has_last = true;

        if (!p_hump->stats.b_has_max
            || (p_m->rank_value > (p_hump->stats.max.rank_value
                                   + HUMP_RANK_EPS)))
        {
            p_hump->stats.max       = *p_m;
            p_hump->stats.b_has_max = true;
            p_out->b_new_max        = true;
        }
        p_out->b_completed = true;
    }
    else
    {
        p_m->index = 0U;
        p_hump->stats.rejected_count++;
        p_out->b_rejected = true;
    }

    p_out->meas = *p_m;

    if (IMU_HUMP_VALID == validity)
    {
        hump_to_monitoring(p_hump, now_ms);
    }
    else
    {
        /* A rejected candidate must not block the next real hump: no
         * minimum-gap penalty, and an ongoing level period still counts
         * towards re-arming. */
        bool     b_was_level  = p_hump->b_lvl_cand;
        uint32_t level_start  = p_hump->lvl_cand_start_ms;
        bool     b_had_end    = p_hump->b_has_last_end;
        uint32_t last_end     = p_hump->last_end_ms;

        hump_to_monitoring(p_hump, now_ms);
        p_hump->b_has_last_end = b_had_end;
        p_hump->last_end_ms    = last_end;
        if (b_was_level)
        {
            p_hump->b_level_timing = true;
            p_hump->level_start_ms = level_start;
        }
    }
}

static bool hump_start_allowed(imu_hump_t const * p_hump,
                               imu_hump_input_t const * p_in)
{
    imu_params_t const * p_params = p_hump->p_params;
    bool                 b_ok     = true;

    if (p_in->b_speed_valid && p_params->b_hump_require_moving
        && (fabsf(p_in->speed_mm_s)
            <= (float)p_params->stationary_speed_mm_s))
    {
        b_ok = false;   /* vehicle not moving: lifted / pushed by hand */
    }

    if (p_in->b_hint_accel_valid
        && (fabsf(p_in->hint_accel_g) > p_params->hump_max_hint_accel_g))
    {
        b_ok = false;   /* strong acceleration corrupts the pitch */
    }

    return (b_ok);
}

static void hump_monitoring(imu_hump_t * p_hump, uint32_t now_ms,
                            imu_hump_input_t const * p_in,
                            imu_hump_out_t * p_out)
{
    imu_params_t const * p_params = p_hump->p_params;
    float                pitch    = p_in->pitch_deg;

    /* Re-arm: level for rearm time AND minimum gap since the last hump. */
    if (fabsf(pitch) < p_params->hump_level_band_deg)
    {
        if (!p_hump->b_level_timing)
        {
            p_hump->b_level_timing = true;
            p_hump->level_start_ms = now_ms;
        }
        if (((now_ms - p_hump->level_start_ms)
             >= p_params->hump_rearm_level_ms)
            && (!p_hump->b_has_last_end
                || ((now_ms - p_hump->last_end_ms)
                    >= p_params->hump_min_gap_ms)))
        {
            p_hump->b_armed = true;
        }
    }
    else
    {
        p_hump->b_level_timing = false;
    }

    if (p_hump->b_armed && (pitch > p_params->hump_ascent_enter_deg)
        && hump_start_allowed(p_hump, p_in))
    {
        if (!p_hump->b_cand)
        {
            p_hump->b_cand        = true;
            p_hump->cand_start_ms = now_ms;
            p_hump->cand_peak_deg = pitch;
            p_hump->cand_peak_ms  = now_ms;
        }
        else if (pitch > p_hump->cand_peak_deg)
        {
            /* fast hump: the maximum may occur before confirmation */
            p_hump->cand_peak_deg = pitch;
            p_hump->cand_peak_ms  = now_ms;
        }
        else
        {
            /* keep the candidate maximum */
        }

        if (p_hump->b_cand
            && ((now_ms - p_hump->cand_start_ms)
                >= p_params->hump_ascent_min_ms))
        {
            (void)memset(&p_hump->cur, 0, sizeof(p_hump->cur));
            p_hump->cur.t_start_ms     = p_hump->cand_start_ms;
            p_hump->cur.peak_pitch_deg = p_hump->cand_peak_deg;
            p_hump->cur.t_peak_ms      = p_hump->cand_peak_ms;
            p_hump->cur.validity       = IMU_HUMP_PROVISIONAL;
            p_hump->rise_mm            = 0.0f;
            p_hump->dist_mm            = 0.0f;
            p_hump->b_rise_ok          = p_in->b_speed_valid;
            p_hump->b_armed            = false;
            p_hump->b_cand             = false;
            p_hump->phase              = IMU_HUMP_ASCENT;
            p_hump->last_t_ms          = now_ms;
            hump_track(p_hump, now_ms, p_in);

            p_out->b_detected = true;
            p_out->meas       = p_hump->cur;
        }
    }
    else
    {
        p_hump->b_cand = false;
    }
}

void imu_hump_step(imu_hump_t * p_hump, uint32_t now_ms,
                   imu_hump_input_t const * p_in, imu_hump_out_t * p_out)
{
    imu_params_t const * p_params;
    imu_hump_meas_t *    p_m;
    float                pitch;
    uint32_t             elapsed;

    if ((NULL == p_hump) || (NULL == p_in) || (NULL == p_out))
    {
        return;
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    p_params = p_hump->p_params;
    p_m      = &p_hump->cur;
    pitch    = p_in->pitch_deg;

    if (0 == isfinite(pitch))
    {
        p_out->phase = p_hump->phase;
        return;   /* never feed NaN into the state machine */
    }

    if (IMU_HUMP_MONITORING == p_hump->phase)
    {
        hump_monitoring(p_hump, now_ms, p_in, p_out);
        p_out->phase = p_hump->phase;
        return;
    }

    hump_track(p_hump, now_ms, p_in);
    p_hump->last_t_ms = now_ms;
    elapsed = now_ms - p_m->t_start_ms;

    if (elapsed > p_params->hump_max_event_ms)
    {
        hump_finish(p_hump, now_ms,
                    (IMU_HUMP_DESCENT == p_hump->phase)
                        ? IMU_HUMP_REJ_TOO_LONG : IMU_HUMP_REJ_NO_DESCENT,
                    p_out);
        p_out->phase = p_hump->phase;
        return;
    }

    switch (p_hump->phase)
    {
        case IMU_HUMP_ASCENT:
            if (pitch > p_m->peak_pitch_deg)
            {
                p_m->peak_pitch_deg = pitch;
                p_m->t_peak_ms      = now_ms;
            }
            if (pitch <= (p_m->peak_pitch_deg - p_params->hump_peak_drop_deg))
            {
                p_m->ascent_ms = p_m->t_peak_ms - p_m->t_start_ms;
                p_hump->dist_at_peak_mm = p_hump->dist_mm;
                p_hump->b_lvl_cand      = false;
                p_hump->b_desc_cand     = false;
                p_hump->phase  = IMU_HUMP_PEAK;
                p_out->b_peak  = true;
                p_out->meas    = *p_m;
                imu_hump_estimate_height(p_params, &p_out->meas);
            }
            else if (elapsed > p_params->hump_ascent_max_ms)
            {
                hump_finish(p_hump, now_ms, IMU_HUMP_REJ_SLOPE, p_out);
            }
            else
            {
                /* still climbing */
            }
            break;

        case IMU_HUMP_PEAK:
            if (pitch > p_m->peak_pitch_deg)
            {
                p_m->peak_pitch_deg = pitch;   /* refine: noisy crest */
                p_m->t_peak_ms      = now_ms;
                p_m->ascent_ms      = now_ms - p_m->t_start_ms;
                p_hump->dist_at_peak_mm = p_hump->dist_mm;
            }
            /* Level after the "crest" and then a fresh climb: the previous
             * candidate was not a hump (e.g. acceleration artefact); restart
             * immediately so that a real hump right behind it is not lost. */
            if (fabsf(pitch) < p_params->hump_level_band_deg)
            {
                if (!p_hump->b_lvl_cand)
                {
                    p_hump->b_lvl_cand        = true;
                    p_hump->lvl_cand_start_ms = now_ms;
                }
            }
            else if ((pitch > p_params->hump_ascent_enter_deg)
                     && p_hump->b_lvl_cand
                     && ((now_ms - p_hump->lvl_cand_start_ms)
                         >= p_params->hump_rearm_level_ms))
            {
                hump_finish(p_hump, now_ms, IMU_HUMP_REJ_NO_DESCENT, p_out);
                p_hump->b_armed       = true;
                p_hump->b_cand        = true;
                p_hump->cand_start_ms = now_ms;
                break;
            }
            else if (pitch < -p_params->hump_level_band_deg)
            {
                p_hump->b_lvl_cand = false;   /* nose-down: not level */
            }
            else
            {
                /* between level band and ascent threshold: keep timing */
            }

            /* No nose-down phase after the crest: not a hump (typically
             * the pitch artefact of a forward acceleration, or a step). */
            if ((p_in->b_speed_valid && p_hump->b_rise_ok)
                ? ((p_hump->dist_mm - p_hump->dist_at_peak_mm)
                   > (HUMP_CREST_MAX_WHEELBASES * p_params->wheelbase_mm))
                : ((now_ms - p_m->t_peak_ms) > p_params->hump_crest_max_ms))
            {
                hump_finish(p_hump, now_ms, IMU_HUMP_REJ_NO_DESCENT, p_out);
                break;
            }
            if (pitch < -p_params->hump_descent_enter_deg)
            {
                if (!p_hump->b_desc_cand)
                {
                    p_hump->b_desc_cand        = true;
                    p_hump->desc_cand_start_ms = now_ms;
                    p_m->min_pitch_deg = fminf(p_m->min_pitch_deg, pitch);
                }
                else if ((now_ms - p_hump->desc_cand_start_ms)
                         >= p_params->hump_descent_min_ms)
                {
                    p_hump->b_desc_cand = false;
                    p_hump->b_lvl_cand  = false;
                    p_hump->phase       = IMU_HUMP_DESCENT;
                }
                else
                {
                    /* timing the descent candidate */
                }
                p_m->min_pitch_deg = fminf(p_m->min_pitch_deg, pitch);
            }
            else
            {
                p_hump->b_desc_cand = false;
            }
            break;

        case IMU_HUMP_DESCENT:
            p_m->min_pitch_deg = fminf(p_m->min_pitch_deg, pitch);
            if ((fabsf(pitch) < p_params->hump_level_band_deg)
                || (pitch > p_params->hump_level_band_deg))
            {
                if (!p_hump->b_lvl_cand)
                {
                    p_hump->b_lvl_cand        = true;
                    p_hump->lvl_cand_start_ms = now_ms;
                }
                else if ((now_ms - p_hump->lvl_cand_start_ms)
                         >= p_params->hump_level_min_ms)
                {
                    hump_finish(p_hump, p_hump->lvl_cand_start_ms,
                                IMU_HUMP_VALID, p_out);
                    /* re-arm timing starts now, not at the level start */
                    p_hump->last_end_ms = now_ms;
                }
                else
                {
                    /* timing the level condition */
                }
            }
            else
            {
                p_hump->b_lvl_cand = false;
            }
            break;

        default:
            hump_to_monitoring(p_hump, now_ms);
            break;
    }

    p_out->phase = p_hump->phase;
}

char const * imu_hump_validity_name(imu_hump_validity_t validity)
{
    char const * p_name;

    switch (validity)
    {
        case IMU_HUMP_VALID:          p_name = "VALID";          break;
        case IMU_HUMP_PROVISIONAL:    p_name = "PROVISIONAL";    break;
        case IMU_HUMP_REJ_LOW_PEAK:   p_name = "REJ_LOW_PEAK";   break;
        case IMU_HUMP_REJ_NO_DESCENT: p_name = "REJ_NO_DESCENT"; break;
        case IMU_HUMP_REJ_TOO_SHORT:  p_name = "REJ_TOO_SHORT";  break;
        case IMU_HUMP_REJ_TOO_LONG:   p_name = "REJ_TOO_LONG";   break;
        case IMU_HUMP_REJ_SLOPE:      p_name = "REJ_SLOPE";      break;
        case IMU_HUMP_REJ_ABORTED:    p_name = "REJ_ABORTED";    break;
        default:                      p_name = "UNKNOWN";        break;
    }

    return (p_name);
}
```

## `include/imu/imu_events.h`

```c
/**
 * @file    imu_events.h
 * @brief   Buddy 4 -> Mission Controller event messages (terrain_event_q)
 *          and the bounded, non-blocking event dispatcher.
 *
 * This header is intended to be included by the Mission Controller. It only
 * depends on <stdint.h>/<stdbool.h> and imu_types.h.
 *
 * Message format: fixed-size imu_event_t (copied by value into the queue).
 * All payload fields are integers in explicit units so that the Mission
 * Controller and Buddy 1 do not need floating point to interpret them:
 *     cdeg = 0.01 degree, cdps = 0.01 degree/s, mg = 0.001 g, mm, ms.
 *
 * Delivery guarantees (deterministic overflow policy):
 *   - Submission never blocks (TMO_POL on the uT-Kernel message buffer).
 *   - Non-critical events (tilt, turning, motion, hump progress): if the
 *     queue is full the NEW event is dropped and counted per type.
 *   - Critical events (IMPACT with CRITICAL severity, IMU_FAULT): if the
 *     queue is full they are kept in a small pending ring
 *     (IMU_EVENT_CRITICAL_PENDING) and retried every cycle, oldest first,
 *     before new events. They also trigger the direct notification hook
 *     (uT-Kernel event flag) immediately, independent of the queue.
 *   - Every event carries a 16-bit sequence number; a gap tells the
 *     receiver that events were dropped.
 */

#ifndef IMU_EVENTS_H
#define IMU_EVENTS_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_config.h"
#include "imu/imu_types.h"

/** Event types carried in terrain_event_q (Design Review Section 4). */
typedef enum
{
    IMU_EVT_HUMP_DETECTED = 1,  /**< credible ascent started               */
    IMU_EVT_HUMP_PEAK,          /**< crest identified (provisional value)  */
    IMU_EVT_HUMP_COMPLETED,     /**< traversal complete, final measurement */
    IMU_EVT_HUMP_REJECTED,      /**< candidate discarded (reason in data)  */
    IMU_EVT_TILT,               /**< TILT_EVENT                            */
    IMU_EVT_TURNING,            /**< TURNING_EVENT / heading change        */
    IMU_EVT_IMPACT,             /**< IMPACT / COLLISION                    */
    IMU_EVT_IMU_FAULT,          /**< IMU_FAULT (raised / persistent / ok)  */
    IMU_EVT_MOTION,             /**< motion classification changed        */
    IMU_EVT_STATUS,             /**< subsystem ready / calibration status  */
    IMU_EVT_TYPE_COUNT
} imu_event_type_t;

/** Status codes for IMU_EVT_IMU_FAULT and IMU_EVT_STATUS. */
typedef enum
{
    IMU_STATUS_READY = 0,          /**< calibration complete, monitoring   */
    IMU_STATUS_CAL_RETRY,          /**< calibration failed, retrying       */
    IMU_STATUS_FAULT_RAISED,
    IMU_STATUS_FAULT_PERSISTENT,   /**< recovery attempts exhausted        */
    IMU_STATUS_RECOVERED,
    IMU_STATUS_RESUMED,            /**< collision fault cleared            */
    IMU_STATUS_MISSION_RESET,
    IMU_STATUS_MAG_CAL_OK,
    IMU_STATUS_MAG_CAL_FAILED
} imu_status_code_t;

/** Hump payload. */
typedef struct
{
    uint32_t index;              /**< valid hump number (0 = not counted) */
    int16_t  peak_pitch_cdeg;    /**< PRIMARY, directly measured          */
    int16_t  min_pitch_cdeg;
    int16_t  peak_vert_up_mg;
    int16_t  peak_vert_down_mg;
    int16_t  height_est_mm;      /**< -1 if not available                 */
    uint8_t  height_conf;        /**< imu_confidence_t                    */
    uint8_t  validity;           /**< imu_hump_validity_t                 */
    uint16_t duration_ms;
    uint8_t  b_new_max;          /**< 1: this hump is the mission maximum */
    uint8_t  reserved;
    int16_t  max_peak_pitch_cdeg;/**< mission maximum after this event    */
    int16_t  max_height_est_mm;
} imu_evt_hump_t;

/** Tilt payload. */
typedef struct
{
    int16_t pitch_cdeg;
    int16_t roll_cdeg;
    int16_t tilt_cdeg;
    uint8_t phase;               /**< imu_event_phase_t                   */
    uint8_t b_reliable;
} imu_evt_tilt_t;

/** Turning payload. */
typedef struct
{
    uint8_t  direction;          /**< imu_turn_dir_t                      */
    uint8_t  phase;              /**< START / END / HEADING_CHANGE        */
    uint8_t  heading_quality;    /**< imu_heading_quality_t               */
    uint8_t  b_unreliable;
    int16_t  rate_cdps;          /**< heading-derived, NOT a gyro rate    */
    int16_t  heading_cdeg;       /**< 0..35999                            */
    int32_t  delta_cdeg;         /**< heading change of the turn          */
    uint32_t duration_ms;
} imu_evt_turn_t;

/** Impact payload. */
typedef struct
{
    int16_t  peak_mg;
    int16_t  horiz_mg;
    int16_t  vert_mg;
    uint16_t duration_ms;
    uint32_t suppressed;         /**< pulses suppressed by cool-down      */
    uint8_t  b_during_hump;
    uint8_t  reserved[3];
} imu_evt_impact_t;

/** Fault / status payload. */
typedef struct
{
    uint8_t  status;             /**< imu_status_code_t                   */
    uint8_t  fault;              /**< imu_fault_t                         */
    uint8_t  last_error;         /**< -(imu_err_t)                        */
    uint8_t  reserved;
    uint32_t attempt;
    uint32_t i2c_failures;
} imu_evt_fault_t;

/** Motion payload. */
typedef struct
{
    uint8_t motion;              /**< imu_motion_class_t                  */
    uint8_t previous;
    uint8_t confidence;          /**< imu_confidence_t                    */
    uint8_t reserved;
    int16_t fwd_accel_mg;
    int16_t vibration_mg;
} imu_evt_motion_t;

/** Message placed in terrain_event_q. */
typedef struct
{
    uint8_t  type;               /**< imu_event_type_t                    */
    uint8_t  severity;           /**< imu_severity_t                      */
    uint8_t  state;              /**< imu_state_t when generated          */
    uint8_t  b_valid;            /**< 0 = measurement degraded / partial  */
    uint32_t timestamp_ms;       /**< ms since boot                       */
    uint16_t seq;                /**< per-producer sequence number        */
    uint16_t reserved;
    union
    {
        imu_evt_hump_t   hump;
        imu_evt_tilt_t   tilt;
        imu_evt_turn_t   turn;
        imu_evt_impact_t impact;
        imu_evt_fault_t  fault;
        imu_evt_motion_t motion;
    } data;
} imu_event_t;

/** Result of a sink (queue) send. */
typedef enum
{
    IMU_SINK_OK = 0,
    IMU_SINK_FULL,
    IMU_SINK_ERROR
} imu_sink_result_t;

/** Non-blocking send function (e.g. tk_snd_mbf(..., TMO_POL)). */
typedef imu_sink_result_t (*imu_event_send_fn_t)(void * p_ctx,
                                                 imu_event_t const * p_evt);

/** Direct notification for critical events (e.g. tk_set_flg). Must not
 *  block. May be NULL. */
typedef void (*imu_critical_notify_fn_t)(void * p_ctx,
                                         imu_event_t const * p_evt);

/** Sink description. */
typedef struct
{
    imu_event_send_fn_t      p_send;
    imu_critical_notify_fn_t p_notify;
    void *                   p_ctx;
} imu_event_sink_t;

/** Dispatcher statistics. */
typedef struct
{
    uint32_t submitted;
    uint32_t sent;
    uint32_t dropped[IMU_EVT_TYPE_COUNT];
    uint32_t rate_limited[IMU_EVT_TYPE_COUNT];
    uint32_t critical_deferred;
    uint32_t critical_lost;      /**< pending ring overflow (should be 0) */
    uint32_t sink_errors;
    uint32_t notifications;
} imu_event_stats_t;

/** Bounded dispatcher (no allocation). */
typedef struct
{
    imu_event_sink_t  sink;
    uint16_t          next_seq;
    imu_event_t       pending[IMU_EVENT_CRITICAL_PENDING];
    uint8_t           pending_head;
    uint8_t           pending_count;
    uint32_t          min_interval_ms[IMU_EVT_TYPE_COUNT];
    uint32_t          last_sent_ms[IMU_EVT_TYPE_COUNT];
    bool              b_sent_once[IMU_EVT_TYPE_COUNT];
    imu_event_stats_t stats;
} imu_event_dispatcher_t;

/** Initialise. @p p_sink may be NULL (events are then counted as dropped). */
void imu_events_init(imu_event_dispatcher_t * p_disp,
                     imu_event_sink_t const * p_sink);

/** Optional per-type rate limit (0 = none). Critical types ignore it. */
void imu_events_set_rate_limit(imu_event_dispatcher_t * p_disp,
                               imu_event_type_t type, uint32_t min_ms);

/** @return true if this event must never be silently dropped. */
bool imu_events_is_critical(imu_event_t const * p_evt);

/**
 * @brief  Stamp (seq) and submit an event. Never blocks.
 * @return IMU_OK (sent), IMU_ERR_QUEUE_FULL (critical: deferred, or
 *         non-critical: dropped), IMU_ERR_BUSY (rate-limited),
 *         IMU_ERR_PARAM.
 */
imu_err_t imu_events_submit(imu_event_dispatcher_t * p_disp,
                            imu_event_t * p_evt, uint32_t now_ms);

/** Retry deferred critical events; call once per cycle. */
void imu_events_service(imu_event_dispatcher_t * p_disp);

/** @return number of deferred critical events. */
uint8_t imu_events_pending(imu_event_dispatcher_t const * p_disp);

char const * imu_event_type_name(imu_event_type_t type);

#endif /* IMU_EVENTS_H */
```

## `src/imu/imu_events.c`

```c
/**
 * @file    imu_events.c
 * @brief   Bounded, non-blocking event dispatcher (see imu_events.h).
 */

#include <stddef.h>
#include <string.h>

#include "imu/imu_events.h"

void imu_events_init(imu_event_dispatcher_t * p_disp,
                     imu_event_sink_t const * p_sink)
{
    if (NULL == p_disp)
    {
        return;
    }

    (void)memset(p_disp, 0, sizeof(*p_disp));
    if (NULL != p_sink)
    {
        p_disp->sink = *p_sink;
    }
}

void imu_events_set_rate_limit(imu_event_dispatcher_t * p_disp,
                               imu_event_type_t type, uint32_t min_ms)
{
    if ((NULL != p_disp) && ((uint32_t)type < (uint32_t)IMU_EVT_TYPE_COUNT))
    {
        p_disp->min_interval_ms[type] = min_ms;
    }
}

bool imu_events_is_critical(imu_event_t const * p_evt)
{
    bool b_crit = false;

    if (NULL != p_evt)
    {
        b_crit = (IMU_EVT_IMU_FAULT == p_evt->type)
              || ((IMU_EVT_IMPACT == p_evt->type)
                  && (IMU_SEV_CRITICAL == p_evt->severity))
              || ((IMU_EVT_TILT == p_evt->type)
                  && (IMU_SEV_CRITICAL == p_evt->severity));
    }

    return (b_crit);
}

static imu_sink_result_t events_send(imu_event_dispatcher_t * p_disp,
                                     imu_event_t const * p_evt)
{
    imu_sink_result_t res = IMU_SINK_ERROR;

    if (NULL != p_disp->sink.p_send)
    {
        res = p_disp->sink.p_send(p_disp->sink.p_ctx, p_evt);
    }
    if (IMU_SINK_OK == res)
    {
        p_disp->stats.sent++;
    }
    else if (IMU_SINK_ERROR == res)
    {
        p_disp->stats.sink_errors++;
    }
    else
    {
        /* full: handled by the caller */
    }

    return (res);
}

static void events_defer(imu_event_dispatcher_t * p_disp,
                         imu_event_t const * p_evt)
{
    uint8_t slot;

    if (p_disp->pending_count >= IMU_EVENT_CRITICAL_PENDING)
    {
        /* Ring full: overwrite the oldest (the newest critical state is the
         * most relevant one for the Mission Controller). */
        p_disp->pending_head = (uint8_t)((p_disp->pending_head + 1U)
                                         % IMU_EVENT_CRITICAL_PENDING);
        p_disp->pending_count--;
        p_disp->stats.critical_lost++;
    }

    slot = (uint8_t)((p_disp->pending_head + p_disp->pending_count)
                     % IMU_EVENT_CRITICAL_PENDING);
    p_disp->pending[slot] = *p_evt;
    p_disp->pending_count++;
    p_disp->stats.critical_deferred++;
}

void imu_events_service(imu_event_dispatcher_t * p_disp)
{
    if (NULL == p_disp)
    {
        return;
    }

    while (p_disp->pending_count > 0U)
    {
        imu_sink_result_t res =
            events_send(p_disp, &p_disp->pending[p_disp->pending_head]);

        if (IMU_SINK_OK != res)
        {
            break;   /* still full: retry next cycle, keep order */
        }
        p_disp->pending_head = (uint8_t)((p_disp->pending_head + 1U)
                                         % IMU_EVENT_CRITICAL_PENDING);
        p_disp->pending_count--;
    }
}

uint8_t imu_events_pending(imu_event_dispatcher_t const * p_disp)
{
    return ((NULL != p_disp) ? p_disp->pending_count : 0U);
}

imu_err_t imu_events_submit(imu_event_dispatcher_t * p_disp,
                            imu_event_t * p_evt, uint32_t now_ms)
{
    imu_sink_result_t res;
    bool              b_crit;
    uint8_t           type;

    if ((NULL == p_disp) || (NULL == p_evt)
        || (0U == p_evt->type)
        || ((uint32_t)p_evt->type >= (uint32_t)IMU_EVT_TYPE_COUNT))
    {
        return (IMU_ERR_PARAM);
    }

    type   = p_evt->type;
    b_crit = imu_events_is_critical(p_evt);
    p_disp->stats.submitted++;

    if (!b_crit && (p_disp->min_interval_ms[type] > 0U)
        && p_disp->b_sent_once[type]
        && ((now_ms - p_disp->last_sent_ms[type])
            < p_disp->min_interval_ms[type]))
    {
        p_disp->stats.rate_limited[type]++;
        return (IMU_ERR_BUSY);
    }

    p_evt->seq = p_disp->next_seq;
    p_disp->next_seq++;

    if (b_crit && (NULL != p_disp->sink.p_notify))
    {
        p_disp->sink.p_notify(p_disp->sink.p_ctx, p_evt);
        p_disp->stats.notifications++;
    }

    /* Critical events never overtake older deferred critical events. */
    if (b_crit && (p_disp->pending_count > 0U))
    {
        events_defer(p_disp, p_evt);
        imu_events_service(p_disp);
        return (IMU_ERR_QUEUE_FULL);
    }

    res = events_send(p_disp, p_evt);
    if (IMU_SINK_OK == res)
    {
        p_disp->last_sent_ms[type] = now_ms;
        p_disp->b_sent_once[type]  = true;
        return (IMU_OK);
    }

    if (b_crit)
    {
        events_defer(p_disp, p_evt);
    }
    else
    {
        p_disp->stats.dropped[type]++;
    }

    return (IMU_ERR_QUEUE_FULL);
}

char const * imu_event_type_name(imu_event_type_t type)
{
    static char const * const k_names[IMU_EVT_TYPE_COUNT] =
    {
        "NONE", "HUMP_DETECTED", "HUMP_PEAK", "HUMP_COMPLETED",
        "HUMP_REJECTED", "TILT_EVENT", "TURNING_EVENT", "IMPACT/COLLISION",
        "IMU_FAULT", "MOTION", "STATUS"
    };

    return (((uint32_t)type < (uint32_t)IMU_EVT_TYPE_COUNT)
            ? k_names[type] : "INVALID");
}
```

## `include/imu/imu_terrain.h`

```c
/**
 * @file    imu_terrain.h
 * @brief   Buddy 4 subsystem state machine (Design Review Section 6.4):
 *          INITIALISATION -> CALIBRATION -> MONITORING
 *          { HUMP_ASCENT -> HUMP_PEAK -> HUMP_DESCENT },
 *          TILT/TURN EVENT reporting, IMPACT EVENT -> FAULT HANDLING,
 *          SENSOR FAULT -> FAULT/SAFE STATE -> RECOVERY -> MONITORING.
 *
 * RTOS-independent: imu_terrain_step() performs ONE sampling period of work
 * and is called by the uT-Kernel IMU task (imu_rtos.c) or by a test harness.
 * It is NOT thread-safe by itself; imu_rtos.c provides the locking.
 *
 * Mapping of the statechart to the implementation (docs Part 3):
 *   - HUMP PROCESSING is delegated to imu_hump.c; its phase is mirrored into
 *     the subsystem state (MONITORING / HUMP_ASCENT / HUMP_PEAK /
 *     HUMP_DESCENT).
 *   - TILT EVENT -> EVENT REPORTING -> MONITORING and TURN EVENT -> EVENT
 *     REPORTING -> MONITORING are run-to-completion transient states inside
 *     one step: they are recorded in the transition trace but never block
 *     sampling, so tilt/turn detection can run in parallel with hump
 *     processing (a hump can be crossed during a gentle turn).
 *   - IMPACT EVENT -> FAULT HANDLING -> FAULT/SAFE STATE for CRITICAL impacts
 *     (MINOR impacts are reported and monitoring continues, configurable).
 *   - SENSOR FAULT -> FAULT/SAFE STATE; RECOVERY verifies the sensor before
 *     MONITORING resumes. A collision fault additionally needs the Mission
 *     Controller's CLEAR_FAULT acknowledgement and settled conditions.
 *   - Unresolved faults never silently return to MONITORING.
 */

#ifndef IMU_TERRAIN_H
#define IMU_TERRAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "imu/imu_calibration.h"
#include "imu/imu_config.h"
#include "imu/imu_events.h"
#include "imu/imu_hump.h"
#include "imu/imu_lsm303dlhc.h"
#include "imu/imu_processing.h"
#include "imu/imu_types.h"

/** Commands accepted by the subsystem (bit mask, processed next step). */
#define IMU_CMD_MISSION_START   (0x0001U)  /**< reset run statistics      */
#define IMU_CMD_RESET           (0x0002U)  /**< back to INITIALISATION    */
#define IMU_CMD_CLEAR_FAULT     (0x0004U)  /**< MC acknowledges collision */
#define IMU_CMD_MAG_CAL_BEGIN   (0x0008U)  /**< start magnetometer cal    */
#define IMU_CMD_MAG_CAL_END     (0x0010U)  /**< finish magnetometer cal   */
#define IMU_CMD_RECALIBRATE     (0x0020U)  /**< redo stationary cal       */
#define IMU_CMD_STOP            (0x0040U)  /**< shutdown sensor           */
#define IMU_CMD_ALL             (0x007FU)

/** One entry of the state-transition trace. */
typedef struct
{
    uint32_t t_ms;
    uint8_t  from;      /**< imu_state_t */
    uint8_t  to;        /**< imu_state_t */
} imu_trace_entry_t;

/** Complete status snapshot (what other tasks may read). */
typedef struct
{
    uint32_t              timestamp_ms;
    imu_state_t           state;
    imu_fault_t           fault;
    imu_err_t             last_error;
    bool                  b_calibrated;
    bool                  b_sensor_ok;
    bool                  b_mag_cal_valid;
    bool                  b_mag_cal_active;

    imu_motion_class_t    motion;
    imu_confidence_t      motion_conf;

    float                 pitch_deg;
    float                 roll_deg;
    float                 tilt_deg;
    bool                  b_attitude_reliable;
    bool                  b_tilted;

    float                 heading_deg;
    imu_heading_quality_t heading_quality;
    float                 turn_rate_dps;
    bool                  b_turn_rate_valid;
    imu_turn_dir_t        turn_dir;

    float                 fwd_accel_g;
    float                 vert_accel_g;
    float                 vibration_g;
    float                 mag_field_gauss;

    imu_hump_phase_t      hump_phase;
    imu_hump_stats_t      hump;
    bool                  b_has_impact;
    imu_impact_t          last_impact;

    float                 cal_g_ref;
    float                 cal_noise_g;
    float                 cal_mount_tilt_deg;
    imu_mag_cal_t         mag_cal;

    lsm303_stats_t        i2c;
    imu_event_stats_t     events;
    uint32_t              cycles;
    uint32_t              consecutive_failures;
    uint32_t              recovery_attempts;
    uint32_t              hints_rejected;
} imu_status_t;

/** Compact integer telemetry record for Buddy 1 (no floats). */
typedef struct
{
    uint32_t timestamp_ms;
    uint8_t  state;              /**< imu_state_t                       */
    uint8_t  motion;             /**< imu_motion_class_t                */
    uint8_t  hump_phase;         /**< imu_hump_phase_t                  */
    uint8_t  fault;              /**< imu_fault_t                       */
    int16_t  pitch_cdeg;
    int16_t  roll_cdeg;
    int16_t  heading_cdeg;       /**< -1 = heading invalid              */
    int16_t  turn_rate_cdps;
    uint16_t hump_count;
    int16_t  last_hump_peak_cdeg;  /**< -1 = none                       */
    int16_t  hump_peak_cdeg;       /**< mission max pitch, -1 = none    */
    int16_t  hump_peak_height_mm;  /**< mission max height est, -1 = n/a*/
    uint8_t  hump_height_conf;     /**< imu_confidence_t                */
    uint8_t  b_sensor_ok;
} imu_telemetry_t;

/** Subsystem instance. */
typedef struct
{
    imu_params_t           params;
    lsm303_t               dev;
    imu_proc_t             proc;
    imu_hump_t             hump;
    imu_event_dispatcher_t disp;

    imu_state_t            state;
    imu_fault_t            fault;
    imu_err_t              last_error;
    uint32_t               next_action_ms;
    uint32_t               attempts;
    uint32_t               retry_interval_ms;
    bool                   b_persistent_reported;

    imu_accel_cal_acc_t    cal_acc;
    uint32_t               cal_attempts;
    bool                   b_cal_fault_reported;
    imu_mag_cal_acc_t      mag_acc;
    bool                   b_mag_cal_active;

    imu_raw3_t             accel_buf[IMU_ACCEL_BATCH_MAX];
    imu_raw3_t             last_raw;
    bool                   b_last_raw_valid;
    uint32_t               frozen_count;
    uint32_t               consec_fail;
    uint32_t               consec_invalid;
    uint32_t               last_accel_ok_ms;
    uint32_t               verify_count;

    bool                   b_fault_ack;
    bool                   b_settle_timing;
    uint32_t               settle_start_ms;

    uint32_t               pending_cmds;
    imu_proc_out_t         last_out;
    imu_status_t           status;

    imu_trace_entry_t      trace[IMU_STATE_TRACE_LEN];
    uint8_t                trace_head;
    uint8_t                trace_count;
    uint32_t               cycles;
} imu_terrain_t;

/**
 * @brief  Initialise the subsystem (no I/O; the sensor is initialised in the
 *         first imu_terrain_step()).
 * @param  p_params  parameter set (copied); NULL = defaults.
 * @param  p_sink    event sink (copied); NULL = events counted as dropped.
 */
imu_err_t imu_terrain_init(imu_terrain_t * p_sys,
                           imu_params_t const * p_params,
                           imu_event_sink_t const * p_sink);

/** One sampling period. @p now_ms = milliseconds since boot. */
void imu_terrain_step(imu_terrain_t * p_sys, uint32_t now_ms);

/** Post commands (IMU_CMD_*), executed at the start of the next step. */
imu_err_t imu_terrain_command(imu_terrain_t * p_sys, uint32_t cmd_mask);

/** Supply the optional Buddy 2 motion hint. */
imu_err_t imu_terrain_set_hint(imu_terrain_t * p_sys,
                               imu_motion_hint_t const * p_hint);

/** Copy the latest status snapshot. */
imu_err_t imu_terrain_get_status(imu_terrain_t const * p_sys,
                                 imu_status_t * p_out);

/** Copy the transition trace (oldest first). @return entries copied. */
uint8_t imu_terrain_get_trace(imu_terrain_t const * p_sys,
                              imu_trace_entry_t * p_out, uint8_t max);

/** Convert a status snapshot into the compact telemetry record. */
void imu_terrain_make_telemetry(imu_status_t const * p_status,
                                imu_telemetry_t * p_out);

/**
 * @brief  Format the Buddy 4 telemetry fields as a JSON fragment (no braces)
 *         for Buddy 1's serializer, e.g.
 *         "hump_peak":523,"hump_peak_mm":12,"hump_count":2,...
 *         "hump_peak" is in centi-degrees of peak pitch (directly measured).
 * @return number of characters written (excluding NUL), or -1 if @p len is
 *         too small (buffer is then an empty string).
 */
int32_t imu_terrain_telemetry_json(imu_telemetry_t const * p_tele,
                                   char * p_buf, size_t len);

char const * imu_state_name(imu_state_t state);
char const * imu_fault_name(imu_fault_t fault);

#endif /* IMU_TERRAIN_H */
```

## `src/imu/imu_terrain.c`

```c
/**
 * @file    imu_terrain.c
 * @brief   Buddy 4 subsystem state machine (see imu_terrain.h).
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "imu/imu_terrain.h"

#define TERRAIN_PLAUS_MAX_FACTOR   (1.8f)   /* > sqrt(3) * full scale     */
#define TERRAIN_CDEG               (100.0f)
#define TERRAIN_MG                 (1000.0f)
#define TERRAIN_I16_MAX            (32767.0f)
#define TERRAIN_NONE_I16           ((int16_t)-1)   /* 'not available' */

/* ------------------------------------------------------------------------ */
/*  Small helpers                                                           */
/* ------------------------------------------------------------------------ */

static bool time_reached(uint32_t now_ms, uint32_t deadline_ms)
{
    return ((int32_t)(now_ms - deadline_ms) >= 0);
}

static int16_t to_i16(float value, float scale)
{
    float v = value * scale;

    if (0 == isfinite(v))
    {
        return (0);
    }
    v = imu_clampf(v, -TERRAIN_I16_MAX, TERRAIN_I16_MAX);

    return ((int16_t)lroundf(v));
}

/** to_i16() if @p b_avail, otherwise the 'not available' marker (-1). */
static int16_t to_i16_or_none(bool b_avail, float value, float scale)
{
    int16_t r = TERRAIN_NONE_I16;

    if (b_avail)
    {
        r = to_i16(value, scale);
    }

    return (r);
}

static void terrain_trace(imu_terrain_t * p_sys, uint32_t now_ms,
                          imu_state_t from, imu_state_t to)
{
    uint8_t slot = (uint8_t)((p_sys->trace_head + p_sys->trace_count)
                             % IMU_STATE_TRACE_LEN);

    if (p_sys->trace_count >= IMU_STATE_TRACE_LEN)
    {
        slot = p_sys->trace_head;
        p_sys->trace_head = (uint8_t)((p_sys->trace_head + 1U)
                                      % IMU_STATE_TRACE_LEN);
    }
    else
    {
        p_sys->trace_count++;
    }

    p_sys->trace[slot].t_ms = now_ms;
    p_sys->trace[slot].from = (uint8_t)from;
    p_sys->trace[slot].to   = (uint8_t)to;
}

static void terrain_set_state(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_state_t next)
{
    if (next != p_sys->state)
    {
        terrain_trace(p_sys, now_ms, p_sys->state, next);
        p_sys->state = next;
    }
}

/** Transient (run-to-completion) state: logged, then back to @p back. */
static void terrain_transient(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_state_t transient)
{
    imu_state_t back = p_sys->state;

    terrain_trace(p_sys, now_ms, back, transient);
    terrain_trace(p_sys, now_ms, transient, back);
}

static void terrain_evt_init(imu_terrain_t const * p_sys, imu_event_t * p_e,
                             imu_event_type_t type, imu_severity_t sev,
                             uint32_t now_ms)
{
    (void)memset(p_e, 0, sizeof(*p_e));
    p_e->type         = (uint8_t)type;
    p_e->severity     = (uint8_t)sev;
    p_e->state        = (uint8_t)p_sys->state;
    p_e->b_valid      = 1U;
    p_e->timestamp_ms = now_ms;
}

static void terrain_emit_status(imu_terrain_t * p_sys, uint32_t now_ms,
                                imu_event_type_t type, imu_severity_t sev,
                                imu_status_code_t code, imu_err_t err)
{
    imu_event_t e;

    terrain_evt_init(p_sys, &e, type, sev, now_ms);
    e.data.fault.status       = (uint8_t)code;
    e.data.fault.fault        = (uint8_t)p_sys->fault;
    e.data.fault.last_error   = (uint8_t)(-(int32_t)err);
    e.data.fault.attempt      = p_sys->attempts;
    e.data.fault.i2c_failures = p_sys->dev.stats.failed_transfers;
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static lsm303_config_t terrain_dev_cfg(imu_params_t const * p_params)
{
    lsm303_config_t cfg;

    cfg.accel_odr_code  = p_params->accel_odr_code;
    cfg.accel_fs_code   = p_params->accel_fs_code;
    cfg.b_accel_fifo    = p_params->b_accel_fifo;
    cfg.mag_odr_code    = p_params->mag_odr_code;
    cfg.mag_gain_code   = p_params->mag_gain_code;
    cfg.b_mag_id_check  = p_params->b_mag_id_check;
    cfg.i2c_baud_hz     = p_params->i2c_baud_hz;
    cfg.i2c_timeout_us  = p_params->i2c_timeout_us;
    cfg.i2c_max_retries = p_params->i2c_max_retries;

    return (cfg);
}

/* ------------------------------------------------------------------------ */
/*  Acquisition                                                             */
/* ------------------------------------------------------------------------ */

typedef struct
{
    imu_err_t  err;            /* bus error, NO_DATA (stale) or OK         */
    uint8_t    count;          /* accel samples this cycle                 */
    bool       b_valid;        /* accel average plausible                  */
    bool       b_frozen;
    imu_vec3_t accel_avg_g;    /* sensor frame                             */
    bool       b_mag_ok;
    imu_vec3_t mag_gauss;      /* sensor frame, uncalibrated               */
} terrain_sample_t;

static void terrain_acquire(imu_terrain_t * p_sys, uint32_t now_ms,
                            terrain_sample_t * p_s)
{
    imu_params_t const * p_params = &p_sys->params;
    bool                 b_overrun = false;
    bool                 b_mag_ovf = false;
    imu_raw3_t           mag_raw;
    imu_vec3_t           sum = { 0.0f, 0.0f, 0.0f };
    float                norm;
    uint8_t              i;

    (void)memset(p_s, 0, sizeof(*p_s));

    p_s->err = lsm303_read_accel(&p_sys->dev, p_sys->accel_buf,
                                 (uint8_t)IMU_ACCEL_BATCH_MAX, &p_s->count,
                                 &b_overrun);
    if (IMU_OK != p_s->err)
    {
        return;
    }

    if (p_s->count > 0U)
    {
        imu_raw3_t const * p_last = &p_sys->accel_buf[p_s->count - 1U];

        p_sys->last_accel_ok_ms = now_ms;

        for (i = 0U; i < p_s->count; i++)
        {
            imu_vec3_t g;

            lsm303_accel_to_g(&p_sys->dev, &p_sys->accel_buf[i], &g);
            sum.x += g.x;
            sum.y += g.y;
            sum.z += g.z;
        }
        imu_vec_scale(&sum, 1.0f / (float)p_s->count, &p_s->accel_avg_g);

        norm = imu_vec_norm(&p_s->accel_avg_g);
        p_s->b_valid = (0 != isfinite(norm))
                    && (norm >= p_params->accel_plaus_min_g)
                    && (norm <= (TERRAIN_PLAUS_MAX_FACTOR
                                 * p_sys->dev.accel_full_scale_g));

        /* Frozen output: identical 12-bit triples cycle after cycle. */
        if (p_sys->b_last_raw_valid && (p_last->x == p_sys->last_raw.x)
            && (p_last->y == p_sys->last_raw.y)
            && (p_last->z == p_sys->last_raw.z))
        {
            p_sys->frozen_count++;
        }
        else
        {
            p_sys->frozen_count = 0U;
        }
        p_sys->last_raw         = *p_last;
        p_sys->b_last_raw_valid = true;
        p_s->b_frozen = (p_sys->frozen_count >= p_params->frozen_cycles);
    }
    else if ((now_ms - p_sys->last_accel_ok_ms) > p_params->stale_data_ms)
    {
        p_s->err = IMU_ERR_NO_DATA;
        return;
    }
    else
    {
        /* no new sample this cycle (timing jitter): not an error */
    }

    p_s->err = lsm303_read_mag(&p_sys->dev, &mag_raw, &b_mag_ovf);
    if (IMU_OK == p_s->err)
    {
        lsm303_mag_to_gauss(&p_sys->dev, &mag_raw, &p_s->mag_gauss);
        p_s->b_mag_ok = !b_mag_ovf;
    }
}

/* ------------------------------------------------------------------------ */
/*  Fault entry                                                             */
/* ------------------------------------------------------------------------ */

static void terrain_enter_sensor_fault(imu_terrain_t * p_sys,
                                       uint32_t now_ms, imu_fault_t cause,
                                       imu_err_t err)
{
    terrain_set_state(p_sys, now_ms, IMU_STATE_SENSOR_FAULT);
    p_sys->fault      = cause;
    p_sys->last_error = err;
    imu_hump_abort(&p_sys->hump, now_ms);

    p_sys->attempts              = 0U;
    p_sys->retry_interval_ms     = p_sys->params.recovery_interval_ms;
    p_sys->next_action_ms        = now_ms + p_sys->retry_interval_ms;
    p_sys->b_persistent_reported = false;

    terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL,
                        IMU_STATUS_FAULT_RAISED, err);
    terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_SAFE);
}

/**
 * @return true if the sample may be processed; false if it was a failure
 *         (and possibly a fault transition happened).
 */
static bool terrain_check_sample(imu_terrain_t * p_sys, uint32_t now_ms,
                                 terrain_sample_t const * p_s)
{
    imu_params_t const * p_params = &p_sys->params;

    if (IMU_ERR_NO_DATA == p_s->err)
    {
        p_sys->last_error = p_s->err;
        terrain_enter_sensor_fault(p_sys, now_ms, IMU_FAULT_STALE_DATA,
                                   p_s->err);
        return (false);
    }

    if (IMU_OK != p_s->err)
    {
        p_sys->consec_fail++;
        p_sys->last_error = p_s->err;
        if (p_sys->consec_fail >= p_params->fault_consec_cycles)
        {
            terrain_enter_sensor_fault(p_sys, now_ms, IMU_FAULT_I2C,
                                       p_s->err);
        }
        return (false);
    }
    p_sys->consec_fail = 0U;

    if (0U == p_s->count)
    {
        return (false);   /* nothing new, not a failure */
    }

    if (p_s->b_frozen)
    {
        terrain_enter_sensor_fault(p_sys, now_ms, IMU_FAULT_FROZEN_DATA,
                                   IMU_ERR_INVALID_DATA);
        return (false);
    }

    if (!p_s->b_valid)
    {
        p_sys->consec_invalid++;
        p_sys->last_error = IMU_ERR_INVALID_DATA;
        if (p_sys->consec_invalid >= p_params->fault_consec_cycles)
        {
            terrain_enter_sensor_fault(p_sys, now_ms, IMU_FAULT_INVALID_DATA,
                                       IMU_ERR_INVALID_DATA);
        }
        return (false);
    }
    p_sys->consec_invalid = 0U;

    return (true);
}

/* ------------------------------------------------------------------------ */
/*  Event builders                                                          */
/* ------------------------------------------------------------------------ */

static void terrain_emit_hump(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_event_type_t type,
                              imu_hump_meas_t const * p_m, bool b_new_max)
{
    imu_event_t              e;
    imu_hump_stats_t const * p_st = &p_sys->hump.stats;

    terrain_evt_init(p_sys, &e, type, IMU_SEV_INFO, now_ms);
    e.b_valid = (uint8_t)(((IMU_HUMP_VALID == p_m->validity)
                           || (IMU_HUMP_PROVISIONAL == p_m->validity))
                          ? 1U : 0U);
    e.data.hump.index             = p_m->index;
    e.data.hump.peak_pitch_cdeg   = to_i16(p_m->peak_pitch_deg, TERRAIN_CDEG);
    e.data.hump.min_pitch_cdeg    = to_i16(p_m->min_pitch_deg, TERRAIN_CDEG);
    e.data.hump.peak_vert_up_mg   = to_i16(p_m->peak_vert_up_g, TERRAIN_MG);
    e.data.hump.peak_vert_down_mg = to_i16(p_m->peak_vert_down_g, TERRAIN_MG);
    e.data.hump.height_est_mm     =
        to_i16_or_none((IMU_CONF_NONE != p_m->height_conf),
                       p_m->height_est_mm, 1.0f);
    e.data.hump.height_conf       = (uint8_t)p_m->height_conf;
    e.data.hump.validity          = (uint8_t)p_m->validity;
    e.data.hump.duration_ms       = (uint16_t)((p_m->total_ms > 65535U)
                                               ? 65535U : p_m->total_ms);
    e.data.hump.b_new_max         = b_new_max ? 1U : 0U;
    e.data.hump.max_peak_pitch_cdeg =
        to_i16_or_none(p_st->b_has_max, p_st->max.peak_pitch_deg,
                       TERRAIN_CDEG);
    e.data.hump.max_height_est_mm =
        to_i16_or_none(p_st->b_has_max
                       && (IMU_CONF_NONE != p_st->max.height_conf),
                       p_st->max.height_est_mm, 1.0f);

    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static void terrain_emit_tilt(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_proc_out_t const * p_out)
{
    imu_event_t e;

    terrain_transient(p_sys, now_ms, IMU_STATE_TILT_EVENT);
    terrain_evt_init(p_sys, &e, IMU_EVT_TILT, p_out->tilt_severity, now_ms);
    e.data.tilt.pitch_cdeg = to_i16(p_out->pitch_deg, TERRAIN_CDEG);
    e.data.tilt.roll_cdeg  = to_i16(p_out->roll_deg, TERRAIN_CDEG);
    e.data.tilt.tilt_cdeg  = to_i16(p_out->tilt_deg, TERRAIN_CDEG);
    e.data.tilt.phase      = (uint8_t)p_out->tilt_phase;
    e.data.tilt.b_reliable = p_out->b_attitude_reliable ? 1U : 0U;
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static void terrain_emit_turn(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_proc_out_t const * p_out)
{
    imu_event_t e;
    float       delta = p_out->turn_event_delta_deg * TERRAIN_CDEG;

    terrain_transient(p_sys, now_ms, IMU_STATE_TURN_EVENT);
    terrain_evt_init(p_sys, &e, IMU_EVT_TURNING, IMU_SEV_INFO, now_ms);
    e.b_valid = (uint8_t)(p_out->b_turn_event_unreliable ? 0U : 1U);
    e.data.turn.direction       = (uint8_t)p_out->turn_event_dir;
    e.data.turn.phase           = (uint8_t)p_out->turn_phase;
    e.data.turn.heading_quality = (uint8_t)p_out->heading_quality;
    e.data.turn.b_unreliable    = p_out->b_turn_event_unreliable ? 1U : 0U;
    e.data.turn.rate_cdps       = to_i16(p_out->turn_rate_dps, TERRAIN_CDEG);
    e.data.turn.heading_cdeg    = to_i16(p_out->heading_deg, TERRAIN_CDEG);
    e.data.turn.delta_cdeg      = (0 != isfinite(delta))
                                  ? (int32_t)lroundf(imu_clampf(delta,
                                        -2.0e9f, 2.0e9f)) : 0;
    e.data.turn.duration_ms     = p_out->turn_event_duration_ms;
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static void terrain_emit_motion(imu_terrain_t * p_sys, uint32_t now_ms,
                                imu_proc_out_t const * p_out)
{
    imu_event_t e;

    terrain_evt_init(p_sys, &e, IMU_EVT_MOTION, IMU_SEV_INFO, now_ms);
    e.data.motion.motion       = (uint8_t)p_out->motion;
    e.data.motion.previous     = (uint8_t)p_out->motion_prev;
    e.data.motion.confidence   = (uint8_t)p_out->motion_conf;
    e.data.motion.fwd_accel_mg = to_i16(p_out->fwd_accel_g, TERRAIN_MG);
    e.data.motion.vibration_mg = to_i16(p_out->vibration_g, TERRAIN_MG);
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static void terrain_emit_impact(imu_terrain_t * p_sys, uint32_t now_ms,
                                imu_impact_t const * p_imp, bool b_hump)
{
    imu_event_t e;
    uint32_t    dur = p_imp->duration_ms;

    terrain_evt_init(p_sys, &e, IMU_EVT_IMPACT, p_imp->severity, now_ms);
    e.data.impact.peak_mg       = to_i16(p_imp->peak_g, TERRAIN_MG);
    e.data.impact.horiz_mg      = to_i16(p_imp->horiz_g, TERRAIN_MG);
    e.data.impact.vert_mg       = to_i16(p_imp->vert_g, TERRAIN_MG);
    e.data.impact.duration_ms   = (uint16_t)((dur > 65535U) ? 65535U : dur);
    e.data.impact.suppressed    = p_imp->suppressed;
    e.data.impact.b_during_hump = b_hump ? 1U : 0U;
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

/* ------------------------------------------------------------------------ */
/*  Processing of one valid sample                                          */
/* ------------------------------------------------------------------------ */

/** @return true if a fault-level impact occurred. */
static bool terrain_process(imu_terrain_t * p_sys, uint32_t now_ms,
                            terrain_sample_t const * p_s, bool b_detect)
{
    imu_params_t const * p_params = &p_sys->params;
    imu_proc_out_t *     p_out    = &p_sys->last_out;
    imu_impact_t         worst;
    imu_hump_input_t     hin;
    imu_hump_out_t       hout;
    bool                 b_hump   = (IMU_HUMP_MONITORING
                                     != p_sys->hump.phase);
    bool                 b_impact = false;
    bool                 b_fault  = false;
    uint32_t             odr_hz   = imu_params_accel_odr_hz(
                                        p_params->accel_odr_code);
    uint32_t             odr_ms   = (odr_hz > 0U) ? (1000U / odr_hz) : 0U;
    uint8_t              i;

    (void)memset(&worst, 0, sizeof(worst));

    /* High-rate impact path over every FIFO sample (oldest first). */
    for (i = 0U; i < p_s->count; i++)
    {
        imu_vec3_t   g;
        imu_impact_t imp;
        uint32_t     t_i = now_ms - ((uint32_t)(p_s->count - 1U - i)
                                     * odr_ms);

        lsm303_accel_to_g(&p_sys->dev, &p_sys->accel_buf[i], &g);
        imu_proc_hf_accel(&p_sys->proc, &g, t_i, b_hump, &imp);
        if (imp.b_detected
            && (!worst.b_detected || (imp.severity > worst.severity)
                || ((imp.severity == worst.severity)
                    && (imp.peak_g > worst.peak_g))))
        {
            worst = imp;
        }
    }

    if (p_sys->b_mag_cal_active && p_s->b_mag_ok)
    {
        imu_cal_mag_add(&p_sys->mag_acc, &p_s->mag_gauss);
    }

    imu_proc_step(&p_sys->proc, &p_s->accel_avg_g,
                  p_s->b_mag_ok ? &p_s->mag_gauss : NULL, now_ms, b_hump,
                  p_out);

    if (worst.b_detected && b_detect)
    {
        b_impact = true;
        p_sys->status.last_impact  = worst;
        p_sys->status.b_has_impact = true;

        /* IMPACT EVENT (classify severity) */
        b_fault = (IMU_SEV_CRITICAL == worst.severity)
               || p_params->b_impact_minor_enters_fault;

        if (b_fault)
        {
            /* FAULT HANDLING: report collision, then FAULT / SAFE STATE */
            terrain_set_state(p_sys, now_ms, IMU_STATE_IMPACT_EVENT);
            terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_HANDLING);
            terrain_emit_impact(p_sys, now_ms, &worst, b_hump);
            imu_hump_abort(&p_sys->hump, now_ms);
            p_sys->fault           = IMU_FAULT_COLLISION;
            p_sys->b_fault_ack     = false;
            p_sys->b_settle_timing = false;
            terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_SAFE);
        }
        else
        {
            /* Minor impact: report and keep monitoring (transient). */
            terrain_transient(p_sys, now_ms, IMU_STATE_IMPACT_EVENT);
            terrain_emit_impact(p_sys, now_ms, &worst, b_hump);
        }
    }

    if (b_detect && !b_fault)
    {
        if (p_out->b_tilt_event)
        {
            terrain_emit_tilt(p_sys, now_ms, p_out);
        }
        if (p_out->b_turn_event)
        {
            terrain_emit_turn(p_sys, now_ms, p_out);
        }

        hin.pitch_deg           = p_out->pitch_deg;
        hin.vert_accel_g        = p_out->vert_accel_g;
        hin.b_attitude_reliable = p_out->b_attitude_reliable;
        hin.b_speed_valid       = imu_proc_hint_fresh(&p_sys->proc, now_ms);
        hin.speed_mm_s          = (float)p_sys->proc.hint.speed_mm_s;
        hin.b_hint_accel_valid  = hin.b_speed_valid
                                  && p_sys->proc.b_hint_accel_valid;
        hin.hint_accel_g        = p_sys->proc.hint_accel_g;
        imu_hump_step(&p_sys->hump, now_ms, &hin, &hout);

        if (hout.b_detected)
        {
            terrain_emit_hump(p_sys, now_ms, IMU_EVT_HUMP_DETECTED,
                              &hout.meas, false);
        }
        if (hout.b_peak)
        {
            terrain_emit_hump(p_sys, now_ms, IMU_EVT_HUMP_PEAK,
                              &hout.meas, false);
        }
        if (hout.b_completed)
        {
            terrain_emit_hump(p_sys, now_ms, IMU_EVT_HUMP_COMPLETED,
                              &hout.meas, hout.b_new_max);
        }
        if (hout.b_rejected)
        {
            terrain_emit_hump(p_sys, now_ms, IMU_EVT_HUMP_REJECTED,
                              &hout.meas, false);
        }

        switch (hout.phase)
        {
            case IMU_HUMP_ASCENT:
                terrain_set_state(p_sys, now_ms, IMU_STATE_HUMP_ASCENT);
                break;
            case IMU_HUMP_PEAK:
                terrain_set_state(p_sys, now_ms, IMU_STATE_HUMP_PEAK);
                break;
            case IMU_HUMP_DESCENT:
                terrain_set_state(p_sys, now_ms, IMU_STATE_HUMP_DESCENT);
                break;
            default:
                terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
                break;
        }
    }

    imu_proc_classify(&p_sys->proc, now_ms, p_sys->hump.phase, b_impact,
                      p_out);
    if (p_out->b_motion_changed && b_detect)
    {
        terrain_emit_motion(p_sys, now_ms, p_out);
    }

    return (b_fault);
}

/* ------------------------------------------------------------------------ */
/*  State handlers                                                          */
/* ------------------------------------------------------------------------ */

static void terrain_begin_calibration(imu_terrain_t * p_sys, uint32_t now_ms)
{
    imu_cal_accel_begin(&p_sys->cal_acc);
    p_sys->consec_fail      = 0U;
    p_sys->consec_invalid   = 0U;
    p_sys->last_accel_ok_ms = now_ms;
    terrain_set_state(p_sys, now_ms, IMU_STATE_CALIBRATION);
}

static void terrain_do_init(imu_terrain_t * p_sys, uint32_t now_ms)
{
    lsm303_config_t cfg;
    imu_err_t       err;

    if (!time_reached(now_ms, p_sys->next_action_ms))
    {
        return;
    }

    cfg = terrain_dev_cfg(&p_sys->params);
    err = lsm303_init(&p_sys->dev, &cfg);

    if (IMU_OK == err)
    {
        p_sys->attempts         = 0U;
        p_sys->fault            = IMU_FAULT_NONE;
        p_sys->b_last_raw_valid = false;
        p_sys->frozen_count     = 0U;
        p_sys->cal_attempts     = 0U;
        p_sys->b_cal_fault_reported = false;
        terrain_begin_calibration(p_sys, now_ms);
        return;
    }

    p_sys->attempts++;
    p_sys->last_error = err;
    p_sys->fault      = IMU_FAULT_INIT_FAILED;

    if (p_sys->attempts == p_sys->params.init_max_attempts)
    {
        terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT,
                            IMU_SEV_CRITICAL, IMU_STATUS_FAULT_RAISED, err);
    }

    /* Retry with exponential back-off (bounded). */
    if (p_sys->attempts >= p_sys->params.init_max_attempts)
    {
        p_sys->retry_interval_ms = (p_sys->retry_interval_ms
                                    < (p_sys->params.recovery_interval_max_ms
                                       / 2U))
                                   ? (p_sys->retry_interval_ms * 2U)
                                   : p_sys->params.recovery_interval_max_ms;
    }
    else
    {
        p_sys->retry_interval_ms = p_sys->params.init_retry_ms;
    }
    p_sys->next_action_ms = now_ms + p_sys->retry_interval_ms;
}

static void terrain_do_calibration(imu_terrain_t * p_sys, uint32_t now_ms)
{
    terrain_sample_t s;
    imu_accel_cal_t  cal;
    imu_vec3_t       v;
    imu_err_t        err;

    if (!time_reached(now_ms, p_sys->next_action_ms))
    {
        return;
    }

    terrain_acquire(p_sys, now_ms, &s);
    if (!terrain_check_sample(p_sys, now_ms, &s))
    {
        return;
    }

    imu_proc_map_accel(&p_sys->proc, &s.accel_avg_g, &v);
    imu_cal_accel_add(&p_sys->cal_acc, &v);

    if (p_sys->cal_acc.n < p_sys->params.cal_samples)
    {
        return;
    }

    err = imu_cal_accel_finish(&p_sys->cal_acc, &p_sys->params, &cal);
    if (IMU_OK == err)
    {
        imu_proc_set_calibration(&p_sys->proc, &cal, NULL);
        p_sys->status.cal_g_ref          = cal.g_ref;
        p_sys->status.cal_noise_g        = cal.noise_max_std_g;
        p_sys->status.cal_mount_tilt_deg = cal.mount_tilt_deg;
        p_sys->cal_attempts = 0U;
        p_sys->fault        = IMU_FAULT_NONE;
        terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
        terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS, IMU_SEV_INFO,
                            IMU_STATUS_READY, IMU_OK);
        return;
    }

    /* Calibration failed -> CALIBRATION again (statechart), bounded report */
    p_sys->cal_attempts++;
    p_sys->last_error = err;
    p_sys->attempts   = p_sys->cal_attempts;
    terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS, IMU_SEV_MINOR,
                        IMU_STATUS_CAL_RETRY, err);

    if ((p_sys->cal_attempts >= p_sys->params.cal_max_attempts)
        && !p_sys->b_cal_fault_reported)
    {
        p_sys->fault = IMU_FAULT_CAL_FAILED;
        p_sys->b_cal_fault_reported = true;
        terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT,
                            IMU_SEV_CRITICAL, IMU_STATUS_FAULT_RAISED, err);
    }

    imu_cal_accel_begin(&p_sys->cal_acc);
    p_sys->next_action_ms = now_ms + p_sys->params.cal_retry_ms;
}

static void terrain_do_monitoring(imu_terrain_t * p_sys, uint32_t now_ms)
{
    terrain_sample_t s;

    terrain_acquire(p_sys, now_ms, &s);
    if (terrain_check_sample(p_sys, now_ms, &s))
    {
        (void)terrain_process(p_sys, now_ms, &s, true);
    }
}

/** A recovery attempt failed (re-init or verification): back-off and, once
 *  the attempt budget is exhausted, report the fault as PERSISTENT (once).
 *  The subsystem keeps trying at the maximum interval but never resumes
 *  without a successful verification. */
static void terrain_recovery_failed(imu_terrain_t * p_sys, uint32_t now_ms,
                                    imu_err_t err)
{
    imu_params_t const * p_params = &p_sys->params;

    p_sys->last_error = err;
    p_sys->retry_interval_ms = (p_sys->retry_interval_ms
                                < (p_params->recovery_interval_max_ms / 2U))
                               ? (p_sys->retry_interval_ms * 2U)
                               : p_params->recovery_interval_max_ms;
    p_sys->next_action_ms = now_ms + p_sys->retry_interval_ms;

    if ((p_sys->attempts >= p_params->recovery_max_attempts)
        && !p_sys->b_persistent_reported)
    {
        p_sys->b_persistent_reported = true;
        terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT,
                            IMU_SEV_CRITICAL, IMU_STATUS_FAULT_PERSISTENT,
                            err);
    }
}

static void terrain_do_fault_safe(imu_terrain_t * p_sys, uint32_t now_ms)
{
    imu_params_t const * p_params = &p_sys->params;
    terrain_sample_t     s;
    imu_err_t            err;

    if (IMU_FAULT_COLLISION == p_sys->fault)
    {
        /* Sensor still works: keep the status current, wait for the MC
         * acknowledgement and settled conditions. */
        terrain_acquire(p_sys, now_ms, &s);
        if (!terrain_check_sample(p_sys, now_ms, &s))
        {
            return;   /* may have escalated to a sensor fault */
        }
        (void)terrain_process(p_sys, now_ms, &s, false);

        if (p_sys->last_out.b_attitude_reliable
            && (p_sys->last_out.tilt_deg < p_params->tilt_threshold_deg))
        {
            if (!p_sys->b_settle_timing)
            {
                p_sys->b_settle_timing = true;
                p_sys->settle_start_ms = now_ms;
            }
        }
        else
        {
            p_sys->b_settle_timing = false;
        }

        if (p_sys->b_fault_ack && p_sys->b_settle_timing
            && ((now_ms - p_sys->settle_start_ms)
                >= p_params->collision_settle_ms))
        {
            p_sys->fault       = IMU_FAULT_NONE;
            p_sys->b_fault_ack = false;
            terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
            terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS, IMU_SEV_INFO,
                                IMU_STATUS_RESUMED, IMU_OK);
        }
        return;
    }

    /* Sensor fault: bounded recovery attempts with back-off. */
    if (!time_reached(now_ms, p_sys->next_action_ms))
    {
        return;
    }

    p_sys->attempts++;
    err = lsm303_recover(&p_sys->dev);

    if (IMU_OK == err)
    {
        p_sys->verify_count     = 0U;
        p_sys->consec_fail      = 0U;
        p_sys->consec_invalid   = 0U;
        p_sys->b_last_raw_valid = false;
        p_sys->frozen_count     = 0U;
        p_sys->last_accel_ok_ms = now_ms;
        terrain_set_state(p_sys, now_ms, IMU_STATE_RECOVERY);
        return;
    }

    terrain_recovery_failed(p_sys, now_ms, err);
}

static void terrain_do_recovery(imu_terrain_t * p_sys, uint32_t now_ms)
{
    terrain_sample_t s;

    terrain_acquire(p_sys, now_ms, &s);

    if ((IMU_OK != s.err) || ((s.count > 0U) && (!s.b_valid || s.b_frozen)))
    {
        /* Verification failed: back to FAULT / SAFE STATE. */
        terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_SAFE);
        terrain_recovery_failed(p_sys, now_ms,
                                (IMU_OK != s.err) ? s.err
                                                  : IMU_ERR_INVALID_DATA);
        return;
    }

    if (s.count > 0U)
    {
        p_sys->verify_count++;
    }

    /* Enough plausible samples AND evidence that the output is alive
     * (a frozen sensor would otherwise pass a short verification). */
    if ((p_sys->verify_count < p_sys->params.recovery_verify_samples)
        || ((p_sys->frozen_count + 1U) >= p_sys->verify_count))
    {
        return;
    }

    p_sys->fault = IMU_FAULT_NONE;
    if (p_sys->proc.accel_cal.b_valid)
    {
        imu_proc_reset_dynamic(&p_sys->proc);
        terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
    }
    else
    {
        terrain_begin_calibration(p_sys, now_ms);
    }
    terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL,
                        IMU_STATUS_RECOVERED, IMU_OK);
}

/* ------------------------------------------------------------------------ */
/*  Commands                                                                */
/* ------------------------------------------------------------------------ */

static bool terrain_is_operational(imu_state_t state)
{
    return ((IMU_STATE_MONITORING == state)
            || (IMU_STATE_HUMP_ASCENT == state)
            || (IMU_STATE_HUMP_PEAK == state)
            || (IMU_STATE_HUMP_DESCENT == state));
}

static void terrain_handle_commands(imu_terrain_t * p_sys, uint32_t now_ms)
{
    uint32_t cmds = p_sys->pending_cmds;

    p_sys->pending_cmds = 0U;

    if (0U != (cmds & IMU_CMD_STOP))
    {
        lsm303_shutdown(&p_sys->dev);
        imu_hump_abort(&p_sys->hump, now_ms);
        terrain_set_state(p_sys, now_ms, IMU_STATE_STOPPED);
        return;
    }

    if (0U != (cmds & IMU_CMD_RESET))
    {
        imu_hump_abort(&p_sys->hump, now_ms);
        p_sys->fault             = IMU_FAULT_NONE;
        p_sys->attempts          = 0U;
        p_sys->retry_interval_ms = p_sys->params.init_retry_ms;
        p_sys->next_action_ms    = now_ms;
        p_sys->b_mag_cal_active  = false;
        p_sys->proc.accel_cal.b_valid = false;
        terrain_set_state(p_sys, now_ms, IMU_STATE_INITIALISATION);
    }

    if (0U != (cmds & IMU_CMD_MISSION_START))
    {
        imu_hump_reset_mission(&p_sys->hump);
        p_sys->status.b_has_impact = false;
        terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS, IMU_SEV_INFO,
                            IMU_STATUS_MISSION_RESET, IMU_OK);
    }

    if (0U != (cmds & IMU_CMD_CLEAR_FAULT))
    {
        p_sys->b_fault_ack = true;
    }

    if ((0U != (cmds & IMU_CMD_RECALIBRATE))
        && terrain_is_operational(p_sys->state))
    {
        imu_hump_abort(&p_sys->hump, now_ms);
        p_sys->cal_attempts = 0U;
        p_sys->next_action_ms = now_ms;
        terrain_begin_calibration(p_sys, now_ms);
    }

    if ((0U != (cmds & IMU_CMD_MAG_CAL_BEGIN))
        && terrain_is_operational(p_sys->state))
    {
        imu_cal_mag_begin(&p_sys->mag_acc);
        p_sys->b_mag_cal_active = true;
    }

    if ((0U != (cmds & IMU_CMD_MAG_CAL_END)) && p_sys->b_mag_cal_active)
    {
        imu_mag_cal_t mag;
        imu_err_t     err = imu_cal_mag_finish(&p_sys->mag_acc,
                                               &p_sys->params, &mag);

        p_sys->b_mag_cal_active = false;
        if (IMU_OK == err)
        {
            imu_proc_set_calibration(&p_sys->proc, NULL, &mag);
            imu_hump_abort(&p_sys->hump, now_ms);
            terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
        }
        terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS,
                            (IMU_OK == err) ? IMU_SEV_INFO : IMU_SEV_MINOR,
                            (IMU_OK == err) ? IMU_STATUS_MAG_CAL_OK
                                            : IMU_STATUS_MAG_CAL_FAILED,
                            err);
    }
}

/* ------------------------------------------------------------------------ */
/*  Status                                                                  */
/* ------------------------------------------------------------------------ */

static void terrain_update_status(imu_terrain_t * p_sys, uint32_t now_ms)
{
    imu_status_t *         p_st  = &p_sys->status;
    imu_proc_out_t const * p_out = &p_sys->last_out;

    p_st->timestamp_ms        = now_ms;
    p_st->state               = p_sys->state;
    p_st->fault               = p_sys->fault;
    p_st->last_error          = p_sys->last_error;
    p_st->b_calibrated        = p_sys->proc.accel_cal.b_valid;
    p_st->b_sensor_ok         = p_sys->dev.b_ready
                                && (IMU_STATE_FAULT_SAFE != p_sys->state
                                    || (IMU_FAULT_COLLISION == p_sys->fault))
                                && (IMU_STATE_RECOVERY != p_sys->state)
                                && (IMU_STATE_INITIALISATION != p_sys->state);
    p_st->b_mag_cal_valid     = p_sys->proc.mag_cal.b_valid;
    p_st->b_mag_cal_active    = p_sys->b_mag_cal_active;
    p_st->mag_cal             = p_sys->proc.mag_cal;

    p_st->motion              = p_sys->proc.motion;
    p_st->motion_conf         = p_sys->proc.motion_conf;
    p_st->pitch_deg           = p_out->pitch_deg;
    p_st->roll_deg            = p_out->roll_deg;
    p_st->tilt_deg            = p_out->tilt_deg;
    p_st->b_attitude_reliable = p_out->b_attitude_reliable;
    p_st->b_tilted            = p_sys->proc.b_tilted;
    p_st->heading_deg         = p_out->heading_deg;
    p_st->heading_quality     = p_out->heading_quality;
    p_st->turn_rate_dps       = p_out->turn_rate_dps;
    p_st->b_turn_rate_valid   = p_out->b_turn_rate_valid;
    p_st->turn_dir            = p_out->turn_dir;
    p_st->fwd_accel_g         = p_out->fwd_accel_g;
    p_st->vert_accel_g        = p_out->vert_accel_g;
    p_st->vibration_g         = p_out->vibration_g;
    p_st->mag_field_gauss     = p_out->mag_field_gauss;

    p_st->hump_phase          = p_sys->hump.phase;
    p_st->hump                = p_sys->hump.stats;
    p_st->i2c                 = p_sys->dev.stats;
    p_st->events              = p_sys->disp.stats;
    p_st->cycles              = p_sys->cycles;
    p_st->consecutive_failures = p_sys->consec_fail;
    p_st->recovery_attempts   = p_sys->attempts;
    p_st->hints_rejected      = p_sys->proc.hints_rejected;
}

/* ------------------------------------------------------------------------ */
/*  Public API                                                              */
/* ------------------------------------------------------------------------ */

imu_err_t imu_terrain_init(imu_terrain_t * p_sys,
                           imu_params_t const * p_params,
                           imu_event_sink_t const * p_sink)
{
    imu_err_t err;

    if (NULL == p_sys)
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_sys, 0, sizeof(*p_sys));

    if (NULL != p_params)
    {
        p_sys->params = *p_params;
    }
    else
    {
        imu_params_default(&p_sys->params);
    }

    err = imu_params_validate(&p_sys->params);
    if (IMU_OK != err)
    {
        return (err);
    }

    err = imu_proc_init(&p_sys->proc, &p_sys->params);
    if (IMU_OK == err)
    {
        err = imu_hump_init(&p_sys->hump, &p_sys->params);
    }
    if (IMU_OK != err)
    {
        return (err);
    }

    imu_events_init(&p_sys->disp, p_sink);
    imu_events_set_rate_limit(&p_sys->disp, IMU_EVT_MOTION,
                              p_sys->params.motion_evt_min_ms);
    imu_events_set_rate_limit(&p_sys->disp, IMU_EVT_TILT,
                              p_sys->params.tilt_evt_min_ms);
    imu_events_set_rate_limit(&p_sys->disp, IMU_EVT_TURNING,
                              p_sys->params.turn_evt_min_ms);

    p_sys->state             = IMU_STATE_INITIALISATION;
    p_sys->fault             = IMU_FAULT_NONE;
    p_sys->last_error        = IMU_OK;
    p_sys->retry_interval_ms = p_sys->params.init_retry_ms;
    p_sys->last_out.motion   = IMU_MOTION_UNKNOWN;
    p_sys->last_out.heading_quality = IMU_HEADING_INVALID;
    terrain_update_status(p_sys, 0U);

    return (IMU_OK);
}

void imu_terrain_step(imu_terrain_t * p_sys, uint32_t now_ms)
{
    if (NULL == p_sys)
    {
        return;
    }

    if (0U == p_sys->cycles)
    {
        p_sys->next_action_ms = now_ms;   /* first call: act immediately */
    }
    p_sys->cycles++;

    terrain_handle_commands(p_sys, now_ms);
    imu_events_service(&p_sys->disp);

    switch (p_sys->state)
    {
        case IMU_STATE_INITIALISATION:
            terrain_do_init(p_sys, now_ms);
            break;

        case IMU_STATE_CALIBRATION:
            terrain_do_calibration(p_sys, now_ms);
            break;

        case IMU_STATE_MONITORING:
        case IMU_STATE_HUMP_ASCENT:
        case IMU_STATE_HUMP_PEAK:
        case IMU_STATE_HUMP_DESCENT:
            terrain_do_monitoring(p_sys, now_ms);
            break;

        case IMU_STATE_FAULT_SAFE:
            terrain_do_fault_safe(p_sys, now_ms);
            break;

        case IMU_STATE_RECOVERY:
            terrain_do_recovery(p_sys, now_ms);
            break;

        case IMU_STATE_STOPPED:
            break;

        default:
            /* Transient states never persist between steps; defensive. */
            terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_SAFE);
            break;
    }

    terrain_update_status(p_sys, now_ms);
}

imu_err_t imu_terrain_command(imu_terrain_t * p_sys, uint32_t cmd_mask)
{
    if ((NULL == p_sys) || (0U == cmd_mask)
        || (0U != (cmd_mask & ~IMU_CMD_ALL)))
    {
        return (IMU_ERR_PARAM);
    }

    p_sys->pending_cmds |= cmd_mask;
    return (IMU_OK);
}

imu_err_t imu_terrain_set_hint(imu_terrain_t * p_sys,
                               imu_motion_hint_t const * p_hint)
{
    if ((NULL == p_sys) || (NULL == p_hint))
    {
        return (IMU_ERR_PARAM);
    }

    return (imu_proc_set_hint(&p_sys->proc, p_hint));
}

imu_err_t imu_terrain_get_status(imu_terrain_t const * p_sys,
                                 imu_status_t * p_out)
{
    if ((NULL == p_sys) || (NULL == p_out))
    {
        return (IMU_ERR_PARAM);
    }

    *p_out = p_sys->status;
    return (IMU_OK);
}

uint8_t imu_terrain_get_trace(imu_terrain_t const * p_sys,
                              imu_trace_entry_t * p_out, uint8_t max)
{
    uint8_t i;
    uint8_t n;

    if ((NULL == p_sys) || (NULL == p_out))
    {
        return (0U);
    }

    n = (p_sys->trace_count < max) ? p_sys->trace_count : max;
    for (i = 0U; i < n; i++)
    {
        p_out[i] = p_sys->trace[(p_sys->trace_head
                                 + (p_sys->trace_count - n) + i)
                                % IMU_STATE_TRACE_LEN];
    }

    return (n);
}

void imu_terrain_make_telemetry(imu_status_t const * p_status,
                                imu_telemetry_t * p_out)
{
    imu_hump_stats_t const * p_h;

    if ((NULL == p_status) || (NULL == p_out))
    {
        return;
    }

    p_h = &p_status->hump;
    (void)memset(p_out, 0, sizeof(*p_out));
    p_out->timestamp_ms   = p_status->timestamp_ms;
    p_out->state          = (uint8_t)p_status->state;
    p_out->motion         = (uint8_t)p_status->motion;
    p_out->hump_phase     = (uint8_t)p_status->hump_phase;
    p_out->fault          = (uint8_t)p_status->fault;
    p_out->pitch_cdeg     = to_i16(p_status->pitch_deg, TERRAIN_CDEG);
    p_out->roll_cdeg      = to_i16(p_status->roll_deg, TERRAIN_CDEG);
    p_out->heading_cdeg   =
        to_i16_or_none(IMU_HEADING_INVALID != p_status->heading_quality,
                       p_status->heading_deg, TERRAIN_CDEG);
    p_out->turn_rate_cdps = to_i16(p_status->b_turn_rate_valid
                                   ? p_status->turn_rate_dps : 0.0f,
                                   TERRAIN_CDEG);
    p_out->hump_count     = (uint16_t)((p_h->valid_count > 65535U)
                                       ? 65535U : p_h->valid_count);
    p_out->last_hump_peak_cdeg =
        to_i16_or_none(p_h->b_has_last, p_h->last.peak_pitch_deg,
                       TERRAIN_CDEG);
    p_out->hump_peak_cdeg =
        to_i16_or_none(p_h->b_has_max, p_h->max.peak_pitch_deg,
                       TERRAIN_CDEG);
    p_out->hump_peak_height_mm =
        to_i16_or_none(p_h->b_has_max
                       && (IMU_CONF_NONE != p_h->max.height_conf),
                       p_h->max.height_est_mm, 1.0f);
    p_out->hump_height_conf = p_h->b_has_max ? (uint8_t)p_h->max.height_conf
                                             : (uint8_t)IMU_CONF_NONE;
    p_out->b_sensor_ok    = p_status->b_sensor_ok ? 1U : 0U;
}

int32_t imu_terrain_telemetry_json(imu_telemetry_t const * p_tele,
                                   char * p_buf, size_t len)
{
    int n;

    if ((NULL == p_tele) || (NULL == p_buf) || (0U == len))
    {
        return (-1);
    }

    n = snprintf(p_buf, len,
                 "\"hump_peak\":%d,\"hump_peak_mm\":%d,\"hump_count\":%u,"
                 "\"imu_state\":%u,\"motion\":%u,\"pitch\":%d,"
                 "\"heading\":%d,\"imu_ok\":%u",
                 (int)p_tele->hump_peak_cdeg,
                 (int)p_tele->hump_peak_height_mm,
                 (unsigned)p_tele->hump_count, (unsigned)p_tele->state,
                 (unsigned)p_tele->motion, (int)p_tele->pitch_cdeg,
                 (int)p_tele->heading_cdeg, (unsigned)p_tele->b_sensor_ok);

    if ((n < 0) || ((size_t)n >= len))
    {
        p_buf[0] = '\0';
        return (-1);
    }

    return ((int32_t)n);
}

char const * imu_state_name(imu_state_t state)
{
    static char const * const k_names[IMU_STATE_COUNT] =
    {
        "INITIALISATION", "CALIBRATION", "MONITORING", "HUMP_ASCENT",
        "HUMP_PEAK", "HUMP_DESCENT", "TILT_EVENT", "TURN_EVENT",
        "IMPACT_EVENT", "FAULT_HANDLING", "SENSOR_FAULT", "FAULT_SAFE",
        "RECOVERY", "STOPPED"
    };

    return (((uint32_t)state < (uint32_t)IMU_STATE_COUNT)
            ? k_names[state] : "INVALID");
}

char const * imu_fault_name(imu_fault_t fault)
{
    static char const * const k_names[] =
    {
        "NONE", "INIT_FAILED", "CAL_FAILED", "I2C", "STALE_DATA",
        "INVALID_DATA", "FROZEN_DATA", "COLLISION"
    };

    return (((uint32_t)fault <= (uint32_t)IMU_FAULT_COLLISION)
            ? k_names[fault] : "INVALID");
}
```

## `include/imu/imu_rtos.h`

```c
/**
 * @file    imu_rtos.h
 * @brief   Buddy 4 IMU / Terrain Task on uT-Kernel 3.0 and the thread-safe
 *          public API used by the Mission Controller, Buddy 1 and Buddy 2.
 *
 * RTOS objects (all statically allocated, TA_USERBUF):
 *   - IMU / Terrain Task        IMU_RTOS_DEFAULT_PRIORITY (Medium-High)
 *   - cyclic handler            wakes the task every sample_period_ms
 *                               (drift-free period, overrun detection)
 *   - mutex (TA_INHERIT)        protects the shared status snapshot and the
 *                               command / hint mailboxes
 *   - message buffer            terrain_event_q (created here unless the
 *                               integration team passes an existing ID)
 *   - event flag (optional)     direct COLLISION / IMU_FAULT notification
 *                               to the Mission Controller (tk_set_flg)
 *
 * The IMU task never holds the mutex while talking to the sensor, never
 * blocks on a queue (TMO_POL), never performs network/MQTT operations and
 * never calls Buddy 2's motor API.
 *
 * INTEGRATION ITEMS TO CONFIRM WITH THE TEAM (marked "CONFIRM" below):
 *   - final numeric task priorities of all buddies
 *   - whether the Mission Controller creates terrain_event_q itself
 *   - event-flag ID and bit patterns used for direct notifications
 *   - kernel object limits in config.h (CNF_MAX_MBFID etc.)
 */

#ifndef IMU_RTOS_H
#define IMU_RTOS_H

#include <stdbool.h>
#include <stdint.h>

#include <tk/tkernel.h>

#include "imu/imu_events.h"
#include "imu/imu_hump.h"
#include "imu/imu_terrain.h"
#include "imu/imu_types.h"

/* CONFIRM: uT-Kernel priority 1 = highest. Proposed team mapping:
 * Motion Control 4 (Very High), Line/Barcode 6, Obstacle Monitor 6,
 * Mission Controller 7 (High), IMU 9 / Obstacle Handling 9 (Medium-High),
 * Wi-Fi/Telemetry 14 (Low). */
#ifndef IMU_RTOS_DEFAULT_PRIORITY
#define IMU_RTOS_DEFAULT_PRIORITY      (9)
#endif

/** Task stack size in bytes (static buffer; see docs Part 3 budget). */
#ifndef IMU_RTOS_STACK_BYTES
#define IMU_RTOS_STACK_BYTES           (2048U)
#endif

/** Depth of terrain_event_q when created by Buddy 4. */
#ifndef IMU_RTOS_EVENT_Q_DEPTH
#define IMU_RTOS_EVENT_Q_DEPTH         (16U)
#endif

/** Bounded mutex wait for API callers (ms; rounded up to kernel ticks). */
#ifndef IMU_RTOS_LOCK_TMO_MS
#define IMU_RTOS_LOCK_TMO_MS           (10)
#endif

/** Task watchdog: a missing cyclic wake-up is detected after N periods. */
#define IMU_RTOS_WAKE_TIMEOUT_FACTOR   (3)

/** Start-up configuration. */
typedef struct
{
    PRI                  task_priority;
    imu_params_t const * p_params;          /**< NULL = defaults            */
    ID                   terrain_event_q;   /**< 0 = create (CONFIRM)       */
    ID                   mc_flag_id;        /**< 0 = no direct notification */
    UINT                 collision_flag_ptn;/**< set on critical IMPACT     */
    UINT                 fault_flag_ptn;    /**< set on IMU_FAULT / tilt    */
    /** Optional hook called from the IMU task every telemetry_period_ms
     *  with a fresh telemetry record. MUST NOT BLOCK (e.g. TMO_POL send to
     *  Buddy 1's telemetry_q). NULL = disabled. */
    void (*p_telemetry_hook)(imu_telemetry_t const * p_tele);
    uint32_t             telemetry_period_ms;
} imu_rtos_config_t;

/** Run-time counters of the RTOS wrapper. */
typedef struct
{
    uint32_t cycles;
    uint32_t overruns;          /**< queued wake-ups found after a cycle */
    uint32_t missed_wakeups;    /**< tk_slp_tsk timed out                */
    uint32_t lock_timeouts;     /**< publication skipped (mutex busy)    */
    uint32_t max_cycle_us;      /**< worst observed cycle execution time */
} imu_rtos_stats_t;

/** Fill @p p_cfg with defaults (own queue, no flag, no hook). */
void imu_rtos_default_config(imu_rtos_config_t * p_cfg);

/**
 * @brief  Create the RTOS objects and start the IMU / Terrain Task.
 *         Call once from usermain() / the system initialisation task.
 * @return IMU_OK, IMU_ERR_PARAM, IMU_ERR_STATE (already running),
 *         IMU_ERR_RTOS (kernel object creation failed).
 */
imu_err_t imu_rtos_start(imu_rtos_config_t const * p_cfg);

/** Request a clean stop: the cyclic handler stops, the sensor is put to
 *  sleep, and the task exits to DORMANT (restartable with imu_rtos_start). */
imu_err_t imu_rtos_stop(void);

bool      imu_rtos_is_running(void);

/** @return message-buffer ID of terrain_event_q (0 if not started). */
ID        imu_rtos_terrain_event_q(void);

imu_err_t imu_rtos_get_stats(imu_rtos_stats_t * p_out);

/* ------------------------------------------------------------------------
 *  Thread-safe query API (any task). All functions copy from the snapshot
 *  published at the end of each IMU cycle; none touches the sensor.
 *  Return IMU_ERR_BUSY if the snapshot could not be locked in time,
 *  IMU_ERR_NOT_READY if the task has not been started.
 * ---------------------------------------------------------------------- */
imu_err_t imu_get_status(imu_status_t * p_out);
imu_err_t imu_get_telemetry(imu_telemetry_t * p_out);
imu_err_t imu_get_motion(imu_motion_class_t * p_motion,
                         imu_confidence_t * p_conf);
imu_err_t imu_get_tilt(float * p_pitch_deg, float * p_roll_deg,
                       float * p_tilt_deg, bool * p_b_reliable);
imu_err_t imu_get_heading(float * p_heading_deg,
                          imu_heading_quality_t * p_quality,
                          float * p_turn_rate_dps, bool * p_b_rate_valid,
                          imu_turn_dir_t * p_turn_dir);
imu_err_t imu_get_hump_state(imu_hump_phase_t * p_phase);
/** @return IMU_ERR_NO_DATA if no valid hump has been completed yet. */
imu_err_t imu_get_last_hump(imu_hump_meas_t * p_out);
/** @return IMU_ERR_NO_DATA if no valid hump has been completed yet. */
imu_err_t imu_get_max_hump(imu_hump_meas_t * p_out);
imu_err_t imu_get_health(imu_state_t * p_state, imu_fault_t * p_fault,
                         bool * p_b_sensor_ok);

/* ------------------------------------------------------------------------
 *  Thread-safe command API (executed at the start of the next IMU cycle).
 * ---------------------------------------------------------------------- */
imu_err_t imu_mission_start(void);      /**< START_RUN: reset hump stats   */
imu_err_t imu_request_reset(void);      /**< subsystem reset -> INIT       */
imu_err_t imu_clear_fault(void);        /**< MC acknowledges a collision   */
imu_err_t imu_recalibrate(void);        /**< vehicle must be stationary    */
imu_err_t imu_mag_cal_begin(void);      /**< start spin-in-place mag cal   */
imu_err_t imu_mag_cal_end(void);        /**< finish mag cal                */

/** Buddy 2 / Mission Controller: optional encoder speed hint. */
imu_err_t imu_set_motion_hint(imu_motion_hint_t const * p_hint);

#endif /* IMU_RTOS_H */
```

## `src/imu/imu_rtos.c`

```c
/**
 * @file    imu_rtos.c
 * @brief   Buddy 4 IMU / Terrain Task (uT-Kernel 3.0) - see imu_rtos.h.
 *
 * uT-Kernel 3.0 service calls used (TEF020-S004 specification; verified
 * against TRON Forum mtk3_bsp branch pico_rp2040, include/tk/syscall.h):
 *   tk_cre_tsk, tk_sta_tsk, tk_slp_tsk, tk_wup_tsk, tk_can_wup, tk_ext_tsk,
 *   tk_cre_cyc, tk_sta_cyc, tk_stp_cyc, tk_cre_mtx, tk_loc_mtx, tk_unl_mtx,
 *   tk_cre_mbf, tk_snd_mbf, tk_set_flg.
 */

#include <string.h>

#include <tk/tkernel.h>

#include "imu/imu_hal.h"
#include "imu/imu_rtos.h"

/* Message-buffer storage: each message = HEADER (INT) + rounded payload. */
#define RTOS_MSG_ROUND(sz)   ((((uint32_t)(sz)) + (sizeof(INT) - 1U)) \
                              & ~(uint32_t)(sizeof(INT) - 1U))
#define RTOS_MBF_BYTES       (IMU_RTOS_EVENT_Q_DEPTH \
                              * (sizeof(INT) \
                                 + RTOS_MSG_ROUND(sizeof(imu_event_t))))

/* ---- Statically allocated objects (no heap) ----------------------------- */
static uint64_t          g_task_stack[IMU_RTOS_STACK_BYTES / 8U];
static uint32_t          g_mbf_buf[(RTOS_MBF_BYTES + 3U) / 4U];
static imu_terrain_t     g_sys;              /* owned by the IMU task      */
static imu_params_t      g_params;
static imu_rtos_config_t g_cfg;

static imu_status_t      g_shared_status;    /* protected by g_mtxid       */
static uint32_t          g_shared_cmds;      /* protected by g_mtxid       */
static imu_motion_hint_t g_shared_hint;      /* protected by g_mtxid       */
static bool              g_b_shared_hint_new;/* protected by g_mtxid       */
static imu_rtos_stats_t  g_stats;            /* protected by g_mtxid       */

static ID                g_tskid    = 0;
static ID                g_cycid    = 0;
static ID                g_mtxid    = 0;
static ID                g_mbfid    = 0;
static volatile bool     g_b_running = false;
static volatile bool     g_b_stop    = false;

/* ------------------------------------------------------------------------ */
/*  Event sink: terrain_event_q + direct notification                       */
/* ------------------------------------------------------------------------ */

static imu_sink_result_t rtos_send(void * p_ctx, imu_event_t const * p_evt)
{
    ER er;

    (void)p_ctx;

    if (g_mbfid <= 0)
    {
        return (IMU_SINK_ERROR);
    }

    er = tk_snd_mbf(g_mbfid, (CONST void *)p_evt, (INT)sizeof(*p_evt),
                    TMO_POL);
    if (E_OK == er)
    {
        return (IMU_SINK_OK);
    }

    return ((E_TMOUT == er) ? IMU_SINK_FULL : IMU_SINK_ERROR);
}

static void rtos_notify(void * p_ctx, imu_event_t const * p_evt)
{
    UINT ptn = 0U;

    (void)p_ctx;

    if (g_cfg.mc_flag_id <= 0)
    {
        return;
    }

    if (IMU_EVT_IMPACT == p_evt->type)
    {
        ptn = g_cfg.collision_flag_ptn;
    }
    else
    {
        ptn = g_cfg.fault_flag_ptn;
    }

    if (0U != ptn)
    {
        (void)tk_set_flg(g_cfg.mc_flag_id, ptn);   /* never blocks */
    }
}

/* ------------------------------------------------------------------------ */
/*  Cyclic handler (task-independent part): only wakes the task             */
/* ------------------------------------------------------------------------ */

static void imu_cyc_handler(void * p_exinf)
{
    (void)p_exinf;
    (void)tk_wup_tsk(g_tskid);
}

/* ------------------------------------------------------------------------ */
/*  IMU / Terrain Task                                                      */
/* ------------------------------------------------------------------------ */

static void imu_task(INT stacd, void * p_exinf)
{
    TMO      wake_tmo;
    uint32_t cmds;
    uint32_t t0_us;
    uint32_t elapsed_us;
    uint32_t last_tele_ms = 0U;
    uint32_t now_ms;
    INT      queued;
    ER       er;
    bool     b_hint_new;
    imu_motion_hint_t hint;
    imu_telemetry_t   tele;

    (void)stacd;
    (void)p_exinf;

    wake_tmo = (TMO)(g_params.sample_period_ms
                     * (uint32_t)IMU_RTOS_WAKE_TIMEOUT_FACTOR);

    for (;;)
    {
        er = tk_slp_tsk(wake_tmo);

        if (g_b_stop)
        {
            break;
        }

        /* 1. Collect commands and hint (short critical section). */
        cmds       = 0U;
        b_hint_new = false;
        if (E_OK == tk_loc_mtx(g_mtxid, (TMO)IMU_RTOS_LOCK_TMO_MS))
        {
            if (E_TMOUT == er)
            {
                g_stats.missed_wakeups++;
            }
            cmds          = g_shared_cmds;
            g_shared_cmds = 0U;
            if (g_b_shared_hint_new)
            {
                hint                = g_shared_hint;
                b_hint_new          = true;
                g_b_shared_hint_new = false;
            }
            (void)tk_unl_mtx(g_mtxid);
        }

        if (0U != cmds)
        {
            (void)imu_terrain_command(&g_sys, cmds);
        }
        if (b_hint_new)
        {
            (void)imu_terrain_set_hint(&g_sys, &hint);
        }

        /* 2. One sampling period of work (sensor I/O without the mutex). */
        now_ms = imu_hal_time_ms();
        t0_us  = imu_hal_time_us();
        imu_terrain_step(&g_sys, now_ms);
        elapsed_us = imu_hal_time_us() - t0_us;

        /* 3. Publish the snapshot for other tasks. */
        if (E_OK == tk_loc_mtx(g_mtxid, (TMO)IMU_RTOS_LOCK_TMO_MS))
        {
            g_shared_status = g_sys.status;
            g_stats.cycles++;
            if (elapsed_us > g_stats.max_cycle_us)
            {
                g_stats.max_cycle_us = elapsed_us;
            }
            (void)tk_unl_mtx(g_mtxid);
        }
        else
        {
            g_stats.lock_timeouts++;   /* single writer: benign race */
        }

        /* 4. Optional telemetry hook (must not block). */
        if ((NULL != g_cfg.p_telemetry_hook)
            && (g_cfg.telemetry_period_ms > 0U)
            && ((now_ms - last_tele_ms) >= g_cfg.telemetry_period_ms))
        {
            last_tele_ms = now_ms;
            imu_terrain_make_telemetry(&g_sys.status, &tele);
            g_cfg.p_telemetry_hook(&tele);
        }

        /* 5. Overrun detection: wake-ups queued while we were working mean
         *    the cycle took longer than the period. Discard them so the
         *    task re-aligns with the cyclic handler instead of bursting. */
        queued = tk_can_wup(TSK_SELF);
        if (queued > 0)
        {
            g_stats.overruns += (uint32_t)queued;
        }
    }

    /* Clean shutdown: sensor to sleep, bus released. */
    (void)imu_terrain_command(&g_sys, IMU_CMD_STOP);
    imu_terrain_step(&g_sys, imu_hal_time_ms());
    if (E_OK == tk_loc_mtx(g_mtxid, (TMO)IMU_RTOS_LOCK_TMO_MS))
    {
        g_shared_status = g_sys.status;      /* publish STOPPED */
        (void)tk_unl_mtx(g_mtxid);
    }
    g_b_running = false;
    tk_ext_tsk();
}

/* ------------------------------------------------------------------------ */
/*  Start / stop                                                            */
/* ------------------------------------------------------------------------ */

void imu_rtos_default_config(imu_rtos_config_t * p_cfg)
{
    if (NULL == p_cfg)
    {
        return;
    }

    (void)memset(p_cfg, 0, sizeof(*p_cfg));
    p_cfg->task_priority       = IMU_RTOS_DEFAULT_PRIORITY;
    p_cfg->p_params            = NULL;
    p_cfg->terrain_event_q     = 0;
    p_cfg->mc_flag_id          = 0;
    p_cfg->collision_flag_ptn  = 0U;
    p_cfg->fault_flag_ptn      = 0U;
    p_cfg->p_telemetry_hook    = NULL;
    p_cfg->telemetry_period_ms = 0U;
}

static imu_err_t rtos_create_objects(void)
{
    T_CMTX cmtx;
    T_CMBF cmbf;
    T_CTSK ctsk;
    T_CCYC ccyc;

    if (0 == g_mtxid)
    {
        (void)memset(&cmtx, 0, sizeof(cmtx));
        cmtx.mtxatr = TA_TPRI | TA_INHERIT;   /* avoid priority inversion */
        g_mtxid = tk_cre_mtx(&cmtx);
        if (g_mtxid <= 0)
        {
            g_mtxid = 0;
            return (IMU_ERR_RTOS);
        }
    }

    if (g_cfg.terrain_event_q > 0)
    {
        g_mbfid = g_cfg.terrain_event_q;      /* owned by the integrator */
    }
    else if (0 == g_mbfid)
    {
        (void)memset(&cmbf, 0, sizeof(cmbf));
        cmbf.mbfatr = TA_TFIFO | TA_USERBUF;
        cmbf.bufsz  = (SZ)(sizeof(g_mbf_buf));
        cmbf.maxmsz = (INT)sizeof(imu_event_t);
        cmbf.bufptr = (void *)g_mbf_buf;
        g_mbfid = tk_cre_mbf(&cmbf);
        if (g_mbfid <= 0)
        {
            g_mbfid = 0;
            return (IMU_ERR_RTOS);
        }
    }
    else
    {
        /* already created by a previous start */
    }

    if (0 == g_tskid)
    {
        (void)memset(&ctsk, 0, sizeof(ctsk));
        ctsk.exinf   = NULL;
        ctsk.tskatr  = TA_HLNG | TA_RNG0 | TA_USERBUF;
        ctsk.task    = (FP)imu_task;
        ctsk.itskpri = g_cfg.task_priority;
        ctsk.stksz   = (SZ)sizeof(g_task_stack);
        ctsk.bufptr  = (void *)g_task_stack;
        g_tskid = tk_cre_tsk(&ctsk);
        if (g_tskid <= 0)
        {
            g_tskid = 0;
            return (IMU_ERR_RTOS);
        }
    }

    if (0 == g_cycid)
    {
        (void)memset(&ccyc, 0, sizeof(ccyc));
        ccyc.exinf  = NULL;
        ccyc.cycatr = TA_HLNG;                 /* started explicitly */
        ccyc.cychdr = (FP)imu_cyc_handler;
        ccyc.cyctim = (RELTIM)g_params.sample_period_ms;
        ccyc.cycphs = (RELTIM)g_params.sample_period_ms;
        g_cycid = tk_cre_cyc(&ccyc);
        if (g_cycid <= 0)
        {
            g_cycid = 0;
            return (IMU_ERR_RTOS);
        }
    }

    return (IMU_OK);
}

imu_err_t imu_rtos_start(imu_rtos_config_t const * p_cfg)
{
    imu_event_sink_t sink;
    imu_err_t        err;

    if (NULL == p_cfg)
    {
        return (IMU_ERR_PARAM);
    }
    if (g_b_running)
    {
        return (IMU_ERR_STATE);
    }

    g_cfg = *p_cfg;
    if (NULL != p_cfg->p_params)
    {
        g_params = *p_cfg->p_params;
    }
    else
    {
        imu_params_default(&g_params);
    }

    if (IMU_OK != imu_params_validate(&g_params))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_create_objects();
    if (IMU_OK != err)
    {
        return (err);
    }

    sink.p_send   = rtos_send;
    sink.p_notify = rtos_notify;
    sink.p_ctx    = NULL;

    /* No I/O here: the sensor is initialised inside the task. */
    err = imu_terrain_init(&g_sys, &g_params, &sink);
    if (IMU_OK != err)
    {
        return (err);
    }

    g_shared_status     = g_sys.status;
    g_shared_cmds       = 0U;
    g_b_shared_hint_new = false;
    (void)memset(&g_stats, 0, sizeof(g_stats));
    g_b_stop    = false;
    g_b_running = true;

    if (E_OK != tk_sta_tsk(g_tskid, 0))
    {
        g_b_running = false;
        return (IMU_ERR_RTOS);
    }
    if (E_OK != tk_sta_cyc(g_cycid))
    {
        g_b_stop = true;
        (void)tk_wup_tsk(g_tskid);
        return (IMU_ERR_RTOS);
    }

    return (IMU_OK);
}

imu_err_t imu_rtos_stop(void)
{
    if (!g_b_running)
    {
        return (IMU_ERR_STATE);
    }

    (void)tk_stp_cyc(g_cycid);
    g_b_stop = true;
    (void)tk_wup_tsk(g_tskid);   /* task exits on its next wake-up */

    return (IMU_OK);
}

bool imu_rtos_is_running(void)
{
    return (g_b_running);
}

ID imu_rtos_terrain_event_q(void)
{
    return (g_mbfid);
}

/* ------------------------------------------------------------------------ */
/*  Thread-safe accessors                                                   */
/* ------------------------------------------------------------------------ */

static imu_err_t rtos_lock(void)
{
    if (0 == g_mtxid)
    {
        return (IMU_ERR_NOT_READY);
    }

    return ((E_OK == tk_loc_mtx(g_mtxid, (TMO)IMU_RTOS_LOCK_TMO_MS))
            ? IMU_OK : IMU_ERR_BUSY);
}

static void rtos_unlock(void)
{
    (void)tk_unl_mtx(g_mtxid);
}

/** Copy the snapshot under the lock (single copy keeps lock time short). */
static imu_err_t rtos_snapshot(imu_status_t * p_out)
{
    imu_err_t err = rtos_lock();

    if (IMU_OK == err)
    {
        *p_out = g_shared_status;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_rtos_get_stats(imu_rtos_stats_t * p_out)
{
    imu_err_t err;

    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_out = g_stats;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_status(imu_status_t * p_out)
{
    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    return (rtos_snapshot(p_out));
}

imu_err_t imu_get_telemetry(imu_telemetry_t * p_out)
{
    imu_status_t st;
    imu_err_t    err;

    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_snapshot(&st);
    if (IMU_OK == err)
    {
        imu_terrain_make_telemetry(&st, p_out);
    }

    return (err);
}

imu_err_t imu_get_motion(imu_motion_class_t * p_motion,
                         imu_confidence_t * p_conf)
{
    imu_err_t err;

    if ((NULL == p_motion) || (NULL == p_conf))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_motion = g_shared_status.motion;
        *p_conf   = g_shared_status.motion_conf;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_tilt(float * p_pitch_deg, float * p_roll_deg,
                       float * p_tilt_deg, bool * p_b_reliable)
{
    imu_err_t err;

    if ((NULL == p_pitch_deg) || (NULL == p_roll_deg) || (NULL == p_tilt_deg)
        || (NULL == p_b_reliable))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_pitch_deg  = g_shared_status.pitch_deg;
        *p_roll_deg   = g_shared_status.roll_deg;
        *p_tilt_deg   = g_shared_status.tilt_deg;
        *p_b_reliable = g_shared_status.b_attitude_reliable;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_heading(float * p_heading_deg,
                          imu_heading_quality_t * p_quality,
                          float * p_turn_rate_dps, bool * p_b_rate_valid,
                          imu_turn_dir_t * p_turn_dir)
{
    imu_err_t err;

    if ((NULL == p_heading_deg) || (NULL == p_quality)
        || (NULL == p_turn_rate_dps) || (NULL == p_b_rate_valid)
        || (NULL == p_turn_dir))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_heading_deg   = g_shared_status.heading_deg;
        *p_quality       = g_shared_status.heading_quality;
        *p_turn_rate_dps = g_shared_status.turn_rate_dps;
        *p_b_rate_valid  = g_shared_status.b_turn_rate_valid;
        *p_turn_dir      = g_shared_status.turn_dir;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_hump_state(imu_hump_phase_t * p_phase)
{
    imu_err_t err;

    if (NULL == p_phase)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_phase = g_shared_status.hump_phase;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_last_hump(imu_hump_meas_t * p_out)
{
    imu_err_t err;

    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        if (g_shared_status.hump.b_has_last)
        {
            *p_out = g_shared_status.hump.last;
        }
        else
        {
            err = IMU_ERR_NO_DATA;
        }
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_max_hump(imu_hump_meas_t * p_out)
{
    imu_err_t err;

    if (NULL == p_out)
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        if (g_shared_status.hump.b_has_max)
        {
            *p_out = g_shared_status.hump.max;
        }
        else
        {
            err = IMU_ERR_NO_DATA;
        }
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_get_health(imu_state_t * p_state, imu_fault_t * p_fault,
                         bool * p_b_sensor_ok)
{
    imu_err_t err;

    if ((NULL == p_state) || (NULL == p_fault) || (NULL == p_b_sensor_ok))
    {
        return (IMU_ERR_PARAM);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        *p_state       = g_shared_status.state;
        *p_fault       = g_shared_status.fault;
        *p_b_sensor_ok = g_shared_status.b_sensor_ok;
        rtos_unlock();
    }

    return (err);
}

/* ------------------------------------------------------------------------ */
/*  Commands                                                                */
/* ------------------------------------------------------------------------ */

static imu_err_t rtos_post(uint32_t cmd)
{
    imu_err_t err;

    if (!g_b_running)
    {
        return (IMU_ERR_NOT_READY);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        g_shared_cmds |= cmd;
        rtos_unlock();
    }

    return (err);
}

imu_err_t imu_mission_start(void)
{
    return (rtos_post(IMU_CMD_MISSION_START));
}

imu_err_t imu_request_reset(void)
{
    return (rtos_post(IMU_CMD_RESET));
}

imu_err_t imu_clear_fault(void)
{
    return (rtos_post(IMU_CMD_CLEAR_FAULT));
}

imu_err_t imu_recalibrate(void)
{
    return (rtos_post(IMU_CMD_RECALIBRATE));
}

imu_err_t imu_mag_cal_begin(void)
{
    return (rtos_post(IMU_CMD_MAG_CAL_BEGIN));
}

imu_err_t imu_mag_cal_end(void)
{
    return (rtos_post(IMU_CMD_MAG_CAL_END));
}

imu_err_t imu_set_motion_hint(imu_motion_hint_t const * p_hint)
{
    imu_err_t err;

    if (NULL == p_hint)
    {
        return (IMU_ERR_PARAM);
    }
    if (!g_b_running)
    {
        return (IMU_ERR_NOT_READY);
    }

    err = rtos_lock();
    if (IMU_OK == err)
    {
        g_shared_hint       = *p_hint;
        g_b_shared_hint_new = true;
        rtos_unlock();
    }

    return (err);
}
```

## `tools/buddy4_bringup/bringup_main.c`

```c
/**
 * @file    bringup_main.c
 * @brief   Buddy 4 stand-alone HARDWARE BRING-UP / CALIBRATION LOGGER.
 *
 * Separate executable (target buddy4_imu_bringup) with its own main(): it
 * does NOT replace or conflict with the team firmware's src/main.c and does
 * NOT use uT-Kernel. It runs the exact same production modules
 * (driver + calibration + processing + hump + terrain state machine) in a
 * bare-metal 50 Hz loop so that the IMU can be wired, verified, calibrated
 * and its thresholds tuned before RTOS integration.
 *
 * Output (USB CDC serial, 115200 8N1 is irrelevant for USB):
 *   "EVT,..." lines for every terrain event
 *   "CSV,..." lines (when logging is on) with one row per cycle:
 *     t_ms,state,motion,pitch,roll,tilt,heading,hq,rate,fwd_g,vert_g,
 *     vib_g,|B|,hump_phase,humps,max_peak_deg,max_h_mm
 *   "RAW,..." lines (raw mode) with sensor-frame accel (g) and mag (gauss)
 *
 * Single-key commands:
 *   l  toggle CSV log            w  toggle RAW sensor log
 *   m  mission start (reset)     c  clear collision fault
 *   b  begin mag calibration     e  end mag calibration (prints result)
 *   k  recalibrate (stationary)  r  subsystem reset
 *   s  print status              h  help
 *
 * Motors are never driven by this program.
 */

#include <stdio.h>

#include "pico/stdlib.h"

#include "imu/imu_events.h"
#include "imu/imu_hal.h"
#include "imu/imu_terrain.h"

#define BRINGUP_PERIOD_MS   (IMU_CFG_SAMPLE_PERIOD_MS)

static imu_terrain_t g_sys;
static bool          g_b_csv = false;
static bool          g_b_raw = false;

static imu_sink_result_t bringup_send(void * p_ctx, imu_event_t const * p_e)
{
    (void)p_ctx;
    printf("EVT,%lu,%u,%s,sev=%u,state=%s,valid=%u",
           (unsigned long)p_e->timestamp_ms, (unsigned)p_e->seq,
           imu_event_type_name((imu_event_type_t)p_e->type),
           (unsigned)p_e->severity,
           imu_state_name((imu_state_t)p_e->state), (unsigned)p_e->b_valid);

    switch (p_e->type)
    {
        case IMU_EVT_HUMP_DETECTED:
        case IMU_EVT_HUMP_PEAK:
        case IMU_EVT_HUMP_COMPLETED:
        case IMU_EVT_HUMP_REJECTED:
            printf(",idx=%lu,peak_cdeg=%d,min_cdeg=%d,h_mm=%d,conf=%u,"
                   "validity=%u,dur=%u,new_max=%u,max_cdeg=%d",
                   (unsigned long)p_e->data.hump.index,
                   (int)p_e->data.hump.peak_pitch_cdeg,
                   (int)p_e->data.hump.min_pitch_cdeg,
                   (int)p_e->data.hump.height_est_mm,
                   (unsigned)p_e->data.hump.height_conf,
                   (unsigned)p_e->data.hump.validity,
                   (unsigned)p_e->data.hump.duration_ms,
                   (unsigned)p_e->data.hump.b_new_max,
                   (int)p_e->data.hump.max_peak_pitch_cdeg);
            break;
        case IMU_EVT_TILT:
            printf(",phase=%u,pitch=%d,roll=%d,tilt=%d",
                   (unsigned)p_e->data.tilt.phase,
                   (int)p_e->data.tilt.pitch_cdeg,
                   (int)p_e->data.tilt.roll_cdeg,
                   (int)p_e->data.tilt.tilt_cdeg);
            break;
        case IMU_EVT_TURNING:
            printf(",dir=%u,phase=%u,rate_cdps=%d,hdg=%d,delta=%ld,dur=%lu",
                   (unsigned)p_e->data.turn.direction,
                   (unsigned)p_e->data.turn.phase,
                   (int)p_e->data.turn.rate_cdps,
                   (int)p_e->data.turn.heading_cdeg,
                   (long)p_e->data.turn.delta_cdeg,
                   (unsigned long)p_e->data.turn.duration_ms);
            break;
        case IMU_EVT_IMPACT:
            printf(",peak_mg=%d,horiz_mg=%d,vert_mg=%d,dur=%u,supp=%lu",
                   (int)p_e->data.impact.peak_mg,
                   (int)p_e->data.impact.horiz_mg,
                   (int)p_e->data.impact.vert_mg,
                   (unsigned)p_e->data.impact.duration_ms,
                   (unsigned long)p_e->data.impact.suppressed);
            break;
        case IMU_EVT_MOTION:
            printf(",motion=%s,conf=%u",
                   imu_motion_name((imu_motion_class_t)p_e->data.motion.motion),
                   (unsigned)p_e->data.motion.confidence);
            break;
        default:
            printf(",status=%u,fault=%s,err=-%u,attempt=%lu",
                   (unsigned)p_e->data.fault.status,
                   imu_fault_name((imu_fault_t)p_e->data.fault.fault),
                   (unsigned)p_e->data.fault.last_error,
                   (unsigned long)p_e->data.fault.attempt);
            break;
    }
    printf("\n");

    return (IMU_SINK_OK);
}

static void bringup_status(void)
{
    imu_status_t st;

    (void)imu_terrain_get_status(&g_sys, &st);
    printf("STATUS state=%s fault=%s err=%d cal=%u g_ref=%.4f noise=%.4f "
           "mount_tilt=%.2f\n", imu_state_name(st.state),
           imu_fault_name(st.fault), (int)st.last_error,
           (unsigned)st.b_calibrated, (double)st.cal_g_ref,
           (double)st.cal_noise_g, (double)st.cal_mount_tilt_deg);
    printf("  pitch=%.2f roll=%.2f tilt=%.2f hdg=%.1f (%s) rate=%.1f "
           "motion=%s\n", (double)st.pitch_deg, (double)st.roll_deg,
           (double)st.tilt_deg, (double)st.heading_deg,
           imu_heading_quality_name(st.heading_quality),
           (double)st.turn_rate_dps, imu_motion_name(st.motion));
    printf("  humps=%lu rejected=%lu max_peak=%.2f deg max_h=%.1f mm\n",
           (unsigned long)st.hump.valid_count,
           (unsigned long)st.hump.rejected_count,
           (double)st.hump.max.peak_pitch_deg,
           (double)st.hump.max.height_est_mm);
    printf("  mag_cal valid=%u hard_iron=(%.4f,%.4f,%.4f) "
           "soft=(%.4f,%.4f,%.4f) ref=%.4f\n", (unsigned)st.b_mag_cal_valid,
           (double)st.mag_cal.hard_iron_gauss.x,
           (double)st.mag_cal.hard_iron_gauss.y,
           (double)st.mag_cal.hard_iron_gauss.z,
           (double)st.mag_cal.soft_iron_scale.x,
           (double)st.mag_cal.soft_iron_scale.y,
           (double)st.mag_cal.soft_iron_scale.z,
           (double)st.mag_cal.ref_field_gauss);
    printf("  i2c transfers=%lu failed=%lu retries=%lu nacks=%lu "
           "timeouts=%lu fifo_ovr=%lu recov=%lu\n",
           (unsigned long)st.i2c.transfers,
           (unsigned long)st.i2c.failed_transfers,
           (unsigned long)st.i2c.retries, (unsigned long)st.i2c.nacks,
           (unsigned long)st.i2c.timeouts,
           (unsigned long)st.i2c.fifo_overruns,
           (unsigned long)st.i2c.recoveries);
}

static void bringup_help(void)
{
    printf("Buddy 4 bring-up: l=CSV w=RAW m=mission c=clear-fault "
           "b/e=mag-cal begin/end k=recal r=reset s=status h=help\n");
}

static void bringup_keys(void)
{
    int ch = getchar_timeout_us(0U);

    while (PICO_ERROR_TIMEOUT != ch)
    {
        switch (ch)
        {
            case 'l': g_b_csv = !g_b_csv;                                 break;
            case 'w': g_b_raw = !g_b_raw;                                 break;
            case 'm': (void)imu_terrain_command(&g_sys, IMU_CMD_MISSION_START); break;
            case 'c': (void)imu_terrain_command(&g_sys, IMU_CMD_CLEAR_FAULT);   break;
            case 'b': (void)imu_terrain_command(&g_sys, IMU_CMD_MAG_CAL_BEGIN); break;
            case 'e': (void)imu_terrain_command(&g_sys, IMU_CMD_MAG_CAL_END);   break;
            case 'k': (void)imu_terrain_command(&g_sys, IMU_CMD_RECALIBRATE);   break;
            case 'r': (void)imu_terrain_command(&g_sys, IMU_CMD_RESET);         break;
            case 's': bringup_status();                                   break;
            case 'h': bringup_help();                                     break;
            default:                                                      break;
        }
        ch = getchar_timeout_us(0U);
    }
}

static void bringup_log(void)
{
    imu_status_t const * p = &g_sys.status;

    if (g_b_csv)
    {
        printf("CSV,%lu,%u,%u,%.3f,%.3f,%.3f,%.2f,%u,%.2f,%.4f,%.4f,%.4f,"
               "%.4f,%u,%lu,%.3f,%.2f\n",
               (unsigned long)p->timestamp_ms, (unsigned)p->state,
               (unsigned)p->motion, (double)p->pitch_deg,
               (double)p->roll_deg, (double)p->tilt_deg,
               (double)p->heading_deg, (unsigned)p->heading_quality,
               (double)p->turn_rate_dps, (double)p->fwd_accel_g,
               (double)p->vert_accel_g, (double)p->vibration_g,
               (double)p->mag_field_gauss, (unsigned)p->hump_phase,
               (unsigned long)p->hump.valid_count,
               (double)p->hump.max.peak_pitch_deg,
               (double)p->hump.max.height_est_mm);
    }

    if (g_b_raw && g_sys.dev.b_ready)
    {
        imu_vec3_t a;
        imu_vec3_t m;
        imu_raw3_t mr;
        bool       b_ovf = false;

        /* Last accelerometer sample of the cycle + one magnetometer read. */
        lsm303_accel_to_g(&g_sys.dev, &g_sys.last_raw, &a);
        if (IMU_OK == lsm303_read_mag(&g_sys.dev, &mr, &b_ovf))
        {
            lsm303_mag_to_gauss(&g_sys.dev, &mr, &m);
            printf("RAW,%lu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%u\n",
                   (unsigned long)imu_hal_time_ms(), (double)a.x,
                   (double)a.y, (double)a.z, (double)m.x, (double)m.y,
                   (double)m.z, (unsigned)b_ovf);
        }
    }
}

int main(void)
{
    imu_event_sink_t sink;
    absolute_time_t  next;

    stdio_init_all();
    sleep_ms(2000);          /* bare-metal only: time to open the terminal */

    sink.p_send   = bringup_send;
    sink.p_notify = NULL;
    sink.p_ctx    = NULL;

    printf("\nBuddy 4 IMU bring-up (LSM303DLHC on I2C0 GP%u/GP%u)\n",
           (unsigned)IMU_I2C_SDA_GPIO, (unsigned)IMU_I2C_SCL_GPIO);
    printf("Keep the vehicle STILL and LEVEL for 3 s (calibration).\n");
    bringup_help();

    if (IMU_OK != imu_terrain_init(&g_sys, NULL, &sink))
    {
        printf("FATAL: invalid parameters in imu_config.h\n");
        for (;;)
        {
            tight_loop_contents();
        }
    }

    next = get_absolute_time();
    for (;;)
    {
        next = delayed_by_ms(next, BRINGUP_PERIOD_MS);
        bringup_keys();
        imu_terrain_step(&g_sys, imu_hal_time_ms());
        bringup_log();
        sleep_until(next);
    }

    return (0);
}
```

## `tools/buddy4_bringup/analyse_log.py`

```python
#!/usr/bin/env python3
"""
Buddy 4 - bring-up log analyser (standard library only).

Reads a terminal capture from the buddy4_imu_bringup firmware (CSV / EVT /
RAW lines) and prints the statistics needed to choose thresholds and to
fill in the terrain analysis report:

  * stationary noise per axis (RAW, first N seconds or --still t0:t1)
  * pitch / roll / vibration distribution while driving (CSV)
  * hump peaks per pass: mean, standard deviation, CV, range (EVT)
  * rejected hump candidates and their reasons (false-positive check)
  * impacts (peak, horizontal ratio) and turning events
  * suggested starting thresholds = k * measured noise (to be validated)

Usage:
  python3 analyse_log.py capture.txt [--still 0:3000] [--true-height 20]
"""

import argparse
import math
import statistics as stats
import sys

VALIDITY = {0: "VALID", 1: "PROVISIONAL", 2: "REJ_LOW_PEAK",
            3: "REJ_NO_DESCENT", 4: "REJ_TOO_SHORT", 5: "REJ_TOO_LONG",
            6: "REJ_SLOPE", 7: "REJ_ABORTED"}


def parse_kv(fields):
    out = {}
    for f in fields:
        if "=" in f:
            k, v = f.split("=", 1)
            out[k] = v
    return out


def describe(name, xs, unit=""):
    if len(xs) < 2:
        print(f"  {name:28s} n={len(xs)} (not enough data)")
        return
    print(f"  {name:28s} n={len(xs):5d} mean={stats.mean(xs):9.4f}{unit} "
          f"std={stats.stdev(xs):8.4f}{unit} min={min(xs):9.4f} "
          f"max={max(xs):9.4f}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log")
    ap.add_argument("--still", default="0:3000",
                    help="time window (ms) known to be stationary for noise")
    ap.add_argument("--true-height", type=float, default=None,
                    help="known hump height in mm (accuracy check)")
    args = ap.parse_args()
    t0, t1 = (int(x) for x in args.still.split(":"))

    raw, csv, humps, rejects, impacts, turns = [], [], [], [], [], []
    with open(args.log, errors="replace") as fh:
        for line in fh:
            line = line.strip()
            if line.startswith("RAW,"):
                p = line.split(",")
                raw.append([float(x) for x in p[1:8]])
            elif line.startswith("CSV,"):
                p = line.split(",")
                csv.append([float(x) for x in p[1:]])
            elif line.startswith("EVT,"):
                p = line.split(",")
                kind = p[3]
                kv = parse_kv(p[4:])
                if kind == "HUMP_COMPLETED":
                    humps.append(kv)
                elif kind == "HUMP_REJECTED":
                    rejects.append(kv)
                elif kind.startswith("IMPACT"):
                    impacts.append(kv)
                elif kind == "TURNING_EVENT":
                    turns.append(kv)

    print(f"Buddy 4 log analysis: {args.log}")
    print(f"  RAW rows={len(raw)} CSV rows={len(csv)} humps={len(humps)} "
          f"rejected={len(rejects)} impacts={len(impacts)} "
          f"turn events={len(turns)}")

    still = [r for r in raw if t0 <= r[0] <= t1]
    if len(still) > 10:
        print("\n1. Stationary sensor noise (RAW, sensor frame)")
        names = ["ax [g]", "ay [g]", "az [g]", "mx [G]", "my [G]", "mz [G]"]
        sig = []
        for i, n in enumerate(names):
            col = [r[i + 1] for r in still]
            describe(n, col)
            sig.append(stats.stdev(col))
        g = [math.sqrt(r[1] ** 2 + r[2] ** 2 + r[3] ** 2) for r in still]
        describe("|a| [g]", g)
        acc_sig = max(sig[:3])
        print("\n   Suggested starting points (validate on the track!):")
        print(f"   cal_max_noise_g      >= {max(4 * acc_sig, 0.005):.4f}")
        print(f"   stationary_vib_g     ~  {max(3 * acc_sig, 0.005):.4f}")
        print(f"   pitch noise ~ {math.degrees(acc_sig):.2f} deg per sample "
              f"-> hump_ascent_enter_deg >= {max(5 * math.degrees(acc_sig), 1.5):.2f}")

    if csv:
        print("\n2. Driving statistics (CSV)")
        moving = [r for r in csv if r[11] > 0.02]
        describe("pitch [deg] (all)", [r[3] for r in csv])
        describe("roll [deg] (all)", [r[4] for r in csv])
        describe("vibration [g] (moving)", [r[11] for r in moving])
        describe("fwd accel [g] (moving)", [r[9] for r in moving])
        describe("|B| [gauss]", [r[12] for r in csv])
        if moving:
            p99 = sorted(abs(r[3]) for r in moving)[int(0.99 * len(moving)) - 1]
            print(f"   99th percentile |pitch| on flat driving = {p99:.2f} deg "
                  f"-> keep hump_ascent_enter_deg above this")

    if humps:
        print("\n3. Hump repeatability (HUMP_COMPLETED)")
        peaks = [int(h["peak_cdeg"]) / 100.0 for h in humps]
        hmm = [int(h["h_mm"]) for h in humps if int(h["h_mm"]) >= 0]
        for h in humps:
            print(f"   #{h.get('idx')}: peak {int(h['peak_cdeg'])/100:.2f} deg, "
                  f"trough {int(h['min_cdeg'])/100:.2f} deg, est {h['h_mm']} mm, "
                  f"dur {h['dur']} ms, new_max={h['new_max']}")
        describe("peak pitch [deg]", peaks)
        if len(peaks) > 1:
            cv = stats.stdev(peaks) / stats.mean(peaks) * 100.0
            print(f"   coefficient of variation = {cv:.1f} % "
                  f"(A-B4-2: consistent if << difference between humps)")
        if args.true_height and hmm:
            err = [x - args.true_height for x in hmm]
            describe("height error [mm]", err)
            gain = args.true_height / stats.mean(hmm) if stats.mean(hmm) else 1
            print(f"   single-hump gain suggestion: hump_height_gain *= {gain:.3f}"
                  " (fit gain+offset with >= 3 different heights)")

    if rejects:
        print("\n4. Rejected hump candidates (should be 0 on flat floor)")
        reasons = {}
        for r in rejects:
            v = VALIDITY.get(int(r.get("validity", -1)), "?")
            reasons[v] = reasons.get(v, 0) + 1
        for k, v in reasons.items():
            print(f"   {k:16s} {v}")

    if impacts:
        print("\n5. Impacts")
        for i in impacts:
            pk = int(i["peak_mg"])
            hz = int(i["horiz_mg"])
            print(f"   peak {pk} mg, horiz {hz} mg ({100*hz/max(pk,1):.0f} %), "
                  f"dur {i['dur']} ms, suppressed {i['supp']}")

    if turns:
        ends = [t for t in turns if t.get("phase") == "1"]
        print(f"\n6. Turns: {len(ends)} completed")
        for t in ends:
            print(f"   dir={'LEFT' if t['dir'] == '1' else 'RIGHT'} "
                  f"delta={int(t['delta'])/100:.1f} deg dur={t['dur']} ms")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

## `tests/imu/Makefile`

```makefile
# Host-side test build for Buddy 4 (no Pico SDK, no uT-Kernel, no hardware).
#
#   make            build + run all tests
#   make replay-demo   build the offline replay tool and run it on a
#                      synthetic RAW log
#   make asan       build + run with AddressSanitizer + UBSan
#   make coverage   build + run with gcov, print line coverage
#   make tidy       clang-tidy static analysis of the production sources
#   make clean

CC      ?= gcc
ROOT    := ../..
SRC     := $(ROOT)/src/imu
INC     := -I$(ROOT)/include -I. -Ifake_tk
CSTD    := -std=c11
WARN    := -Wall -Wextra -Wpedantic -Wshadow -Wdouble-promotion \
           -Wfloat-equal -Wmissing-prototypes -Werror
DEFS    := -DIMU_STATE_TRACE_LEN=64U

PROD    := $(SRC)/imu_params.c $(SRC)/imu_math.c $(SRC)/imu_lsm303dlhc.c \
           $(SRC)/imu_calibration.c $(SRC)/imu_processing.c \
           $(SRC)/imu_hump.c $(SRC)/imu_events.c $(SRC)/imu_terrain.c \
           $(SRC)/imu_rtos.c
TESTS   := sim_lsm303.c scenario.c test_util.c fake_tk/fake_tk.c \
           test_unit_driver.c test_unit_math_cal.c test_unit_processing.c \
           test_unit_hump.c test_unit_events.c test_blackbox.c \
           test_robustness.c test_rtos.c test_main.c

all: run

buddy4_tests: $(PROD) $(TESTS) $(wildcard $(ROOT)/include/imu/*.h) *.h
	$(CC) $(CSTD) -O2 -g $(WARN) $(DEFS) $(INC) $(PROD) $(TESTS) -lm -o $@

run: buddy4_tests
	./buddy4_tests

# Offline replay of RAW logs captured with the bring-up firmware.
REPLAY_SRC := sim_lsm303.c scenario.c test_util.c replay_main.c
buddy4_replay: $(PROD) $(REPLAY_SRC)
	$(CC) $(CSTD) -O2 -g $(WARN) $(DEFS) $(INC) \
	    $(filter-out $(SRC)/imu_rtos.c,$(PROD)) $(REPLAY_SRC) -lm -o $@

replay-demo: buddy4_replay
	./buddy4_replay --gen > synthetic_log.txt
	./buddy4_replay synthetic_log.txt | tail -25

asan: $(PROD) $(TESTS)
	$(CC) $(CSTD) -O1 -g -fsanitize=address,undefined \
	    -fno-sanitize-recover=all -fno-omit-frame-pointer $(WARN) $(DEFS) \
	    $(INC) $(PROD) $(TESTS) -lm -o buddy4_tests_asan
	./buddy4_tests_asan

coverage: $(PROD) $(TESTS)
	rm -rf cov && mkdir -p cov
	$(CC) $(CSTD) -O0 -g --coverage $(DEFS) $(INC) $(PROD) $(TESTS) -lm \
	    -o cov/buddy4_tests_cov
	cd cov && ./buddy4_tests_cov > /dev/null
	cd cov && for f in params math lsm303dlhc calibration processing hump \
	    events terrain rtos; do \
	    printf "%-22s" "imu_$$f.c"; \
	    gcov -n -b buddy4_tests_cov-imu_$$f.gcno 2>/dev/null \
	    | grep -A3 "File '.*imu_$$f.c'" | grep -E "Lines|Taken" \
	    | tr '\n' ' '; echo; done

tidy:
	clang-tidy -checks='-*,clang-analyzer-*,bugprone-*,cert-*,-bugprone-easily-swappable-parameters,-cert-err33-c,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling' \
	    $(PROD) -- $(CSTD) $(DEFS) $(INC) 2>/dev/null \
	    | grep -E "warning:|error:" || echo "clang-tidy: no findings" 

clean:
	rm -rf buddy4_tests buddy4_tests_asan buddy4_replay synthetic_log.txt \
	    cov *.gcda *.gcno

.PHONY: all run asan coverage tidy clean replay-demo
```

## `tests/imu/test_util.h`

```c
/**
 * @file    test_util.h
 * @brief   HOST-ONLY minimal test framework + Buddy 4 test harness helpers.
 */

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "imu/imu_events.h"
#include "imu/imu_terrain.h"
#include "scenario.h"
#include "sim_lsm303.h"

/* ------------------------------ framework ---------------------------- */
extern uint32_t g_tests_run;
extern uint32_t g_tests_failed;
extern uint32_t g_asserts;
extern bool     g_cur_failed;
extern char const * g_cur_test;
extern bool     g_verbose;
extern char const * g_filter;

#define TEST_ASSERT(cond)                                                    \
    do {                                                                     \
        g_asserts++;                                                         \
        if (!(cond)) {                                                       \
            printf("    FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
            g_cur_failed = true;                                             \
        }                                                                    \
    } while (0)

#define TEST_ASSERT_MSG(cond, ...)                                           \
    do {                                                                     \
        g_asserts++;                                                         \
        if (!(cond)) {                                                       \
            printf("    FAIL %s:%d: %s -- ", __FILE__, __LINE__, #cond);     \
            printf(__VA_ARGS__);                                             \
            printf("\n");                                                    \
            g_cur_failed = true;                                             \
        }                                                                    \
    } while (0)

#define TEST_ASSERT_EQ(a, b)                                                 \
    TEST_ASSERT_MSG((long long)(a) == (long long)(b), "%lld != %lld",        \
                    (long long)(a), (long long)(b))

#define TEST_ASSERT_NEAR(a, b, tol)                                          \
    TEST_ASSERT_MSG(fabs((double)(a) - (double)(b)) <= (double)(tol),        \
                    "%.4f vs %.4f (tol %.4f)", (double)(a), (double)(b),     \
                    (double)(tol))

#define RUN_TEST(fn)  test_run(#fn, fn)

void test_run(char const * p_name, void (*p_fn)(void));
void test_section(char const * p_title);

/* --------------------------- event capture --------------------------- */
#define CAP_MAX 512U

typedef struct
{
    imu_event_t ev[CAP_MAX];
    uint32_t    count;        /* total accepted */
    uint32_t    capacity;     /* emulated queue capacity (<= CAP_MAX)   */
    uint32_t    level;        /* current queue level (drained by test)  */
    uint32_t    notified;
    imu_event_t last_notified;
    bool        b_error;      /* sink returns ERROR                      */
} capture_t;

extern capture_t g_cap;

void      cap_reset(uint32_t capacity);
imu_event_sink_t cap_sink(void);
uint32_t  cap_count_type(imu_event_type_t type);
imu_event_t const * cap_last_of(imu_event_type_t type);
void      cap_drain(void);

/* ------------------------------ harness ------------------------------ */
typedef struct
{
    imu_terrain_t sys;
    imu_params_t  params;
    scenario_t    scn;
    uint32_t      now_ms;
    bool          b_hint;      /* emulate Buddy 2 encoder hint     */
    bool          b_no_drain;  /* MC does not read its queue       */
} harness_t;

extern harness_t g_h;

/** Reset sim + capture, default params (callers may edit g_h.params
 *  before harness_start). Scenario defaults: stationary, level. */
void harness_setup(void);

/** Initialise imu_terrain with g_h.params and attach scenario stimulus. */
imu_err_t harness_start(void);

/** Run for @p ms of simulated time (steps at the sample period). */
void harness_run_ms(uint32_t ms);

/** Run until the subsystem reaches MONITORING (returns false on timeout). */
bool harness_until_monitoring(uint32_t max_ms);

/** Run until @p state (returns false on timeout). */
bool harness_until_state(imu_state_t state, uint32_t max_ms);

imu_status_t harness_status(void);

/** Did the trace ever contain a transition into @p state? */
bool harness_trace_has(imu_state_t state);

/* ------------------------------ suites -------------------------------- */
void suite_unit_driver(void);
void suite_unit_math_cal(void);
void suite_unit_processing(void);
void suite_unit_hump(void);
void suite_unit_events(void);
void suite_blackbox(void);
void suite_robustness(void);
void suite_rtos(void);

#endif /* TEST_UTIL_H */
```

## `tests/imu/test_util.c`

```c
/**
 * @file    test_util.c
 * @brief   HOST-ONLY test framework and harness (see test_util.h).
 */

#include <string.h>

#include "test_util.h"

uint32_t     g_tests_run    = 0U;
uint32_t     g_tests_failed = 0U;
uint32_t     g_asserts      = 0U;
bool         g_cur_failed   = false;
char const * g_cur_test     = "";
bool         g_verbose      = false;
char const * g_filter       = NULL;

capture_t g_cap;
harness_t g_h;

void test_run(char const * p_name, void (*p_fn)(void))
{
    if ((NULL != g_filter) && (NULL == strstr(p_name, g_filter)))
    {
        return;
    }
    g_cur_test   = p_name;
    g_cur_failed = false;
    g_tests_run++;
    p_fn();
    if (g_cur_failed)
    {
        g_tests_failed++;
        printf("  [FAIL] %s\n", p_name);
    }
    else
    {
        printf("  [ ok ] %s\n", p_name);
    }
}

void test_section(char const * p_title)
{
    if (NULL == g_filter)
    {
        printf("\n== %s ==\n", p_title);
    }
}

/* ----------------------------- capture -------------------------------- */

static imu_sink_result_t cap_send(void * p_ctx, imu_event_t const * p_evt)
{
    (void)p_ctx;

    if (g_cap.b_error)
    {
        return (IMU_SINK_ERROR);
    }
    if (g_cap.level >= g_cap.capacity)
    {
        return (IMU_SINK_FULL);
    }
    if (g_cap.count < CAP_MAX)
    {
        g_cap.ev[g_cap.count] = *p_evt;
    }
    g_cap.count++;
    g_cap.level++;
    if (g_verbose)
    {
        printf("      evt t=%u seq=%u %s sev=%u state=%s\n",
               (unsigned)p_evt->timestamp_ms, (unsigned)p_evt->seq,
               imu_event_type_name((imu_event_type_t)p_evt->type),
               (unsigned)p_evt->severity,
               imu_state_name((imu_state_t)p_evt->state));
    }
    return (IMU_SINK_OK);
}

static void cap_notify(void * p_ctx, imu_event_t const * p_evt)
{
    (void)p_ctx;
    g_cap.notified++;
    g_cap.last_notified = *p_evt;
}

void cap_reset(uint32_t capacity)
{
    (void)memset(&g_cap, 0, sizeof(g_cap));
    g_cap.capacity = (capacity > CAP_MAX) ? CAP_MAX : capacity;
}

imu_event_sink_t cap_sink(void)
{
    imu_event_sink_t s;

    s.p_send   = cap_send;
    s.p_notify = cap_notify;
    s.p_ctx    = NULL;
    return (s);
}

uint32_t cap_count_type(imu_event_type_t type)
{
    uint32_t i;
    uint32_t n = 0U;
    uint32_t lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;

    for (i = 0U; i < lim; i++)
    {
        if (g_cap.ev[i].type == (uint8_t)type)
        {
            n++;
        }
    }
    return (n);
}

imu_event_t const * cap_last_of(imu_event_type_t type)
{
    uint32_t lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    uint32_t i;

    for (i = lim; i > 0U; i--)
    {
        if (g_cap.ev[i - 1U].type == (uint8_t)type)
        {
            return (&g_cap.ev[i - 1U]);
        }
    }
    return (NULL);
}

void cap_drain(void)
{
    g_cap.level = 0U;
}

/* ----------------------------- harness -------------------------------- */

void harness_setup(void)
{
    sim_reset();
    cap_reset(CAP_MAX);
    (void)memset(&g_h, 0, sizeof(g_h));
    imu_params_default(&g_h.params);
    scn_init(&g_h.scn);
}

imu_err_t harness_start(void)
{
    imu_event_sink_t sink = cap_sink();

    sim_cfg()->p_fn  = scn_stimulus;
    sim_cfg()->p_ctx = &g_h.scn;
    g_h.now_ms = sim_time_ms();

    return (imu_terrain_init(&g_h.sys, &g_h.params, &sink));
}

void harness_run_ms(uint32_t ms)
{
    uint32_t period = g_h.params.sample_period_ms;
    uint32_t t;

    for (t = 0U; t < ms; t += period)
    {
        sim_advance_ms(period);
        g_h.now_ms = sim_time_ms();
        if (g_h.b_hint)
        {
            imu_motion_hint_t h;
            double            v;
            double            a;

            scn_speed_at(&g_h.scn, (double)g_h.now_ms * 0.001, &v, &a);
            h.b_valid       = true;
            h.b_accel_valid = true;
            h.speed_mm_s    = (int32_t)lround(v * 1000.0);
            h.accel_mm_s2   = (int32_t)lround(a * 1000.0);
            h.timestamp_ms  = g_h.now_ms;
            (void)imu_terrain_set_hint(&g_h.sys, &h);
        }
        imu_terrain_step(&g_h.sys, g_h.now_ms);
        if (!g_h.b_no_drain)
        {
            cap_drain();      /* the "Mission Controller" drains its queue */
        }
    }
}

bool harness_until_state(imu_state_t state, uint32_t max_ms)
{
    uint32_t t;
    uint32_t period = g_h.params.sample_period_ms;

    for (t = 0U; t < max_ms; t += period)
    {
        if (g_h.sys.state == state)
        {
            return (true);
        }
        harness_run_ms(period);
    }
    return (g_h.sys.state == state);
}

bool harness_until_monitoring(uint32_t max_ms)
{
    return (harness_until_state(IMU_STATE_MONITORING, max_ms));
}

imu_status_t harness_status(void)
{
    imu_status_t st;

    (void)imu_terrain_get_status(&g_h.sys, &st);
    return (st);
}

bool harness_trace_has(imu_state_t state)
{
    imu_trace_entry_t tr[IMU_STATE_TRACE_LEN];
    uint8_t           n = imu_terrain_get_trace(&g_h.sys, tr,
                                                (uint8_t)IMU_STATE_TRACE_LEN);
    uint8_t           i;

    for (i = 0U; i < n; i++)
    {
        if (tr[i].to == (uint8_t)state)
        {
            return (true);
        }
    }
    return (false);
}
```

## `tests/imu/sim_lsm303.h`

```c
/**
 * @file    sim_lsm303.h
 * @brief   HOST-ONLY register-level LSM303DLHC simulator implementing the
 *          imu_hal.h interface. Used by unit, black-box and robustness tests.
 *          Never linked into firmware.
 *
 * Models: accelerometer CTRL/STATUS/FIFO (stream mode, 32 deep, OVRN),
 * 12-bit left-justified little-endian output, ODR from CTRL_REG1_A,
 * full-scale from CTRL_REG4_A; magnetometer CRA/CRB/MR, ID registers,
 * big-endian X/Z/Y output, -4096 overflow, gain-dependent scaling.
 *
 * Fault injection: absent device (NACK), timeouts, intermittent failures,
 * stuck bus (fails until bus-recovery), wrong ID, write-ignore (read-back
 * mismatch), frozen output, random garbage bytes, magnetometer overflow.
 */

#ifndef SIM_LSM303_H
#define SIM_LSM303_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_hal.h"
#include "imu/imu_types.h"

/** Physical stimulus: specific force (g) and field (gauss), SENSOR frame. */
typedef void (*sim_stimulus_fn_t)(void * p_ctx, double t_s,
                                  imu_vec3_t * p_accel_g,
                                  imu_vec3_t * p_mag_gauss);

typedef struct
{
    /* stimulus */
    sim_stimulus_fn_t p_fn;
    void *            p_ctx;
    imu_vec3_t        const_accel;   /* used when p_fn == NULL */
    imu_vec3_t        const_mag;

    /* fault injection */
    bool             b_accel_absent;
    bool             b_mag_absent;
    bool             b_wrong_id;
    bool             b_ignore_writes;
    bool             b_frozen;
    bool             b_garbage;
    bool             b_mag_overflow;
    bool             b_bus_stuck;        /* cleared by bus recovery */
    bool             b_stuck_survives_recovery;
    uint32_t         fail_next;          /* fail the next N transfers */
    imu_hal_status_t fail_status;
    uint32_t         fail_every;         /* fail every Nth transfer (0=off) */
    uint32_t         rng;

    /* observation */
    uint32_t transfers;
    uint32_t failures_injected;
    uint32_t recover_calls;
    uint32_t init_calls;
    bool     b_bus_initialised;
} sim_config_t;

/** Reset the simulator (registers to power-on defaults, time = 0). */
void   sim_reset(void);

/** Access the configuration / fault-injection block. */
sim_config_t * sim_cfg(void);

/** Advance simulated time, generating accelerometer samples at the ODR. */
void   sim_advance_ms(uint32_t ms);

/** Set absolute time without generating samples (for wrap tests). */
void   sim_set_time_ms(uint32_t ms);

uint32_t sim_time_ms(void);

/** Number of samples currently in the simulated FIFO. */
uint32_t sim_fifo_level(void);

/** Read a raw register value (white-box checks). */
uint8_t sim_accel_reg(uint8_t reg);
uint8_t sim_mag_reg(uint8_t reg);

#endif /* SIM_LSM303_H */
```

## `tests/imu/sim_lsm303.c`

```c
/**
 * @file    sim_lsm303.c
 * @brief   HOST-ONLY LSM303DLHC register simulator (see sim_lsm303.h).
 */

#include <math.h>
#include <string.h>

#include "imu/lsm303dlhc_regs.h"
#include "sim_lsm303.h"

#define SIM_REGS 0x40U

typedef struct
{
    sim_config_t cfg;
    uint8_t      a_regs[SIM_REGS];
    uint8_t      m_regs[SIM_REGS];
    imu_raw3_t   fifo[LSM303_FIFO_DEPTH];
    uint32_t     fifo_count;
    bool         b_fifo_ovrn;
    imu_raw3_t   latest;
    bool         b_new;
    bool         b_overrun;
    double       now_ms;
    double       next_sample_ms;
    imu_raw3_t   frozen_val;
    bool         b_frozen_val;
} sim_state_t;

static sim_state_t g_sim;

static uint32_t sim_rand(void)
{
    g_sim.cfg.rng = (g_sim.cfg.rng * 1664525U) + 1013904223U;
    return (g_sim.cfg.rng >> 8);
}

void sim_reset(void)
{
    (void)memset(&g_sim, 0, sizeof(g_sim));
    g_sim.a_regs[LSM303_REG_CTRL_REG1_A] = 0x07U;   /* power-on default */
    g_sim.m_regs[LSM303_REG_CRA_REG_M]   = 0x10U;
    g_sim.m_regs[LSM303_REG_CRB_REG_M]   = 0x20U;
    g_sim.m_regs[LSM303_REG_MR_REG_M]    = 0x03U;
    g_sim.cfg.const_accel.z = 1.0f;
    g_sim.cfg.const_mag.y   = 0.40f;
    g_sim.cfg.const_mag.z   = 0.12f;
    g_sim.cfg.rng           = 12345U;
    g_sim.cfg.fail_status   = IMU_HAL_NACK;
}

sim_config_t * sim_cfg(void)
{
    return (&g_sim.cfg);
}

uint32_t sim_time_ms(void)
{
    return ((uint32_t)(uint64_t)g_sim.now_ms);
}

void sim_set_time_ms(uint32_t ms)
{
    g_sim.now_ms         = (double)ms;
    g_sim.next_sample_ms = (double)ms;
}

uint32_t sim_fifo_level(void)
{
    return (g_sim.fifo_count);
}

uint8_t sim_accel_reg(uint8_t reg)
{
    return (g_sim.a_regs[reg & 0x3FU]);
}

uint8_t sim_mag_reg(uint8_t reg)
{
    return (g_sim.m_regs[reg & 0x3FU]);
}

static double sim_odr_hz(void)
{
    static double const k_odr[8] = { 0.0, 1.0, 10.0, 25.0, 50.0, 100.0,
                                     200.0, 400.0 };
    uint8_t code = (uint8_t)(g_sim.a_regs[LSM303_REG_CTRL_REG1_A] >> 4);

    return ((code < 8U) ? k_odr[code] : 1344.0);
}

static double sim_accel_sens(void)
{
    static double const k_sens[4] = { 0.001, 0.002, 0.004, 0.012 };

    return (k_sens[(g_sim.a_regs[LSM303_REG_CTRL_REG4_A] >> 4) & 0x03U]);
}

static int16_t sim_quant_accel(float g)
{
    double c = floor(((double)g / sim_accel_sens()) + 0.5);

    if (c > 2047.0)  { c = 2047.0; }
    if (c < -2048.0) { c = -2048.0; }

    return ((int16_t)c);
}

static void sim_stimulus(double t_ms, imu_vec3_t * p_a, imu_vec3_t * p_m)
{
    if (NULL != g_sim.cfg.p_fn)
    {
        g_sim.cfg.p_fn(g_sim.cfg.p_ctx, t_ms * 0.001, p_a, p_m);
    }
    else
    {
        *p_a = g_sim.cfg.const_accel;
        *p_m = g_sim.cfg.const_mag;
    }
}

static void sim_push_sample(double t_ms)
{
    imu_vec3_t a;
    imu_vec3_t m;
    imu_raw3_t r;

    sim_stimulus(t_ms, &a, &m);
    r.x = sim_quant_accel(a.x);
    r.y = sim_quant_accel(a.y);
    r.z = sim_quant_accel(a.z);

    if (g_sim.cfg.b_frozen)
    {
        if (!g_sim.b_frozen_val)
        {
            g_sim.frozen_val   = r;
            g_sim.b_frozen_val = true;
        }
        r = g_sim.frozen_val;
    }
    else
    {
        g_sim.b_frozen_val = false;
    }

    if (g_sim.b_new)
    {
        g_sim.b_overrun = true;
    }
    g_sim.latest = r;
    g_sim.b_new  = true;

    if ((0U != (g_sim.a_regs[LSM303_REG_CTRL_REG5_A] & LSM303_CTRL5_FIFO_EN))
        && (LSM303_FIFO_MODE_STREAM
            == (g_sim.a_regs[LSM303_REG_FIFO_CTRL_REG_A] & 0xC0U)))
    {
        if (g_sim.fifo_count >= LSM303_FIFO_DEPTH)
        {
            (void)memmove(&g_sim.fifo[0], &g_sim.fifo[1],
                          sizeof(g_sim.fifo[0]) * (LSM303_FIFO_DEPTH - 1U));
            g_sim.fifo_count  = LSM303_FIFO_DEPTH - 1U;
            g_sim.b_fifo_ovrn = true;
        }
        g_sim.fifo[g_sim.fifo_count] = r;
        g_sim.fifo_count++;
    }
}

void sim_advance_ms(uint32_t ms)
{
    double end = g_sim.now_ms + (double)ms;
    double odr = sim_odr_hz();

    if ((odr > 0.0)
        && (0U != (g_sim.a_regs[LSM303_REG_CTRL_REG1_A] & 0x07U)))
    {
        double period = 1000.0 / odr;

        if (g_sim.next_sample_ms < g_sim.now_ms)
        {
            g_sim.next_sample_ms = g_sim.now_ms;
        }
        while (g_sim.next_sample_ms <= end)
        {
            sim_push_sample(g_sim.next_sample_ms);
            g_sim.next_sample_ms += period;
        }
    }
    g_sim.now_ms = end;
}

/* ---------------------------------------------------------------------- */

static bool sim_inject_failure(imu_hal_status_t * p_status)
{
    g_sim.cfg.transfers++;

    if (g_sim.cfg.b_bus_stuck)
    {
        *p_status = IMU_HAL_TIMEOUT;
        g_sim.cfg.failures_injected++;
        return (true);
    }
    if (g_sim.cfg.fail_next > 0U)
    {
        g_sim.cfg.fail_next--;
        *p_status = g_sim.cfg.fail_status;
        g_sim.cfg.failures_injected++;
        return (true);
    }
    if ((g_sim.cfg.fail_every > 0U)
        && (0U == (g_sim.cfg.transfers % g_sim.cfg.fail_every)))
    {
        *p_status = g_sim.cfg.fail_status;
        g_sim.cfg.failures_injected++;
        return (true);
    }
    if (!g_sim.cfg.b_bus_initialised)
    {
        *p_status = IMU_HAL_ERROR;
        return (true);
    }

    return (false);
}

static void sim_encode_accel(imu_raw3_t const * p_r, uint8_t * p_buf)
{
    int16_t const v[3] = { p_r->x, p_r->y, p_r->z };
    uint8_t       i;

    for (i = 0U; i < 3U; i++)
    {
        uint16_t w = (uint16_t)((uint16_t)v[i] << 4);

        p_buf[2U * i]        = (uint8_t)(w & 0xFFU);
        p_buf[(2U * i) + 1U] = (uint8_t)(w >> 8);
    }
}

static int16_t sim_mag_count(float gauss, bool b_z)
{
    static double const k_xy[8] = { 0, 1100, 855, 670, 450, 400, 330, 230 };
    static double const k_z[8]  = { 0, 980, 760, 600, 400, 355, 295, 205 };
    uint8_t gn = (uint8_t)(g_sim.m_regs[LSM303_REG_CRB_REG_M] >> 5);
    double  c  = floor(((double)gauss * (b_z ? k_z[gn] : k_xy[gn])) + 0.5);

    if (g_sim.cfg.b_mag_overflow || (c > 2047.0) || (c < -2048.0))
    {
        return ((int16_t)LSM303_MAG_OVERFLOW_VALUE);
    }

    return ((int16_t)c);
}

static void sim_read_accel(uint8_t reg, uint8_t * p_rx, size_t len)
{
    uint8_t addr = (uint8_t)(reg & 0x7FU);
    size_t  i;

    if ((LSM303_REG_OUT_X_L_A == addr) && (6U == len))
    {
        imu_raw3_t r;

        if ((0U != (g_sim.a_regs[LSM303_REG_CTRL_REG5_A]
                    & LSM303_CTRL5_FIFO_EN)) && (g_sim.fifo_count > 0U))
        {
            r = g_sim.fifo[0];
            (void)memmove(&g_sim.fifo[0], &g_sim.fifo[1],
                          sizeof(g_sim.fifo[0]) * (LSM303_FIFO_DEPTH - 1U));
            g_sim.fifo_count--;
            g_sim.b_fifo_ovrn = false;
        }
        else
        {
            r = g_sim.latest;
        }
        g_sim.b_new     = false;
        g_sim.b_overrun = false;
        sim_encode_accel(&r, p_rx);
        return;
    }

    for (i = 0U; i < len; i++)
    {
        uint8_t a = (uint8_t)((addr + i) & 0x3FU);
        uint8_t v = g_sim.a_regs[a];

        if (LSM303_REG_STATUS_REG_A == a)
        {
            v = (uint8_t)((g_sim.b_new ? LSM303_STATUS_ZYXDA : 0U)
                          | (g_sim.b_overrun ? LSM303_STATUS_ZYXOR : 0U));
        }
        else if (LSM303_REG_FIFO_SRC_REG_A == a)
        {
            if (g_sim.fifo_count >= LSM303_FIFO_DEPTH)
            {
                v = (uint8_t)(LSM303_FIFO_SRC_OVRN | 0x1FU);
            }
            else if (0U == g_sim.fifo_count)
            {
                v = LSM303_FIFO_SRC_EMPTY;
            }
            else
            {
                v = (uint8_t)g_sim.fifo_count;
            }
        }
        else
        {
            /* plain register */
        }
        p_rx[i] = v;
    }
}

static void sim_read_mag(uint8_t reg, uint8_t * p_rx, size_t len)
{
    imu_vec3_t a;
    imu_vec3_t m;
    int16_t    out[3];
    uint8_t    buf[SIM_REGS];
    size_t     i;

    (void)memcpy(buf, g_sim.m_regs, sizeof(buf));
    buf[LSM303_REG_IRA_REG_M]      = g_sim.cfg.b_wrong_id ? 0x00U
                                                          : LSM303_IRA_VALUE;
    buf[LSM303_REG_IRA_REG_M + 1U] = LSM303_IRB_VALUE;
    buf[LSM303_REG_IRA_REG_M + 2U] = LSM303_IRC_VALUE;
    buf[LSM303_REG_SR_REG_M]       = LSM303_SR_DRDY;

    sim_stimulus(g_sim.now_ms, &a, &m);
    out[0] = sim_mag_count(m.x, false);
    out[1] = sim_mag_count(m.z, true);
    out[2] = sim_mag_count(m.y, false);
    for (i = 0U; i < 3U; i++)
    {
        buf[LSM303_REG_OUT_X_H_M + (2U * i)]      =
            (uint8_t)((uint16_t)out[i] >> 8);
        buf[LSM303_REG_OUT_X_H_M + (2U * i) + 1U] =
            (uint8_t)((uint16_t)out[i] & 0xFFU);
    }

    for (i = 0U; i < len; i++)
    {
        p_rx[i] = buf[(reg + i) & 0x3FU];
    }
}

/* ------------------------------ HAL ---------------------------------- */

imu_hal_status_t imu_hal_i2c_init(uint32_t baud_hz)
{
    (void)baud_hz;
    g_sim.cfg.init_calls++;
    g_sim.cfg.b_bus_initialised = true;
    return (IMU_HAL_OK);
}

void imu_hal_i2c_deinit(void)
{
    g_sim.cfg.b_bus_initialised = false;
}

imu_hal_status_t imu_hal_i2c_write(uint8_t addr7, uint8_t const * p_data,
                                   size_t len, uint32_t timeout_us)
{
    imu_hal_status_t st;
    uint8_t          reg;

    (void)timeout_us;
    if (sim_inject_failure(&st))
    {
        return (st);
    }
    if ((LSM303_ACCEL_ADDR == addr7) && g_sim.cfg.b_accel_absent)
    {
        return (IMU_HAL_NACK);
    }
    if ((LSM303_MAG_ADDR == addr7) && g_sim.cfg.b_mag_absent)
    {
        return (IMU_HAL_NACK);
    }
    if ((LSM303_ACCEL_ADDR != addr7) && (LSM303_MAG_ADDR != addr7))
    {
        return (IMU_HAL_NACK);
    }
    if ((NULL == p_data) || (len < 2U))
    {
        return (IMU_HAL_ERROR);
    }

    reg = (uint8_t)(p_data[0] & 0x3FU);
    if (!g_sim.cfg.b_ignore_writes)
    {
        if (LSM303_ACCEL_ADDR == addr7)
        {
            g_sim.a_regs[reg] = p_data[1];
            if ((LSM303_REG_FIFO_CTRL_REG_A == reg)
                && (LSM303_FIFO_MODE_BYPASS == (p_data[1] & 0xC0U)))
            {
                g_sim.fifo_count  = 0U;
                g_sim.b_fifo_ovrn = false;
            }
        }
        else
        {
            g_sim.m_regs[reg] = p_data[1];
        }
    }

    return (IMU_HAL_OK);
}

imu_hal_status_t imu_hal_i2c_write_read(uint8_t addr7,
                                        uint8_t const * p_tx, size_t tx_len,
                                        uint8_t * p_rx, size_t rx_len,
                                        uint32_t timeout_us)
{
    imu_hal_status_t st;
    size_t           i;

    (void)timeout_us;
    if (sim_inject_failure(&st))
    {
        return (st);
    }
    if ((LSM303_ACCEL_ADDR == addr7) && g_sim.cfg.b_accel_absent)
    {
        return (IMU_HAL_NACK);
    }
    if ((LSM303_MAG_ADDR == addr7) && g_sim.cfg.b_mag_absent)
    {
        return (IMU_HAL_NACK);
    }
    if ((LSM303_ACCEL_ADDR != addr7) && (LSM303_MAG_ADDR != addr7))
    {
        return (IMU_HAL_NACK);
    }
    if ((NULL == p_tx) || (NULL == p_rx) || (1U != tx_len) || (0U == rx_len))
    {
        return (IMU_HAL_ERROR);
    }

    if (LSM303_ACCEL_ADDR == addr7)
    {
        sim_read_accel(p_tx[0], p_rx, rx_len);
    }
    else
    {
        sim_read_mag(p_tx[0], p_rx, rx_len);
    }

    if (g_sim.cfg.b_garbage)
    {
        for (i = 0U; i < rx_len; i++)
        {
            p_rx[i] = (uint8_t)(sim_rand() & 0xFFU);
        }
    }

    return (IMU_HAL_OK);
}

void imu_hal_i2c_bus_recover(void)
{
    g_sim.cfg.recover_calls++;
    if (!g_sim.cfg.b_stuck_survives_recovery)
    {
        g_sim.cfg.b_bus_stuck = false;
    }
}

uint32_t imu_hal_time_ms(void)
{
    return (sim_time_ms());
}

uint32_t imu_hal_time_us(void)
{
    return ((uint32_t)(uint64_t)(g_sim.now_ms * 1000.0));
}
```

## `tests/imu/scenario.h`

```c
/**
 * @file    scenario.h
 * @brief   HOST-ONLY physically-based vehicle motion generator used to feed
 *          the LSM303DLHC simulator with known ground truth.
 *
 * Model: rigid car, wheelbase L, IMU at fraction k of L from the rear axle,
 * driving along a straight or curved path at speed v(t). Humps have a
 * raised-cosine profile z(x) = h/2 (1 - cos(2 pi x / Lh)). Pitch is
 * asin((z_front - z_rear) / L). Specific force = R^T (a_world + g z_up)
 * plus optional impact pulses, motor vibration and sensor noise. The
 * magnetometer sees the Earth field (plus optional hard/soft iron and a
 * disturbance source). The sensor frame = M * vehicle frame (mounting).
 */

#ifndef SCENARIO_H
#define SCENARIO_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_types.h"

#define SCN_MAX_HUMPS     (8U)
#define SCN_MAX_IMPACTS   (8U)
#define SCN_MAX_SEGMENTS  (16U)

typedef struct
{
    double start_m;      /* distance along path where the hump starts */
    double height_m;
    double length_m;
} scn_hump_t;

typedef struct
{
    double     t_s;
    double     duration_s;
    double     peak_g;
    imu_vec3_t dir_vehicle;   /* unit direction, vehicle frame */
} scn_impact_t;

/** Piecewise motion segment: from t_start for duration. */
typedef struct
{
    double t_start_s;
    double duration_s;
    double v_start_mps;
    double v_end_mps;
    double yaw_rate_dps;      /* + = clockwise (right turn) */
    double roll_deg;          /* static roll (side slope / lifted) */
    double pitch_deg;         /* static pitch (ramp) */
} scn_segment_t;

typedef struct
{
    /* vehicle */
    double wheelbase_m;
    double imu_pos_frac;
    double heading0_deg;

    /* environment */
    double bh_gauss;          /* horizontal earth field */
    double bup_gauss;         /* vertical (up) component */

    /* mounting: sensor = M * vehicle */
    double mount[3][3];
    imu_vec3_t hard_iron;     /* sensor frame, gauss */
    imu_vec3_t soft_scale;    /* sensor frame */
    imu_vec3_t disturb_gauss; /* extra field, sensor frame */
    double disturb_t0_s;
    double disturb_t1_s;

    /* noise */
    double accel_noise_g;     /* sensor white noise (sigma) */
    double vib_noise_g;       /* extra when moving (sigma) */
    double mag_noise_gauss;
    uint32_t seed;

    scn_segment_t seg[SCN_MAX_SEGMENTS];
    uint32_t      n_seg;
    scn_hump_t    hump[SCN_MAX_HUMPS];
    uint32_t      n_hump;
    scn_impact_t  imp[SCN_MAX_IMPACTS];
    uint32_t      n_imp;

    /* integrated state */
    double t_s;
    double s_m;
    double heading_deg;
    double v_mps;

    /* ground truth log */
    double max_pitch_deg;
} scenario_t;

void scn_init(scenario_t * p_s);
void scn_add_segment(scenario_t * p_s, double t0, double dur, double v0,
                     double v1, double yaw_rate_dps);
void scn_add_hump(scenario_t * p_s, double start_m, double height_m,
                  double length_m);
void scn_add_impact(scenario_t * p_s, double t_s, double dur_s,
                    double peak_g, float dx, float dy, float dz);

/** Stimulus callback for the simulator (sim_stimulus_fn_t). */
void scn_stimulus(void * p_ctx, double t_s, imu_vec3_t * p_accel_g,
                  imu_vec3_t * p_mag_gauss);

/** Speed / along-path acceleration at time t (encoder truth). */
void scn_speed_at(scenario_t const * p_s, double t_s, double * p_v,
                  double * p_a);

/** Ground-truth peak pitch (deg) for a hump with the car's wheelbase. */
double scn_true_peak_pitch_deg(double height_m, double length_m,
                               double wheelbase_m);

#endif /* SCENARIO_H */
```

## `tests/imu/scenario.c`

```c
/**
 * @file    scenario.c
 * @brief   HOST-ONLY vehicle motion generator (see scenario.h).
 */

#include <math.h>
#include <string.h>

#include "scenario.h"

#define SCN_PI        (3.14159265358979323846)
#define SCN_D2R       (SCN_PI / 180.0)
#define SCN_G         (9.80665)
#define SCN_DT_S      (0.0005)

static double scn_randn(scenario_t * p_s)
{
    double u1;
    double u2;

    p_s->seed = (p_s->seed * 1664525U) + 1013904223U;
    u1 = ((double)(p_s->seed >> 8) + 1.0) / 16777217.0;
    p_s->seed = (p_s->seed * 1664525U) + 1013904223U;
    u2 = ((double)(p_s->seed >> 8) + 1.0) / 16777217.0;

    return (sqrt(-2.0 * log(u1)) * cos(2.0 * SCN_PI * u2));
}

void scn_init(scenario_t * p_s)
{
    (void)memset(p_s, 0, sizeof(*p_s));
    p_s->wheelbase_m   = 0.140;
    p_s->imu_pos_frac  = 0.5;
    p_s->bh_gauss      = 0.40;
    p_s->bup_gauss     = 0.12;
    p_s->mount[0][0]   = 1.0;
    p_s->mount[1][1]   = 1.0;
    p_s->mount[2][2]   = 1.0;
    p_s->soft_scale.x  = 1.0f;
    p_s->soft_scale.y  = 1.0f;
    p_s->soft_scale.z  = 1.0f;
    p_s->accel_noise_g = 0.002;
    p_s->vib_noise_g   = 0.0;
    p_s->mag_noise_gauss = 0.002;
    p_s->seed          = 4242U;
    p_s->disturb_t0_s  = -1.0;
    p_s->disturb_t1_s  = -1.0;
}

void scn_add_segment(scenario_t * p_s, double t0, double dur, double v0,
                     double v1, double yaw_rate_dps)
{
    if (p_s->n_seg < SCN_MAX_SEGMENTS)
    {
        scn_segment_t * p = &p_s->seg[p_s->n_seg];

        (void)memset(p, 0, sizeof(*p));
        p->t_start_s    = t0;
        p->duration_s   = dur;
        p->v_start_mps  = v0;
        p->v_end_mps    = v1;
        p->yaw_rate_dps = yaw_rate_dps;
        p_s->n_seg++;
    }
}

void scn_add_hump(scenario_t * p_s, double start_m, double height_m,
                  double length_m)
{
    if (p_s->n_hump < SCN_MAX_HUMPS)
    {
        p_s->hump[p_s->n_hump].start_m  = start_m;
        p_s->hump[p_s->n_hump].height_m = height_m;
        p_s->hump[p_s->n_hump].length_m = length_m;
        p_s->n_hump++;
    }
}

void scn_add_impact(scenario_t * p_s, double t_s, double dur_s,
                    double peak_g, float dx, float dy, float dz)
{
    if (p_s->n_imp < SCN_MAX_IMPACTS)
    {
        scn_impact_t * p = &p_s->imp[p_s->n_imp];

        p->t_s           = t_s;
        p->duration_s    = dur_s;
        p->peak_g        = peak_g;
        p->dir_vehicle.x = dx;
        p->dir_vehicle.y = dy;
        p->dir_vehicle.z = dz;
        p_s->n_imp++;
    }
}

static scn_segment_t const * scn_seg_at(scenario_t const * p_s, double t)
{
    uint32_t i;

    for (i = 0U; i < p_s->n_seg; i++)
    {
        scn_segment_t const * p = &p_s->seg[i];

        if ((t >= p->t_start_s) && (t < (p->t_start_s + p->duration_s)))
        {
            return (p);
        }
    }

    return (NULL);
}

static void scn_kin(scenario_t const * p_s, double t, double * p_v,
                    double * p_a, double * p_yaw_rate,
                    double * p_roll, double * p_pitch)
{
    scn_segment_t const * p = scn_seg_at(p_s, t);

    *p_v = 0.0; *p_a = 0.0; *p_yaw_rate = 0.0; *p_roll = 0.0; *p_pitch = 0.0;
    if (NULL != p)
    {
        double f = (t - p->t_start_s) / p->duration_s;

        *p_a        = (p->v_end_mps - p->v_start_mps) / p->duration_s;
        *p_v        = p->v_start_mps + (f * (p->v_end_mps - p->v_start_mps));
        *p_yaw_rate = p->yaw_rate_dps;
        *p_roll     = p->roll_deg;
        *p_pitch    = p->pitch_deg;
    }
}

/** Ground height and its first/second derivative w.r.t. distance. */
static void scn_ground(scenario_t const * p_s, double x, double * p_z,
                       double * p_dz, double * p_d2z)
{
    uint32_t i;

    *p_z = 0.0; *p_dz = 0.0; *p_d2z = 0.0;
    for (i = 0U; i < p_s->n_hump; i++)
    {
        scn_hump_t const * p = &p_s->hump[i];
        double             u = x - p->start_m;

        if ((u >= 0.0) && (u <= p->length_m))
        {
            double k = 2.0 * SCN_PI / p->length_m;

            *p_z   += 0.5 * p->height_m * (1.0 - cos(k * u));
            *p_dz  += 0.5 * p->height_m * k * sin(k * u);
            *p_d2z += 0.5 * p->height_m * k * k * cos(k * u);
        }
    }
}

void scn_speed_at(scenario_t const * p_s, double t_s, double * p_v,
                  double * p_a)
{
    double yr;
    double r;
    double pt;

    scn_kin(p_s, t_s, p_v, p_a, &yr, &r, &pt);
}

double scn_true_peak_pitch_deg(double height_m, double length_m,
                               double wheelbase_m)
{
    double best = 0.0;
    double s;
    double k = 2.0 * SCN_PI / length_m;

    for (s = 0.0; s <= (length_m + wheelbase_m); s += 0.0005)
    {
        double zf = ((s >= 0.0) && (s <= length_m))
                    ? 0.5 * height_m * (1.0 - cos(k * s)) : 0.0;
        double sr = s - wheelbase_m;
        double zr = ((sr >= 0.0) && (sr <= length_m))
                    ? 0.5 * height_m * (1.0 - cos(k * sr)) : 0.0;
        double p  = asin((zf - zr) / wheelbase_m) / SCN_D2R;

        if (p > best)
        {
            best = p;
        }
    }

    return (best);
}

static void scn_integrate_to(scenario_t * p_s, double t)
{
    while (p_s->t_s < t)
    {
        double v;
        double a;
        double yr;
        double r;
        double pt;
        double dt = fmin(SCN_DT_S, t - p_s->t_s);

        scn_kin(p_s, p_s->t_s, &v, &a, &yr, &r, &pt);
        p_s->s_m         += v * dt;
        p_s->heading_deg += yr * dt;
        p_s->v_mps        = v;
        p_s->t_s         += dt;
    }
}

void scn_stimulus(void * p_ctx, double t_s, imu_vec3_t * p_accel_g,
                  imu_vec3_t * p_mag_gauss)
{
    scenario_t * p_s = (scenario_t *)p_ctx;
    double v, a, yr, roll_s, pitch_s;
    double zf, dzf, d2zf, zr, dzr, d2zr;
    double L = p_s->wheelbase_m;
    double k = p_s->imu_pos_frac;
    double pitch, roll, psi;
    double fx[3], ly[3], uz[3], l0[3], u0[3];
    double f_world[3], b_world[3];
    double f_b[3], m_b[3], f_s[3], m_s[3];
    double z_dd;
    uint32_t i;
    uint32_t r;

    scn_integrate_to(p_s, t_s);
    scn_kin(p_s, t_s, &v, &a, &yr, &roll_s, &pitch_s);

    scn_ground(p_s, p_s->s_m, &zf, &dzf, &d2zf);
    scn_ground(p_s, p_s->s_m - L, &zr, &dzr, &d2zr);

    pitch = asin(fmax(-0.99, fmin(0.99, (zf - zr) / L))) + (pitch_s * SCN_D2R);
    roll  = roll_s * SCN_D2R;
    psi   = p_s->heading_deg * SCN_D2R;
    if ((pitch / SCN_D2R) > p_s->max_pitch_deg)
    {
        p_s->max_pitch_deg = pitch / SCN_D2R;
    }

    /* Vertical acceleration of the IMU point. */
    z_dd = ((1.0 - k) * ((d2zr * v * v) + (dzr * a)))
         + (k * ((d2zf * v * v) + (dzf * a)));

    /* Body axes in world ENU. */
    fx[0] = sin(psi) * cos(pitch);
    fx[1] = cos(psi) * cos(pitch);
    fx[2] = sin(pitch);
    l0[0] = -cos(psi);
    l0[1] = sin(psi);
    l0[2] = 0.0;
    u0[0] = (fx[1] * l0[2]) - (fx[2] * l0[1]);
    u0[1] = (fx[2] * l0[0]) - (fx[0] * l0[2]);
    u0[2] = (fx[0] * l0[1]) - (fx[1] * l0[0]);
    for (i = 0U; i < 3U; i++)
    {
        ly[i] = (cos(roll) * l0[i]) + (sin(roll) * u0[i]);
        uz[i] = (-sin(roll) * l0[i]) + (cos(roll) * u0[i]);
    }

    /* World acceleration: along-path + centripetal + vertical, plus 1 g. */
    {
        double omega   = yr * SCN_D2R;             /* clockwise + */
        double lat_acc = -v * omega;               /* to the left */
        double hdir[3] = { sin(psi), cos(psi), 0.0 };

        for (i = 0U; i < 3U; i++)
        {
            f_world[i] = (a * hdir[i]) + (lat_acc * l0[i]);
        }
        f_world[2] += z_dd;
        for (i = 0U; i < 3U; i++)
        {
            f_world[i] /= SCN_G;
        }
        f_world[2] += 1.0;
    }

    b_world[0] = 0.0;
    b_world[1] = p_s->bh_gauss;
    b_world[2] = p_s->bup_gauss;

    f_b[0] = (f_world[0] * fx[0]) + (f_world[1] * fx[1]) + (f_world[2] * fx[2]);
    f_b[1] = (f_world[0] * ly[0]) + (f_world[1] * ly[1]) + (f_world[2] * ly[2]);
    f_b[2] = (f_world[0] * uz[0]) + (f_world[1] * uz[1]) + (f_world[2] * uz[2]);
    m_b[0] = (b_world[0] * fx[0]) + (b_world[1] * fx[1]) + (b_world[2] * fx[2]);
    m_b[1] = (b_world[0] * ly[0]) + (b_world[1] * ly[1]) + (b_world[2] * ly[2]);
    m_b[2] = (b_world[0] * uz[0]) + (b_world[1] * uz[1]) + (b_world[2] * uz[2]);

    /* Impacts (vehicle frame, half-sine pulse). */
    for (i = 0U; i < p_s->n_imp; i++)
    {
        scn_impact_t const * p = &p_s->imp[i];

        if ((t_s >= p->t_s) && (t_s <= (p->t_s + p->duration_s)))
        {
            double amp = p->peak_g * sin(SCN_PI * (t_s - p->t_s)
                                         / p->duration_s);

            f_b[0] += amp * (double)p->dir_vehicle.x;
            f_b[1] += amp * (double)p->dir_vehicle.y;
            f_b[2] += amp * (double)p->dir_vehicle.z;
        }
    }

    /* Noise. */
    for (i = 0U; i < 3U; i++)
    {
        f_b[i] += p_s->accel_noise_g * scn_randn(p_s);
        if (fabs(v) > 1e-3)
        {
            f_b[i] += p_s->vib_noise_g * scn_randn(p_s);
        }
        m_b[i] += p_s->mag_noise_gauss * scn_randn(p_s);
    }

    /* Mounting: sensor = M * vehicle. */
    for (r = 0U; r < 3U; r++)
    {
        f_s[r] = (p_s->mount[r][0] * f_b[0]) + (p_s->mount[r][1] * f_b[1])
               + (p_s->mount[r][2] * f_b[2]);
        m_s[r] = (p_s->mount[r][0] * m_b[0]) + (p_s->mount[r][1] * m_b[1])
               + (p_s->mount[r][2] * m_b[2]);
    }

    /* Hard / soft iron and disturbance (sensor frame). */
    m_s[0] = (m_s[0] * (double)p_s->soft_scale.x) + (double)p_s->hard_iron.x;
    m_s[1] = (m_s[1] * (double)p_s->soft_scale.y) + (double)p_s->hard_iron.y;
    m_s[2] = (m_s[2] * (double)p_s->soft_scale.z) + (double)p_s->hard_iron.z;
    if ((t_s >= p_s->disturb_t0_s) && (t_s < p_s->disturb_t1_s))
    {
        m_s[0] += (double)p_s->disturb_gauss.x;
        m_s[1] += (double)p_s->disturb_gauss.y;
        m_s[2] += (double)p_s->disturb_gauss.z;
    }

    p_accel_g->x   = (float)f_s[0];
    p_accel_g->y   = (float)f_s[1];
    p_accel_g->z   = (float)f_s[2];
    p_mag_gauss->x = (float)m_s[0];
    p_mag_gauss->y = (float)m_s[1];
    p_mag_gauss->z = (float)m_s[2];
}
```

## `tests/imu/fake_tk/tk/tkernel.h`

```c
/**
 * @file    tkernel.h  (TEST DOUBLE - HOST ONLY)
 * @brief   Minimal fake of the uT-Kernel 3.0 API subset used by imu_rtos.c.
 *
 * Types, structure layouts and constants mirror include/tk/typedef.h,
 * include/tk/syscall.h and include/tk/errno.h of the TRON Forum uT-Kernel
 * 3.0 RP2040 BSP (with USE_OBJECT_NAME = 0). This file is only on the
 * include path of the host test build; the firmware build uses the real
 * kernel headers. It lets the RTOS wrapper logic (object creation,
 * locking, queue overflow mapping, overrun accounting, shutdown) be
 * exercised without hardware.
 */
#ifndef FAKE_TK_TKERNEL_H
#define FAKE_TK_TKERNEL_H

#include <stdbool.h>
#include <stdint.h>

#define CONST const
#define LOCAL static
#define EXPORT

typedef int32_t  INT;
typedef uint32_t UINT;
typedef int32_t  W;
typedef uint32_t UW;
typedef INT      ID;
typedef INT      PRI;
typedef INT      ER;
typedef INT      SZ;
typedef W        TMO;
typedef UW       RELTIM;
typedef UW       ATR;
typedef void     (*FP)();

#define TSK_SELF    0
#define TMO_POL     0
#define TMO_FEVR    (-1)

#define TA_HLNG     0x00000001UL
#define TA_USERBUF  0x00000020UL
#define TA_RNG0     0x00000000UL
#define TA_TFIFO    0x00000000UL
#define TA_TPRI     0x00000001UL
#define TA_INHERIT  0x00000002UL
#define TA_STA      0x00000002UL

#define E_OK        (0)
#define E_PAR       (-17)
#define E_NOMEM     (-33)
#define E_QOVR      (-43)
#define E_TMOUT     (-50)

typedef struct t_ctsk {
    void   *exinf;
    ATR     tskatr;
    FP      task;
    PRI     itskpri;
    SZ      stksz;
    void   *bufptr;
} T_CTSK;

typedef struct t_cmtx {
    void   *exinf;
    ATR     mtxatr;
    PRI     ceilpri;
} T_CMTX;

typedef struct t_cmbf {
    void   *exinf;
    ATR     mbfatr;
    SZ      bufsz;
    INT     maxmsz;
    void   *bufptr;
} T_CMBF;

typedef struct t_ccyc {
    void   *exinf;
    ATR     cycatr;
    FP      cychdr;
    RELTIM  cyctim;
    RELTIM  cycphs;
} T_CCYC;

ID  tk_cre_tsk(CONST T_CTSK *pk_ctsk);
ER  tk_sta_tsk(ID tskid, INT stacd);
ER  tk_slp_tsk(TMO tmout);
ER  tk_wup_tsk(ID tskid);
INT tk_can_wup(ID tskid);
void tk_ext_tsk(void);
ID  tk_cre_cyc(CONST T_CCYC *pk_ccyc);
ER  tk_sta_cyc(ID cycid);
ER  tk_stp_cyc(ID cycid);
ID  tk_cre_mtx(CONST T_CMTX *pk_cmtx);
ER  tk_loc_mtx(ID mtxid, TMO tmout);
ER  tk_unl_mtx(ID mtxid);
ID  tk_cre_mbf(CONST T_CMBF *pk_cmbf);
ER  tk_snd_mbf(ID mbfid, CONST void *msg, INT msgsz, TMO tmout);
INT tk_rcv_mbf(ID mbfid, void *msg, TMO tmout);
ER  tk_set_flg(ID flgid, UINT setptn);

/* ---- test control ---- */
typedef struct
{
    T_CTSK   ctsk;
    T_CMTX   cmtx;
    T_CMBF   cmbf;
    T_CCYC   ccyc;
    uint32_t n_cre_tsk, n_cre_mtx, n_cre_mbf, n_cre_cyc;
    uint32_t n_sta_tsk, n_sta_cyc, n_stp_cyc, n_wup;
    ER       cre_mbf_result;      /* 0 = success */
    bool     b_lock_fail;
    bool     b_slp_timeout;
    INT      can_wup_result;
    uint32_t slp_calls;
    uint32_t stop_after;          /* call imu_rtos_stop() after N sleeps */
    uint32_t period_ms;           /* sim time advanced per sleep */
    uint32_t mbf_capacity;
    uint32_t mbf_level;
    uint32_t mbf_sent;
    uint8_t  mbf_last[64];
    INT      mbf_last_size;
    ID       flg_id;
    UINT     flg_ptn;
    uint32_t flg_calls;
    uint32_t locks;
    uint32_t unlocks;
} fake_tk_t;

extern fake_tk_t g_fake_tk;

void fake_tk_reset(void);
/** Run the created task body until it calls tk_ext_tsk(). */
void fake_tk_run_task(void);

#endif /* FAKE_TK_TKERNEL_H */
```

## `tests/imu/fake_tk/fake_tk.c`

```c
/**
 * @file    fake_tk.c  (TEST DOUBLE - HOST ONLY)
 * @brief   Fake uT-Kernel service calls for host tests of imu_rtos.c.
 */
#include <setjmp.h>
#include <string.h>

#include <tk/tkernel.h>

#include "imu/imu_rtos.h"
#include "sim_lsm303.h"

fake_tk_t g_fake_tk;
static jmp_buf g_exit_jmp;
static FP      g_saved_task;   /* survives fake_tk_reset(): kernel objects persist */

void fake_tk_reset(void)
{
    (void)memset(&g_fake_tk, 0, sizeof(g_fake_tk));
    g_fake_tk.period_ms    = 20U;
    g_fake_tk.mbf_capacity = 16U;
}

ID tk_cre_tsk(CONST T_CTSK *pk)  { g_fake_tk.ctsk = *pk; g_saved_task = pk->task; g_fake_tk.n_cre_tsk++; return 3; }
ER tk_sta_tsk(ID id, INT stacd)  { (void)id; (void)stacd; g_fake_tk.n_sta_tsk++; return E_OK; }
ER tk_wup_tsk(ID id)             { (void)id; g_fake_tk.n_wup++; return E_OK; }
INT tk_can_wup(ID id)            { (void)id; return g_fake_tk.can_wup_result; }
ID tk_cre_cyc(CONST T_CCYC *pk)  { g_fake_tk.ccyc = *pk; g_fake_tk.n_cre_cyc++; return 5; }
ER tk_sta_cyc(ID id)             { (void)id; g_fake_tk.n_sta_cyc++; return E_OK; }
ER tk_stp_cyc(ID id)             { (void)id; g_fake_tk.n_stp_cyc++; return E_OK; }
ID tk_cre_mtx(CONST T_CMTX *pk)  { g_fake_tk.cmtx = *pk; g_fake_tk.n_cre_mtx++; return 7; }

ER tk_loc_mtx(ID id, TMO tmo)
{
    (void)id; (void)tmo;
    if (g_fake_tk.b_lock_fail) { return E_TMOUT; }
    g_fake_tk.locks++;
    return E_OK;
}

ER tk_unl_mtx(ID id) { (void)id; g_fake_tk.unlocks++; return E_OK; }

ID tk_cre_mbf(CONST T_CMBF *pk)
{
    g_fake_tk.cmbf = *pk;
    g_fake_tk.n_cre_mbf++;
    return (0 != g_fake_tk.cre_mbf_result) ? g_fake_tk.cre_mbf_result : 9;
}

ER tk_snd_mbf(ID id, CONST void *msg, INT sz, TMO tmo)
{
    (void)id;
    if (TMO_POL != tmo) { return E_PAR; }       /* must never block */
    if (sz > g_fake_tk.cmbf.maxmsz && g_fake_tk.cmbf.maxmsz > 0) { return E_PAR; }
    if (g_fake_tk.mbf_level >= g_fake_tk.mbf_capacity) { return E_TMOUT; }
    g_fake_tk.mbf_level++;
    g_fake_tk.mbf_sent++;
    g_fake_tk.mbf_last_size = sz;
    (void)memcpy(g_fake_tk.mbf_last, msg, (size_t)sz);
    return E_OK;
}

INT tk_rcv_mbf(ID id, void *msg, TMO tmo) { (void)id; (void)msg; (void)tmo; return E_TMOUT; }

ER tk_set_flg(ID id, UINT ptn)
{
    g_fake_tk.flg_id = id;
    g_fake_tk.flg_ptn |= ptn;
    g_fake_tk.flg_calls++;
    return E_OK;
}

ER tk_slp_tsk(TMO tmo)
{
    (void)tmo;
    g_fake_tk.slp_calls++;
    if ((g_fake_tk.stop_after > 0U) && (g_fake_tk.slp_calls >= g_fake_tk.stop_after))
    {
        (void)imu_rtos_stop();
    }
    sim_advance_ms(g_fake_tk.period_ms);
    return g_fake_tk.b_slp_timeout ? E_TMOUT : E_OK;
}

void tk_ext_tsk(void)
{
    longjmp(g_exit_jmp, 1);
}

void fake_tk_run_task(void)
{
    if (0 == setjmp(g_exit_jmp))
    {
        void (*p_task)(INT, void *) = (void (*)(INT, void *))g_saved_task;
        if (NULL != p_task)
        {
            p_task(0, NULL);
        }
    }
}
```

## `tests/imu/test_unit_driver.c`

```c
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
```

## `tests/imu/test_unit_math_cal.c`

```c
/**
 * @file    test_unit_math_cal.c
 * @brief   Unit tests: math helpers, parameter validation, calibration.
 */

#include <string.h>

#include "imu/imu_calibration.h"
#include "imu/imu_config.h"
#include "imu/imu_math.h"
#include "test_util.h"

static uint32_t g_rng = 99U;

static double urand(void)
{
    g_rng = (g_rng * 1664525U) + 1013904223U;
    return ((double)(g_rng >> 8) / 16777216.0);
}

static double nrand(void)
{
    double u1 = urand() + 1e-9;
    double u2 = urand();

    return (sqrt(-2.0 * log(u1)) * cos(6.283185307 * u2));
}

static void t_wrap(void)
{
    TEST_ASSERT_NEAR(imu_wrap_360(-1.0f), 359.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_360(360.0f), 0.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_360(720.5f), 0.5, 1e-3);
    TEST_ASSERT_NEAR(imu_wrap_360(-720.0f), 0.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(190.0f), -170.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(-180.0f), 180.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(180.0f), 180.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(-190.0f), 170.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(10.0f - 350.0f), 20.0, 1e-4);
    TEST_ASSERT_NEAR(imu_wrap_180(350.0f - 10.0f), -20.0, 1e-4);
}

static void t_lpf(void)
{
    float a1 = imu_lpf_alpha(1.0f, 0.02f);
    float a2 = imu_lpf_alpha(10.0f, 0.02f);
    float y  = 0.0f;
    int   i;

    TEST_ASSERT(a1 > 0.0f && a1 < a2 && a2 < 1.0f);
    TEST_ASSERT_NEAR(imu_lpf_alpha(0.0f, 0.02f), 1.0, 1e-6);
    for (i = 0; i < 500; i++)
    {
        y = imu_lpf_step(y, 1.0f, a1);
    }
    TEST_ASSERT_NEAR(y, 1.0, 1e-3);   /* unity DC gain */
}

static void t_align_to_z(void)
{
    int i;

    for (i = 0; i < 200; i++)
    {
        imu_vec3_t u = { (float)nrand(), (float)nrand(),
                         (float)fabs(nrand()) + 0.2f };
        imu_vec3_t r;
        imu_mat3_t m;
        float      det;

        TEST_ASSERT(imu_mat_align_to_z(&u, &m));
        (void)imu_vec_normalise(&u, &u);
        imu_mat_apply(&m, &u, &r);
        TEST_ASSERT_NEAR(r.x, 0.0, 1e-4);
        TEST_ASSERT_NEAR(r.y, 0.0, 1e-4);
        TEST_ASSERT_NEAR(r.z, 1.0, 1e-4);
        det = (m.m[0][0] * ((m.m[1][1] * m.m[2][2]) - (m.m[1][2] * m.m[2][1])))
            - (m.m[0][1] * ((m.m[1][0] * m.m[2][2]) - (m.m[1][2] * m.m[2][0])))
            + (m.m[0][2] * ((m.m[1][0] * m.m[2][1]) - (m.m[1][1] * m.m[2][0])));
        TEST_ASSERT_NEAR(det, 1.0, 1e-4);   /* proper rotation */
    }
    {
        imu_vec3_t z  = { 0.0f, 0.0f, 2.0f };
        imu_vec3_t nz = { 0.0f, 0.0f, -1.0f };
        imu_vec3_t nan_v = { NAN, 0.0f, 1.0f };
        imu_mat3_t m;

        TEST_ASSERT(imu_mat_align_to_z(&z, &m));
        TEST_ASSERT_NEAR(m.m[0][0], 1.0, 1e-6);
        TEST_ASSERT(!imu_mat_align_to_z(&nz, &m));
        TEST_ASSERT(!imu_mat_align_to_z(&nan_v, &m));
    }
}

static void t_frame_map(void)
{
    uint8_t const ax[3] = { 1U, 0U, 2U };
    int8_t const  sg[3] = { 1, -1, 1 };
    imu_vec3_t    s = { 1.0f, 2.0f, 3.0f };
    imu_vec3_t    v;

    imu_frame_map(ax, sg, &s, &v);
    TEST_ASSERT_NEAR(v.x, 2.0, 1e-6);
    TEST_ASSERT_NEAR(v.y, -1.0, 1e-6);
    TEST_ASSERT_NEAR(v.z, 3.0, 1e-6);
}

static void t_params_validate(void)
{
    imu_params_t p;

    imu_params_default(&p);
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_OK);
    TEST_ASSERT_EQ(imu_params_validate(NULL), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.axis_sign[1] = -1;                 /* mirror: left-handed */
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);              /* 90 deg yaw mount: valid */
    p.axis_map[0] = 1U; p.axis_sign[0] = 1;
    p.axis_map[1] = 0U; p.axis_sign[1] = -1;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_OK);

    imu_params_default(&p);              /* upside-down: x, -y, -z valid */
    p.axis_sign[1] = -1; p.axis_sign[2] = -1;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_OK);

    imu_params_default(&p);
    p.axis_map[1] = 0U;                  /* duplicate axis */
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.axis_sign[2] = 0;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.turn_exit_dps = p.turn_enter_dps + 1.0f;   /* inverted hysteresis */
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.impact_minor_g = NAN;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.sample_period_ms = 0U;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.turn_rate_window_ms = 1000U;       /* longer than heading history */
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.hump_min_peak_deg = p.hump_ascent_enter_deg - 0.5f;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.accel_odr_code = 0x0FU;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.wheelbase_mm = INFINITY;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);

    imu_params_default(&p);
    p.impact_critical_g = p.impact_minor_g - 0.1f;
    TEST_ASSERT_EQ(imu_params_validate(&p), IMU_ERR_PARAM);
}

static void cal_feed(imu_accel_cal_acc_t * p_acc, imu_vec3_t mean,
                     double sigma, uint32_t n)
{
    uint32_t i;

    for (i = 0U; i < n; i++)
    {
        imu_vec3_t v = { mean.x + (float)(sigma * nrand()),
                         mean.y + (float)(sigma * nrand()),
                         mean.z + (float)(sigma * nrand()) };

        imu_cal_accel_add(p_acc, &v);
    }
}

static void t_accel_cal_ok(void)
{
    imu_params_t        p;
    imu_accel_cal_acc_t acc;
    imu_accel_cal_t     cal;
    imu_vec3_t          m = { 0.0f, 0.0f, 1.02f };
    imu_vec3_t          r;

    imu_params_default(&p);
    imu_cal_accel_begin(&acc);
    cal_feed(&acc, m, 0.003, 100U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_OK);
    TEST_ASSERT(cal.b_valid);
    TEST_ASSERT_NEAR(cal.g_ref, 1.02, 0.002);
    TEST_ASSERT_NEAR(cal.noise_max_std_g, 0.003, 0.0015);
    TEST_ASSERT_NEAR(cal.mount_tilt_deg, 0.0, 0.5);
    imu_mat_apply(&cal.level_rot, &cal.gravity_mean_g, &r);
    TEST_ASSERT_NEAR(r.z, 1.02, 0.002);
}

static void t_accel_cal_mount_tilt(void)
{
    imu_params_t        p;
    imu_accel_cal_acc_t acc;
    imu_accel_cal_t     cal;
    float const         a = 10.0f * IMU_DEG_TO_RAD;
    imu_vec3_t          m = { sinf(a), 0.0f, cosf(a) };
    imu_vec3_t          r;

    imu_params_default(&p);
    imu_cal_accel_begin(&acc);
    cal_feed(&acc, m, 0.002, 100U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_OK);
    TEST_ASSERT_NEAR(cal.mount_pitch_deg, 10.0, 0.3);
    TEST_ASSERT_NEAR(cal.mount_tilt_deg, 10.0, 0.3);
    imu_mat_apply(&cal.level_rot, &m, &r);
    TEST_ASSERT_NEAR(r.x, 0.0, 0.003);
    TEST_ASSERT_NEAR(r.z, 1.0, 0.003);
}

static void t_accel_cal_failures(void)
{
    imu_params_t        p;
    imu_accel_cal_acc_t acc;
    imu_accel_cal_t     cal;
    imu_vec3_t          level = { 0.0f, 0.0f, 1.0f };
    imu_vec3_t          low   = { 0.0f, 0.0f, 0.7f };
    imu_vec3_t          tilt  = { 0.70710678f, 0.0f, 0.70710678f };
    imu_vec3_t          nan_v = { NAN, 0.0f, 1.0f };

    imu_params_default(&p);

    imu_cal_accel_begin(&acc);
    cal_feed(&acc, level, 0.05, 100U);           /* moving / handled */
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_MOTION);
    TEST_ASSERT(!cal.b_valid);

    imu_cal_accel_begin(&acc);
    cal_feed(&acc, low, 0.002, 100U);            /* wrong scale */
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_RANGE);

    imu_cal_accel_begin(&acc);
    cal_feed(&acc, tilt, 0.002, 100U);           /* 45 deg: axis map? */
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_RANGE);

    imu_cal_accel_begin(&acc);
    cal_feed(&acc, level, 0.002, 5U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_SAMPLES);

    imu_cal_accel_begin(&acc);
    imu_cal_accel_add(&acc, &nan_v);             /* ignored */
    TEST_ASSERT_EQ(acc.n, 0);
    TEST_ASSERT_EQ(imu_cal_accel_finish(NULL, &p, &cal), IMU_ERR_PARAM);

    /* Boundary: noise just below / just above tolerance. */
    imu_cal_accel_begin(&acc);
    cal_feed(&acc, level, 0.016, 400U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_OK);
    imu_cal_accel_begin(&acc);
    cal_feed(&acc, level, 0.025, 400U);
    TEST_ASSERT_EQ(imu_cal_accel_finish(&acc, &p, &cal), IMU_ERR_CAL_MOTION);
}

static void t_mag_cal(void)
{
    imu_params_t      p;
    imu_mag_cal_acc_t acc;
    imu_mag_cal_t     cal;
    int               i;
    float const       hx = 0.10f;
    float const       hy = -0.05f;
    float const       sx = 1.2f;
    float const       sy = 0.9f;

    imu_params_default(&p);
    imu_cal_mag_begin(&acc);
    for (i = 0; i < 360; i += 3)
    {
        float      a = (float)i * IMU_DEG_TO_RAD;
        imu_vec3_t m = { (0.40f * cosf(a) * sx) + hx,
                         (0.40f * sinf(a) * sy) + hy, 0.12f };

        imu_cal_mag_add(&acc, &m);
    }
    TEST_ASSERT_EQ(imu_cal_mag_finish(&acc, &p, &cal), IMU_OK);
    TEST_ASSERT_NEAR(cal.hard_iron_gauss.x, hx, 0.005);
    TEST_ASSERT_NEAR(cal.hard_iron_gauss.y, hy, 0.005);
    /* Corrected radii must be equal (circle). */
    TEST_ASSERT_NEAR(cal.soft_iron_scale.x * sx, cal.soft_iron_scale.y * sy,
                     0.02);
    TEST_ASSERT_NEAR(cal.ref_field_gauss,
                     sqrt(pow(0.4 * 1.05, 2) + (0.12 * 0.12)), 0.03);
    {
        imu_vec3_t raw = { (0.40f * sx) + hx, hy, 0.12f };
        imu_vec3_t c;

        imu_cal_mag_apply(&cal, &raw, &c);
        TEST_ASSERT_NEAR(c.y, 0.0, 0.01);
    }

    /* Partial rotation (60 deg) -> coverage error. */
    imu_cal_mag_begin(&acc);
    for (i = 0; i < 60; i++)
    {
        float      a = (float)i * IMU_DEG_TO_RAD;
        imu_vec3_t m = { 0.40f * cosf(a), 0.40f * sinf(a), 0.12f };

        imu_cal_mag_add(&acc, &m);
    }
    TEST_ASSERT_EQ(imu_cal_mag_finish(&acc, &p, &cal), IMU_ERR_CAL_COVERAGE);
    TEST_ASSERT(!cal.b_valid);

    imu_cal_mag_begin(&acc);
    TEST_ASSERT_EQ(imu_cal_mag_finish(&acc, &p, &cal), IMU_ERR_CAL_SAMPLES);
}

void suite_unit_math_cal(void)
{
    test_section("UNIT (white-box): math, parameters, calibration");
    RUN_TEST(t_wrap);
    RUN_TEST(t_lpf);
    RUN_TEST(t_align_to_z);
    RUN_TEST(t_frame_map);
    RUN_TEST(t_params_validate);
    RUN_TEST(t_accel_cal_ok);
    RUN_TEST(t_accel_cal_mount_tilt);
    RUN_TEST(t_accel_cal_failures);
    RUN_TEST(t_mag_cal);
}
```

## `tests/imu/test_unit_processing.c`

```c
/**
 * @file    test_unit_processing.c
 * @brief   Unit tests: attitude, heading, turn rate, turn/tilt detectors,
 *          impact detector, motion classifier, encoder hint handling.
 */

#include <string.h>

#include "imu/imu_math.h"
#include "imu/imu_processing.h"
#include "test_util.h"

#define BH   (0.40f)
#define BUP  (0.12f)

static imu_params_t g_p;
static imu_proc_t   g_proc;
static uint32_t     g_t;

static void proc_setup(void)
{
    imu_accel_cal_t ac;
    imu_mag_cal_t   mc;

    imu_params_default(&g_p);
    TEST_ASSERT_EQ(imu_proc_init(&g_proc, &g_p), IMU_OK);
    (void)memset(&ac, 0, sizeof(ac));
    imu_mat_identity(&ac.level_rot);
    ac.g_ref   = 1.0f;
    ac.b_valid = true;
    (void)memset(&mc, 0, sizeof(mc));
    mc.b_valid = true;
    mc.soft_iron_scale.x = 1.0f;
    mc.soft_iron_scale.y = 1.0f;
    mc.soft_iron_scale.z = 1.0f;
    mc.ref_field_gauss = sqrtf((BH * BH) + (BUP * BUP));
    imu_proc_set_calibration(&g_proc, &ac, &mc);
    g_t = 1000U;
}

static imu_vec3_t mag_for(float heading_deg)
{
    float      a = heading_deg * IMU_DEG_TO_RAD;
    imu_vec3_t m = { BH * cosf(a), BH * sinf(a), BUP };

    return (m);
}

static void step(imu_vec3_t acc, imu_vec3_t const * p_mag, bool b_hump,
                 imu_proc_out_t * p_out)
{
    g_t += g_p.sample_period_ms;
    imu_proc_step(&g_proc, &acc, p_mag, g_t, b_hump, p_out);
}

static imu_vec3_t level(void)
{
    imu_vec3_t a = { 0.0f, 0.0f, 1.0f };

    return (a);
}

static void t_pitch_roll_sign(void)
{
    imu_proc_out_t out;
    imu_vec3_t     m = mag_for(0.0f);
    float          a = 10.0f * IMU_DEG_TO_RAD;
    imu_vec3_t     nose_up = { sinf(a), 0.0f, cosf(a) };
    imu_vec3_t     left_up = { 0.0f, sinf(a), cosf(a) };
    int            i;

    proc_setup();
    for (i = 0; i < 50; i++)
    {
        step(nose_up, &m, false, &out);
    }
    TEST_ASSERT_NEAR(out.pitch_deg, 10.0, 0.05);
    TEST_ASSERT_NEAR(out.roll_deg, 0.0, 0.05);
    TEST_ASSERT_NEAR(out.tilt_deg, 10.0, 0.05);
    TEST_ASSERT(out.b_attitude_reliable);

    proc_setup();
    for (i = 0; i < 50; i++)
    {
        step(left_up, &m, false, &out);
    }
    TEST_ASSERT_NEAR(out.roll_deg, 10.0, 0.05);
    TEST_ASSERT_NEAR(out.pitch_deg, 0.0, 0.05);
}

static void t_attitude_reliability(void)
{
    imu_proc_out_t out;
    imu_vec3_t     heavy = { 0.0f, 0.0f, 1.3f };
    int            i;

    proc_setup();
    for (i = 0; i < 30; i++)
    {
        step(heavy, NULL, false, &out);
    }
    TEST_ASSERT(!out.b_attitude_reliable);
    TEST_ASSERT_NEAR(out.vert_accel_g, 0.3, 0.01);
}

static void t_heading_cardinal(void)
{
    float const hdg[] = { 0.0f, 45.0f, 90.0f, 135.0f, 180.0f, 225.0f,
                          270.0f, 315.0f, 359.0f };
    uint32_t    k;

    for (k = 0U; k < (sizeof(hdg) / sizeof(hdg[0])); k++)
    {
        imu_proc_out_t out;
        imu_vec3_t     m = mag_for(hdg[k]);
        int            i;

        proc_setup();
        for (i = 0; i < 20; i++)
        {
            step(level(), &m, false, &out);
        }
        TEST_ASSERT_NEAR(imu_wrap_180(out.heading_deg - hdg[k]), 0.0, 0.5);
        TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_GOOD);
    }
}

static void t_heading_tilt_compensated(void)
{
    scenario_t s;
    imu_vec3_t a;
    imu_vec3_t m;
    imu_proc_out_t out;
    int        i;

    proc_setup();
    scn_init(&s);
    s.accel_noise_g   = 0.0;
    s.mag_noise_gauss = 0.0;
    s.heading0_deg    = 60.0;
    s.heading_deg     = 60.0;
    s.bh_gauss        = BH;
    s.bup_gauss       = BUP;
    scn_add_segment(&s, 0.0, 100.0, 0.0, 0.0, 0.0);
    s.seg[0].pitch_deg = 15.0;
    s.seg[0].roll_deg  = 10.0;

    for (i = 0; i < 40; i++)
    {
        scn_stimulus(&s, 0.02 * i, &a, &m);
        step(a, &m, false, &out);
    }
    TEST_ASSERT_NEAR(out.pitch_deg, 15.0, 0.2);
    TEST_ASSERT_NEAR(out.roll_deg, 10.0, 0.3);
    TEST_ASSERT_NEAR(imu_wrap_180(out.heading_deg - 60.0f), 0.0, 1.0);
}

/** Drive the heading at a constant rate; returns the last output. */
static void spin(float h0, float rate_dps, uint32_t ms, imu_proc_out_t * p_out,
                 uint32_t * p_starts, uint32_t * p_ends, float * p_delta)
{
    uint32_t t;
    float    h = h0;

    for (t = 0U; t < ms; t += g_p.sample_period_ms)
    {
        imu_vec3_t m = mag_for(imu_wrap_360(h));

        step(level(), &m, false, p_out);
        h += rate_dps * ((float)g_p.sample_period_ms * 0.001f);
        if (p_out->b_turn_event && (IMU_PHASE_START == p_out->turn_phase))
        {
            (*p_starts)++;
        }
        if (p_out->b_turn_event && (IMU_PHASE_END == p_out->turn_phase))
        {
            (*p_ends)++;
            *p_delta = p_out->turn_event_delta_deg;
        }
    }
}

static void t_turn_rate_wraparound(void)
{
    imu_proc_out_t out;
    uint32_t       s = 0U;
    uint32_t       e = 0U;
    float          d = 0.0f;

    proc_setup();
    spin(340.0f, 0.0f, 400U, &out, &s, &e, &d);
    spin(340.0f, 50.0f, 1000U, &out, &s, &e, &d);   /* crosses 360 -> 0 */
    TEST_ASSERT(out.b_turn_rate_valid);
    TEST_ASSERT_NEAR(out.turn_rate_dps, 50.0, 6.0);
    TEST_ASSERT(out.heading_deg < 40.0f);
}

static void t_turn_left_right(void)
{
    imu_proc_out_t out;
    uint32_t       s = 0U;
    uint32_t       e = 0U;
    float          d = 0.0f;

    /* LEFT: heading decreases. 90 deg at 60 deg/s. */
    proc_setup();
    spin(100.0f, 0.0f, 500U, &out, &s, &e, &d);
    spin(100.0f, -60.0f, 1500U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(g_proc.turn_state, IMU_TURN_LEFT);
    TEST_ASSERT_EQ(s, 1);
    spin(10.0f, 0.0f, 1000U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(e, 1);
    TEST_ASSERT_NEAR(d, -90.0, 12.0);
    TEST_ASSERT_EQ(g_proc.turn_state, IMU_TURN_NONE);

    /* RIGHT */
    proc_setup();
    s = 0U; e = 0U;
    spin(200.0f, 0.0f, 500U, &out, &s, &e, &d);
    spin(200.0f, 60.0f, 1500U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(g_proc.turn_state, IMU_TURN_RIGHT);
    spin(290.0f, 0.0f, 1000U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(e, 1);
    TEST_ASSERT_NEAR(d, 90.0, 12.0);
}

static void t_turn_debounce(void)
{
    imu_proc_out_t out;
    uint32_t       s = 0U;
    uint32_t       e = 0U;
    float          d = 0.0f;

    /* 100 ms wiggle at high rate: shorter than turn_min_duration. */
    proc_setup();
    spin(0.0f, 0.0f, 500U, &out, &s, &e, &d);
    spin(0.0f, 80.0f, 100U, &out, &s, &e, &d);
    spin(8.0f, -80.0f, 100U, &out, &s, &e, &d);
    spin(0.0f, 0.0f, 800U, &out, &s, &e, &d);
    TEST_ASSERT_EQ(s, 0);

    /* Slow drift below the turn threshold -> heading-change events. */
    proc_setup();
    {
        uint32_t hc = 0U;
        uint32_t t;
        float    h = 0.0f;

        for (t = 0U; t < 5000U; t += 20U)
        {
            imu_vec3_t m = mag_for(imu_wrap_360(h));

            step(level(), &m, false, &out);
            h += 10.0f * 0.02f;
            if (out.b_turn_event)
            {
                TEST_ASSERT_EQ(out.turn_phase, IMU_PHASE_HEADING_CHANGE);
                hc++;
            }
        }
        TEST_ASSERT(hc >= 2U && hc <= 3U);   /* 50 deg / 20 deg */
    }
}

static void t_heading_disturbed(void)
{
    imu_proc_out_t out;
    uint32_t       starts = 0U;
    uint32_t       t;
    float          h = 0.0f;

    proc_setup();
    for (t = 0U; t < 3000U; t += 20U)
    {
        imu_vec3_t m = mag_for(imu_wrap_360(h));

        if ((t > 1000U) && (t < 2000U))
        {
            m.x += 0.5f;    /* steel / motor field: |B| doubles */
        }
        step(level(), &m, false, &out);
        if ((t > 1100U) && (t < 1900U))
        {
            TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_DISTURBED);
            TEST_ASSERT(!out.b_turn_rate_valid);
        }
        if (out.b_turn_event && (IMU_PHASE_START == out.turn_phase))
        {
            starts++;
        }
    }
    TEST_ASSERT_EQ(starts, 0);
    TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_GOOD);
}

static void t_heading_outlier(void)
{
    imu_proc_out_t out;
    imu_vec3_t     m0 = mag_for(30.0f);
    imu_vec3_t     m1 = mag_for(150.0f);
    int            i;

    proc_setup();
    for (i = 0; i < 30; i++)
    {
        step(level(), &m0, false, &out);
    }
    step(level(), &m1, false, &out);        /* single glitch */
    TEST_ASSERT_NEAR(out.heading_deg, 30.0, 1.0);
    for (i = 0; i < 10; i++)
    {
        step(level(), &m1, false, &out);     /* persistent -> accepted */
    }
    TEST_ASSERT_NEAR(out.heading_deg, 150.0, 3.0);
    step(level(), NULL, false, &out);
    TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_INVALID);
}

static void t_uncalibrated_mag(void)
{
    imu_proc_out_t out;
    imu_vec3_t     m = mag_for(10.0f);
    int            i;

    imu_params_default(&g_p);
    TEST_ASSERT_EQ(imu_proc_init(&g_proc, &g_p), IMU_OK);
    for (i = 0; i < 10; i++)
    {
        step(level(), &m, false, &out);
    }
    TEST_ASSERT_EQ(out.heading_quality, IMU_HEADING_UNCALIBRATED);
}

static void tilt_run(imu_vec3_t a, uint32_t ms, bool b_hump,
                     uint32_t * p_start, uint32_t * p_end,
                     imu_severity_t * p_sev)
{
    imu_proc_out_t out;
    imu_vec3_t     m = mag_for(0.0f);
    uint32_t       t;

    for (t = 0U; t < ms; t += g_p.sample_period_ms)
    {
        step(a, &m, b_hump, &out);
        if (out.b_tilt_event)
        {
            if (IMU_PHASE_END == out.tilt_phase)
            {
                (*p_end)++;
            }
            else
            {
                (*p_start)++;
                *p_sev = out.tilt_severity;
            }
        }
    }
}

static imu_vec3_t tilt_vec(float pitch_deg, float roll_deg)
{
    float      p = pitch_deg * IMU_DEG_TO_RAD;
    float      r = roll_deg * IMU_DEG_TO_RAD;
    imu_vec3_t a = { sinf(p), cosf(p) * sinf(r), cosf(p) * cosf(r) };

    return (a);
}

static void t_tilt_detector(void)
{
    uint32_t       s = 0U;
    uint32_t       e = 0U;
    imu_severity_t sev = IMU_SEV_INFO;

    proc_setup();
    tilt_run(level(), 500U, false, &s, &e, &sev);
    tilt_run(tilt_vec(0.0f, 20.0f), 200U, false, &s, &e, &sev); /* short */
    TEST_ASSERT_EQ(s, 0);
    tilt_run(tilt_vec(0.0f, 20.0f), 600U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 1);
    TEST_ASSERT_EQ(sev, IMU_SEV_MINOR);
    /* Hysteresis: 13 deg is below threshold but above threshold - hyst. */
    tilt_run(tilt_vec(0.0f, 13.0f), 1000U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(e, 0);
    tilt_run(tilt_vec(0.0f, 40.0f), 200U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 2);                 /* escalation to critical */
    TEST_ASSERT_EQ(sev, IMU_SEV_CRITICAL);
    tilt_run(level(), 1000U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(e, 1);

    /* Critical tilt is reported immediately (no debounce). */
    proc_setup();
    s = 0U; e = 0U;
    tilt_run(level(), 300U, false, &s, &e, &sev);
    tilt_run(tilt_vec(0.0f, 40.0f), 100U, false, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 1);
    TEST_ASSERT_EQ(sev, IMU_SEV_CRITICAL);

    /* Pitch during a hump is expected: suppressed; roll is not. */
    proc_setup();
    s = 0U; e = 0U;
    tilt_run(level(), 300U, true, &s, &e, &sev);
    tilt_run(tilt_vec(20.0f, 0.0f), 1000U, true, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 0);
    tilt_run(tilt_vec(5.0f, 20.0f), 1000U, true, &s, &e, &sev);
    TEST_ASSERT_EQ(s, 1);
}

static void hf_pulse(float peak, imu_vec3_t dir, uint32_t dur_ms,
                     bool b_hump, imu_impact_t * p_last, uint32_t * p_n)
{
    uint32_t t;

    for (t = 0U; t < (dur_ms + 50U); t += 5U)
    {
        float        k = (t < dur_ms) ? peak * sinf(IMU_PI_F * (float)t
                                                     / (float)dur_ms) : 0.0f;
        imu_vec3_t   a = { dir.x * k, dir.y * k, 1.0f + (dir.z * k) };
        imu_impact_t imp;

        g_t += 5U;
        imu_proc_hf_accel(&g_proc, &a, g_t, b_hump, &imp);
        if (imp.b_detected)
        {
            *p_last = imp;
            (*p_n)++;
        }
    }
}

static void hf_idle(uint32_t ms)
{
    uint32_t     t;
    imu_vec3_t   a = { 0.0f, 0.0f, 1.0f };
    imu_impact_t imp;

    for (t = 0U; t < ms; t += 5U)
    {
        g_t += 5U;
        imu_proc_hf_accel(&g_proc, &a, g_t, false, &imp);
    }
}

static void t_impact_detector(void)
{
    imu_vec3_t const fwd  = { -1.0f, 0.0f, 0.0f };
    imu_vec3_t const vert = { 0.0f, 0.0f, 1.0f };
    imu_impact_t     last;
    uint32_t         n = 0U;

    proc_setup();
    hf_pulse(3.0f, fwd, 20U, false, &last, &n);       /* during warm-up */
    TEST_ASSERT_EQ(n, 0);
    hf_idle(1000U);

    hf_pulse(2.5f, fwd, 20U, false, &last, &n);        /* frontal crash */
    TEST_ASSERT_EQ(n, 1);
    TEST_ASSERT_EQ(last.severity, IMU_SEV_CRITICAL);
    TEST_ASSERT(last.horiz_g > (0.8f * last.peak_g));

    hf_idle(1500U);
    hf_pulse(1.6f, vert, 20U, false, &last, &n);       /* vertical knock */
    TEST_ASSERT_EQ(n, 2);
    TEST_ASSERT_EQ(last.severity, IMU_SEV_MINOR);

    hf_idle(1500U);
    hf_pulse(0.6f, fwd, 20U, false, &last, &n);        /* vibration */
    TEST_ASSERT_EQ(n, 2);

    hf_idle(1500U);
    hf_pulse(1.5f, fwd, 400U, false, &last, &n);       /* slow push */
    TEST_ASSERT_EQ(n, 2);

    /* Sustained step (e.g. pushed against a wall, harsh braking): the
     * high-pass excursion lasts longer than an impact -> rejected. */
    hf_idle(1500U);
    {
        uint32_t     t;
        imu_impact_t imp;

        for (t = 0U; t < 400U; t += 5U)
        {
            imu_vec3_t a = { -3.0f, 0.0f, 1.0f };

            g_t += 5U;
            imu_proc_hf_accel(&g_proc, &a, g_t, false, &imp);
            if (imp.b_detected)
            {
                n++;
            }
        }
        hf_idle(500U);
    }
    TEST_ASSERT_EQ(n, 2);
    TEST_ASSERT(g_proc.imp_rejected_long >= 1U);

    hf_idle(1500U);
    hf_pulse(1.4f, vert, 20U, true, &last, &n);        /* hump: scaled */
    TEST_ASSERT_EQ(n, 2);
    hf_pulse(2.2f, vert, 20U, true, &last, &n);
    TEST_ASSERT_EQ(n, 3);
    TEST_ASSERT_EQ(last.severity, IMU_SEV_MINOR);      /* vertical */
}

static void t_impact_cooldown(void)
{
    imu_vec3_t const side = { 0.0f, 1.0f, 0.0f };
    imu_vec3_t const fwd  = { -1.0f, 0.0f, 0.0f };
    imu_impact_t     last;
    uint32_t         n = 0U;
    int              i;

    proc_setup();
    hf_idle(1000U);
    for (i = 0; i < 5; i++)
    {
        hf_pulse(1.5f, side, 15U, false, &last, &n);   /* minor x5 */
        hf_idle(50U);
    }
    TEST_ASSERT_EQ(n, 1);
    TEST_ASSERT(g_proc.imp_suppressed >= 4U);

    hf_pulse(3.5f, fwd, 15U, false, &last, &n);        /* escalation */
    TEST_ASSERT_EQ(n, 2);
    TEST_ASSERT_EQ(last.severity, IMU_SEV_CRITICAL);
    TEST_ASSERT(last.suppressed >= 4U);

    hf_idle(1200U);
    hf_pulse(1.5f, side, 15U, false, &last, &n);       /* after cooldown */
    TEST_ASSERT_EQ(n, 3);
}

static void classify_run(imu_vec3_t a, uint32_t ms, imu_hump_phase_t hp,
                         imu_proc_out_t * p_out, uint32_t * p_changes)
{
    uint32_t   t;
    imu_vec3_t m = mag_for(0.0f);

    for (t = 0U; t < ms; t += g_p.sample_period_ms)
    {
        step(a, &m, false, p_out);
        imu_proc_classify(&g_proc, g_t, hp, false, p_out);
        if (p_out->b_motion_changed)
        {
            (*p_changes)++;
        }
    }
}

static void t_classifier(void)
{
    imu_proc_out_t    out;
    uint32_t          ch = 0U;
    imu_vec3_t const  acc = { 0.10f, 0.0f, 1.0f };
    imu_vec3_t const  dec = { -0.10f, 0.0f, 1.0f };
    imu_vec3_t const  mid = { 0.045f, 0.0f, 1.0f };
    imu_motion_hint_t h;

    proc_setup();
    classify_run(level(), 500U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_STATIONARY);
    TEST_ASSERT_EQ(out.motion_conf, IMU_CONF_LOW);   /* inferred */

    classify_run(acc, 500U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_ACCELERATING);
    classify_run(mid, 500U, IMU_HUMP_MONITORING, &out, &ch);   /* hyst */
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_ACCELERATING);
    classify_run(dec, 500U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_DECELERATING);
    classify_run(level(), 500U, IMU_HUMP_ASCENT, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_CLIMBING_HUMP);
    classify_run(level(), 100U, IMU_HUMP_DESCENT, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_DESCENDING_HUMP);

    /* With an encoder hint the stationary/steady decision is measured. */
    h.b_valid = true;
    h.b_accel_valid = true;
    h.accel_mm_s2 = 0;
    h.speed_mm_s = 300;
    h.timestamp_ms = g_t;
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_OK);
    classify_run(level(), 100U, IMU_HUMP_MONITORING, &out, &ch);
    h.timestamp_ms = g_t;
    (void)imu_proc_set_hint(&g_proc, &h);
    classify_run(level(), 100U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_STEADY_MOTION);
    TEST_ASSERT_EQ(out.motion_conf, IMU_CONF_HIGH);

    h.speed_mm_s = 0;
    h.timestamp_ms = g_t;
    (void)imu_proc_set_hint(&g_proc, &h);
    classify_run(level(), 180U, IMU_HUMP_MONITORING, &out, &ch);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_STATIONARY);
    TEST_ASSERT_EQ(out.motion_conf, IMU_CONF_HIGH);

    /* Impact overrides and holds. */
    imu_proc_classify(&g_proc, g_t, IMU_HUMP_MONITORING, true, &out);
    TEST_ASSERT_EQ(out.motion, IMU_MOTION_SUDDEN_IMPACT);
}

static void t_classifier_no_flapping(void)
{
    imu_proc_out_t out;
    uint32_t       ch = 0U;
    uint32_t       t;
    uint32_t       rng = 7U;
    imu_vec3_t     m = mag_for(0.0f);

    proc_setup();
    classify_run(level(), 300U, IMU_HUMP_MONITORING, &out, &ch);
    ch = 0U;
    /* Noise around the accel threshold (0.06 g) +/- 0.03 g every cycle. */
    for (t = 0U; t < 5000U; t += 20U)
    {
        float      n;
        imu_vec3_t a;

        rng = (rng * 1664525U) + 1013904223U;
        n = (((float)(rng >> 8) / 16777216.0f) - 0.5f) * 0.06f;
        a.x = 0.06f + n;
        a.y = 0.0f;
        a.z = 1.0f;
        step(a, &m, false, &out);
        imu_proc_classify(&g_proc, g_t, IMU_HUMP_MONITORING, false, &out);
        if (out.b_motion_changed)
        {
            ch++;
        }
    }
    TEST_ASSERT_MSG(ch <= 6U, "changes=%u", (unsigned)ch);
}

static void t_hint_validation(void)
{
    imu_motion_hint_t h;
    imu_proc_out_t    out;
    int               i;

    proc_setup();
    (void)memset(&h, 0, sizeof(h));
    TEST_ASSERT_EQ(imu_proc_set_hint(NULL, &h), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, NULL), IMU_ERR_PARAM);

    h.b_valid = true;
    h.speed_mm_s = 1000000;
    h.timestamp_ms = g_t;
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(g_proc.hints_rejected, 1);
    TEST_ASSERT(!imu_proc_hint_fresh(&g_proc, g_t));

    h.speed_mm_s = 100;
    h.b_accel_valid = true;
    h.accel_mm_s2 = -900000;
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_ERR_PARAM);

    h.b_accel_valid = false;
    h.timestamp_ms = g_t + 5000U;            /* from the future */
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_OK);
    TEST_ASSERT(!imu_proc_hint_fresh(&g_proc, g_t));

    h.timestamp_ms = g_t - 1000U;            /* stale */
    TEST_ASSERT_EQ(imu_proc_set_hint(&g_proc, &h), IMU_OK);
    TEST_ASSERT(!imu_proc_hint_fresh(&g_proc, g_t));

    /* Derived acceleration from successive speeds: 0 -> 98 mm/s in 100 ms
     * = 0.98 m/s^2 = 0.1 g. */
    h.b_valid = true;
    h.speed_mm_s = 0;
    h.timestamp_ms = g_t;
    (void)imu_proc_set_hint(&g_proc, &h);
    h.speed_mm_s = 98;
    h.timestamp_ms = g_t + 100U;
    (void)imu_proc_set_hint(&g_proc, &h);
    TEST_ASSERT(g_proc.b_hint_accel_valid);
    TEST_ASSERT_NEAR(g_proc.hint_accel_g, 0.1, 0.005);

    /* Pitch compensation: 0.1 g forward accel no longer looks like 5.7 deg */
    {
        imu_vec3_t a = { 0.10f, 0.0f, 1.0f };
        imu_vec3_t m = mag_for(0.0f);

        h.b_accel_valid = true;
        h.accel_mm_s2 = 981;
        for (i = 0; i < 30; i++)
        {
            h.timestamp_ms = g_t;
            (void)imu_proc_set_hint(&g_proc, &h);
            step(a, &m, false, &out);
        }
        TEST_ASSERT_NEAR(out.pitch_deg, 0.0, 0.2);
        TEST_ASSERT(out.b_fwd_from_hint);
    }
}

static void t_nan_input(void)
{
    imu_proc_out_t out;
    imu_vec3_t     bad = { NAN, 0.0f, 1.0f };
    imu_vec3_t     inf = { 0.0f, INFINITY, 1.0f };
    imu_impact_t   imp;

    proc_setup();
    step(level(), NULL, false, &out);
    step(bad, &bad, false, &out);
    step(inf, &inf, false, &out);
    imu_proc_hf_accel(&g_proc, &bad, g_t, false, &imp);
    TEST_ASSERT(!imp.b_detected);
    step(level(), NULL, false, &out);
    TEST_ASSERT(0 != isfinite(out.pitch_deg));
    TEST_ASSERT(0 != isfinite(g_proc.accel_lp.x));
    TEST_ASSERT(0 != isfinite(g_proc.heading_f_deg));
}

void suite_unit_processing(void)
{
    test_section("UNIT (white-box): processing");
    RUN_TEST(t_pitch_roll_sign);
    RUN_TEST(t_attitude_reliability);
    RUN_TEST(t_heading_cardinal);
    RUN_TEST(t_heading_tilt_compensated);
    RUN_TEST(t_turn_rate_wraparound);
    RUN_TEST(t_turn_left_right);
    RUN_TEST(t_turn_debounce);
    RUN_TEST(t_heading_disturbed);
    RUN_TEST(t_heading_outlier);
    RUN_TEST(t_uncalibrated_mag);
    RUN_TEST(t_tilt_detector);
    RUN_TEST(t_impact_detector);
    RUN_TEST(t_impact_cooldown);
    RUN_TEST(t_classifier);
    RUN_TEST(t_classifier_no_flapping);
    RUN_TEST(t_hint_validation);
    RUN_TEST(t_nan_input);
}
```

## `tests/imu/test_unit_hump.c`

```c
/**
 * @file    test_unit_hump.c
 * @brief   Unit tests: hump state machine and measurement (direct input).
 */

#include <string.h>

#include "imu/imu_hump.h"
#include "imu/imu_math.h"
#include "test_util.h"

static imu_params_t g_p;
static imu_hump_t   g_hump;
static uint32_t     g_t;

typedef struct
{
    uint32_t detected;
    uint32_t peaks;
    uint32_t completed;
    uint32_t rejected;
    uint32_t new_max;
    imu_hump_meas_t last_done;
    imu_hump_validity_t last_reject;
    imu_hump_phase_t    seq[8];
    uint32_t            n_seq;
} hump_log_t;

static hump_log_t g_log;

static void hump_setup(void)
{
    imu_params_default(&g_p);
    TEST_ASSERT_EQ(imu_hump_init(&g_hump, &g_p), IMU_OK);
    (void)memset(&g_log, 0, sizeof(g_log));
    g_t = 10000U;
}

static void feed(float pitch, bool b_speed, float speed)
{
    imu_hump_input_t in;
    imu_hump_out_t   out;

    (void)memset(&in, 0, sizeof(in));
    in.pitch_deg           = pitch;
    in.b_attitude_reliable = true;
    in.b_speed_valid       = b_speed;
    in.speed_mm_s          = speed;
    g_t += g_p.sample_period_ms;
    imu_hump_step(&g_hump, g_t, &in, &out);

    if ((0U == g_log.n_seq) || (g_log.seq[g_log.n_seq - 1U] != out.phase))
    {
        if (g_log.n_seq < 8U)
        {
            g_log.seq[g_log.n_seq] = out.phase;
            g_log.n_seq++;
        }
    }
    if (out.b_detected)  { g_log.detected++; }
    if (out.b_peak)      { g_log.peaks++; }
    if (out.b_completed) { g_log.completed++; g_log.last_done = out.meas; }
    if (out.b_rejected)  { g_log.rejected++;
                           g_log.last_reject = out.meas.validity; }
    if (out.b_new_max)   { g_log.new_max++; }
}

static void flat(uint32_t ms)
{
    uint32_t t;

    for (t = 0U; t < ms; t += g_p.sample_period_ms)
    {
        feed(0.0f, false, 0.0f);
    }
}

/** Positive lobe, level gap, negative lobe (time-scaled by @p k). */
static void hump(float peak, float trough, float k)
{
    uint32_t const up   = (uint32_t)(400.0f * k);
    uint32_t const gap  = (uint32_t)(100.0f * k);
    uint32_t const down = (uint32_t)(400.0f * k);
    uint32_t       t;

    for (t = 0U; t < up; t += g_p.sample_period_ms)
    {
        feed(peak * sinf(IMU_PI_F * (float)t / (float)up), false, 0.0f);
    }
    for (t = 0U; t < gap; t += g_p.sample_period_ms)
    {
        feed(0.0f, false, 0.0f);
    }
    for (t = 0U; t < down; t += g_p.sample_period_ms)
    {
        feed(-trough * sinf(IMU_PI_F * (float)t / (float)down), false, 0.0f);
    }
    flat(400U);
}

static void t_canonical(void)
{
    hump_setup();
    flat(500U);
    hump(6.0f, 6.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.detected, 1);
    TEST_ASSERT_EQ(g_log.peaks, 1);
    TEST_ASSERT_EQ(g_log.completed, 1);
    TEST_ASSERT_EQ(g_log.rejected, 0);
    TEST_ASSERT_NEAR(g_log.last_done.peak_pitch_deg, 6.0, 0.1);
    TEST_ASSERT(g_log.last_done.min_pitch_deg < -5.5f);
    TEST_ASSERT_EQ(g_log.last_done.index, 1);
    TEST_ASSERT_EQ(g_log.last_done.validity, IMU_HUMP_VALID);
    /* Sequence: MONITORING -> ASCENT -> PEAK -> DESCENT -> MONITORING */
    TEST_ASSERT_EQ(g_log.n_seq, 5);
    TEST_ASSERT_EQ(g_log.seq[1], IMU_HUMP_ASCENT);
    TEST_ASSERT_EQ(g_log.seq[2], IMU_HUMP_PEAK);
    TEST_ASSERT_EQ(g_log.seq[3], IMU_HUMP_DESCENT);
    TEST_ASSERT_EQ(g_log.seq[4], IMU_HUMP_MONITORING);
}

static void t_boundaries(void)
{
    /* Just above the minimum valid peak. */
    hump_setup();
    flat(500U);
    hump(3.8f, 3.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 1);

    /* Crosses the ascent threshold but stays below min valid peak. */
    hump_setup();
    flat(500U);
    hump(3.3f, 3.3f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 0);
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.last_reject, IMU_HUMP_REJ_LOW_PEAK);

    /* Below the ascent threshold: invisible. */
    hump_setup();
    flat(500U);
    hump(2.8f, 2.8f, 1.0f);
    TEST_ASSERT_EQ(g_log.detected, 0);

    /* Descent too shallow: no valid descent. */
    hump_setup();
    flat(500U);
    hump(6.0f, 1.8f, 1.0f);
    flat(1500U);                      /* crest timeout elapses */
    TEST_ASSERT_EQ(g_log.completed, 0);
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.last_reject, IMU_HUMP_REJ_NO_DESCENT);
}

static void t_spike_rejected(void)
{
    hump_setup();
    flat(500U);
    feed(12.0f, false, 0.0f);      /* 20 ms spike */
    feed(-12.0f, false, 0.0f);
    flat(500U);
    feed(9.0f, false, 0.0f);
    feed(9.0f, false, 0.0f);       /* 40 ms < ascent_min 60 ms */
    flat(500U);
    TEST_ASSERT_EQ(g_log.detected, 0);
    TEST_ASSERT_EQ(g_log.completed, 0);
}

static void t_slope_and_no_descent(void)
{
    uint32_t t;

    /* Continuous ramp: never peaks -> rejected as SLOPE. */
    hump_setup();
    flat(500U);
    for (t = 0U; t < 5000U; t += 20U)
    {
        feed(5.0f, false, 0.0f);
    }
    TEST_ASSERT_EQ(g_log.completed, 0);
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.last_reject, IMU_HUMP_REJ_SLOPE);

    /* Acceleration artefact: up then flat, no nose-down -> NO_DESCENT. */
    hump_setup();
    flat(500U);
    for (t = 0U; t < 500U; t += 20U)
    {
        feed(4.5f, false, 0.0f);
    }
    flat(3000U);
    TEST_ASSERT_EQ(g_log.completed, 0);
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.last_reject, IMU_HUMP_REJ_NO_DESCENT);
}

static void t_artefact_then_real_hump(void)
{
    uint32_t t;

    hump_setup();
    flat(500U);
    for (t = 0U; t < 500U; t += 20U)
    {
        feed(4.5f, false, 0.0f);      /* artefact */
    }
    flat(300U);
    hump(7.0f, 7.0f, 1.0f);           /* real hump right behind it */
    TEST_ASSERT_EQ(g_log.rejected, 1);
    TEST_ASSERT_EQ(g_log.completed, 1);
    TEST_ASSERT_NEAR(g_log.last_done.peak_pitch_deg, 7.0, 0.2);
}

static void t_multiple_retain_max(void)
{
    hump_setup();
    flat(500U);
    hump(5.0f, 5.0f, 1.0f);
    flat(200U);
    hump(9.0f, 9.0f, 1.0f);
    flat(200U);
    hump(7.0f, 7.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 3);
    TEST_ASSERT_EQ(g_log.new_max, 2);
    TEST_ASSERT_EQ(g_hump.stats.valid_count, 3);
    TEST_ASSERT(g_hump.stats.b_has_max);
    TEST_ASSERT_NEAR(g_hump.stats.max.peak_pitch_deg, 9.0, 0.1);
    TEST_ASSERT_EQ(g_hump.stats.max.index, 2);
    TEST_ASSERT_NEAR(g_hump.stats.last.peak_pitch_deg, 7.0, 0.1);
    TEST_ASSERT_EQ(g_hump.stats.last.index, 3);

    /* Equal peak: maximum keeps the first occurrence (stable). */
    flat(200U);
    hump(9.0f, 9.0f, 1.0f);
    TEST_ASSERT_EQ(g_hump.stats.max.index, 2);
}

static void t_no_double_count(void)
{
    uint32_t t;

    hump_setup();
    flat(500U);
    hump(6.0f, 6.0f, 1.0f);
    /* Wobble after the hump (suspension bounce) below the threshold, and
     * one above-threshold rebound shorter than the re-arm window. */
    for (t = 0U; t < 600U; t += 20U)
    {
        feed(2.0f * sinf((float)t * 0.05f), false, 0.0f);
    }
    TEST_ASSERT_EQ(g_log.completed, 1);
    TEST_ASSERT_EQ(g_log.detected, 1);
}

static void t_rearm(void)
{
    /* Second hump closer than the minimum gap is not counted twice;
     * a properly separated one is. */
    hump_setup();
    flat(500U);
    hump(6.0f, 6.0f, 1.0f);           /* ends with 400 ms flat */
    flat(400U);
    hump(6.0f, 6.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 2);
}

static void t_speed_variation(void)
{
    float const k[] = { 0.25f, 0.5f, 1.0f, 2.0f, 3.0f };
    uint32_t    i;

    for (i = 0U; i < (sizeof(k) / sizeof(k[0])); i++)
    {
        hump_setup();
        flat(500U);
        hump(8.0f, 8.0f, k[i]);
        TEST_ASSERT_MSG(1U == g_log.completed, "time scale %.2f", (double)k[i]);
        TEST_ASSERT_NEAR(g_log.last_done.peak_pitch_deg, 8.0, 0.6);
    }

    /* Far too slow (8 s traversal) exceeds the max event duration. */
    hump_setup();
    flat(500U);
    hump(8.0f, 8.0f, 8.0f);
    TEST_ASSERT_EQ(g_log.completed, 0);
}

static void t_reset_and_abort(void)
{
    uint32_t t;

    hump_setup();
    flat(500U);
    hump(6.0f, 6.0f, 1.0f);
    TEST_ASSERT_EQ(g_hump.stats.valid_count, 1);
    imu_hump_reset_mission(&g_hump);
    TEST_ASSERT_EQ(g_hump.stats.valid_count, 0);
    TEST_ASSERT(!g_hump.stats.b_has_max);
    TEST_ASSERT(!g_hump.stats.b_has_last);

    flat(500U);
    for (t = 0U; t < 200U; t += 20U)
    {
        feed(6.0f, false, 0.0f);
    }
    TEST_ASSERT_EQ(g_hump.phase, IMU_HUMP_ASCENT);
    imu_hump_abort(&g_hump, g_t);
    TEST_ASSERT_EQ(g_hump.phase, IMU_HUMP_MONITORING);
    TEST_ASSERT_EQ(g_hump.stats.rejected_count, 1);
    TEST_ASSERT_EQ(g_hump.stats.valid_count, 0);

    imu_hump_reset_mission(NULL);
    imu_hump_abort(NULL, 0U);
    imu_hump_step(NULL, 0U, NULL, NULL);
    TEST_ASSERT_EQ(imu_hump_init(NULL, &g_p), IMU_ERR_PARAM);
}

static void t_nan_ignored(void)
{
    hump_setup();
    flat(500U);
    feed(NAN, false, 0.0f);
    feed(INFINITY, false, 0.0f);
    hump(6.0f, 6.0f, 1.0f);
    TEST_ASSERT_EQ(g_log.completed, 1);
}

static void t_height_estimates(void)
{
    imu_hump_meas_t m;

    imu_params_default(&g_p);
    (void)memset(&m, 0, sizeof(m));
    m.peak_pitch_deg = 12.37f;        /* asin(30 / 140) */
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_method, IMU_HEIGHT_METHOD_GEOMETRIC);
    TEST_ASSERT_NEAR(m.height_est_mm, 30.0, 0.1);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_LOW);      /* uncalibrated */

    g_p.b_hump_height_calibrated = true;
    g_p.hump_height_gain = 1.1f;
    g_p.hump_height_offset_mm = -1.0f;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_NEAR(m.height_est_mm, 32.0, 0.1);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_MEDIUM);

    m.b_length_valid = true;
    m.length_est_mm  = 400.0f;        /* long hump: pitch = slope only */
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_LOW);

    imu_params_default(&g_p);
    g_p.height_method = IMU_HEIGHT_METHOD_INTEGRATED;
    m.b_rise_valid = true;
    m.rise_mm      = 15.0f;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_method, IMU_HEIGHT_METHOD_INTEGRATED);
    TEST_ASSERT_NEAR(m.height_est_mm, 30.0, 0.1);   /* 15 / 0.5 */
    m.b_rise_valid = false;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_method, IMU_HEIGHT_METHOD_GEOMETRIC);

    g_p.height_method = IMU_HEIGHT_METHOD_NONE;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_NONE);

    imu_params_default(&g_p);
    m.peak_pitch_deg = -3.0f;          /* never negative heights */
    m.b_attitude_degraded = true;
    imu_hump_estimate_height(&g_p, &m);
    TEST_ASSERT(m.height_est_mm >= 0.0f);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_LOW);
}

static void t_hint_gating(void)
{
    uint32_t t;

    /* Vehicle stationary (encoders): tilting by hand is not a hump. */
    hump_setup();
    for (t = 0U; t < 500U; t += 20U)
    {
        feed(0.0f, true, 0.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(8.0f * sinf(IMU_PI_F * (float)t / 400.0f), true, 0.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(-8.0f * sinf(IMU_PI_F * (float)t / 400.0f), true, 0.0f);
    }
    TEST_ASSERT_EQ(g_log.detected, 0);

    /* Moving with speed: integrated rise and length are measured. */
    hump_setup();
    for (t = 0U; t < 500U; t += 20U)
    {
        feed(0.0f, true, 300.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(8.0f * sinf(IMU_PI_F * (float)t / 400.0f), true, 300.0f);
    }
    for (t = 0U; t < 100U; t += 20U)
    {
        feed(0.0f, true, 300.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(-8.0f * sinf(IMU_PI_F * (float)t / 400.0f), true, 300.0f);
    }
    for (t = 0U; t < 400U; t += 20U)
    {
        feed(0.0f, true, 300.0f);
    }
    TEST_ASSERT_EQ(g_log.completed, 1);
    TEST_ASSERT(g_log.last_done.b_rise_valid);
    TEST_ASSERT(g_log.last_done.rise_mm > 5.0f);
    TEST_ASSERT(g_log.last_done.length_est_mm > 200.0f);
}

static void t_names(void)
{
    TEST_ASSERT(0 == strcmp(imu_hump_validity_name(IMU_HUMP_VALID), "VALID"));
    TEST_ASSERT(0 == strcmp(imu_hump_validity_name((imu_hump_validity_t)99),
                            "UNKNOWN"));
}

void suite_unit_hump(void)
{
    test_section("UNIT (white-box): hump detector");
    RUN_TEST(t_canonical);
    RUN_TEST(t_boundaries);
    RUN_TEST(t_spike_rejected);
    RUN_TEST(t_slope_and_no_descent);
    RUN_TEST(t_artefact_then_real_hump);
    RUN_TEST(t_multiple_retain_max);
    RUN_TEST(t_no_double_count);
    RUN_TEST(t_rearm);
    RUN_TEST(t_speed_variation);
    RUN_TEST(t_reset_and_abort);
    RUN_TEST(t_nan_ignored);
    RUN_TEST(t_height_estimates);
    RUN_TEST(t_hint_gating);
    RUN_TEST(t_names);
}
```

## `tests/imu/test_unit_events.c`

```c
/**
 * @file    test_unit_events.c
 * @brief   Unit tests: bounded event dispatcher (overflow policy, critical
 *          retention, ordering, rate limiting, notification).
 */

#include <string.h>

#include "imu/imu_events.h"
#include "test_util.h"

static imu_event_t mk(imu_event_type_t type, imu_severity_t sev)
{
    imu_event_t e;

    (void)memset(&e, 0, sizeof(e));
    e.type     = (uint8_t)type;
    e.severity = (uint8_t)sev;
    return (e);
}

static void t_seq_and_names(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;
    int                    i;

    cap_reset(64U);
    imu_events_init(&d, &s);
    for (i = 0; i < 5; i++)
    {
        e = mk(IMU_EVT_HUMP_DETECTED, IMU_SEV_INFO);
        TEST_ASSERT_EQ(imu_events_submit(&d, &e, 100U), IMU_OK);
    }
    TEST_ASSERT_EQ(g_cap.count, 5);
    TEST_ASSERT_EQ(g_cap.ev[0].seq, 0);
    TEST_ASSERT_EQ(g_cap.ev[4].seq, 4);
    TEST_ASSERT(0 == strcmp(imu_event_type_name(IMU_EVT_IMPACT),
                            "IMPACT/COLLISION"));
    TEST_ASSERT(0 == strcmp(imu_event_type_name((imu_event_type_t)77),
                            "INVALID"));
    TEST_ASSERT(sizeof(imu_event_t) <= 40U);
}

static void t_overflow_policy(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;

    cap_reset(2U);
    imu_events_init(&d, &s);

    e = mk(IMU_EVT_TILT, IMU_SEV_MINOR);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_OK);
    e = mk(IMU_EVT_TURNING, IMU_SEV_INFO);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_OK);
    e = mk(IMU_EVT_HUMP_PEAK, IMU_SEV_INFO);           /* queue full */
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_QUEUE_FULL);
    TEST_ASSERT_EQ(d.stats.dropped[IMU_EVT_HUMP_PEAK], 1);

    e = mk(IMU_EVT_IMPACT, IMU_SEV_CRITICAL);          /* critical */
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_QUEUE_FULL);
    TEST_ASSERT_EQ(imu_events_pending(&d), 1);
    TEST_ASSERT_EQ(g_cap.notified, 1);                 /* immediate */
    e = mk(IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL);
    (void)imu_events_submit(&d, &e, 0U);
    TEST_ASSERT_EQ(imu_events_pending(&d), 2);

    imu_events_service(&d);                            /* still full */
    TEST_ASSERT_EQ(imu_events_pending(&d), 2);

    cap_drain();
    imu_events_service(&d);
    TEST_ASSERT_EQ(imu_events_pending(&d), 0);
    TEST_ASSERT_EQ(g_cap.ev[2].type, IMU_EVT_IMPACT);   /* order kept */
    TEST_ASSERT_EQ(g_cap.ev[3].type, IMU_EVT_IMU_FAULT);
    TEST_ASSERT_EQ(d.stats.critical_lost, 0);
}

static void t_pending_ring_bound(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;
    uint32_t               i;

    cap_reset(0U);   /* consumer never drains */
    imu_events_init(&d, &s);
    for (i = 0U; i < 50U; i++)
    {
        e = mk(IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL);
        e.data.fault.attempt = i;
        (void)imu_events_submit(&d, &e, i);
    }
    TEST_ASSERT_EQ(imu_events_pending(&d), IMU_EVENT_CRITICAL_PENDING);
    TEST_ASSERT_EQ(d.stats.critical_lost, 50U - IMU_EVENT_CRITICAL_PENDING);
    /* Newest critical information is the one retained. */
    g_cap.capacity = 64U;
    imu_events_service(&d);
    TEST_ASSERT_EQ(g_cap.count, IMU_EVENT_CRITICAL_PENDING);
    TEST_ASSERT_EQ(g_cap.ev[IMU_EVENT_CRITICAL_PENDING - 1U]
                       .data.fault.attempt, 49);
}

static void t_rate_limit(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;
    uint32_t               t;
    uint32_t               ok = 0U;

    cap_reset(CAP_MAX);
    imu_events_init(&d, &s);
    imu_events_set_rate_limit(&d, IMU_EVT_MOTION, 250U);
    imu_events_set_rate_limit(&d, IMU_EVT_IMPACT, 250U);
    for (t = 0U; t < 1000U; t += 20U)
    {
        e = mk(IMU_EVT_MOTION, IMU_SEV_INFO);
        if (IMU_OK == imu_events_submit(&d, &e, t))
        {
            ok++;
        }
    }
    TEST_ASSERT_EQ(ok, 4);   /* 0, 260, 520, 780 */
    TEST_ASSERT(d.stats.rate_limited[IMU_EVT_MOTION] > 40U);

    /* Critical events are never rate-limited. */
    ok = 0U;
    for (t = 0U; t < 100U; t += 20U)
    {
        e = mk(IMU_EVT_IMPACT, IMU_SEV_CRITICAL);
        if (IMU_OK == imu_events_submit(&d, &e, t))
        {
            ok++;
        }
    }
    TEST_ASSERT_EQ(ok, 5);
}

static void t_sink_errors(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;

    cap_reset(8U);
    g_cap.b_error = true;
    imu_events_init(&d, &s);
    e = mk(IMU_EVT_TILT, IMU_SEV_MINOR);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_QUEUE_FULL);
    TEST_ASSERT_EQ(d.stats.sink_errors, 1);
    e = mk(IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL);       /* kept for retry */
    (void)imu_events_submit(&d, &e, 0U);
    TEST_ASSERT_EQ(imu_events_pending(&d), 1);
    g_cap.b_error = false;
    imu_events_service(&d);
    TEST_ASSERT_EQ(imu_events_pending(&d), 0);

    imu_events_init(&d, NULL);                         /* no sink */
    e = mk(IMU_EVT_TILT, IMU_SEV_MINOR);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_QUEUE_FULL);

    e = mk((imu_event_type_t)0, IMU_SEV_INFO);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_PARAM);
    e = mk(IMU_EVT_TYPE_COUNT, IMU_SEV_INFO);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_events_submit(NULL, &e, 0U), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_events_submit(&d, NULL, 0U), IMU_ERR_PARAM);
    imu_events_service(NULL);
    imu_events_init(NULL, NULL);
}

static void t_critical_classification(void)
{
    imu_event_t e = mk(IMU_EVT_IMPACT, IMU_SEV_MINOR);

    TEST_ASSERT(!imu_events_is_critical(&e));
    e.severity = IMU_SEV_CRITICAL;
    TEST_ASSERT(imu_events_is_critical(&e));
    e = mk(IMU_EVT_IMU_FAULT, IMU_SEV_INFO);
    TEST_ASSERT(imu_events_is_critical(&e));
    e = mk(IMU_EVT_TILT, IMU_SEV_CRITICAL);
    TEST_ASSERT(imu_events_is_critical(&e));
    e = mk(IMU_EVT_HUMP_COMPLETED, IMU_SEV_INFO);
    TEST_ASSERT(!imu_events_is_critical(&e));
    TEST_ASSERT(!imu_events_is_critical(NULL));
}

void suite_unit_events(void)
{
    test_section("UNIT (white-box): event dispatcher");
    RUN_TEST(t_seq_and_names);
    RUN_TEST(t_overflow_policy);
    RUN_TEST(t_pending_ring_bound);
    RUN_TEST(t_rate_limit);
    RUN_TEST(t_sink_errors);
    RUN_TEST(t_critical_classification);
}
```

## `tests/imu/test_blackbox.c`

```c
/**
 * @file    test_blackbox.c
 * @brief   BLACK-BOX system tests: only the public imu_terrain API, the
 *          emitted events and the status snapshot are observed. The sensor
 *          is the register-level simulator driven by the physical vehicle
 *          model (known ground truth). Covers the 16 required scenarios
 *          plus additional integration behaviours.
 *
 * Test design technique per case is noted: EP = equivalence partitioning,
 * BVA = boundary value, ST = state-transition, SC = scenario/use-case.
 */

#include <string.h>

#include "imu/imu_math.h"
#include "imu/imu_terrain.h"
#include "test_util.h"

/* Standard timeline: 0..3 s stationary (calibration), then segments. */
static void bb_boot(void)
{
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
}

static void bb_drive(double t0, double dur, double v)
{
    scn_add_segment(&g_h.scn, t0, dur, v, v, 0.0);
}

static uint32_t count_motion_seen(imu_motion_class_t m)
{
    uint32_t n   = 0U;
    uint32_t lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    uint32_t i;

    for (i = 0U; i < lim; i++)
    {
        if ((IMU_EVT_MOTION == g_cap.ev[i].type)
            && (g_cap.ev[i].data.motion.motion == (uint8_t)m))
        {
            n++;
        }
    }
    return (n);
}

static uint32_t count_status(imu_event_type_t type, imu_status_code_t code)
{
    uint32_t n   = 0U;
    uint32_t lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    uint32_t i;

    for (i = 0U; i < lim; i++)
    {
        if ((g_cap.ev[i].type == (uint8_t)type)
            && (g_cap.ev[i].data.fault.status == (uint8_t)code))
        {
            n++;
        }
    }
    return (n);
}

/* --- 1. Sensor initialisation and communication failures (ST, EP) ---- */
static void bb01_init_failure_then_success(void)
{
    imu_status_t st;

    harness_setup();
    sim_cfg()->b_accel_absent = true;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    harness_run_ms(3000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_INITIALISATION);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_INIT_FAILED);
    TEST_ASSERT(!st.b_sensor_ok);
    TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT, IMU_STATUS_FAULT_RAISED), 1);
    TEST_ASSERT(g_cap.notified >= 1U);
    TEST_ASSERT(!harness_trace_has(IMU_STATE_MONITORING));

    sim_cfg()->b_accel_absent = false;      /* cable re-seated */
    TEST_ASSERT(harness_until_monitoring(6000U));
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_READY), 1);
    TEST_ASSERT_EQ(harness_status().fault, IMU_FAULT_NONE);
}

static void bb02_wrong_device(void)
{
    harness_setup();
    sim_cfg()->b_wrong_id = true;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_INITIALISATION);
    TEST_ASSERT_EQ(harness_status().last_error, IMU_ERR_ID);
}

static void bb03_calibration_while_moving(void)
{
    imu_status_t st;

    harness_setup();
    g_h.scn.vib_noise_g = 0.05;
    bb_drive(0.0, 6.0, 0.2);                 /* moving during start-up */
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    harness_run_ms(5500U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_CALIBRATION);
    TEST_ASSERT(count_status(IMU_EVT_STATUS, IMU_STATUS_CAL_RETRY) >= 1U);
    TEST_ASSERT(harness_until_monitoring(6000U));   /* stopped -> ok */
    st = harness_status();
    TEST_ASSERT(st.b_calibrated);
    TEST_ASSERT(st.cal_noise_g <= g_h.params.cal_max_noise_g);
    TEST_ASSERT(g_h.sys.cal_attempts == 0U);
}

/* --- 2. Normal stationary readings (EP) ------------------------------ */
static void bb04_stationary(void)
{
    imu_status_t st;
    uint32_t     t;

    harness_setup();
    bb_boot();
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_READY), 1);
    for (t = 0U; t < 10000U; t += 500U)
    {
        harness_run_ms(500U);
        st = harness_status();
        TEST_ASSERT(fabsf(st.pitch_deg) < 0.5f);
        TEST_ASSERT(fabsf(st.roll_deg) < 0.5f);
    }
    TEST_ASSERT_EQ(st.motion, IMU_MOTION_STATIONARY);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_DETECTED), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_TILT), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMU_FAULT), 0);
    TEST_ASSERT(st.b_sensor_ok);
    TEST_ASSERT(st.b_calibrated);
    TEST_ASSERT_NEAR(st.cal_g_ref, 1.0, 0.01);
}

/* Sensor mounted tilted and rotated: axis map + levelling (SC). */
static void bb05_mounting(void)
{
    double const a = 8.0 * 3.14159265358979 / 180.0;
    imu_status_t st;

    harness_setup();
    /* Sensor rotated +90 deg about Z (sensor X = vehicle LEFT, sensor Y =
     * vehicle BACK), then pitched by 8 deg on its bracket. */
    (void)memset(g_h.scn.mount, 0, sizeof(g_h.scn.mount));
    g_h.scn.mount[0][0] = 0.0;      g_h.scn.mount[0][1] = cos(a);
    g_h.scn.mount[0][2] = sin(a);
    g_h.scn.mount[1][0] = -1.0;
    g_h.scn.mount[2][1] = -sin(a);  g_h.scn.mount[2][2] = cos(a);
    /* vehicle fwd = -sensor Y, vehicle left = +sensor X, up = +sensor Z */
    g_h.params.axis_map[0] = 1U; g_h.params.axis_sign[0] = -1;
    g_h.params.axis_map[1] = 0U; g_h.params.axis_sign[1] = 1;
    g_h.params.axis_map[2] = 2U; g_h.params.axis_sign[2] = 1;
    bb_drive(3.0, 4.0, 0.25);
    scn_add_segment(&g_h.scn, 7.0, 2.0, 0.25, 0.25, -60.0);  /* left */
    bb_drive(9.0, 3.0, 0.25);
    scn_add_hump(&g_h.scn, 0.5, 0.025, 0.10);
    bb_boot();
    st = harness_status();
    TEST_ASSERT_NEAR(st.cal_mount_tilt_deg, 8.0, 0.5);
    TEST_ASSERT(fabsf(st.pitch_deg) < 0.5f);
    harness_run_ms(10000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 1);
    TEST_ASSERT(cap_count_type(IMU_EVT_TURNING) >= 2U);
    {
        imu_event_t const * p = cap_last_of(IMU_EVT_TURNING);

        TEST_ASSERT(NULL != p);
        if (NULL != p)
        {
            TEST_ASSERT_EQ(p->data.turn.direction, IMU_TURN_LEFT);
        }
    }
}

/* --- 3. Normal straight-line movement (EP) --------------------------- */
static void bb06_straight_cruise(void)
{
    imu_status_t st;

    harness_setup();
    g_h.b_hint = true;
    g_h.scn.vib_noise_g = 0.03;
    bb_drive(3.0, 12.0, 0.3);
    bb_boot();
    harness_run_ms(11000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.motion, IMU_MOTION_STEADY_MOTION);
    TEST_ASSERT_EQ(st.motion_conf, IMU_CONF_HIGH);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_DETECTED), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_TILT), 0);
    TEST_ASSERT_EQ(count_motion_seen(IMU_MOTION_TURNING_LEFT), 0);
    TEST_ASSERT_EQ(count_motion_seen(IMU_MOTION_TURNING_RIGHT), 0);
    TEST_ASSERT_EQ(st.heading_quality, IMU_HEADING_UNCALIBRATED);
}

/* --- 4. Acceleration and deceleration (EP) --------------------------- */
static void bb07_accel_decel_with_hint(void)
{
    harness_setup();
    g_h.b_hint = true;
    g_h.scn.vib_noise_g = 0.01;
    scn_add_segment(&g_h.scn, 3.0, 0.6, 0.0, 0.4, 0.0);   /* +0.068 g */
    bb_drive(3.6, 3.0, 0.4);
    scn_add_segment(&g_h.scn, 6.6, 0.6, 0.4, 0.0, 0.0);   /* -0.068 g */
    bb_boot();
    harness_run_ms(7000U);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_ACCELERATING) >= 1U);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_STEADY_MOTION) >= 1U);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_DECELERATING) >= 1U);
    TEST_ASSERT_EQ(harness_status().motion, IMU_MOTION_STATIONARY);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_DETECTED), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
}

static void bb07b_accel_without_hint(void)
{
    harness_setup();
    g_h.scn.vib_noise_g = 0.01;
    scn_add_segment(&g_h.scn, 3.0, 0.6, 0.0, 0.4, 0.0);
    bb_drive(3.6, 4.0, 0.4);
    scn_add_segment(&g_h.scn, 7.6, 0.6, 0.4, 0.0, 0.0);
    bb_boot();
    harness_run_ms(8000U);
    /* Documented limitation: the pitch artefact may open a candidate, but
     * it must never be counted as a hump. */
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 0);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_DECELERATING) >= 1U);
}

/* --- 5. Left and right turns (EP) ------------------------------------ */
static void bb08_turns(void)
{
    uint32_t i;
    uint32_t lim;
    uint32_t left_end = 0U;
    uint32_t right_end = 0U;

    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    bb_drive(3.0, 1.0, 0.2);
    scn_add_segment(&g_h.scn, 4.0, 1.5, 0.2, 0.2, -60.0);   /* 90 L */
    bb_drive(5.5, 2.0, 0.2);
    scn_add_segment(&g_h.scn, 7.5, 1.5, 0.2, 0.2, 60.0);    /* 90 R */
    bb_drive(9.0, 2.0, 0.2);
    bb_boot();
    harness_run_ms(8500U);

    lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    for (i = 0U; i < lim; i++)
    {
        imu_event_t const * p = &g_cap.ev[i];

        if ((IMU_EVT_TURNING == p->type) && (IMU_PHASE_END == p->data.turn.phase))
        {
            if (IMU_TURN_LEFT == p->data.turn.direction)
            {
                left_end++;
                TEST_ASSERT_NEAR((double)p->data.turn.delta_cdeg / 100.0,
                                 -90.0, 15.0);
            }
            else
            {
                right_end++;
                TEST_ASSERT_NEAR((double)p->data.turn.delta_cdeg / 100.0,
                                 90.0, 15.0);
            }
        }
    }
    TEST_ASSERT_EQ(left_end, 1);
    TEST_ASSERT_EQ(right_end, 1);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_TURNING_LEFT) >= 1U);
    TEST_ASSERT(count_motion_seen(IMU_MOTION_TURNING_RIGHT) >= 1U);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
}

/* --- 6/7. Small and large hump (EP, accuracy vs ground truth) -------- */
static void run_single_hump(double h_m, double len_m, double v,
                            imu_hump_meas_t * p_out)
{
    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    bb_drive(3.0, 30.0, v);
    scn_add_hump(&g_h.scn, 0.4, h_m, len_m);
    bb_boot();
    harness_run_ms((uint32_t)(1000.0 * ((0.4 + len_m + 0.6) / v)) + 1500U);
    (void)memset(p_out, 0, sizeof(*p_out));
    if (harness_status().hump.b_has_last)
    {
        *p_out = harness_status().hump.last;
    }
}

static void bb09_small_hump(void)
{
    imu_hump_meas_t m;
    double const    truth = scn_true_peak_pitch_deg(0.012, 0.10, 0.14);

    run_single_hump(0.012, 0.10, 0.25, &m);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_DETECTED), 1);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_PEAK), 1);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 1);
    TEST_ASSERT_NEAR(m.peak_pitch_deg, truth, 0.25 * truth);
    TEST_ASSERT_NEAR(m.height_est_mm, 12.0, 4.0);
    TEST_ASSERT_EQ(m.height_conf, IMU_CONF_LOW);
    printf("      small hump: truth %.2f deg, measured %.2f deg, est %.1f mm "
           "(true 12.0)\n", truth, (double)m.peak_pitch_deg,
           (double)m.height_est_mm);
}

static void bb10_large_hump(void)
{
    imu_hump_meas_t m;
    double const    truth = scn_true_peak_pitch_deg(0.030, 0.10, 0.14);

    run_single_hump(0.030, 0.10, 0.25, &m);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT_NEAR(m.peak_pitch_deg, truth, 0.20 * truth);
    TEST_ASSERT_NEAR(m.height_est_mm, 30.0, 7.0);
    TEST_ASSERT(m.peak_vert_up_g > 0.0f);
    printf("      large hump: truth %.2f deg, measured %.2f deg, est %.1f mm "
           "(true 30.0)\n", truth, (double)m.peak_pitch_deg,
           (double)m.height_est_mm);
}

/* --- 8. Multiple humps, retain highest (SC) -------------------------- */
static void bb11_multiple_humps(void)
{
    imu_status_t        st;
    imu_telemetry_t     tele;
    imu_event_t const * p_last;
    uint32_t            i;
    uint32_t            lim;
    uint32_t            new_max = 0U;

    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    bb_drive(3.0, 30.0, 0.25);
    scn_add_hump(&g_h.scn, 0.4, 0.015, 0.10);
    scn_add_hump(&g_h.scn, 1.2, 0.035, 0.10);
    scn_add_hump(&g_h.scn, 2.0, 0.020, 0.10);
    bb_boot();
    harness_run_ms(11000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 3);
    TEST_ASSERT_EQ(st.hump.max.index, 2);
    TEST_ASSERT_EQ(st.hump.last.index, 3);
    TEST_ASSERT(st.hump.max.peak_pitch_deg > st.hump.last.peak_pitch_deg);
    TEST_ASSERT_EQ(st.hump.rejected_count, 0);

    lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    for (i = 0U; i < lim; i++)
    {
        if ((IMU_EVT_HUMP_COMPLETED == g_cap.ev[i].type)
            && (0U != g_cap.ev[i].data.hump.b_new_max))
        {
            new_max++;
        }
    }
    TEST_ASSERT_EQ(new_max, 2);
    p_last = cap_last_of(IMU_EVT_HUMP_COMPLETED);
    TEST_ASSERT(NULL != p_last);
    if (NULL != p_last)
    {
        TEST_ASSERT_EQ(p_last->data.hump.max_peak_pitch_cdeg,
                       (int16_t)lroundf(st.hump.max.peak_pitch_deg * 100.0f));
    }
    imu_terrain_make_telemetry(&st, &tele);
    TEST_ASSERT_EQ(tele.hump_count, 3);
    TEST_ASSERT_EQ(tele.hump_peak_cdeg,
                   (int16_t)lroundf(st.hump.max.peak_pitch_deg * 100.0f));
    TEST_ASSERT(tele.hump_peak_height_mm > 25);
}

/* --- 9. Noise and false hump detections (EP) ------------------------- */
static void bb12_noise_no_false_humps(void)
{
    uint32_t k;

    harness_setup();
    g_h.scn.vib_noise_g = 0.06;          /* rough floor, worn motors */
    bb_drive(3.0, 30.0, 0.3);
    for (k = 0U; k < 6U; k++)            /* 2 mm ridges: below threshold */
    {
        scn_add_hump(&g_h.scn, 0.3 + (0.5 * (double)k), 0.002, 0.04);
    }
    for (k = 0U; k < 4U; k++)            /* vertical knocks */
    {
        scn_add_impact(&g_h.scn, 5.0 + (double)k * 1.7, 0.01, 0.8,
                       0.0f, 0.0f, 1.0f);
    }
    bb_boot();
    harness_run_ms(12000U);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
    TEST_ASSERT(harness_status().b_sensor_ok);
}

/* --- 10. Short sudden impact -> fault handling (ST) ------------------ */
static void bb13_impact_collision(void)
{
    imu_status_t        st;
    imu_event_t const * p;

    harness_setup();
    bb_drive(3.0, 2.0, 0.3);
    scn_add_impact(&g_h.scn, 4.5, 0.015, 3.0, -1.0f, 0.0f, 0.0f);
    bb_boot();
    harness_run_ms(2400U);
    TEST_ASSERT(harness_status().state != IMU_STATE_FAULT_SAFE);
    harness_run_ms(200U);                    /* impact at t = 4.5 s */
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_FAULT_SAFE);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_COLLISION);
    TEST_ASSERT_EQ(st.motion, IMU_MOTION_SUDDEN_IMPACT);
    TEST_ASSERT(st.b_sensor_ok);                 /* sensor itself is fine */
    p = cap_last_of(IMU_EVT_IMPACT);
    TEST_ASSERT(NULL != p);
    if (NULL != p)
    {
        TEST_ASSERT_EQ(p->severity, IMU_SEV_CRITICAL);
        TEST_ASSERT(p->data.impact.peak_mg > 2000);
        TEST_ASSERT(p->data.impact.horiz_mg > p->data.impact.vert_mg);
    }
    TEST_ASSERT(g_cap.notified >= 1U);
    TEST_ASSERT_EQ(g_cap.last_notified.type, IMU_EVT_IMPACT);
    TEST_ASSERT(harness_trace_has(IMU_STATE_IMPACT_EVENT));
    TEST_ASSERT(harness_trace_has(IMU_STATE_FAULT_HANDLING));

    /* No silent return: stays in FAULT/SAFE without acknowledgement. */
    harness_run_ms(5000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_FAULT_SAFE);

    TEST_ASSERT_EQ(imu_terrain_command(&g_h.sys, IMU_CMD_CLEAR_FAULT), IMU_OK);
    harness_run_ms(1000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_RESUMED), 1);
}

/* --- 11. Repeated impact suppression (BVA on cool-down) -------------- */
static void bb14_repeated_impacts(void)
{
    uint32_t            k;
    imu_event_t const * p;

    harness_setup();
    bb_drive(3.0, 6.0, 0.2);
    for (k = 0U; k < 5U; k++)
    {
        scn_add_impact(&g_h.scn, 4.0 + (0.12 * (double)k), 0.012, 1.6,
                       0.0f, 1.0f, 0.0f);          /* side scrapes */
    }
    scn_add_impact(&g_h.scn, 6.0, 0.012, 1.6, 0.0f, 1.0f, 0.0f);
    bb_boot();
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 2);
    p = cap_last_of(IMU_EVT_IMPACT);
    TEST_ASSERT(NULL != p);
    if (NULL != p)
    {
        TEST_ASSERT_EQ(p->severity, IMU_SEV_MINOR);
        TEST_ASSERT(p->data.impact.suppressed >= 3U);
    }
    TEST_ASSERT(harness_status().state != IMU_STATE_FAULT_SAFE);
}

/* --- 12. Tilt events (EP, BVA) --------------------------------------- */
static void bb15_tilt(void)
{
    imu_event_t const * p;
    uint32_t            i;
    uint32_t            lim;
    uint32_t            starts = 0U;
    uint32_t            ends   = 0U;

    harness_setup();
    scn_add_segment(&g_h.scn, 4.0, 2.0, 0.0, 0.0, 0.0);
    g_h.scn.seg[0].roll_deg = 20.0;              /* side slope */
    scn_add_segment(&g_h.scn, 8.0, 1.0, 0.0, 0.0, 0.0);
    g_h.scn.seg[1].roll_deg = 40.0;              /* about to roll over */
    bb_boot();
    harness_run_ms(8000U);

    lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    for (i = 0U; i < lim; i++)
    {
        p = &g_cap.ev[i];
        if (IMU_EVT_TILT == p->type)
        {
            if (IMU_PHASE_END == p->data.tilt.phase)
            {
                ends++;
            }
            else
            {
                starts++;
            }
        }
    }
    TEST_ASSERT_EQ(starts, 2);
    TEST_ASSERT_EQ(ends, 2);
    TEST_ASSERT(g_cap.notified >= 1U);           /* critical tilt */
    TEST_ASSERT_EQ(g_cap.last_notified.type, IMU_EVT_TILT);
    TEST_ASSERT(harness_trace_has(IMU_STATE_TILT_EVENT) || (starts == 2U));
    TEST_ASSERT(harness_status().state == IMU_STATE_MONITORING);
}

/* --- 13/14. Disconnection, invalid data, recovery (ST) --------------- */
static void bb16_disconnect_and_recover(void)
{
    imu_status_t st;

    harness_setup();
    bb_drive(3.0, 20.0, 0.2);
    bb_boot();
    harness_run_ms(1000U);
    sim_cfg()->b_bus_stuck = true;           /* SDA held low / unplugged */
    harness_run_ms(200U);
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_FAULT_SAFE);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_I2C);
    TEST_ASSERT(!st.b_sensor_ok);
    TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT, IMU_STATUS_FAULT_RAISED), 1);
    TEST_ASSERT_EQ(g_cap.last_notified.type, IMU_EVT_IMU_FAULT);
    TEST_ASSERT(harness_trace_has(IMU_STATE_SENSOR_FAULT));

    harness_run_ms(2000U);                   /* bus-clear frees the bus */
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_NONE);
    TEST_ASSERT(harness_trace_has(IMU_STATE_RECOVERY));
    TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT, IMU_STATUS_RECOVERED), 1);
    TEST_ASSERT(st.b_calibrated);            /* calibration retained */
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_READY), 1);
    TEST_ASSERT(sim_cfg()->recover_calls >= 1U);
}

static void bb17_persistent_fault(void)
{
    uint32_t t;

    harness_setup();
    bb_boot();
    sim_cfg()->b_bus_stuck = true;
    sim_cfg()->b_stuck_survives_recovery = true;
    for (t = 0U; t < 60000U; t += 1000U)
    {
        harness_run_ms(1000U);
        TEST_ASSERT(harness_status().state != IMU_STATE_MONITORING);
    }
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_FAULT_SAFE);
    TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT,
                                IMU_STATUS_FAULT_PERSISTENT), 1);
    /* Bounded recovery effort: back-off up to 2 s -> < 40 attempts/min. */
    TEST_ASSERT(sim_cfg()->recover_calls < 40U);
    TEST_ASSERT(sim_cfg()->recover_calls >= 10U);
}

static void bb18_invalid_and_frozen_data(void)
{
    imu_status_t st;

    harness_setup();
    bb_boot();
    sim_cfg()->b_frozen = true;
    harness_run_ms(1200U);
    st = harness_status();
    TEST_ASSERT((st.state == IMU_STATE_FAULT_SAFE)
                || (st.state == IMU_STATE_RECOVERY));
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_FROZEN_DATA);
    {
        uint32_t t;

        /* A frozen sensor must never pass recovery verification. */
        for (t = 0U; t < 30000U; t += 500U)
        {
            harness_run_ms(500U);
            TEST_ASSERT(harness_status().state != IMU_STATE_MONITORING);
        }
        TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT, IMU_STATUS_RECOVERED),
                       0);
        TEST_ASSERT_EQ(count_status(IMU_EVT_IMU_FAULT,
                                    IMU_STATUS_FAULT_PERSISTENT), 1);
    }
    sim_cfg()->b_frozen = false;            /* sensor comes back to life */
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);

    harness_setup();
    bb_boot();
    sim_cfg()->const_accel.z = 0.0f;           /* dead output: 0 g */
    sim_cfg()->p_fn = NULL;
    harness_run_ms(500U);
    st = harness_status();
    TEST_ASSERT_EQ(st.state, IMU_STATE_FAULT_SAFE);
    TEST_ASSERT_EQ(st.fault, IMU_FAULT_INVALID_DATA);
}

static void bb19_intermittent_absorbed(void)
{
    harness_setup();
    bb_drive(3.0, 20.0, 0.2);
    scn_add_hump(&g_h.scn, 0.5, 0.025, 0.10);
    bb_boot();
    sim_cfg()->fail_every = 7U;               /* 1 in 7 transfers NACK */
    harness_run_ms(6000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT(harness_status().i2c.retries > 10U);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMU_FAULT), 0);
}

/* --- 15. Event generation and queue overflow (EP) -------------------- */
static void bb20_queue_overflow(void)
{
    imu_status_t st;

    harness_setup();
    bb_drive(3.0, 10.0, 0.25);
    scn_add_hump(&g_h.scn, 0.3, 0.025, 0.10);
    scn_add_hump(&g_h.scn, 1.0, 0.025, 0.10);
    scn_add_impact(&g_h.scn, 9.0, 0.015, 3.0, -1.0f, 0.0f, 0.0f);
    cap_reset(2U);
    g_h.b_no_drain = true;                   /* MC stalled */
    bb_boot();
    harness_run_ms(7000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 2);   /* detection unaffected */
    TEST_ASSERT(st.events.dropped[IMU_EVT_HUMP_COMPLETED]
                + st.events.dropped[IMU_EVT_HUMP_DETECTED] >= 2U);
    TEST_ASSERT_EQ(g_cap.count, 2);
    TEST_ASSERT(g_cap.notified >= 1U);        /* direct path still works */
    TEST_ASSERT(imu_events_pending(&g_h.sys.disp) >= 1U);
    TEST_ASSERT_EQ(st.events.critical_lost, 0);

    cap_drain();                              /* MC catches up */
    g_h.b_no_drain = false;
    harness_run_ms(100U);
    TEST_ASSERT_EQ(imu_events_pending(&g_h.sys.disp), 0);
    TEST_ASSERT(NULL != cap_last_of(IMU_EVT_IMPACT));
}

/* --- 16. Mission restart and peak reset (ST) ------------------------- */
static void bb21_mission_restart(void)
{
    imu_status_t    st;
    imu_telemetry_t tele;

    harness_setup();
    bb_drive(3.0, 30.0, 0.25);
    scn_add_hump(&g_h.scn, 0.3, 0.030, 0.10);
    scn_add_hump(&g_h.scn, 1.5, 0.015, 0.10);
    bb_boot();
    harness_run_ms(4000U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 1);
    TEST_ASSERT(st.hump.b_has_max);

    TEST_ASSERT_EQ(imu_terrain_command(&g_h.sys, IMU_CMD_MISSION_START),
                   IMU_OK);
    harness_run_ms(40U);
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 0);
    TEST_ASSERT(!st.hump.b_has_max);
    imu_terrain_make_telemetry(&st, &tele);
    TEST_ASSERT_EQ(tele.hump_peak_cdeg, -1);
    TEST_ASSERT_EQ(tele.hump_peak_height_mm, -1);
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_MISSION_RESET), 1);

    harness_run_ms(5000U);                     /* smaller hump, new run */
    st = harness_status();
    TEST_ASSERT_EQ(st.hump.valid_count, 1);
    TEST_ASSERT_EQ(st.hump.max.index, 1);
    TEST_ASSERT(st.hump.max.peak_pitch_deg < 8.0f);
}

/* --- Additional integration behaviours ------------------------------- */
static void bb22_hump_during_turn(void)
{
    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    bb_drive(3.0, 1.0, 0.25);
    scn_add_segment(&g_h.scn, 4.0, 3.0, 0.25, 0.25, 35.0);
    bb_drive(7.0, 3.0, 0.25);
    scn_add_hump(&g_h.scn, 0.45, 0.025, 0.10);
    bb_boot();
    harness_run_ms(7000U);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT(cap_count_type(IMU_EVT_TURNING) >= 2U);
}

static void bb23_fast_hump_no_collision(void)
{
    harness_setup();
    g_h.scn.vib_noise_g = 0.03;
    bb_drive(3.0, 10.0, 0.5);
    scn_add_hump(&g_h.scn, 0.5, 0.030, 0.08);
    bb_boot();
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
    TEST_ASSERT(harness_status().state != IMU_STATE_FAULT_SAFE);
    {
        imu_event_t const * p = cap_last_of(IMU_EVT_IMPACT);

        TEST_ASSERT((NULL == p) || (IMU_SEV_CRITICAL != p->severity));
    }
}

static void bb24_speed_repeatability(void)
{
    double const    v[] = { 0.12, 0.2, 0.3, 0.45 };
    double          mn  = 1e9;
    double          mx  = -1e9;
    uint32_t        i;
    imu_hump_meas_t m;

    for (i = 0U; i < 4U; i++)
    {
        run_single_hump(0.020, 0.10, v[i], &m);
        TEST_ASSERT_MSG(1U == harness_status().hump.valid_count,
                        "speed %.2f", v[i]);
        mn = fmin(mn, (double)m.peak_pitch_deg);
        mx = fmax(mx, (double)m.peak_pitch_deg);
        printf("      v=%.2f m/s: peak %.2f deg, est %.1f mm\n", v[i],
               (double)m.peak_pitch_deg, (double)m.height_est_mm);
    }
    printf("      truth %.2f deg; spread %.1f %%\n",
           scn_true_peak_pitch_deg(0.020, 0.10, 0.14),
           100.0 * (mx - mn) / mx);
    TEST_ASSERT(((mx - mn) / mx) < 0.30);
}

static void bb25_mag_calibration_spin(void)
{
    imu_status_t st;

    harness_setup();
    g_h.scn.hard_iron.x = 0.15f;              /* motor magnets */
    g_h.scn.hard_iron.y = -0.10f;
    g_h.scn.soft_scale.x = 1.1f;
    g_h.scn.vib_noise_g = 0.02;
    scn_add_segment(&g_h.scn, 4.0, 9.0, 0.0, 0.0, 45.0);  /* 405 deg spin */
    bb_drive(13.0, 4.0, 0.0);
    bb_boot();
    harness_run_ms(100U);
    TEST_ASSERT_EQ(harness_status().heading_quality,
                   IMU_HEADING_UNCALIBRATED);
    harness_run_ms(800U);
    TEST_ASSERT_EQ(imu_terrain_command(&g_h.sys, IMU_CMD_MAG_CAL_BEGIN),
                   IMU_OK);
    harness_run_ms(9200U);
    TEST_ASSERT_EQ(imu_terrain_command(&g_h.sys, IMU_CMD_MAG_CAL_END), IMU_OK);
    harness_run_ms(500U);
    st = harness_status();
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_MAG_CAL_OK), 1);
    TEST_ASSERT(st.b_mag_cal_valid);
    TEST_ASSERT_NEAR(st.mag_cal.hard_iron_gauss.x, 0.15, 0.02);
    TEST_ASSERT_NEAR(st.mag_cal.hard_iron_gauss.y, -0.10, 0.02);
    TEST_ASSERT_EQ(st.heading_quality, IMU_HEADING_GOOD);

    /* Not enough rotation -> rejected, previous state kept. */
    harness_setup();
    scn_add_segment(&g_h.scn, 4.0, 1.0, 0.0, 0.0, 45.0);  /* 45 deg */
    bb_boot();
    harness_run_ms(900U);
    (void)imu_terrain_command(&g_h.sys, IMU_CMD_MAG_CAL_BEGIN);
    harness_run_ms(1500U);
    (void)imu_terrain_command(&g_h.sys, IMU_CMD_MAG_CAL_END);
    harness_run_ms(100U);
    TEST_ASSERT_EQ(count_status(IMU_EVT_STATUS, IMU_STATUS_MAG_CAL_FAILED), 1);
    TEST_ASSERT(!harness_status().b_mag_cal_valid);
}

static void bb26_magnetic_disturbance(void)
{
    uint32_t i;
    uint32_t lim;
    uint32_t starts = 0U;
    bool     b_seen_disturbed = false;
    uint32_t t;

    harness_setup();
    g_h.params.b_mag_cal_valid     = true;    /* stored calibration */
    g_h.params.mag_ref_field_gauss = 0.4176f;
    bb_drive(3.0, 10.0, 0.25);
    g_h.scn.disturb_gauss.x = 0.35f;          /* steel plate / cable */
    g_h.scn.disturb_gauss.y = 0.25f;
    g_h.scn.disturb_t0_s = 5.0;
    g_h.scn.disturb_t1_s = 6.5;
    bb_boot();
    for (t = 0U; t < 6000U; t += 100U)
    {
        harness_run_ms(100U);
        if (IMU_HEADING_DISTURBED == harness_status().heading_quality)
        {
            b_seen_disturbed = true;
        }
    }
    lim = (g_cap.count < CAP_MAX) ? g_cap.count : CAP_MAX;
    for (i = 0U; i < lim; i++)
    {
        if ((IMU_EVT_TURNING == g_cap.ev[i].type)
            && (IMU_PHASE_START == g_cap.ev[i].data.turn.phase))
        {
            starts++;
        }
    }
    TEST_ASSERT(b_seen_disturbed);
    TEST_ASSERT_EQ(starts, 0);
    TEST_ASSERT_EQ(harness_status().heading_quality, IMU_HEADING_GOOD);
}

static void bb27_reset_and_recalibrate_and_stop(void)
{
    harness_setup();
    bb_drive(3.0, 2.0, 0.3);
    scn_add_impact(&g_h.scn, 4.0, 0.015, 3.0, -1.0f, 0.0f, 0.0f);
    bb_boot();
    harness_run_ms(2500U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_FAULT_SAFE);
    (void)imu_terrain_command(&g_h.sys, IMU_CMD_RESET);
    harness_run_ms(40U);
    TEST_ASSERT(harness_trace_has(IMU_STATE_INITIALISATION));
    TEST_ASSERT(harness_until_monitoring(4000U));

    (void)imu_terrain_command(&g_h.sys, IMU_CMD_RECALIBRATE);
    harness_run_ms(40U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_CALIBRATION);
    TEST_ASSERT(harness_until_monitoring(4000U));

    (void)imu_terrain_command(&g_h.sys, IMU_CMD_STOP);
    harness_run_ms(40U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_STOPPED);
    TEST_ASSERT_EQ(sim_mag_reg(0x02U), 0x03);       /* magnetometer asleep */
    harness_run_ms(1000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_STOPPED);
}

static void bb28_telemetry_json(void)
{
    imu_status_t    st;
    imu_telemetry_t tele;
    char            buf[256];
    char            small[16];
    int32_t         n;

    harness_setup();
    bb_drive(3.0, 10.0, 0.25);
    scn_add_hump(&g_h.scn, 0.3, 0.025, 0.10);
    bb_boot();
    harness_run_ms(3500U);
    st = harness_status();
    imu_terrain_make_telemetry(&st, &tele);
    n = imu_terrain_telemetry_json(&tele, buf, sizeof(buf));
    TEST_ASSERT(n > 0);
    TEST_ASSERT(n < 160);      /* leaves room in Buddy 1's 256 B frame */
    TEST_ASSERT(NULL != strstr(buf, "\"hump_peak\":"));
    {
        char expect[32];

        (void)snprintf(expect, sizeof(expect), "\"hump_peak\":%d,",
                       (int)tele.hump_peak_cdeg);
        TEST_ASSERT(NULL != strstr(buf, expect));
    }
    TEST_ASSERT(tele.hump_peak_cdeg > 300);
    TEST_ASSERT_EQ(imu_terrain_telemetry_json(&tele, small, sizeof(small)),
                   -1);
    TEST_ASSERT_EQ(small[0], '\0');
    printf("      telemetry fragment (%d B): %s\n", (int)n, buf);
}

void suite_blackbox(void)
{
    test_section("BLACK-BOX: system scenarios (public API + events only)");
    RUN_TEST(bb01_init_failure_then_success);
    RUN_TEST(bb02_wrong_device);
    RUN_TEST(bb03_calibration_while_moving);
    RUN_TEST(bb04_stationary);
    RUN_TEST(bb05_mounting);
    RUN_TEST(bb06_straight_cruise);
    RUN_TEST(bb07_accel_decel_with_hint);
    RUN_TEST(bb07b_accel_without_hint);
    RUN_TEST(bb08_turns);
    RUN_TEST(bb09_small_hump);
    RUN_TEST(bb10_large_hump);
    RUN_TEST(bb11_multiple_humps);
    RUN_TEST(bb12_noise_no_false_humps);
    RUN_TEST(bb13_impact_collision);
    RUN_TEST(bb14_repeated_impacts);
    RUN_TEST(bb15_tilt);
    RUN_TEST(bb16_disconnect_and_recover);
    RUN_TEST(bb17_persistent_fault);
    RUN_TEST(bb18_invalid_and_frozen_data);
    RUN_TEST(bb19_intermittent_absorbed);
    RUN_TEST(bb20_queue_overflow);
    RUN_TEST(bb21_mission_restart);
    RUN_TEST(bb22_hump_during_turn);
    RUN_TEST(bb23_fast_hump_no_collision);
    RUN_TEST(bb24_speed_repeatability);
    RUN_TEST(bb25_mag_calibration_spin);
    RUN_TEST(bb26_magnetic_disturbance);
    RUN_TEST(bb27_reset_and_recalibrate_and_stop);
    RUN_TEST(bb28_telemetry_json);
}
```

## `tests/imu/test_robustness.c`

```c
/**
 * @file    test_robustness.c
 * @brief   ROBUSTNESS / "PENETRATION" tests: adversarial and malformed
 *          inputs against every external input surface of Buddy 4.
 *
 * Attack / failure surfaces of this subsystem (it has no network surface):
 *   S1  I2C bus data (corrupted, spoofed, garbage, extreme, frozen)
 *   S2  inter-task API (NULL pointers, invalid commands, command floods)
 *   S3  encoder hint from Buddy 2 / MC (spoofed, absurd, replayed, future)
 *   S4  parameter block (out-of-range / NaN / fuzzed configuration)
 *   S5  time base (wrap-around, jumps, time going backwards)
 *   S6  event queue (permanently full consumer, critical-event storms)
 *   S7  floating-point (NaN / Inf injection into every processing entry)
 * Each test asserts: no crash (run under ASan/UBSan), bounded memory, no
 * NaN in published state, state machine stays inside its enum, and no
 * unsafe outcome (e.g. silent return to MONITORING, fake valid hump).
 */

#include <string.h>

#include "imu/imu_calibration.h"
#include "imu/imu_hump.h"
#include "imu/imu_processing.h"
#include "imu/imu_terrain.h"
#include "test_util.h"

static uint32_t g_frng = 2024U;

static uint32_t frand(void)
{
    g_frng ^= g_frng << 13;
    g_frng ^= g_frng >> 17;
    g_frng ^= g_frng << 5;
    return (g_frng);
}

static float frandf(float lo, float hi)
{
    return (lo + ((hi - lo) * ((float)(frand() & 0xFFFFFFU) / 16777215.0f)));
}

static bool status_sane(imu_status_t const * p)
{
    return ((p->state < IMU_STATE_COUNT)
            && (p->motion < IMU_MOTION_COUNT)
            && (p->fault <= IMU_FAULT_COLLISION)
            && (0 != isfinite(p->pitch_deg)) && (0 != isfinite(p->roll_deg))
            && (0 != isfinite(p->tilt_deg))
            && (0 != isfinite(p->heading_deg))
            && (0 != isfinite(p->turn_rate_dps))
            && (0 != isfinite(p->vibration_g))
            && (0 != isfinite(p->hump.max.peak_pitch_deg))
            && (0 != isfinite(p->hump.max.height_est_mm))
            && (p->hump.max.height_est_mm >= 0.0f)
            /* transient states never persist between steps */
            && (p->state != IMU_STATE_TILT_EVENT)
            && (p->state != IMU_STATE_TURN_EVENT)
            && (p->state != IMU_STATE_IMPACT_EVENT)
            && (p->state != IMU_STATE_FAULT_HANDLING)
            && (p->state != IMU_STATE_SENSOR_FAULT));
}

/* S1: random garbage on the bus for a long time. */
static void p01_i2c_garbage_fuzz(void)
{
    uint32_t t;
    bool     b_sane = true;

    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    harness_run_ms(3000U);
    sim_cfg()->b_garbage = true;
    for (t = 0U; t < 30000U; t += 100U)
    {
        imu_status_t st;

        harness_run_ms(100U);
        st = harness_status();
        b_sane = b_sane && status_sane(&st);
        TEST_ASSERT(st.state != IMU_STATE_MONITORING
                    || st.b_calibrated);
    }
    TEST_ASSERT(b_sane);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 0);
    TEST_ASSERT(harness_status().state != IMU_STATE_HUMP_DESCENT);
    TEST_ASSERT(g_cap.count < 3000U);   /* no event storm */
}

/* S1: random fault pattern sweep, every failure ratio. */
static void p02_failure_ratio_sweep(void)
{
    uint32_t every;

    for (every = 1U; every <= 12U; every++)
    {
        imu_status_t st;

        harness_setup();
        scn_add_segment(&g_h.scn, 3.0, 20.0, 0.2, 0.2, 0.0);
        TEST_ASSERT_EQ(harness_start(), IMU_OK);
        (void)harness_until_monitoring(5000U);
        sim_cfg()->fail_every  = every;
        sim_cfg()->fail_status = ((every % 2U) == 0U) ? IMU_HAL_TIMEOUT
                                                      : IMU_HAL_NACK;
        harness_run_ms(5000U);
        st = harness_status();
        TEST_ASSERT(status_sane(&st));
        if (1U == every)
        {
            /* 100 % loss: must end in a reported fault state. */
            TEST_ASSERT_MSG(st.state != IMU_STATE_MONITORING,
                            "every=%u", (unsigned)every);
            TEST_ASSERT(cap_count_type(IMU_EVT_IMU_FAULT) >= 1U);
        }
        else
        {
            /* Up to 50 % isolated NACKs are absorbed by the retries. */
            TEST_ASSERT_MSG(st.state == IMU_STATE_MONITORING,
                            "every=%u state=%s", (unsigned)every,
                            imu_state_name(st.state));
        }
    }
}

/* S1: extreme but well-formed values (saturation on every axis). */
static void p03_extreme_values(void)
{
    imu_status_t st;

    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    sim_cfg()->p_fn = NULL;
    sim_cfg()->const_accel.x = -100.0f;
    sim_cfg()->const_accel.y = 100.0f;
    sim_cfg()->const_accel.z = -100.0f;
    sim_cfg()->const_mag.x = 50.0f;
    harness_run_ms(2000U);
    st = harness_status();
    TEST_ASSERT(status_sane(&st));
    TEST_ASSERT(st.heading_quality == IMU_HEADING_INVALID);
    TEST_ASSERT(st.state != IMU_STATE_HUMP_PEAK);
}

/* S2: NULL / invalid arguments to every public entry point. */
static void p04_api_misuse(void)
{
    imu_terrain_t * p_sys = &g_h.sys;
    imu_status_t    st;
    imu_params_t    bad;
    imu_trace_entry_t tr[4];

    harness_setup();
    TEST_ASSERT_EQ(imu_terrain_init(NULL, NULL, NULL), IMU_ERR_PARAM);
    imu_params_default(&bad);
    bad.sample_period_ms = 1000000U;
    TEST_ASSERT_EQ(imu_terrain_init(p_sys, &bad, NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_init(p_sys, NULL, NULL), IMU_OK); /* defaults */
    imu_terrain_step(NULL, 0U);
    TEST_ASSERT_EQ(imu_terrain_command(NULL, IMU_CMD_RESET), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_command(p_sys, 0U), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_command(p_sys, 0x8000U), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_command(p_sys, 0xFFFFFFFFU), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_set_hint(NULL, NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_set_hint(p_sys, NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_get_status(NULL, &st), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_get_status(p_sys, NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_terrain_get_trace(NULL, tr, 4U), 0);
    TEST_ASSERT_EQ(imu_terrain_get_trace(p_sys, NULL, 4U), 0);
    imu_terrain_make_telemetry(NULL, NULL);
    TEST_ASSERT_EQ(imu_terrain_telemetry_json(NULL, NULL, 0U), -1);
    TEST_ASSERT(0 == strcmp(imu_state_name((imu_state_t)200), "INVALID"));
    TEST_ASSERT(0 == strcmp(imu_fault_name((imu_fault_t)200), "INVALID"));
    TEST_ASSERT(0 == strcmp(imu_motion_name((imu_motion_class_t)200),
                            "INVALID"));
    imu_params_default(NULL);
    TEST_ASSERT_EQ(imu_proc_init(NULL, NULL), IMU_ERR_PARAM);
    imu_proc_step(NULL, NULL, NULL, 0U, false, NULL);
    imu_proc_classify(NULL, 0U, IMU_HUMP_MONITORING, false, NULL);
    imu_proc_hf_accel(NULL, NULL, 0U, false, NULL);
    imu_cal_accel_add(NULL, NULL);
    imu_cal_mag_add(NULL, NULL);
    imu_cal_mag_apply(NULL, NULL, NULL);
}

/* S2: command flooding every cycle. */
static void p05_command_flood(void)
{
    uint32_t t;
    uint32_t mask = 0U;
    bool     b_sane = true;

    harness_setup();
    scn_add_segment(&g_h.scn, 3.0, 20.0, 0.2, 0.2, 0.0);
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    for (t = 0U; t < 15000U; t += 20U)
    {
        imu_status_t st;

        mask = (frand() & (IMU_CMD_ALL & ~IMU_CMD_STOP));
        if (0U != mask)
        {
            (void)imu_terrain_command(&g_h.sys, mask);
        }
        harness_run_ms(20U);
        st = harness_status();
        b_sane = b_sane && status_sane(&st);
    }
    TEST_ASSERT(b_sane);
    /* Event production is bounded by the period, not by the attacker. */
    TEST_ASSERT(g_cap.count <= (2U * (15000U / 20U)));

    /* A RESET storm keeps the subsystem out of MONITORING (it re-inits),
     * but never crashes and never produces false terrain events. */
    cap_reset(CAP_MAX);
    for (t = 0U; t < 3000U; t += 20U)
    {
        (void)imu_terrain_command(&g_h.sys, IMU_CMD_RESET);
        harness_run_ms(20U);
    }
    TEST_ASSERT(harness_status().state != IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
    harness_run_ms(4000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
}

/* S3: spoofed / absurd encoder hints. */
static void p06_hint_spoofing(void)
{
    imu_motion_hint_t h;
    uint32_t          t;
    uint32_t          rejected;

    harness_setup();
    scn_add_segment(&g_h.scn, 3.0, 20.0, 0.25, 0.25, 0.0);
    scn_add_hump(&g_h.scn, 0.6, 0.025, 0.10);
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));

    for (t = 0U; t < 6000U; t += 20U)
    {
        (void)memset(&h, 0, sizeof(h));
        h.b_valid       = true;
        h.b_accel_valid = (0U != (frand() & 1U));
        h.speed_mm_s    = (int32_t)frand();          /* mostly absurd */
        h.accel_mm_s2   = (int32_t)frand();
        h.timestamp_ms  = frand();                   /* random age   */
        (void)imu_terrain_set_hint(&g_h.sys, &h);
        harness_run_ms(20U);
    }
    rejected = harness_status().hints_rejected;
    TEST_ASSERT(rejected > 200U);
    TEST_ASSERT(status_sane(&g_h.sys.status));
    /* The real hump is still detected despite the hostile hint stream. */
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
}

/* S3: hint replayed from far in the past and "future" hints. */
static void p07_hint_replay(void)
{
    imu_motion_hint_t h;

    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    (void)memset(&h, 0, sizeof(h));
    h.b_valid = true;
    h.speed_mm_s = 500;
    h.timestamp_ms = g_h.now_ms - 60000U;
    TEST_ASSERT_EQ(imu_terrain_set_hint(&g_h.sys, &h), IMU_OK);
    harness_run_ms(500U);
    TEST_ASSERT_EQ(harness_status().motion_conf, IMU_CONF_LOW); /* ignored */
    h.timestamp_ms = g_h.now_ms + 60000U;
    TEST_ASSERT_EQ(imu_terrain_set_hint(&g_h.sys, &h), IMU_OK);
    harness_run_ms(500U);
    TEST_ASSERT_EQ(harness_status().motion_conf, IMU_CONF_LOW);
}

/* S4: random parameter blocks. */
static void p08_parameter_fuzz(void)
{
    uint32_t i;
    uint32_t accepted = 0U;

    for (i = 0U; i < 300U; i++)
    {
        imu_params_t p;
        imu_err_t    err;

        imu_params_default(&p);
        switch (frand() % 8U)
        {
            case 0U: p.hump_ascent_enter_deg = frandf(-10.0f, 60.0f); break;
            case 1U: p.impact_minor_g = frandf(-1.0f, 20.0f);         break;
            case 2U: p.axis_map[frand() % 3U] = (uint8_t)(frand() % 5U); break;
            case 3U: p.axis_sign[frand() % 3U] = (int8_t)((int)(frand() % 5U) - 2); break;
            case 4U: p.sample_period_ms = frand() % 200U;              break;
            case 5U: p.attitude_lpf_hz = frandf(-5.0f, 200.0f);       break;
            case 6U: p.wheelbase_mm = (0U != (frand() & 1U)) ? NAN
                                      : frandf(0.0f, 3000.0f);         break;
            default: p.turn_exit_dps = frandf(0.0f, 60.0f);           break;
        }

        harness_setup();
        g_h.params = p;
        err = harness_start();
        if (IMU_OK == err)
        {
            accepted++;
            TEST_ASSERT_EQ(imu_params_validate(&p), IMU_OK);
            harness_run_ms(3000U);
            TEST_ASSERT(status_sane(&g_h.sys.status));
        }
        else
        {
            TEST_ASSERT_EQ(err, IMU_ERR_PARAM);
        }
    }
    TEST_ASSERT(accepted > 20U);
    TEST_ASSERT(accepted < 300U);
}

/* S5: millisecond counter wrap-around during a hump. */
static void p09_time_wraparound(void)
{
    uint32_t const start = 0xFFFFFFFFU - 4500U;   /* wraps after 4.5 s */

    harness_setup();
    sim_set_time_ms(start);
    g_h.scn.t_s = (double)start * 0.001;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    /* Drive over a hump that straddles the wrap. */
    scn_add_segment(&g_h.scn, g_h.scn.t_s, 1.0e6, 0.25, 0.25, 0.0);
    scn_add_hump(&g_h.scn, 0.3, 0.025, 0.10);
    harness_run_ms(6000U);
    TEST_ASSERT(sim_time_ms() < 10000U);            /* really wrapped */
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMU_FAULT), 0);
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 1);
}

/* S5: time jumps forward / backwards (debugger halt, bad time source). */
static void p10_time_jumps(void)
{
    uint32_t k;

    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    for (k = 0U; k < 20U; k++)
    {
        uint32_t jump = (0U != (k & 1U)) ? 50000U : (uint32_t)(-30000);

        imu_terrain_step(&g_h.sys, g_h.now_ms + jump);
        imu_terrain_step(&g_h.sys, g_h.now_ms);
        TEST_ASSERT(status_sane(&g_h.sys.status));
    }
    harness_run_ms(2000U);
    TEST_ASSERT(status_sane(&g_h.sys.status));
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_HUMP_COMPLETED), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
}

/* S6: consumer never reads + storm of critical events. */
static void p11_event_storm(void)
{
    uint32_t k;

    harness_setup();
    g_h.params.impact_cooldown_ms = 10U;
    scn_add_segment(&g_h.scn, 3.0, 30.0, 0.2, 0.2, 0.0);
    for (k = 0U; k < SCN_MAX_IMPACTS; k++)
    {
        scn_add_impact(&g_h.scn, 4.0 + (0.3 * (double)k), 0.01, 3.5,
                       -1.0f, 0.0f, 0.0f);
    }
    cap_reset(0U);
    g_h.b_no_drain = true;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    for (k = 0U; k < 40U; k++)
    {
        (void)imu_terrain_command(&g_h.sys, IMU_CMD_CLEAR_FAULT);
        harness_run_ms(100U);
        TEST_ASSERT(imu_events_pending(&g_h.sys.disp)
                    <= IMU_EVENT_CRITICAL_PENDING);
    }
    TEST_ASSERT(g_cap.notified >= 1U);    /* direct path independent */
    /* Repeated hits keep "conditions normal" false: even with the MC
     * spamming CLEAR_FAULT the subsystem does not resume monitoring
     * while the vehicle is still being struck. */
    TEST_ASSERT_EQ(g_h.sys.fault, IMU_FAULT_COLLISION);
    TEST_ASSERT(status_sane(&g_h.sys.status));
}

/* S7: NaN / Inf everywhere. */
static void p12_nan_injection(void)
{
    imu_params_t        p;
    imu_proc_t          proc;
    imu_hump_t          hump;
    imu_proc_out_t      out;
    imu_hump_out_t      hout;
    imu_hump_input_t    hin;
    imu_accel_cal_acc_t acc;
    imu_mag_cal_acc_t   macc;
    uint32_t            i;
    float const         evil[] = { NAN, INFINITY, -INFINITY, 1e30f, -1e30f,
                                   0.0f };

    imu_params_default(&p);
    TEST_ASSERT_EQ(imu_proc_init(&proc, &p), IMU_OK);
    TEST_ASSERT_EQ(imu_hump_init(&hump, &p), IMU_OK);
    imu_cal_accel_begin(&acc);
    imu_cal_mag_begin(&macc);
    for (i = 0U; i < 600U; i++)
    {
        imu_vec3_t   v = { evil[i % 6U], evil[(i / 6U) % 6U],
                           evil[(i / 36U) % 6U] };
        imu_impact_t imp;

        imu_proc_hf_accel(&proc, &v, i * 5U, false, &imp);
        imu_proc_step(&proc, &v, &v, i * 20U, false, &out);
        imu_proc_classify(&proc, i * 20U, IMU_HUMP_MONITORING, false, &out);
        (void)memset(&hin, 0, sizeof(hin));
        hin.pitch_deg    = v.x;
        hin.vert_accel_g = v.y;
        hin.speed_mm_s   = v.z;
        hin.b_speed_valid = true;
        imu_hump_step(&hump, i * 20U, &hin, &hout);
        imu_cal_accel_add(&acc, &v);
        imu_cal_mag_add(&macc, &v);
    }
    TEST_ASSERT_EQ(hump.stats.valid_count, 0);
    TEST_ASSERT(acc.n < 600U);
    TEST_ASSERT(0 != isfinite(proc.heading_f_deg));
}

/* S1: magnetometer overflow stream does not fault the IMU. */
static void p13_mag_overflow_stream(void)
{
    harness_setup();
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    sim_cfg()->b_mag_overflow = true;
    harness_run_ms(3000U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    TEST_ASSERT_EQ(harness_status().heading_quality, IMU_HEADING_INVALID);
    TEST_ASSERT(!harness_status().b_turn_rate_valid);
    sim_cfg()->b_mag_overflow = false;
    harness_run_ms(500U);
    TEST_ASSERT(harness_status().heading_quality != IMU_HEADING_INVALID);
}

/* S1: frozen-output boundary (BVA): N-1 cycles OK, N cycles -> fault. */
static void p14_frozen_boundary(void)
{
    harness_setup();
    g_h.params.frozen_cycles = 10U;
    g_h.scn.accel_noise_g = 0.003;
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    sim_cfg()->b_frozen = true;
    harness_run_ms(9U * 20U);
    TEST_ASSERT_EQ(harness_status().state, IMU_STATE_MONITORING);
    harness_run_ms(3U * 20U);
    TEST_ASSERT_EQ(harness_status().fault, IMU_FAULT_FROZEN_DATA);
}

/* Static memory / message size budget (NFR14). */
static void p15_memory_budget(void)
{
    printf("      sizeof(imu_terrain_t)=%u  imu_status_t=%u  imu_event_t=%u "
           "imu_telemetry_t=%u imu_params_t=%u\n",
           (unsigned)sizeof(imu_terrain_t), (unsigned)sizeof(imu_status_t),
           (unsigned)sizeof(imu_event_t), (unsigned)sizeof(imu_telemetry_t),
           (unsigned)sizeof(imu_params_t));
    TEST_ASSERT(sizeof(imu_terrain_t) < 6144U);
    TEST_ASSERT(sizeof(imu_event_t) <= 40U);
    TEST_ASSERT(sizeof(imu_telemetry_t) <= 32U);
}

/* Long soak: 10 minutes of mixed driving, counts match ground truth. */
static void p16_soak(void)
{
    uint32_t k;
    double   t = 3.0;

    harness_setup();
    g_h.scn.vib_noise_g = 0.03;
    for (k = 0U; k < 8U; k++)
    {
        scn_add_segment(&g_h.scn, t, 40.0, 0.25, 0.25, 0.0);
        scn_add_hump(&g_h.scn, 2.0 + (10.0 * (double)k),
                     0.012 + (0.003 * (double)k), 0.10);
        t += 40.0;
        scn_add_segment(&g_h.scn, t, 1.5, 0.25, 0.25,
                        (0U != (k & 1U)) ? 60.0 : -60.0);
        t += 1.5;
    }
    TEST_ASSERT_EQ(harness_start(), IMU_OK);
    TEST_ASSERT(harness_until_monitoring(5000U));
    harness_run_ms((uint32_t)(t * 1000.0));
    TEST_ASSERT_EQ(harness_status().hump.valid_count, 8);
    TEST_ASSERT_EQ(harness_status().hump.max.index, 8);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMU_FAULT), 0);
    TEST_ASSERT_EQ(cap_count_type(IMU_EVT_IMPACT), 0);
    TEST_ASSERT(status_sane(&g_h.sys.status));
}

void suite_robustness(void)
{
    test_section("ROBUSTNESS / PENETRATION (adversarial inputs)");
    RUN_TEST(p01_i2c_garbage_fuzz);
    RUN_TEST(p02_failure_ratio_sweep);
    RUN_TEST(p03_extreme_values);
    RUN_TEST(p04_api_misuse);
    RUN_TEST(p05_command_flood);
    RUN_TEST(p06_hint_spoofing);
    RUN_TEST(p07_hint_replay);
    RUN_TEST(p08_parameter_fuzz);
    RUN_TEST(p09_time_wraparound);
    RUN_TEST(p10_time_jumps);
    RUN_TEST(p11_event_storm);
    RUN_TEST(p12_nan_injection);
    RUN_TEST(p13_mag_overflow_stream);
    RUN_TEST(p14_frozen_boundary);
    RUN_TEST(p15_memory_budget);
    RUN_TEST(p16_soak);
}
```

## `tests/imu/test_rtos.c`

```c
/**
 * @file    test_rtos.c
 * @brief   Integration tests of the uT-Kernel wrapper (imu_rtos.c) against a
 *          fake kernel: object attributes, task loop, locking, queue
 *          overflow mapping, direct notification, overruns, start/stop.
 */

#include <string.h>

#include <tk/tkernel.h>

#include "imu/imu_rtos.h"
#include "test_util.h"

static scenario_t g_rscn;

static void rtos_sim_setup(void)
{
    sim_reset();
    fake_tk_reset();
    scn_init(&g_rscn);
    sim_cfg()->p_fn  = scn_stimulus;
    sim_cfg()->p_ctx = &g_rscn;
}

static void r01_not_started(void)
{
    imu_status_t       st;
    imu_motion_class_t m;
    imu_confidence_t   c;

    rtos_sim_setup();
    TEST_ASSERT_EQ(imu_get_status(&st), IMU_ERR_NOT_READY);
    TEST_ASSERT_EQ(imu_get_motion(&m, &c), IMU_ERR_NOT_READY);
    TEST_ASSERT_EQ(imu_mission_start(), IMU_ERR_NOT_READY);
    TEST_ASSERT_EQ(imu_rtos_stop(), IMU_ERR_STATE);
    TEST_ASSERT_EQ(imu_rtos_terrain_event_q(), 0);
    TEST_ASSERT(!imu_rtos_is_running());
    TEST_ASSERT_EQ(imu_rtos_start(NULL), IMU_ERR_PARAM);
}

static void r02_creation_failure(void)
{
    imu_rtos_config_t cfg;

    rtos_sim_setup();
    imu_rtos_default_config(&cfg);
    g_fake_tk.cre_mbf_result = E_NOMEM;
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_ERR_RTOS);
    TEST_ASSERT(!imu_rtos_is_running());
    /* The mutex was created first, with priority inheritance. */
    TEST_ASSERT_EQ(g_fake_tk.n_cre_mtx, 1);
    TEST_ASSERT(0U != (g_fake_tk.cmtx.mtxatr & TA_INHERIT));

    {
        imu_params_t p;

        imu_params_default(&p);
        p.sample_period_ms = 0U;
        cfg.p_params = &p;
        TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_ERR_PARAM);
    }
}

static void r03_objects_and_attributes(void)
{
    imu_rtos_config_t cfg;

    rtos_sim_setup();
    imu_rtos_default_config(&cfg);
    cfg.mc_flag_id         = 11;
    cfg.collision_flag_ptn = 0x01U;
    cfg.fault_flag_ptn     = 0x02U;
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_OK);
    TEST_ASSERT(imu_rtos_is_running());
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_ERR_STATE);

    TEST_ASSERT_EQ(g_fake_tk.n_cre_tsk, 1);
    TEST_ASSERT_EQ(g_fake_tk.ctsk.itskpri, IMU_RTOS_DEFAULT_PRIORITY);
    TEST_ASSERT(0U != (g_fake_tk.ctsk.tskatr & TA_USERBUF));   /* no heap */
    TEST_ASSERT(0U != (g_fake_tk.ctsk.tskatr & TA_HLNG));
    TEST_ASSERT_EQ(g_fake_tk.ctsk.stksz, IMU_RTOS_STACK_BYTES);
    TEST_ASSERT(NULL != g_fake_tk.ctsk.bufptr);
    TEST_ASSERT_EQ(((uintptr_t)g_fake_tk.ctsk.bufptr) % 8U, 0);

    TEST_ASSERT_EQ(g_fake_tk.n_cre_mtx, 0);          /* reused */
    TEST_ASSERT(0U != (g_fake_tk.cmbf.mbfatr & TA_USERBUF));
    TEST_ASSERT_EQ(g_fake_tk.cmbf.maxmsz, (INT)sizeof(imu_event_t));
    TEST_ASSERT(g_fake_tk.cmbf.bufsz
                >= (SZ)(IMU_RTOS_EVENT_Q_DEPTH
                        * (sizeof(imu_event_t) + sizeof(INT))));
    TEST_ASSERT_EQ(g_fake_tk.ccyc.cyctim, IMU_CFG_SAMPLE_PERIOD_MS);
    TEST_ASSERT_EQ(g_fake_tk.n_sta_tsk, 1);
    TEST_ASSERT_EQ(g_fake_tk.n_sta_cyc, 1);
    TEST_ASSERT_EQ(imu_rtos_terrain_event_q(), 9);
}

static void r04_task_loop_and_api(void)
{
    imu_status_t          st;
    imu_telemetry_t       tele;
    imu_motion_class_t    m;
    imu_confidence_t      c;
    float                 p, r, t, h, rate;
    bool                  b, b2;
    imu_heading_quality_t q;
    imu_turn_dir_t        dir;
    imu_hump_phase_t      ph;
    imu_hump_meas_t       meas;
    imu_state_t           s;
    imu_fault_t           f;
    imu_rtos_stats_t      rs;

    /* Continue from r03: run 4 s of the real task body. */
    g_fake_tk.stop_after = 200U;
    fake_tk_run_task();
    TEST_ASSERT(!imu_rtos_is_running());
    TEST_ASSERT_EQ(g_fake_tk.n_stp_cyc, 1);
    TEST_ASSERT_EQ(g_fake_tk.locks, g_fake_tk.unlocks);   /* balanced */

    TEST_ASSERT_EQ(imu_get_status(&st), IMU_OK);
    TEST_ASSERT_EQ(st.state, IMU_STATE_STOPPED);           /* clean stop */
    TEST_ASSERT(st.b_calibrated);
    TEST_ASSERT(st.cycles >= 199U);
    TEST_ASSERT_EQ(imu_get_telemetry(&tele), IMU_OK);
    TEST_ASSERT_EQ(tele.hump_peak_cdeg, -1);
    TEST_ASSERT_EQ(imu_get_motion(&m, &c), IMU_OK);
    TEST_ASSERT_EQ(imu_get_tilt(&p, &r, &t, &b), IMU_OK);
    TEST_ASSERT(fabsf(p) < 1.0f);
    TEST_ASSERT_EQ(imu_get_heading(&h, &q, &rate, &b2, &dir), IMU_OK);
    TEST_ASSERT_EQ(imu_get_hump_state(&ph), IMU_OK);
    TEST_ASSERT_EQ(imu_get_last_hump(&meas), IMU_ERR_NO_DATA);
    TEST_ASSERT_EQ(imu_get_max_hump(&meas), IMU_ERR_NO_DATA);
    TEST_ASSERT_EQ(imu_get_health(&s, &f, &b), IMU_OK);
    TEST_ASSERT_EQ(imu_rtos_get_stats(&rs), IMU_OK);
    TEST_ASSERT(rs.cycles >= 199U);

    /* READY status event went through the message buffer. */
    TEST_ASSERT(g_fake_tk.mbf_sent >= 1U);
    TEST_ASSERT_EQ(g_fake_tk.mbf_last_size, (INT)sizeof(imu_event_t));

    TEST_ASSERT_EQ(imu_get_status(NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_get_tilt(NULL, &r, &t, &b), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_get_last_hump(NULL), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_set_motion_hint(NULL), IMU_ERR_PARAM);
}

static void r05_restart_hump_collision_overflow(void)
{
    imu_rtos_config_t cfg;
    imu_hump_meas_t   meas;
    imu_status_t      st;
    imu_rtos_stats_t  rs;

    rtos_sim_setup();
    scn_add_segment(&g_rscn, 3.0, 30.0, 0.25, 0.25, 0.0);
    scn_add_hump(&g_rscn, 0.3, 0.025, 0.10);
    scn_add_impact(&g_rscn, 6.5, 0.015, 3.0, -1.0f, 0.0f, 0.0f);
    imu_rtos_default_config(&cfg);
    cfg.mc_flag_id         = 11;
    cfg.collision_flag_ptn = 0x01U;
    cfg.fault_flag_ptn     = 0x02U;
    g_fake_tk.mbf_capacity = 3U;       /* tiny queue, never drained */
    g_fake_tk.can_wup_result = 1;      /* every cycle overran once */
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_OK);   /* restart */
    TEST_ASSERT_EQ(g_fake_tk.n_cre_tsk, 0);         /* objects reused */
    TEST_ASSERT_EQ(imu_mission_start(), IMU_OK);
    g_fake_tk.stop_after = 400U;                    /* 8 s */
    fake_tk_run_task();

    TEST_ASSERT_EQ(imu_get_max_hump(&meas), IMU_OK);
    TEST_ASSERT_EQ(meas.index, 1);
    TEST_ASSERT_EQ(imu_get_status(&st), IMU_OK);
    TEST_ASSERT(st.events.dropped[IMU_EVT_HUMP_COMPLETED]
                + st.events.dropped[IMU_EVT_HUMP_PEAK]
                + st.events.dropped[IMU_EVT_MOTION] >= 1U);
    TEST_ASSERT(g_fake_tk.flg_calls >= 1U);           /* direct path */
    TEST_ASSERT(0U != (g_fake_tk.flg_ptn & 0x01U));
    TEST_ASSERT_EQ(g_fake_tk.flg_id, 11);
    TEST_ASSERT_EQ(g_fake_tk.mbf_level, 3);           /* bounded */
    TEST_ASSERT_EQ(imu_rtos_get_stats(&rs), IMU_OK);
    TEST_ASSERT(rs.overruns >= 390U);
}

static void r06_lock_contention_and_timeouts(void)
{
    imu_rtos_config_t  cfg;
    imu_status_t       st;
    imu_motion_class_t m;
    imu_confidence_t   c;
    imu_rtos_stats_t   rs;
    imu_motion_hint_t  h;

    rtos_sim_setup();
    imu_rtos_default_config(&cfg);
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_OK);
    g_fake_tk.b_lock_fail = true;
    TEST_ASSERT_EQ(imu_get_status(&st), IMU_ERR_BUSY);
    TEST_ASSERT_EQ(imu_get_motion(&m, &c), IMU_ERR_BUSY);
    TEST_ASSERT_EQ(imu_clear_fault(), IMU_ERR_BUSY);
    (void)memset(&h, 0, sizeof(h));
    TEST_ASSERT_EQ(imu_set_motion_hint(&h), IMU_ERR_BUSY);
    g_fake_tk.b_slp_timeout = true;           /* cyclic handler missing */
    g_fake_tk.stop_after    = 50U;
    fake_tk_run_task();                       /* must not deadlock */
    g_fake_tk.b_lock_fail = false;
    TEST_ASSERT_EQ(imu_rtos_get_stats(&rs), IMU_OK);
    TEST_ASSERT_EQ(rs.cycles, 0);             /* publication was skipped */
    TEST_ASSERT(rs.lock_timeouts >= 49U);

    /* Commands posted while running are applied at the next cycle. */
    rtos_sim_setup();
    TEST_ASSERT_EQ(imu_rtos_start(&cfg), IMU_OK);
    g_fake_tk.b_slp_timeout = true;
    TEST_ASSERT_EQ(imu_request_reset(), IMU_OK);
    TEST_ASSERT_EQ(imu_recalibrate(), IMU_OK);
    TEST_ASSERT_EQ(imu_mag_cal_begin(), IMU_OK);
    TEST_ASSERT_EQ(imu_mag_cal_end(), IMU_OK);
    h.b_valid = true;
    h.speed_mm_s = 100;
    h.timestamp_ms = 0U;
    TEST_ASSERT_EQ(imu_set_motion_hint(&h), IMU_OK);
    g_fake_tk.stop_after = 150U;
    fake_tk_run_task();
    TEST_ASSERT_EQ(imu_rtos_get_stats(&rs), IMU_OK);
    TEST_ASSERT(rs.missed_wakeups >= 140U);
    TEST_ASSERT(rs.max_cycle_us < 20000U);
}

void suite_rtos(void)
{
    test_section("INTEGRATION: uT-Kernel wrapper (fake kernel)");
    RUN_TEST(r01_not_started);
    RUN_TEST(r02_creation_failure);
    RUN_TEST(r03_objects_and_attributes);
    RUN_TEST(r04_task_loop_and_api);
    RUN_TEST(r05_restart_hump_collision_overflow);
    RUN_TEST(r06_lock_contention_and_timeouts);
}
```

## `tests/imu/test_main.c`

```c
/**
 * @file    test_main.c
 * @brief   HOST-ONLY test runner for the Buddy 4 IMU / Terrain subsystem.
 *
 * Usage: ./buddy4_tests [-v] [-f substring]
 *        -v prints every emitted event, -f runs only matching tests.
 * Exit code: 0 if all tests passed, 1 otherwise.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "test_util.h"


/** Informative only: host execution time of one sampling period. */
static void perf_report(void)
{
    clock_t  c0;
    clock_t  c1;
    uint32_t n = 0U;

    test_section("PERFORMANCE (host, informative - NOT a Pico W figure)");
    harness_setup();
    g_h.scn.vib_noise_g = 0.02;
    scn_add_segment(&g_h.scn, 3.0, 100.0, 0.25, 0.25, 0.0);
    scn_add_hump(&g_h.scn, 1.0, 0.02, 0.10);
    (void)harness_start();
    (void)harness_until_monitoring(5000U);
    c0 = clock();
    while (n < 5000U)
    {
        harness_run_ms(20U);
        n++;
    }
    c1 = clock();
    printf("  %u cycles, %.2f us per cycle incl. simulator (host CPU)\n",
           (unsigned)n, 1e6 * (double)(c1 - c0) / CLOCKS_PER_SEC / n);
}

int main(int argc, char ** argv)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "-v"))
        {
            g_verbose = true;
        }
        else if ((0 == strcmp(argv[i], "-f")) && ((i + 1) < argc))
        {
            g_filter = argv[i + 1];
            i++;
        }
        else
        {
            printf("usage: %s [-v] [-f name-substring]\n", argv[0]);
            return (2);
        }
    }

    suite_unit_driver();
    suite_unit_math_cal();
    suite_unit_processing();
    suite_unit_hump();
    suite_unit_events();
    suite_blackbox();
    suite_robustness();
    suite_rtos();
    if (NULL == g_filter)
    {
        perf_report();
    }

    printf("\n==================================================\n");
    printf("  %u tests, %u assertions, %u FAILED\n", (unsigned)g_tests_run,
           (unsigned)g_asserts, (unsigned)g_tests_failed);
    printf("==================================================\n");

    return ((0U == g_tests_failed) ? 0 : 1);
}
```

## `tests/imu/replay_main.c`

```c
/**
 * @file    replay_main.c
 * @brief   HOST-ONLY offline replay tool: feeds RAW sensor logs recorded with
 *          the bring-up firmware ('w' key) through the production algorithms
 *          (driver -> calibration -> processing -> hump -> state machine)
 *          so that thresholds can be tuned on the PC with real data.
 *
 *   ./buddy4_replay log.txt [key=value ...]
 *
 * Input lines (other lines are ignored, so a whole terminal capture works):
 *   RAW,t_ms,ax,ay,az,mx,my,mz,ovf        (sensor frame, g and gauss)
 *
 * Optional parameter overrides (subset), e.g.:
 *   wheelbase_mm=150 hump_ascent_enter_deg=2.5 impact_minor_g=1.0
 *   attitude_lpf_hz=6 tilt_threshold_deg=12
 *
 *   ./buddy4_replay --gen > synthetic.txt   writes a synthetic RAW log
 *                                           (stationary, 3 humps, turn).
 *
 * The log is sampled at the cycle rate (50 Hz, one accelerometer sample per
 * cycle), so impact detection results are NOT representative in replay.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_util.h"

#define REPLAY_MAX_ROWS  (200000U)

typedef struct
{
    uint32_t   t_ms;
    imu_vec3_t a;
    imu_vec3_t m;
} replay_row_t;

static replay_row_t * g_rows;
static uint32_t       g_n_rows;
static uint32_t       g_cursor;

static void replay_stimulus(void * p_ctx, double t_s, imu_vec3_t * p_a,
                            imu_vec3_t * p_m)
{
    uint32_t const t_ms = (uint32_t)(t_s * 1000.0);

    (void)p_ctx;
    while (((g_cursor + 1U) < g_n_rows) && (g_rows[g_cursor + 1U].t_ms <= t_ms))
    {
        g_cursor++;
    }
    *p_a = g_rows[g_cursor].a;
    *p_m = g_rows[g_cursor].m;
}

static int replay_override(imu_params_t * p, char const * p_kv)
{
    char         key[64];
    double       v;
    char const * p_eq = strchr(p_kv, '=');

    if ((NULL == p_eq) || ((size_t)(p_eq - p_kv) >= sizeof(key)))
    {
        return (-1);
    }
    (void)memcpy(key, p_kv, (size_t)(p_eq - p_kv));
    key[p_eq - p_kv] = '\0';
    v = atof(p_eq + 1);

#define OVR_F(name) if (0 == strcmp(key, #name)) { p->name = (float)v; return 0; }
#define OVR_U(name) if (0 == strcmp(key, #name)) { p->name = (uint32_t)v; return 0; }
    OVR_F(wheelbase_mm) OVR_F(hump_ascent_enter_deg) OVR_F(hump_min_peak_deg)
    OVR_F(hump_descent_enter_deg) OVR_F(hump_level_band_deg)
    OVR_F(hump_peak_drop_deg) OVR_F(attitude_lpf_hz) OVR_F(heading_lpf_hz)
    OVR_F(tilt_threshold_deg) OVR_F(turn_enter_dps) OVR_F(turn_exit_dps)
    OVR_F(impact_minor_g) OVR_F(impact_critical_g) OVR_F(accel_enter_g)
    OVR_F(accel_exit_g) OVR_F(cal_max_noise_g) OVR_F(hump_height_gain)
    OVR_F(hump_height_offset_mm)
    OVR_U(hump_ascent_min_ms) OVR_U(hump_crest_max_ms) OVR_U(hump_max_event_ms)
    OVR_U(hump_rearm_level_ms) OVR_U(turn_min_duration_ms)
    OVR_U(tilt_min_duration_ms) OVR_U(cal_samples)
#undef OVR_F
#undef OVR_U
    return (-1);
}

static void replay_generate(void)
{
    scenario_t s;
    uint32_t   t;

    scn_init(&s);
    s.vib_noise_g = 0.02;
    scn_add_segment(&s, 3.0, 12.0, 0.25, 0.25, 0.0);
    scn_add_segment(&s, 15.0, 1.5, 0.25, 0.25, -60.0);
    scn_add_segment(&s, 16.5, 5.0, 0.25, 0.25, 0.0);
    scn_add_hump(&s, 0.5, 0.015, 0.10);
    scn_add_hump(&s, 1.5, 0.030, 0.10);
    scn_add_hump(&s, 2.5, 0.020, 0.10);
    printf("# synthetic Buddy 4 RAW log: humps 15/30/20 mm, L=140 mm\n");
    for (t = 0U; t < 22000U; t += 20U)
    {
        imu_vec3_t a;
        imu_vec3_t m;

        scn_stimulus(&s, (double)t * 0.001, &a, &m);
        printf("RAW,%u,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,0\n", (unsigned)t,
               (double)a.x, (double)a.y, (double)a.z, (double)m.x,
               (double)m.y, (double)m.z);
    }
}

int main(int argc, char ** argv)
{
    FILE *       p_f;
    char         line[256];
    int          i;
    imu_status_t st;

    if ((argc >= 2) && (0 == strcmp(argv[1], "--gen")))
    {
        replay_generate();
        return (0);
    }
    if (argc < 2)
    {
        printf("usage: %s log.txt [param=value ...] | --gen\n", argv[0]);
        return (2);
    }

    g_rows = (replay_row_t *)calloc(REPLAY_MAX_ROWS, sizeof(replay_row_t));
    p_f    = fopen(argv[1], "r");
    if ((NULL == g_rows) || (NULL == p_f))
    {
        printf("cannot open %s\n", argv[1]);
        return (2);
    }
    while ((NULL != fgets(line, sizeof(line), p_f))
           && (g_n_rows < REPLAY_MAX_ROWS))
    {
        unsigned t;
        float    ax, ay, az, mx, my, mz;
        char *   p_raw = strstr(line, "RAW,");

        if ((NULL != p_raw)
            && (7 == sscanf(p_raw, "RAW,%u,%f,%f,%f,%f,%f,%f", &t, &ax, &ay,
                            &az, &mx, &my, &mz)))
        {
            replay_row_t * p_r = &g_rows[g_n_rows];

            p_r->t_ms = t;
            p_r->a.x = ax; p_r->a.y = ay; p_r->a.z = az;
            p_r->m.x = mx; p_r->m.y = my; p_r->m.z = mz;
            g_n_rows++;
        }
    }
    (void)fclose(p_f);
    if (g_n_rows < 10U)
    {
        printf("no RAW rows found\n");
        return (2);
    }

    harness_setup();
    for (i = 2; i < argc; i++)
    {
        if (0 != replay_override(&g_h.params, argv[i]))
        {
            printf("unknown parameter: %s\n", argv[i]);
            return (2);
        }
    }
    g_verbose = true;                 /* print every event */
    sim_set_time_ms(g_rows[0].t_ms);
    if (IMU_OK != harness_start())
    {
        printf("invalid parameter set\n");
        return (2);
    }
    sim_cfg()->p_fn  = replay_stimulus;
    sim_cfg()->p_ctx = NULL;
    harness_run_ms(g_rows[g_n_rows - 1U].t_ms - g_rows[0].t_ms);

    st = harness_status();
    printf("\nREPLAY SUMMARY: %u rows, state=%s, humps valid=%u rejected=%u\n",
           (unsigned)g_n_rows, imu_state_name(st.state),
           (unsigned)st.hump.valid_count, (unsigned)st.hump.rejected_count);
    if (st.hump.b_has_max)
    {
        printf("  max hump #%u: peak %.2f deg, est %.1f mm (conf %u)\n",
               (unsigned)st.hump.max.index,
               (double)st.hump.max.peak_pitch_deg,
               (double)st.hump.max.height_est_mm,
               (unsigned)st.hump.max.height_conf);
    }
    printf("  calibration: g_ref=%.4f noise=%.4f g mount_tilt=%.2f deg\n",
           (double)st.cal_g_ref, (double)st.cal_noise_g,
           (double)st.cal_mount_tilt_deg);
    free(g_rows);
    return (0);
}
```
