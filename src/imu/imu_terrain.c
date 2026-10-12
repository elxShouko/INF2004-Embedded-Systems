/**
 * @file    imu_terrain.c
 * @brief   Buddy 4 subsystem state machine (see imu_terrain.h).
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "imu/imu_terrain.h"

#define TERRAIN_PLAUS_MAX_FACTOR   (1.8f)   /* > sqrt(3) * full scale     */
#define TERRAIN_CDEG               (100.0f)
#define TERRAIN_MG                 (1000.0f)
#define TERRAIN_I16_MAX            (32767.0f)
#define TERRAIN_NONE_I16           ((int16_t)-1)   /* 'not available' */

/* ------------------------------------------------------------------------ */
/*  Small helpers                                                           */
/* ------------------------------------------------------------------------ */

static bool time_reached(uint32_t now_ms, uint32_t deadline_ms)
{
    return ((int32_t)(now_ms - deadline_ms) >= 0);
}

static int16_t to_i16(float value, float scale)
{
    float v = value * scale;

    if (0 == isfinite(v))
    {
        return (0);
    }
    v = imu_clampf(v, -TERRAIN_I16_MAX, TERRAIN_I16_MAX);

    return ((int16_t)lroundf(v));
}

/** to_i16() if @p b_avail, otherwise the 'not available' marker (-1). */
static int16_t to_i16_or_none(bool b_avail, float value, float scale)
{
    int16_t r = TERRAIN_NONE_I16;

    if (b_avail)
    {
        r = to_i16(value, scale);
    }

    return (r);
}

static void terrain_trace(imu_terrain_t * p_sys, uint32_t now_ms,
                          imu_state_t from, imu_state_t to)
{
    uint8_t slot = (uint8_t)((p_sys->trace_head + p_sys->trace_count)
                             % IMU_STATE_TRACE_LEN);

    if (p_sys->trace_count >= IMU_STATE_TRACE_LEN)
    {
        slot = p_sys->trace_head;
        p_sys->trace_head = (uint8_t)((p_sys->trace_head + 1U)
                                      % IMU_STATE_TRACE_LEN);
    }
    else
    {
        p_sys->trace_count++;
    }

    p_sys->trace[slot].t_ms = now_ms;
    p_sys->trace[slot].from = (uint8_t)from;
    p_sys->trace[slot].to   = (uint8_t)to;
}

static void terrain_set_state(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_state_t next)
{
    if (next != p_sys->state)
    {
        terrain_trace(p_sys, now_ms, p_sys->state, next);
        p_sys->state = next;
    }
}

/** Transient (run-to-completion) state: logged, then back to @p back. */
static void terrain_transient(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_state_t transient)
{
    imu_state_t back = p_sys->state;

    terrain_trace(p_sys, now_ms, back, transient);
    terrain_trace(p_sys, now_ms, transient, back);
}

static void terrain_evt_init(imu_terrain_t const * p_sys, imu_event_t * p_e,
                             imu_event_type_t type, imu_severity_t sev,
                             uint32_t now_ms)
{
    (void)memset(p_e, 0, sizeof(*p_e));
    p_e->type         = (uint8_t)type;
    p_e->severity     = (uint8_t)sev;
    p_e->state        = (uint8_t)p_sys->state;
    p_e->b_valid      = 1U;
    p_e->timestamp_ms = now_ms;
}

