#include <stdio.h>
#include "CH559.h"
#include "util.h"
#include "console.h"
#include "config.h"
#include "USBHost.h"
#include "led.h"

/* --------------------------------------------------
   Menu state
-------------------------------------------------- */

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

static const char __code header[] =
"  ____      _             _     _       _\r\n"
" |  _ \\ ___| |_ _ __ ___ | |   (_)_ __ | | __\r\n"
" | |_) / _ \\ __| '__/ _ \\| |   | | '_ \\| |/ /\r\n"
" |  _ <  __/ |_| | | (_) | |___| | | | |   <\r\n"
" |_| \\_\\___|\\__|_|  \\___/|_____|_|_| |_|_|\\_\\\r\n";
static const char __code title1[] = "RetroLink v1.02";
static const char __code title2[] = "USB Input Adapter";
static const char __code title3[] = "ST / Amiga / C64";

static const char __code github_url[] =
"github.com/Fbeen/RetroLink";

static const char __code main_menu[] =
"1. Learn Controller\r\n"
"2. Emulate ST or Amiga mouse\r\n"
"3. Swap Mouse Buttons\r\n"
"4. Set Mouse Speed\r\n"
"5. Autofire Frequency\r\n";

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

static const char __code txt_mb_normal[] =
"Mouse buttons normal";

static const char __code txt_mb_swapped[] =
"Mouse buttons swapped";

static const char __code txt_st_mode[] =
"Emulate Atari ST mouse";

static const char __code txt_amiga_mode[] =
"Emulate Amiga mouse";

static const char __code learn_prompts[] =
"Push UP\0"
"Push DOWN\0"
"Push LEFT\0"
"Push RIGHT\0"
"Press FIRE\0"
"Press AUTOFIRE\0";

static const char __code txt_autofire[] =
"Autofire Frequency";

static const char __code txt_autofire_menu[] =
"1 - Autofire 8 Hz\r\n"
"2 - Autofire 9 Hz\r\n"
"3 - Autofire 10 Hz\r\n"
"4 - Autofire 11 Hz\r\n"
"5 - Autofire 12 Hz\r\n"
"0 - Back\r\n";

static const char __code txt_autofire_updated[] =
"Autofire frequency updated";

/* --------------------------------------------------
   Helpers
-------------------------------------------------- */

/*
 * prints 40 empty lines on the console. Looks like a page refresh
 */
static void console_clear(void)
{
    uint8_t i;

    for(i = 0; i < 40; i++)
        puts("");
}

/*
 * Return a pointer to the N-th prompt string from the learn_prompts table.
 * This is UP, DOWN, LEFT, RIGHT, FIRE and AUTOFIRE.
 */
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

/*
 * Check if the given HID report is within the learned idle range.
 */
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

/*
 * Find the first byte in the report that deviates from the idle range.
 * Returns the index, or 255 if no change is detected.
 */
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

/*
 * Initialize idle detection by resetting min/max values
 * and starting the sampling process.
 */
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

/*
 * Update idle min/max values using the latest HID report.
 */
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

/*
 * Build a control mapping entry based on the current HID report.
 * Determines axis direction or button based on deviation from idle.
 */
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
        CTRL_SET_AXIS(map, AXIS_NEG);
        map->threshold = idle_mid;
        map->mask = 0;
        return 1;
    }

    if(v > idle_mid)
    {
        CTRL_SET_AXIS(map, AXIS_POS);
        map->threshold = idle_mid;
        map->mask = 0;
        return 1;
    }

    CTRL_SET_BUTTON(map);
    map->threshold = v;
    map->mask = 0;

    return 1;
}

/* --------------------------------------------------
   UI functions
-------------------------------------------------- */

/*
 * Print the RetroLink header and project information.
 */
static void show_header(void)
{
    puts(header);
    puts(title1);
    puts(title2);
    puts(title3);
    puts("");
    puts(github_url);
    puts("");
}

/*
 * Display the main menu and switch to MENU_MAIN state.
 */
void show_main_menu(void)
{
    show_header();
    puts("Main Menu");
    puts(stripe);
    puts(main_menu);

    menu_state = MENU_MAIN;
}

/*
 * Display the mouse speed configuration menu.
 */
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

/*
 * Start the controller learning wizard.
 * If started from hardware button, adds a small delay.
 */
void start_learning(uint8_t console)
{
    learn_step = 0;
    learn_wait_release = 0;

    console_clear();
    puts(txt_learning);
    puts(stripe);

    learner_idle_begin();

    /* delays only when started with pcb button */
    if(!console) {
        delay(500);
    }

    led_activate(200, 600);

    menu_state = MENU_JOY_LEARN;
}

