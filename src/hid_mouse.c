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

static const __code uint16_t mouse_speed_table[] = {64,96,128,192,256};

/*
 * Parses a USB HID mouse report and converts it into movement data
 * suitable for the retro system (e.g. Atari ST).
 *
 * The function reads relative X/Y movement and button states from the
 * HID report buffer. It applies a configurable speed scaling factor
 * using fixed-point arithmetic to allow smooth and precise motion.
 *
 * Movement is accumulated in higher precision (accx/accy) so that
 * fractional motion is preserved between calls. The integer part is
 * extracted and sent as quadrature steps, while the fractional part
 * is retained for the next update.
 *
 * The resulting movement (dx, dy) is inverted if required and forwarded
 * together with the button state to the retro mouse handler.
 */
void hid_mouse_parse(uint8_t *RxBuffer)
{
    static __xdata int32_t accx = 0;
    static __xdata int32_t accy = 0;

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