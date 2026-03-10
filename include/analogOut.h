#ifndef __analogOut_H__
#define __analogOut_H__

struct VendorProductID
{
    unsigned long idVendorH;
    unsigned long idVendorL;
    unsigned long idProductH;
    unsigned long idProductL;
};

void parseJoystickData(unsigned char port, struct VendorProductID* vendorProductID, unsigned char* RxBuffer);

#endif
