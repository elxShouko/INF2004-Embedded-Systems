/** @file drv_navigation_map.h
 * @brief Member 3 drv navigation map module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#ifndef NAVIGATION_MAP_H
#define NAVIGATION_MAP_H

#include <stdbool.h>
#include "line/navigation_types.h"

#define NAV_ALPHABET_SIZE 26U
#define NAV_ACTION_BYTES 10U /* 26 actions, three bits each. */

typedef struct
{
    uint32_t selected; /* Low 26 bits; wide ingress permits range validation. */
    uint8_t actions[NAV_ACTION_BYTES];
} nav_update_t;

typedef struct
{
    uint8_t       actions[NAV_ACTION_BYTES];
    uint32_t      revision;
} nav_map_t;

/** @brief Read one three-bit action; invalid indices return NONE. */
nav_command_t nav_action_get (const uint8_t * p_actions, uint32_t index);
/** @brief Validate and replace one action without changing adjacent entries. */
bool nav_action_set (uint8_t * p_actions, uint32_t index, nav_command_t action);
/** @brief Parse a text boundary into a packed partial update transaction. */
bool nav_map_parse (const char * p_text, uint8_t * p_actions,
                    uint32_t * p_selected);
/** @brief Convert a NUL-terminated text boundary; preserve output on error. */
bool nav_update_parse (nav_update_t * p_update, const char * p_text);
/** @brief Check selection, actions and unused bits before narrowing. */
bool nav_update_valid (const nav_update_t * p_update);
/** @brief Commit a validated binary partial map transaction. */
bool nav_map_commit (nav_map_t * p_map, const nav_update_t * p_update);

/** @brief Install the user-requested startup letter-to-action mapping. */
void nav_map_init (nav_map_t * p_map);
/** @brief Return the current action for a supported uppercase letter. */
nav_command_t nav_map_lookup (const nav_map_t * p_map, char letter);
/* Exact grammar: MAP A=FORWARD B=LEFT ...; updates commit all or none.
 * Supported actions: FORWARD, LEFT, RIGHT, UTURN, NONE.
 * Owner task must serialize updates/lookups; this is not an ISR API.
 */
/** @brief Validate and commit a complete mapping update transaction. */
bool nav_map_apply (nav_map_t * p_map, const char * p_text);

#endif

/*** End of file. ***/

