/**
 * @file    imu_events.c
 * @brief   Bounded, non-blocking event dispatcher (see imu_events.h).
 */

#include <stddef.h>
#include <string.h>

#include "imu/imu_events.h"

void imu_events_init(imu_event_dispatcher_t * p_disp,
                     imu_event_sink_t const * p_sink)
{
    if (NULL == p_disp)
    {
        return;
    }

    (void)memset(p_disp, 0, sizeof(*p_disp));
    if (NULL != p_sink)
    {
        p_disp->sink = *p_sink;
    }
}

void imu_events_set_rate_limit(imu_event_dispatcher_t * p_disp,
                               imu_event_type_t type, uint32_t min_ms)
{
    if ((NULL != p_disp) && ((uint32_t)type < (uint32_t)IMU_EVT_TYPE_COUNT))
    {
        p_disp->min_interval_ms[type] = min_ms;
    }
}

bool imu_events_is_critical(imu_event_t const * p_evt)
{
    bool b_crit = false;

    if (NULL != p_evt)
    {
        b_crit = (IMU_EVT_IMU_FAULT == p_evt->type)
              || ((IMU_EVT_IMPACT == p_evt->type)
                  && (IMU_SEV_CRITICAL == p_evt->severity))
              || ((IMU_EVT_TILT == p_evt->type)
                  && (IMU_SEV_CRITICAL == p_evt->severity));
    }

    return (b_crit);
}

static imu_sink_result_t events_send(imu_event_dispatcher_t * p_disp,
                                     imu_event_t const * p_evt)
{
    imu_sink_result_t res = IMU_SINK_ERROR;

    if (NULL != p_disp->sink.p_send)
    {
        res = p_disp->sink.p_send(p_disp->sink.p_ctx, p_evt);
    }
    if (IMU_SINK_OK == res)
    {
        p_disp->stats.sent++;
    }
    else if (IMU_SINK_ERROR == res)
    {
        p_disp->stats.sink_errors++;
    }
    else
    {
        /* full: handled by the caller */
    }

    return (res);
}

static void events_defer(imu_event_dispatcher_t * p_disp,
                         imu_event_t const * p_evt)
{
    uint8_t slot;

    if (p_disp->pending_count >= IMU_EVENT_CRITICAL_PENDING)
    {
        /* Ring full: overwrite the oldest (the newest critical state is the
         * most relevant one for the Mission Controller). */
        p_disp->pending_head = (uint8_t)((p_disp->pending_head + 1U)
                                         % IMU_EVENT_CRITICAL_PENDING);
        p_disp->pending_count--;
        p_disp->stats.critical_lost++;
    }

    slot = (uint8_t)((p_disp->pending_head + p_disp->pending_count)
                     % IMU_EVENT_CRITICAL_PENDING);
    p_disp->pending[slot] = *p_evt;
    p_disp->pending_count++;
    p_disp->stats.critical_deferred++;
}

void imu_events_service(imu_event_dispatcher_t * p_disp)
{
    if (NULL == p_disp)
    {
        return;
    }

    while (p_disp->pending_count > 0U)
    {
        imu_sink_result_t res =
            events_send(p_disp, &p_disp->pending[p_disp->pending_head]);

        if (IMU_SINK_OK != res)
        {
            break;   /* still full: retry next cycle, keep order */
        }
        p_disp->pending_head = (uint8_t)((p_disp->pending_head + 1U)
                                         % IMU_EVENT_CRITICAL_PENDING);
        p_disp->pending_count--;
    }
}

uint8_t imu_events_pending(imu_event_dispatcher_t const * p_disp)
{
    return ((NULL != p_disp) ? p_disp->pending_count : 0U);
}

imu_err_t imu_events_submit(imu_event_dispatcher_t * p_disp,
                            imu_event_t * p_evt, uint32_t now_ms)
{
    imu_sink_result_t res;
    bool              b_crit;
    uint8_t           type;

    if ((NULL == p_disp) || (NULL == p_evt)
        || (0U == p_evt->type)
        || ((uint32_t)p_evt->type >= (uint32_t)IMU_EVT_TYPE_COUNT))
    {
        return (IMU_ERR_PARAM);
    }

    type   = p_evt->type;
    b_crit = imu_events_is_critical(p_evt);
    p_disp->stats.submitted++;

    if (!b_crit && (p_disp->min_interval_ms[type] > 0U)
        && p_disp->b_sent_once[type]
        && ((now_ms - p_disp->last_sent_ms[type])
            < p_disp->min_interval_ms[type]))
    {
        p_disp->stats.rate_limited[type]++;
        return (IMU_ERR_BUSY);
    }

    p_evt->seq = p_disp->next_seq;
    p_disp->next_seq++;

    if (b_crit && (NULL != p_disp->sink.p_notify))
    {
        p_disp->sink.p_notify(p_disp->sink.p_ctx, p_evt);
        p_disp->stats.notifications++;
    }

    /* Critical events never overtake older deferred critical events. */
    if (b_crit && (p_disp->pending_count > 0U))
    {
        events_defer(p_disp, p_evt);
        imu_events_service(p_disp);
        return (IMU_ERR_QUEUE_FULL);
    }

    res = events_send(p_disp, p_evt);
    if (IMU_SINK_OK == res)
    {
        p_disp->last_sent_ms[type] = now_ms;
        p_disp->b_sent_once[type]  = true;
        return (IMU_OK);
    }

    if (b_crit)
    {
        events_defer(p_disp, p_evt);
    }
    else
    {
        p_disp->stats.dropped[type]++;
    }

    return (IMU_ERR_QUEUE_FULL);
}

char const * imu_event_type_name(imu_event_type_t type)
{
    static char const * const k_names[IMU_EVT_TYPE_COUNT] =
    {
        "NONE", "HUMP_DETECTED", "HUMP_PEAK", "HUMP_COMPLETED",
        "HUMP_REJECTED", "TILT_EVENT", "TURNING_EVENT", "IMPACT/COLLISION",
        "IMU_FAULT", "MOTION", "STATUS"
    };

    return (((uint32_t)type < (uint32_t)IMU_EVT_TYPE_COUNT)
            ? k_names[type] : "INVALID");
}
