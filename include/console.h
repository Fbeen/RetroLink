#ifndef CONSOLE_H
#define CONSOLE_H

static void show_header(void);
static void show_main_menu(void);
static void show_mouse_menu(void);
static void show_mouse_speed_menu(void);
static void show_joystick_menu(void);
static void start_joystick_learning(void);
static void next_learn_step(void);
void console_start(void);
void console_task(void);
void swap_mouse_buttons(void);
static void show_autofire_menu(void);

#endif
