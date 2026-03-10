#include <stdio.h>
#include "CH559.h"
#include "util.h"
#include "console.h"
#include "config.h"

typedef enum
{
    MENU_MAIN,
    MENU_MOUSE,
    MENU_MOUSE_SPEED,
    MENU_JOYSTICK
} console_state_t;

static console_state_t menu_state = MENU_MAIN;

void console_start(void)
{
    console_show_header();
    console_show_main_menu();
}

static void console_show_header(void)
{
    puts("");
    puts("  ____      _             _     _       _");
    puts(" |  _ \\ ___| |_ _ __ ___ | |   (_)_ __ | | __");
    puts(" | |_) / _ \\ __| '__/ _ \\| |   | | '_ \\| |/ /");
    puts(" |  _ <  __/ |_| | | (_) | |___| | | | |   <");
    puts(" |_| \\_\\___|\\__|_|  \\___/|_____|_|_| |_|_|\\_\\");
    puts("");
    puts(" RetroLink v1.02");
    puts(" USB Input Adapter");
    puts(" ST / Amiga / C64");
    puts("");
}

void console_show_main_menu(void)
{
    puts("\nMain Menu");
    puts("--------------------------------");
    puts("1. Mouse Settings");
    puts("2. Joystick Settings");
    puts("");

    menu_state = MENU_MAIN;
}

static void console_show_mouse_menu(void)
{
    puts("\nMouse Menu");
    puts("--------------------------------");
    printf("1. Set Mouse Speed (%d)", g_config.mouse_speed);
    puts("2. Map Mouse Buttons");
    puts("0. Back");
    puts("");

    menu_state = MENU_MOUSE;
}

static void console_show_mouse_speed_menu(void)
{
    puts("\nMouse Speed");
    puts("--------------------------------");
    puts("Current speed:");
    UART0Send('0' + g_config.mouse_speed);
    puts("");

    puts("1 - Very Slow");
    puts("2 - Slow");
    puts("3 - Normal");
    puts("4 - Fast");
    puts("5 - Turbo");
    puts("0 - Back");

    menu_state = MENU_MOUSE_SPEED;
}

static void console_show_joystick_menu(void)
{
//    printf("\n");
//    printf("Joystick Menu\n");
//    printf("--------------------------------\n");
//    printf("1. Learn Buttons\n");
//    printf("2. Set Autofire Speed\n");
    // printf("3. Show Mapping\n");
//    printf("0. Back\n");

    menu_state = MENU_JOYSTICK;
}

void console_task(void)
{
    if(UART0Available())
    {
        char c = UART0Receive();

        switch(menu_state)
        {
            case MENU_MOUSE:
                switch(c)
                {
                    case '1':
                        console_show_mouse_speed_menu();
                        break;

                    case '2':
                        printf("\nMap Mouse Buttons Menu\n");
                        break;

                    default:
                        console_show_main_menu();
                }
                break;

            case MENU_MOUSE_SPEED:
                if(c >= '1' && c <= '5')
                {
                    g_config.mouse_speed = c - '0';
                    config_save();

                    printf("\nMouse speed set to %d\n", g_config.mouse_speed);

                    console_show_mouse_menu();
                } else if(c == '0') {
                    console_show_mouse_menu();
                }
                break;

            case MENU_JOYSTICK:
                switch(c)
                {
                    case '1':
                        printf("\nLearn Buttons Menu\n");
                        break;

                    case '2':
                        printf("\nSet Autofire Speed Menu\n");
                        break;

                    default:
                        console_show_main_menu();
                }
                break;

            default: // MENU_MAIN
                switch(c)
                {
                    case '1':
                        console_show_mouse_menu();
                        break;

                    case '2':
                        printf("\nJoystick Menu\n");
                        break;
                }
                break;
        }
    }
}