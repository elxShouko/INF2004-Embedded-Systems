/**
 * @file s1_fault.h
 * @brief Buddy 1: the shared fault bits (one bit per fault, 8 bits in total).
 *
 * The mask is carried in telemetry bits 23 to 30
 * (docs/buddy1/TELEMETRY_COMMAND_CONTRACT.md, section 6.5). In the firmware it
 * is held in ONE kernel event flag and changed only with tk_set_flg and
 * tk_clr_flg, which are atomic across both cores. Nobody may read the mask,
 * change a copy and write it back: two such writers would lose each other's
 * bits on the Cortex-M0+, which has no atomic read-modify-write instruction.
 *
 * Each bit has one owner who sets and clears it. Bits whose owner never sets
 * them stay 0 in the telemetry.
 *
 * NOTE: No kernel dependency, so it is also used by the PC tests.
 */

#ifndef S1_FAULT_H
#define S1_FAULT_H

#include <stdint.h>

#define S1_FAULT_BITS 8u

#define S1_FAULT_MOTOR_STALL 0x01u   /**< Buddy 2. */
#define S1_FAULT_ENCODER 0x02u       /**< Buddy 2. */
#define S1_FAULT_IMU 0x04u           /**< Buddy 4. */
#define S1_FAULT_NO_ECHO 0x08u       /**< Buddy 5. */
#define S1_FAULT_WIFI_LOSS 0x10u     /**< Buddy 1: link or broker lost. */
#define S1_FAULT_ESTOP_LATCHED 0x20u /**< Buddy 1: EMERGENCY_STOP latched. */
#define S1_FAULT_LINE_SENSOR 0x40u   /**< Buddy 3. */
#define S1_FAULT_BARCODE 0x80u       /**< Buddy 3. */

/** All defined bits. A mask with any other bit set is invalid. */
#define S1_FAULT_ALL 0xFFu

/** The bits Buddy 1 sets and clears itself. */
#define S1_FAULT_OWNED_BY_S1 (S1_FAULT_WIFI_LOSS | S1_FAULT_ESTOP_LATCHED)

#endif /* S1_FAULT_H */

/*** end of file ***/