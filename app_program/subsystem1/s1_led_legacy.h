/**
 * @file s1_led_legacy.h
 * @brief The template's LED heartbeat example, kept so subsystem 4 builds.
 *
 * NOTE: Remove with task_led.c when subsystem 4 drops its use of tid_led.
 */

#ifndef S1_LED_LEGACY_H
#define S1_LED_LEGACY_H

#include "../app.h"

/** @brief Create the cyclic handler of the LED example. */
ER s1_led_legacy_init (void);

/** @brief Start the LED task and its cyclic handler. */
ER s1_led_legacy_start (void);

#endif /* S1_LED_LEGACY_H */

/*** end of file ***/
