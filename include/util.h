#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdio.h>
#if 0
#define DEBUG_OUT(...) printf(__VA_ARGS__);
#else
#define DEBUG_OUT(...) (void)0;
#endif

void initClock();
void delayUs(unsigned short n);
void delay(unsigned short n);
void initUART0(unsigned long baud, int alt);
char UART0Available(void);
unsigned char UART0Receive();
void UART0Send(unsigned char b);

/* ================= Mouse/Joystick mode ================= */

typedef enum
{
    CTRL_MODE_NONE = 0,
    CTRL_MODE_MOUSE = 1,
    CTRL_MODE_JOYSTICK = 2
} controller_mode_t;

#define PIN_MODE_INPUT 0
#define PIN_MODE_INPUT_PULLUP 1
#define PIN_MODE_OUTPUT 2
#define PIN_MODE_OUTPUT_OPEN_DRAIN 3
#define PIN_MODE_OUTPUT_OPEN_DRAIN_2CLK 4
#define PIN_MODE_INPUT_OUTPUT_PULLUP 5
#define PIN_MODE_INPUT_OUTPUT_PULLUP_2CLK 6
void pinMode(unsigned char port, unsigned char pin, unsigned char mode);

typedef void(* __data FunctionReference)();
extern FunctionReference runBootloader;
/*
void print_u8(uint8_t v);
void print_str(const char *s);
void print_hex8(uint8_t v);
void debug_print_report(uint8_t *r, uint8_t len);
*/
#endif
