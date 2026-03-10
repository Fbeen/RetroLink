#include "CH559.h"
#include "USBHost.h"
#include "hid_mouse.h"
#include "util.h"
#include <string.h>

/* Your parsers live elsewhere; keep the same signature you used before */
extern void parseMouseData(uint8_t hub, vendor_product_id_t *vp, unsigned char __xdata *report);
extern void parseJoystickData(uint8_t hub, vendor_product_id_t *vp, unsigned char __xdata *report);

/* ---------------- Setup requests ---------------- */

typedef const unsigned char __code *PUINT8C;

__code unsigned char GetDeviceDescriptorRequest[] =
{
    USB_REQ_TYP_IN, USB_GET_DESCRIPTOR,
    0x00, USB_DESCR_TYP_DEVICE,
    0x00, 0x00,
    sizeof(USB_DEV_DESCR), 0x00
};

__code unsigned char GetConfigurationDescriptorRequest[] =
{
    USB_REQ_TYP_IN, USB_GET_DESCRIPTOR,
    0x00, USB_DESCR_TYP_CONFIG,
    0x00, 0x00,
    /* first fetch only config header size */
    sizeof(USB_CFG_DESCR), 0x00
};

__code unsigned char SetUSBAddressRequest[] =
{
    USB_REQ_TYP_OUT, USB_SET_ADDRESS,
    USB_DEVICE_ADDR, 0x00,
    0x00, 0x00,
    0x00, 0x00
};

__code unsigned char SetupSetUsbConfig[] =
{
    USB_REQ_TYP_OUT, USB_SET_CONFIGURATION,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00
};

__code unsigned char SetHIDIdleRequest[] =
{
    USB_REQ_TYP_CLASS | USB_REQ_RECIP_INTERF, HID_SET_IDLE,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00
};

/* ---------------- Buffers ---------------- */

#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 64
#endif

__at(0x0000) unsigned char __xdata RxBuffer[MAX_PACKET_SIZE];
__at(0x0100) unsigned char __xdata TxBuffer[MAX_PACKET_SIZE];

#define RECEIVE_BUFFER_LEN 512
__xdata unsigned char receiveDataBuffer[RECEIVE_BUFFER_LEN];

/* ---------------- Minimal state ---------------- */

__xdata uint8_t endpoint0Size;

static root_hub_device_t rootHubDevice;
static hid_device_t hidDevice;
static vendor_product_id_t vendorProductID;

/* ---------------- Helpers ---------------- */

static void fillTxBuffer(PUINT8C data, unsigned char len)
{
    unsigned char i;
    for (i = 0; i < len; i++)
        TxBuffer[i] = data[i];
}

static void setHostUsbAddr(unsigned char addr)
{
    USB_DEV_AD = (USB_DEV_AD & bUDA_GP_BIT) | (addr & 0x7F);
}

static void setUsbSpeed(unsigned char fullSpeed)
{
    if (fullSpeed)
    {
        USB_CTRL &= ~bUC_LOW_SPEED;
        UH_SETUP &= ~bUH_PRE_PID_EN;
    }
    else
    {
        USB_CTRL |= bUC_LOW_SPEED;
    }
}

static void disableRootHubPort(void)
{
    rootHubDevice.status  = ROOT_DEVICE_DISCONNECT;
    rootHubDevice.address = 0;
    rootHubDevice.speed   = 1;

    UHUB0_CTRL = 0;
}

static void resetRootHubPort(void)
{
    endpoint0Size = DEFAULT_ENDP0_SIZE;
    setHostUsbAddr(0);
    setUsbSpeed(1);

    UHUB0_CTRL = (UHUB0_CTRL & ~bUH_LOW_SPEED) | bUH_BUS_RESET;
    delay(15);
    UHUB0_CTRL = (UHUB0_CTRL & ~bUH_BUS_RESET);

    delayUs(250);
    UIF_DETECT = 0;
}

