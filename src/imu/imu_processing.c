/**
 * @file    imu_processing.c
 * @brief   Buddy 4 signal processing (see imu_processing.h for the model).
 */

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "imu/imu_processing.h"

#define PROC_STANDARD_GRAVITY_MM_S2  (9806.65f)
#define PROC_TIME_GAP_RESET_MS       (1000U)   /* discontinuity -> re-seed */
#define PROC_RELEASE_FRACTION        (0.7f)    /* impact hysteresis        */
#define PROC_UNWRAP_REBASE_DEG       (3600.0f)
#define PROC_MIN_FIELD_GAUSS         (0.05f)
#define PROC_MAX_FIELD_GAUSS         (4.0f)
#define PROC_MIN_EAST_NORM           (0.02f)
#define PROC_HINT_DIFF_MIN_MS        (20U)
#define PROC_HINT_DIFF_MAX_MS        (500U)
#define PROC_HINT_DIFF_ALPHA         (0.5f)
#define PROC_NOISE_VIB_FACTOR        (3.0f)

/* ------------------------------------------------------------------------ */
/*  Init / calibration                                                      */
/* ------------------------------------------------------------------------ */

static float proc_odr_period_s(imu_params_t const * p_params)
{
    uint32_t hz = imu_params_accel_odr_hz(p_params->accel_odr_code);

    return ((hz > 0U) ? (1.0f / (float)hz) : 0.01f);
}

imu_err_t imu_proc_init(imu_proc_t * p_proc, imu_params_t const * p_params)
{
    if ((NULL == p_proc) || (NULL == p_params))
    {
        return (IMU_ERR_PARAM);
    }

    if (IMU_OK != imu_params_validate(p_params))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_proc, 0, sizeof(*p_proc));
    p_proc->p_params = p_params;

    imu_mat_identity(&p_proc->accel_cal.level_rot);
    p_proc->accel_cal.g_ref = 1.0f;
    p_proc->accel_cal.b_valid = false;
    imu_cal_mag_from_params(p_params, &p_proc->mag_cal);

    imu_proc_reset_dynamic(p_proc);

    return (IMU_OK);
}

void imu_proc_set_calibration(imu_proc_t * p_proc,
                              imu_accel_cal_t const * p_accel,
                              imu_mag_cal_t const * p_mag)
{
    if (NULL == p_proc)
    {
        return;
    }

    if (NULL != p_accel)
    {
        p_proc->accel_cal = *p_accel;
    }
    if (NULL != p_mag)
    {
        p_proc->mag_cal = *p_mag;
    }

    imu_proc_reset_dynamic(p_proc);
}

void imu_proc_reset_dynamic(imu_proc_t * p_proc)
{
    imu_params_t const * p_params;
    float                odr_dt;

    if ((NULL == p_proc) || (NULL == p_proc->p_params))
    {
        return;
    }

    p_params = p_proc->p_params;
    odr_dt   = proc_odr_period_s(p_params);

    p_proc->b_seeded            = false;
    p_proc->b_hf_seeded         = false;
    p_proc->hf_alpha            = imu_lpf_alpha(p_params->impact_lpf_hz,
                                                odr_dt);
    p_proc->vib_alpha           = imu_lpf_alpha(p_params->vib_lpf_hz, odr_dt);
    p_proc->vib_var             = 0.0f;
    p_proc->b_imp_active        = false;
    p_proc->b_imp_sustained     = false;
    p_proc->b_imp_cooldown      = false;
    p_proc->imp_suppressed      = 0U;
    p_proc->up_last.x           = 0.0f;
    p_proc->up_last.y           = 0.0f;
    p_proc->up_last.z           = 1.0f;
    p_proc->pitch_deg           = 0.0f;
    p_proc->roll_deg            = 0.0f;
    p_proc->tilt_deg            = 0.0f;
    p_proc->b_attitude_reliable = false;

    p_proc->b_heading_seeded    = false;
    p_proc->heading_outliers    = 0U;
    p_proc->hist_head           = 0U;
    p_proc->hist_count          = 0U;
    p_proc->b_turn_rate_valid   = false;
    p_proc->turn_rate_dps       = 0.0f;
    p_proc->heading_quality     = IMU_HEADING_INVALID;

    p_proc->turn_state          = IMU_TURN_NONE;
    p_proc->b_turn_cand         = false;
    p_proc->b_turn_exit_timing  = false;

    p_proc->b_tilted            = false;
    p_proc->b_tilt_critical_sent = false;
    p_proc->b_tilt_cand         = false;

    p_proc->motion              = IMU_MOTION_UNKNOWN;
    p_proc->motion_cand         = IMU_MOTION_UNKNOWN;
    p_proc->motion_conf         = IMU_CONF_NONE;
    p_proc->b_impact_hold       = false;
}

void imu_proc_map_accel(imu_proc_t const * p_proc,
                        imu_vec3_t const * p_sensor_g,
                        imu_vec3_t * p_vehicle_g)
{
    imu_vec3_t corrected;

    imu_vec_sub(p_sensor_g, &p_proc->p_params->accel_bias_g, &corrected);
    imu_frame_map(p_proc->p_params->axis_map, p_proc->p_params->axis_sign,
                  &corrected, p_vehicle_g);
}

