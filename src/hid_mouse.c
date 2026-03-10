#include "hid_mouse.h"
#include "retro_mouse.h"
#include "config.h"
#include "util.h"

/*
speedtable (fixed point ×256)
64  = 0.25 x
96  = 0.375 x
128 = 0.5 x
192 = 0.75 x
256 = 1.0 x
*/

static const uint16_t mouse_speed_table[] = {64,96,128,192,256};

void hid_mouse_parse(uint8_t *RxBuffer)
{
    static int32_t accx = 0;
    static int32_t accy = 0;

    mouse_report_t m;

    int8_t dx = (int8_t)RxBuffer[1];
    int8_t dy = (int8_t)RxBuffer[2];

    uint16_t speed = mouse_speed_table[g_config.mouse_speed - 1];

    // schaal USB beweging
    accx += (int16_t)dx * speed;
    accy += (int16_t)dy * speed;

    // integer deel naar ST
    m.dx = -(accx >> 8);
    m.dy = -(accy >> 8);

    // fractie bewaren
    accx &= 0xFF;
    accy &= 0xFF;

    m.buttons = RxBuffer[0];

    rm_event(&m);
}