static unsigned char enableRootHubPort(void)
{
    if (USB_HUB_ST & bUHS_H0_ATTACH)
    {
        if ((UHUB0_CTRL & bUH_PORT_EN) == 0x00)
        {
            if (USB_HUB_ST & bUHS_DM_LEVEL)
            {
                rootHubDevice.speed = 0;
                UHUB0_CTRL |= bUH_LOW_SPEED;
            }
            else
            {
                rootHubDevice.speed = 1;
                UHUB0_CTRL &= ~bUH_LOW_SPEED;
            }
        }
        UHUB0_CTRL |= bUH_PORT_EN;
        return ERR_SUCCESS;
    }
    return ERR_USB_DISCON;
}

static void selectHubPort(void)
{
    setHostUsbAddr(rootHubDevice.address);
    setUsbSpeed(rootHubDevice.speed);
}

/* ---------------- USB transfers ---------------- */

static unsigned char hostTransfer(unsigned char endp_pid, unsigned char tog, unsigned short timeout)
{
    unsigned short retries = 0;
    unsigned char  r;
    unsigned short i;

    UH_RX_CTRL = tog;
    UH_TX_CTRL = tog;

    do
    {
        UH_EP_PID = endp_pid;
        UIF_TRANSFER = 0;

        for (i = 200; i != 0 && UIF_TRANSFER == 0; i--)
            delayUs(1);

        UH_EP_PID = 0x00;

        if (UIF_TRANSFER == 0)
            return ERR_USB_UNKNOWN;

        if (U_TOG_OK)
            return ERR_SUCCESS;

        r = USB_INT_ST & MASK_UIS_H_RES;

        if (r == USB_PID_STALL)
            return (r | ERR_USB_TRANSFER);

        if (r == USB_PID_NAK)
        {
            if (timeout == 0)
                return (r | ERR_USB_TRANSFER);
            if (timeout < 0xFFFF)
                timeout--;
            /* keep retrying */
        }
        else
        {
            /* For most cases, treat other responses as transfer error */
            if (r == USB_PID_ACK)
                return ERR_SUCCESS;

            if (r == USB_PID_DATA0 || r == USB_PID_DATA1)
                return ERR_SUCCESS;

            if (r)
                return (r | ERR_USB_TRANSFER);
        }

        delayUs(15);
    }
    while (++retries < 200);

    return ERR_USB_TRANSFER;
}

static unsigned char hostCtrlTransfer(unsigned char __xdata *DataBuf, unsigned short *RetLen, unsigned short maxLength)
{
    unsigned short RemLen;
    unsigned char  s, RxLen, i;
    unsigned char __xdata *pBuf = DataBuf;

    PXUSB_SETUP_REQ pSetupReq = (PXUSB_SETUP_REQ)TxBuffer;

    delayUs(200);

    if (RetLen)
        *RetLen = 0;

    UH_TX_LEN = sizeof(USB_SETUP_REQ);
    s = hostTransfer((unsigned char)(USB_PID_SETUP << 4), 0, 10000);
    if (s != ERR_SUCCESS)
        return s;

    UH_RX_CTRL = UH_TX_CTRL = bUH_R_TOG | bUH_R_AUTO_TOG | bUH_T_TOG | bUH_T_AUTO_TOG;
    UH_TX_LEN = 0x01;

    RemLen = ((unsigned short)pSetupReq->wLengthH << 8) | pSetupReq->wLengthL;

    if (RemLen && pBuf)
    {
        if (pSetupReq->bRequestType & USB_REQ_TYP_IN)
        {
            while (RemLen)
            {
                s = hostTransfer((unsigned char)(USB_PID_IN << 4), UH_RX_CTRL, 10000);
                if (s != ERR_SUCCESS)
                    return s;

                RxLen = (USB_RX_LEN < RemLen) ? USB_RX_LEN : RemLen;
                RemLen -= RxLen;

                if (RetLen)
                    *RetLen += RxLen;

                for (i = 0; i < RxLen; i++)
                    pBuf[i] = RxBuffer[i];

                pBuf += RxLen;

                if (USB_RX_LEN == 0 || USB_RX_LEN < endpoint0Size)
                    break;
            }
            UH_TX_LEN = 0x00;
        }
        else
        {
            /* OUT data stage not used by our minimal stack (except rare class reqs),
               but keep structure intact. */
            while (RemLen)
            {
                UH_TX_LEN = (RemLen >= endpoint0Size) ? endpoint0Size : RemLen;
                s = hostTransfer((unsigned char)(USB_PID_OUT << 4), UH_TX_CTRL, 10000);
                if (s != ERR_SUCCESS)
                    return s;

                RemLen -= UH_TX_LEN;
                if (RetLen)
                    *RetLen += UH_TX_LEN;
            }
        }
    }

    s = hostTransfer((UH_TX_LEN ? (USB_PID_IN << 4) : (USB_PID_OUT << 4)),
                     bUH_R_TOG | bUH_T_TOG, 10000);
    if (s != ERR_SUCCESS)
        return s;

    if (UH_TX_LEN == 0 || USB_RX_LEN == 0)
        return ERR_SUCCESS;

    return ERR_USB_BUF_OVER;
}

