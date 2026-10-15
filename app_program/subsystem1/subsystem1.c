/**
 * @file subsystem1.c
 * @brief Subsystem 1 (Buddy 1): creates the kernel objects and starts the
 * tasks. The work itself is in s1_net.c, s1_codec.c and the MQTT client in
 * lib/liblwip (mqtt_utk.c).
 */

#include "subsystem1.h"

#include "s1_led_legacy.h"
#include "s1_sim.h"

/* Create every kernel object that subsystem 1 owns. */
ER
subsystem1_init (void)
{
    ER result = s1_net_init();

    if (E_OK == result)
    {
        result = s1_sim_init();
    }
    if (E_OK == result)
    {
        result = s1_led_legacy_init();
    }
    return result;
}

/* Start the tasks and hand the hook table to the MQTT client. */
ER
subsystem1_start (void)
{
    ER result = s1_net_start();

    if (E_OK == result)
    {
        result = s1_sim_start();
    }
    if (E_OK == result)
    {
        result = s1_led_legacy_start();
    }
    return result;
}

/*** end of file ***/
