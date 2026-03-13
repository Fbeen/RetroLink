#include "hid_joystick.h"
#include "retro_joystick.h"
#include "config.h"
#include "util.h"

/* joystick debug */
uint8_t control_active(control_map_t *map, uint8_t *report)
{
    uint8_t v = report[map->offset];

    if(map->type == CTRL_INPUT_BUTTON)
    {
        return (v & map->mask) ? 1 : 0;
    }

    /* axis */

    if(map->axis_dir == AXIS_NEG)
    {
        if(v < map->threshold - 20)
            return 1;
    }
    else
    {
        if(v > map->threshold + 20)
            return 1;
    }

    return 0;
}

/* joystick parser */
void parseJoystickData(uint8_t  *report)
{
    joystick_report_t j;

    j.up    = control_active(&g_config.joy_up, report);
    j.down  = control_active(&g_config.joy_down, report);
    j.left  = control_active(&g_config.joy_left, report);
    j.right = control_active(&g_config.joy_right, report);
    j.fire  = control_active(&g_config.joy_fire, report);

    rj_event(&j);
}