/**
 * @file s1_sim.h
 * @brief Buddy 1: simulated telemetry producer for stand-alone demos.
 *
 * Stands in for Buddies 2 to 5 until their subsystems send real frames. It
 * pushes one frame every 100 ms through s1_net_telemetry_send(), exactly as a
 * real producer would.
 *
 * NOTE: Set S1_SIMULATE_TELEMETRY to 0 in the integrated robot build.
 */

#ifndef S1_SIM_H
#define S1_SIM_H

#include <stdbool.h>

#include "../app.h"

/** Set to 0 to remove the simulator from the build (integrated robot). */
#ifndef S1_SIMULATE_TELEMETRY
#define S1_SIMULATE_TELEMETRY 1u
#endif

/** @brief Create the cyclic handler of the simulator. */
ER s1_sim_init (void);

/** @brief Start the simulator task and its cyclic handler. */
ER s1_sim_start (void);

/** @brief Pause or resume frame production (used by the unit tests). */
void s1_sim_set_enabled (bool b_enabled);

#endif /* S1_SIM_H */

/*** end of file ***/
