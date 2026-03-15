#include <stdio.h>
#include "CH559.h"
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

__xdata config_t            g_config;

#define CONFIG_FLASH_ADDR  0xF000
#define CONFIG_MAGIC       0x52
#define CONFIG_VERSION     2

/* Macro that calculates the address of one joystick profile */
#define PROFILE_ADDR(i) (PROFILE_FLASH_ADDR + (i * sizeof(joystick_profile_t)))

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
    g_config.mouse_swap_mode = 0;
    g_config.mouse_swap_buttons = 0;
    g_config.joy_autofire_speed = 0;

    /* leave joystick button mappings empty */
}

bool config_load(void)
{
    uint8_t i;
    __code uint8_t *flash = (__code uint8_t *)CONFIG_FLASH_ADDR;
    __xdata uint8_t *dst = (__xdata uint8_t *)&g_config;

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
    uint8_t i;
    uint8_t *src = (uint8_t *)&g_config;

    flash_erase_sector(CONFIG_FLASH_ADDR);

    for(i = 0; i < sizeof(config_t); i += 2)
    {
        flash_write_word(CONFIG_FLASH_ADDR + i, src[i] | ((uint16_t)src[i + 1] << 8));
    }
}
