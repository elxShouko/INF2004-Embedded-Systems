/** @file member3_navigation.c
 * @brief Member 3 member3 navigation module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#include <stddef.h>
/* This port otherwise typedefs size_t as SZ; use the compiler's size_t in
 * this C-library-using translation unit without changing shared config.
 */
#define PROHIBIT_DEF_SIZE_T 1
#include "member3_navigation.h"
#include "drv_ir_rtos.h"
#include "drv_s3_codec.h"
#include <string.h>

#define S3_INPUTS_PER_CYCLE 4U
#define S3_INPUT_CAPACITY 4U
#define S3_OUTPUT_CAPACITY 16U
#define S3_TASK_STACK_BYTES 8192U

/* Owner-task state only. No callback accesses this context on another core. */
typedef struct
{
    s3_config_t         config;
    s3_status_t         status;
    nav_map_t           map;
    line_tracker_t      line;
    barcode_capture_t   barcode;
    junction_detector_t junction;
    uint64_t            previous_sample_us;
    uint64_t            absent_since_us;
    uint64_t            found_since_us;
    uint64_t            recovery_since_us;
    uint64_t            last_barcode_us;
    uint64_t            odometry_us;
    uint64_t            distance_um;
    uint32_t            event_id;
    char                pending_letter;
    nav_command_t       pending_command;
    uint8_t             b_absent : 1;
    uint8_t             b_finding : 1;
    uint8_t             b_recovering : 1;
    uint8_t             b_previous_sample : 1;
    uint8_t             b_have_odometry : 1;
    uint8_t             b_odometry_forward : 1;
    uint8_t             b_have_barcode : 1;
    uint8_t             b_odometry_usable : 1;
} s3_context_t;

volatile ID                 mbf_s3_input;
volatile ID                 mbf_s3_output;
volatile ID                 mtx_s3_status;
volatile ID                 tid_s3_navigation;
static volatile s3_status_t g_status;

static void publish_status (s3_context_t * p_context);
static void latch_state (s3_context_t * p_context, s3_fault_t reason);
static bool accept_odometry (s3_context_t * p_context,
                              const s3_odometry_t * p_odometry, uint64_t now);
static void emit (s3_context_t * p_context, s3_output_t * p_output);
static void navigation_event (s3_context_t * p_context, nav_event_kind_t kind,
                              nav_command_t command, char letter);
static void fault (s3_context_t * p_context, s3_fault_t reason);
static void reset_processing (s3_context_t * p_context);
static bool configure (s3_context_t * p_context, const s3_config_t * p_config);
static bool change_mode (s3_context_t * p_context, const s3_input_t * p_input,
                         uint64_t now);
static void input (s3_context_t * p_context, const s3_input_t * p_input,
                   uint64_t now);
static line_observation_t update_line (s3_context_t *      p_context,
                                       const ir_sample_t * p_sample,
                                       uint8_t             mask);
static void               confirm_line_found (s3_context_t * p_context);
static void recover_line (s3_context_t *     p_context,
                          line_observation_t observation, uint64_t now);
static bool check_line_loss (s3_context_t *     p_context,
                             line_observation_t observation, uint64_t now);
static void process_barcode (s3_context_t * p_context, uint64_t now,
                             uint8_t mask);
static void process_junction (s3_context_t * p_context, uint64_t now,
                              uint8_t mask);
static void process (s3_context_t * p_context, const ir_sample_t * p_sample);

/**
 * @brief Copy one request into the bounded owner-task input queue.
 */
ER s3_submit (const s3_input_t * p_input, TMO timeout)
{
    if (NULL == p_input)
    {
        return E_PAR;
    }

    if (mbf_s3_input <= 0)
    {
        return E_OBJ;
    }

    uint8_t bytes[S3_INPUT_WIRE_MAX];
    const uint32_t size = s3_input_encode(p_input, bytes);

    return (0U == size) ? E_PAR :
        tk_snd_mbf(mbf_s3_input, bytes, (INT)size, timeout);
}

/**
 * @brief Decode one compact output without exposing partially received data.
 * @note Encoded lengths are bounded by the private queue maximum.
 */
ER s3_receive (s3_output_t * p_output, TMO timeout)
{
    if (NULL == p_output)
    {
        return E_PAR;
    }

    if (mbf_s3_output <= 0)
    {
        return E_OBJ;
    }

    uint8_t bytes[S3_OUTPUT_WIRE_MAX];
    const INT size = tk_rcv_mbf(mbf_s3_output, bytes, timeout);

    if (size < E_OK)
    {
        return size;
    }
    return s3_output_decode(bytes, (uint32_t)size, p_output) ? E_OK : E_PAR;
}

