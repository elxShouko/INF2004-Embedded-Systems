/** @file vehicle_profile.c @brief Explicit startup source for the actual robot. */
#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include "vehicle_profile.h"
#if APP_ROBOT
#include <robot_profile.h>

ER vehicle_profile_startup (void)
{
    return (vehicle_runtime_robot_prepare(&g_vehicle_profile));
}
#endif

/*** end of file ***/
