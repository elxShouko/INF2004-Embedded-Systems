/** @file measure_representation.c @brief sizeof evidence, never a wire format. */
#include "../../../app_program/subsystem5/s5_live.h"
#if !defined(S5_SIZE_ONLY)
#include <stdio.h>
#endif

/* ARM object symbol lengths measure the target ABI without executing firmware. */
#define S5_SIZE(type) const unsigned char g_size_##type[sizeof(type)] = {0}
S5_SIZE(s5_scan_sample_t);
S5_SIZE(s5_scan_context_t);
S5_SIZE(s5_obstacle_profile_t);
S5_SIZE(s5_obstacle_plan_t);
S5_SIZE(s5_clear_box_t);
S5_SIZE(s5_clearance_t);
S5_SIZE(s5_route_config_t);
S5_SIZE(s5_runtime_config_t);
S5_SIZE(s5_runtime_input_t);
S5_SIZE(s5_runtime_status_t);
S5_SIZE(s5_runtime_events_t);
S5_SIZE(s5_runtime_t);
S5_SIZE(s5_acquire_config_t);
S5_SIZE(s5_acquire_io_t);
S5_SIZE(s5_acquire_t);
S5_SIZE(drv_s5_t);
S5_SIZE(s5_live_config_t);
S5_SIZE(s5_live_status_t);

#if !defined(S5_SIZE_ONLY)
#define S5_PRINT(type) (void)printf(#type " %zu\n", sizeof(type))
int main (void)
{
    S5_PRINT(s5_scan_sample_t);
    S5_PRINT(s5_scan_context_t);
    S5_PRINT(s5_obstacle_profile_t);
    S5_PRINT(s5_obstacle_plan_t);
    S5_PRINT(s5_clear_box_t);
    S5_PRINT(s5_clearance_t);
    S5_PRINT(s5_route_config_t);
    S5_PRINT(s5_runtime_config_t);
    S5_PRINT(s5_runtime_input_t);
    S5_PRINT(s5_runtime_status_t);
    S5_PRINT(s5_runtime_events_t);
    S5_PRINT(s5_runtime_t);
    S5_PRINT(s5_acquire_config_t);
    S5_PRINT(s5_acquire_io_t);
    S5_PRINT(s5_acquire_t);
    S5_PRINT(drv_s5_t);
    S5_PRINT(s5_live_config_t);
    S5_PRINT(s5_live_status_t);
    return (0);
}
#endif
