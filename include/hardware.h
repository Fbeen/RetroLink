#ifndef HARDWARE_H
#define HARDWARE_H

#include "CH559.h"

/* hardware register addresses */

#define PORT1 0x90
#define PORT2 0xA0
#define PORT3 0xB0

/* port configuration registers */

SFR(PORT_CFG, 0xC6);

SFR(P1_DIR, 0xBA);
SFR(P1, PORT1);

SFR(P2_DIR, 0xBC);
SFR(P2, PORT2);

SFR(P3_DIR, 0xBE);
SFR(P3, PORT3);

/* mouse output pins */

// SBIT(MUP,    PORT2, 2);
// SBIT(MDOWN,  PORT2, 3);
// SBIT(MLEFT,  PORT2, 0);
// SBIT(MRIGHT, PORT2, 1);

SBIT(MX2, PORT2, 0); // DB9 pin 1 = X2
SBIT(MX1, PORT2, 1); // DB9 pin 2 = X1
SBIT(MY1, PORT2, 2); // DB9 pin 3 = Y1
SBIT(MY2, PORT2, 3); // DB9 pin 4 = Y2
SBIT(MBTN_LEFT, PORT2, 4);
SBIT(MBTN_RIGHT, PORT2, 6);

#endif