/* ---------------- Enumeration steps ---------------- */

static unsigned char getDeviceDescriptor(void)
{
    unsigned char s;
    unsigned short len;

    endpoint0Size = DEFAULT_ENDP0_SIZE;

    fillTxBuffer(GetDeviceDescriptorRequest, sizeof(GetDeviceDescriptorRequest));
    s = hostCtrlTransfer(receiveDataBuffer, &len, RECEIVE_BUFFER_LEN);
    if (s != ERR_SUCCESS)
        return s;

    endpoint0Size = ((PXUSB_DEV_DESCR)receiveDataBuffer)->bMaxPacketSize0;

    /* Store VID/PID for your config table */
    vendorProductID.idVendorL  = ((PXUSB_DEV_DESCR)receiveDataBuffer)->idVendorL;
    vendorProductID.idVendorH  = ((PXUSB_DEV_DESCR)receiveDataBuffer)->idVendorH;
    vendorProductID.idProductL = ((PXUSB_DEV_DESCR)receiveDataBuffer)->idProductL;
    vendorProductID.idProductH = ((PXUSB_DEV_DESCR)receiveDataBuffer)->idProductH;

    return ERR_SUCCESS;
}

static unsigned char setUsbAddress(unsigned char addr)
{
    unsigned char s;
    PXUSB_SETUP_REQ pSetupReq = (PXUSB_SETUP_REQ)TxBuffer;

    fillTxBuffer(SetUSBAddressRequest, sizeof(SetUSBAddressRequest));
    pSetupReq->wValueL = addr;

    s = hostCtrlTransfer(0, 0, 0);
    if (s != ERR_SUCCESS)
        return s;

    setHostUsbAddr(addr);
    delay(100);

    return ERR_SUCCESS;
}

static unsigned char getConfigurationDescriptor(void)
{
    unsigned char s;
    unsigned short len;
    unsigned short total;

    /* 1) fetch config header */
    fillTxBuffer(GetConfigurationDescriptorRequest, sizeof(GetConfigurationDescriptorRequest));
    s = hostCtrlTransfer(receiveDataBuffer, &len, RECEIVE_BUFFER_LEN);
    if (s != ERR_SUCCESS)
        return s;

    total = (unsigned short)((PXUSB_CFG_DESCR)receiveDataBuffer)->wTotalLengthL |
            ((unsigned short)((PXUSB_CFG_DESCR)receiveDataBuffer)->wTotalLengthH << 8);

    if (total > RECEIVE_BUFFER_LEN)
        return ERR_USB_BUF_OVER;

    /* 2) fetch full config */
    fillTxBuffer(GetConfigurationDescriptorRequest, sizeof(GetConfigurationDescriptorRequest));
    ((PXUSB_SETUP_REQ)TxBuffer)->wLengthL = (unsigned char)(total & 0xFF);
    ((PXUSB_SETUP_REQ)TxBuffer)->wLengthH = (unsigned char)(total >> 8);

    s = hostCtrlTransfer(receiveDataBuffer, &len, RECEIVE_BUFFER_LEN);
    if (s != ERR_SUCCESS)
        return s;

    return ERR_SUCCESS;
}

