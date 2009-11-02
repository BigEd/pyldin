#include "ps2func.h"

extern void 	cdvd_irx;
extern int  	size_cdvd_irx;

extern void 	iomanX_irx;
extern int  	size_iomanX_irx;

extern void 	usb_mass_irx;
extern int  	size_usb_mass_irx;

void systemInit(void)
{
    int ret;

    SifExecModuleBuffer(&iomanX_irx, size_iomanX_irx, 0, NULL, &ret);
    SifExecModuleBuffer(&cdvd_irx, size_cdvd_irx, 0, NULL, &ret);
}

void usbDriveInit(void)
{
    int ret;
    SifExecModuleBuffer(&usb_mass_irx, size_usb_mass_irx, 0, NULL, &ret);
}
