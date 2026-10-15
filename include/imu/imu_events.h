/**
 * @file    imu_events.h
 * @brief   Buddy 4 -> Mission Controller event messages (terrain_event_q)
 *          and the bounded, non-blocking event dispatcher.
 *
 * This header is intended to be included by the Mission Controller. It only
 * depends on <stdint.h>/<stdbool.h> and imu_types.h.
 *
 * Message format: fixed-size imu_event_t (copied by value into the queue).
 * All payload fields are integers in explicit units so that the Mission
 * Controller and Buddy 1 do not need floating point to interpret them:
 *     cdeg = 0.01 degree, cdps = 0.01 degree/s, mg = 0.001 g, mm, ms.
 *
 * Delivery guarantees (deterministic overflow policy):
 *   - Submission never blocks (TMO_POL on the uT-Kernel message buffer).
 *   - Non-critical events (tilt, turning, motion, hump progress): if the
 *     queue is full the NEW event is dropped and counted per type.
 *   - Critical events (IMPACT with CRITICAL severity, IMU_FAULT): if the
 *     queue is full they are kept in a small pending ring
 *     (IMU_EVENT_CRITICAL_PENDING) and retried every cycle, oldest first,
 *     before new events. They also trigger the direct notification hook
 *     (uT-Kernel event flag) immediately, independent of the queue.
 *   - Every event carries a 16-bit sequence number; a gap tells the
 *     receiver that events were dropped.
 */

#ifndef IMU_EVENTS_H
#define IMU_EVENTS_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/imu_config.h"
#include "imu/imu_types.h"

/** Event types carried in terrain_event_q (Design Review Section 4). */
typedef enum
{
    IMU_EVT_HUMP_DETECTED = 1,  /**< credible ascent started               */
    IMU_EVT_HUMP_PEAK,          /**< crest identified (provisional value)  */
    IMU_EVT_HUMP_COMPLETED,     /**< traversal complete, final measurement */
    IMU_EVT_HUMP_REJECTED,      /**< candidate discarded (reason in data)  */
    IMU_EVT_TILT,               /**< TILT_EVENT                            */
    IMU_EVT_TURNING,            /**< TURNING_EVENT / heading change        */
    IMU_EVT_IMPACT,             /**< IMPACT / COLLISION                    */
    IMU_EVT_IMU_FAULT,          /**< IMU_FAULT (raised / persistent / ok)  */
    IMU_EVT_MOTION,             /**< motion classification changed        */
    IMU_EVT_STATUS,             /**< subsystem ready / calibration status  */
    IMU_EVT_TYPE_COUNT
} imu_event_type_t;

/** Status codes for IMU_EVT_IMU_FAULT and IMU_EVT_STATUS. */
typedef enum
{
    IMU_STATUS_READY = 0,          /**< calibration complete, monitoring   */
    IMU_STATUS_CAL_RETRY,          /**< calibration failed, retrying       */
    IMU_STATUS_FAULT_RAISED,
    IMU_STATUS_FAULT_PERSISTENT,   /**< recovery attempts exhausted        */
    IMU_STATUS_RECOVERED,
    IMU_STATUS_RESUMED,            /**< collision fault cleared            */
    IMU_STATUS_MISSION_RESET,
    IMU_STATUS_MAG_CAL_OK,
    IMU_STATUS_MAG_CAL_FAILED
} imu_status_code_t;

/** Hump payload. */
typedef struct
{
    uint32_t index;              /**< valid hump number (0 = not counted) */
    int16_t  peak_pitch_cdeg;    /**< PRIMARY, directly measured          */
    int16_t  min_pitch_cdeg;
    int16_t  peak_vert_up_mg;
    int16_t  peak_vert_down_mg;
    int16_t  height_est_mm;      /**< -1 if not available                 */
    uint8_t  height_conf;        /**< imu_confidence_t                    */
    uint8_t  validity;           /**< imu_hump_validity_t                 */
    uint16_t duration_ms;
    uint8_t  b_new_max;          /**< 1: this hump is the mission maximum */
    uint8_t  reserved;
    int16_t  max_peak_pitch_cdeg;/**< mission maximum after this event    */
    int16_t  max_height_est_mm;
} imu_evt_hump_t;

/** Tilt payload. */
typedef struct
{
    int16_t pitch_cdeg;
    int16_t roll_cdeg;
    int16_t tilt_cdeg;
    uint8_t phase;               /**< imu_event_phase_t                   */
    uint8_t b_reliable;
} imu_evt_tilt_t;

/** Turning payload. */
typedef struct
{
    uint8_t  direction;          /**< imu_turn_dir_t                      */
    uint8_t  phase;              /**< START / END / HEADING_CHANGE        */
    uint8_t  heading_quality;    /**< imu_heading_quality_t               */
    uint8_t  b_unreliable;
    int16_t  rate_cdps;          /**< heading-derived, NOT a gyro rate    */
    int16_t  heading_cdeg;       /**< 0..35999                            */
    int32_t  delta_cdeg;         /**< heading change of the turn          */
    uint32_t duration_ms;
} imu_evt_turn_t;

