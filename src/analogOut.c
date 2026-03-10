
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "CH559.h"
#include "util.h"
#include "analogOut.h"

#define GPIO1 0x90
#define GPIO2 0xA0
#define GPIO3 0xBE

#define YS_LEFT     0x02
#define YS_RIGHT    0x03
#define YS_UP       0x00
#define YS_DOWN     0x01
#define YS_BTN1     0x04
#define YS_BTN2     0x06

struct joystickButtons {
    unsigned char idVendorH;    // Vendor High byte
    unsigned char idVendorL;    // Vendor Low byte
    unsigned char idProductH;   // Product High byte
    unsigned char idProductL;   // Product Low byte

    unsigned char xPos;     // nth byte from data keeping x-as, counting from zero!
    unsigned char yPos;     // nth byte from data keeping y-as, counting from zero!
    unsigned char bPos;     // nth byte from data keeping buttons, counting from zero!

    unsigned char left;     // left value e.g 0x01, 0xFF, 0x02, 0x04 etc
    unsigned char right;    // right value
    unsigned char up;       // up value
    unsigned char down;     // down value
    unsigned char btn1;     // button 1 value
    unsigned char btn2;     // button 2 value
    unsigned char autoFire; // autoFire value
    unsigned char btnUp;    // btnUp value
};

#define JOYSTICKS 1

struct joystickButtons joystick[] = {
    // Generic USB Joystick
    {
        0x00,   // Vendor High byte     (You will be able to find your VendorId and ProductId with Debugging on)
        0x79,   // Vendor Low byte
        0x00,   // Product High byte
        0x06,   // Product Low byte

        0,      // nth byte from data keeping x-as, counting from zero!     (You will be able to find your data with Debugging on and pressing the buttons)
        1,      // nth byte from data keeping y-as, counting from zero!
        5,      // nth byte from data keeping buttons, counting from zero!

        0x00,   // left value           (e.g 0x01, 0xFF, 0x02, 0x04 etc)
        0xFF,   // right value
        0x00,   // up value
        0xFF,   // down value
        0x2F,   // button 1 value
        0x4F,   // button 2 value
        0x8F,   // autoFire value
        0x1F    // btnUp value
    } /* ,
    {
        ... // add more
    } */
};

SBIT(GP2MS1, GPIO2, YS_BTN1);
SBIT(GP2MS2, GPIO2, YS_BTN2);
SBIT(GP3MS1, GPIO3, YS_BTN1);
SBIT(GP3MS2, GPIO3, YS_BTN2);


void parseJoystickData(unsigned char port, struct VendorProductID* vendorProductID, unsigned char* RxBuffer) {
    for(int j = 0 ; j < JOYSTICKS ; j++) { // loop through all our joystick configs
        if(joystick[j].idVendorL == vendorProductID->idVendorL &&
           joystick[j].idVendorH == vendorProductID->idVendorH &&
           joystick[j].idProductL == vendorProductID->idProductL &&
           joystick[j].idProductH == vendorProductID->idProductH )
        {
            unsigned char gpio = 0;
            if(RxBuffer[joystick[j].bPos] == joystick[j].btn1) {
                gpio |= 1 << YS_BTN1;
            } else if(RxBuffer[joystick[j].bPos] == joystick[j].autoFire) {
                // todo
            }

            if(RxBuffer[joystick[j].bPos] == joystick[j].btn2) {
                gpio |= 1 << YS_BTN2;
            }

            if(RxBuffer[joystick[j].xPos] == joystick[j].left) {
                gpio |= 1 << YS_LEFT;
            } else if(RxBuffer[joystick[j].xPos] == joystick[j].right) {
                gpio |= 1 << YS_RIGHT;
            }

            if(RxBuffer[joystick[j].yPos] == joystick[j].up || RxBuffer[joystick[j].bPos] == joystick[j].btnUp) {
                gpio |= 1 << YS_UP;
            } else if(RxBuffer[joystick[j].yPos] == joystick[j].down) {
                gpio |= 1 << YS_DOWN;
            }

            if(port == 0) {
                P2 = gpio;
            } else {
                P3 = gpio;
            }
        }
    }
}
