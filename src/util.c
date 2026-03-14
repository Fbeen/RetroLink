#include "CH559.h"
#include "util.h"

FunctionReference runBootloader = (FunctionReference)0xF400;

#ifndef FREQ_SYS
#define	FREQ_SYS	48000000
#endif 

/*
 * Initialize system clock.
 * Sets CPU clock to 48 MHz via PLL (required for USB).
 */
void initClock()
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;

	CLOCK_CFG &= ~MASK_SYS_CK_DIV;
	CLOCK_CFG |= 6; 															  
	PLL_CFG = ((24 << 0) | (6 << 5)) & 255;

    SAFE_MOD = 0xFF;

	delay(7);
}

/**
 * Initialize UART0 port with given boud rate
 * pins: tx = P3.1 rx = P3.0
 * alt != 0 pins: tx = P0.2 rx = P0.3
 */
void initUART0(unsigned long baud, int alt)
{
	unsigned long x;
	if(alt)
	{
		PORT_CFG |= bP0_OC;
		P0_DIR |= bTXD_;
		P0_PU |= bTXD_ | bRXD_;
		PIN_FUNC |= bUART0_PIN_X;
	}

 	SM0 = 0;
	SM1 = 1;
	SM2 = 0;
	REN = 1;
   //RCLK = 0;
    //TCLK = 0;
    PCON |= SMOD;
    x = (((unsigned long)FREQ_SYS / 8) / baud + 1) / 2;

    TMOD = TMOD & ~ bT1_GATE & ~ bT1_CT & ~ MASK_T1_MOD | bT1_M1;
    T2MOD = T2MOD | bTMR_CLK | bT1_CLK;
    TH1 = (256 - x) & 255;
    TR1 = 1;
	TI = 1;
}

/* non blocking function that looks if there is a character available */
char UART0Available(void)
{
    return RI;
}

unsigned char UART0Receive()
{
    while(RI == 0);
    RI = 0;
    return SBUF;
}

void UART0Send(unsigned char b)
{
    TI = 0;          // clear flag
    SBUF = b;        // start transmission
    while(TI == 0);  // wait until finished
}

/**
* #define PIN_MODE_INPUT 0
* #define PIN_MODE_INPUT_PULLUP 1
* #define PIN_MODE_OUTPUT 2
* #define PIN_MODE_OUTPUT_OPEN_DRAIN 3
* #define PIN_MODE_OUTPUT_OPEN_DRAIN_2CLK 4
* #define PIN_MODE_INPUT_OUTPUT_PULLUP 5
* #define PIN_MODE_INPUT_OUTPUT_PULLUP_2CLK 6
 */
void pinMode(unsigned char port, unsigned char pin, unsigned char mode)
{
	volatile unsigned char *dir[] = {&P0_DIR, &P1_DIR, &P2_DIR, &P3_DIR};
	volatile unsigned char *pu[] = {&P0_PU, &P1_PU, &P2_PU, &P3_PU};
	switch (mode)
	{
	case PIN_MODE_INPUT: //Input only, no pull up
		PORT_CFG &= ~(bP0_OC << port);
		*dir[port] &= ~(1 << pin);
		*pu[port] &= ~(1 << pin);
		break;
	case PIN_MODE_INPUT_PULLUP: //Input only, pull up
		PORT_CFG &= ~(bP0_OC << port);
		*dir[port] &= ~(1 << pin);
		*pu[port] |= 1 << pin;
		break;
	case PIN_MODE_OUTPUT: //Push-pull output, high and low level strong drive
		PORT_CFG &= ~(bP0_OC << port);
		*dir[port] |= ~(1 << pin);
		break;
	case PIN_MODE_OUTPUT_OPEN_DRAIN: //Open drain output, no pull-up, support input
		PORT_CFG |= (bP0_OC << port);
		*dir[port] &= ~(1 << pin);
		*pu[port] &= ~(1 << pin);
		break;
	case PIN_MODE_OUTPUT_OPEN_DRAIN_2CLK: //Open-drain output, no pull-up, only drives 2 clocks high when the transition output goes from low to high
		PORT_CFG |= (bP0_OC << port);
		*dir[port] |= 1 << pin;
		*pu[port] &= ~(1 << pin);
		break;
	case PIN_MODE_INPUT_OUTPUT_PULLUP: //Weakly bidirectional (standard 51 mode), open drain output, with pull-up
		PORT_CFG |= (bP0_OC << port);
		*dir[port] &= ~(1 << pin);
		*pu[port] |= 1 << pin;
		break;
	case PIN_MODE_INPUT_OUTPUT_PULLUP_2CLK: //Quasi-bidirectional (standard 51 mode), open-drain output, with pull-up, when the transition output is low to high, only drives 2 clocks high
		PORT_CFG |= (bP0_OC << port);
		*dir[port] |= 1 << pin;
		*pu[port] |= 1 << pin;
		break;
	default:
		break;
	}
}
/*
unsigned char getPortAddress(unsigned char port)
{
	const unsigned char portAddr[] = {0x80, 0x90, 0xA0, 0xB0, 0xC0};
	return portAddr[port];
}

unsigned char digitalRead(unsigned char port, unsigned char pin)
{

}
*/
/**
 * stdio printf directed to UART0 using putchar and getchar
 */

int putchar(int c)
{
    while (!TI);
    TI = 0;
    SBUF = c & 0xFF;
    return c;
}

int getchar() 
{
    while(!RI);
    RI = 0;
    return SBUF;
}

/*******************************************************************************
* Function Name  : delayUs(UNIT16 n)
* Description    : us
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/ 
void	delayUs(unsigned short n)
{
	while (n) 
	{  // total = 12~13 Fsys cycles, 1uS @Fsys=12MHz
		++ SAFE_MOD;  // 2 Fsys cycles, for higher Fsys, add operation here
#ifdef	FREQ_SYS
#if		FREQ_SYS >= 14000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 16000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 18000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 20000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 22000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 24000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 26000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 28000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 30000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 32000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 34000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 36000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 38000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 40000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 42000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 44000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 46000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 48000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 50000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 52000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 54000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 56000000
		++ SAFE_MOD;
#endif
#endif
		--n;
	}
}

/*******************************************************************************
* Function Name  : delay(UNIT16 n)
* Description    : ms
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/
void delay(unsigned short n)
{
	while (n) 
	{
		delayUs(1000);
		--n;
	}
}

void print_u8(uint8_t v)
{
    uint8_t hundreds = v / 100;
    uint8_t tens     = (v / 10) % 10;
    uint8_t ones     = v % 10;

    if(hundreds)
        UART0Send('0' + hundreds);

    if(hundreds || tens)
        UART0Send('0' + tens);

    UART0Send('0' + ones);
}

void print_str(const char *s)
{
    while(*s)
    {
        UART0Send(*s++);
    }
}

static const char __code hex_digits[] = "0123456789ABCDEF";

void print_hex8(uint8_t v)
{
    UART0Send(hex_digits[(v >> 4) & 0x0F]);
    UART0Send(hex_digits[v & 0x0F]);
}

void debug_print_report(uint8_t *r, uint8_t len)
{
    uint8_t i;

    if(len > 16)
        len = 16;

    for(i = 0; i < len; i++)
    {
        UART0Send(' ');
        print_hex8(r[i]);
    }

    UART0Send('\r');
    UART0Send('\n');
}