static void terrain_emit_status(imu_terrain_t * p_sys, uint32_t now_ms,
                                imu_event_type_t type, imu_severity_t sev,
                                imu_status_code_t code, imu_err_t err)
{
    imu_event_t e;

    terrain_evt_init(p_sys, &e, type, sev, now_ms);
    e.data.fault.status       = (uint8_t)code;
    e.data.fault.fault        = (uint8_t)p_sys->fault;
    e.data.fault.last_error   = (uint8_t)(-(int32_t)err);
    e.data.fault.attempt      = p_sys->attempts;
    e.data.fault.i2c_failures = p_sys->dev.stats.failed_transfers;
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static lsm303_config_t terrain_dev_cfg(imu_params_t const * p_params)
{
    lsm303_config_t cfg;

    cfg.accel_odr_code  = p_params->accel_odr_code;
    cfg.accel_fs_code   = p_params->accel_fs_code;
    cfg.b_accel_fifo    = p_params->b_accel_fifo;
    cfg.mag_odr_code    = p_params->mag_odr_code;
    cfg.mag_gain_code   = p_params->mag_gain_code;
    cfg.b_mag_id_check  = p_params->b_mag_id_check;
    cfg.i2c_baud_hz     = p_params->i2c_baud_hz;
    cfg.i2c_timeout_us  = p_params->i2c_timeout_us;
    cfg.i2c_max_retries = p_params->i2c_max_retries;

    return (cfg);
}

/* ------------------------------------------------------------------------ */
/*  Acquisition                                                             */
/* ------------------------------------------------------------------------ */

typedef struct
{
    imu_err_t  err;            /* bus error, NO_DATA (stale) or OK         */
    uint8_t    count;          /* accel samples this cycle                 */
    bool       b_valid;        /* accel average plausible                  */
    bool       b_frozen;
    imu_vec3_t accel_avg_g;    /* sensor frame                             */
    bool       b_mag_ok;
    imu_vec3_t mag_gauss;      /* sensor frame, uncalibrated               */
} terrain_sample_t;

static void terrain_acquire(imu_terrain_t * p_sys, uint32_t now_ms,
                            terrain_sample_t * p_s)
{
    imu_params_t const * p_params = &p_sys->params;
    bool                 b_overrun = false;
    bool                 b_mag_ovf = false;
    imu_raw3_t           mag_raw;
    imu_vec3_t           sum = { 0.0f, 0.0f, 0.0f };
    float                norm;
    uint8_t              i;

    (void)memset(p_s, 0, sizeof(*p_s));

    p_s->err = lsm303_read_accel(&p_sys->dev, p_sys->accel_buf,
                                 (uint8_t)IMU_ACCEL_BATCH_MAX, &p_s->count,
                                 &b_overrun);
    if (IMU_OK != p_s->err)
    {
        return;
    }

    if (p_s->count > 0U)
    {
        imu_raw3_t const * p_last = &p_sys->accel_buf[p_s->count - 1U];

        p_sys->last_accel_ok_ms = now_ms;

        for (i = 0U; i < p_s->count; i++)
        {
            imu_vec3_t g;

            lsm303_accel_to_g(&p_sys->dev, &p_sys->accel_buf[i], &g);
            sum.x += g.x;
            sum.y += g.y;
            sum.z += g.z;
        }
        imu_vec_scale(&sum, 1.0f / (float)p_s->count, &p_s->accel_avg_g);

        norm = imu_vec_norm(&p_s->accel_avg_g);
        p_s->b_valid = (0 != isfinite(norm))
                    && (norm >= p_params->accel_plaus_min_g)
                    && (norm <= (TERRAIN_PLAUS_MAX_FACTOR
                                 * p_sys->dev.accel_full_scale_g));

        /* Frozen output: identical 12-bit triples cycle after cycle. */
        if (p_sys->b_last_raw_valid && (p_last->x == p_sys->last_raw.x)
            && (p_last->y == p_sys->last_raw.y)
            && (p_last->z == p_sys->last_raw.z))
        {
            p_sys->frozen_count++;
        }
        else
        {
            p_sys->frozen_count = 0U;
        }
        p_sys->last_raw         = *p_last;
        p_sys->b_last_raw_valid = true;
        p_s->b_frozen = (p_sys->frozen_count >= p_params->frozen_cycles);
    }
    else if ((now_ms - p_sys->last_accel_ok_ms) > p_params->stale_data_ms)
    {
        p_s->err = IMU_ERR_NO_DATA;
        return;
    }
    else
    {
        /* no new sample this cycle (timing jitter): not an error */
    }

    p_s->err = lsm303_read_mag(&p_sys->dev, &mag_raw, &b_mag_ovf);
    if (IMU_OK == p_s->err)
    {
        lsm303_mag_to_gauss(&p_sys->dev, &mag_raw, &p_s->mag_gauss);
        p_s->b_mag_ok = !b_mag_ovf;
    }
}

/* ------------------------------------------------------------------------ */
/*  Fault entry                                                             */
/* ------------------------------------------------------------------------ */

static void terrain_enter_sensor_fault(imu_terrain_t * p_sys,
                                       uint32_t now_ms, imu_fault_t cause,
                                       imu_err_t err)
{
    terrain_set_state(p_sys, now_ms, IMU_STATE_SENSOR_FAULT);
    p_sys->fault      = cause;
    p_sys->last_error = err;
    imu_hump_abort(&p_sys->hump, now_ms);

    p_sys->attempts              = 0U;
    p_sys->retry_interval_ms     = p_sys->params.recovery_interval_ms;
    p_sys->next_action_ms        = now_ms + p_sys->retry_interval_ms;
    p_sys->b_persistent_reported = false;

    terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL,
                        IMU_STATUS_FAULT_RAISED, err);
    terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_SAFE);
}

/**
 * @return true if the sample may be processed; false if it was a failure
 *         (and possibly a fault transition happened).
 */
static bool terrain_check_sample(imu_terrain_t * p_sys, uint32_t now_ms,
                                 terrain_sample_t const * p_s)
{
    imu_params_t const * p_params = &p_sys->params;

    if (IMU_ERR_NO_DATA == p_s->err)
    {
        p_sys->last_error = p_s->err;
        terrain_enter_sensor_fault(p_sys, now_ms, IMU_FAULT_STALE_DATA,
                                   p_s->err);
        return (false);
    }

    if (IMU_OK != p_s->err)
    {
        p_sys->consec_fail++;
        p_sys->last_error = p_s->err;
        if (p_sys->consec_fail >= p_params->fault_consec_cycles)
        {
            terrain_enter_sensor_fault(p_sys, now_ms, IMU_FAULT_I2C,
                                       p_s->err);
        }
        return (false);
    }
    p_sys->consec_fail = 0U;

    if (0U == p_s->count)
    {
        return (false);   /* nothing new, not a failure */
    }

    if (p_s->b_frozen)
    {
        terrain_enter_sensor_fault(p_sys, now_ms, IMU_FAULT_FROZEN_DATA,
                                   IMU_ERR_INVALID_DATA);
        return (false);
    }

    if (!p_s->b_valid)
    {
        p_sys->consec_invalid++;
        p_sys->last_error = IMU_ERR_INVALID_DATA;
        if (p_sys->consec_invalid >= p_params->fault_consec_cycles)
        {
            terrain_enter_sensor_fault(p_sys, now_ms, IMU_FAULT_INVALID_DATA,
                                       IMU_ERR_INVALID_DATA);
        }
        return (false);
    }
    p_sys->consec_invalid = 0U;

    return (true);
}

/* ------------------------------------------------------------------------ */
/*  Event builders                                                          */
/* ------------------------------------------------------------------------ */

