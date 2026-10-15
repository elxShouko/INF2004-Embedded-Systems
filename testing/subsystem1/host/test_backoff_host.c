/* Host (PC) unit tests for net_backoff.h. Build and run with run_host_tests.sh. */
#include <stdio.h>

#include "../../../lib/liblwip/sysdepend/pico_rp2040/net_backoff.h"

static int g_checks = 0;
static int g_failures = 0;

static void
check (int b_ok, const char * p_name)
{
    g_checks++;
    if (!b_ok)
    {
        g_failures++;
        printf ("  FAIL %s\n", p_name);
    }
}

int
main (void)
{
    uint32_t delay = NET_BACKOFF_INITIAL_MS;
    uint32_t expected[] = { 2000u, 4000u, 8000u, 8000u, 8000u };
    unsigned k;

    printf ("T-backoff-1 sequence: %u", (unsigned) delay);
    for (k = 0u; k < 5u; k++)
    {
        delay = net_backoff_next_ms (delay);
        printf (" -> %u", (unsigned) delay);
        check (delay == expected[k], "FR19 step");
    }
    printf ("\n");
    check (net_backoff_next_ms (0u) == 0u, "zero stays zero");
    check (net_backoff_next_ms (7999u) == 8000u, "7999 capped to 8000");
    check (net_backoff_next_ms (4000u) == 8000u, "4000 doubles to cap");
    check (net_backoff_next_ms (4294967295u) == 8000u, "huge value is capped");

    check (net_backoff_time_reached (100u, 100u), "equal");
    check (!net_backoff_time_reached (99u, 100u), "before");
    check (net_backoff_time_reached (5u, 4294967290u), "after wrap");
    check (!net_backoff_time_reached (4294967290u, 5u), "before wrap");
    check (net_backoff_time_elapsed (3u, 4294967295u, 4u), "elapsed across wrap");
    check (!net_backoff_time_elapsed (2u, 4294967295u, 4u), "not yet across wrap");

    printf ("\n%d checks, %d failed\n", g_checks, g_failures);
    return (0 == g_failures) ? 0 : 1;
}
