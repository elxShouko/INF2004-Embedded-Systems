#include <stddef.h>
/* This port otherwise typedefs size_t as SZ; use the compiler's size_t in
 * this C-library-using translation unit without changing shared config.
 */
#define PROHIBIT_DEF_SIZE_T 1
#include "member3_navigation.h"
#include "drv_ir_rtos.h"

#include <stddef.h>
#include <string.h>

ID mbf_s3_input;
ID mbf_s3_output;
ID mtx_s3_status;
ID tid_s3_navigation;
static s3_status_t g_status;

ER s3_submit(const s3_input_t *p_input, TMO timeout)
{
    if (p_input == NULL)
    {
        return E_PAR;
    }
    if (mbf_s3_input <= 0)
    {
        return E_OBJ;
    }
    return tk_snd_mbf(mbf_s3_input, p_input, sizeof(*p_input), timeout);
}

ER s3_receive(s3_output_t *p_output, TMO timeout)
{
    if (p_output == NULL)
    {
        return E_PAR;
    }
    if (mbf_s3_output <= 0)
    {
        return E_OBJ;
    }
    INT size = tk_rcv_mbf(mbf_s3_output, p_output, timeout);
    return (size < E_OK) ? size : ((size == sizeof(*p_output)) ? E_OK : E_PAR);
}

ER s3_get_status(s3_status_t *p_status)
{
    if (p_status == NULL)
    {
        return E_PAR;
    }
    if (mtx_s3_status <= 0)
    {
        return E_OBJ;
    }
    ER result = tk_loc_mtx(mtx_s3_status, TMO_FEVR);
    if (result < E_OK)
    {
        return result;
    }
    *p_status = g_status;
    return tk_unl_mtx(mtx_s3_status);
}