/**
 * @brief Poll for a coherent status copy without waiting for the mutex.
 */
ER s3_get_status (s3_status_t * p_status)
{
    if (NULL == p_status)
    {
        return E_PAR;
    }

    if (mtx_s3_status <= 0)
    {
        return E_OBJ;
    }

    ER result = tk_loc_mtx(mtx_s3_status, TMO_POL);

    if (result < E_OK)
    {
        return result;
    }

    const s3_status_t snapshot = g_status;
    result = tk_unl_mtx(mtx_s3_status);

    if (E_OK == result)
    {
        *p_status = snapshot;
    }

    return result;
}

/**
 * @brief Create navigation objects before any subsystem task starts.
 * @note The zero compound literal initializes the entire status object; it is
 * not a numerical narrowing conversion.
 */
ER s3_navigation_init (void)
{
    T_CMBF buffer = {0};
    T_CMTX mutex  = {0};

    if (mbf_s3_input > 0)
    {
        return E_OBJ;
    }

    g_status       = (s3_status_t){0};
    g_status.state = LINE_STATE_CALIBRATING;
    g_status.mode  = LINE_MODE_PAUSED;
    mutex.mtxatr   = TA_INHERIT;
    mtx_s3_status  = tk_cre_mtx(&mutex);

    if (mtx_s3_status < E_OK)
    {
        return mtx_s3_status;
    }

    buffer.mbfatr = TA_TFIFO;
    buffer.maxmsz = S3_INPUT_WIRE_MAX;
    buffer.bufsz  = S3_INPUT_CAPACITY *
        (((S3_INPUT_WIRE_MAX + 3U) & ~3U) + sizeof(UW));
    mbf_s3_input  = tk_cre_mbf(&buffer);

    if (mbf_s3_input < E_OK)
    {
        tk_del_mtx(mtx_s3_status);

        return mbf_s3_input;
    }

    buffer.maxmsz = S3_OUTPUT_WIRE_MAX;
    buffer.bufsz  = S3_OUTPUT_CAPACITY *
        (((S3_OUTPUT_WIRE_MAX + 3U) & ~3U) + sizeof(UW));
    mbf_s3_output = tk_cre_mbf(&buffer);

    if (mbf_s3_output < E_OK)
    {
        tk_del_mbf(mbf_s3_input);
        tk_del_mtx(mtx_s3_status);
        mbf_s3_input = 0;

        return mbf_s3_output;
    }

    return E_OK;
}

/**
 * @brief Create and start the single navigation owner task.
 * @note FP is the template callback type. The registered task uses the required
 * INT and void-pointer argument ABI.
 */
ER s3_navigation_start (void)
{
    if ((mbf_s3_input <= 0) || (mbf_s3_output <= 0) || (mtx_s3_status <= 0) ||
        (tid_s3_navigation > 0))
    {
        return E_OBJ;
    }

    T_CTSK task  = {0};
    task.task    = (FP)s3_navigation_task;
    task.itskpri = S3_PRI_NAVIGATION;
    task.stksz =
        S3_TASK_STACK_BYTES; /* Bounded queue payloads + processing contexts. */
    task.tskatr = TA_HLNG | TA_RNG3;
#if TK_SUPPORT_SMP
    task.tskatr |= TA_ASSPRC;
    task.assprc = TP_PRC1;
#endif
    tid_s3_navigation = tk_cre_tsk(&task);

    if (tid_s3_navigation < E_OK)
    {
        return tid_s3_navigation;
    }

    ER result = tk_sta_tsk(tid_s3_navigation, 0);

    if (result < E_OK)
    {
        tk_del_tsk(tid_s3_navigation);
        tid_s3_navigation = 0;
    }

    return result;
}

/**
 * @brief Serialize requests and sensor processing on the owner task.
 * @note The positive kernel tick fits uint32_t; fixed message sizes fit
 * INT32_MAX. Void casts intentionally discard unused task arguments.
 */
