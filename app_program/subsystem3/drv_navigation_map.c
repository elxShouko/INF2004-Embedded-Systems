#include "line/navigation_map.h"

#include <stddef.h>
#include <string.h>

void nav_map_init(nav_map_t *p_map)
{
    if (p_map != NULL)
    {
        memset(p_map, 0, sizeof(*p_map));
        p_map->actions[0] = NAV_COMMAND_STRAIGHT;
        p_map->actions[1] = NAV_COMMAND_LEFT;
        p_map->actions[2] = NAV_COMMAND_RIGHT;
        p_map->actions[3] = NAV_COMMAND_U_TURN;
    }
}

nav_command_t nav_map_lookup(const nav_map_t *p_map, char letter)
{
    if ((p_map == NULL) || (letter < 'A') || (letter > 'Z'))
    {
        return NAV_COMMAND_NONE;
    }
    return p_map->actions[(unsigned int)(letter - 'A')];
}

bool nav_map_apply(nav_map_t *p_map, const char *p_text)
{
    if ((p_map == NULL) || (p_text == NULL) ||
        (strncmp(p_text, "MAP ", 4U) != 0) || (p_map->revision == UINT32_MAX))
    {
        return false;
    }
    nav_map_t candidate = *p_map;
    uint32_t seen = 0U;
    const char *p_cursor = p_text + 4U;
    while (*p_cursor != '\0')
    {
        while (*p_cursor == ' ')
        {
            ++p_cursor;
        }
        if (*p_cursor == '\0')
        {
            break;
        }
        if ((*p_cursor < 'A') || (*p_cursor > 'Z'))
        {
            return false;
        }
        const unsigned int index = (unsigned int)(*p_cursor - 'A');
        ++p_cursor;
        if ((*p_cursor != '=') || ((seen & (1UL << index)) != 0U))
        {
            return false;
        }
        ++p_cursor;
        const char *p_value = p_cursor;
        while ((*p_cursor != '\0') && (*p_cursor != ' '))
        {
            ++p_cursor;
        }
        const size_t length = (size_t)(p_cursor - p_value);
        nav_command_t action;
        if ((length == 7U) && (memcmp(p_value, "FORWARD", length) == 0))
        {
            action = NAV_COMMAND_STRAIGHT;
        }
        else if ((length == 4U) && (memcmp(p_value, "LEFT", length) == 0))
        {
            action = NAV_COMMAND_LEFT;
        }
        else if ((length == 5U) && (memcmp(p_value, "RIGHT", length) == 0))
        {
            action = NAV_COMMAND_RIGHT;
        }
        else if ((length == 5U) && (memcmp(p_value, "UTURN", length) == 0))
        {
            action = NAV_COMMAND_U_TURN;
        }
        else if ((length == 4U) && (memcmp(p_value, "NONE", length) == 0))
        {
            action = NAV_COMMAND_NONE;
        }
        else
        {
            return false;
        }
        candidate.actions[index] = action;
        seen |= (1UL << index);
    }
    if (seen == 0U)
    {
        return false;
    }
    ++candidate.revision;
    *p_map = candidate;
    return true;
}