static void terrain_emit_hump(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_event_type_t type,
                              imu_hump_meas_t const * p_m, bool b_new_max)
{
    imu_event_t              e;
    imu_hump_stats_t const * p_st = &p_sys->hump.stats;

    terrain_evt_init(p_sys, &e, type, IMU_SEV_INFO, now_ms);
    e.b_valid = (uint8_t)(((IMU_HUMP_VALID == p_m->validity)
                           || (IMU_HUMP_PROVISIONAL == p_m->validity))
                          ? 1U : 0U);
    e.data.hump.index             = p_m->index;
    e.data.hump.peak_pitch_cdeg   = to_i16(p_m->peak_pitch_deg, TERRAIN_CDEG);
    e.data.hump.min_pitch_cdeg    = to_i16(p_m->min_pitch_deg, TERRAIN_CDEG);
    e.data.hump.peak_vert_up_mg   = to_i16(p_m->peak_vert_up_g, TERRAIN_MG);
    e.data.hump.peak_vert_down_mg = to_i16(p_m->peak_vert_down_g, TERRAIN_MG);
    e.data.hump.height_est_mm     =
        to_i16_or_none((IMU_CONF_NONE != p_m->height_conf),
                       p_m->height_est_mm, 1.0f);
    e.data.hump.height_conf       = (uint8_t)p_m->height_conf;
    e.data.hump.validity          = (uint8_t)p_m->validity;
    e.data.hump.duration_ms       = (uint16_t)((p_m->total_ms > 65535U)
                                               ? 65535U : p_m->total_ms);
    e.data.hump.b_new_max         = b_new_max ? 1U : 0U;
    e.data.hump.max_peak_pitch_cdeg =
        to_i16_or_none(p_st->b_has_max, p_st->max.peak_pitch_deg,
                       TERRAIN_CDEG);
    e.data.hump.max_height_est_mm =
        to_i16_or_none(p_st->b_has_max
                       && (IMU_CONF_NONE != p_st->max.height_conf),
                       p_st->max.height_est_mm, 1.0f);

    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static void terrain_emit_tilt(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_proc_out_t const * p_out)
{
    imu_event_t e;

    terrain_transient(p_sys, now_ms, IMU_STATE_TILT_EVENT);
    terrain_evt_init(p_sys, &e, IMU_EVT_TILT, p_out->tilt_severity, now_ms);
    e.data.tilt.pitch_cdeg = to_i16(p_out->pitch_deg, TERRAIN_CDEG);
    e.data.tilt.roll_cdeg  = to_i16(p_out->roll_deg, TERRAIN_CDEG);
    e.data.tilt.tilt_cdeg  = to_i16(p_out->tilt_deg, TERRAIN_CDEG);
    e.data.tilt.phase      = (uint8_t)p_out->tilt_phase;
    e.data.tilt.b_reliable = p_out->b_attitude_reliable ? 1U : 0U;
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static void terrain_emit_turn(imu_terrain_t * p_sys, uint32_t now_ms,
                              imu_proc_out_t const * p_out)
{
    imu_event_t e;
    float       delta = p_out->turn_event_delta_deg * TERRAIN_CDEG;

    terrain_transient(p_sys, now_ms, IMU_STATE_TURN_EVENT);
    terrain_evt_init(p_sys, &e, IMU_EVT_TURNING, IMU_SEV_INFO, now_ms);
    e.b_valid = (uint8_t)(p_out->b_turn_event_unreliable ? 0U : 1U);
    e.data.turn.direction       = (uint8_t)p_out->turn_event_dir;
    e.data.turn.phase           = (uint8_t)p_out->turn_phase;
    e.data.turn.heading_quality = (uint8_t)p_out->heading_quality;
    e.data.turn.b_unreliable    = p_out->b_turn_event_unreliable ? 1U : 0U;
    e.data.turn.rate_cdps       = to_i16(p_out->turn_rate_dps, TERRAIN_CDEG);
    e.data.turn.heading_cdeg    = to_i16(p_out->heading_deg, TERRAIN_CDEG);
    e.data.turn.delta_cdeg      = (0 != isfinite(delta))
                                  ? (int32_t)lroundf(imu_clampf(delta,
                                        -2.0e9f, 2.0e9f)) : 0;
    e.data.turn.duration_ms     = p_out->turn_event_duration_ms;
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static void terrain_emit_motion(imu_terrain_t * p_sys, uint32_t now_ms,
                                imu_proc_out_t const * p_out)
{
    imu_event_t e;

    terrain_evt_init(p_sys, &e, IMU_EVT_MOTION, IMU_SEV_INFO, now_ms);
    e.data.motion.motion       = (uint8_t)p_out->motion;
    e.data.motion.previous     = (uint8_t)p_out->motion_prev;
    e.data.motion.confidence   = (uint8_t)p_out->motion_conf;
    e.data.motion.fwd_accel_mg = to_i16(p_out->fwd_accel_g, TERRAIN_MG);
    e.data.motion.vibration_mg = to_i16(p_out->vibration_g, TERRAIN_MG);
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

static void terrain_emit_impact(imu_terrain_t * p_sys, uint32_t now_ms,
                                imu_impact_t const * p_imp, bool b_hump)
{
    imu_event_t e;
    uint32_t    dur = p_imp->duration_ms;

    terrain_evt_init(p_sys, &e, IMU_EVT_IMPACT, p_imp->severity, now_ms);
    e.data.impact.peak_mg       = to_i16(p_imp->peak_g, TERRAIN_MG);
    e.data.impact.horiz_mg      = to_i16(p_imp->horiz_g, TERRAIN_MG);
    e.data.impact.vert_mg       = to_i16(p_imp->vert_g, TERRAIN_MG);
    e.data.impact.duration_ms   = (uint16_t)((dur > 65535U) ? 65535U : dur);
    e.data.impact.suppressed    = p_imp->suppressed;
    e.data.impact.b_during_hump = b_hump ? 1U : 0U;
    (void)imu_events_submit(&p_sys->disp, &e, now_ms);
}

/* ------------------------------------------------------------------------ */
/*  Processing of one valid sample                                          */
/* ------------------------------------------------------------------------ */

/** @return true if a fault-level impact occurred. */
static bool terrain_process(imu_terrain_t * p_sys, uint32_t now_ms,
                            terrain_sample_t const * p_s, bool b_detect)
{
    imu_params_t const * p_params = &p_sys->params;
    imu_proc_out_t *     p_out    = &p_sys->last_out;
    imu_impact_t         worst;
    imu_hump_input_t     hin;
    imu_hump_out_t       hout;
    bool                 b_hump   = (IMU_HUMP_MONITORING
                                     != p_sys->hump.phase);
    bool                 b_impact = false;
    bool                 b_fault  = false;
    uint32_t             odr_hz   = imu_params_accel_odr_hz(
                                        p_params->accel_odr_code);
    uint32_t             odr_ms   = (odr_hz > 0U) ? (1000U / odr_hz) : 0U;
    uint8_t              i;

    (void)memset(&worst, 0, sizeof(worst));

    /* High-rate impact path over every FIFO sample (oldest first). */
    for (i = 0U; i < p_s->count; i++)
    {
        imu_vec3_t   g;
        imu_impact_t imp;
        uint32_t     t_i = now_ms - ((uint32_t)(p_s->count - 1U - i)
                                     * odr_ms);

        lsm303_accel_to_g(&p_sys->dev, &p_sys->accel_buf[i], &g);
        imu_proc_hf_accel(&p_sys->proc, &g, t_i, b_hump, &imp);
        if (imp.b_detected
            && (!worst.b_detected || (imp.severity > worst.severity)
                || ((imp.severity == worst.severity)
                    && (imp.peak_g > worst.peak_g))))
        {
            worst = imp;
        }
    }

    if (p_sys->b_mag_cal_active && p_s->b_mag_ok)
    {
        imu_cal_mag_add(&p_sys->mag_acc, &p_s->mag_gauss);
    }

    imu_proc_step(&p_sys->proc, &p_s->accel_avg_g,
                  p_s->b_mag_ok ? &p_s->mag_gauss : NULL, now_ms, b_hump,
                  p_out);

    if (worst.b_detected && b_detect)
    {
        b_impact = true;
        p_sys->status.last_impact  = worst;
        p_sys->status.b_has_impact = true;

        /* IMPACT EVENT (classify severity) */
        b_fault = (IMU_SEV_CRITICAL == worst.severity)
               || p_params->b_impact_minor_enters_fault;

        if (b_fault)
        {
            /* FAULT HANDLING: report collision, then FAULT / SAFE STATE */
            terrain_set_state(p_sys, now_ms, IMU_STATE_IMPACT_EVENT);
            terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_HANDLING);
            terrain_emit_impact(p_sys, now_ms, &worst, b_hump);
            imu_hump_abort(&p_sys->hump, now_ms);
            p_sys->fault           = IMU_FAULT_COLLISION;
            p_sys->b_fault_ack     = false;
            p_sys->b_settle_timing = false;
            terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_SAFE);
        }
        else
        {
            /* Minor impact: report and keep monitoring (transient). */
            terrain_transient(p_sys, now_ms, IMU_STATE_IMPACT_EVENT);
            terrain_emit_impact(p_sys, now_ms, &worst, b_hump);
        }
    }

    if (b_detect && !b_fault)
    {
        if (p_out->b_tilt_event)
        {
            terrain_emit_tilt(p_sys, now_ms, p_out);
        }
        if (p_out->b_turn_event)
        {
            terrain_emit_turn(p_sys, now_ms, p_out);
        }

        hin.pitch_deg           = p_out->pitch_deg;
        hin.vert_accel_g        = p_out->vert_accel_g;
        hin.b_attitude_reliable = p_out->b_attitude_reliable;
        hin.b_speed_valid       = imu_proc_hint_fresh(&p_sys->proc, now_ms);
        hin.speed_mm_s          = (float)p_sys->proc.hint.speed_mm_s;
        hin.b_hint_accel_valid  = hin.b_speed_valid
                                  && p_sys->proc.b_hint_accel_valid;
        hin.hint_accel_g        = p_sys->proc.hint_accel_g;
        imu_hump_step(&p_sys->hump, now_ms, &hin, &hout);

        if (hout.b_detected)
        {
            terrain_emit_hump(p_sys, now_ms, IMU_EVT_HUMP_DETECTED,
                              &hout.meas, false);
        }
        if (hout.b_peak)
        {
            terrain_emit_hump(p_sys, now_ms, IMU_EVT_HUMP_PEAK,
                              &hout.meas, false);
        }
        if (hout.b_completed)
        {
            terrain_emit_hump(p_sys, now_ms, IMU_EVT_HUMP_COMPLETED,
                              &hout.meas, hout.b_new_max);
        }
        if (hout.b_rejected)
        {
            terrain_emit_hump(p_sys, now_ms, IMU_EVT_HUMP_REJECTED,
                              &hout.meas, false);
        }

        switch (hout.phase)
        {
            case IMU_HUMP_ASCENT:
                terrain_set_state(p_sys, now_ms, IMU_STATE_HUMP_ASCENT);
                break;
            case IMU_HUMP_PEAK:
                terrain_set_state(p_sys, now_ms, IMU_STATE_HUMP_PEAK);
                break;
            case IMU_HUMP_DESCENT:
                terrain_set_state(p_sys, now_ms, IMU_STATE_HUMP_DESCENT);
                break;
            default:
                terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
                break;
        }
    }

    imu_proc_classify(&p_sys->proc, now_ms, p_sys->hump.phase, b_impact,
                      p_out);
    if (p_out->b_motion_changed && b_detect)
    {
        terrain_emit_motion(p_sys, now_ms, p_out);
    }

    return (b_fault);
}

/* ------------------------------------------------------------------------ */
/*  State handlers                                                          */
/* ------------------------------------------------------------------------ */

static void terrain_begin_calibration(imu_terrain_t * p_sys, uint32_t now_ms)
{
    imu_cal_accel_begin(&p_sys->cal_acc);
    p_sys->consec_fail      = 0U;
    p_sys->consec_invalid   = 0U;
    p_sys->last_accel_ok_ms = now_ms;
    terrain_set_state(p_sys, now_ms, IMU_STATE_CALIBRATION);
}

static void terrain_do_init(imu_terrain_t * p_sys, uint32_t now_ms)
{
    lsm303_config_t cfg;
    imu_err_t       err;

    if (!time_reached(now_ms, p_sys->next_action_ms))
    {
        return;
    }

    cfg = terrain_dev_cfg(&p_sys->params);
    err = lsm303_init(&p_sys->dev, &cfg);

    if (IMU_OK == err)
    {
        p_sys->attempts         = 0U;
        p_sys->fault            = IMU_FAULT_NONE;
        p_sys->b_last_raw_valid = false;
        p_sys->frozen_count     = 0U;
        p_sys->cal_attempts     = 0U;
        p_sys->b_cal_fault_reported = false;
        terrain_begin_calibration(p_sys, now_ms);
        return;
    }

    p_sys->attempts++;
    p_sys->last_error = err;
    p_sys->fault      = IMU_FAULT_INIT_FAILED;

    if (p_sys->attempts == p_sys->params.init_max_attempts)
    {
        terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT,
                            IMU_SEV_CRITICAL, IMU_STATUS_FAULT_RAISED, err);
    }

    /* Retry with exponential back-off (bounded). */
    if (p_sys->attempts >= p_sys->params.init_max_attempts)
    {
        p_sys->retry_interval_ms = (p_sys->retry_interval_ms
                                    < (p_sys->params.recovery_interval_max_ms
                                       / 2U))
                                   ? (p_sys->retry_interval_ms * 2U)
                                   : p_sys->params.recovery_interval_max_ms;
    }
    else
    {
        p_sys->retry_interval_ms = p_sys->params.init_retry_ms;
    }
    p_sys->next_action_ms = now_ms + p_sys->retry_interval_ms;
}

static void terrain_do_calibration(imu_terrain_t * p_sys, uint32_t now_ms)
{
    terrain_sample_t s;
    imu_accel_cal_t  cal;
    imu_vec3_t       v;
    imu_err_t        err;

    if (!time_reached(now_ms, p_sys->next_action_ms))
    {
        return;
    }

    terrain_acquire(p_sys, now_ms, &s);
    if (!terrain_check_sample(p_sys, now_ms, &s))
    {
        return;
    }

    imu_proc_map_accel(&p_sys->proc, &s.accel_avg_g, &v);
    imu_cal_accel_add(&p_sys->cal_acc, &v);

    if (p_sys->cal_acc.n < p_sys->params.cal_samples)
    {
        return;
    }

    err = imu_cal_accel_finish(&p_sys->cal_acc, &p_sys->params, &cal);
    if (IMU_OK == err)
    {
        imu_proc_set_calibration(&p_sys->proc, &cal, NULL);
        p_sys->status.cal_g_ref          = cal.g_ref;
        p_sys->status.cal_noise_g        = cal.noise_max_std_g;
        p_sys->status.cal_mount_tilt_deg = cal.mount_tilt_deg;
        p_sys->cal_attempts = 0U;
        p_sys->fault        = IMU_FAULT_NONE;
        terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
        terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS, IMU_SEV_INFO,
                            IMU_STATUS_READY, IMU_OK);
        return;
    }

    /* Calibration failed -> CALIBRATION again (statechart), bounded report */
    p_sys->cal_attempts++;
    p_sys->last_error = err;
    p_sys->attempts   = p_sys->cal_attempts;
    terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS, IMU_SEV_MINOR,
                        IMU_STATUS_CAL_RETRY, err);

    if ((p_sys->cal_attempts >= p_sys->params.cal_max_attempts)
        && !p_sys->b_cal_fault_reported)
    {
        p_sys->fault = IMU_FAULT_CAL_FAILED;
        p_sys->b_cal_fault_reported = true;
        terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT,
                            IMU_SEV_CRITICAL, IMU_STATUS_FAULT_RAISED, err);
    }

    imu_cal_accel_begin(&p_sys->cal_acc);
    p_sys->next_action_ms = now_ms + p_sys->params.cal_retry_ms;
}