void s3_navigation_task (INT stacd, void * p_exinf)
{
    (void)stacd;
    (void)p_exinf;
    s3_context_t context = {0};
    context.status.state = LINE_STATE_CALIBRATING;
    context.status.mode  = LINE_MODE_PAUSED;
    context.config.period_ms =
        CNF_TIMER_PERIOD; /* Respect shared kernel tick. */
    nav_map_init(&context.map);

    if (s3_ir_init() < E_OK)
    {
        fault(&context, S3_FAULT_DRIVER);
    }

    for (;;)
    {
        /* Bounded work per cycle prevents configuration flood starving sensors.
         */

        for (uint32_t count = 0U; count < S3_INPUTS_PER_CYCLE; ++count)
        {
            uint8_t bytes[S3_INPUT_WIRE_MAX];
            s3_input_t message;
            INT size = tk_rcv_mbf(mbf_s3_input, bytes, TMO_POL);

            if (E_TMOUT == size)
            {
                break;
            }

            if ((size < E_OK) ||
                !s3_input_decode(bytes, (uint32_t)size, &message))
            {
                fault(&context, S3_FAULT_KERNEL);
                break;
            }

            input(&context, &message, s3_clock_us());
        }

        ir_sample_t sample;
        s3_ir_read(&sample);
        process(&context, &sample);
        publish_status(&context);
        if (E_OK != tk_dly_tsk(context.config.period_ms))
        {
            latch_state(&context, S3_FAULT_KERNEL);
            navigation_event(&context, NAV_EVENT_FAULT, NAV_COMMAND_NONE, '\0');
            publish_status(&context);
            tk_ext_tsk();
            return; /* Defensive if an invalid platform/test stub returns. */
        }
    }
}

/** @brief Publish once without waiting; busy copies keep their old time. */
static void publish_status (s3_context_t * p_context)
{
    p_context->status.map_revision = p_context->map.revision;
    memcpy(p_context->status.mapping, p_context->map.actions,
           sizeof(p_context->status.mapping));
    ER lock = tk_loc_mtx(mtx_s3_status, TMO_POL);

    if (E_OK == lock)
    {
        g_status = p_context->status;
        if (tk_unl_mtx(mtx_s3_status) != E_OK)
        {
            fault(p_context, S3_FAULT_KERNEL);
        }
    }
    else if (lock != E_TMOUT)
    {
        fault(p_context, S3_FAULT_KERNEL);
    }
    else
    {
        /* Skip this publication; old acquisition time exposes staleness.
         * Sensing must continue even if another task holds the mutex.
         */
    }

}

/** @brief Latch condition bits without sending recursively or changing time. */
static void latch_state (s3_context_t * p_context, s3_fault_t reason)
{
    p_context->status.fault = (uint8_t)(
        ((uint32_t)p_context->status.fault | (uint32_t)reason) & 31U);
    p_context->status.state = LINE_STATE_FAULT_REPORTED;
    p_context->status.mode = LINE_MODE_PAUSED;
}

/**
 * @brief Publish an output or latch the existing queue-overload policy.
 */
static void emit (s3_context_t * p_context, s3_output_t * p_output)
{
    if (UINT32_MAX == p_context->event_id)
    {
        latch_state(p_context, S3_FAULT_KERNEL);
        if (p_context->status.output_drops < UINT32_MAX)
        {
            ++p_context->status.output_drops;
        }
        return;
    }
    if ((UINT32_MAX - 1U) == p_context->event_id)
    {
        /* Reserve the final unique ID for a terminal fault, never a command. */
        latch_state(p_context, S3_FAULT_KERNEL);
        *p_output = (s3_output_t){0};
        p_output->kind = S3_OUTPUT_NAVIGATION;
        p_output->navigation.kind = NAV_EVENT_FAULT;
        p_output->navigation.state = LINE_STATE_FAULT_REPORTED;
        p_output->navigation.timestamp_us = p_context->status.raw.timestamp_us;
        p_output->fault = p_context->status.fault;
    }
    p_output->event_id     = ++p_context->event_id;
    p_output->map_revision = p_context->map.revision;

    uint8_t bytes[S3_OUTPUT_WIRE_MAX];
    const uint32_t size = s3_output_encode(p_output, bytes);

    if ((0U == size) ||
        (tk_snd_mbf(mbf_s3_output, bytes, (INT)size, TMO_POL) != E_OK))
    {
        if (p_context->status.output_drops < UINT32_MAX)
        {
            ++p_context->status.output_drops;
        }
        latch_state(p_context, S3_FAULT_OUTPUT_FULL);
    }
}

