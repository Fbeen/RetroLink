#ifndef CONSOLE_H
#define CONSOLE_H

void console_start(void);
static void console_show_header(void);
void console_show_main_menu(void);
static void console_show_mouse_menu(void);
static void console_show_joystick_menu(void);
void console_task(void);

#endif