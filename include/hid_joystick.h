#ifndef HID_JOYSTICK_H
#define HID_JOYSTICK_H

#include <stdint.h>
#include "config.h"
/*
typedef struct
{
    uint8_t idVendorL;
    uint8_t idVendorH;
    uint8_t idProductL;
    uint8_t idProductH;
} vendor_product_id_t;
*/

/* joystick report structure used as layer between hid_joystick and retro_joystick */
typedef struct
{
    uint8_t up;
    uint8_t down;
    uint8_t left;
    uint8_t right;
    uint8_t fire;
    uint8_t autofire;
} joystick_report_t;

uint8_t control_active(control_map_t *map, uint8_t *report);
void parseJoystickData(uint8_t *report);

#endif