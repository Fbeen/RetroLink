/*
static const char __code header[] =
"  ____      _             _     _       _\r\n"
" |  _ \\ ___| |_ _ __ ___ | |   (_)_ __ | | __\r\n"
" | |_) / _ \\ __| '__/ _ \\| |   | | '_ \\| |/ /\r\n"
" |  _ <  __/ |_| | | (_) | |___| | | | |   <\r\n"
" |_| \\_\\___|\\__|_|  \\___/|_____|_|_| |_|_|\\_\\\r\n";
*/
#include <stdio.h>
#include "CH559.h"
#include "util.h"
#include "console.h"
#include "config.h"

/* --------------------------------------------------
   Menu state
-------------------------------------------------- */

typedef enum
{
    MENU_MAIN,
    MENU_MOUSE_SPEED,
    MENU_JOY_LEARN
} console_state_t;

static console_state_t __xdata menu_state = MENU_MAIN;

/* --------------------------------------------------
   Learner state
-------------------------------------------------- */

#define REPORT_LEN 8
#define IDLE_SAMPLES 2000
#define LEARN_STEPS 6

static uint8_t __xdata idle_min[REPORT_LEN];
static uint8_t __xdata idle_max[REPORT_LEN];

static uint16_t __xdata idle_sample_counter = 0;

static uint8_t learn_step = 0;
static uint8_t learn_wait_release = 0;

/* --------------------------------------------------
   Constant strings
-------------------------------------------------- */

static const char __code stripe[] =
"--------------------------------";

static const char __code title1[] = "RetroLink v1.02";
static const char __code title2[] = "USB Input Adapter";
static const char __code title3[] = "ST / Amiga / C64";

static const char __code github_url[] =
"github.com/Fbeen/RetroLink";

static const char __code main_menu[] =
"1. Set Mouse Speed\r\n"
"2. Map Mouse Buttons\r\n"
"3. Learn Controller\r\n"
"4. Autofire Settings\r\n";

static const char __code speed_menu[] =
"1 - Very Slow\r\n"
"2 - Slow\r\n"
"3 - Normal\r\n"
"4 - Fast\r\n"
"5 - Turbo\r\n"
"0 - Back\r\n";

static const char __code txt_mouse_speed[] =
"Mouse Speed";

static const char __code txt_current_speed[] =
"Current speed:";

static const char __code txt_speed_updated[] =
"Mouse speed updated";

static const char __code txt_learning[] =
"Controller Learning Wizard";

static const char __code txt_idle_scan[] =
"Release joystick and scanning...";

static const char __code txt_done[] =
"Controller configuration complete";

static const char __code learn_prompts[] =
"Push UP\0"
"Push DOWN\0"
"Push LEFT\0"
"Push RIGHT\0"
"Press FIRE\0"
"Press AUTOFIRE\0";

/* --------------------------------------------------
   Helpers
-------------------------------------------------- */

static void console_clear(void)
{
    uint8_t i;

    for(i = 0; i < 40; i++)
        puts("");
}

static const char* get_prompt(uint8_t index)
{
    const char* p = learn_prompts;

    while(index)
    {
        while(*p++) ;
        index--;
    }

    return p;
}

static uint8_t report_is_idle(uint8_t *r)
{
    uint8_t i;
    uint8_t v;

    for(i=0;i<REPORT_LEN;i++)
    {
        v = r[i];

        if(v < idle_min[i] || v > idle_max[i])
            return 0;
    }

    return 1;
}

static uint8_t find_changed_byte(uint8_t *r)
{
    uint8_t i;
    uint8_t v;

    for(i=0;i<REPORT_LEN;i++)
    {
        v = r[i];

        if(v < idle_min[i] || v > idle_max[i])
            return i;
    }

    return 255;
}

/* --------------------------------------------------
   Learner
-------------------------------------------------- */

static void learner_idle_begin(void)
{
    uint8_t i;

    idle_sample_counter = 0;

    for(i=0;i<REPORT_LEN;i++)
    {
        idle_min[i] = 0xFF;
        idle_max[i] = 0x00;
    }

    puts(txt_idle_scan);
}

static void learner_idle_update(void)
{
    uint8_t i;
    uint8_t v;

    for(i=0;i<REPORT_LEN;i++)
    {
        v = g_last_report[i];

        if(v < idle_min[i])
            idle_min[i] = v;

        if(v > idle_max[i])
            idle_max[i] = v;
    }

    idle_sample_counter++;
}