/**
 * @brief Snapshot navigation data and correlate b_accepted manoeuvres.
 */
static void navigation_event (s3_context_t * p_context, nav_event_kind_t kind,
                              nav_command_t command, char letter)
{
    s3_output_t output                  = {0};
    output.kind                         = S3_OUTPUT_NAVIGATION;
    output.navigation.timestamp_us      = p_context->status.raw.timestamp_us;
    output.navigation.kind              = kind;
    output.navigation.command           = command;
    output.navigation.barcode_character = letter;
    output.navigation.black_mask        = p_context->status.black_mask;
    output.navigation.state             = p_context->status.state;
    output.fault                        = p_context->status.fault;
    emit(p_context, &output);

    if ((NAV_EVENT_JUNCTION == kind) && (command != NAV_COMMAND_NONE) &&
        (S3_FAULT_NONE == p_context->status.fault))
    {
        p_context->status.pending_manoeuvre_event_id = output.event_id;
    }
}

/**
 * @brief Latch the first navigation fault and request controller handling.
 */
static void fault (s3_context_t * p_context, s3_fault_t reason)
{
    if (S3_FAULT_NONE == p_context->status.fault)
    {
        latch_state(p_context, reason);
        navigation_event(p_context, NAV_EVENT_FAULT, NAV_COMMAND_NONE, '\0');
    }
}

/**
 * @brief Clear transient sensing evidence after a mode/config change.
 */
static void reset_processing (s3_context_t * p_context)
{
    line_tracker_init(&p_context->line, p_context->config.line_stable_us);
    barcode_capture_init(&p_context->barcode,
                         p_context->config.barcode_quiet_us,
                         p_context->config.barcode_timeout_us);

    if (p_context->config.junction_enabled)
    {
        junction_init(&p_context->junction, &p_context->config.junction);
    }

    p_context->b_absent          = false;
    p_context->b_finding         = false;
    p_context->b_recovering      = false;
    p_context->b_previous_sample = false;
    p_context->b_have_barcode    = false;
    p_context->b_odometry_usable = false;
    p_context->status.line_valid = false;
}

/**
 * @brief Accept measured settings only while the subsystem is paused.
 * @note CNF_TIMER_PERIOD is the positive kernel millisecond constant; uint32_t
 * holds it and uint64_t holds its microsecond product.
 */
static bool configure (s3_context_t * p_context, const s3_config_t * p_config)
{
    barcode_capture_t   barcode;
    junction_detector_t junction;

    if ((UINT32_MAX == p_context->event_id) ||
        (p_context->status.mode != LINE_MODE_PAUSED) ||
        (p_config->calibration.valid_mask != 7U) ||
        ((p_config->calibration.black_high_mask & ~7U) != 0U) ||
        (p_config->period_ms < (uint32_t)CNF_TIMER_PERIOD) ||
        (p_config->period_ms > 100U) ||
        ((p_config->period_ms % (uint32_t)CNF_TIMER_PERIOD) != 0U) ||
        (p_config->line_lost_us < p_config->line_stable_us) ||
        (0U == p_config->line_lost_us) || (0U == p_config->line_found_us) ||
        (p_config->recovery_timeout_us <= p_config->line_found_us) ||
        (p_config->max_sample_gap_us < (uint64_t)p_config->period_ms * 1000U) ||
        (0U == p_config->odometry_stale_us) ||
        (0U == p_config->barcode_rearm_us) ||
        (!barcode_capture_init(&barcode, p_config->barcode_quiet_us,
                               p_config->barcode_timeout_us)) ||
        ((p_config->junction_enabled) &&
         (!junction_init(&junction, &p_config->junction))))
    {
        return false;
    }

    p_context->config                            = *p_config;
    p_context->status.calibrated                 = true;
    p_context->status.fault                      = S3_FAULT_NONE;
    p_context->status.state                      = LINE_STATE_TRACKING;
    p_context->status.pending_manoeuvre_event_id = 0U;
    p_context->b_have_odometry                   = false;
    p_context->b_odometry_usable                 = false;
    p_context->pending_letter                    = '\0';
    p_context->pending_command                   = NAV_COMMAND_NONE;
    reset_processing(p_context);

    return true;
}

/**
 * @brief Validate a mode request and preserve pending manoeuvre ownership.
 */
