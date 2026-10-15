/**
 * @file    imu_hump.c
 * @brief   Buddy 4 hump detector (see imu_hump.h for the model).
 */

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "imu/imu_hump.h"
#include "imu/imu_math.h"

#define HUMP_RANK_EPS            (1.0e-3f)
#define HUMP_LONG_FACTOR         (1.5f)   /* hump longer than 1.5 L       */
#define HUMP_MAX_STEP_MS         (200U)   /* integration step bound       */
#define HUMP_CREST_MAX_WHEELBASES (1.5f)  /* crest->descent distance bound*/

static void hump_to_monitoring(imu_hump_t * p_hump, uint32_t now_ms)
{
    p_hump->phase          = IMU_HUMP_MONITORING;
    p_hump->b_armed        = false;
    p_hump->b_level_timing = false;
    p_hump->b_cand         = false;
    p_hump->b_desc_cand    = false;
    p_hump->b_lvl_cand     = false;
    p_hump->b_has_last_end = true;
    p_hump->last_end_ms    = now_ms;
}

imu_err_t imu_hump_init(imu_hump_t * p_hump, imu_params_t const * p_params)
{
    if ((NULL == p_hump) || (NULL == p_params))
    {
        return (IMU_ERR_PARAM);
    }

    (void)memset(p_hump, 0, sizeof(*p_hump));
    p_hump->p_params = p_params;
    p_hump->phase    = IMU_HUMP_MONITORING;

    return (IMU_OK);
}

void imu_hump_reset_mission(imu_hump_t * p_hump)
{
    if (NULL == p_hump)
    {
        return;
    }

    (void)memset(&p_hump->stats, 0, sizeof(p_hump->stats));
    (void)memset(&p_hump->cur, 0, sizeof(p_hump->cur));
    p_hump->phase          = IMU_HUMP_MONITORING;
    p_hump->b_armed        = false;
    p_hump->b_level_timing = false;
    p_hump->b_cand         = false;
    p_hump->b_desc_cand    = false;
    p_hump->b_lvl_cand     = false;
    p_hump->b_has_last_end = false;
}

void imu_hump_abort(imu_hump_t * p_hump, uint32_t now_ms)
{
    if (NULL == p_hump)
    {
        return;
    }

    if (IMU_HUMP_MONITORING != p_hump->phase)
    {
        p_hump->stats.rejected_count++;
        p_hump->cur.validity = IMU_HUMP_REJ_ABORTED;
    }

    hump_to_monitoring(p_hump, now_ms);
}

void imu_hump_estimate_height(imu_params_t const * p_params,
                              imu_hump_meas_t * p_meas)
{
    imu_height_method_t method;
    float               h = 0.0f;
    imu_confidence_t    conf;

    if ((NULL == p_params) || (NULL == p_meas))
    {
        return;
    }

    method = p_params->height_method;
    if ((IMU_HEIGHT_METHOD_INTEGRATED == method) && !p_meas->b_rise_valid)
    {
        method = IMU_HEIGHT_METHOD_GEOMETRIC;   /* no speed: fall back */
    }

    conf = p_params->b_hump_height_calibrated ? IMU_CONF_MEDIUM
                                              : IMU_CONF_LOW;

    switch (method)
    {
        case IMU_HEIGHT_METHOD_GEOMETRIC:
            h = p_params->wheelbase_mm
              * sinf(fmaxf(p_meas->peak_pitch_deg, 0.0f) * IMU_DEG_TO_RAD);
            if (p_meas->b_length_valid
                && (p_meas->length_est_mm
                    > (HUMP_LONG_FACTOR * p_params->wheelbase_mm)))
            {
                conf = IMU_CONF_LOW;   /* long hump: pitch = slope only */
            }
            break;

        case IMU_HEIGHT_METHOD_INTEGRATED:
            h = p_meas->rise_mm / p_params->imu_pos_frac;
            break;

        default:
            conf = IMU_CONF_NONE;
            break;
    }

    if (IMU_CONF_NONE != conf)
    {
        h = (h * p_params->hump_height_gain) + p_params->hump_height_offset_mm;
        if ((0 == isfinite(h)) || (h < 0.0f))
        {
            h = 0.0f;
        }
        if (p_meas->b_attitude_degraded)
        {
            conf = IMU_CONF_LOW;
        }
    }

    p_meas->height_est_mm = h;
    p_meas->height_conf   = conf;
    p_meas->height_method = method;
}

