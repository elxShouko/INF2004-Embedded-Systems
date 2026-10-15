/** @file vehicle_rt.h @brief Shared integration lifecycle; initial task only. */
#ifndef VEHICLE_RT_H
#define VEHICLE_RT_H
#include "app.h"
#include "vehicle.h"
#include "subsystem3/member3_navigation.h"

typedef struct
{
    vehicle_config_t mission;
    wheel_loop_config_t motion;
    s3_config_t navigation;
} vehicle_setup_t;

/* No default calibration or automatic enable. setup is optional, pre-start,
 * copied once and requires a separately authorised hardware procedure. No
 * caller in usermain supplies it. Current provider blockers still prevent RUN.
 */
ER vehicle_runtime_init (void);
ER vehicle_runtime_setup (vehicle_setup_t const * p_setup);
ER vehicle_runtime_start (uint32_t started_mask);
ER vehicle_runtime_status (vehicle_t * p_status);
#endif /* VEHICLE_RT_H */
/*** end of file ***/
