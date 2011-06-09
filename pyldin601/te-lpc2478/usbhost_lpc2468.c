/*
**************************************************************************************************************
*                                                 NXP USB Host Hardware Abstraction
*
*                                           for NXP LPC_24xx
*               programmed by Shir-Kuan Lin
*
**************************************************************************************************************
*/
 
/*
**************************************************************************************************************
*                                            INCLUDE HEADER FILES
**************************************************************************************************************
*/

#include "usbhost_inc.h"

/*
**************************************************************************************************************
*                                              GLOBAL VARIABLES
**************************************************************************************************************
*/
int gUSBConnected;

volatile  U32   HOST_RhscIntr = 0;         /* Root Hub Status Change interrupt                       */
volatile  U32   HOST_WdhIntr  = 0;         /* Semaphore to wait until the TD is submitted            */
volatile  U8    HOST_TDControlStatus = 0;

void  Host_Isr (void) __attribute__ ((interrupt("IRQ")));

/*
**************************************************************************************************************
*
**************************************************************************************************************
*/
void SystemInit (void)
{
}

/*
**************************************************************************************************************
*                                        Configure the GPIO of  THE HOST CONTROLLER
*
* Description: This function configures the GPIO of lpc17xx host controller
*
* Arguments  : None
*
* Returns    : 	HostBaseAddr = 0x20080000
*
**************************************************************************************************************
*/
U8* Host_GPIO_configuration (void)
{
    U32  pinsel;
    U32  pinmode;

    PCONP         |= 0x80000000;          /* Power on the usb*/
    VICIntEnable    = (1 << 22);           /* Enable the USB interrupt source */
    OTG_CLK_CTRL  = 0x1f;       /*enable the usbhost clock*/
    while (OTG_CLK_STAT != 0x1f);

    //OTG_STAT_CTRL &= 0xFFFFFFFD;	/* Set port func bits so that only U2 can be hosts. */
    OTG_STAT_CTRL = 0x01;

#if 0
    PCONP         |= Bit(31);
    VICIntEnClr    = (1 << 22);                               /* Enable the USB interrupt source            */
    OTG_CLK_CTRL   = 0x00000001;                              /* Enable USB host clock                      */

    while ((OTG_CLK_STAT & 0x00000001) == 0) {
        ;
    }
#endif

    pinsel    = PINSEL1;        /* P0[30] = USB_D-1 01  P0[29] = USB_D+1 01*/
    pinsel   &= 0xC3FFFFFF;
    pinsel   |= 0x18000000;
    PINSEL1   = pinsel;

     //===   Verify Host Controller and Allocate Resources (see Ch. 5.1.1.2 in OpenHCI) ========
    while ( (LPC_USB->HcRevision & 0xFF) != 0x10);		  /* It should be version 1.0  */

    PRINT_Log("======\nInitializing Host Stack\n");

   /*
    *   NOTE:  Endpoint Descriptors (HcED), Transfer Descriptors (HcTD),
     and Host Controller Communications Area (HcHCCA), as well as data buffer for TD (CBP in HcTD)
     should use the dedicated USB RAM (16kB, 0x7FD0 0000 - 0x7FD0 3FFF)
   */
    return ((U8*) 0x7FD00000);        //	HostBaseAddr = 0x7FD00000;
}


/*
**************************************************************************************************************
*                                       Enable the USB Interrupt of  THE HOST CONTROLLER
*
* Description: This function enables the USB Interrupt of lpc17xx host controller
*
* Arguments  : None
*
* Returns    : 	
*
**************************************************************************************************************
*/
void Host_IRQ_Init (void)
{
    //====== Install Interrupt Routines (see LPC2468 Technical Manual)  =======
    VICIntSelect &= ~(1 << 22);                        /* Configure the ISR handler: bit 22 = 0==> USB interrupt as IRQ category  (not FIQ) */
    VICVectAddr22 =  (U32)Host_Isr;                    /* Set the vector address                     */
    VICIntEnable  = (1 << 22);                         /* Enable the USB interrupt source: bit 22 ==> USB       */
}

/*
**************************************************************************************************************
*
**************************************************************************************************************
*/
volatile U32* HcRhPortStatus_Address(void)
{
    return &(p_HcRhPortStatus[1]);
}

/*
**************************************************************************************************************
*                                         INTERRUPT SERVICE ROUTINE
*
* Description: This function services the interrupt caused by host controller
*
* Arguments  : None
*
* Returns    : None
*
**************************************************************************************************************
*/

void  Host_Isr (void)
{

    if ((USB_INT_STAT & 0x00000008) > 0) {			 // bit 3 =  USB_HOST_INT  for USB OTG controller

	U32 int_status = LPC_USB->HcInterruptStatus;                          /* Read Interrupt Status                */
	U32 ie_status  = LPC_USB->HcInterruptEnable;                          /* Read Interrupt enable status         */

	int_status = int_status & ie_status;

	if (!int_status) {
	    goto exit;
	} else {

	    //====== Root Hub Status Change  =======
            if (int_status & OR_INTR_STATUS_RHSC) {                 /* Root hub status change interrupt (RHSC) = bit 6     */
		if (p_HcRhPortStatus[1] & OR_RH_PORT_CSC) {
		    if (LPC_USB->HcRhStatus & OR_RH_STATUS_DRWE) {
			/*
			 * When DRWE is on, Connect Status Change
			 * means a remote wakeup event.
			*/
//			HOST_RhscIntr = 1;// JUST SOMETHING FOR A BREAKPOINT
			PRINT_Log("remote wakeup event!\n");
		    } else {
			/*
			 * When DRWE is off, Connect Status Change
			 * is NOT a remote wakeup event
			*/
			if (p_HcRhPortStatus[1] & OR_RH_PORT_CCS) {
			    HOST_RhscIntr = 1;					 // bit 0 = CCS = 1: device connected
			    PRINT_Log("Device just Connected!\n");
			} else {
			    HOST_RhscIntr = 0;					 // bit 0 = CCS = 0: no device connected
			    PRINT_Log("\nDevice just Disconnected!!!\n");
			}
		    }
		    p_HcRhPortStatus[1] = OR_RH_PORT_CSC;	// clear CSC by wrinting '1'
		}
	    }
	    //====== Writeback Done Head  =======
	    if (int_status & OR_INTR_STATUS_WDH) {                 /* Writeback Done Head interrupt        */
		HOST_WdhIntr = 1;
//		HID_InputKeyboard();
	    }
	    //====== Resume Detected =======
	    if (int_status & OR_INTR_STATUS_RD) {                  /* Resume Detected interrupt        */
		;  // user's code
	    }
	}
        LPC_USB->HcInterruptStatus = int_status;                   /* Clear interrupt status register by wrinting '1'   */
    }
 exit:
    VICVectAddr = 0;
}
