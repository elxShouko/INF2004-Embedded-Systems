/**
 * @file    bringup_main.c
 * @brief   Buddy 4 stand-alone HARDWARE BRING-UP / CALIBRATION LOGGER.
 *
 * Separate executable (target buddy4_imu_bringup) with its own main(): it
 * does NOT replace or conflict with the team firmware's src/main.c and does
 * NOT use uT-Kernel. It runs the exact same production modules
 * (driver + calibration + processing + hump + terrain state machine) in a
 * bare-metal 50 Hz loop so that the IMU can be wired, verified, calibrated
 * and its thresholds tuned before RTOS integration.
 *
 * Output (USB CDC serial, 115200 8N1 is irrelevant for USB):
 *   "EVT,..." lines for every terrain event
 *   "CSV,..." lines (when logging is on) with one row per cycle:
 *     t_ms,state,motion,pitch,roll,tilt,heading,hq,rate,fwd_g,vert_g,
 *     vib_g,|B|,hump_phase,humps,max_peak_deg,max_h_mm
 *   "RAW,..." lines (raw mode) with sensor-frame accel (g) and mag (gauss)
 *
 * Single-key commands:
 *   l  toggle CSV log            w  toggle RAW sensor log
 *   m  mission start (reset)     c  clear collision fault
 *   b  begin mag calibration     e  end mag calibration (prints result)
 *   k  recalibrate (stationary)  r  subsystem reset
 *   s  print status              h  help
 *
 * Motors are never driven by this program.
 */

#include <stdio.h>

#include "pico/stdlib.h"

#include "imu/imu_events.h"
#include "imu/imu_hal.h"
#include "imu/imu_terrain.h"

#define BRINGUP_PERIOD_MS   (IMU_CFG_SAMPLE_PERIOD_MS)

static imu_terrain_t g_sys;
static bool          g_b_csv = false;
static bool          g_b_raw = false;

static imu_sink_result_t bringup_send(void * p_ctx, imu_event_t const * p_e)
{
    (void)p_ctx;
    printf("EVT,%lu,%u,%s,sev=%u,state=%s,valid=%u",
           (unsigned long)p_e->timestamp_ms, (unsigned)p_e->seq,
           imu_event_type_name((imu_event_type_t)p_e->type),
           (unsigned)p_e->severity,
           imu_state_name((imu_state_t)p_e->state), (unsigned)p_e->b_valid);

    switch (p_e->type)
    {
        case IMU_EVT_HUMP_DETECTED:
        case IMU_EVT_HUMP_PEAK:
        case IMU_EVT_HUMP_COMPLETED:
        case IMU_EVT_HUMP_REJECTED:
            printf(",idx=%lu,peak_cdeg=%d,min_cdeg=%d,h_mm=%d,conf=%u,"
                   "validity=%u,dur=%u,new_max=%u,max_cdeg=%d",
                   (unsigned long)p_e->data.hump.index,
                   (int)p_e->data.hump.peak_pitch_cdeg,
                   (int)p_e->data.hump.min_pitch_cdeg,
                   (int)p_e->data.hump.height_est_mm,
                   (unsigned)p_e->data.hump.height_conf,
                   (unsigned)p_e->data.hump.validity,
                   (unsigned)p_e->data.hump.duration_ms,
                   (unsigned)p_e->data.hump.b_new_max,
                   (int)p_e->data.hump.max_peak_pitch_cdeg);
            break;
        case IMU_EVT_TILT:
            printf(",phase=%u,pitch=%d,roll=%d,tilt=%d",
                   (unsigned)p_e->data.tilt.phase,
                   (int)p_e->data.tilt.pitch_cdeg,
                   (int)p_e->data.tilt.roll_cdeg,
                   (int)p_e->data.tilt.tilt_cdeg);
            break;
        case IMU_EVT_TURNING:
            printf(",dir=%u,phase=%u,rate_cdps=%d,hdg=%d,delta=%ld,dur=%lu",
                   (unsigned)p_e->data.turn.direction,
                   (unsigned)p_e->data.turn.phase,
                   (int)p_e->data.turn.rate_cdps,
                   (int)p_e->data.turn.heading_cdeg,
                   (long)p_e->data.turn.delta_cdeg,
                   (unsigned long)p_e->data.turn.duration_ms);
            break;
        case IMU_EVT_IMPACT:
            printf(",peak_mg=%d,horiz_mg=%d,vert_mg=%d,dur=%u,supp=%lu",
                   (int)p_e->data.impact.peak_mg,
                   (int)p_e->data.impact.horiz_mg,
                   (int)p_e->data.impact.vert_mg,
                   (unsigned)p_e->data.impact.duration_ms,
                   (unsigned long)p_e->data.impact.suppressed);
            break;
        case IMU_EVT_MOTION:
            printf(",motion=%s,conf=%u",
                   imu_motion_name((imu_motion_class_t)p_e->data.motion.motion),
                   (unsigned)p_e->data.motion.confidence);
            break;
        default:
            printf(",status=%u,fault=%s,err=-%u,attempt=%lu",
                   (unsigned)p_e->data.fault.status,
                   imu_fault_name((imu_fault_t)p_e->data.fault.fault),
                   (unsigned)p_e->data.fault.last_error,
                   (unsigned long)p_e->data.fault.attempt);
            break;
    }
    printf("\n");

    return (IMU_SINK_OK);
}