static unsigned char setUsbConfig(unsigned char cfg)
{
    PXUSB_SETUP_REQ pSetupReq = (PXUSB_SETUP_REQ)TxBuffer;

    fillTxBuffer(SetupSetUsbConfig, sizeof(SetupSetUsbConfig));
    pSetupReq->wValueL = cfg;

    return hostCtrlTransfer(0, 0, 0);
}

static void setHidIdle(uint8_t interfaceNumber)
{
    /* Not fatal if this fails; many devices ignore it. */
    fillTxBuffer(SetHIDIdleRequest, sizeof(SetHIDIdleRequest));
    ((PXUSB_SETUP_REQ)TxBuffer)->wIndexL = interfaceNumber;
    (void)hostCtrlTransfer(0, 0, 0);
}

/* Find first HID interface + first interrupt IN endpoint */
static unsigned char bindFirstHidInterface(void)
{
    unsigned short total;
    unsigned short i;

    PXUSB_CFG_DESCR cfg = (PXUSB_CFG_DESCR)receiveDataBuffer;
    total = (unsigned short)cfg->wTotalLengthL | ((unsigned short)cfg->wTotalLengthH << 8);

    PXUSB_ITF_DESCR currentInterface = 0;

    hidDevice.connected = 0;
    hidDevice.type      = HID_TYPE_UNKNOWN;
    hidDevice.endpoint  = 0;
    hidDevice.interface = 0;
    hidDevice.toggle    = 0;

    i = cfg->bLength;

    while (i < total)
    {
        unsigned char __xdata *desc = &receiveDataBuffer[i];

        if (desc[0] == 0) /* safety */
            break;

        switch (desc[1])
        {
            case USB_DESCR_TYP_INTERF:
            {
                currentInterface = (PXUSB_ITF_DESCR)desc;

                if (currentInterface->bInterfaceClass == USB_DEV_CLASS_HID)
                {
                    /* Protocol: 2 = mouse, 1 = keyboard, 0 = none (joystick/gamepad often uses 0) */
                    if (currentInterface->bInterfaceProtocol == 2)
                        hidDevice.type = HID_TYPE_MOUSE;
                    else
                        hidDevice.type = HID_TYPE_JOYSTICK;

                    hidDevice.interface = currentInterface->bInterfaceNumber;
                }
                break;
            }

            case USB_DESCR_TYP_ENDP:
            {
                if (currentInterface && currentInterface->bInterfaceClass == USB_DEV_CLASS_HID)
                {
                    PXUSB_ENDP_DESCR ep = (PXUSB_ENDP_DESCR)desc;

                    /* We want Interrupt IN endpoint */
                    if ((ep->bEndpointAddress & 0x80) && ((ep->bmAttributes & 0x03) == 0x03))
                    {
                        hidDevice.endpoint  = ep->bEndpointAddress;
                        hidDevice.connected = 1;
                        hidDevice.toggle    = 0;

                        setHidIdle(hidDevice.interface);
                        return ERR_SUCCESS;
                    }
                }
                break;
            }

            default:
                break;
        }

        i += desc[0];
    }

    return ERR_USB_UNSUPPORT;
}

/* ---------------- Public API ---------------- */

void resetHubDevices(uint8_t hubindex)
{
    /* keep signature; only hubindex==0 exists */
    (void)hubindex;

    vendorProductID.idVendorL  = 0;
    vendorProductID.idVendorH  = 0;
    vendorProductID.idProductL = 0;
    vendorProductID.idProductH = 0;

    hidDevice.connected = 0;
    hidDevice.type      = HID_TYPE_UNKNOWN;
    hidDevice.endpoint  = 0;
    hidDevice.interface = 0;
    hidDevice.toggle    = 0;
}

