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
#include "config.h"
#include "hardware.h"

//CH559 Timer0 clock selection
//bTMR_CLK affects Timer0&1&2 at the same time, pay attention when using
#define mTimer0ClkFsys( ) (T2MOD |= bTMR_CLK | bT0_CLK) //timer, clock=Fsys
#define mTimer0Clk4DivFsys( ) (T2MOD &= ~bTMR_CLK;T2MOD |= bT0_CLK) //timer, clock=Fsys/4
#define mTimer0Clk12DivFsys( ) (T2MOD &= ~(bTMR_CLK | bT0_CLK)) //timer, clock= Fsys/12
#define mTimer0CountClk( ) (TMOD |= bT0_CT) //counter, the falling edge of T0 pin is valid

/******************************************** ************************************
* Function Name : mTimer0ModSetup(UINT8 mode)
* Description : CH559 Timing counter 0 mode 0 setting
* Input: UINT8 mode, Timer0 mode selection
                   0: Mode 0, 13-bit timer, TL0 high 3 bits are invalid
                   1: Mode 1, 16-bit timer
                   2: Mode 2, 8-bit auto-reload timer
                   3: Mode 3, two 8-bit timers
***************************************** *******************************************/
void mTimer0ModSetup(unsigned char mode)
{
    TMOD &= 0xf0;
    TMOD |= mode;
}

/****************************************** ****************************************
* Function Name : mTimer0SetData(UINT16 dat)
* Description : CH559Timer0 TH0 and TL0 assignment
* Input : UINT16 dat; timer assignment
* Output : None
* Return : None
*************************** ***************************************************** **/
void mTimer0SetData(unsigned short dat)
{
    unsigned short tmp;
    tmp = 65536 - dat;
    TL0 = tmp & 0xff;
    TH0 = (tmp>>8) & 0xff;
}

void mTimer0Interrupt( void ) __interrupt INT_NO_TMR0 // timer0 interrupt-serviceroutine, gebruik registergroep 1
{   // In modus 3 gebruikt TH0 Timer1 interrupt-resource
    if(USBHost_getControllerMode() == CTRL_MODE_MOUSE)
    {
        rm_nextStep();
    }
    else
    {
        /* joystick timing / autofire later */
    }
    mTimer0SetData(20000);  // set timer on 0.5 millisecond (48.000.000 hz / 20.000 = 2400 hz)
}

void main()
{
    SP = 0x80;

    /* system clock for USB */
    initClock();

    /* debug UART */
    initUART0(1000000, 1);

    /* retro mouse pins + state */
    rm_init();

    /* load configuration */
    if(!config_load())
    {
        puts("Config data corrupt, defaults loaded");
        config_save();
    }

    /* start USB host */
    resetHubDevices(0);
    initUSB_Host();

    /* start console */
    console_start();

    /* start timer (quadrature generator) */
    mTimer0ModSetup(1);
    mTimer0ClkFsys();
    mTimer0SetData(0x2323);

    TR0 = 1;
    ET0 = 1;
    EA  = 1;

    while(1)
    {
        if(!(P4_IN & (1 << 6)))
            runBootloader();

        checkRootHubConnections();
        pollHIDdevice();
        console_task();
    }
}
