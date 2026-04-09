#ifndef HARDWARE_H
#define HARDWARE_H

#include "CH559.h"

/* remove this line on the final hardware version */
#define BETA_VERSION

/* hardware register addresses */
#define PORT0 0x80
#define PORT1 0x90
#define PORT2 0xA0
#define PORT3 0xB0
#define PORT4 0xC0

/* port configuration registers */

SFR(PORT_CFG, 0xC6);

SFR(P0, PORT0);
SFR(P0_DIR, 0xC4);
SFR(P0_PU,  0xC5);

SFR(P1, PORT1);
SFR(P1_DIR, 0xBA);

SFR(P2, PORT2);
SFR(P2_DIR, 0xBC);

SFR(P3, PORT3);
SFR(P3_DIR, 0xBE);

SFR(P4, PORT4);
// SFR(P4_DIR, 0xC3); // already defined in ch559.h


/* ST mouse aliases */
SBIT(MX2, PORT2, 6);        // DB9 pin 1 = X2
SBIT(MX1, PORT2, 4);        // DB9 pin 2 = X1
SBIT(MY1, PORT2, 3);        // DB9 pin 3 = Y1
SBIT(MY2, PORT2, 2);        // DB9 pin 4 = Y2
SBIT(MBTN_LEFT, PORT2, 5);  // DB9 pin 6 = LEFT MOUSE BUTTON
SBIT(MBTN_RIGHT, PORT2, 1); // DB9 pin 9 = RIGHT MOUSE BUTTON

/* ST joystick aliases */
SBIT(JOY_UP, PORT2, 6);     // DB9 pin 1 = UP
SBIT(JOY_DOWN,  PORT2, 4);  // DB9 pin 2 = DOWN
SBIT(JOY_LEFT,  PORT2, 3);  // DB9 pin 3 = LEFT
SBIT(JOY_RIGHT, PORT2, 2);  // DB9 pin 4 = RIGHT
SBIT(JOY_FIRE,  PORT2, 5);  // DB9 pin 6 = FIRE

/* status LED */
#ifdef BETA_VERSION
    SBIT(LED_STATUS, PORT0, 4);   // P0.4
#else
    SBIT(LED_STATUS, PORT4, 3);   // P4.3
#endif

void hw_setup(void);

#endif