/** @file motion_bus.h
 * @brief Fixed one-slot copied requests; caller holds lock.
 */
#ifndef MOTION_BUS_H
#define MOTION_BUS_H
#include "motion_run.h"
#include "drv_encoder.h"
typedef enum
{
    MOTION_BUS_CONFIG = 0, MOTION_BUS_ENABLE, MOTION_BUS_COMMAND, MOTION_BUS_ACK
} motion_bus_kind_t;
typedef struct
{
    wheel_loop_config_t config;
    drv_encoder_port_t encoder_port;
} motion_bus_config_t;

/* One active variant, selected only after validating the full-width tag.
 * Native internal storage; never serialize its padding or pointer bytes.
 */
typedef union
{
    motion_bus_config_t configure;
    move_plan_command_t command;
    uint32_t ack_id;
} motion_bus_payload_t;

typedef struct
{
    motion_bus_kind_t kind;
    motion_bus_payload_t payload;
} motion_bus_request_t;
typedef struct
{
    motion_bus_request_t request;
    motion_run_status_t status;
    uint32_t cancel_id;
    bool b_pending;
    bool b_inflight;
    bool b_stop;
    bool b_cancel;
} motion_bus_t;
/* All access INCLUDING status is under the same short task-only mutex.
 * No driver/PID/kernel/network operations while this lock is held.
 * Emergency state is deliberately NOT here and never takes this mutex.
 */
bool motion_bus_put (motion_bus_t * p_bus,
                     motion_bus_request_t const * p_request);
void motion_bus_take (motion_bus_t * p_bus, motion_bus_request_t * p_request,
    bool * p_present, bool * p_stop, bool * p_cancel);
bool motion_bus_stop (motion_bus_t * p_bus, bool b_cancel, uint32_t id);
#endif /* MOTION_BUS_H */

/*** end of file ***/