static uint8_t build_mapping(control_map_t *map)
{
    uint8_t idx;
    uint8_t v;
    uint8_t idle_mid;

    idx = find_changed_byte(g_last_report);

    if(idx == 255)
        return 0;

    v = g_last_report[idx];
    idle_mid = (idle_min[idx] + idle_max[idx]) >> 1;

    map->offset = idx;

    if(v < idle_mid)
    {
        map->type = CTRL_INPUT_AXIS;
        map->axis_dir = AXIS_NEG;
        map->threshold = idle_mid;
        map->mask = 0;
        return 1;
    }

    if(v > idle_mid)
    {
        map->type = CTRL_INPUT_AXIS;
        map->axis_dir = AXIS_POS;
        map->threshold = idle_mid;
        map->mask = 0;
        return 1;
    }

    map->type = CTRL_INPUT_BUTTON;
    map->threshold = v;
    map->mask = 0;
    map->axis_dir = 0;

    return 1;
}

/* --------------------------------------------------
   UI functions
-------------------------------------------------- */

static void show_header(void)
{
    puts(title1);
    puts(title2);
    puts(title3);
    puts("");
    puts(github_url);
    puts("");
}

static void show_main_menu(void)
{
    show_header();
    puts("Main Menu");
    puts(stripe);
    puts(main_menu);

    menu_state = MENU_MAIN;
}

static void show_mouse_speed_menu(void)
{
    console_clear();

    puts(txt_mouse_speed);
    puts(stripe);

    puts(txt_current_speed);
    UART0Send('0' + g_config.mouse_speed);
    puts("");

    puts(speed_menu);

    menu_state = MENU_MOUSE_SPEED;
}

/* --------------------------------------------------
   Learning wizard
-------------------------------------------------- */

static void start_learning(void)
{
    learn_step = 0;
    learn_wait_release = 0;

    console_clear();
    puts(txt_learning);
    puts(stripe);

    learner_idle_begin();

    menu_state = MENU_JOY_LEARN;
}

static void next_learn_step(void)
{
    learn_step++;

    if(learn_step >= LEARN_STEPS)
    {
        puts("");
        puts(txt_done);

        config_save();

        console_clear();
        show_main_menu();
        return;
    }

    puts("");
    puts(get_prompt(learn_step));
}

/* --------------------------------------------------
   Public API
-------------------------------------------------- */

void console_start(void)
{
    show_main_menu();
}

void console_task(void)
{
    char c;

    /* learner */

    if(menu_state == MENU_JOY_LEARN)
    {
        if(idle_sample_counter < IDLE_SAMPLES)
        {
            learner_idle_update();

            if(idle_sample_counter == IDLE_SAMPLES)
            {
                uint8_t i;

                /* jitter marge toevoegen */
                for(i=0;i<REPORT_LEN;i++)
                {
                    if(idle_min[i] > 2)
                        idle_min[i] -= 2;

                    if(idle_max[i] < 253)
                        idle_max[i] += 2;
                }

                puts(get_prompt(learn_step));
            }

            return;
        }

        if(!learn_wait_release)
        {
            if(!report_is_idle(g_last_report))
            {
                control_map_t map;

                if(build_mapping(&map))
                {
                    switch(learn_step)
                    {
                        case 0: g_config.joy_up = map; break;
                        case 1: g_config.joy_down = map; break;
                        case 2: g_config.joy_left = map; break;
                        case 3: g_config.joy_right = map; break;
                        case 4: g_config.joy_fire = map; break;
                        case 5: g_config.joy_autofire = map; break;
                    }

                    learn_wait_release = 1;
                }
            }
        }
        else
        {
            if(report_is_idle(g_last_report))
            {
                learn_wait_release = 0;
                next_learn_step();
            }
        }
    }

    /* UART */

    if(!UART0Available())
        return;

    c = UART0Receive();

    switch(menu_state)
    {
        case MENU_MAIN:

            if(c == '1')
                show_mouse_speed_menu();
            else if(c == '2')
                puts("Map Mouse Buttons (TODO)");
            else if(c == '3')
                start_learning();
            else if(c == '4')
                puts("Autofire Settings (TODO)");

            break;

        case MENU_MOUSE_SPEED:

            if(c >= '1' && c <= '5')
            {
                g_config.mouse_speed = c - '0';
                config_save();

                console_clear();
                puts(txt_speed_updated);
                show_main_menu();
            }
            else if(c == '0')
            {
                show_main_menu();
            }

            break;

        case MENU_JOY_LEARN:
            break;
    }
}