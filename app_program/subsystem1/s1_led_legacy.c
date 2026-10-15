/**
 * @file s1_led_legacy.c
 * @brief The template's LED heartbeat example, moved here unchanged in
 * behaviour. See s1_led_legacy.h.
 */

#include "s1_led_legacy.h"

#include "subsystem1.h"

EXPORT ID tid_led; /* IF-05, see subsystem1.h */
EXPORT ID cyc_led;

/* Create the cyclic handler. It is created stopped and started later, once
 * the task it wakes exists. */
ER
s1_led_legacy_init (void)
{
    T_CCYC ccyc   = { 0 };
    ER     result = E_OK;

    ccyc.cycatr = TA_HLNG;
    ccyc.cychdr = led_cyclic_handler;
    ccyc.cyctim = S1_TICK_MS; /* period, ms */
    ccyc.cycphs = S1_TICK_MS; /* delay before the first call, ms */
    cyc_led     = tk_cre_cyc (&ccyc);
    if (cyc_led < E_OK)
    {
        result = cyc_led;
    }
    return result;
}

/* Start the LED task, then the cyclic handler. */
ER
s1_led_legacy_start (void)
{
    ER result = E_OK;

    tid_led = app_start_task (led_task, S1_PRI_LED, 0, "led_task");
    if (tid_led < E_OK)
    {
        result = tid_led;
    }
    else
    {
        result = tk_sta_cyc (cyc_led);
    }
    return result;
}

/*** end of file ***/
