#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "hardware.h"

typedef struct
{
    uint16_t interval;      // ms, 0 = always on
    uint16_t duration;      // ms, 0 = endless
    uint16_t counter;       // increased by ISR

    uint8_t state;
    uint8_t active;

} led_t;

void led_on();
void led_off();
void led_activate(uint16_t interval, uint16_t duration);
void led_tick();

#endif