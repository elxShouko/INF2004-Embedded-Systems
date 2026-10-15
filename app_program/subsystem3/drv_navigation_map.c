/** @file drv_navigation_map.c
 * @brief Member 3 drv navigation map module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#include "drv_navigation_map.h"

#include <stddef.h>
#include <string.h>

/**
 * @brief Install the user-requested startup letter-to-action mapping.
 */
void nav_map_init (nav_map_t * p_map)
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

/**
 * @brief Return the current action for a supported uppercase letter.
 * @note Bounds checks limit the uppercase index to 0 through 25 before
 * conversion.
 */
nav_command_t nav_map_lookup (const nav_map_t * p_map, char letter)
{
    if ((NULL == p_map) || (letter < 'A') || (letter > 'Z'))
    {
        return NAV_COMMAND_NONE;
    }

    return p_map->actions[(unsigned int)(letter - 'A')];
}

/**
 * @brief Validate and commit a complete mapping update transaction.
 * @note The cursor only advances from the value start, so its nonnegative
 * difference fits size_t for a valid input string.
 */
bool nav_map_apply (nav_map_t * p_map, const char * p_text)
{
    if ((NULL == p_map) || (NULL == p_text) ||
        (strncmp(p_text, "MAP ", 4U) != 0) || (UINT32_MAX == p_map->revision))
    {
        return false;
    }

    nav_map_t    candidate = *p_map;
    uint32_t     seen      = 0U;
    const char * p_cursor  = p_text + 4U;

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
        const char * p_value = p_cursor;

        while ((*p_cursor != '\0') && (*p_cursor != ' '))
        {
            ++p_cursor;
        }

        const size_t  length = (size_t)(p_cursor - p_value);
        nav_command_t action;

        if ((7U == length) && (memcmp(p_value, "FORWARD", length) == 0))
        {
            action = NAV_COMMAND_STRAIGHT;
        }
        else if ((4U == length) && (memcmp(p_value, "LEFT", length) == 0))
        {
            action = NAV_COMMAND_LEFT;
        }
        else if ((5U == length) && (memcmp(p_value, "RIGHT", length) == 0))
        {
            action = NAV_COMMAND_RIGHT;
        }
        else if ((5U == length) && (memcmp(p_value, "UTURN", length) == 0))
        {
            action = NAV_COMMAND_U_TURN;
        }
        else if ((4U == length) && (memcmp(p_value, "NONE", length) == 0))
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

    if (0U == seen)
    {
        return false;
    }

    ++candidate.revision;
    *p_map = candidate;

    return true;
}

/*** End of file. ***/

