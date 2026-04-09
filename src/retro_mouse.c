#include "retro_mouse.h"
#include "hardware.h"
#include "config.h"

volatile uint8_t mouse_buttons = 0;

static __xdata int16_t  mx = 0;
static __xdata int16_t  my = 0;

static uint8_t __xdata x_state = 0;
static uint8_t __xdata y_state = 0;

// quadrature states: 00 -> 01 -> 11 -> 10
// bit0 = A, bit1 = B
static const __code uint8_t quad[4] =
{
    0b00,
    0b01,
    0b11,
    0b10
};

/*
 * initializes default mouse DB9 signal states
 */
void rm_init(void)
{
    // reset movement accumulators
    mx = 0;
    my = 0;

    // start quadrature states
    x_state = 0;
    y_state = 0;

    // stable encoder condition
    MX1 = 0;
    MX2 = 0;
    MY1 = 0;
    MY2 = 0;
}

/*
 * Generates one quadrature step for the emulated mouse on both X and Y axes.
 *
 * The function consumes pending movement (mx, my) and advances or reverses
 * the quadrature state machines (x_state, y_state) one step at a time,
 * depending on the direction of movement. Each call outputs exactly one
 * step per axis if movement is pending.
 *
 * The current quadrature states are translated into signal levels using
 * a lookup table (quad[]) and written to the corresponding hardware pins.
 *
 * It supports both Atari ST and Amiga mouse pinouts. When swap mode is
 * enabled, X2 and Y2 signals are remapped to match the Amiga wiring.
 *
 * This function is typically called from a timer interrupt to generate
 * properly timed quadrature signals for smooth mouse movement.
 */
void rm_nextStep(void)
{
    // ---- X axis ----
    if (mx > 0)
    {
        x_state = (x_state - 1) & 3;
        mx--;
    }
    else if (mx < 0)
    {
        x_state = (x_state + 1) & 3;
        mx++;
    }
    // ---- Y axis ----
    if (my > 0)
    {
        y_state = (y_state + 1) & 3;
        my--;
    }
    else if (my < 0)
    {
        y_state = (y_state - 1) & 3;
        my++;
    }

    uint8_t xs = quad[x_state];
    uint8_t ys = quad[y_state];

    /*
    On the atari:   On the Amiga:
    pin 1 X2        pin 1 Y2
    pin 2 X1        pin 2 X1
    pin 3 Y1        pin 3 Y1
    pin 4 Y2        pin 4 X2

    So X2 and Y2 must be swapped
    */
    if(g_config.mouse_swap_mode) {
        /* Amiga mouse */

        // ---- X pins ----
        MX1 = (xs & 1) ? 1 : 0;
        MY2 = (xs & 2) ? 1 : 0;

        // ---- Y pins ----
        MY1 = (ys & 1) ? 1 : 0;
        MX2 = (ys & 2) ? 1 : 0;
    } else {
        /* ST mouse */

        // ---- X pins ----
        MX1 = (xs & 1) ? 1 : 0;
        MX2 = (xs & 2) ? 1 : 0;

        // ---- Y pins ----
        MY1 = (ys & 1) ? 1 : 0;
        MY2 = (ys & 2) ? 1 : 0;
    }
}

/* 
 * calculates x and y mouse movement counters mx and my which will be handled by the ISR to create quadrature signals
 * sets mouse buttons DB9 signal states, active low
 */
void rm_event(mouse_report_t *m)
{
    mx += m->dx;
    my += m->dy;

    mouse_buttons = m->buttons;

    uint8_t left  = mouse_buttons & 1;
    uint8_t right = mouse_buttons & 2;

    if(g_config.mouse_swap_buttons)
    {
        uint8_t t = left;
        left = right;
        right = t;
    }

    MBTN_LEFT  = !left;
    MBTN_RIGHT = !right;
}