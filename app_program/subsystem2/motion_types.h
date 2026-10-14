/**
 * @file motion_types.h
 * @brief Buddy 2 internal unit types; not a shared motion-command contract.
 */
#ifndef MOTION_TYPES_H
#define MOTION_TYPES_H

#include <stdint.h>

/* Wheel speed: signed millimetres/second. Distance: signed micrometres.
 * Encoder count: signed decoded transitions, not an assumed CPR/PPR unit.
 * Vehicle-forward is positive; reverse is negative. No raw PWM is exposed.
 */
typedef int32_t motion_types_speed_mm_s_t;
typedef int64_t motion_types_distance_um_t;
typedef int64_t motion_types_encoder_count_t;

/* Unknown is deliberately zero so zero-initialised settings fail closed.
 * NORMAL: positive electrical drive/count corresponds to vehicle-forward.
 * REVERSED: that relationship is inverted. Establish each independently.
 * For motors, electrical-positive is A active with B low.
 * For counts, the electrical reference sequence is defined in M1B, not here.
 */
typedef enum
{
    MOTION_TYPES_POLARITY_UNKNOWN = 0,
    MOTION_TYPES_POLARITY_NORMAL,
    MOTION_TYPES_POLARITY_REVERSED
} motion_types_polarity_t;

#endif /* MOTION_TYPES_H */

/*** end of file ***/

