/** @file drv_ir_rtos.h
 * @brief Member 3 drv ir rtos module.
 * @note See BARR_COMPLIANCE.md for compatibility exceptions under review.
 */

#ifndef DRV_IR_RTOS_H
#define DRV_IR_RTOS_H

#include "../app.h"
#include "line/ir_input.h"

/** @brief Configure only the three owned IR GPIOs as digital inputs. */
ER s3_ir_init (void);
/** @brief Snapshot the owned GPIO levels and their observation timestamp. */
void s3_ir_read (ir_sample_t * p_sample);
/** @brief Read a rollover-consistent RP2040 microsecond timestamp. */
uint64_t s3_clock_us (void);

#endif

/*** End of file. ***/