void imu_proc_level_accel(imu_proc_t const * p_proc,
                          imu_vec3_t const * p_sensor_g,
                          imu_vec3_t * p_level_g)
{
    imu_vec3_t v;

    imu_proc_map_accel(p_proc, p_sensor_g, &v);
    imu_mat_apply(&p_proc->accel_cal.level_rot, &v, p_level_g);
}

/* ------------------------------------------------------------------------ */
/*  High-rate impact detection                                              */
/* ------------------------------------------------------------------------ */

static imu_severity_t proc_impact_severity(imu_params_t const * p_params,
                                           imu_impact_t const * p_peak)
{
    imu_severity_t sev   = IMU_SEV_MINOR;
    float          ratio = (p_peak->peak_g > 0.0f)
                           ? (p_peak->horiz_g / p_peak->peak_g) : 0.0f;

    /* CRITICAL: very large in any direction, or large and mostly
     * horizontal (collision-like). Otherwise MINOR: bump, kerb, hump
     * edge, hand knock. */
    if ((p_peak->peak_g >= p_params->impact_critical_any_g)
        || ((p_peak->peak_g >= p_params->impact_critical_g)
            && (ratio >= p_params->impact_horiz_ratio)))
    {
        sev = IMU_SEV_CRITICAL;
    }

    return (sev);
}