static void terrain_do_monitoring(imu_terrain_t * p_sys, uint32_t now_ms)
{
    terrain_sample_t s;

    terrain_acquire(p_sys, now_ms, &s);
    if (terrain_check_sample(p_sys, now_ms, &s))
    {
        (void)terrain_process(p_sys, now_ms, &s, true);
    }
}

/** A recovery attempt failed (re-init or verification): back-off and, once
 *  the attempt budget is exhausted, report the fault as PERSISTENT (once).
 *  The subsystem keeps trying at the maximum interval but never resumes
 *  without a successful verification. */
static void terrain_recovery_failed(imu_terrain_t * p_sys, uint32_t now_ms,
                                    imu_err_t err)
{
    imu_params_t const * p_params = &p_sys->params;

    p_sys->last_error = err;
    p_sys->retry_interval_ms = (p_sys->retry_interval_ms
                                < (p_params->recovery_interval_max_ms / 2U))
                               ? (p_sys->retry_interval_ms * 2U)
                               : p_params->recovery_interval_max_ms;
    p_sys->next_action_ms = now_ms + p_sys->retry_interval_ms;

    if ((p_sys->attempts >= p_params->recovery_max_attempts)
        && !p_sys->b_persistent_reported)
    {
        p_sys->b_persistent_reported = true;
        terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT,
                            IMU_SEV_CRITICAL, IMU_STATUS_FAULT_PERSISTENT,
                            err);
    }
}

