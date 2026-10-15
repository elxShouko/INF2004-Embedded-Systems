/*
 *	Buddy 5's host-testable obstacle calculations are not yet wired to
 *	verified sensors, RTOS tasks, or agreed subsystem interfaces.
 */

#include "subsystem5.h"

EXPORT ER subsystem5_init(void)
{
	return E_OK;
}

EXPORT ER subsystem5_start(void)
{
	return E_NOSPT;
}
