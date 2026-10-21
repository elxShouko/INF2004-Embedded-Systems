/** @file member3_navigation.h
 * @brief Member 3 member3 navigation module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#ifndef MEMBER3_NAVIGATION_H
#define MEMBER3_NAVIGATION_H

#include "../app.h"
#include "member3_data.h"

/* Created by S3 init BEFORE app_main starts any subsystem tasks. */
IMPORT volatile ID mbf_s3_input;
IMPORT volatile ID mbf_s3_output;
IMPORT volatile ID mtx_s3_status;
IMPORT volatile ID tid_s3_navigation;

/* Task-context APIs. All processing/configuration is serialized by S3 task.
 * E_OK here means queued; use S3_OUTPUT_REPLY for actual validation result.
 * Controller should drain output frequently and poll status for latched fault.
 * Line events are changes, not motion keepalives. A controller must validate
 * calibrated/line_valid, fault, mode/state AND the age of raw.timestamp_us
 * before refreshing Buddy 2 motion. A fresh raw sample alone does not make
 * a retained filtered observation suitable for steering. See the proposed
 * handoff in docs/buddy3/production-timing-and-handoff.md. vehicle_rt.c is
 * the current controller caller; these providers never actuate motors.
 */
/** @brief Copy one request into the bounded owner-task input queue. */
ER s3_submit (const s3_input_t * p_input, TMO timeout);
/** @brief Decode one compact owner output; errors preserve caller storage. */
ER s3_receive (s3_output_t * p_output, TMO timeout);
/** @brief Poll once for a copied status; task context only, no retry loop.
 * E_OK: coherent copy (may be old); E_TMOUT: mutex busy, no wait performed;
 * E_PAR: NULL output; E_OBJ: objects unavailable. Other kernel errors pass
 * through, including unlock errors. On EVERY error output remains unchanged.
 * Treat failure as unavailable feedback; never renew motion from that buffer.
 * No kernel object wait is requested; target execution latency still requires
 * measurement under SMP/preemption. Reads never modify acquisition timestamps.
 */
ER s3_get_status (s3_status_t * p_status);
/** @brief Create navigation objects before any subsystem task starts. */
ER s3_navigation_init (void);
/** @brief Create and start the single navigation owner task. */
ER s3_navigation_start (void);
/** @brief Serialize requests and sensor processing on the owner task. */
void s3_navigation_task (INT stacd, void * p_exinf);

#endif

/*** End of file. ***/

