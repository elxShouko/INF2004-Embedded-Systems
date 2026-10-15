/** @file drv_navigation_map.h
 * @brief Member 3 drv navigation map module.
 * @note See BARR_COMPLIANCE.md for compatibility exceptions under review.
 */

#ifndef NAVIGATION_MAP_H
#define NAVIGATION_MAP_H

#include <stdbool.h>
#include "line/navigation_types.h"

#define NAV_ALPHABET_SIZE 26U

typedef struct
{
    nav_command_t actions[NAV_ALPHABET_SIZE];
    uint32_t      revision;
} nav_map_t;

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

