/*
 *	Buddy 5 live owner. Unconfigured startup creates objects only;
 *	physical GPIO activation requires an explicit validated pre-start profile.
 */

#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include "subsystem5.h"
#include "s5_live.h"

EXPORT ER subsystem5_init(void)
{
	return s5_live_init();
}

EXPORT ER subsystem5_start(void)
{
	return s5_live_start();
}
