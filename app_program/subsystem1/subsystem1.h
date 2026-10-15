/**
 * @file subsystem1.h
 * @brief Subsystem 1 (Buddy 1): Wi-Fi, MQTT telemetry, remote commands and
 * connection recovery.
 *
 * This header is subsystem 1's PUBLIC interface. Other subsystems include it
 * as "../subsystem1/subsystem1.h" and use only what is declared here and in
 * the headers it includes:
 *   - s1_net.h    the API: s1_net_telemetry_send(), s1_net_command_receive()
 *   - s1_codec.h  the data types: telemetry frame, command
 *
 * RULE: Application tasks never call cyw43, lwIP or MQTT functions. All
 * network code runs in the Wi-Fi service task (TEAM.md section 3).
 */

#ifndef SUBSYSTEM1_H
#define SUBSYSTEM1_H

#include "../app.h"
#include "s1_codec.h"
#include "s1_net.h"

/** Priority of the telemetry simulator task (band 12-13 in TEAM.md). */
#define S1_PRI_SIM 13

/* ---------------------------------------------------------------------- */
/* Legacy template example, kept ONLY so subsystem 4's example (IF-05)    */
/* still builds. Delete this block, s1_led_legacy.[ch] and task_led.c     */
/* once subsystem 4 no longer uses tid_led. These names are the            */
/* template's own and are exempt from the BARR-C naming check.            */
/* ---------------------------------------------------------------------- */
#define S1_PRI_LED 12  /* priority band 12-13 (TEAM.md) */
#define S1_LED_PIN 19  /* GP19: LED + 330R resistor to GND */
#define S1_TICK_MS 250 /* LED toggles every tick */

/* IF-05: the LED task's ID, so subsystem 4 may pause/resume it */
/* NOLINTNEXTLINE(readability-identifier-naming): template-owned name (IF-05) */
IMPORT ID tid_led;

/* Private to subsystem 1, shared between its own .c files */
/* NOLINTNEXTLINE(readability-identifier-naming): template-owned name */
IMPORT ID   cyc_led;
IMPORT void led_task (INT stacd, void * exinf);
IMPORT void led_cyclic_handler (void * exinf);

#endif /* SUBSYSTEM1_H */

/*** end of file ***/