static void hump_track(imu_hump_t * p_hump, uint32_t now_ms,
                       imu_hump_input_t const * p_in)
{
    imu_hump_meas_t * p_m = &p_hump->cur;
    uint32_t          dt_ms = now_ms - p_hump->last_t_ms;
    float             dt_s;

    if (dt_ms > HUMP_MAX_STEP_MS)
    {
        dt_ms = HUMP_MAX_STEP_MS;
    }
    dt_s = (float)dt_ms * 0.001f;

    if (p_in->vert_accel_g > p_m->peak_vert_up_g)
    {
        p_m->peak_vert_up_g = p_in->vert_accel_g;
    }
    if (p_in->vert_accel_g < p_m->peak_vert_down_g)
    {
        p_m->peak_vert_down_g = p_in->vert_accel_g;
    }
    if (!p_in->b_attitude_reliable)
    {
        p_m->b_attitude_degraded = true;
    }

    if (p_in->b_speed_valid)
    {
        p_hump->dist_mm += fabsf(p_in->speed_mm_s) * dt_s;
        if (IMU_HUMP_DESCENT != p_hump->phase)
        {
            p_hump->rise_mm += p_in->speed_mm_s
                             * sinf(p_in->pitch_deg * IMU_DEG_TO_RAD) * dt_s;
            if (p_hump->rise_mm > p_m->rise_mm)
            {
                p_m->rise_mm = p_hump->rise_mm;
            }
        }
    }
    else
    {
        p_hump->b_rise_ok = false;
    }
}

static void hump_finish(imu_hump_t * p_hump, uint32_t now_ms,
                        imu_hump_validity_t validity, imu_hump_out_t * p_out)
{
    imu_params_t const * p_params = p_hump->p_params;
    imu_hump_meas_t *    p_m      = &p_hump->cur;

    p_m->t_end_ms       = now_ms;
    p_m->total_ms       = now_ms - p_m->t_start_ms;
    p_m->b_rise_valid   = p_hump->b_rise_ok;
    p_m->b_length_valid = p_hump->b_rise_ok;
    p_m->length_est_mm  = p_hump->b_rise_ok ? p_hump->dist_mm : 0.0f;

    if (IMU_HUMP_VALID == validity)
    {
        if (p_m->peak_pitch_deg < p_params->hump_min_peak_deg)
        {
            validity = IMU_HUMP_REJ_LOW_PEAK;
        }
        else if (-p_m->min_pitch_deg < p_params->hump_min_descent_deg)
        {
            validity = IMU_HUMP_REJ_NO_DESCENT;
        }
        else if (p_m->total_ms < p_params->hump_min_event_ms)
        {
            validity = IMU_HUMP_REJ_TOO_SHORT;
        }
        else
        {
            /* complete, credible traversal */
        }
    }

    p_m->validity = validity;
    imu_hump_estimate_height(p_params, p_m);
    p_m->rank_value = ((IMU_RANK_BY_HEIGHT_EST == p_params->rank_metric)
                       && (IMU_CONF_NONE != p_m->height_conf))
                      ? p_m->height_est_mm : p_m->peak_pitch_deg;

    if (IMU_HUMP_VALID == validity)
    {
        p_hump->stats.valid_count++;
        p_m->index = p_hump->stats.valid_count;
        p_hump->stats.last       = *p_m;
        p_hump->stats.b_has_last = true;

        if (!p_hump->stats.b_has_max
            || (p_m->rank_value > (p_hump->stats.max.rank_value
                                   + HUMP_RANK_EPS)))
        {
            p_hump->stats.max       = *p_m;
            p_hump->stats.b_has_max = true;
            p_out->b_new_max        = true;
        }
        p_out->b_completed = true;
    }
    else
    {
        p_m->index = 0U;
        p_hump->stats.rejected_count++;
        p_out->b_rejected = true;
    }

    p_out->meas = *p_m;

    if (IMU_HUMP_VALID == validity)
    {
        hump_to_monitoring(p_hump, now_ms);
    }
    else
    {
        /* A rejected candidate must not block the next real hump: no
         * minimum-gap penalty, and an ongoing level period still counts
         * towards re-arming. */
        bool     b_was_level  = p_hump->b_lvl_cand;
        uint32_t level_start  = p_hump->lvl_cand_start_ms;
        bool     b_had_end    = p_hump->b_has_last_end;
        uint32_t last_end     = p_hump->last_end_ms;

        hump_to_monitoring(p_hump, now_ms);
        p_hump->b_has_last_end = b_had_end;
        p_hump->last_end_ms    = last_end;
        if (b_was_level)
        {
            p_hump->b_level_timing = true;
            p_hump->level_start_ms = level_start;
        }
    }
}

