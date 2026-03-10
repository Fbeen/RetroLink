#ifndef INPUT_MOUSE_H
#define INPUT_MOUSE_H

#include <stdint.h>
#include "hid_mouse.h"

//extern volatile int16_t mx;
//extern volatile int16_t my;

extern volatile uint8_t mouse_buttons;

void rm_init(void);
void rm_nextStep(void);
void rm_event(mouse_report_t *m);

#endif