/** @file vehicle_profile.h @brief One immutable production robot profile. */
#ifndef VEHICLE_PROFILE_H
#define VEHICLE_PROFILE_H
#include "vehicle_rt.h"
#include "subsystem2/bench_motion.h"

typedef enum
{
    VEHICLE_STAGE_BENCH = 0, VEHICLE_STAGE_LINE, VEHICLE_STAGE_BARCODE
} vehicle_stage_t;

typedef struct
{
    vehicle_setup_t setup;
    motion_bench_config_t bench;
    uint32_t run_limit_ms; /* Explicit operator test limit; 0 forbids a run. */
    uint8_t stage : 2;
} vehicle_robot_profile_t;

#if APP_ROBOT
ER vehicle_profile_startup (void);
/* Initial task only; no motor/servo activation in this call. */
ER vehicle_runtime_robot_prepare (vehicle_robot_profile_t const * p_profile);
#endif
#endif /* VEHICLE_PROFILE_H */

/*** end of file ***/
