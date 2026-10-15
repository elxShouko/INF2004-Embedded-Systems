/* Calls the on-target test entry point on the PC, after creating the objects. */
#include "../../../app_program/subsystem1/s1_net.h"
extern void subsystem1_test (void);
int main (void)
{
    s1_net_init ();
    subsystem1_test ();
    return 0;
}
