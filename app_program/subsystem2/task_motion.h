/** @file task_motion.h
 * @brief Buddy 2 startup hooks, real micro T-Kernel only.
 */
#ifndef TASK_MOTION_H
#define TASK_MOTION_H
#if defined(MOTION_RTOS_HOST_TEST)
#include "test_rtos.h"
#else
#include "subsystem2.h"
#endif
ER task_motion_init (void);
ER task_motion_start (void);
#endif /* TASK_MOTION_H */

/*** end of file ***/

