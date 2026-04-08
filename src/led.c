#include "CH559.h"
#include "util.h"
#include "hardware.h"
#include "led.h"

/* time interval in milliseconds */
#define REPEAT_MS       100

static led_t led = { 0, 0, 0, 0 };

/* turn the led on */
void led_on()
{
    led_activate(0, 0);
}

/* turn the led off */
void led_off()
{
    led.interval = 0;
    led.duration = 0;
    led.counter = 0;
    led.active = 0;
    led.state = 0;

    LED_STATUS = 0;
}

/* 
 * activates the led with a given interval and duration
 *
 * interval: for flashing set microseconds or zero for continuously
 * duration: the time before the LED is turned off or zero to leave the LED on
 */
void led_activate(uint16_t interval, uint16_t duration)
{
    led.interval = interval / REPEAT_MS;
    led.duration = duration / REPEAT_MS;
    led.counter = 0;
    led.active = 1;
    led.state = 1;

    LED_STATUS = 1;
}

/* 
 * called 10 times per second by ISR
 *
 * switches on or off LEDs to let them flash
 * switches off LEDs when time is up
 */
void led_tick()
{
    if(led.active)
    {
        led.counter ++;

        if(led.interval > 0 && led.counter % led.interval == 0)
        {
            if(led.state) {
                led.state = 0;
                LED_STATUS = 0;
            } else {
                led.state = 1;
                LED_STATUS = 1;
            }
        }

        if(led.duration > 0 && led.counter >= led.duration)
        {
            led.interval = 0;
            led.duration = 0;
            led.active = 0;
            led.state = 0;
            LED_STATUS = 0;
        }
    }
}

