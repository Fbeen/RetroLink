#include <stdio.h>
#include "CH559.h"
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

__xdata config_t g_config;

#define CONFIG_FLASH_ADDR 0xF000
#define CONFIG_MAGIC      0x524C
#define CONFIG_VERSION    105

#ifndef ROM_CMD_PROG
#define ROM_CMD_PROG  0x9A
#endif

#ifndef ROM_CMD_ERASE
#define ROM_CMD_ERASE 0xA6
#endif

#ifndef bDATA_WE
#define bDATA_WE      0x04
#endif

static void flash_data_enable(void)
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    GLOBAL_CFG |= bDATA_WE;
    SAFE_MOD = 0x00;
}

static void flash_data_disable(void)
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    GLOBAL_CFG &= ~bDATA_WE;
    SAFE_MOD = 0x00;
}

void flash_erase_sector(uint16_t addr)
{
    flash_data_enable();

    ROM_ADDR = addr;
    ROM_CTRL = ROM_CMD_ERASE;

    flash_data_disable();
}

void flash_write_word(uint16_t addr, uint16_t value)
{
    flash_data_enable();

    ROM_ADDR = addr;
    ROM_DATA = value;
    ROM_CTRL = ROM_CMD_PROG;

    flash_data_disable();
}

void config_default(void)
{
    g_config.magic = CONFIG_MAGIC;
    g_config.version = CONFIG_VERSION;

    g_config.mouse_speed = 2;
    g_config.mouse_left_button = 1;
    g_config.mouse_right_button = 2;

    g_config.joy_autofire_speed = 0;

    /* --------------------------------------------------
       Default joystick mapping for:
       IDLE      7F 7F 8C 7F 7F 0F 00 C0
       LEFT      00 7F 8C 7F 7F 0F 00 C0
       RIGHT     FF 7F 8C 7F 7F 0F 00 C0
       UP        7F 00 8C 7F 7F 0F 00 C0
       DOWN      7F FF 8C 7F 7F 0F 00 C0
       FIRE      7F 7F 8C 7F 7F 2F 00 C0
       AUTOFIRE  7F 7F 8C 7F 7F 1F 00 C0
    -------------------------------------------------- */

    /* UP: byte 1, 00-3F active */
    g_config.joy_up.offset = 1;
    g_config.joy_up.mask = 0;
    g_config.joy_up.threshold = 0x40;
    g_config.joy_up.type = CTRL_INPUT_AXIS;
    g_config.joy_up.axis_dir = AXIS_NEG;

    /* DOWN: byte 1, C0-FF active */
    g_config.joy_down.offset = 1;
    g_config.joy_down.mask = 0;
    g_config.joy_down.threshold = 0xBF;
    g_config.joy_down.type = CTRL_INPUT_AXIS;
    g_config.joy_down.axis_dir = AXIS_POS;

    /* LEFT: byte 0, 00-3F active */
    g_config.joy_left.offset = 0;
    g_config.joy_left.mask = 0;
    g_config.joy_left.threshold = 0x40;
    g_config.joy_left.type = CTRL_INPUT_AXIS;
    g_config.joy_left.axis_dir = AXIS_NEG;

    /* RIGHT: byte 0, C0-FF active */
    g_config.joy_right.offset = 0;
    g_config.joy_right.mask = 0;
    g_config.joy_right.threshold = 0xBF;
    g_config.joy_right.type = CTRL_INPUT_AXIS;
    g_config.joy_right.axis_dir = AXIS_POS;

    /* FIRE: byte 5 bit 5 */
    g_config.joy_fire.offset = 5;
    g_config.joy_fire.mask = 0x20;
    g_config.joy_fire.threshold = 0x2F;
    g_config.joy_fire.type = CTRL_INPUT_BUTTON;
    g_config.joy_fire.axis_dir = 0;

    /* AUTOFIRE: byte 5 bit 4 */
    g_config.joy_autofire.offset = 5;
    g_config.joy_autofire.mask = 0x10;
    g_config.joy_autofire.threshold = 0x1F;
    g_config.joy_autofire.type = CTRL_INPUT_BUTTON;
    g_config.joy_autofire.axis_dir = 0;
}

bool config_load(void)
{
    uint8_t i;
    __code uint8_t *flash = (__code uint8_t *)CONFIG_FLASH_ADDR;
    uint8_t *dst = (uint8_t *)&g_config;

    for(i = 0; i < sizeof(config_t); i++)
        dst[i] = flash[i];

    if(g_config.magic != CONFIG_MAGIC ||
       g_config.version != CONFIG_VERSION)
    {
        config_default();
        return false;
    }

    if(g_config.mouse_speed < 1 || g_config.mouse_speed > 5)
        g_config.mouse_speed = 2;

    return true;
}

void config_save(void)
{
    uint16_t i;
    uint8_t *src = (uint8_t *)&g_config;

    flash_erase_sector(CONFIG_FLASH_ADDR);

    for(i = 0; i < sizeof(config_t); i += 2)
    {
        uint16_t w = src[i] | ((uint16_t)src[i + 1] << 8);
        flash_write_word(CONFIG_FLASH_ADDR + i, w);
    }
}