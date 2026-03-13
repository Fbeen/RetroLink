#include <stdio.h>
#include "retro_joystick.h"
#include "hardware.h"

/* active low Atari / Amiga joystick */
void rj_event(joystick_report_t *j)
{
    JOY_UP    = !j->up;
    JOY_DOWN  = !j->down;
    JOY_LEFT  = !j->left;
    JOY_RIGHT = !j->right;
    JOY_FIRE  = !j->fire;
}