static bool hump_start_allowed(imu_hump_t const * p_hump,
                               imu_hump_input_t const * p_in)
{
    imu_params_t const * p_params = p_hump->p_params;
    bool                 b_ok     = true;

    if (p_in->b_speed_valid && p_params->b_hump_require_moving
        && (fabsf(p_in->speed_mm_s)
            <= (float)p_params->stationary_speed_mm_s))
    {
        b_ok = false;   /* vehicle not moving: lifted / pushed by hand */
    }

    if (p_in->b_hint_accel_valid
        && (fabsf(p_in->hint_accel_g) > p_params->hump_max_hint_accel_g))
    {
        b_ok = false;   /* strong acceleration corrupts the pitch */
    }

    return (b_ok);
}

static void hump_monitoring(imu_hump_t * p_hump, uint32_t now_ms,
                            imu_hump_input_t const * p_in,
                            imu_hump_out_t * p_out)
{
    imu_params_t const * p_params = p_hump->p_params;
    float                pitch    = p_in->pitch_deg;

    /* Re-arm: level for rearm time AND minimum gap since the last hump. */
    if (fabsf(pitch) < p_params->hump_level_band_deg)
    {
        if (!p_hump->b_level_timing)
        {
            p_hump->b_level_timing = true;
            p_hump->level_start_ms = now_ms;
        }
        if (((now_ms - p_hump->level_start_ms)
             >= p_params->hump_rearm_level_ms)
            && (!p_hump->b_has_last_end
                || ((now_ms - p_hump->last_end_ms)
                    >= p_params->hump_min_gap_ms)))
        {
            p_hump->b_armed = true;
        }
    }
    else
    {
        p_hump->b_level_timing = false;
    }

    if (p_hump->b_armed && (pitch > p_params->hump_ascent_enter_deg)
        && hump_start_allowed(p_hump, p_in))
    {
        if (!p_hump->b_cand)
        {
            p_hump->b_cand        = true;
            p_hump->cand_start_ms = now_ms;
            p_hump->cand_peak_deg = pitch;
            p_hump->cand_peak_ms  = now_ms;
        }
        else if (pitch > p_hump->cand_peak_deg)
        {
            /* fast hump: the maximum may occur before confirmation */
            p_hump->cand_peak_deg = pitch;
            p_hump->cand_peak_ms  = now_ms;
        }
        else
        {
            /* keep the candidate maximum */
        }

        if (p_hump->b_cand
            && ((now_ms - p_hump->cand_start_ms)
                >= p_params->hump_ascent_min_ms))
        {
            (void)memset(&p_hump->cur, 0, sizeof(p_hump->cur));
            p_hump->cur.t_start_ms     = p_hump->cand_start_ms;
            p_hump->cur.peak_pitch_deg = p_hump->cand_peak_deg;
            p_hump->cur.t_peak_ms      = p_hump->cand_peak_ms;
            p_hump->cur.validity       = IMU_HUMP_PROVISIONAL;
            p_hump->rise_mm            = 0.0f;
            p_hump->dist_mm            = 0.0f;
            p_hump->b_rise_ok          = p_in->b_speed_valid;
            p_hump->b_armed            = false;
            p_hump->b_cand             = false;
            p_hump->phase              = IMU_HUMP_ASCENT;
            p_hump->last_t_ms          = now_ms;
            hump_track(p_hump, now_ms, p_in);

            p_out->b_detected = true;
            p_out->meas       = p_hump->cur;
        }
    }
    else
    {
        p_hump->b_cand = false;
    }
}