static void terrain_do_fault_safe(imu_terrain_t * p_sys, uint32_t now_ms)
{
    imu_params_t const * p_params = &p_sys->params;
    terrain_sample_t     s;
    imu_err_t            err;

    if (IMU_FAULT_COLLISION == p_sys->fault)
    {
        /* Sensor still works: keep the status current, wait for the MC
         * acknowledgement and settled conditions. */
        terrain_acquire(p_sys, now_ms, &s);
        if (!terrain_check_sample(p_sys, now_ms, &s))
        {
            return;   /* may have escalated to a sensor fault */
        }
        (void)terrain_process(p_sys, now_ms, &s, false);

        if (p_sys->last_out.b_attitude_reliable
            && (p_sys->last_out.tilt_deg < p_params->tilt_threshold_deg))
        {
            if (!p_sys->b_settle_timing)
            {
                p_sys->b_settle_timing = true;
                p_sys->settle_start_ms = now_ms;
            }
        }
        else
        {
            p_sys->b_settle_timing = false;
        }

        if (p_sys->b_fault_ack && p_sys->b_settle_timing
            && ((now_ms - p_sys->settle_start_ms)
                >= p_params->collision_settle_ms))
        {
            p_sys->fault       = IMU_FAULT_NONE;
            p_sys->b_fault_ack = false;
            terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
            terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS, IMU_SEV_INFO,
                                IMU_STATUS_RESUMED, IMU_OK);
        }
        return;
    }

    /* Sensor fault: bounded recovery attempts with back-off. */
    if (!time_reached(now_ms, p_sys->next_action_ms))
    {
        return;
    }

    p_sys->attempts++;
    err = lsm303_recover(&p_sys->dev);

    if (IMU_OK == err)
    {
        p_sys->verify_count     = 0U;
        p_sys->consec_fail      = 0U;
        p_sys->consec_invalid   = 0U;
        p_sys->b_last_raw_valid = false;
        p_sys->frozen_count     = 0U;
        p_sys->last_accel_ok_ms = now_ms;
        terrain_set_state(p_sys, now_ms, IMU_STATE_RECOVERY);
        return;
    }

    terrain_recovery_failed(p_sys, now_ms, err);
}