ER s3_navigation_init(void)
{
    T_CMBF buffer = {0};
    T_CMTX mutex = {0};
    if (mbf_s3_input > 0)
    {
        return E_OBJ;
    }
    memset(&g_status, 0, sizeof(g_status));
    g_status.state = LINE_STATE_CALIBRATING;
    g_status.mode = LINE_MODE_PAUSED;
    mutex.mtxatr = TA_INHERIT;
    mtx_s3_status = tk_cre_mtx(&mutex);
    if (mtx_s3_status < E_OK)
    {
        return mtx_s3_status;
    }
    buffer.mbfatr = TA_TFIFO;
    buffer.maxmsz = sizeof(s3_input_t);
    buffer.bufsz = 4U * (sizeof(s3_input_t) + sizeof(UW));
    mbf_s3_input = tk_cre_mbf(&buffer);
    if (mbf_s3_input < E_OK)
    {
        tk_del_mtx(mtx_s3_status);
        return mbf_s3_input;
    }
    buffer.maxmsz = sizeof(s3_output_t);
    buffer.bufsz = 16U * (sizeof(s3_output_t) + sizeof(UW));
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

ER s3_navigation_start(void)
{
    if ((mbf_s3_input <= 0) || (mbf_s3_output <= 0) || (mtx_s3_status <= 0) ||
        (tid_s3_navigation > 0))
    {
        return E_OBJ;
    }
    T_CTSK task = {0};
    task.task = (FP)s3_navigation_task;
    task.itskpri = S3_PRI_NAVIGATION;
    task.stksz = 8192U; /* Bounded queue payloads + processing contexts. */
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

/* Owner-task state only. No callback accesses this context on another core. */
typedef struct
{
    s3_config_t config;
    s3_status_t status;
    nav_map_t map;
    line_tracker_t line;
    barcode_capture_t barcode;
    junction_detector_t junction;
    uint64_t previous_sample_us;
    uint64_t absent_since_us;
    uint64_t found_since_us;
    uint64_t recovery_since_us;
    uint64_t last_barcode_us;
    uint64_t odometry_us;
    uint64_t distance_um;
    uint32_t event_id;
    bool absent;
    bool finding;
    bool recovering;
    bool previous_sample;
    bool have_odometry;
    bool odometry_forward;
    bool have_barcode;
} s3_context_t;

static void emit(s3_context_t *p_context, s3_output_t *p_output)
{
    p_output->event_id = ++p_context->event_id;
    p_output->map_revision = p_context->map.revision;
    if (tk_snd_mbf(mbf_s3_output, p_output, sizeof(*p_output), TMO_POL) != E_OK)
    {
        ++p_context->status.output_drops;
        p_context->status.fault = S3_FAULT_OUTPUT_FULL;
        p_context->status.state = LINE_STATE_FAULT_REPORTED;
        p_context->status.mode = LINE_MODE_PAUSED;
    }
}

static void navigation_event(s3_context_t *p_context, nav_event_kind_t kind,
                              nav_command_t command, char letter)
{
    s3_output_t output = {0};
    output.kind = S3_OUTPUT_NAVIGATION;
    output.navigation.timestamp_us = p_context->status.raw.timestamp_us;
    output.navigation.kind = kind;
    output.navigation.command = command;
    output.navigation.barcode_character = letter;
    output.navigation.black_mask = p_context->status.black_mask;
    output.navigation.state = p_context->status.state;
    output.fault = p_context->status.fault;
    emit(p_context, &output);
    if ((kind == NAV_EVENT_BARCODE_VALID) && (command != NAV_COMMAND_NONE) &&
        (p_context->status.fault == S3_FAULT_NONE))
    {
        p_context->status.pending_manoeuvre_event_id = output.event_id;
    }
}

static void fault(s3_context_t *p_context, s3_fault_t reason)
{
    if (p_context->status.fault == S3_FAULT_NONE)
    {
        p_context->status.fault = reason;
        p_context->status.state = LINE_STATE_FAULT_REPORTED;
        p_context->status.mode = LINE_MODE_PAUSED;
        navigation_event(p_context, NAV_EVENT_FAULT, NAV_COMMAND_NONE, '\0');
    }
}

static void reset_processing(s3_context_t *p_context)
{
    line_tracker_init(&p_context->line, p_context->config.line_stable_us);
    barcode_capture_init(&p_context->barcode, p_context->config.barcode_quiet_us,
                         p_context->config.barcode_timeout_us);
    if (p_context->config.junction_enabled)
    {
        junction_init(&p_context->junction, &p_context->config.junction);
    }
    p_context->absent = false;
    p_context->finding = false;
    p_context->recovering = false;
    p_context->previous_sample = false;
    p_context->have_barcode = false;
    p_context->status.line_valid = false;
}

static bool configure(s3_context_t *p_context, const s3_config_t *p_config)
{
    barcode_capture_t barcode;
    junction_detector_t junction;
    if ((p_context->status.mode != LINE_MODE_PAUSED) ||
        (p_config->calibration.valid_mask != 7U) ||
        ((p_config->calibration.black_high_mask & ~7U) != 0U) ||
        (p_config->period_ms < CNF_TIMER_PERIOD) || (p_config->period_ms > 100U) ||
        ((p_config->period_ms % CNF_TIMER_PERIOD) != 0U) ||
        (p_config->line_lost_us < p_config->line_stable_us) ||
        (p_config->line_lost_us == 0U) || (p_config->line_found_us == 0U) ||
        (p_config->recovery_timeout_us <= p_config->line_found_us) ||
        (p_config->max_sample_gap_us < (uint64_t)p_config->period_ms * 1000U) ||
        (p_config->odometry_stale_us == 0U) || (p_config->barcode_rearm_us == 0U) ||
        !barcode_capture_init(&barcode, p_config->barcode_quiet_us,
                              p_config->barcode_timeout_us) ||
        (p_config->junction_enabled && !junction_init(&junction, &p_config->junction)))
    {
        return false;
    }
    p_context->config = *p_config;
    p_context->status.calibrated = true;
    p_context->status.fault = S3_FAULT_NONE;
    p_context->status.state = LINE_STATE_TRACKING;
    p_context->status.pending_manoeuvre_event_id = 0U;
    p_context->have_odometry = false;
    reset_processing(p_context);
    return true;
}

static void input(s3_context_t *p_context, const s3_input_t *p_input, uint64_t now)
{
    bool accepted = false;
    switch (p_input->kind)
    {
        case S3_INPUT_CONFIG:
            accepted = configure(p_context, &p_input->value.config);
            break;
        case S3_INPUT_MAPPING:
            accepted = (memchr(p_input->value.mapping, '\0', S3_MAPPING_TEXT_MAX) != NULL) &&
                       nav_map_apply(&p_context->map, p_input->value.mapping);
            break;
        case S3_INPUT_MODE:
            if ((p_input->value.mode >= LINE_MODE_PAUSED) &&
                (p_input->value.mode <= LINE_MODE_RECOVERY) &&
                ((p_input->value.mode == LINE_MODE_PAUSED) ||
                 (p_context->status.calibrated && (p_context->status.fault == S3_FAULT_NONE) &&
                  (p_context->status.pending_manoeuvre_event_id == 0U))))
            {
                accepted = true;
                p_context->status.mode = p_input->value.mode;
                reset_processing(p_context);
                p_context->have_odometry = false;
                if (p_input->value.mode == LINE_MODE_RECOVERY)
                {
                    p_context->status.state = LINE_STATE_RECOVERY;
                    p_context->recovering = true;
                    p_context->recovery_since_us = now;
                }
                else if (p_context->status.fault == S3_FAULT_NONE && p_context->status.calibrated)
                {
                    p_context->status.state = (p_context->status.pending_manoeuvre_event_id != 0U)
                        ? LINE_STATE_AWAITING_MANOEUVRE : LINE_STATE_TRACKING;
                }
            }
            break;
        case S3_INPUT_ODOMETRY:
            accepted = (p_input->value.odometry.timestamp_us <= now) &&
                (!p_context->have_odometry ||
                 ((p_input->value.odometry.timestamp_us >= p_context->odometry_us) &&
                  (p_input->value.odometry.forward_distance_um >= p_context->distance_um)));
            if (accepted)
            {
                p_context->odometry_us = p_input->value.odometry.timestamp_us;
                p_context->distance_um = p_input->value.odometry.forward_distance_um;
                p_context->odometry_forward = p_input->value.odometry.tracking_forward;
                p_context->have_odometry = true;
            }
            break;
        case S3_INPUT_MANOEUVRE_DONE:
            accepted = (p_context->status.pending_manoeuvre_event_id != 0U) &&
                (p_input->value.manoeuvre_event_id == p_context->status.pending_manoeuvre_event_id) &&
                (p_context->status.fault == S3_FAULT_NONE);
            if (accepted)
            {
                p_context->status.pending_manoeuvre_event_id = 0U;
                p_context->status.mode = LINE_MODE_RECOVERY;
                p_context->status.state = LINE_STATE_RECOVERY;
                reset_processing(p_context);
                p_context->recovering = true;
                p_context->recovery_since_us = now;
                p_context->have_odometry = false;
            }
            break;
        default:
            break;
    }
    /* High-rate odometry has no per-packet reply; status/events report faults. */
    if (p_input->kind != S3_INPUT_ODOMETRY)
    {
        s3_output_t reply = {0};
        reply.kind = S3_OUTPUT_REPLY;
        reply.request_id = p_input->request_id;
        reply.accepted = accepted;
        emit(p_context, &reply);
    }
}

static void process(s3_context_t *p_context, const ir_sample_t *p_sample)
{
    p_context->status.raw = *p_sample;
    if (!p_context->status.calibrated || (p_context->status.fault != S3_FAULT_NONE))
    {
        return;
    }
    uint8_t mask;
    if (!ir_normalize(p_sample, &p_context->config.calibration, &mask))
    {
        fault(p_context, S3_FAULT_DRIVER);
        return;
    }
    p_context->status.black_mask = mask;
    const uint64_t now = p_sample->timestamp_us;
    const bool active = (p_context->status.mode == LINE_MODE_TRACKING) ||
                        (p_context->status.mode == LINE_MODE_RECOVERY);
    if (active && p_context->previous_sample &&
        ((now < p_context->previous_sample_us) ||
         ((now - p_context->previous_sample_us) > p_context->config.max_sample_gap_us)))
    {
        fault(p_context, S3_FAULT_SAMPLE_GAP);
        return;
    }
    if (active)
    {
        p_context->previous_sample_us = now;
        p_context->previous_sample = true;
    }
    line_observation_t observation = p_context->status.line;
    if (line_tracker_update(&p_context->line, p_sample->timestamp_us, mask, &observation))
    {
        const bool changed = !p_context->status.line_valid ||
                             (observation != p_context->status.line);
        p_context->status.line = observation;
        p_context->status.line_valid = true;
        if (changed && (p_context->status.mode == LINE_MODE_TRACKING) &&
            (p_context->status.state != LINE_STATE_AWAITING_MANOEUVRE))
        {
            s3_output_t output = {0};
            output.kind = S3_OUTPUT_LINE;
            output.line = observation;
            output.navigation.timestamp_us = p_sample->timestamp_us;
            output.navigation.kind = NAV_EVENT_LINE_STATUS;
            output.navigation.black_mask = mask;
            emit(p_context, &output);
        }
    }
    if (!active || p_context->status.fault != S3_FAULT_NONE)
    {
        return; /* Continue sensing while paused/bypassing; no steering/events. */
    }
    if (p_context->status.state == LINE_STATE_AWAITING_MANOEUVRE)
    {
        return;
    }
    if (p_context->status.state == LINE_STATE_RECOVERY)
    {
        if (!p_context->recovering)
        {
            p_context->recovery_since_us = now;
            p_context->recovering = true;
        }
        if ((now - p_context->recovery_since_us) >= p_context->config.recovery_timeout_us)
        {
            fault(p_context, S3_FAULT_RECOVERY_TIMEOUT);
            return;
        }
        if (p_context->status.line_valid && observation == LINE_OBSERVATION_CENTRED)
        {
            if (!p_context->finding)
            {
                p_context->finding = true;
                p_context->found_since_us = now;
            }
            else if ((now - p_context->found_since_us) >= p_context->config.line_found_us)
            {
                navigation_event(p_context, NAV_EVENT_LINE_FOUND, NAV_COMMAND_NONE, '\0');
                if (p_context->status.fault == S3_FAULT_NONE)
                {
                    p_context->status.mode = LINE_MODE_PAUSED; /* Controller explicitly resumes. */
                    p_context->status.state = LINE_STATE_TRACKING;
                }
                return;
            }
        }
        else
        {
            p_context->finding = false;
        }
        return;
    }
    if (p_context->status.line_valid && observation == LINE_OBSERVATION_ABSENT)
    {
        if (!p_context->absent)
        {
            p_context->absent_since_us = now;
            p_context->absent = true;
        }
        else if ((now - p_context->absent_since_us) >= p_context->config.line_lost_us)
        {
            p_context->status.state = LINE_STATE_RECOVERY;
            p_context->status.mode = LINE_MODE_RECOVERY;
            p_context->recovering = true;
            p_context->recovery_since_us = now;
            navigation_event(p_context, NAV_EVENT_LINE_LOST, NAV_COMMAND_NONE, '\0');
            return;
        }
    }
    else
    {
        p_context->absent = false;
    }

    char letter = '\0';
    barcode_result_t result = barcode_capture_update(&p_context->barcode, now,
                                                      (mask & 4U) != 0U, &letter);
    p_context->status.state = p_context->barcode.capturing
        ? LINE_STATE_BARCODE_DECODING : LINE_STATE_TRACKING;
    if (result == BARCODE_RESULT_VALID &&
        (!p_context->have_barcode ||
         (now - p_context->last_barcode_us >= p_context->config.barcode_rearm_us)))
    {
        p_context->have_barcode = true;
        p_context->last_barcode_us = now;
        nav_command_t command = nav_map_lookup(&p_context->map, letter);
        if (command != NAV_COMMAND_NONE)
        {
            p_context->status.state = LINE_STATE_AWAITING_MANOEUVRE;
        }
        navigation_event(p_context, NAV_EVENT_BARCODE_VALID, command, letter);
    }
    else if (result == BARCODE_RESULT_INVALID)
    {
        navigation_event(p_context, NAV_EVENT_BARCODE_INVALID, NAV_COMMAND_NONE, '\0');
    }
    if (p_context->config.junction_enabled &&
        (p_context->status.state != LINE_STATE_AWAITING_MANOEUVRE))
    {
        junction_sample_t sample = {now, p_context->distance_um, mask,
            p_context->have_odometry && p_context->odometry_forward &&
            (now >= p_context->odometry_us) &&
            (now - p_context->odometry_us <= p_context->config.odometry_stale_us)};
        s3_output_t output = {0};
        if (junction_update(&p_context->junction, &sample, &output.navigation) ==
            JUNCTION_RESULT_DETECTED)
        {
            output.kind = S3_OUTPUT_NAVIGATION;
            emit(p_context, &output);
        }
    }
}

void s3_navigation_task(INT stacd, void *exinf)
{
    (void)stacd;
    (void)exinf;
    s3_context_t context = {0};
    context.status.state = LINE_STATE_CALIBRATING;
    context.status.mode = LINE_MODE_PAUSED;
    context.config.period_ms = CNF_TIMER_PERIOD; /* Respect shared kernel tick. */
    nav_map_init(&context.map);
    if (s3_ir_init() < E_OK)
    {
        fault(&context, S3_FAULT_DRIVER);
    }
    while (1)
    {
        /* Bounded work per cycle prevents configuration flood starving sensors. */
        for (unsigned int count = 0U; count < 4U; ++count)
        {
            s3_input_t message;
            INT size = tk_rcv_mbf(mbf_s3_input, &message, TMO_POL);
            if (size == E_TMOUT)
            {
                break;
            }
            if (size != sizeof(message))
            {
                fault(&context, S3_FAULT_KERNEL);
                break;
            }
            input(&context, &message, s3_clock_us());
        }
        ir_sample_t sample;
        s3_ir_read(&sample);
        process(&context, &sample);
        context.status.map_revision = context.map.revision;
        memcpy(context.status.mapping, context.map.actions, sizeof(context.status.mapping));
        ER lock = tk_loc_mtx(mtx_s3_status, TMO_FEVR);
        if (lock == E_OK)
        {
            g_status = context.status;
            tk_unl_mtx(mtx_s3_status);
        }
        else
        {
            fault(&context, S3_FAULT_KERNEL);
        }
        tk_dly_tsk(context.config.period_ms);
    }
}
