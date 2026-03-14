#include "hardware.h"

void hw_setup(void)
{
    // port setup
    PORT_CFG = 0b00101101;

    /* first all outputs safe to HIGH */
    P2 = 0xFF;

    /* force bit5 LOW */
    P2 &= ~(1 << 5);

    /* and then configure them to output */
    P2_DIR = 0b01111111;
}