#ifndef INPUT_JOYSTICK_H
#define INPUT_JOYSTICK_H

#include <stdint.h>
#include "hid_joystick.h"

extern uint8_t autofire_active;

void rj_init(void);
void rj_event(joystick_report_t *j);

#endif