static void bringup_status(void)
{
    imu_status_t st;

    (void)imu_terrain_get_status(&g_sys, &st);
    printf("STATUS state=%s fault=%s err=%d cal=%u g_ref=%.4f noise=%.4f "
           "mount_tilt=%.2f\n", imu_state_name(st.state),
           imu_fault_name(st.fault), (int)st.last_error,
           (unsigned)st.b_calibrated, (double)st.cal_g_ref,
           (double)st.cal_noise_g, (double)st.cal_mount_tilt_deg);
    printf("  pitch=%.2f roll=%.2f tilt=%.2f hdg=%.1f (%s) rate=%.1f "
           "motion=%s\n", (double)st.pitch_deg, (double)st.roll_deg,
           (double)st.tilt_deg, (double)st.heading_deg,
           imu_heading_quality_name(st.heading_quality),
           (double)st.turn_rate_dps, imu_motion_name(st.motion));
    printf("  humps=%lu rejected=%lu max_peak=%.2f deg max_h=%.1f mm\n",
           (unsigned long)st.hump.valid_count,
           (unsigned long)st.hump.rejected_count,
           (double)st.hump.max.peak_pitch_deg,
           (double)st.hump.max.height_est_mm);
    printf("  mag_cal valid=%u hard_iron=(%.4f,%.4f,%.4f) "
           "soft=(%.4f,%.4f,%.4f) ref=%.4f\n", (unsigned)st.b_mag_cal_valid,
           (double)st.mag_cal.hard_iron_gauss.x,
           (double)st.mag_cal.hard_iron_gauss.y,
           (double)st.mag_cal.hard_iron_gauss.z,
           (double)st.mag_cal.soft_iron_scale.x,
           (double)st.mag_cal.soft_iron_scale.y,
           (double)st.mag_cal.soft_iron_scale.z,
           (double)st.mag_cal.ref_field_gauss);
    printf("  i2c transfers=%lu failed=%lu retries=%lu nacks=%lu "
           "timeouts=%lu fifo_ovr=%lu recov=%lu\n",
           (unsigned long)st.i2c.transfers,
           (unsigned long)st.i2c.failed_transfers,
           (unsigned long)st.i2c.retries, (unsigned long)st.i2c.nacks,
           (unsigned long)st.i2c.timeouts,
           (unsigned long)st.i2c.fifo_overruns,
           (unsigned long)st.i2c.recoveries);
}

static void bringup_help(void)
{
    printf("Buddy 4 bring-up: l=CSV w=RAW m=mission c=clear-fault "
           "b/e=mag-cal begin/end k=recal r=reset s=status h=help\n");
}