static void terrain_do_recovery(imu_terrain_t * p_sys, uint32_t now_ms)
{
    terrain_sample_t s;

    terrain_acquire(p_sys, now_ms, &s);

    if ((IMU_OK != s.err) || ((s.count > 0U) && (!s.b_valid || s.b_frozen)))
    {
        /* Verification failed: back to FAULT / SAFE STATE. */
        terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_SAFE);
        terrain_recovery_failed(p_sys, now_ms,
                                (IMU_OK != s.err) ? s.err
                                                  : IMU_ERR_INVALID_DATA);
        return;
    }

    if (s.count > 0U)
    {
        p_sys->verify_count++;
    }

    /* Enough plausible samples AND evidence that the output is alive
     * (a frozen sensor would otherwise pass a short verification). */
    if ((p_sys->verify_count < p_sys->params.recovery_verify_samples)
        || ((p_sys->frozen_count + 1U) >= p_sys->verify_count))
    {
        return;
    }

    p_sys->fault = IMU_FAULT_NONE;
    if (p_sys->proc.accel_cal.b_valid)
    {
        imu_proc_reset_dynamic(&p_sys->proc);
        terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
    }
    else
    {
        terrain_begin_calibration(p_sys, now_ms);
    }
    terrain_emit_status(p_sys, now_ms, IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL,
                        IMU_STATUS_RECOVERED, IMU_OK);
}

/* ------------------------------------------------------------------------ */
/*  Commands                                                                */
/* ------------------------------------------------------------------------ */

static bool terrain_is_operational(imu_state_t state)
{
    return ((IMU_STATE_MONITORING == state)
            || (IMU_STATE_HUMP_ASCENT == state)
            || (IMU_STATE_HUMP_PEAK == state)
            || (IMU_STATE_HUMP_DESCENT == state));
}

static void terrain_handle_commands(imu_terrain_t * p_sys, uint32_t now_ms)
{
    uint32_t cmds = p_sys->pending_cmds;

    p_sys->pending_cmds = 0U;

    if (0U != (cmds & IMU_CMD_STOP))
    {
        lsm303_shutdown(&p_sys->dev);
        imu_hump_abort(&p_sys->hump, now_ms);
        terrain_set_state(p_sys, now_ms, IMU_STATE_STOPPED);
        return;
    }

    if (0U != (cmds & IMU_CMD_RESET))
    {
        imu_hump_abort(&p_sys->hump, now_ms);
        p_sys->fault             = IMU_FAULT_NONE;
        p_sys->attempts          = 0U;
        p_sys->retry_interval_ms = p_sys->params.init_retry_ms;
        p_sys->next_action_ms    = now_ms;
        p_sys->b_mag_cal_active  = false;
        p_sys->proc.accel_cal.b_valid = false;
        terrain_set_state(p_sys, now_ms, IMU_STATE_INITIALISATION);
    }

    if (0U != (cmds & IMU_CMD_MISSION_START))
    {
        imu_hump_reset_mission(&p_sys->hump);
        p_sys->status.b_has_impact = false;
        terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS, IMU_SEV_INFO,
                            IMU_STATUS_MISSION_RESET, IMU_OK);
    }

    if (0U != (cmds & IMU_CMD_CLEAR_FAULT))
    {
        p_sys->b_fault_ack = true;
    }

    if ((0U != (cmds & IMU_CMD_RECALIBRATE))
        && terrain_is_operational(p_sys->state))
    {
        imu_hump_abort(&p_sys->hump, now_ms);
        p_sys->cal_attempts = 0U;
        p_sys->next_action_ms = now_ms;
        terrain_begin_calibration(p_sys, now_ms);
    }

    if ((0U != (cmds & IMU_CMD_MAG_CAL_BEGIN))
        && terrain_is_operational(p_sys->state))
    {
        imu_cal_mag_begin(&p_sys->mag_acc);
        p_sys->b_mag_cal_active = true;
    }

    if ((0U != (cmds & IMU_CMD_MAG_CAL_END)) && p_sys->b_mag_cal_active)
    {
        imu_mag_cal_t mag;
        imu_err_t     err = imu_cal_mag_finish(&p_sys->mag_acc,
                                               &p_sys->params, &mag);

        p_sys->b_mag_cal_active = false;
        if (IMU_OK == err)
        {
            imu_proc_set_calibration(&p_sys->proc, NULL, &mag);
            imu_hump_abort(&p_sys->hump, now_ms);
            terrain_set_state(p_sys, now_ms, IMU_STATE_MONITORING);
        }
        terrain_emit_status(p_sys, now_ms, IMU_EVT_STATUS,
                            (IMU_OK == err) ? IMU_SEV_INFO : IMU_SEV_MINOR,
                            (IMU_OK == err) ? IMU_STATUS_MAG_CAL_OK
                                            : IMU_STATUS_MAG_CAL_FAILED,
                            err);
    }
}

/* ------------------------------------------------------------------------ */
/*  Status                                                                  */
/* ------------------------------------------------------------------------ */

