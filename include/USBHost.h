#ifndef __USBHOST_H__
#define __USBHOST_H__

#include <stdint.h>
#include "util.h"

#define ROOT_HUB_COUNT 2
#define MAX_HID_DEVICES 4

/* ================= USB error codes ================= */

#define ERR_SUCCESS         0x00
#define ERR_USB_CONNECT     0x15
#define ERR_USB_DISCON      0x16
#define ERR_USB_BUF_OVER    0x17
#define ERR_USB_TRANSFER    0x20
#define ERR_USB_UNKNOWN     0xFE
#define ERR_USB_UNSUPPORT   0xFB

/* ================= Device state ================= */

#define ROOT_DEVICE_DISCONNECT  0
#define ROOT_DEVICE_CONNECTED   1
#define ROOT_DEVICE_FAILED      2
#define ROOT_DEVICE_SUCCESS     3

/* ================= HID device types ================= */

#define HID_TYPE_UNKNOWN   0
#define HID_TYPE_MOUSE     1
#define HID_TYPE_JOYSTICK  2

/* ================= Vendor product id ================= */

typedef struct
{
    unsigned char idVendorL;
    unsigned char idVendorH;
    unsigned char idProductL;
    unsigned char idProductH;

} vendor_product_id_t;

/* ================= Root hub device ================= */

typedef struct
{
    uint8_t status;
    uint8_t address;
    uint8_t speed;

} root_hub_device_t;

/* ================= HID device ================= */

typedef struct
{
    uint8_t connected;

    uint8_t rootHub;

    uint8_t interface;

    uint8_t endpoint;

    uint8_t toggle;

    uint8_t type;        // mouse / joystick

} hid_device_t;

/* ================= Vendor/Product ================= */

typedef struct
{
    uint16_t vendor;
    uint16_t product;

} usb_vid_pid_t;

/* ================= API ================= */

void initUSB_Host(void);

static unsigned char initializeRootHubConnection(void);

uint8_t checkRootHubConnections(void);

void pollHIDdevice(void);

void resetHubDevices(uint8_t hub);

static void setControllerMode(controller_mode_t mode);

controller_mode_t USBHost_getControllerMode(void);

#endif