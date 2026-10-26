/** @file drv_navigation_map.c
 * @brief Member 3 drv navigation map module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

#include "drv_navigation_map.h"

#include <stddef.h>
#include <string.h>

/**
 * @brief Start with every letter disabled until a runtime map is received.
 */
void nav_map_init (nav_map_t * p_map)
{
    if (p_map != NULL)
    {
        memset(p_map, 0, sizeof(*p_map));
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

    return nav_action_get(p_map->actions, (uint32_t)(letter - 'A'));
}

/** @brief Read one action from a byte-defined, three-bit-per-letter array. */
nav_command_t nav_action_get (const uint8_t * p_actions, uint32_t index)
{
    uint32_t value = 0U;

    if ((NULL != p_actions) && (index < NAV_ALPHABET_SIZE))
    {
        for (uint32_t offset = 0U; offset < 3U; ++offset)
        {
            const uint32_t bit = index * 3U + offset;
            value |= (((uint32_t)p_actions[bit / 8U] >> (bit % 8U)) & 1U)
                     << offset;
        }
    }

    return (value <= (uint32_t)NAV_COMMAND_U_TURN)
               ? (nav_command_t)value : NAV_COMMAND_NONE;
}

/** @brief Validate before modifying exactly the selected action's bits. */
bool nav_action_set (uint8_t * p_actions, uint32_t index, nav_command_t action)
{
    if ((NULL == p_actions) || (index >= NAV_ALPHABET_SIZE) ||
        ((uint32_t)action > (uint32_t)NAV_COMMAND_U_TURN))
    {
        return false;
    }

    for (uint32_t offset = 0U; offset < 3U; ++offset)
    {
        const uint32_t bit = index * 3U + offset;
        const uint32_t mask = 1U << (bit % 8U);
        p_actions[bit / 8U] = (uint8_t)(
            ((uint32_t)p_actions[bit / 8U] & ~mask) |
            ((((uint32_t)action >> offset) & 1U) << (bit % 8U)));
    }

    return true;
}

/** @brief Parse into private storage and preserve outputs on rejection. */
bool nav_map_parse (const char * p_text, uint8_t * p_actions,
                    uint32_t * p_selected)
{
    if ((NULL == p_text) || (NULL == p_actions) || (NULL == p_selected) ||
        (strncmp(p_text, "MAP ", 4U) != 0))
    {
        return false;
    }

    uint8_t candidate[NAV_ACTION_BYTES] = {0};
    uint32_t seen = 0U;
    const char * p_cursor = p_text + 4U;

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
        const uint32_t index = (uint32_t)(*p_cursor - 'A');
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
        const size_t length = (size_t)(p_cursor - p_value);
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
        nav_action_set(candidate, index, action);
        seen |= (1UL << index);
    }
    if (0U == seen)
    {
        return false;
    }
    memcpy(p_actions, candidate, sizeof(candidate));
    *p_selected = seen;
    return true;
}

/** @brief Validate selected actions, unselected entries and padding. */
bool nav_update_valid (const nav_update_t * p_update)
{
    if ((NULL == p_update) || (0U == p_update->selected) ||
        ((p_update->selected >> NAV_ALPHABET_SIZE) != 0U) ||
        ((p_update->actions[NAV_ACTION_BYTES - 1U] & 0xc0U) != 0U))
    {
        return false;
    }
    for (uint32_t index = 0U; index < NAV_ALPHABET_SIZE; ++index)
    {
        uint32_t value = 0U;
        for (uint32_t offset = 0U; offset < 3U; ++offset)
        {
            const uint32_t bit = index * 3U + offset;
            value |= (((uint32_t)p_update->actions[bit / 8U] >>
                       (bit % 8U)) & 1U) << offset;
        }
        if ((value > (uint32_t)NAV_COMMAND_U_TURN) ||
            (((p_update->selected & (1UL << index)) == 0U) && (value != 0U)))
        {
            return false;
        }
    }
    return true;
}

/** @brief Convert complete text input without publishing a partial update. */
bool nav_update_parse (nav_update_t * p_update, const char * p_text)
{
    nav_update_t candidate = {0};
    if ((NULL == p_update) ||
        !nav_map_parse(p_text, candidate.actions, &candidate.selected))
    {
        return false;
    }
    *p_update = candidate;
    return true;
}

/** @brief Commit a binary partial map, preserving pending manoeuvre actions. */
bool nav_map_commit (nav_map_t * p_map, const nav_update_t * p_update)
{
    if ((NULL == p_map) || (UINT32_MAX == p_map->revision) ||
        !nav_update_valid(p_update))
    {
        return false;
    }
    nav_map_t candidate = *p_map;
    for (uint32_t index = 0U; index < NAV_ALPHABET_SIZE; ++index)
    {
        if ((p_update->selected & (1UL << index)) != 0U)
        {
            nav_action_set(candidate.actions, index,
                           nav_action_get(p_update->actions, index));
        }
    }
    ++candidate.revision;
    *p_map = candidate;
    return true;
}

/** @brief Keep the standalone text entry point over the binary transaction. */
bool nav_map_apply (nav_map_t * p_map, const char * p_text)
{
    nav_update_t update;
    return nav_update_parse(&update, p_text) && nav_map_commit(p_map, &update);
}

/** @brief Replace the map with exactly one letter for each movement. */
bool nav_map_install (nav_map_t * p_map, const nav_update_t * p_update)
{
    if ((NULL == p_map) || (UINT32_MAX == p_map->revision) ||
        !nav_update_valid(p_update))
    {
        return false;
    }
    uint32_t seen = 0U;
    uint32_t count = 0U;
    for (uint32_t index = 0U; index < NAV_ALPHABET_SIZE; ++index)
    {
        if ((p_update->selected & (1UL << index)) != 0U)
        {
            const nav_command_t action = nav_action_get(p_update->actions,
                                                        index);
            const uint32_t bit = 1UL << (uint32_t)action;
            if ((NAV_COMMAND_NONE == action) || ((seen & bit) != 0U))
            {
                return false;
            }
            seen |= bit;
            ++count;
        }
    }
    if ((4U != count) || (0x1eU != seen))
    {
        return false;
    }
    memcpy(p_map->actions, p_update->actions, sizeof(p_map->actions));
    ++p_map->revision;
    return true;
}

/*** End of file. ***/