/** Impact payload. */
typedef struct
{
    int16_t  peak_mg;
    int16_t  horiz_mg;
    int16_t  vert_mg;
    uint16_t duration_ms;
    uint32_t suppressed;         /**< pulses suppressed by cool-down      */
    uint8_t  b_during_hump;
    uint8_t  reserved[3];
} imu_evt_impact_t;

/** Fault / status payload. */
typedef struct
{
    uint8_t  status;             /**< imu_status_code_t                   */
    uint8_t  fault;              /**< imu_fault_t                         */
    uint8_t  last_error;         /**< -(imu_err_t)                        */
    uint8_t  reserved;
    uint32_t attempt;
    uint32_t i2c_failures;
} imu_evt_fault_t;

/** Motion payload. */
typedef struct
{
    uint8_t motion;              /**< imu_motion_class_t                  */
    uint8_t previous;
    uint8_t confidence;          /**< imu_confidence_t                    */
    uint8_t reserved;
    int16_t fwd_accel_mg;
    int16_t vibration_mg;
} imu_evt_motion_t;

/** Message placed in terrain_event_q. */
typedef struct
{
    uint8_t  type;               /**< imu_event_type_t                    */
    uint8_t  severity;           /**< imu_severity_t                      */
    uint8_t  state;              /**< imu_state_t when generated          */
    uint8_t  b_valid;            /**< 0 = measurement degraded / partial  */
    uint32_t timestamp_ms;       /**< ms since boot                       */
    uint16_t seq;                /**< per-producer sequence number        */
    uint16_t reserved;
    union
    {
        imu_evt_hump_t   hump;
        imu_evt_tilt_t   tilt;
        imu_evt_turn_t   turn;
        imu_evt_impact_t impact;
        imu_evt_fault_t  fault;
        imu_evt_motion_t motion;
    } data;
} imu_event_t;

/** Result of a sink (queue) send. */
typedef enum
{
    IMU_SINK_OK = 0,
    IMU_SINK_FULL,
    IMU_SINK_ERROR
} imu_sink_result_t;

/** Non-blocking send function (e.g. tk_snd_mbf(..., TMO_POL)). */
typedef imu_sink_result_t (*imu_event_send_fn_t)(void * p_ctx,
                                                 imu_event_t const * p_evt);

/** Direct notification for critical events (e.g. tk_set_flg). Must not
 *  block. May be NULL. */
typedef void (*imu_critical_notify_fn_t)(void * p_ctx,
                                         imu_event_t const * p_evt);

/** Sink description. */
typedef struct
{
    imu_event_send_fn_t      p_send;
    imu_critical_notify_fn_t p_notify;
    void *                   p_ctx;
} imu_event_sink_t;

/** Dispatcher statistics. */
typedef struct
{
    uint32_t submitted;
    uint32_t sent;
    uint32_t dropped[IMU_EVT_TYPE_COUNT];
    uint32_t rate_limited[IMU_EVT_TYPE_COUNT];
    uint32_t critical_deferred;
    uint32_t critical_lost;      /**< pending ring overflow (should be 0) */
    uint32_t sink_errors;
    uint32_t notifications;
} imu_event_stats_t;

/** Bounded dispatcher (no allocation). */
typedef struct
{
    imu_event_sink_t  sink;
    uint16_t          next_seq;
    imu_event_t       pending[IMU_EVENT_CRITICAL_PENDING];
    uint8_t           pending_head;
    uint8_t           pending_count;
    uint32_t          min_interval_ms[IMU_EVT_TYPE_COUNT];
    uint32_t          last_sent_ms[IMU_EVT_TYPE_COUNT];
    bool              b_sent_once[IMU_EVT_TYPE_COUNT];
    imu_event_stats_t stats;
} imu_event_dispatcher_t;

/** Initialise. @p p_sink may be NULL (events are then counted as dropped). */
void imu_events_init(imu_event_dispatcher_t * p_disp,
                     imu_event_sink_t const * p_sink);

/** Optional per-type rate limit (0 = none). Critical types ignore it. */
void imu_events_set_rate_limit(imu_event_dispatcher_t * p_disp,
                               imu_event_type_t type, uint32_t min_ms);

/** @return true if this event must never be silently dropped. */
bool imu_events_is_critical(imu_event_t const * p_evt);

/**
 * @brief  Stamp (seq) and submit an event. Never blocks.
 * @return IMU_OK (sent), IMU_ERR_QUEUE_FULL (critical: deferred, or
 *         non-critical: dropped), IMU_ERR_BUSY (rate-limited),
 *         IMU_ERR_PARAM.
 */
imu_err_t imu_events_submit(imu_event_dispatcher_t * p_disp,
                            imu_event_t * p_evt, uint32_t now_ms);

/** Retry deferred critical events; call once per cycle. */
void imu_events_service(imu_event_dispatcher_t * p_disp);

/** @return number of deferred critical events. */
uint8_t imu_events_pending(imu_event_dispatcher_t const * p_disp);

char const * imu_event_type_name(imu_event_type_t type);

#endif /* IMU_EVENTS_H */
