#include <stdio.h>
#include "retro_joystick.h"
#include "hardware.h"

/* autofire status */
uint8_t autofire_active = 0;

void rj_init(void)
{
    JOY_UP    = 1;
    JOY_DOWN  = 1;
    JOY_LEFT  = 1;
    JOY_RIGHT = 1;
    JOY_FIRE  = 1;

    autofire_active = 0;
}

/* active low Atari / Amiga joystick */
void rj_event(joystick_report_t *j)
{
    JOY_UP    = !j->up;
    JOY_DOWN  = !j->down;
    JOY_LEFT  = !j->left;
    JOY_RIGHT = !j->right;
    JOY_FIRE  = !j->fire;

    autofire_active = j->autofire;
}