void initUSB_Host(void)
{
    IE_USB = 0;

    USB_CTRL   = bUC_HOST_MODE;
    USB_DEV_AD = 0x00;

    UH_EP_MOD  = bUH_EP_TX_EN | bUH_EP_RX_EN;
    UH_RX_DMA  = 0x0000;
    UH_TX_DMA  = 0x0001;

    UH_RX_CTRL = 0x00;
    UH_TX_CTRL = 0x00;

    USB_CTRL   = bUC_HOST_MODE | bUC_INT_BUSY | bUC_DMA_EN;
    UH_SETUP   = bUH_SOF_EN;

    USB_INT_FG = 0xFF;

    disableRootHubPort();
    resetHubDevices(0);

    USB_INT_EN = bUIE_TRANSFER | bUIE_DETECT;
}

static unsigned char initializeRootHubConnection(void)
{
    unsigned char s;
    unsigned char cfg;
    unsigned char addr;
    unsigned char i;
    unsigned char retry;

    for (retry = 0; retry < 5; retry++)
    {
        delay(50);

        resetHubDevices(0);
        resetRootHubPort();

        /* enable port */
        for (i = 0; i < 100; i++)
        {
            delay(1);
            if (enableRootHubPort() == ERR_SUCCESS)
                break;
        }
        if (i == 100)
        {
            disableRootHubPort();
            continue;
        }

        selectHubPort();

        s = getDeviceDescriptor();
        if (s != ERR_SUCCESS)
            continue;

        /* address 1 is enough (single device) */
        addr = 1;
        s = setUsbAddress(addr);
        if (s != ERR_SUCCESS)
            continue;

        rootHubDevice.address = addr;

        s = getConfigurationDescriptor();
        if (s != ERR_SUCCESS)
            continue;

        cfg = ((PXUSB_CFG_DESCR)receiveDataBuffer)->bConfigurationValue;

        s = setUsbConfig(cfg);
        if (s != ERR_SUCCESS)
            continue;

        s = bindFirstHidInterface();
        if (s != ERR_SUCCESS)
            continue;

        rootHubDevice.status = ROOT_DEVICE_SUCCESS;
        return ERR_SUCCESS;
    }

    rootHubDevice.status = ROOT_DEVICE_FAILED;
    return ERR_USB_UNKNOWN;
}

uint8_t checkRootHubConnections(void)
{
    unsigned char s = ERR_SUCCESS;

    if (!UIF_DETECT)
        return ERR_SUCCESS;

    UIF_DETECT = 0;

    if (USB_HUB_ST & bUHS_H0_ATTACH)
    {
        if (rootHubDevice.status == ROOT_DEVICE_DISCONNECT || (UHUB0_CTRL & bUH_PORT_EN) == 0x00)
        {
            disableRootHubPort();
            rootHubDevice.status = ROOT_DEVICE_CONNECTED;
            s = initializeRootHubConnection();
        }
    }
    else
    {
        if (rootHubDevice.status >= ROOT_DEVICE_CONNECTED)
        {
            resetHubDevices(0);
            disableRootHubPort();
            s = ERR_USB_DISCON;
        }
    }

    return s;
}

void pollHIDdevice(void)
{
    unsigned char s;
    unsigned char len;

    if (!hidDevice.connected)
        return;

    selectHubPort();

    s = hostTransfer(
            (USB_PID_IN << 4) | (hidDevice.endpoint & 0x7F),
            hidDevice.toggle ? (bUH_R_TOG | bUH_T_TOG) : 0,
            0);

    if (s != ERR_SUCCESS)
        return;

    hidDevice.toggle ^= 1;

    len = USB_RX_LEN;
    if (!len)
        return;

    if (hidDevice.type == HID_TYPE_MOUSE)
    {
        hid_mouse_parse(RxBuffer);
    }
    else if (hidDevice.type == HID_TYPE_JOYSTICK)
    {
        parseJoystickData(0, &vendorProductID, RxBuffer);
    }
}