static void terrain_update_status(imu_terrain_t * p_sys, uint32_t now_ms)
{
    imu_status_t *         p_st  = &p_sys->status;
    imu_proc_out_t const * p_out = &p_sys->last_out;

    p_st->timestamp_ms        = now_ms;
    p_st->state               = p_sys->state;
    p_st->fault               = p_sys->fault;
    p_st->last_error          = p_sys->last_error;
    p_st->b_calibrated        = p_sys->proc.accel_cal.b_valid;
    p_st->b_sensor_ok         = p_sys->dev.b_ready
                                && (IMU_STATE_FAULT_SAFE != p_sys->state
                                    || (IMU_FAULT_COLLISION == p_sys->fault))
                                && (IMU_STATE_RECOVERY != p_sys->state)
                                && (IMU_STATE_INITIALISATION != p_sys->state);
    p_st->b_mag_cal_valid     = p_sys->proc.mag_cal.b_valid;
    p_st->b_mag_cal_active    = p_sys->b_mag_cal_active;
    p_st->mag_cal             = p_sys->proc.mag_cal;

    p_st->motion              = p_sys->proc.motion;
    p_st->motion_conf         = p_sys->proc.motion_conf;
    p_st->pitch_deg           = p_out->pitch_deg;
    p_st->roll_deg            = p_out->roll_deg;
    p_st->tilt_deg            = p_out->tilt_deg;
    p_st->b_attitude_reliable = p_out->b_attitude_reliable;
    p_st->b_tilted            = p_sys->proc.b_tilted;
    p_st->heading_deg         = p_out->heading_deg;
    p_st->heading_quality     = p_out->heading_quality;
    p_st->turn_rate_dps       = p_out->turn_rate_dps;
    p_st->b_turn_rate_valid   = p_out->b_turn_rate_valid;
    p_st->turn_dir            = p_out->turn_dir;
    p_st->fwd_accel_g         = p_out->fwd_accel_g;
    p_st->vert_accel_g        = p_out->vert_accel_g;
    p_st->vibration_g         = p_out->vibration_g;
    p_st->mag_field_gauss     = p_out->mag_field_gauss;

    p_st->hump_phase          = p_sys->hump.phase;
    p_st->hump                = p_sys->hump.stats;
    p_st->i2c                 = p_sys->dev.stats;
    p_st->events              = p_sys->disp.stats;
    p_st->cycles              = p_sys->cycles;
    p_st->consecutive_failures = p_sys->consec_fail;
    p_st->recovery_attempts   = p_sys->attempts;
    p_st->hints_rejected      = p_sys->proc.hints_rejected;
}

/* ------------------------------------------------------------------------ */
/*  Public API                                                              */
/* ------------------------------------------------------------------------ */

imu_err_t imu_terrain_init(imu_terrain_t * p_sys,
                           imu_params_t const * p_params,
                           imu_event_sink_t const * p_sink)
{
    imu_err_t err;

    if (NULL == p_sys)
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_sys, 0, sizeof(*p_sys));

    if (NULL != p_params)
    {
        p_sys->params = *p_params;
    }
    else
    {
        imu_params_default(&p_sys->params);
    }

    err = imu_params_validate(&p_sys->params);
    if (IMU_OK != err)
    {
        return (err);
    }

    err = imu_proc_init(&p_sys->proc, &p_sys->params);
    if (IMU_OK == err)
    {
        err = imu_hump_init(&p_sys->hump, &p_sys->params);
    }
    if (IMU_OK != err)
    {
        return (err);
    }

    imu_events_init(&p_sys->disp, p_sink);
    imu_events_set_rate_limit(&p_sys->disp, IMU_EVT_MOTION,
                              p_sys->params.motion_evt_min_ms);
    imu_events_set_rate_limit(&p_sys->disp, IMU_EVT_TILT,
                              p_sys->params.tilt_evt_min_ms);
    imu_events_set_rate_limit(&p_sys->disp, IMU_EVT_TURNING,
                              p_sys->params.turn_evt_min_ms);

    p_sys->state             = IMU_STATE_INITIALISATION;
    p_sys->fault             = IMU_FAULT_NONE;
    p_sys->last_error        = IMU_OK;
    p_sys->retry_interval_ms = p_sys->params.init_retry_ms;
    p_sys->last_out.motion   = IMU_MOTION_UNKNOWN;
    p_sys->last_out.heading_quality = IMU_HEADING_INVALID;
    terrain_update_status(p_sys, 0U);

    return (IMU_OK);
}

void imu_terrain_step(imu_terrain_t * p_sys, uint32_t now_ms)
{
    if (NULL == p_sys)
    {
        return;
    }

    if (0U == p_sys->cycles)
    {
        p_sys->next_action_ms = now_ms;   /* first call: act immediately */
    }
    p_sys->cycles++;

    terrain_handle_commands(p_sys, now_ms);
    imu_events_service(&p_sys->disp);

    switch (p_sys->state)
    {
        case IMU_STATE_INITIALISATION:
            terrain_do_init(p_sys, now_ms);
            break;

        case IMU_STATE_CALIBRATION:
            terrain_do_calibration(p_sys, now_ms);
            break;

        case IMU_STATE_MONITORING:
        case IMU_STATE_HUMP_ASCENT:
        case IMU_STATE_HUMP_PEAK:
        case IMU_STATE_HUMP_DESCENT:
            terrain_do_monitoring(p_sys, now_ms);
            break;

        case IMU_STATE_FAULT_SAFE:
            terrain_do_fault_safe(p_sys, now_ms);
            break;

        case IMU_STATE_RECOVERY:
            terrain_do_recovery(p_sys, now_ms);
            break;

        case IMU_STATE_STOPPED:
            break;

        default:
            /* Transient states never persist between steps; defensive. */
            terrain_set_state(p_sys, now_ms, IMU_STATE_FAULT_SAFE);
            break;
    }

    terrain_update_status(p_sys, now_ms);
}