static bool change_mode (s3_context_t * p_context, const s3_input_t * p_input,
                         uint64_t now)
{
    bool b_accepted = false;
    /* All valid modes fit uint32_t; a negative input becomes out of range. */

    if (((uint32_t)p_input->value.mode <= (uint32_t)LINE_MODE_RECOVERY) &&
        ((LINE_MODE_PAUSED == p_input->value.mode) ||
         ((p_context->status.calibrated) &&
          (S3_FAULT_NONE == p_context->status.fault) &&
          (0U == p_context->status.pending_manoeuvre_event_id))))
    {
        b_accepted             = true;
        p_context->status.mode = p_input->value.mode;
        reset_processing(p_context);
        p_context->b_have_odometry = false;

        if (LINE_MODE_RECOVERY == p_input->value.mode)
        {
            p_context->status.state      = LINE_STATE_RECOVERY;
            p_context->b_recovering      = true;
            p_context->recovery_since_us = now;
        }
        else if ((S3_FAULT_NONE == p_context->status.fault) &&
                 (p_context->status.calibrated))
        {
            p_context->status.state =
                (p_context->status.pending_manoeuvre_event_id != 0U)
                    ? LINE_STATE_AWAITING_MANOEUVRE
                    : LINE_STATE_TRACKING;
        }
        else
        {
            /* Preserve the latched fault state. */
        }
    }

    return b_accepted;
}

/** @brief Reject stale/regressing evidence and conflicting duplicates. */
static bool accept_odometry (s3_context_t * p_context,
                              const s3_odometry_t * p_odometry, uint64_t now)
{
    bool b_accepted = (p_odometry->timestamp_us <= now) &&
        ((now - p_odometry->timestamp_us) <=
         p_context->config.odometry_stale_us);

    if (b_accepted && p_context->b_have_odometry)
    {
        b_accepted = (p_odometry->timestamp_us >= p_context->odometry_us) &&
            (p_odometry->forward_distance_um >= p_context->distance_um) &&
            ((p_odometry->timestamp_us != p_context->odometry_us) ||
             ((p_odometry->forward_distance_um == p_context->distance_um) &&
              (p_odometry->tracking_forward ==
               p_context->b_odometry_forward)));
    }
    p_context->b_odometry_usable = b_accepted;
    if (b_accepted)
    {
        p_context->odometry_us = p_odometry->timestamp_us;
        p_context->distance_um = p_odometry->forward_distance_um;
        p_context->b_odometry_forward = p_odometry->tracking_forward;
        p_context->b_have_odometry = true;
    }
    return b_accepted;
}

/**
 * @brief Dispatch a request and reply with its validation result.
 */
static void input (s3_context_t * p_context, const s3_input_t * p_input,
                   uint64_t now)
{
    bool b_accepted = false;

    switch (p_input->kind)
    {
        case S3_INPUT_CONFIG:
            b_accepted = configure(p_context, &p_input->value.config);
        break;
        case S3_INPUT_MAPPING:
            b_accepted = ('\0' == p_context->pending_letter) &&
                (0U == p_context->status.pending_manoeuvre_event_id) &&
                nav_map_install(&p_context->map, &p_input->value.mapping);
        break;
        case S3_INPUT_MODE:
            b_accepted = change_mode(p_context, p_input, now);
        break;
        case S3_INPUT_ODOMETRY:
            b_accepted = accept_odometry(p_context,
                                         &p_input->value.odometry, now);

        break;
        case S3_INPUT_MANOEUVRE_DONE:
            b_accepted = (LINE_MODE_TRACKING == p_context->status.mode) &&
                         (LINE_STATE_AWAITING_MANOEUVRE ==
                          p_context->status.state) &&
                         (p_context->status.pending_manoeuvre_event_id != 0U) &&
                         (p_input->value.manoeuvre_event_id ==
                          p_context->status.pending_manoeuvre_event_id) &&
                         (S3_FAULT_NONE == p_context->status.fault);

            if (b_accepted)
            {
                p_context->status.pending_manoeuvre_event_id = 0U;
                p_context->status.mode  = LINE_MODE_RECOVERY;
                p_context->status.state = LINE_STATE_RECOVERY;
                reset_processing(p_context);
                p_context->b_recovering      = true;
                p_context->recovery_since_us = now;
                p_context->b_have_odometry   = false;
            }

        break;
        default:
        break;
    }

    /* High-rate odometry has no per-packet reply; status/events report faults.
     */

    if (p_input->kind != S3_INPUT_ODOMETRY)
    {
        s3_output_t reply = {0};
        reply.kind        = S3_OUTPUT_REPLY;
        reply.request_id  = p_input->request_id;
        reply.accepted    = b_accepted;
        emit(p_context, &reply);
    }
}

