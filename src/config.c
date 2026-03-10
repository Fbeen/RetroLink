#include <stdio.h>
#include "CH559.h"
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

config_t g_config;

#define CONFIG_FLASH_ADDR 0xF000 // Must be even!
#define CONFIG_MAGIC      0x524C
#define CONFIG_VERSION    102

/* fallback als jouw CH559.h deze defines niet heeft */
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

    /* DataFlash erase is 1KB block erase */
    ROM_ADDR = addr;          /* 0xF000 is sector-aligned */
    ROM_CTRL = ROM_CMD_ERASE; /* 0xA6 */

    flash_data_disable();
}

void flash_write_word(uint16_t addr, uint16_t value)
{
    flash_data_enable();

    /* Address must be even */
    ROM_ADDR = addr;
    ROM_DATA = value;
    ROM_CTRL = ROM_CMD_PROG;  /* 0x9A */

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
}

bool config_load(void)
{
    uint8_t i;
    __code uint8_t *flash = (__code uint8_t *)CONFIG_FLASH_ADDR;
    uint8_t *dst = (uint8_t *)&g_config;

    for(i = 0; i < sizeof(config_t); i++)
    {
        dst[i] = flash[i];
    }

    if(g_config.magic != CONFIG_MAGIC ||
       g_config.version != CONFIG_VERSION)
    {
        config_default();
        return false;
    }

    if(g_config.mouse_speed < 1 || g_config.mouse_speed > 5)
    {
        g_config.mouse_speed = 2;
    }

    return true;
}

void config_save(void)
{
    uint16_t i;
    uint8_t *src = (uint8_t *)&g_config;

    /* safety: struct size must be even for 16-bit writes */
    flash_erase_sector(CONFIG_FLASH_ADDR);

    for(i = 0; i < sizeof(config_t); i += 2)
    {
        uint16_t w = src[i] | ((uint16_t)src[i + 1] << 8);
        flash_write_word(CONFIG_FLASH_ADDR + i, w);
    }
}