imu_err_t imu_terrain_command(imu_terrain_t * p_sys, uint32_t cmd_mask)
{
    if ((NULL == p_sys) || (0U == cmd_mask)
        || (0U != (cmd_mask & ~IMU_CMD_ALL)))
    {
        return (IMU_ERR_PARAM);
    }

    p_sys->pending_cmds |= cmd_mask;
    return (IMU_OK);
}

imu_err_t imu_terrain_set_hint(imu_terrain_t * p_sys,
                               imu_motion_hint_t const * p_hint)
{
    if ((NULL == p_sys) || (NULL == p_hint))
    {
        return (IMU_ERR_PARAM);
    }

    return (imu_proc_set_hint(&p_sys->proc, p_hint));
}

imu_err_t imu_terrain_get_status(imu_terrain_t const * p_sys,
                                 imu_status_t * p_out)
{
    if ((NULL == p_sys) || (NULL == p_out))
    {
        return (IMU_ERR_PARAM);
    }

    *p_out = p_sys->status;
    return (IMU_OK);
}

uint8_t imu_terrain_get_trace(imu_terrain_t const * p_sys,
                              imu_trace_entry_t * p_out, uint8_t max)
{
    uint8_t i;
    uint8_t n;

    if ((NULL == p_sys) || (NULL == p_out))
    {
        return (0U);
    }

    n = (p_sys->trace_count < max) ? p_sys->trace_count : max;
    for (i = 0U; i < n; i++)
    {
        p_out[i] = p_sys->trace[(p_sys->trace_head
                                 + (p_sys->trace_count - n) + i)
                                % IMU_STATE_TRACE_LEN];
    }

    return (n);
}

void imu_terrain_make_telemetry(imu_status_t const * p_status,
                                imu_telemetry_t * p_out)
{
    imu_hump_stats_t const * p_h;

    if ((NULL == p_status) || (NULL == p_out))
    {
        return;
    }

    p_h = &p_status->hump;
    (void)memset(p_out, 0, sizeof(*p_out));
    p_out->timestamp_ms   = p_status->timestamp_ms;
    p_out->state          = (uint8_t)p_status->state;
    p_out->motion         = (uint8_t)p_status->motion;
    p_out->hump_phase     = (uint8_t)p_status->hump_phase;
    p_out->fault          = (uint8_t)p_status->fault;
    p_out->pitch_cdeg     = to_i16(p_status->pitch_deg, TERRAIN_CDEG);
    p_out->roll_cdeg      = to_i16(p_status->roll_deg, TERRAIN_CDEG);
    p_out->heading_cdeg   =
        to_i16_or_none(IMU_HEADING_INVALID != p_status->heading_quality,
                       p_status->heading_deg, TERRAIN_CDEG);
    p_out->turn_rate_cdps = to_i16(p_status->b_turn_rate_valid
                                   ? p_status->turn_rate_dps : 0.0f,
                                   TERRAIN_CDEG);
    p_out->hump_count     = (uint16_t)((p_h->valid_count > 65535U)
                                       ? 65535U : p_h->valid_count);
    p_out->last_hump_peak_cdeg =
        to_i16_or_none(p_h->b_has_last, p_h->last.peak_pitch_deg,
                       TERRAIN_CDEG);
    p_out->hump_peak_cdeg =
        to_i16_or_none(p_h->b_has_max, p_h->max.peak_pitch_deg,
                       TERRAIN_CDEG);
    p_out->hump_peak_height_mm =
        to_i16_or_none(p_h->b_has_max
                       && (IMU_CONF_NONE != p_h->max.height_conf),
                       p_h->max.height_est_mm, 1.0f);
    p_out->hump_height_conf = p_h->b_has_max ? (uint8_t)p_h->max.height_conf
                                             : (uint8_t)IMU_CONF_NONE;
    p_out->b_sensor_ok    = p_status->b_sensor_ok ? 1U : 0U;
}

int32_t imu_terrain_telemetry_json(imu_telemetry_t const * p_tele,
                                   char * p_buf, size_t len)
{
    int n;

    if ((NULL == p_tele) || (NULL == p_buf) || (0U == len))
    {
        return (-1);
    }

    n = snprintf(p_buf, len,
                 "\"hump_peak\":%d,\"hump_peak_mm\":%d,\"hump_count\":%u,"
                 "\"imu_state\":%u,\"motion\":%u,\"pitch\":%d,"
                 "\"heading\":%d,\"imu_ok\":%u",
                 (int)p_tele->hump_peak_cdeg,
                 (int)p_tele->hump_peak_height_mm,
                 (unsigned)p_tele->hump_count, (unsigned)p_tele->state,
                 (unsigned)p_tele->motion, (int)p_tele->pitch_cdeg,
                 (int)p_tele->heading_cdeg, (unsigned)p_tele->b_sensor_ok);

    if ((n < 0) || ((size_t)n >= len))
    {
        p_buf[0] = '\0';
        return (-1);
    }

    return ((int32_t)n);
}

char const * imu_state_name(imu_state_t state)
{
    static char const * const k_names[IMU_STATE_COUNT] =
    {
        "INITIALISATION", "CALIBRATION", "MONITORING", "HUMP_ASCENT",
        "HUMP_PEAK", "HUMP_DESCENT", "TILT_EVENT", "TURN_EVENT",
        "IMPACT_EVENT", "FAULT_HANDLING", "SENSOR_FAULT", "FAULT_SAFE",
        "RECOVERY", "STOPPED"
    };

    return (((uint32_t)state < (uint32_t)IMU_STATE_COUNT)
            ? k_names[state] : "INVALID");
}

char const * imu_fault_name(imu_fault_t fault)
{
    static char const * const k_names[] =
    {
        "NONE", "INIT_FAILED", "CAL_FAILED", "I2C", "STALE_DATA",
        "INVALID_DATA", "FROZEN_DATA", "COLLISION"
    };

    return (((uint32_t)fault <= (uint32_t)IMU_FAULT_COLLISION)
            ? k_names[fault] : "INVALID");
}
