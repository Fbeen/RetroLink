#include "hardware.h"

void hw_setup(void)
{
    PORT_CFG |= (1 << 2);  // PORT2 open drain
    P2 = 0xFF;             // alles HIGH-Z
    P2_DIR = 0xFF;         // outputs enabled
}