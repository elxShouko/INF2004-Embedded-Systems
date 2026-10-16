/** @file vehicle_rt.h @brief Shared integration lifecycle; initial task only. */
#ifndef VEHICLE_RT_H
#define VEHICLE_RT_H
#include "app.h"
#include "vehicle.h"
#include "subsystem5/s5_live.h"
#include "subsystem3/member3_navigation.h"

typedef struct
{
    vehicle_config_t mission;
    wheel_loop_config_t motion;
    s3_config_t navigation;
    s5_live_config_t obstacle;
    bool b_obstacle_requested;
} vehicle_setup_t;

/* No default calibration or automatic enable. setup is optional, pre-start,
 * copied once and requires a separately authorised hardware procedure. No
 * caller in usermain supplies it. Hardware calibration still gates RUN.
 * APP_TEST=1 deliberately leaves the controller dormant to preserve test queues.
 */
ER vehicle_runtime_init (void);
ER vehicle_runtime_setup (vehicle_setup_t const * p_setup);
ER vehicle_runtime_start (uint32_t started_mask);
ER vehicle_runtime_status (vehicle_t * p_status);
/* Task-context copied certificate, only while stopped awaiting a scan route.
 * Does not create/verify clearance. The owner validates geometry/epoch/expiry.
 * No default caller; use only an independently validated evidence source.
 */
ER vehicle_runtime_clearance (s5_clearance_t const * p_clearance);
#endif /* VEHICLE_RT_H */
/*** end of file ***/