/**
 * @brief Update the filtered line snapshot and publish b_changed observations.
 */
static line_observation_t update_line (s3_context_t *      p_context,
                                       const ir_sample_t * p_sample,
                                       uint8_t             mask)
{
    line_observation_t observation = p_context->status.line;

    if (line_tracker_update(&p_context->line, p_sample->timestamp_us, mask,
                            &observation))
    {
        const bool b_changed         = (!p_context->status.line_valid) ||
                                       (observation != p_context->status.line);
        p_context->status.line       = observation;
        p_context->status.line_valid = true;

        if (b_changed && (LINE_MODE_TRACKING == p_context->status.mode) &&
            (p_context->status.state != LINE_STATE_AWAITING_MANOEUVRE))
        {
            s3_output_t output             = {0};
            output.kind                    = S3_OUTPUT_LINE;
            output.line                    = observation;
            output.navigation.timestamp_us = p_sample->timestamp_us;
            output.navigation.kind         = NAV_EVENT_LINE_STATUS;
            output.navigation.black_mask   = (uint8_t)(mask & 7U);
            emit(p_context, &output);
        }
    }

    return observation;
}

/**
 * @brief Confirm sustained line reacquisition or report recovery timeout.
 */
static void recover_line (s3_context_t *     p_context,
                          line_observation_t observation, uint64_t now)
{
    if (!p_context->b_recovering)
    {
        p_context->recovery_since_us = now;
        p_context->b_recovering      = true;
    }

    if ((now - p_context->recovery_since_us) >=
        p_context->config.recovery_timeout_us)
    {
        fault(p_context, S3_FAULT_RECOVERY_TIMEOUT);

        return;
    }

    /* The filter retains its last stable observation during a transition.
     * Raw loss of either front sensing point must break confirmation now,
     * rather than allowing old centred evidence to complete recovery.
     */
    if ((p_context->status.line_valid) &&
        (3U == (p_context->status.black_mask & 3U)) &&
        (LINE_OBSERVATION_CENTRED == observation))
    {
        if (!p_context->b_finding)
        {
            p_context->b_finding      = true;
            p_context->found_since_us = now;
        }
        else if ((now - p_context->found_since_us) >=
                 p_context->config.line_found_us)
        {
            confirm_line_found(p_context);

            return;
        }
        else
        {
            /* Continue checking the reacquisition interval. */
        }
    }
    else
    {
        p_context->b_finding = false;
    }

    return;
}

/**
 * @brief Confirm sustained absence before entering line recovery.
 */
static bool check_line_loss (s3_context_t *     p_context,
                             line_observation_t observation, uint64_t now)
{
    if ((p_context->status.line_valid) &&
        (LINE_OBSERVATION_ABSENT == observation))
    {
        if (!p_context->b_absent)
        {
            p_context->absent_since_us = now;
            p_context->b_absent        = true;
        }
        else if ((now - p_context->absent_since_us) >=
                 p_context->config.line_lost_us)
        {
            p_context->status.state      = LINE_STATE_RECOVERY;
            p_context->status.mode       = LINE_MODE_RECOVERY;
            p_context->b_recovering      = true;
            p_context->recovery_since_us = now;
            navigation_event(p_context, NAV_EVENT_LINE_LOST, NAV_COMMAND_NONE,
                             '\0');

            return true;
        }
        else
        {
            /* Keep the first absence time until the interval expires. */
        }
    }
    else
    {
        p_context->b_absent = false;
    }

    return false;
}

/**
 * @brief Decode the side stream and snapshot its configured action.
 */
static void process_barcode (s3_context_t * p_context, uint64_t now,
                             uint8_t mask)
{
    char             letter = '\0';
    barcode_result_t result = barcode_capture_update(
        &p_context->barcode, now, (mask & 4U) != 0U, &letter);
    p_context->status.state = p_context->barcode.capturing
                                  ? LINE_STATE_BARCODE_DECODING
                                  : LINE_STATE_TRACKING;

    if ((BARCODE_RESULT_VALID == result) &&
        ((!p_context->b_have_barcode) || (now - p_context->last_barcode_us >=
                                          p_context->config.barcode_rearm_us)))
    {
        p_context->b_have_barcode  = true;
        p_context->last_barcode_us = now;
        nav_command_t command      = nav_map_lookup(&p_context->map, letter);

        if ((command != NAV_COMMAND_NONE) &&
            ('\0' == p_context->pending_letter))
        {
            p_context->pending_letter = letter;
            p_context->pending_command = command;
        }

        navigation_event(p_context, NAV_EVENT_BARCODE_VALID, command, letter);
    }
    else if (BARCODE_RESULT_INVALID == result)
    {
        navigation_event(p_context, NAV_EVENT_BARCODE_INVALID, NAV_COMMAND_NONE,
                         '\0');
    }
    else
    {
        /* A pending or recently repeated barcode issues no event. */
    }
}

