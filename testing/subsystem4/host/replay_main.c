/**
 * @file    replay_main.c
 * @brief   HOST-ONLY offline replay tool: feeds RAW sensor logs recorded with
 *          the bring-up firmware ('w' key) through the production algorithms
 *          (driver -> calibration -> processing -> hump -> state machine)
 *          so that thresholds can be tuned on the PC with real data.
 *
 *   ./buddy4_replay log.txt [key=value ...]
 *
 * Input lines (other lines are ignored, so a whole terminal capture works):
 *   RAW,t_ms,ax,ay,az,mx,my,mz,ovf        (sensor frame, g and gauss)
 *
 * Optional parameter overrides (subset), e.g.:
 *   wheelbase_mm=150 hump_ascent_enter_deg=2.5 impact_minor_g=1.0
 *   attitude_lpf_hz=6 tilt_threshold_deg=12
 *
 *   ./buddy4_replay --gen > synthetic.txt   writes a synthetic RAW log
 *                                           (stationary, 3 humps, turn).
 *
 * The log is sampled at the cycle rate (50 Hz, one accelerometer sample per
 * cycle), so impact detection results are NOT representative in replay.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_util.h"

#define REPLAY_MAX_ROWS  (200000U)

typedef struct
{
    uint32_t   t_ms;
    imu_vec3_t a;
    imu_vec3_t m;
} replay_row_t;

static replay_row_t * g_rows;
static uint32_t       g_n_rows;
static uint32_t       g_cursor;

static void replay_stimulus(void * p_ctx, double t_s, imu_vec3_t * p_a,
                            imu_vec3_t * p_m)
{
    uint32_t const t_ms = (uint32_t)(t_s * 1000.0);

    (void)p_ctx;
    while (((g_cursor + 1U) < g_n_rows) && (g_rows[g_cursor + 1U].t_ms <= t_ms))
    {
        g_cursor++;
    }
    *p_a = g_rows[g_cursor].a;
    *p_m = g_rows[g_cursor].m;
}

static int replay_override(imu_params_t * p, char const * p_kv)
{
    char         key[64];
    double       v;
    char const * p_eq = strchr(p_kv, '=');

    if ((NULL == p_eq) || ((size_t)(p_eq - p_kv) >= sizeof(key)))
    {
        return (-1);
    }
    (void)memcpy(key, p_kv, (size_t)(p_eq - p_kv));
    key[p_eq - p_kv] = '\0';
    v = atof(p_eq + 1);

#define OVR_F(name) if (0 == strcmp(key, #name)) { p->name = (float)v; return 0; }
#define OVR_U(name) if (0 == strcmp(key, #name)) { p->name = (uint32_t)v; return 0; }
    OVR_F(wheelbase_mm) OVR_F(hump_ascent_enter_deg) OVR_F(hump_min_peak_deg)
    OVR_F(hump_descent_enter_deg) OVR_F(hump_level_band_deg)
    OVR_F(hump_peak_drop_deg) OVR_F(attitude_lpf_hz) OVR_F(heading_lpf_hz)
    OVR_F(tilt_threshold_deg) OVR_F(turn_enter_dps) OVR_F(turn_exit_dps)
    OVR_F(impact_minor_g) OVR_F(impact_critical_g) OVR_F(accel_enter_g)
    OVR_F(accel_exit_g) OVR_F(cal_max_noise_g) OVR_F(hump_height_gain)
    OVR_F(hump_height_offset_mm)
    OVR_U(hump_ascent_min_ms) OVR_U(hump_crest_max_ms) OVR_U(hump_max_event_ms)
    OVR_U(hump_rearm_level_ms) OVR_U(turn_min_duration_ms)
    OVR_U(tilt_min_duration_ms) OVR_U(cal_samples)
#undef OVR_F
#undef OVR_U
    return (-1);
}

static void replay_generate(void)
{
    scenario_t s;
    uint32_t   t;

    scn_init(&s);
    s.vib_noise_g = 0.02;
    scn_add_segment(&s, 3.0, 12.0, 0.25, 0.25, 0.0);
    scn_add_segment(&s, 15.0, 1.5, 0.25, 0.25, -60.0);
    scn_add_segment(&s, 16.5, 5.0, 0.25, 0.25, 0.0);
    scn_add_hump(&s, 0.5, 0.015, 0.10);
    scn_add_hump(&s, 1.5, 0.030, 0.10);
    scn_add_hump(&s, 2.5, 0.020, 0.10);
    printf("# synthetic Buddy 4 RAW log: humps 15/30/20 mm, L=140 mm\n");
    for (t = 0U; t < 22000U; t += 20U)
    {
        imu_vec3_t a;
        imu_vec3_t m;

        scn_stimulus(&s, (double)t * 0.001, &a, &m);
        printf("RAW,%u,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,0\n", (unsigned)t,
               (double)a.x, (double)a.y, (double)a.z, (double)m.x,
               (double)m.y, (double)m.z);
    }
}

int main(int argc, char ** argv)
{
    FILE *       p_f;
    char         line[256];
    int          i;
    imu_status_t st;

    if ((argc >= 2) && (0 == strcmp(argv[1], "--gen")))
    {
        replay_generate();
        return (0);
    }
    if (argc < 2)
    {
        printf("usage: %s log.txt [param=value ...] | --gen\n", argv[0]);
        return (2);
    }

    g_rows = (replay_row_t *)calloc(REPLAY_MAX_ROWS, sizeof(replay_row_t));
    p_f    = fopen(argv[1], "r");
    if ((NULL == g_rows) || (NULL == p_f))
    {
        printf("cannot open %s\n", argv[1]);
        return (2);
    }
    while ((NULL != fgets(line, sizeof(line), p_f))
           && (g_n_rows < REPLAY_MAX_ROWS))
    {
        unsigned t;
        float    ax, ay, az, mx, my, mz;
        char *   p_raw = strstr(line, "RAW,");

        if ((NULL != p_raw)
            && (7 == sscanf(p_raw, "RAW,%u,%f,%f,%f,%f,%f,%f", &t, &ax, &ay,
                            &az, &mx, &my, &mz)))
        {
            replay_row_t * p_r = &g_rows[g_n_rows];

            p_r->t_ms = t;
            p_r->a.x = ax; p_r->a.y = ay; p_r->a.z = az;
            p_r->m.x = mx; p_r->m.y = my; p_r->m.z = mz;
            g_n_rows++;
        }
    }
    (void)fclose(p_f);
    if (g_n_rows < 10U)
    {
        printf("no RAW rows found\n");
        return (2);
    }

    harness_setup();
    for (i = 2; i < argc; i++)
    {
        if (0 != replay_override(&g_h.params, argv[i]))
        {
            printf("unknown parameter: %s\n", argv[i]);
            return (2);
        }
    }
    g_b_verbose = true;                 /* print every event */
    sim_set_time_ms(g_rows[0].t_ms);
    if (IMU_OK != harness_start())
    {
        printf("invalid parameter set\n");
        return (2);
    }
    sim_cfg()->p_fn  = replay_stimulus;
    sim_cfg()->p_ctx = NULL;
    harness_run_ms(g_rows[g_n_rows - 1U].t_ms - g_rows[0].t_ms);

    st = harness_status();
    printf("\nREPLAY SUMMARY: %u rows, state=%s, humps valid=%u rejected=%u\n",
           (unsigned)g_n_rows, imu_state_name(st.state),
           (unsigned)st.hump.valid_count, (unsigned)st.hump.rejected_count);
    if (st.hump.b_has_max)
    {
        printf("  max hump #%u: peak %.2f deg, est %.1f mm (conf %u)\n",
               (unsigned)st.hump.max.index,
               (double)st.hump.max.peak_pitch_deg,
               (double)st.hump.max.height_est_mm,
               (unsigned)st.hump.max.height_conf);
    }
    printf("  calibration: g_ref=%.4f noise=%.4f g mount_tilt=%.2f deg\n",
           (double)st.cal_g_ref, (double)st.cal_noise_g,
           (double)st.cal_mount_tilt_deg);
    free(g_rows);
    return (0);
}