static void bringup_keys(void)
{
    int ch = getchar_timeout_us(0U);

    while (PICO_ERROR_TIMEOUT != ch)
    {
        switch (ch)
        {
            case 'l': g_b_csv = !g_b_csv;                                 break;
            case 'w': g_b_raw = !g_b_raw;                                 break;
            case 'm': (void)imu_terrain_command(&g_sys, IMU_CMD_MISSION_START); break;
            case 'c': (void)imu_terrain_command(&g_sys, IMU_CMD_CLEAR_FAULT);   break;
            case 'b': (void)imu_terrain_command(&g_sys, IMU_CMD_MAG_CAL_BEGIN); break;
            case 'e': (void)imu_terrain_command(&g_sys, IMU_CMD_MAG_CAL_END);   break;
            case 'k': (void)imu_terrain_command(&g_sys, IMU_CMD_RECALIBRATE);   break;
            case 'r': (void)imu_terrain_command(&g_sys, IMU_CMD_RESET);         break;
            case 's': bringup_status();                                   break;
            case 'h': bringup_help();                                     break;
            default:                                                      break;
        }
        ch = getchar_timeout_us(0U);
    }
}

static void bringup_log(void)
{
    imu_status_t const * p = &g_sys.status;

    if (g_b_csv)
    {
        printf("CSV,%lu,%u,%u,%.3f,%.3f,%.3f,%.2f,%u,%.2f,%.4f,%.4f,%.4f,"
               "%.4f,%u,%lu,%.3f,%.2f\n",
               (unsigned long)p->timestamp_ms, (unsigned)p->state,
               (unsigned)p->motion, (double)p->pitch_deg,
               (double)p->roll_deg, (double)p->tilt_deg,
               (double)p->heading_deg, (unsigned)p->heading_quality,
               (double)p->turn_rate_dps, (double)p->fwd_accel_g,
               (double)p->vert_accel_g, (double)p->vibration_g,
               (double)p->mag_field_gauss, (unsigned)p->hump_phase,
               (unsigned long)p->hump.valid_count,
               (double)p->hump.max.peak_pitch_deg,
               (double)p->hump.max.height_est_mm);
    }

    if (g_b_raw && g_sys.dev.b_ready)
    {
        imu_vec3_t a;
        imu_vec3_t m;
        imu_raw3_t mr;
        bool       b_ovf = false;

        /* Last accelerometer sample of the cycle + one magnetometer read. */
        lsm303_accel_to_g(&g_sys.dev, &g_sys.last_raw, &a);
        if (IMU_OK == lsm303_read_mag(&g_sys.dev, &mr, &b_ovf))
        {
            lsm303_mag_to_gauss(&g_sys.dev, &mr, &m);
            printf("RAW,%lu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%u\n",
                   (unsigned long)imu_hal_time_ms(), (double)a.x,
                   (double)a.y, (double)a.z, (double)m.x, (double)m.y,
                   (double)m.z, (unsigned)b_ovf);
        }
    }
}

int main(void)
{
    imu_event_sink_t sink;
    absolute_time_t  next;

    stdio_init_all();
    sleep_ms(2000);          /* bare-metal only: time to open the terminal */

    sink.p_send   = bringup_send;
    sink.p_notify = NULL;
    sink.p_ctx    = NULL;

    printf("\nBuddy 4 IMU bring-up (LSM303DLHC on I2C0 GP%u/GP%u)\n",
           (unsigned)IMU_I2C_SDA_GPIO, (unsigned)IMU_I2C_SCL_GPIO);
    printf("Keep the vehicle STILL and LEVEL for 3 s (calibration).\n");
    bringup_help();

    if (IMU_OK != imu_terrain_init(&g_sys, NULL, &sink))
    {
        printf("FATAL: invalid parameters in imu_config.h\n");
        for (;;)
        {
            tight_loop_contents();
        }
    }

    next = get_absolute_time();
    for (;;)
    {
        next = delayed_by_ms(next, BRINGUP_PERIOD_MS);
        bringup_keys();
        imu_terrain_step(&g_sys, imu_hal_time_ms());
        bringup_log();
        sleep_until(next);
    }

    return (0);
}
