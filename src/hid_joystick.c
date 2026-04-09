#include "hid_joystick.h"
#include "retro_joystick.h"
#include "config.h"
#include "util.h"

/*
 * Check if a mapped control is active based on the current HID report.
 * Supports both button (bitmask or exact match) and axis-based inputs.
 */
uint8_t control_active(control_map_t *map, uint8_t *report)
{
    uint8_t v = report[map->offset];

    if(CTRL_IS_BUTTON(map))
    {
        /* mask==0 => exacte byte compare */
        if(map->mask == 0)
            return (v == map->threshold) ? 1 : 0;

        /* gewone bitmask knop */
        return ((v & map->mask) == map->mask) ? 1 : 0;
    }

    /* axis */
    if(CTRL_AXIS_DIR(map) == AXIS_NEG)
    {
        if(v < map->threshold)
            return 1;
    }
    else
    {
        if(v > map->threshold)
            return 1;
    }

    return 0;
}

/*
 * Parse a joystick HID report and convert it into a normalized
 * joystick state using the configured control mappings.
 */
void parseJoystickData(uint8_t *report)
{
    joystick_report_t j;

    j.up       = control_active(&g_config.map[CTRL_MAP_UP], report);
    j.down     = control_active(&g_config.map[CTRL_MAP_DOWN], report);
    j.left     = control_active(&g_config.map[CTRL_MAP_LEFT], report);
    j.right    = control_active(&g_config.map[CTRL_MAP_RIGHT], report);
    j.fire     = control_active(&g_config.map[CTRL_MAP_FIRE], report);
    j.autofire = control_active(&g_config.map[CTRL_MAP_AUTOFIRE], report);

    rj_event(&j);
}