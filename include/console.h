#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum
{
    MENU_MAIN,
    MENU_JOY_LEARN,
    MENU_MOUSE_SPEED,
    MENU_AUTOFIRE
} console_state_t;

static void console_clear(void);
static const char* get_prompt(uint8_t index);
static uint8_t report_is_idle(uint8_t *r);
static uint8_t find_changed_byte(uint8_t *r);
static void learner_idle_begin(void);
static void learner_idle_update(void);
// static uint8_t build_mapping(control_map_t *map);
static void show_header(void);
void show_main_menu(void);
static void show_mouse_speed_menu(void);
void start_learning(void);
static void next_learn_step(void);
void console_start(void);
void console_task(void);
void swap_mouse_buttons(void);
void swap_mouse_mode(void);
static void show_autofire_menu(void);
void inc_mouse_speed();
void inc_autofire_speed();
static void abort_learning(void);
console_state_t console_get_state(void);

#endif
