#include <stdio.h>
#include "CH559.h"
#include "util.h"
#include "console.h"
#include "controller_learn.h"
#include "config.h"

typedef enum
{
    MENU_MAIN,
    MENU_MOUSE,
    MENU_MOUSE_SPEED,
    MENU_JOYSTICK,
    MENU_JOY_LEARN
} console_state_t;

static console_state_t __xdata menu_state = MENU_MAIN;

static uint8_t __xdata learn_step = 0;

/* ---------------- constant strings in flash ---------------- */

static const char __code newline[] = "";
static const char __code stripe[] = "--------------------------------";
/*
static const char __code header1[] = "  ____      _             _     _       _";
static const char __code header2[] = " |  _ \\ ___| |_ _ __ ___ | |   (_)_ __ | | __";
static const char __code header3[] = " | |_) / _ \\ __| '__/ _ \\| |   | | '_ \\| |/ /";
static const char __code header4[] = " |  _ <  __/ |_| | | (_) | |___| | | | |   <";
static const char __code header5[] = " |_| \\_\\___|\\__|_|  \\___/|_____|_|_| |_|_|\\_\\";
*/
static const char __code title1[] = " RetroLink v1.02";
static const char __code title2[] = " USB Input Adapter";
static const char __code title3[] = " ST / Amiga / C64";

static const char __code main_menu1[] = "1. Mouse Settings";
static const char __code main_menu2[] = "2. Joystick Settings";

static const char __code mouse_menu1[] = "1. Set Mouse Speed";
static const char __code mouse_menu2[] = "2. Map Mouse Buttons";
static const char __code menu_back[]  = "0. Back";

static const char __code speed1[] = "1 - Very Slow";
static const char __code speed2[] = "2 - Slow";
static const char __code speed3[] = "3 - Normal";
static const char __code speed4[] = "4 - Fast";
static const char __code speed5[] = "5 - Turbo";

static const char __code joy_menu1[] = "1. Learn Controller";
static const char __code joy_menu2[] = "2. Set Autofire Speed";

static const char __code learn_press[] = "Press ENTER when ready";

static const char __code txt_main_menu[] = "Main Menu";
static const char __code txt_mouse_menu[] = "Mouse Menu";
static const char __code txt_joystick_menu[] = "Joystick Menu";
static const char __code txt_mouse_speed[] = "Mouse Speed";
static const char __code txt_current_speed[] = "Current speed:";
static const char __code txt_learning[] = "Controller Learning Wizard";
static const char __code txt_done[] = "Controller configuration complete";
static const char __code txt_no_change[] = "No change detected";
static const char __code txt_autofire[] = "Autofire menu";
static const char __code txt_map_buttons[] = "Map Mouse Buttons";
static const char __code txt_mouse_speed_updated[] = "Mouse speed updated";

/* joystick learning prompts */

static const char __code learn_prompts[][22] =
{
    "Release all controls",
    "Push UP",
    "Push DOWN",
    "Push LEFT",
    "Push RIGHT",
    "Press FIRE",
    "Press AUTOFIRE"
};

#define LEARN_STEPS (sizeof(learn_prompts)/sizeof(learn_prompts[0]))

/* ---------------- helpers ---------------- */

static void show_header(void)
{
/*    puts(newline);
    puts(header1);
    puts(header2);
    puts(header3);
    puts(header4);
    puts(header5);
*/
    puts(newline);
    puts(title1);
    puts(title2);
    puts(title3);
    puts(newline);
}

static void show_main_menu(void)
{
    puts(newline);
    puts(txt_main_menu);
    puts(stripe);
    puts(main_menu1);
    puts(main_menu2);
    puts(newline);

    menu_state = MENU_MAIN;
}

static void show_mouse_menu(void)
{
    puts(newline);
    puts(txt_mouse_menu);
    puts(stripe);
    puts(mouse_menu1);
    puts(mouse_menu2);
    puts(menu_back);
    puts(newline);

    menu_state = MENU_MOUSE;
}