/*
 * Advance to the next learning step or finalize the process.
 */
static void next_learn_step(void)
{
    learn_step++;

    if(learn_step >= LEARN_STEPS)
    {
        puts("");
        puts(txt_done);
        led_activate(2000, 2000);

        /* now save the vid and pid */
        g_config.vid = get_vid();
        g_config.pid = get_pid();
        config_save();

        console_clear();
        show_main_menu();
        return;
    }

    puts("");
    puts(get_prompt(learn_step));
    led_activate(200, 600);
}

/* --------------------------------------------------
   Public API
-------------------------------------------------- */

/*
 * Initialize the console and show the main menu.
 */
void console_start(void)
{
    show_main_menu();
}

/*
 * Handle console input, learning wizard logic,
 * and button-based abort during learning.
 */
void console_task(void)
{
    char c;

    /* PCB button abort (alleen tijdens learning) */
    if(menu_state == MENU_JOY_LEARN)
    {
        if(!(P4_IN & (1 << 6)))   // pressed
        {
            abort_learning();

            /* wacht tot losgelaten (simpel debounce) */
            while(!(P4_IN & (1 << 6)));

            return;
        }
    }
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
                    g_config.map[learn_step] = map;
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

    /* ESC = abort learning */
    if(c == 27 && menu_state == MENU_JOY_LEARN)
    {
        abort_learning();
        return;
    }
    
    switch(menu_state)
    {
        case MENU_MAIN:

            if(c == '1')
                start_learning(true);
            else if(c == '2')
                swap_mouse_mode();
            else if(c == '3')
                swap_mouse_buttons();
            else if(c == '4')
                show_mouse_speed_menu();
            else if(c == '5')
                show_autofire_menu();

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

        case MENU_AUTOFIRE:

            if(c >= '1' && c <= '5')
            {
                g_config.joy_autofire_speed = c - '0';
                config_save();

                console_clear();
                puts(txt_autofire_updated);
                show_main_menu();
            }
            else if(c == '0')
            {
                show_main_menu();
            }

            break;
    }
}

/*
 * Toggle mouse button swap setting and provide feedback.
 */
void swap_mouse_buttons(void)
{
    // swap buttons
    g_config.mouse_swap_buttons ^= 1;
    config_save();
    
    console_clear();
    if(g_config.mouse_swap_buttons) {
        puts(txt_mb_swapped);
        delay(1000);
        led_activate(200, 600);
    } else {
        puts(txt_mb_normal);
        delay(1000);
        led_activate(200, 200);
    }
    puts("");

    show_main_menu();
}

/*
 * Toggle mouse mode (ST/Amiga) and provide feedback.
 */
void swap_mouse_mode(void)
{
    // swap buttons
    g_config.mouse_swap_mode ^= 1;
    config_save();

    console_clear();
    if(g_config.mouse_swap_mode) {
        puts(txt_amiga_mode);
        delay(1000);
        led_activate(200, 600);
    } else {
        puts(txt_st_mode);
        delay(1000);
        led_activate(200, 200);
    }
    puts("");

    show_main_menu();
}

/*
 * Display the autofire configuration menu.
 */
static void show_autofire_menu(void)
{
    console_clear();

    puts(txt_autofire);
    puts(stripe);

    puts(txt_autofire_menu);

    menu_state = MENU_AUTOFIRE;
}

/*
 * Increase mouse speed setting and indicate new value via LED.
 */
void inc_mouse_speed()
{
    g_config.mouse_speed++;

    if( g_config.mouse_speed > 5)
         g_config.mouse_speed = 1;

    config_save();
    delay(1000);
    led_activate(200, 400 * g_config.mouse_speed);
}

/*
 * Increase autofire speed setting and indicate new value via LED.
 */
void inc_autofire_speed()
{
    g_config.joy_autofire_speed++;

    if( g_config.joy_autofire_speed > 5)
         g_config.joy_autofire_speed = 1;

    config_save();
    delay(1000);
    led_activate(200, 400 * g_config.joy_autofire_speed);
}

/*
 * Abort the learning wizard and return to the main menu.
 */
static void abort_learning(void)
{
    puts("");
    puts("Learning aborted");

    console_clear();
    show_main_menu();
}

/*
 * Get the current console menu state.
 */
console_state_t console_get_state(void)
{
    return menu_state;
}