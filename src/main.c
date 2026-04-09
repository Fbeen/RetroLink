typedef unsigned char *PUINT8;
typedef unsigned char __xdata *PUINT8X;
typedef const unsigned char __code *PUINT8C;
typedef unsigned char __xdata UINT8X;
typedef unsigned char  __data             UINT8D;

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "CH559.h"
#include "USBHost.h"
#include "util.h"
#include "console.h"
#include "retro_mouse.h"
#include "retro_joystick.h"
#include "config.h"
#include "hardware.h"
#include "led.h"

//CH559 Timer0 clock selection
//bTMR_CLK affects Timer0&1&2 at the same time, pay attention when using
#define mTimer0ClkFsys( ) (T2MOD |= bTMR_CLK | bT0_CLK) //timer, clock=Fsys
#define mTimer0Clk4DivFsys( ) (T2MOD &= ~bTMR_CLK;T2MOD |= bT0_CLK) //timer, clock=Fsys/4
#define mTimer0Clk12DivFsys( ) (T2MOD &= ~(bTMR_CLK | bT0_CLK)) //timer, clock= Fsys/12
#define mTimer0CountClk( ) (TMOD |= bT0_CT) //counter, the falling edge of T0 pin is valid


/* --------------------------------------------------
   Timer defines
-------------------------------------------------- */

#define BTN_STEP     10    // 1 second
#define BTN_T_BOOT   100   // 10 seconds

/* --------------------------------------------------
   Button variables
-------------------------------------------------- */

static uint8_t  btn_event = 0;

/* --------------------------------------------------
   Autofire
-------------------------------------------------- */

static uint16_t autofire_counter = 0;
static uint8_t autofire_state = 0;

static const uint16_t __code autofire_ticks[] =
{
    0,
    150,
    133,
    120,
    109,
    100
};

/* --------------------------------------------------
   Timer helpers
-------------------------------------------------- */

void mTimer0ModSetup(unsigned char mode)
{
    TMOD &= 0xf0;
    TMOD |= mode;
}

void mTimer0SetData(unsigned short dat)
{
    unsigned short tmp;
    tmp = 65536 - dat;
    TL0 = tmp & 0xff;
    TH0 = (tmp >> 8) & 0xff;
}

/* --------------------------------------------------
   TIMER ISR
-------------------------------------------------- */

/*
 * Called every 100 milliseconds from the ISR below.
 * Handles button presses and press durations
 */
void btn_tick()
{
    static uint16_t btn_ticks = 0;
    static uint8_t  btn_state = 0;
    static uint8_t level = 1;

    if(!(P4_IN & (1 << 6)))   // pressed (active low)
    {
        // initializes a new button press
        if(btn_state == 0)
        {
            btn_state = 1;
            btn_ticks = 0;
            level = 1;
        }

        btn_ticks++; // timer ticks

        // on fixed intervals
        if(btn_ticks % BTN_STEP == 0)
        {
            // next menu option
            if(level < 6)
            {
                level++;
                // puts("LED");
                led_activate(500, 500);
            } else
            // after 10 seconds activate firmware flash mode
            if(btn_ticks >= BTN_T_BOOT)
            {
                btn_ticks = BTN_T_BOOT;
                led_on();
                level = 7;
            }
        }
    }
    else // release
    {
        if(btn_state == 1)
        {
            btn_event = level; /* activates button event in main function */
            btn_state = 0;
            led_off();
        }
    }
}

/*
 * ISR of hardware timer 0. Called 2400 times per second!
 */
void mTimer0Interrupt(void) __interrupt INT_NO_TMR0
{
    static uint8_t ticks = 0;

    /* ---- MOUSE / JOYSTICK ---- */

    if(USBHost_getControllerMode() == CTRL_MODE_MOUSE)
    {
        rm_nextStep(); // generates mouse quadrature (movements)
    }
    else
    {
        /* joystick autofire button pressed ? */
        if(autofire_active)
        {
            autofire_counter++;

            if(autofire_counter >= autofire_ticks[g_config.joy_autofire_speed])
            {
                autofire_counter = 0;
                autofire_state ^= 1;
            }

            JOY_FIRE = autofire_state ? 0 : 1;
        }
    }

    /* ---- BUTTON HANDLING ---- */

    ticks++;

    /* prescaler 2400 hz / 240 = 10 hz or each 100 ms */
    if(ticks >= 240)
    {
        ticks = 0;

        if(console_get_state() == MENU_MAIN)
            btn_tick();

        led_tick();
    }

    mTimer0SetData(20000);  // reset timer 0 to 2400 Hz (48.000.000 / 20.000 = 2400 Hz)
}

/* --------------------------------------------------
   MAIN
-------------------------------------------------- */

/*
 * program start point
 */
void main()
{
    SP = 0x80;

    hw_setup();
    initClock();
    initUART0(1000000, 1);

    if(USBHost_getControllerMode() == CTRL_MODE_MOUSE)
        rm_init();
    else
        rj_init();

    if(!config_load())
    {
        puts("Config corrupt, defaults loaded");
        config_save();
    }

    resetHubDevices(0);
    initUSB_Host();

    console_start();

    mTimer0ModSetup(1);
    mTimer0ClkFsys();
    mTimer0SetData(20000 /*0x2323 */);

    TR0 = 1;
    ET0 = 1;
    EA  = 1;

    /* loop forever */
    while(1)
    {
        /* processes pcb button menu choices */
        if(btn_event)
        {
            uint8_t ev = btn_event;
            btn_event = 0;

            switch(ev)
            {
                case 1: show_main_menu(); break;
                case 2: start_learning(false); break;
                case 3: swap_mouse_mode(); break;
                case 4: swap_mouse_buttons(); break;
                case 5: inc_mouse_speed(); break;
                case 6: inc_autofire_speed(); break;
                case 7: runBootloader(); break;
            }
        }

        checkRootHubConnections(); // Detects and handles USB device connect/disconnect events on the root hub
        pollHIDdevice();           // Polls the active HID device for new input reports and processes them
        console_task();            // Handles console input/output, including command parsing and user interaction
    }
}