static void show_mouse_speed_menu(void)
{
    puts(newline);
    puts(txt_mouse_speed);
    puts(stripe);

    puts(txt_current_speed);
    UART0Send('0' + g_config.mouse_speed);
    puts(newline);

    puts(speed1);
    puts(speed2);
    puts(speed3);
    puts(speed4);
    puts(speed5);
    puts(menu_back);

    menu_state = MENU_MOUSE_SPEED;
}

static void show_joystick_menu(void)
{
    puts(newline);
    puts(txt_joystick_menu);
    puts(stripe);
    puts(joy_menu1);
    puts(joy_menu2);
    puts(menu_back);
    puts(newline);

    menu_state = MENU_JOYSTICK;
}

static void start_joystick_learning(void)
{
    learn_step = 0;
    controller_learning_active = 1;
    controller_learning_step = 0;
    controller_learn_reset();

    puts(newline);
    puts(txt_learning); // Controller Learning Wizard
    puts(stripe);       // --------------------------

    menu_state = MENU_JOY_LEARN;

    next_learn_step();
}

static void next_learn_step(void)
{
    if(learn_step >= LEARN_STEPS)
    {
        controller_learning_active = 0;
        puts(newline);
        puts(txt_done);       // Controller configuration complete
        show_joystick_menu();
        return;
    }

    controller_learning_step = learn_step;

    puts(newline);
    puts(learn_prompts[learn_step]); // Push UP/DOWN/LEFT/RIGHT/FIRE/AUTOFIRE

    puts(learn_press);

    learn_step++;
}

/* ---------------- public API ---------------- */

void console_start(void)
{
    show_header();
    show_main_menu();
}

void console_task(void)
{
    char c;

    if(!UART0Available())
        return;

    c = UART0Receive();

    switch(menu_state)
    {
        case MENU_MAIN:

            if(c == '1')
                show_mouse_menu();
            else if(c == '2')
                show_joystick_menu();

            break;

        case MENU_MOUSE:

            if(c == '1')
                show_mouse_speed_menu();
            else if(c == '2')
                puts(txt_map_buttons);
            else if(c == '0')
                show_main_menu();

            break;

        case MENU_MOUSE_SPEED:

            if(c >= '1' && c <= '5')
            {
                g_config.mouse_speed = c - '0';
                config_save();

                puts(txt_mouse_speed_updated);
                show_mouse_menu();
            }
            else if(c == '0')
            {
                show_mouse_menu();
            }

            break;

        case MENU_JOYSTICK:

            if(c == '1')
                start_joystick_learning();
            else if(c == '2')
                puts(txt_autofire);
            else if(c == '0')
                show_main_menu();

            break;

        case MENU_JOY_LEARN:

            if(c == '\r')
            {
                /* stap 0 = idle capture */

                if(controller_learning_step == 0)
                {
                    controller_learn_capture_idle(g_last_report, g_last_report_len);
                    puts("Idle captured len=");
                    print_u8(g_last_report_len);
                    puts("\r\n");
                }
                else
                {
                    control_map_t map;

                    controller_learn_capture_sample(g_last_report);

                    if(controller_learn_analyze(&map))
                    {
                        puts("Mapping detected: offset=");
                        print_u8(map.offset);
                        puts(" mask=");
                        print_u8(map.mask);
                        puts(" thr=");
                        print_u8(map.threshold);
                        puts("\r\n");

                        switch(controller_learning_step)
                        {
                            case 1:
                                g_config.joy_up = map;
                                break;

                            case 2:
                                g_config.joy_down = map;
                                break;

                            case 3:
                                g_config.joy_left = map;
                                break;

                            case 4:
                                g_config.joy_right = map;
                                break;

                            case 5:
                                g_config.joy_fire = map;
                                break;

                            case 6:
                                g_config.joy_autofire = map;
                                break;
                        }
                    }
                    else
                    {
                        puts(txt_no_change);
                    }
                }

                next_learn_step();
            }

            break;
    }
}