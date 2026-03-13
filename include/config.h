#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "controller_learn.h"

/*
 * WARNING:
 * The structure must have a even size in bytes
 * Reason: We store this in FLASH with two bytes at a time.
*/
typedef struct
{
    uint16_t magic;
    uint16_t version;

    uint8_t mouse_speed;

    uint8_t mouse_left_button;
    uint8_t mouse_right_button;

    uint8_t joy_autofire_speed;

    /* joystick learning mappings */

    control_map_t joy_up;
    control_map_t joy_down;
    control_map_t joy_left;
    control_map_t joy_right;
    control_map_t joy_fire;
    control_map_t joy_autofire;

} config_t;

extern config_t g_config;

void config_default(void);
bool config_load(void);
void config_save(void);
void flash_erase_sector(uint16_t addr);
void flash_write_byte(uint16_t addr, uint8_t value);

#endif// This are the joystick configurations which can be different any brand and device
