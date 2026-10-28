/** @file motion_bus.c @brief Bounded copy/admission, no queue allocation. */
#include "motion_bus.h"
#include <stddef.h>
bool motion_bus_put (motion_bus_t * p_bus,
                     motion_bus_request_t const * p_request)
{
    bool b_ok = (NULL != p_bus) && (NULL != p_request);

    if (b_ok)
    {
        b_ok = (!p_bus->b_pending) && (!p_bus->b_inflight) &&
               (!p_bus->b_stop);
        if (b_ok)
        {
            switch (p_request->kind)
            {
                case MOTION_BUS_CONFIG:
                    b_ok = (MOTION_RUN_UNCONFIGURED == p_bus->status.state);
                break;
                case MOTION_BUS_ENABLE:
                    b_ok = (MOTION_RUN_CONFIGURED == p_bus->status.state);
                break;
                case MOTION_BUS_ACK:
                    b_ok = (0U != p_request->payload.ack_id) &&
                        (p_request->payload.ack_id ==
                         p_bus->status.terminal_id);
                break;
                case MOTION_BUS_COMMAND:
                    b_ok = ((MOTION_RUN_READY == p_bus->status.state) &&
                            (0U == p_bus->status.terminal_id)) ||
                        ((MOTION_RUN_ACTIVE == p_bus->status.state) &&
                         (MOVE_PLAN_SPEED == p_bus->status.active_kind) &&
                         (MOVE_PLAN_SPEED == p_request->payload.command.kind) &&
                         (p_request->payload.command.id ==
                          p_bus->status.active_id));
                break;
                default:
                    b_ok = false;
                break;
            }
        }
        if (b_ok)
        {
            p_bus->request = *p_request;
            p_bus->b_pending = true;
        }
    }
    return (b_ok);
}

void motion_bus_take (motion_bus_t * p_bus, motion_bus_request_t * p_request,
    bool * p_present, bool * p_stop, bool * p_cancel)
{
    if ((NULL != p_bus) && (NULL != p_request) && (NULL != p_present) &&
        (NULL != p_stop) && (NULL != p_cancel))
    {
        *p_present = p_bus->b_pending;
        *p_stop = p_bus->b_stop;
        *p_cancel = p_bus->b_cancel;
        *p_request = p_bus->request;
        p_bus->b_inflight = p_bus->b_inflight || p_bus->b_pending;
        p_bus->b_pending = false;
        p_bus->b_stop = false;
        p_bus->b_cancel = false;
    }
}

bool motion_bus_stop (motion_bus_t * p_bus, bool b_cancel, uint32_t id)
{
    bool b_ok = (NULL != p_bus);

    if (b_ok && b_cancel)
    {
        b_ok = (0U != id) && ((id == p_bus->status.active_id) ||
            ((p_bus->b_pending || p_bus->b_inflight) &&
             (MOTION_BUS_COMMAND == p_bus->request.kind) &&
             (id == p_bus->request.payload.command.id)));
    }
    if (b_ok)
    {
        p_bus->b_stop = true;
        p_bus->b_cancel = b_cancel;
        p_bus->cancel_id = id;
    }
    return (b_ok);
}

/*** end of file ***/

