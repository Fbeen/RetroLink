#ifndef __HID_MOUSE_H__
#define __HID_MOUSE_H__

#include <stdint.h>

/* HID parser structures */

typedef struct _HID_Device_Button{
    uint8_t Usage_min;
    uint8_t Usage_max;
    uint16_t Logical_min;
    uint16_t Logical_max;
    uint8_t Size_bit;
    uint8_t Size_reserved;
} BUTTON;

typedef struct _HID_Device_XY{
    uint16_t Logical_min;
    uint16_t Logical_max;
    uint8_t Size_bit;
    uint8_t Size_reserved;
} XY;

typedef struct _HID_Device_Wheel{
    uint16_t Logical_min;
    uint16_t Logical_max;
    uint8_t Size_bit;
    uint8_t Size_reserved;
} WHEEL;

typedef struct _HID_Device_Mouse{
    uint16_t X;
    uint16_t Y;
    uint8_t Left;
    uint8_t Right;
    uint8_t Middle;
    int8_t Wheel;

    BUTTON Button;
    XY X_Y;
    WHEEL Wheel_;
} MOUSE;

/* mouse report structure used as layer between hid_mouse and retro_mouse */
typedef struct
{
    int8_t dx;
    int8_t dy;
    int8_t wheel;

    uint8_t buttons;

} mouse_report_t;

void hid_mouse_parse(uint8_t *RxBuffer);

#endif


