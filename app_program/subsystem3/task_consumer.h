/** @file task_consumer.h
 * @brief Declare the retained template's Subsystem 3 entry point.
 * @note S3-DEV-01 preserves required template names and data interfaces.
 */
#ifndef TASK_CONSUMER_H
#define TASK_CONSUMER_H

#include "subsystem3.h"

/** @brief Consume demo samples while retaining the S2/S4 interfaces. */
IMPORT void consumer_task (INT stacd, void * p_exinf);

#endif

/*** End of file. ***/