void imu_proc_hf_accel(imu_proc_t * p_proc, imu_vec3_t const * p_sensor_g,
                       uint32_t t_ms, bool b_hump_active,
                       imu_impact_t * p_out)
{
    imu_params_t const * p_params;
    imu_vec3_t           a;
    imu_vec3_t           hp;
    float                mag;
    float                thr;
    float                release;

    if ((NULL == p_proc) || (NULL == p_sensor_g) || (NULL == p_out))
    {
        return;
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    if (!imu_vec_is_finite(p_sensor_g))
    {
        return;
    }

    p_params = p_proc->p_params;
    imu_proc_level_accel(p_proc, p_sensor_g, &a);

    if (!p_proc->b_hf_seeded)
    {
        p_proc->hf_lp        = a;
        p_proc->b_hf_seeded  = true;
        p_proc->hf_seed_t_ms = t_ms;
        return;
    }

    imu_vec_sub(&a, &p_proc->hf_lp, &hp);
    p_proc->hf_lp.x = imu_lpf_step(p_proc->hf_lp.x, a.x, p_proc->hf_alpha);
    p_proc->hf_lp.y = imu_lpf_step(p_proc->hf_lp.y, a.y, p_proc->hf_alpha);
    p_proc->hf_lp.z = imu_lpf_step(p_proc->hf_lp.z, a.z, p_proc->hf_alpha);

    mag = imu_vec_norm(&hp);
    p_proc->vib_var = imu_lpf_step(p_proc->vib_var, mag * mag,
                                   p_proc->vib_alpha);

    if ((t_ms - p_proc->hf_seed_t_ms) < p_params->impact_warmup_ms)
    {
        return;   /* filter settling: no detection */
    }

    thr = p_params->impact_minor_g;
    if (b_hump_active)
    {
        thr *= p_params->hump_impact_thr_scale;
    }
    release = thr * PROC_RELEASE_FRACTION;

    if (p_proc->b_imp_sustained)
    {
        /* Long excursion rejected: wait until it has clearly ended. */
        if (mag < release)
        {
            p_proc->b_imp_sustained = false;
        }
        return;
    }

    if (!p_proc->b_imp_active)
    {
        if (mag > thr)
        {
            p_proc->b_imp_active       = true;
            p_proc->imp_start_ms       = t_ms;
            p_proc->imp_last_above_ms  = t_ms;
            p_proc->imp_peak.peak_g    = mag;
            p_proc->imp_peak.horiz_g   = sqrtf((hp.x * hp.x) + (hp.y * hp.y));
            p_proc->imp_peak.vert_g    = fabsf(hp.z);
            p_proc->imp_peak.t_ms      = t_ms;
        }
        return;
    }

    if (mag > p_proc->imp_peak.peak_g)
    {
        p_proc->imp_peak.peak_g  = mag;
        p_proc->imp_peak.horiz_g = sqrtf((hp.x * hp.x) + (hp.y * hp.y));
        p_proc->imp_peak.vert_g  = fabsf(hp.z);
        p_proc->imp_peak.t_ms    = t_ms;
    }

    if (mag > release)
    {
        p_proc->imp_last_above_ms = t_ms;
    }

    if ((t_ms - p_proc->imp_start_ms) > p_params->impact_max_duration_ms)
    {
        /* Too long for an impact: sustained manoeuvre / vibration. */
        p_proc->b_imp_active    = false;
        p_proc->b_imp_sustained = true;
        p_proc->imp_rejected_long++;
        return;
    }

    if (mag <= release)
    {
        imu_severity_t sev;
        bool           b_report = true;

        p_proc->b_imp_active = false;
        sev = proc_impact_severity(p_params, &p_proc->imp_peak);

        if (p_proc->b_imp_cooldown
            && ((t_ms - p_proc->imp_last_report_ms)
                < p_params->impact_cooldown_ms))
        {
            /* Within cool-down: only an escalation is reported. */
            if (sev <= p_proc->imp_last_sev)
            {
                b_report = false;
                p_proc->imp_suppressed++;
            }
        }

        if (b_report)
        {
            *p_out = p_proc->imp_peak;
            p_out->b_detected  = true;
            p_out->severity    = sev;
            p_out->duration_ms = p_proc->imp_last_above_ms
                               - p_proc->imp_start_ms;
            p_out->suppressed  = p_proc->imp_suppressed;

            p_proc->imp_suppressed     = 0U;
            p_proc->imp_last_report_ms = t_ms;
            p_proc->imp_last_sev       = sev;
            p_proc->b_imp_cooldown     = true;
        }
    }
}

/* ------------------------------------------------------------------------ */
/*  Heading                                                                 */
/* ------------------------------------------------------------------------ */

static void proc_hist_clear(imu_proc_t * p_proc)
{
    p_proc->hist_head         = 0U;
    p_proc->hist_count        = 0U;
    p_proc->b_turn_rate_valid = false;
}

static void proc_hist_push(imu_proc_t * p_proc, uint32_t t_ms, float h)
{
    p_proc->hist_t[p_proc->hist_head] = t_ms;
    p_proc->hist_h[p_proc->hist_head] = h;
    p_proc->hist_head = (uint8_t)((p_proc->hist_head + 1U)
                                  % IMU_HEADING_HIST_LEN);
    if (p_proc->hist_count < IMU_HEADING_HIST_LEN)
    {
        p_proc->hist_count++;
    }
}

/** Rate = d(unwrapped heading)/dt over >= turn_rate_window_ms. */
static void proc_turn_rate(imu_proc_t * p_proc, uint32_t now_ms)
{
    uint8_t i;
    uint8_t idx;

    p_proc->b_turn_rate_valid = false;

    for (i = 1U; i < p_proc->hist_count; i++)
    {
        uint32_t age;

        idx = (uint8_t)((p_proc->hist_head + IMU_HEADING_HIST_LEN - 1U - i)
                        % IMU_HEADING_HIST_LEN);
        age = now_ms - p_proc->hist_t[idx];

        if (age >= p_proc->p_params->turn_rate_window_ms)
        {
            p_proc->turn_rate_dps = (p_proc->heading_unwrapped
                                     - p_proc->hist_h[idx])
                                  / ((float)age * 0.001f);
            p_proc->b_turn_rate_valid = true;
            break;
        }
    }
}

static void proc_unwrap_rebase(imu_proc_t * p_proc)
{
    float   shift;
    uint8_t i;

    if (fabsf(p_proc->heading_unwrapped) < PROC_UNWRAP_REBASE_DEG)
    {
        return;
    }

    shift = 360.0f * floorf(p_proc->heading_unwrapped / 360.0f);
    p_proc->heading_unwrapped     -= shift;
    p_proc->heading_ref_unwrapped -= shift;
    p_proc->turn_start_unw        -= shift;
    p_proc->turn_cand_unw         -= shift;
    for (i = 0U; i < IMU_HEADING_HIST_LEN; i++)
    {
        p_proc->hist_h[i] -= shift;
    }
}

/**
 * Tilt-compensated heading by vector algebra (convention-independent):
 *   up = unit gravity reaction (vehicle frame)
 *   E  = B x up (magnetic east),  N = up x E (magnetic north)
 *   heading of +X = atan2(E.x, N.x), clockwise from north.
 */
static bool proc_heading_raw(imu_vec3_t const * p_b, imu_vec3_t const * p_up,
                             float * p_heading)
{
    imu_vec3_t e;
    imu_vec3_t e_n;
    imu_vec3_t n;

    imu_vec_cross(p_b, p_up, &e);
    if ((imu_vec_norm(&e) < PROC_MIN_EAST_NORM)
        || !imu_vec_normalise(&e, &e_n))
    {
        return (false);   /* field almost vertical: heading undefined */
    }

    imu_vec_cross(p_up, &e_n, &n);
    *p_heading = imu_wrap_360(atan2f(e_n.x, n.x) * IMU_RAD_TO_DEG);

    return (0 != isfinite(*p_heading));
}

static void proc_heading_step(imu_proc_t * p_proc,
                              imu_vec3_t const * p_mag_sensor,
                              uint32_t now_ms, float dt_s)
{
    imu_params_t const * p_params = p_proc->p_params;
    imu_vec3_t           m_cal;
    imu_vec3_t           m_veh;
    imu_vec3_t           m_lev;
    float                raw;
    float                delta;
    float                alpha;

    if ((NULL == p_mag_sensor) || !imu_vec_is_finite(p_mag_sensor))
    {
        p_proc->heading_quality = IMU_HEADING_INVALID;
        proc_hist_clear(p_proc);
        return;
    }

    if (p_proc->mag_cal.b_valid)
    {
        imu_cal_mag_apply(&p_proc->mag_cal, p_mag_sensor, &m_cal);
    }
    else
    {
        m_cal = *p_mag_sensor;
    }

    imu_frame_map(p_params->axis_map, p_params->axis_sign, &m_cal, &m_veh);
    imu_mat_apply(&p_proc->accel_cal.level_rot, &m_veh, &m_lev);
    p_proc->mag_field = imu_vec_norm(&m_lev);

    if ((p_proc->mag_field < PROC_MIN_FIELD_GAUSS)
        || (p_proc->mag_field > PROC_MAX_FIELD_GAUSS)
        || !proc_heading_raw(&m_lev, &p_proc->up_last, &raw))
    {
        p_proc->heading_quality = IMU_HEADING_INVALID;
        proc_hist_clear(p_proc);
        return;
    }

    if (!p_proc->mag_cal.b_valid)
    {
        p_proc->heading_quality = IMU_HEADING_UNCALIBRATED;
    }
    else if ((p_proc->mag_cal.ref_field_gauss > 0.0f)
             && (fabsf(p_proc->mag_field - p_proc->mag_cal.ref_field_gauss)
                 > (p_params->mag_field_tol_frac
                    * p_proc->mag_cal.ref_field_gauss)))
    {
        p_proc->heading_quality = IMU_HEADING_DISTURBED;
    }
    else
    {
        p_proc->heading_quality = IMU_HEADING_GOOD;
    }

    if (IMU_HEADING_DISTURBED == p_proc->heading_quality)
    {
        /* Do not integrate a corrupted heading into the turn-rate history. */
        proc_hist_clear(p_proc);
        return;
    }

    if (!p_proc->b_heading_seeded)
    {
        p_proc->heading_f_deg         = raw;
        p_proc->heading_unwrapped     = raw;
        p_proc->heading_ref_unwrapped = raw;
        p_proc->b_heading_seeded      = true;
        p_proc->heading_outliers      = 0U;
        proc_hist_clear(p_proc);
        proc_hist_push(p_proc, now_ms, p_proc->heading_unwrapped);
        return;
    }

    delta = imu_wrap_180(raw - p_proc->heading_f_deg);

    if (fabsf(delta) > p_params->heading_max_jump_deg)
    {
        p_proc->heading_outliers++;
        if (p_proc->heading_outliers <= p_params->heading_outlier_limit)
        {
            return;   /* reject a single implausible jump */
        }
        /* Persistent: accept the new heading, restart the rate history. */
        p_proc->heading_f_deg = raw;
        p_proc->heading_unwrapped += delta;
        p_proc->heading_outliers = 0U;
        proc_hist_clear(p_proc);
        proc_hist_push(p_proc, now_ms, p_proc->heading_unwrapped);
        return;
    }

    p_proc->heading_outliers = 0U;
    alpha = imu_lpf_alpha(p_params->heading_lpf_hz, dt_s);
    p_proc->heading_f_deg     = imu_wrap_360(p_proc->heading_f_deg
                                             + (alpha * delta));
    p_proc->heading_unwrapped += alpha * delta;
    proc_unwrap_rebase(p_proc);
    proc_hist_push(p_proc, now_ms, p_proc->heading_unwrapped);
    proc_turn_rate(p_proc, now_ms);
}

static bool proc_heading_usable(imu_proc_t const * p_proc)
{
    return ((IMU_HEADING_GOOD == p_proc->heading_quality)
            || ((IMU_HEADING_UNCALIBRATED == p_proc->heading_quality)
                && p_proc->p_params->b_turn_allow_uncal_mag));
}

/* ------------------------------------------------------------------------ */
/*  Turning detector                                                        */
/* ------------------------------------------------------------------------ */

static void proc_turn_end(imu_proc_t * p_proc, uint32_t now_ms,
                          bool b_unreliable, imu_proc_out_t * p_out)
{
    p_out->b_turn_event            = true;
    p_out->turn_phase              = IMU_PHASE_END;
    p_out->turn_event_dir          = p_proc->turn_state;
    p_out->turn_event_delta_deg    = p_proc->heading_unwrapped
                                   - p_proc->turn_start_unw;
    p_out->turn_event_duration_ms  = now_ms - p_proc->turn_start_ms;
    p_out->b_turn_event_unreliable = b_unreliable;

    p_proc->turn_state            = IMU_TURN_NONE;
    p_proc->b_turn_exit_timing    = false;
    p_proc->b_turn_cand           = false;
    p_proc->heading_ref_unwrapped = p_proc->heading_unwrapped;
}

static void proc_turn_step(imu_proc_t * p_proc, uint32_t now_ms,
                           imu_proc_out_t * p_out)
{
    imu_params_t const * p_params = p_proc->p_params;
    bool                 b_ok     = proc_heading_usable(p_proc)
                                    && p_proc->b_turn_rate_valid;
    float                rate     = p_proc->turn_rate_dps;
    imu_turn_dir_t       dir      = (rate < 0.0f) ? IMU_TURN_LEFT
                                                  : IMU_TURN_RIGHT;

    if (IMU_TURN_NONE == p_proc->turn_state)
    {
        if (b_ok && (fabsf(rate) > p_params->turn_enter_dps))
        {
            if (!p_proc->b_turn_cand || (p_proc->turn_cand_dir != dir))
            {
                p_proc->b_turn_cand     = true;
                p_proc->turn_cand_dir   = dir;
                p_proc->turn_cand_start = now_ms;
                /* The turn began roughly one window earlier. */
                p_proc->turn_cand_unw   = p_proc->heading_unwrapped
                    - (rate * (float)p_params->turn_rate_window_ms * 0.001f);
            }
            else if ((now_ms - p_proc->turn_cand_start)
                     >= p_params->turn_min_duration_ms)
            {
                p_proc->turn_state         = dir;
                p_proc->turn_start_unw     = p_proc->turn_cand_unw;
                p_proc->turn_start_ms      = p_proc->turn_cand_start;
                p_proc->b_turn_cand        = false;
                p_proc->b_turn_exit_timing = false;

                p_out->b_turn_event            = true;
                p_out->turn_phase              = IMU_PHASE_START;
                p_out->turn_event_dir          = dir;
                p_out->turn_event_delta_deg    = p_proc->heading_unwrapped
                                               - p_proc->turn_start_unw;
                p_out->turn_event_duration_ms  = now_ms
                                               - p_proc->turn_start_ms;
                p_out->b_turn_event_unreliable = false;
            }
            else
            {
                /* keep timing the candidate */
            }
        }
        else
        {
            p_proc->b_turn_cand = false;

            /* Slow heading drift / gentle curve monitoring. */
            if (b_ok && p_proc->b_heading_seeded
                && (fabsf(p_proc->heading_unwrapped
                          - p_proc->heading_ref_unwrapped)
                    >= p_params->heading_change_deg))
            {
                p_out->b_turn_event            = true;
                p_out->turn_phase              = IMU_PHASE_HEADING_CHANGE;
                p_out->turn_event_delta_deg    = p_proc->heading_unwrapped
                                     - p_proc->heading_ref_unwrapped;
                p_out->turn_event_dir          =
                    (p_out->turn_event_delta_deg < 0.0f) ? IMU_TURN_LEFT
                                                         : IMU_TURN_RIGHT;
                p_out->turn_event_duration_ms  = 0U;
                p_out->b_turn_event_unreliable = false;
                p_proc->heading_ref_unwrapped  = p_proc->heading_unwrapped;
            }
        }
        return;
    }

    /* Currently turning: look for the end (low rate, reversal or loss). */
    if (!b_ok || (fabsf(rate) < p_params->turn_exit_dps)
        || (dir != p_proc->turn_state))
    {
        if (!p_proc->b_turn_exit_timing)
        {
            p_proc->b_turn_exit_timing = true;
            p_proc->turn_exit_start    = now_ms;
        }
        else if ((now_ms - p_proc->turn_exit_start)
                 >= p_params->turn_exit_duration_ms)
        {
            proc_turn_end(p_proc, now_ms, !b_ok, p_out);
        }
        else
        {
            /* keep timing the exit */
        }
    }
    else
    {
        p_proc->b_turn_exit_timing = false;
    }
}

/* ------------------------------------------------------------------------ */
/*  Tilt detector                                                           */
/* ------------------------------------------------------------------------ */

static void proc_tilt_step(imu_proc_t * p_proc, uint32_t now_ms,
                           bool b_hump_active, imu_proc_out_t * p_out)
{
    imu_params_t const * p_params = p_proc->p_params;
    float                tilt     = p_proc->tilt_deg;
    bool                 b_crit;

    if (!p_proc->b_attitude_reliable)
    {
        p_proc->b_tilt_cand = false;   /* gravity direction not trustworthy */
        return;
    }

    /* While crossing a hump the pitch excursion is expected: use roll only
     * unless the total tilt is already critical. */
    if (b_hump_active && (tilt < p_params->tilt_critical_deg))
    {
        tilt = fabsf(p_proc->roll_deg);
    }
    b_crit = (tilt >= p_params->tilt_critical_deg);

    if (!p_proc->b_tilted)
    {
        if (tilt > p_params->tilt_threshold_deg)
        {
            if (!p_proc->b_tilt_cand)
            {
                p_proc->b_tilt_cand     = true;
                p_proc->tilt_cand_start = now_ms;
            }
            if (((now_ms - p_proc->tilt_cand_start)
                 >= p_params->tilt_min_duration_ms) || b_crit)
            {
                p_proc->b_tilted             = true;
                p_proc->b_tilt_cand          = false;
                p_proc->b_tilt_critical_sent = b_crit;
                p_out->b_tilt_event  = true;
                p_out->tilt_phase    = IMU_PHASE_START;
                p_out->tilt_severity = b_crit ? IMU_SEV_CRITICAL
                                              : IMU_SEV_MINOR;
            }
        }
        else
        {
            p_proc->b_tilt_cand = false;
        }
        return;
    }

    if (b_crit && !p_proc->b_tilt_critical_sent)
    {
        p_proc->b_tilt_critical_sent = true;
        p_out->b_tilt_event  = true;
        p_out->tilt_phase    = IMU_PHASE_ESCALATE;
        p_out->tilt_severity = IMU_SEV_CRITICAL;
        return;
    }

    if (tilt < (p_params->tilt_threshold_deg - p_params->tilt_hysteresis_deg))
    {
        if (!p_proc->b_tilt_cand)
        {
            p_proc->b_tilt_cand     = true;
            p_proc->tilt_cand_start = now_ms;
        }
        else if ((now_ms - p_proc->tilt_cand_start)
                 >= p_params->tilt_min_duration_ms)
        {
            p_proc->b_tilted             = false;
            p_proc->b_tilt_cand          = false;
            p_proc->b_tilt_critical_sent = false;
            p_out->b_tilt_event  = true;
            p_out->tilt_phase    = IMU_PHASE_END;
            p_out->tilt_severity = IMU_SEV_INFO;
        }
        else
        {
            /* keep timing */
        }
    }
    else
    {
        p_proc->b_tilt_cand = false;
    }
}

/* ------------------------------------------------------------------------ */
/*  Main step                                                               */
/* ------------------------------------------------------------------------ */

void imu_proc_step(imu_proc_t * p_proc, imu_vec3_t const * p_accel_sensor_g,
                   imu_vec3_t const * p_mag_sensor_gauss, uint32_t now_ms,
                   bool b_hump_active, imu_proc_out_t * p_out)
{
    imu_params_t const * p_params;
    imu_vec3_t           a;
    imu_vec3_t           f;
    float                dt_s;
    float                norm;
    float                alpha;

    if ((NULL == p_proc) || (NULL == p_accel_sensor_g) || (NULL == p_out))
    {
        return;
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    p_out->motion      = p_proc->motion;
    p_out->motion_prev = p_proc->motion;
    p_out->motion_conf = p_proc->motion_conf;

    if (!imu_vec_is_finite(p_accel_sensor_g))
    {
        return;
    }

    p_params = p_proc->p_params;

    /* Time base: tolerate jitter, re-seed after a discontinuity (pause,
     * fault, or time moving backwards - unsigned difference is huge). */
    if (p_proc->b_seeded
        && ((now_ms - p_proc->last_t_ms) > PROC_TIME_GAP_RESET_MS))
    {
        imu_proc_reset_dynamic(p_proc);
    }

    if (p_proc->b_seeded && (now_ms != p_proc->last_t_ms))
    {
        dt_s = (float)(now_ms - p_proc->last_t_ms) * 0.001f;
    }
    else
    {
        dt_s = (float)p_params->sample_period_ms * 0.001f;
    }

    imu_proc_level_accel(p_proc, p_accel_sensor_g, &a);
    p_out->accel_cycle_g = a;

    if (!p_proc->b_seeded)
    {
        p_proc->accel_lp = a;
        p_proc->b_seeded = true;
    }
    else
    {
        alpha = imu_lpf_alpha(p_params->attitude_lpf_hz, dt_s);
        p_proc->accel_lp.x = imu_lpf_step(p_proc->accel_lp.x, a.x, alpha);
        p_proc->accel_lp.y = imu_lpf_step(p_proc->accel_lp.y, a.y, alpha);
        p_proc->accel_lp.z = imu_lpf_step(p_proc->accel_lp.z, a.z, alpha);
    }
    p_proc->last_t_ms = now_ms;

    /* Forward-acceleration compensation from the encoder hint (if any). */
    f = p_proc->accel_lp;
    p_proc->b_fwd_from_hint = false;
    if (imu_proc_hint_fresh(p_proc, now_ms) && p_proc->b_hint_accel_valid)
    {
        f.x -= p_proc->hint_accel_g;
        p_proc->fwd_accel_g     = p_proc->hint_accel_g;
        p_proc->b_fwd_from_hint = true;
    }
    else
    {
        /* Without encoders: valid only on level ground (not in a hump). */
        p_proc->fwd_accel_g = p_proc->accel_lp.x;
    }

    norm = imu_vec_norm(&f);
    p_proc->b_attitude_reliable =
        (fabsf(norm - p_proc->accel_cal.g_ref)
         <= p_params->attitude_valid_tol_g);

    p_proc->pitch_deg = atan2f(f.x, sqrtf((f.y * f.y) + (f.z * f.z)))
                      * IMU_RAD_TO_DEG;
    p_proc->roll_deg  = atan2f(f.y, f.z) * IMU_RAD_TO_DEG;
    p_proc->tilt_deg  = atan2f(sqrtf((f.x * f.x) + (f.y * f.y)), f.z)
                      * IMU_RAD_TO_DEG;

    if (p_proc->b_attitude_reliable)
    {
        (void)imu_vec_normalise(&f, &p_proc->up_last);
    }

    proc_heading_step(p_proc, p_mag_sensor_gauss, now_ms, dt_s);
    proc_turn_step(p_proc, now_ms, p_out);
    proc_tilt_step(p_proc, now_ms, b_hump_active, p_out);

    p_out->pitch_deg           = p_proc->pitch_deg;
    p_out->roll_deg            = p_proc->roll_deg;
    p_out->tilt_deg            = p_proc->tilt_deg;
    p_out->b_attitude_reliable = p_proc->b_attitude_reliable;
    p_out->accel_lp_g          = p_proc->accel_lp;
    p_out->fwd_accel_g         = p_proc->fwd_accel_g;
    p_out->b_fwd_from_hint     = p_proc->b_fwd_from_hint;
    p_out->vert_accel_g        = a.z - p_proc->accel_cal.g_ref;
    p_out->vibration_g         = sqrtf(fmaxf(p_proc->vib_var, 0.0f));
    p_out->heading_deg         = p_proc->heading_f_deg;
    p_out->heading_quality     = p_proc->heading_quality;
    p_out->mag_field_gauss     = p_proc->mag_field;
    p_out->turn_rate_dps       = p_proc->b_turn_rate_valid
                                 ? p_proc->turn_rate_dps : 0.0f;
    p_out->b_turn_rate_valid   = p_proc->b_turn_rate_valid
                                 && proc_heading_usable(p_proc);
    p_out->turn_dir            = p_proc->turn_state;
}

/* ------------------------------------------------------------------------ */
/*  Motion classification                                                   */
/* ------------------------------------------------------------------------ */

void imu_proc_classify(imu_proc_t * p_proc, uint32_t now_ms,
                       imu_hump_phase_t hump_phase, bool b_new_impact,
                       imu_proc_out_t * p_out)
{
    imu_params_t const * p_params;
    imu_motion_class_t   cand;
    imu_confidence_t     conf;
    bool                 b_immediate = true;
    bool                 b_hint;
    float                acc_thr;
    float                dec_thr;
    float                vib_thr;

    if ((NULL == p_proc) || (NULL == p_out))
    {
        return;
    }

    p_params = p_proc->p_params;
    b_hint   = imu_proc_hint_fresh(p_proc, now_ms);
    p_out->motion_prev = p_proc->motion;

    if (b_new_impact)
    {
        p_proc->b_impact_hold     = true;
        p_proc->impact_hold_until = now_ms;   /* start of hold window */
    }
    if (p_proc->b_impact_hold
        && ((now_ms - p_proc->impact_hold_until) >= p_params->impact_hold_ms))
    {
        p_proc->b_impact_hold = false;
    }

    if (p_proc->b_impact_hold)
    {
        cand = IMU_MOTION_SUDDEN_IMPACT;
        conf = IMU_CONF_MEDIUM;
    }
    else if ((IMU_HUMP_ASCENT == hump_phase) || (IMU_HUMP_PEAK == hump_phase))
    {
        cand = IMU_MOTION_CLIMBING_HUMP;
        conf = IMU_CONF_MEDIUM;
    }
    else if (IMU_HUMP_DESCENT == hump_phase)
    {
        cand = IMU_MOTION_DESCENDING_HUMP;
        conf = IMU_CONF_MEDIUM;
    }
    else if (IMU_TURN_LEFT == p_proc->turn_state)
    {
        cand = IMU_MOTION_TURNING_LEFT;
        conf = (IMU_HEADING_GOOD == p_proc->heading_quality)
               ? IMU_CONF_MEDIUM : IMU_CONF_LOW;
    }
    else if (IMU_TURN_RIGHT == p_proc->turn_state)
    {
        cand = IMU_MOTION_TURNING_RIGHT;
        conf = (IMU_HEADING_GOOD == p_proc->heading_quality)
               ? IMU_CONF_MEDIUM : IMU_CONF_LOW;
    }
    else
    {
        b_immediate = false;
        acc_thr = (IMU_MOTION_ACCELERATING == p_proc->motion)
                  ? p_params->accel_exit_g : p_params->accel_enter_g;
        dec_thr = (IMU_MOTION_DECELERATING == p_proc->motion)
                  ? p_params->accel_exit_g : p_params->accel_enter_g;
        vib_thr = fmaxf(p_params->stationary_vib_g,
                        PROC_NOISE_VIB_FACTOR
                        * p_proc->accel_cal.noise_max_std_g);

        if (p_proc->fwd_accel_g > acc_thr)
        {
            cand = IMU_MOTION_ACCELERATING;
            conf = p_proc->b_fwd_from_hint ? IMU_CONF_HIGH : IMU_CONF_MEDIUM;
        }
        else if (p_proc->fwd_accel_g < -dec_thr)
        {
            cand = IMU_MOTION_DECELERATING;
            conf = p_proc->b_fwd_from_hint ? IMU_CONF_HIGH : IMU_CONF_MEDIUM;
        }
        else if (b_hint)
        {
            int32_t speed = p_proc->hint.speed_mm_s;

            cand = ((speed <= p_params->stationary_speed_mm_s)
                    && (speed >= -p_params->stationary_speed_mm_s))
                   ? IMU_MOTION_STATIONARY : IMU_MOTION_STEADY_MOTION;
            conf = IMU_CONF_HIGH;
        }
        else if (sqrtf(fmaxf(p_proc->vib_var, 0.0f)) < vib_thr)
        {
            /* No vibration at all: very probably standing still (motors
             * off). Inferred, therefore LOW confidence. */
            cand = IMU_MOTION_STATIONARY;
            conf = IMU_CONF_LOW;
        }
        else
        {
            cand = IMU_MOTION_STEADY_MOTION;
            conf = IMU_CONF_LOW;
        }
    }

    if (cand == p_proc->motion)
    {
        p_proc->motion_cand = cand;
        p_proc->motion_conf = conf;
    }
    else if (b_immediate || (IMU_MOTION_UNKNOWN == p_proc->motion))
    {
        p_proc->motion      = cand;
        p_proc->motion_cand = cand;
        p_proc->motion_conf = conf;
    }
    else if (cand != p_proc->motion_cand)
    {
        p_proc->motion_cand       = cand;
        p_proc->motion_cand_start = now_ms;
    }
    else if ((now_ms - p_proc->motion_cand_start) >= p_params->class_hold_ms)
    {
        p_proc->motion      = cand;
        p_proc->motion_conf = conf;
    }
    else
    {
        /* candidate not yet stable: keep the current class */
    }

    p_out->motion           = p_proc->motion;
    p_out->motion_conf      = p_proc->motion_conf;
    p_out->b_motion_changed = (p_out->motion != p_out->motion_prev);
}

/* ------------------------------------------------------------------------ */
/*  Encoder hint                                                            */
/* ------------------------------------------------------------------------ */

imu_err_t imu_proc_set_hint(imu_proc_t * p_proc,
                            imu_motion_hint_t const * p_hint)
{
    imu_motion_hint_t prev;

    if ((NULL == p_proc) || (NULL == p_hint))
    {
        return (IMU_ERR_PARAM);
    }

    prev = p_proc->hint;

    if (!p_hint->b_valid)
    {
        p_proc->hint.b_valid      = false;
        p_proc->b_hint_accel_valid = false;
        return (IMU_OK);
    }

    if ((p_hint->speed_mm_s > IMU_CFG_HINT_MAX_SPEED_MM_S)
        || (p_hint->speed_mm_s < -IMU_CFG_HINT_MAX_SPEED_MM_S)
        || (p_hint->b_accel_valid
            && ((p_hint->accel_mm_s2 > IMU_CFG_HINT_MAX_ACCEL_MM_S2)
                || (p_hint->accel_mm_s2 < -IMU_CFG_HINT_MAX_ACCEL_MM_S2))))
    {
        p_proc->hints_rejected++;
        return (IMU_ERR_PARAM);   /* spoofed / corrupted: ignore */
    }

    if (p_hint->b_accel_valid)
    {
        p_proc->hint_accel_g = (float)p_hint->accel_mm_s2
                             / PROC_STANDARD_GRAVITY_MM_S2;
        p_proc->b_hint_accel_valid = true;
    }
    else if (prev.b_valid
             && ((p_hint->timestamp_ms - prev.timestamp_ms)
                 >= PROC_HINT_DIFF_MIN_MS)
             && ((p_hint->timestamp_ms - prev.timestamp_ms)
                 <= PROC_HINT_DIFF_MAX_MS))
    {
        float dv = (float)(p_hint->speed_mm_s - prev.speed_mm_s);
        float dt = (float)(p_hint->timestamp_ms - prev.timestamp_ms) * 0.001f;
        float a  = (dv / dt) / PROC_STANDARD_GRAVITY_MM_S2;

        p_proc->hint_accel_g = p_proc->b_hint_accel_valid
            ? imu_lpf_step(p_proc->hint_accel_g, a, PROC_HINT_DIFF_ALPHA)
            : a;
        p_proc->b_hint_accel_valid = true;
    }
    else
    {
        p_proc->b_hint_accel_valid = false;
    }

    p_proc->hint = *p_hint;
    return (IMU_OK);
}

bool imu_proc_hint_fresh(imu_proc_t const * p_proc, uint32_t now_ms)
{
    return ((NULL != p_proc) && p_proc->hint.b_valid
            && ((now_ms - p_proc->hint.timestamp_ms)
                <= p_proc->p_params->hint_max_age_ms));
}

/* ------------------------------------------------------------------------ */
/*  Names                                                                   */
/* ------------------------------------------------------------------------ */

char const * imu_motion_name(imu_motion_class_t motion)
{
    static char const * const k_names[IMU_MOTION_COUNT] =
    {
        "UNKNOWN", "STATIONARY", "STEADY_MOTION", "ACCELERATING",
        "DECELERATING", "TURNING_LEFT", "TURNING_RIGHT", "CLIMBING_HUMP",
        "DESCENDING_HUMP", "SUDDEN_IMPACT"
    };

    return (((uint32_t)motion < (uint32_t)IMU_MOTION_COUNT)
            ? k_names[motion] : "INVALID");
}

char const * imu_heading_quality_name(imu_heading_quality_t quality)
{
    char const * p_name;

    switch (quality)
    {
        case IMU_HEADING_INVALID:      p_name = "INVALID";      break;
        case IMU_HEADING_UNCALIBRATED: p_name = "UNCALIBRATED"; break;
        case IMU_HEADING_DISTURBED:    p_name = "DISTURBED";    break;
        case IMU_HEADING_GOOD:         p_name = "GOOD";         break;
        default:                       p_name = "INVALID";      break;
    }

    return (p_name);
}