/**
 * @brief Evaluate crossing evidence only with fresh forward odometry.
 */
static void process_junction (s3_context_t * p_context, uint64_t now,
                              uint8_t mask)
{
    if ((p_context->config.junction_enabled) &&
        (p_context->status.state != LINE_STATE_AWAITING_MANOEUVRE))
    {
        junction_sample_t sample = {now, p_context->distance_um, mask,
                                    (p_context->b_have_odometry) &&
                                        (p_context->b_odometry_usable) &&
                                        (p_context->b_odometry_forward) &&
                                        (now >= p_context->odometry_us) &&
                                        (now - p_context->odometry_us <=
                                         p_context->config.odometry_stale_us)};
        s3_output_t       output = {0};

        if (junction_update(&p_context->junction, &sample,
                            &output.navigation) == JUNCTION_RESULT_DETECTED)
        {
            const char letter = p_context->pending_letter;
            const nav_command_t command = p_context->pending_command;
            if ('\0' != letter)
            {
                p_context->status.state = LINE_STATE_AWAITING_MANOEUVRE;
            }
            navigation_event(p_context, NAV_EVENT_JUNCTION, command, letter);
            p_context->pending_letter = '\0';
            p_context->pending_command = NAV_COMMAND_NONE;
        }
    }
}

/**
 * @brief Run the existing sampling gates and state-machine stages in order.
 */
static void process (s3_context_t * p_context, const ir_sample_t * p_sample)
{
    p_context->status.raw = *p_sample;

    if ((!p_context->status.calibrated) ||
        (p_context->status.fault != S3_FAULT_NONE))
    {
        return;
    }

    uint8_t mask = 0U;

    if (!ir_normalize(p_sample, &p_context->config.calibration, &mask))
    {
        fault(p_context, S3_FAULT_DRIVER);

        return;
    }

    p_context->status.black_mask = (uint8_t)(mask & 7U);
    const uint64_t now           = p_sample->timestamp_us;
    const bool     b_active = (LINE_MODE_TRACKING == p_context->status.mode) ||
                              (LINE_MODE_RECOVERY == p_context->status.mode);

    if (b_active && (p_context->b_previous_sample) &&
        ((now < p_context->previous_sample_us) ||
         ((now - p_context->previous_sample_us) >
          p_context->config.max_sample_gap_us)))
    {
        fault(p_context, S3_FAULT_SAMPLE_GAP);

        return;
    }

    if (b_active)
    {
        p_context->previous_sample_us = now;
        p_context->b_previous_sample  = true;
    }

    const line_observation_t observation =
        update_line(p_context, p_sample, mask);

    if ((!b_active) || (p_context->status.fault != S3_FAULT_NONE))
    {
        return; /* Continue sensing while paused/bypassing; no steering/events.
                 */
    }

    if (LINE_STATE_AWAITING_MANOEUVRE == p_context->status.state)
    {
        return;
    }

    if (LINE_STATE_RECOVERY == p_context->status.state)
    {
        recover_line(p_context, observation, now);

        return;
    }

    if (check_line_loss(p_context, observation, now))
    {
        return;
    }

    process_barcode(p_context, now, mask);
    process_junction(p_context, now, mask);
}

/**
 * @brief Publish reacquisition before allowing controller-directed resumption.
 */
static void confirm_line_found (s3_context_t * p_context)
{
    navigation_event(p_context, NAV_EVENT_LINE_FOUND, NAV_COMMAND_NONE, '\0');

    if (S3_FAULT_NONE == p_context->status.fault)
    {
        p_context->status.mode =
            LINE_MODE_PAUSED; /* Controller explicitly resumes. */
        p_context->status.state = LINE_STATE_TRACKING;
    }
}

/*** End of file. ***/

