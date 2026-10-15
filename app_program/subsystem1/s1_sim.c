/**
 * @file s1_sim.c
 * @brief Buddy 1: simulated telemetry producer for stand-alone demos.
 *
 * A cyclic handler wakes the task every 100 ms. The kernel keeps the period
 * exact, so the simulator does not drift (a delay in the task would not).
 */

#include "s1_sim.h"

#include "s1_net.h"
#include "subsystem1.h"

#define S1_SIM_PERIOD_MS 100
#define S1_SIM_MOTOR_SPEED 120
#define S1_SIM_ENCODER_STEP 5u
#define S1_SIM_LINE_STATUS_CENTRE 0x02u
#define S1_SIM_OBSTACLE_X10 455 /* 45.5 cm */

typedef struct
{
    ID       task_id;
    ID       cyclic_id;
    bool     b_enabled;
    uint32_t encoder_count;
} s1_sim_module_t;

static s1_sim_module_t g_s1_sim = {0};

static void s1_sim_task (INT stacd, void * p_exinf);
static void s1_sim_cyclic_handler (void * p_exinf);

/* ---------------------------------------------------------------------- */
/* Public functions                                                       */
/* ---------------------------------------------------------------------- */

/* Create the cyclic handler, stopped. */
ER
s1_sim_init (void)
{
    T_CCYC cyclic_info = {0};
    ER     result      = E_OK;

    g_s1_sim.b_enabled = true;
    if (0u != S1_SIMULATE_TELEMETRY)
    {
        cyclic_info.cycatr = TA_HLNG;
        cyclic_info.cychdr = s1_sim_cyclic_handler;
        cyclic_info.cyctim = S1_SIM_PERIOD_MS;
        cyclic_info.cycphs = S1_SIM_PERIOD_MS;
        g_s1_sim.cyclic_id = tk_cre_cyc(&cyclic_info);
        if (g_s1_sim.cyclic_id < E_OK)
        {
            result = g_s1_sim.cyclic_id;
        }
    }
    return result;
}

/* Start the task, then the cyclic handler that wakes it. */
ER
s1_sim_start (void)
{
    ER result = E_OK;

    if (0u != S1_SIMULATE_TELEMETRY)
    {
        g_s1_sim.task_id =
            app_start_task(s1_sim_task, S1_PRI_SIM, 0, "s1_sim_task");
        if (g_s1_sim.task_id < E_OK)
        {
            result = g_s1_sim.task_id;
        }
        else
        {
            result = tk_sta_cyc(g_s1_sim.cyclic_id);
        }
    }
    return result;
}

/* Pause or resume frame production. */
void
s1_sim_set_enabled (bool b_enabled)
{
    g_s1_sim.b_enabled = b_enabled;
}

/* ---------------------------------------------------------------------- */
/* Private functions                                                      */
/* ---------------------------------------------------------------------- */

/* Build one frame per wake-up and send it, like a real producer. */
static void
s1_sim_task (INT stacd, void * p_exinf)
{
    s1_codec_frame_t frame = {0};
    SYSTIM           time  = {0};

    (void) stacd;   /* Cast is safe: discards an unused task parameter. */
    (void) p_exinf; /* Cast is safe: discards an unused task parameter. */

    for (;;)
    {
        tk_slp_tsk(TMO_FEVR);
        if (g_s1_sim.b_enabled)
        {
            tk_get_otm(&time);
            g_s1_sim.encoder_count += S1_SIM_ENCODER_STEP;
            frame.timestamp_ms          = time.lo;
            frame.robot_state           = S1_ROBOT_STATE_OPERATIONAL;
            frame.motor_speed_left      = S1_SIM_MOTOR_SPEED;
            frame.motor_speed_right     = S1_SIM_MOTOR_SPEED;
            frame.encoder_count_left    = g_s1_sim.encoder_count;
            frame.encoder_count_right   = g_s1_sim.encoder_count;
            frame.line_status           = S1_SIM_LINE_STATUS_CENTRE;
            frame.hump_peak_x10         = 0;
            frame.obstacle_distance_x10 = S1_SIM_OBSTACLE_X10;
            s1_net_telemetry_send(&frame);
        }
    }
}

/* Wake the task. A handler may only use calls that never wait. */
static void
s1_sim_cyclic_handler (void * p_exinf)
{
    (void) p_exinf; /* Cast is safe: discards an unused handler parameter. */
    tk_wup_tsk(g_s1_sim.task_id);
}

/*** end of file ***/