void imu_hump_step(imu_hump_t * p_hump, uint32_t now_ms,
                   imu_hump_input_t const * p_in, imu_hump_out_t * p_out)
{
    imu_params_t const * p_params;
    imu_hump_meas_t *    p_m;
    float                pitch;
    uint32_t             elapsed;

    if ((NULL == p_hump) || (NULL == p_in) || (NULL == p_out))
    {
        return;
    }

    (void)memset(p_out, 0, sizeof(*p_out));
    p_params = p_hump->p_params;
    p_m      = &p_hump->cur;
    pitch    = p_in->pitch_deg;

    if (0 == isfinite(pitch))
    {
        p_out->phase = p_hump->phase;
        return;   /* never feed NaN into the state machine */
    }

    if (IMU_HUMP_MONITORING == p_hump->phase)
    {
        hump_monitoring(p_hump, now_ms, p_in, p_out);
        p_out->phase = p_hump->phase;
        return;
    }

    hump_track(p_hump, now_ms, p_in);
    p_hump->last_t_ms = now_ms;
    elapsed = now_ms - p_m->t_start_ms;

    if (elapsed > p_params->hump_max_event_ms)
    {
        hump_finish(p_hump, now_ms,
                    (IMU_HUMP_DESCENT == p_hump->phase)
                        ? IMU_HUMP_REJ_TOO_LONG : IMU_HUMP_REJ_NO_DESCENT,
                    p_out);
        p_out->phase = p_hump->phase;
        return;
    }

    switch (p_hump->phase)
    {
        case IMU_HUMP_ASCENT:
            if (pitch > p_m->peak_pitch_deg)
            {
                p_m->peak_pitch_deg = pitch;
                p_m->t_peak_ms      = now_ms;
            }
            if (pitch <= (p_m->peak_pitch_deg - p_params->hump_peak_drop_deg))
            {
                p_m->ascent_ms = p_m->t_peak_ms - p_m->t_start_ms;
                p_hump->dist_at_peak_mm = p_hump->dist_mm;
                p_hump->b_lvl_cand      = false;
                p_hump->b_desc_cand     = false;
                p_hump->phase  = IMU_HUMP_PEAK;
                p_out->b_peak  = true;
                p_out->meas    = *p_m;
                imu_hump_estimate_height(p_params, &p_out->meas);
            }
            else if (elapsed > p_params->hump_ascent_max_ms)
            {
                hump_finish(p_hump, now_ms, IMU_HUMP_REJ_SLOPE, p_out);
            }
            else
            {
                /* still climbing */
            }
            break;

        case IMU_HUMP_PEAK:
            if (pitch > p_m->peak_pitch_deg)
            {
                p_m->peak_pitch_deg = pitch;   /* refine: noisy crest */
                p_m->t_peak_ms      = now_ms;
                p_m->ascent_ms      = now_ms - p_m->t_start_ms;
                p_hump->dist_at_peak_mm = p_hump->dist_mm;
            }
            /* Level after the "crest" and then a fresh climb: the previous
             * candidate was not a hump (e.g. acceleration artefact); restart
             * immediately so that a real hump right behind it is not lost. */
            if (fabsf(pitch) < p_params->hump_level_band_deg)
            {
                if (!p_hump->b_lvl_cand)
                {
                    p_hump->b_lvl_cand        = true;
                    p_hump->lvl_cand_start_ms = now_ms;
                }
            }
            else if ((pitch > p_params->hump_ascent_enter_deg)
                     && p_hump->b_lvl_cand
                     && ((now_ms - p_hump->lvl_cand_start_ms)
                         >= p_params->hump_rearm_level_ms))
            {
                hump_finish(p_hump, now_ms, IMU_HUMP_REJ_NO_DESCENT, p_out);
                p_hump->b_armed       = true;
                p_hump->b_cand        = true;
                p_hump->cand_start_ms = now_ms;
                break;
            }
            else if (pitch < -p_params->hump_level_band_deg)
            {
                p_hump->b_lvl_cand = false;   /* nose-down: not level */
            }
            else
            {
                /* between level band and ascent threshold: keep timing */
            }

            /* No nose-down phase after the crest: not a hump (typically
             * the pitch artefact of a forward acceleration, or a step). */
            if ((p_in->b_speed_valid && p_hump->b_rise_ok)
                ? ((p_hump->dist_mm - p_hump->dist_at_peak_mm)
                   > (HUMP_CREST_MAX_WHEELBASES * p_params->wheelbase_mm))
                : ((now_ms - p_m->t_peak_ms) > p_params->hump_crest_max_ms))
            {
                hump_finish(p_hump, now_ms, IMU_HUMP_REJ_NO_DESCENT, p_out);
                break;
            }
            if (pitch < -p_params->hump_descent_enter_deg)
            {
                if (!p_hump->b_desc_cand)
                {
                    p_hump->b_desc_cand        = true;
                    p_hump->desc_cand_start_ms = now_ms;
                    p_m->min_pitch_deg = fminf(p_m->min_pitch_deg, pitch);
                }
                else if ((now_ms - p_hump->desc_cand_start_ms)
                         >= p_params->hump_descent_min_ms)
                {
                    p_hump->b_desc_cand = false;
                    p_hump->b_lvl_cand  = false;
                    p_hump->phase       = IMU_HUMP_DESCENT;
                }
                else
                {
                    /* timing the descent candidate */
                }
                p_m->min_pitch_deg = fminf(p_m->min_pitch_deg, pitch);
            }
            else
            {
                p_hump->b_desc_cand = false;
            }
            break;

        case IMU_HUMP_DESCENT:
            p_m->min_pitch_deg = fminf(p_m->min_pitch_deg, pitch);
            if ((fabsf(pitch) < p_params->hump_level_band_deg)
                || (pitch > p_params->hump_level_band_deg))
            {
                if (!p_hump->b_lvl_cand)
                {
                    p_hump->b_lvl_cand        = true;
                    p_hump->lvl_cand_start_ms = now_ms;
                }
                else if ((now_ms - p_hump->lvl_cand_start_ms)
                         >= p_params->hump_level_min_ms)
                {
                    hump_finish(p_hump, p_hump->lvl_cand_start_ms,
                                IMU_HUMP_VALID, p_out);
                    /* re-arm timing starts now, not at the level start */
                    p_hump->last_end_ms = now_ms;
                }
                else
                {
                    /* timing the level condition */
                }
            }
            else
            {
                p_hump->b_lvl_cand = false;
            }
            break;

        default:
            hump_to_monitoring(p_hump, now_ms);
            break;
    }

    p_out->phase = p_hump->phase;
}

char const * imu_hump_validity_name(imu_hump_validity_t validity)
{
    char const * p_name;

    switch (validity)
    {
        case IMU_HUMP_VALID:          p_name = "VALID";          break;
        case IMU_HUMP_PROVISIONAL:    p_name = "PROVISIONAL";    break;
        case IMU_HUMP_REJ_LOW_PEAK:   p_name = "REJ_LOW_PEAK";   break;
        case IMU_HUMP_REJ_NO_DESCENT: p_name = "REJ_NO_DESCENT"; break;
        case IMU_HUMP_REJ_TOO_SHORT:  p_name = "REJ_TOO_SHORT";  break;
        case IMU_HUMP_REJ_TOO_LONG:   p_name = "REJ_TOO_LONG";   break;
        case IMU_HUMP_REJ_SLOPE:      p_name = "REJ_SLOPE";      break;
        case IMU_HUMP_REJ_ABORTED:    p_name = "REJ_ABORTED";    break;
        default:                      p_name = "UNKNOWN";        break;
    }

    return (p_name);
}
