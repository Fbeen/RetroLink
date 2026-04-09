#include "hardware.h"

#ifdef BETA_VERSION

/*
 * Initialize hardware configuration:
 * - Configure PORT2 as open-drain for DB9 signals
 * - Set all PORT2 pins to released state (high-Z)
 * - Configure P0.4 as push-pull output for status LED
 * - Turn LED off at startup
 */
void hw_setup(void)
{
    /* PORT2: open-drain for DB9 */
    PORT_CFG |= (1 << 2);
    P2 = 0xFF;
    P2_DIR = 0xFF;

    /* P0.4: status LED */
    PORT_CFG &= ~(1 << 0);   // PORT0 push-pull
    P0_DIR |= (1 << 4);      // P0.4 output

    LED_STATUS = 0;
}

#else

/*
 * Initialize hardware configuration:
 * - Configure PORT2 as open-drain for DB9 signals
 * - Set all PORT2 pins to released state (high-Z)
 * - Configure P4.3 as push-pull output for status LED
 * - Turn LED off at startup
 */
void hw_setup(void)
{
    /* PORT2: open-drain for DB9 */
    PORT_CFG |= (1 << 2);
    P2 = 0xFF;
    P2_DIR = 0xFF;

    /* P4.3: status LED */
    P4_DIR |= (1 << 3);      // P4.3 output

    LED_STATUS = 0;
}

#endif