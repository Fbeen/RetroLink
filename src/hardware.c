#include "hardware.h"

void hw_setup(void)
{
    PORT_CFG |= (1 << 2);  // PORT2 open drain
    P2 = 0xFF;             // alles HIGH-Z
    P2_DIR = 0xFF;         // outputs enabled

    /* P0 push-pull for status LED */
    PORT_CFG &= ~(1 << 0);    // P0 not open-drain
    P0_DIR |= (1 << 4);       // P0.4 output
    P0_PU  |= (1 << 4);       // optional, harmless here

    LED_STATUS = 0;           // LED off
}