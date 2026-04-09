#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

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

/* input type */
#define CTRL_INPUT_BUTTON  0
#define CTRL_INPUT_AXIS    1
/* input axis */
#define AXIS_NEG 0
#define AXIS_POS 1

/* flag masks */
#define CTRL_FLAG_TYPE 0x01
#define CTRL_FLAG_AXIS 0x02

/* helpers */
#define CTRL_SET_AXIS(map, dir) ((map)->flags = CTRL_FLAG_TYPE | ((dir) << 1))
#define CTRL_SET_BUTTON(map) ((map)->flags = CTRL_INPUT_BUTTON)
#define CTRL_IS_AXIS(map) ((map)->flags & CTRL_FLAG_TYPE)
#define CTRL_IS_BUTTON(map) (!((map)->flags & CTRL_FLAG_TYPE))
#define CTRL_AXIS_DIR(map) (((map)->flags >> 1) & 1)

typedef struct
{
    uint8_t offset;
    uint8_t mask;
    uint8_t threshold;
    uint8_t flags; // bit0-1: input type, bit2: axis_dir
} control_map_t;

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
    uint8_t mouse_swap_mode;
    uint8_t mouse_swap_buttons;
    uint8_t joy_autofire_speed;

    uint16_t vid; // vendor id of stored joystick configuration
    uint16_t pid; // product id of stored joystick configuration
    control_map_t map[CTRL_MAP_COUNT]; // joystick button mapping
} config_t;

extern config_t             g_config;

void config_default(void);
bool config_load(void);
void config_save(void);
void flash_erase_sector(uint16_t addr);
void flash_write_byte(uint16_t addr, uint8_t value);
uint16_t get_vid(void);
uint16_t get_pid(void);

#endif// This are the joystick configurations which can be different any brand and device
