#include "hid_joystick.h"
#include "retro_joystick.h"
#include "config.h"
#include "util.h"

uint8_t control_active(control_map_t *map, uint8_t *report)
{
    uint8_t v = report[map->offset];

    if(map->type == CTRL_INPUT_BUTTON)
    {
        /* mask==0 => exacte byte compare */
        if(map->mask == 0)
            return (v == map->threshold) ? 1 : 0;

        /* gewone bitmask knop */
        return ((v & map->mask) == map->mask) ? 1 : 0;
    }

    /* axis */
    if(map->axis_dir == AXIS_NEG)
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

void parseJoystickData(uint8_t *report)
{
    joystick_report_t j;

    j.up       = control_active(&g_config.joy_up, report);
    j.down     = control_active(&g_config.joy_down, report);
    j.left     = control_active(&g_config.joy_left, report);
    j.right    = control_active(&g_config.joy_right, report);
    j.fire     = control_active(&g_config.joy_fire, report);
    j.autofire = control_active(&g_config.joy_autofire, report);

    rj_event(&j);
}