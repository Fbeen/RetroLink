#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "controller_learn.h"

/* 6 buttons per joystick */
#define CTRL_MAP_COUNT     6

/* max joysticks mappings in FLASH */
#define MAX_JOYSTICK_PROFILES 8

/* array index of each button */
#define CTRL_MAP_UP        0
#define CTRL_MAP_DOWN      1
#define CTRL_MAP_LEFT      2
#define CTRL_MAP_RIGHT     3
#define CTRL_MAP_FIRE      4
#define CTRL_MAP_AUTOFIRE  5

/*
 * WARNING:
 * The structure must have a even size in bytes
 * Reason: We store this in FLASH with two bytes at a time.
*/
typedef struct
{
    uint8_t magic;
    uint8_t version;
    uint8_t mouse_speed;
    uint8_t mouse_swap_buttons;
    uint8_t joy_autofire_speed;
    uint8_t reserved;

    control_map_t map[CTRL_MAP_COUNT]; // joystick button mapping
} config_t;

extern config_t             g_config;

void config_default(void);
bool config_load(void);
void config_save(void);
void flash_erase_sector(uint16_t addr);
void flash_write_byte(uint16_t addr, uint8_t value);

#endif// This are the joystick configurations which can be different any brand and device
