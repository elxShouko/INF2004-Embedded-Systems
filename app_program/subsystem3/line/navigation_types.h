/** @file navigation_types.h
 * @brief Member 3 navigation types module.
 * @note See BARR_COMPLIANCE.md for compatibility exceptions under review.
 */

#ifndef NAVIGATION_TYPES_H
#define NAVIGATION_TYPES_H

#include <stdint.h>

/* Proposed Member 3 boundary types; no queue or controller implementation. */
typedef enum
{
    LINE_STATE_CALIBRATING = 0,
    LINE_STATE_TRACKING,
    LINE_STATE_BARCODE_DECODING,
    LINE_STATE_AWAITING_MANOEUVRE,
    LINE_STATE_RECOVERY,
    LINE_STATE_FAULT_REPORTED
} line_state_t;

typedef enum
{
    NAV_COMMAND_NONE = 0,
    NAV_COMMAND_LEFT,
    NAV_COMMAND_RIGHT,
    NAV_COMMAND_STRAIGHT,
    NAV_COMMAND_U_TURN
} nav_command_t;

typedef enum
{
    NAV_EVENT_LINE_STATUS = 0,
    NAV_EVENT_JUNCTION,
    NAV_EVENT_BARCODE_VALID,
    NAV_EVENT_BARCODE_INVALID,
    NAV_EVENT_LINE_LOST,
    NAV_EVENT_LINE_FOUND,
    NAV_EVENT_FAULT
} nav_event_kind_t;

typedef enum
{
    LINE_MODE_PAUSED = 0,
    LINE_MODE_TRACKING,
    LINE_MODE_BYPASS,
    LINE_MODE_RECOVERY
} line_mode_t;

/* Position/correction units and junction classifications remain deferred. */
typedef struct
{
    uint64_t         timestamp_us;
    nav_event_kind_t kind;
    nav_command_t    command;
    line_state_t     state;
    uint8_t          black_mask;
    char             barcode_character;
} nav_event_t;

#endif

/*** End of file. ***/

