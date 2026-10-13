#ifndef NAVIGATION_MAP_H
#define NAVIGATION_MAP_H

#include <stdbool.h>
#include "line/navigation_types.h"

#define NAV_ALPHABET_SIZE 26U

typedef struct
{
    nav_command_t actions[NAV_ALPHABET_SIZE];
    uint32_t revision;
} nav_map_t;

void nav_map_init(nav_map_t *p_map);
nav_command_t nav_map_lookup(const nav_map_t *p_map, char letter);
/* Exact grammar: MAP A=FORWARD B=LEFT ...; updates commit all or none.
 * Supported actions: FORWARD, LEFT, RIGHT, UTURN, NONE.
 * Owner task must serialize updates/lookups; this is not an ISR API.
 */
bool nav_map_apply(nav_map_t *p_map, const char *p_text);

#endif
