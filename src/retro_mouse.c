#include "retro_mouse.h"
#include "hardware.h"

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

void rm_init(void)
{
    // port setup
    PORT_CFG = 0b00101101;

    /* first all outputs safe to HIGH */
    P2 = 0xFF;

    /* force bit5 LOW */
    P2 &= ~(1 << 5);

    /* and then configure them to output */
    P2_DIR = 0b01111111;

    // reset movement accumulators
    mx = 0;
    my = 0;

    // start quadrature states
    x_state = 0;
    y_state = 0;

    // stabiele encoder toestand
    MX1 = 0;
    MX2 = 0;
    MY1 = 0;
    MY2 = 0;
}

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
// #include <stdio.h>
// printf("mx=%d my=%d\r\n", mx, my);

    uint8_t xs = quad[x_state];
    uint8_t ys = quad[y_state];

    // ---- X pins ----
    MX1 = (xs & 1) ? 1 : 0;
    MX2 = (xs & 2) ? 1 : 0;

    // ---- Y pins ----
    MY1 = (ys & 1) ? 1 : 0;
    MY2 = (ys & 2) ? 1 : 0;
}

void rm_event(mouse_report_t *m)
{
    mx += m->dx;
    my += m->dy;

    mouse_buttons = m->buttons;

    /* active low buttons */
    MBTN_LEFT  = !(mouse_buttons & 1);
    MBTN_RIGHT = !(mouse_buttons & 2);
}
