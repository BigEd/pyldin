/*
**************************************************************************************************************
*                                                 NXP USB Host Stack
*															 For HID
*               V1.2 (2010.07.22)
*
*               programmed by Shir-Kuan Lin	 (Hsinchu City, TAIWAN)
*
* No Warranty for any Loss
**************************************************************************************************************
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * THE PROGRAMMER SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
**************************************************************************************************************
*/
 
/*
**************************************************************************************************************
*                                            INCLUDE HEADER FILES
**************************************************************************************************************
*/

#include "usbhost_inc.h"
#include "usb.h"
#include "usb_create.h"

#define  OK                        0
#define  ERR_TD_FAIL              -1

#define ED_EOF   0xFF
#define sKip     Bit(14)
/*
**************************************************************************************************************
*                                              GLOBAL VARIABLES
**************************************************************************************************************
*/
volatile  HCED        *p_EDCtrl;                    /* Control endpoint descriptor structure                  */
volatile  HCED        *p_EDBulkIn;                  /* BulkIn endpoint descriptor  structure                  */
volatile  HCED        *p_EDBulkOut;                 /* BulkOut endpoint descriptor structure                  */
volatile  HCTD        *p_TDHead;                    /* Head transfer descriptor structure                     */
volatile  HCTD        *p_TDTail;                    /* Tail transfer descriptor structure                     */
volatile  HCCA        *p_Hcca;                      /* Host Controller Communications Area structure          */ 
volatile  U8  		   *p_TDBuffer;                  /* Current Buffer Pointer of transfer descriptor          */

U8   *p_HCdata;
volatile  HCED  *p_StaticHcED;		 // 16 * 31 = 496 bytes
U32 HcEDlist[65];    // totally 65 ED lists
HCED_LIST *p_HcEDlist=(HCED_LIST*) HcEDlist;    // totally 65 ED lists

/*
**************************************************************************************************************
*                                         DELAY IN MILLI SECONDS
*
* Description: This function provides a delay in milli seconds
*
* Arguments  : delay    The delay required
*
* Returns    : None
*
**************************************************************************************************************
*/

void  Host_DelayMS (U32  delay)
{
    volatile  U32  i;


    for (i = 0; i < delay; i++) {
        Host_DelayUS(1000);
    }
}

/*
**************************************************************************************************************
*                                         DELAY IN MICRO SECONDS
*
* Description: This function provides a delay in micro seconds
*
* Arguments  : delay    The delay required
*
* Returns    : None
*
**************************************************************************************************************
*/

void  Host_DelayUS (U32  delay)
{
    volatile  U32  i;


    for (i = 0; i < (4 * delay); i++) {    /* This logic was tested. It gives app. 1 micro sec delay        */
        ;
    }
}



/*
**************************************************************************************************************
*    p_HcEDlist[0]-[30] for p_StaticHcED[0]-[30]
*    p_HcEDlist[31]-[62] for HccaIntTable[0]-[31]
*    p_HcEDlist[63] for HcEDCtrl
*    p_HcEDlist[64] for HcEDBulk
**************************************************************************************************************
*/
void Init_HcEDlist(void) {
  U32 *p_list, *p_hced;
  U32 i, j;

   p_list = (U32*) p_HcEDlist;
   p_hced = (U32*) p_StaticHcED;
   for (i=0; i<31; i++) {
	  p_list[i] = 0;
	  p_hced[(i<<2)+0] = 0;
	  p_hced[(i<<2)+1] = 0;
	  p_hced[(i<<2)+2] = 0;
	  p_hced[(i<<2)+3] = 0;
	  p_StaticHcED[i].Control |= sKip;
	  if (i==0) {
		 p_HcEDlist[i].NextIndex = ED_EOF;
		 p_StaticHcED[i].Next = 0;
	  } else {
		 j = (i-1)/2;                    // e.g., [3] and [4] are followed by [1]
		 p_HcEDlist[i].NextIndex = j;    // e.g., [3] and [4] are followed by [1]
		 p_StaticHcED[i].Next = (U32) &p_StaticHcED[j];
	  }
   }

   for ( i=0; i<25; i+=	0x8) {
    U32 k;
	static U8 balance[8] = { 15, 23, 19, 27, 17, 25, 21, 29};
	 for (k = 0; k<8; k++) {
       j = balance[k]+ ( (i&Bit(3))>>3 );		// i.3 =1 ==> +1, otherwise +0
       p_Hcca->HccaIntTable[k+i] = (U32) &p_StaticHcED[j];
	   p_list[(k+i)+31] = 0;
	   p_HcEDlist[(k+i)+31].NextIndex = j;    // e.g., [31] and [31+16] are followed by [15]
	 }
// i = 0, or 0x10
//       p_Hcca->HccaIntTable[0x00+i] = (U32) &p_StaticHcED[15];
//     p_Hcca->HccaIntTable[0x08+i] = (U32) &p_StaticHcED[16];
//       p_Hcca->HccaIntTable[0x04+i] = (U32) &p_StaticHcED[17];
//     p_Hcca->HccaIntTable[0x0C+i] = (U32) &p_StaticHcED[18];
//       p_Hcca->HccaIntTable[0x02+i] = (U32) &p_StaticHcED[19];
//     p_Hcca->HccaIntTable[0x0A+i] = (U32) &p_StaticHcED[20];
//      p_Hcca->HccaIntTable[0x06+i] = (U32) &p_StaticHcED[21];
//     p_Hcca->HccaIntTable[0x0E+i] = (U32) &p_StaticHcED[22];
//      p_Hcca->HccaIntTable[0x01+i] = (U32) &p_StaticHcED[23];
//     p_Hcca->HccaIntTable[0x09+i] = (U32) &p_StaticHcED[24];
//      p_Hcca->HccaIntTable[0x05+i] = (U32) &p_StaticHcED[25];
//     p_Hcca->HccaIntTable[0x0D+i] = (U32) &p_StaticHcED[26];
//      p_Hcca->HccaIntTable[0x03+i] = (U32) &p_StaticHcED[27];
//     p_Hcca->HccaIntTable[0x0B+i] = (U32) &p_StaticHcED[28];
//      p_Hcca->HccaIntTable[0x07+i] = (U32) &p_StaticHcED[29];
//     p_Hcca->HccaIntTable[0x0F+i] = (U32) &p_StaticHcED[30];
   }
}

/*
**************************************************************************************************************
 *   list must be 31, 15, 7, 3, 1
**************************************************************************************************************
*/
U32 CheckBandwidth (U32 list, U32* p_bBestList) {
  U32 worst_bd, best_bd, endList;

    worst_bd = 0;
	best_bd	= 0xFFFFFFFF;
	endList = list*2;
	for (; list <= endList; list++) {
	  U32 bd, i;
	   
	   bd = 0;
	   for (i=list; i!=ED_EOF; i = p_HcEDlist[i].NextIndex) {
	       bd += p_HcEDlist[i].Bandwidth;
	   } 
	
	   if (bd < best_bd) {
	       best_bd = bd;
		   *p_bBestList = list;
	   }
	   if (bd > worst_bd) {
	       worst_bd = bd;
	   }
	}  // END OF  for (; list <= endlist; list++)
    return worst_bd;
}


/*
**************************************************************************************************************
**************************************************************************************************************
*/
U32 MaxBandwidthInUse, AvailableBandwidth;
BOOL Open_Pipe(U32 bInterval, U32 bd) {
  U32 whichList, junk, oldValue;

    oldValue = MaxBandwidthInUse;
	if (bInterval <=1) {
		whichList = 0;
	    p_HcEDlist[0].Bandwidth += bd;
		MaxBandwidthInUse += bd;
	}else {
		whichList = 31;
//        while (whichList >= bInterval && (whichList >>=1) )
//		       continue;
		if (bInterval < 32) {
			whichList = 15;
			if (bInterval < 16) {
			    whichList = 7;
			    if (bInterval < 8) {
			       whichList = 3;
			       if (bInterval < 4) {
					 whichList = 1;
			       }
			    }
			}
		}
		CheckBandwidth(whichList, &whichList);
		p_HcEDlist[whichList].Bandwidth += bd;

		MaxBandwidthInUse = CheckBandwidth(31, &junk);   // 31 is top list  of the list sset for 32 ms.
	}   // END OF else 

  //<<<<<--------------
    if (MaxBandwidthInUse > AvailableBandwidth) {
	  // over available bandwidth, reject this request
		p_HcEDlist[whichList].Bandwidth -= bd;
		MaxBandwidthInUse = oldValue;
		return FALSE;   // cannot_commit_bandwidth	
	}

	//Endpoint->ListIndex = WhichList;
	//InsetEDforEndpoint(endpoint);
	return TRUE;
}

/*
**************************************************************************************************************
*                                         INITIALIZE THE HOST CONTROLLER
*
* Description: This function initializes lpc17xx host controller
*
* Arguments  : None
*
* Returns    : 
*
**************************************************************************************************************
*/
void  Host_Init (void)
{
	U8   *pD;


	pD = Host_GPIO_configuration();		   	// pD = HostBaseAddr;

   /*
	*   NOTE:  Endpoint Descriptors (HcED), Transfer Descriptors (HcTD),
	           and Host Controller Communications Area (HcHCCA), as well as data buffer for TD (CBP in HcTD)
			     should use the dedicated USB RAM (16kB, 0x2008 0000 - 0x2008 3FFF for LPC17xx;  0x7FD0 0000 - 0x7FD0 3FFF for LPC24xx)
		                                 AHB SRAM - bank 1 (16 kB), present on devices with 64 kB of total SRAM.
   */

    p_Hcca  = (volatile  HCCA       *) pD;	  // (256-byte boundary) 256 Bytes space for HC Communications Area (HCCA) 	
	pD += 256;
	p_StaticHcED = (volatile  HCED *) pD;
	pD += (31 * sizeof(HCED) ); 		   // align to 512 boundary

	p_TDHead     = (volatile  HCTD  *) pD;	  // (16-byte boundary) 16-Byte General Transfer Descriptor (HCTD) 
    pD += 32;
	p_TDTail     = (volatile  HCTD  *) pD;
    pD += 32;
    p_EDCtrl     = (volatile  HCED  *) pD;	  // (16-byte boundary) 16-Byte Endpoint Descriptor (HCED)  
    pD += 32;
    p_EDBulkIn   = (volatile  HCED  *) pD;
    pD += 32;
    p_EDBulkOut  = (volatile  HCED  *) pD;
    pD += 32;
    p_TDBuffer   = (volatile  U8 *) pD; 	  // must align to 2-byte boundary; 8 HCTD buffers (16*16 = 256 bytes)
    pD += 512;

	p_HCdata = pD;


                                                              /* Initialize all the TDs, EDs and HCCA to 0  */
    Host_EDInit(p_EDCtrl);
    Host_EDInit(p_EDBulkIn);
    Host_EDInit(p_EDBulkOut);
    Host_TDInit(p_TDHead);
    Host_TDInit(p_TDTail);
    Host_HCCAInit(p_Hcca);

    
    Init_HcEDlist();
   //<<<<<<<<<<=========

//    Host_DelayMS(50);                                         /* Wait 50 ms before apply reset              */
      //===   Initialize HC Operational Registers  ========
	                     /* make value equal to HARDWARE RES (will be done by HostControllerReset)   */
//	LPC_USB->HcControl       = 0;                                     
//	LPC_USB->HcControlHeadED = 0;                                      /* Initialize Control list head to Zero       */
//	LPC_USB->HcBulkHeadED    = 0;                                      /* Initialize Bulk list head to Zero          */
    
      //====== Setup Host Contrller (see Ch. 5.1.1.4 and Ch. 5.1.1.5 in OpenHCI)  =======
                                /* SOFTWARE RESET                             */
    LPC_USB->HcCommandStatus = OR_CMD_STATUS_HCR;               /*  HCR (HostControllerReset) = 1  */
        /*   
		    reset all HC Operational Registers to ZERO with exception of
			     HcRevision, HcFmInterval, HcLSThreshold, HcRhDescriptorA,
		*/
    LPC_USB->HcFmInterval    = DEFAULT_FMINTERVAL;              /*  FSMPS (bits 30-16) : FI (bits 13-0) = ( (6/7)*(FI-210) << 16) | FI , where FI = 12000 - 1 */	   
                                /* Now in USBSuspend state; not more than 1 ms to enter USBOperational (Ch. 6.2.3), otherwise will enter USBResume */              

                                                              /* Put HC in operational state                */
    LPC_USB->HcControl  = (LPC_USB->HcControl & (~OR_CONTROL_HCFS)) | OR_CONTROL_HC_OPER;		  /*  HostControllerFunctionalState (HCFS) = 10b  */
    LPC_USB->HcRhStatus = OR_RH_STATUS_LPSC;                           /* Set Global Power                           */
    
    LPC_USB->HcHCCA = (U32)p_Hcca; 
    LPC_USB->HcInterruptStatus |= LPC_USB->HcInterruptStatus;         /* Clear Interrrupt Status                    */
                                                              /* Enable  interrupts                         */
    LPC_USB->HcInterruptEnable  =  OR_INTR_ENABLE_MIE |
                          OR_INTR_ENABLE_WDH |
                          OR_INTR_ENABLE_RD |
                          OR_INTR_ENABLE_RHSC;
	LPC_USB->HcPeriodicStart =  (DEFAULT_FMINTERVAL & 0x3FFF) * 0.9;  /* 90% of the value in FrameInteral  in HcFmInterval */

    /* Enable the USB Interrupt */
	Host_IRQ_Init();

    PRINT_Log("\nHost Initialized\n");
}


/*
**************************************************************************************************************
*                                     PROCESS TRANSFER DESCRIPTOR
*
* Description: This function processes the transfer descriptor
*
* Arguments  : ed            Endpoint descriptor that contains this transfer descriptor
*              token         SETUP, IN, OUT
*              buffer        Current Buffer Pointer of the transfer descriptor
*              buffer_len    Length of the buffer
*
* Returns    : OK       if TD submission is successful
*              ERROR    if TD submission fails
*
**************************************************************************************************************
*/

S32  Host_ProcessTD (volatile  HCED       *ed,
                            volatile  U32  token,
                            volatile  U8 *buffer,
                                      U32  buffer_len)
{
    volatile  U32   td_toggle;


    if (ed == p_EDCtrl) {
        if (token == TD_SETUP) {
            td_toggle = TD_TOGGLE_0;
        } else {
            td_toggle = TD_TOGGLE_1;
        }
    } else {
        td_toggle = 0;
    }
    p_TDHead->Control = (TD_ROUNDING    |
                      token           |
                      TD_DELAY_INT(0) |                           
                      td_toggle       |
                      TD_CC);
    p_TDTail->Control = 0;
    p_TDHead->CurrBufPtr   = (U32) buffer;
    p_TDTail->CurrBufPtr   = 0;
    p_TDHead->Next         = (U32) p_TDTail;
    p_TDTail->Next         = 0;
    p_TDHead->BufEnd       = (U32)(buffer + (buffer_len - 1));
    p_TDTail->BufEnd       = 0;

    ed->HeadTd  = (U32)p_TDHead | ((ed->HeadTd) & 0x00000002);
    ed->TailTd  = (U32)p_TDTail;
    ed->Next    = 0;

    if (ed == p_EDCtrl) {
        LPC_USB->HcControlHeadED = (U32)ed;
        LPC_USB->HcCommandStatus = LPC_USB->HcCommandStatus | OR_CMD_STATUS_CLF;
        LPC_USB->HcControl       = LPC_USB->HcControl       | OR_CONTROL_CLE;
    } else {
        LPC_USB->HcBulkHeadED    = (U32)ed;
        LPC_USB->HcCommandStatus = LPC_USB->HcCommandStatus | OR_CMD_STATUS_BLF;
        LPC_USB->HcControl       = LPC_USB->HcControl       | OR_CONTROL_BLE;
    }    

    Host_WDHWait();

    if (!(p_TDHead->Control & 0xF0000000)) {	  // completion code (bits 31-28) = 0: No Error
//	if (!HOST_TDControlStatus) {
        return (OK);
    } else {      
        return (ERR_TD_FAIL);
    }
}

/*
**************************************************************************************************************
*                                       ENUMERATE THE DEVICE
*
* Description: This function is used to enumerate the device connected
*
* Arguments  : None
*
* Returns    : None
*
**************************************************************************************************************
*/
extern void HID_BuildDeviceInstance (U8 *p_TDBuffer);
extern BOOL  HID_ParseCurrentConfiguration(void);
extern void HID_CopyConfigurationDescriptor(U32 n, U8* p_TDBuffer);
extern void  HID_ExtractConfigurationDescriptor(void);
extern void Build_interface_instances(void);
extern BOOL USB_parse_reportDescriptor(void);
BOOL HID_GetandCopyStringDescriptor(U32 n, U32 index, U8* p_TDBuffer);
extern struct  usb_device_instance  *p_usbD;
extern U8	*p_BF;

BOOL  Host_EnumDev (void)
{
    S32  rc;
    U8 *pD;
    U32 i, n;
    volatile U32 *p_RhPortStatus;


    PRINT_Log("Connect an HID device\n");
    while (!HOST_RhscIntr);
    Host_DelayMS(100);                            // debounce delay
    while (!HOST_RhscIntr);
    Host_DelayMS(100);                            /* USB 2.0 spec says atleast 50ms delay beore port reset */

    p_RhPortStatus = HcRhPortStatus_Address();
    *p_RhPortStatus = OR_RH_PORT_PRS;        // Initiate port reset

//      while (!(*p_RhPortStatus & OR_RH_PORT_PRSC) );		 // bit 20 = PRSC = 1: port reset is complete	==> PES (bit 2) =1: port is enabled
    while (*p_RhPortStatus & OR_RH_PORT_PRS); // Wait for port reset to complete...
    *p_RhPortStatus = OR_RH_PORT_PRSC;	// clear PRSC by wrinting '1'
    Host_DelayMS(100);                                                 /* Wait for 100 MS after port reset  */

    p_EDCtrl->Control = 8 << 16;                                         /* max pkt size (MPS) = 8 (bits 26-16)             */
    if (*p_RhPortStatus & OR_RH_PORT_LSDA) {				 // bit 9 = LSDA = 1: Low Speed Device
        p_EDCtrl->Control |= Bit(13);                                    /*  S (bit 13) = 1: Low Speed Endpoint  */
    }

    //========== Read first 8 bytes of device desc  (esp. for low speed device) ==============
    //==>   rc = HOST_GET_DESCRIPTOR(USB_DEVICE_DESCRIPTOR_TYPE, 0, p_TDBuffer, 8);
    n = 8;						 // 8 bytes
    rc = Host_CtrlRecv(USB_DEVICE_TO_HOST | REQUEST_TO_DEVICE, GET_DESCRIPTOR,    \
         (USB_DEVICE_DESCRIPTOR_TYPE << 8)|(0), 0, n, p_TDBuffer);		 //(descType << 8)|(descIndex)
    if (rc != OK) {
        PRINT_Err(rc);
        return (FALSE);
    }
          //--- update the real bMaxPacketSize0
    n = p_EDCtrl->Control;
    n =	(n&0x00FFFF) | ( ((USB_DEVICE_DESCRIPTOR *) p_TDBuffer)->bMaxPacketSize0) << 16;         // p_TDBuffer[7]<< 16   /* Get max pkt size of endpoint 0    */
    p_EDCtrl->Control = n;         // updated

    //========== Set Address ==============
    //==>    rc = HOST_SET_ADDRESS(1);                                          /* Set the device address to 1       */
    n =1;  // new bus address
    rc =Host_CtrlSend(USB_HOST_TO_DEVICE | REQUEST_TO_DEVICE, USB_REQUEST_SET_ADDRESS, n, 0, 0, p_TDBuffer);
    if (rc != OK) {
        PRINT_Err(rc);
        return (FALSE);
    }
    p_EDCtrl->Control = (p_EDCtrl->Control) | n;                          /* Modify control pipe with address 1 */

    PRINT_Log("Set Address as %u.\n", n);

    //========== Read all 18 bytes of device desc  ==============
    //==>    rc = HOST_GET_DESCRIPTOR(USB_DESCRIPTOR_TYPE_DEVICE, 0, p_TDBuffer, 18);
    n = 18;						 // 8 bytes
    rc = Host_CtrlRecv(USB_DEVICE_TO_HOST | REQUEST_TO_DEVICE, GET_DESCRIPTOR,    \
         (USB_DEVICE_DESCRIPTOR_TYPE << 8)|(0), 0, n, p_TDBuffer);		 //(descType << 8)|(descIndex)
    if (rc != OK) {
        PRINT_Err(rc);
        return (FALSE);
    }

    if (p_TDBuffer[1] != USB_DEVICE_DESCRIPTOR_TYPE) {    
        return (FALSE);
    }

    HID_BuildDeviceInstance ((U8*) p_TDBuffer);
    p_usbD->bUSB_DeviceAddress = 1;

    //======= Get the configuration descriptor =============
    for (n = 0; n < p_usbD->bNumConfigurations; n++) {
	U32 length;
	length = 9; 			  //   (Read first 9 bytes) 
	while (1) {
	    rc = Host_CtrlRecv(USB_DEVICE_TO_HOST | REQUEST_TO_DEVICE, GET_DESCRIPTOR,    \
               (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)|(n), 0, length, p_TDBuffer);		 //(descType << 8)|(descIndex): descINdex = n
	    if (rc != OK) {
		PRINT_Err(rc);
		return (FALSE);
	    }

	    if (length != 9) {
		if (p_TDBuffer[1] != USB_CONFIGURATION_DESCRIPTOR_TYPE) {
		    return (FALSE);
		}
		HID_CopyConfigurationDescriptor(n, (U8*) p_TDBuffer);
		break;
	    }
	    length = ((USB_CONFIGURATION_DESCRIPTOR *) p_TDBuffer)->wTotalLength;                                                             /* Get the first configuration data  */
	}   // END OF while(1)

    }	// END OF for (n = 0; n < p_usbD->bNumConfigurations; n++)

    PRINT_Log("Got %u Configuration Descriptors.\n", n);

    if (!HID_ParseCurrentConfiguration() ){    // parse current configuration value
        PRINT_Log("An interface is NOT HID.\n");
        PRINT_Err(-11);
        return (FALSE);
    }

    HID_ExtractConfigurationDescriptor ();
    PRINT_Log("Find out the total number of Report Descriptors is %u.\n", p_usbD->bNumReportDescriptors);

    //>>>>>======= Get the String descriptors =============
    n = 0;
    pD = (U8 *) &((USB_DEVICE_DESCRIPTOR *) p_usbD->p_USB_DeviceDescriptor)->iManufacturer;
    if ( *pD ) {
	if (!HID_GetandCopyStringDescriptor(n, *pD, (U8*) p_TDBuffer) ) {
//	    return (FALSE);
	}
	n++;
	*pD = n;			// correct  (p_usbD->p_USB_DeviceDescriptor)->iManufacturer
    }

    pD = (U8 *) &((USB_DEVICE_DESCRIPTOR *) p_usbD->p_USB_DeviceDescriptor)->iProduct;
    if ( *pD ) {
	if (!HID_GetandCopyStringDescriptor(n, *pD, (U8*) p_TDBuffer) ) {
//	    return (FALSE);
	}
	n++;
	*pD = n;			// correct  (p_usbD->p_USB_DeviceDescriptor)->iProduct
    }

    pD = (U8 *) &((USB_DEVICE_DESCRIPTOR *) p_usbD->p_USB_DeviceDescriptor)->iSerialNumber;
    if ( *pD ) {
	if (!HID_GetandCopyStringDescriptor(n, *pD, (U8*) p_TDBuffer) ) {
//	    return (FALSE);
	}
	n++;
	*pD = n;			// correct  (p_usbD->p_USB_DeviceDescriptor)->iSerialNumber
    }

    pD = p_usbD->p_USB_ConfigDescriptor;
    for (i = 0; i < p_usbD->bNumConfigurations; i++) {   	
	// local variables
	U8 *p_U8, *end_pD;

	p_U8 = (U8 *) &((USB_CONFIGURATION_DESCRIPTOR *)pD)->iConfiguration;
	if ( *p_U8 ) {
	    if (!HID_GetandCopyStringDescriptor(n, *p_U8, (U8*) p_TDBuffer) ) {
		return (FALSE);
	    }
	    n++;
	    *p_U8 = n;			// correct  (p_usbD->p_USB_ConfigDescriptor)->iConfiguration
	}

	end_pD = pD + ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;

	while(1) {
	    pD += pD[0];					// pD[0] = bLength;
	    if (pD >= end_pD){
	    	break;						// end of configuration descriptor[i]
	    }

	    if (pD[1] == USB_INTERFACE_DESCRIPTOR_TYPE){	// pD[1] = (USB_COMMON_DESCRIPTOR *)pD->bDescriptorType
		p_U8 = (U8 *) &((USB_INTERFACE_DESCRIPTOR *)pD)->iInterface;
		if ( *p_U8 ) {
		    if (!HID_GetandCopyStringDescriptor(n, *p_U8, (U8*) p_TDBuffer) ) {
			return (FALSE);
	            }
		    n++;
		    *p_U8 = n;			// correct  (p_usbD->p_USB_ConfigDescriptor)->iInterface
		}
//			 continue;			// goto while(1)
	    }
	}  // END OF while(1)
    }   // END OF for (i = 0; i < p_usbD->bNumConfigurations; i++)

    p_usbD->bNumStrings = n;
    p_usbD->wSize_StringDescriptor = (p_BF - p_usbD->p_USB_StringDescriptor);
    pD =(U8*) ( ( ((U32) p_BF)+3) & ~0x03);	// align by 4
    p_usbD->p_HID_ReportDescriptor  = pD;
    pD += p_usbD->wSize_ReportDescriptor;
    p_BF =(U8*) ( ( ((U32) pD)+3) & ~0x03);	    // align by 4
    Build_interface_instances();
    PRINT_Log("Got String Descriptor with \"%u\" Strings.\n", p_usbD->bNumStrings);
    //<<<<<<<<<------------

    //========== Set Configuration ==============
    //==>    rc = USBH_SET_CONFIGURATION(1);                                          /* Set the device address to 1       */
    n = p_usbD->bUSB_Configuration;  // configuration value
    rc =Host_CtrlSend(USB_HOST_TO_DEVICE | REQUEST_TO_DEVICE, USB_REQUEST_SET_CONFIGURATION, n, 0, 0, p_TDBuffer);
    if (rc != OK) {
        PRINT_Err(rc);
        return (FALSE);
    }
    Host_DelayMS(10);                                               /* Some devices may require this delay */
    PRINT_Log("Set Configuration as %u.\n", n);

      //======= Get the report descriptors =============
    pD = p_usbD->p_HID_ReportDescriptor;

    i = p_usbD->bUSB_Configuration;
    for (n = 0; n < p_usbD->bNumReportDescriptors; n++) {
	U32 ifc, length, k;

	ifc = p_usbD->p_subRepDescriptor[n].ID;
	if ((ifc & 0x00FF) != i) {
	    i = ifc & 0x0000FF;
	    //---- set configuration
	    rc =Host_CtrlSend(USB_HOST_TO_DEVICE | REQUEST_TO_DEVICE, USB_REQUEST_SET_CONFIGURATION, i, 0, 0, p_TDBuffer);
	    if (rc != OK) {
		PRINT_Err(rc);
		return (FALSE);
	    }
	    Host_DelayMS(10);                                               /* Some devices may require this delay */
	}

	if ( ((ifc &0x00FF) == p_usbD->bUSB_Configuration) & (ifc>>16 == 0) ){
	    //--- set Idle ---
	    rc =Host_CtrlSend(USB_HOST_TO_DEVICE | REQUEST_TO_INTERFACE | USB_REQUEST_TYPE_CLASS, \
	    HID_REQUEST_SET_IDLE, 0, (ifc&0x00FF00)>>8, 0, p_TDBuffer);
	    if (rc != OK) {
		PRINT_Err(rc);
//            return (FALSE);
	    }
	}

	ifc = ifc>>8;
	if (ifc & 0xFF00) {
	    // alternate setting != 0
	    //---- set interface
	    rc =Host_CtrlSend(USB_HOST_TO_DEVICE | REQUEST_TO_INTERFACE, USB_REQUEST_SET_INTERFACE, (ifc>>8), (ifc&0x00FF), 0, p_TDBuffer);
	    if (rc != OK) {
		PRINT_Err(rc);
		return (FALSE);
	    }
//         Host_DelayMS(50);                                               /* Some devices may require this delay */
	}
	//--- get Report descriptor ---
	length = p_usbD->p_subRepDescriptor[n].wRepLength; 			 
	rc = Host_CtrlRecv(USB_DEVICE_TO_HOST | REQUEST_TO_INTERFACE, GET_DESCRIPTOR,    \
               (HID_REPORT_DESCRIPTOR_TYPE << 8)|(0), (ifc&0x00FF), length, p_TDBuffer);		 //(descType << 8)|(descIndex): descINdex = n
	if (rc != OK) {
	    PRINT_Err(rc);
	    return (FALSE);
	}
	//--- copying .....
	for (k=0; k< length; k++) {
	    pD[k] = p_TDBuffer[k];
	}
	pD +=  length;

	if (ifc & 0xFF00) {
	    // alternate setting != 0
	    //---- set interface
	    rc =Host_CtrlSend(USB_HOST_TO_DEVICE | REQUEST_TO_INTERFACE, USB_REQUEST_SET_INTERFACE, 0, (ifc&0x00FF), 0, p_TDBuffer);
	    if (rc != OK) {
		PRINT_Err(rc);
		return (FALSE);
	    }
 //        Host_DelayMS(50);                                               /* Some devices may require this delay */
	}
    }	// END OF for (n = 0; n < p_usbD->bNumReportDescriptors; n++)
    PRINT_Log("Got %u Report Descriptors.\n", n);

    if ( p_usbD->bUSB_Configuration != i) {
	i = p_usbD->bUSB_Configuration;
	//---- set configuration
	rc =Host_CtrlSend(USB_HOST_TO_DEVICE | REQUEST_TO_DEVICE, USB_REQUEST_SET_CONFIGURATION, i, 0, 0, p_TDBuffer);
	if (rc != OK) {
	    PRINT_Err(rc);
	    return (FALSE);
	}
	Host_DelayMS(10);                                               /* Some devices may require this delay */
    }

    USB_parse_reportDescriptor ();

    // Test Interupt IN transfer for interface0
    n = p_EDCtrl->Control & 0xE07F;	  // clear bits MPS (26-16), D (12-11), EN (10-7).
    n |= (0x20 | (0x7F & p_usbD->p_IFCs[0].bInEPaddress) ) << 7;    // 0x20 = IN endpoint
    n |= p_usbD->p_IFCs[0].wInMaxPacketSize << 16;       // max pkt size (MPS) = n (bits 26-16)         
    p_EDBulkIn->Control = n;
    //=================================
    //--- check Interrupt IN transfer ---
    if ( p_usbD->p_IFCs[0].bInNumRIDs !=0 ) {
	U32 length;
	volatile  HCED       *ed;

	length = 0;
	for (i = 0; i< p_usbD->p_IFCs[0].bInNumRIDs; i++) {
	    if (length < p_usbD->p_IFCs[0].p_inRIDinfo[0].wSize) {
		length = p_usbD->p_IFCs[0].p_inRIDinfo[0].wSize;
	    }
	}
	if (length > ((n<<5)>>16) ) {
	    length = (n<<5)>>16;  // bits 26-16
	}

//*********************
//	length =p_usbD->p_IFCs[0].wInMaxPacketSize;  // It is OK to set length = wInMaxPacketSize
	ed = p_EDBulkIn;
	pD = p_HCdata;
	p_TDHead->Control = (TD_ROUNDING    |
                      TD_IN           |
                      TD_DELAY_INT(0) |                           
                      0       |
                      TD_CC);
	p_TDTail->Control = 0;
	p_TDHead->CurrBufPtr   = (U32) pD;
	p_TDTail->CurrBufPtr   = 0;
	p_TDHead->Next         = (U32) p_TDTail;
	p_TDTail->Next         = 0;
	p_TDHead->BufEnd       = ((U32) pD + (length - 1));
	p_TDTail->BufEnd       = 0;

	p_TDHead->HcEDPtr = (U32) ed;
	p_TDHead->InitBufPtr = (U32) pD;
	p_TDHead->wLength = length;
	p_TDHead->wRxCount = 0;

	pD += length;
	p_HCdata = (U8*) ( ((U32) pD + 3) & ~0x03 );   // align to 4-byte boundary

	ed->HeadTd  = (U32)p_TDHead | ((ed->HeadTd) & 0x00000002);
	ed->TailTd  = (U32)p_TDTail;		 // if TailTd = HeadTd, the list contains no HcTD.
	ed->Next    = 0;

	n = p_HcEDlist[7].NextIndex;		// Assume it is 8ms interval, so use list no. 7
	p_StaticHcED[7].Next = (U32) ed;  //&p_StaticHcED[j];
	ed->Next = (U32) &p_StaticHcED[n];

	LPC_USB->HcControl       |= OR_CONTROL_PLE;

#if 0
	//==  Interrupt Transfer Loop
	n = 0;
	while (1) {
	    U32 cnt;
	    volatile  HCTD       *td;

	    Host_WDHWait();
	    n++;
	    td = (volatile  HCTD  *)  (p_Hcca->HccaDoneHead & (~Bit(0)) );  // clear bit 0
	    if ( td->CurrBufPtr ){
		cnt = (td->CurrBufPtr - td->InitBufPtr);
	    } else {
		cnt = td->wLength;
	    }
	    td->wRxCount = cnt;

	    PRINT_Log("\n%u: Input Report: ", n);
	    pD = (U8*) p_TDHead->InitBufPtr;
	    for (i=0; i< cnt; i++) {
		PRINT_Log("%X, ", pD[i]);
	    }

	    td->Control |= TD_CC;
//    p_TDTail->Control = 0;
	    td->CurrBufPtr   = td->InitBufPtr;
//    p_TDTail->CurrBufPtr   = 0;
	    td->Next         = (U32) p_TDTail;
//    p_TDTail->Next         = 0;
	    cnt = td->wLength;
//        td->BufEnd       = (td->CurrBufPtr + (cnt - 1));
//    p_TDTail->BufEnd       = 0;
//	    td->HcEDPtr = (U32) ed;
//	    td->InitBufPtr = (U32) p_TDBuffer;
//	    td->wLength = cnt;
	    td->wRxCount = 0;

	    ed = (HCED *) td->HcEDPtr;
	    ed->HeadTd  = (U32)td | ((ed->HeadTd) & 0x00000002);

	    if (HOST_RhscIntr == 0) {	   //
		ed->Control |=sKip;
		PRINT_Log("\nDevice is just Disconnected!\n");
		PRINT_Log("\nReset and Try again!\n");
		return (TRUE);
	    }

	}
#endif
	return (TRUE);
//<<<<<<<<<<<<<<<<<<<<<<<<<
    }  // END OF if ( p_usbD->p_IFCs[0].bInNumRIDs !=0 )


    return (FALSE);
}

static U8 scan_table[] = {
/*         00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F */
/* 00 */ 0x00, 0x43, 0x00, 0x3f, 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26,
/* 10 */ 0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c, 0x02, 0x03,
/* 20 */ 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x1c, 0x01, 0x0e, 0x0f, 0x39, 0x0c, 0x0d, 0x1a,
/* 30 */ 0x1b, 0x2b, 0x30, 0x27, 0x28, 0x29, 0x33, 0x34, 0x35, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
/* 40 */ 0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x0a, 0x00, 0x00, 0x52, 0x47, 0x49, 0x53, 0x4f, 0x51, 0x4d,
/* 50 */ 0x4b, 0x50, 0x48, 0x00, 0x1a, 0x0d, 0x00, 0x00, 0x3a, 0x60, 0x1c, 0x1b, 0x00, 0x2b, 0x00, 0x00,
/* 60 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x4f, 0x00, 0x4b, 0x47, 0x74, 0x00, 0x00,
/* 70 */ 0x52, 0x62, 0x50, 0x69, 0x4d, 0x48, 0x01, 0x65, 0x57, 0x68, 0x64, 0x67, 0x66, 0x63, 0x6a, 0x00,
/* 80 */ 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static U8 old_scan = 0;
static U8 old_mode = 0;

static int hid_kbd_inited = 0;

void HID_InputKeyboard(void)
{
    int i;
    U32 cnt;
    U8 *pD;
    volatile  HCTD       *td;
    volatile  HCED       *ed;

    if (!HOST_RhscIntr) {
	if (hid_kbd_inited) {
	    hid_kbd_inited = 0;
	    Host_Init();
	}
	return;
    } else if (!hid_kbd_inited) {
	if (Host_EnumDev()) {
	    uart0Puts("USB Keyboard inited\n");
	    hid_kbd_inited = 1;
	}
    }

    if (!HOST_WdhIntr)
	return;

    td = (volatile  HCTD  *)  (p_Hcca->HccaDoneHead & (~Bit(0)) );  // clear bit 0
    if ( td->CurrBufPtr )
	cnt = (td->CurrBufPtr - td->InitBufPtr);
    else
	cnt = td->wLength;

    td->wRxCount = cnt;

//    PRINT_Log("\nInput Report: ");
    pD = (U8*) p_TDHead->InitBufPtr;
//    for (i=0; i< cnt; i++)
//	PRINT_Log("%X, ", pD[i]);

    if (pD[0] == 0x44 && pD[2] == 0x48)
	resetRequested();

    pD[0] |= (pD[0] >> 4);

    if (!(old_mode & 1) && (pD[0] & 1))
	jkeybModeDown(1);
    if ((old_mode & 1) && !(pD[0] & 1))
	jkeybModeUp(1);

    if (!(old_mode & 2) && (pD[0] & 2))
	jkeybModeDown(2);
    if ((old_mode & 2) && !(pD[0] & 2))
	jkeybModeUp(2);

    if (!(old_mode & 4) && (pD[0] & 4))
	jkeybDown(0x46);
    if ((old_mode & 4) && !(pD[0] & 4))
	jkeybUp();

    old_mode = pD[0];

    U8 scan = scan_table[pD[2]];
//    PRINT_Log(" [%02X - %02X]", pD[2], scan);
    if (scan != old_scan) {
	if (pD[2] == 0)
	    jkeybUp();
	else
	    jkeybDown(scan);
	old_scan = scan;
    }

    td->Control |= TD_CC;
    td->CurrBufPtr   = td->InitBufPtr;
    td->Next         = (U32) p_TDTail;
    cnt = td->wLength;
    td->wRxCount = 0;

    ed = (HCED *) td->HcEDPtr;
    ed->HeadTd  = (U32)td | ((ed->HeadTd) & 0x00000002);

    HOST_WdhIntr = 0;

#if 1
    if (HOST_RhscIntr == 0) {	   //
	ed->Control |=sKip;
	PRINT_Log("\nDevice is just Disconnected!\n");
	PRINT_Log("\nReset and Try again!\n");
	return (TRUE);
    }
#endif

}

/*
 *  
 *    Parameters:      None
 *    Return Value:    None
 */
BOOL HID_GetandCopyStringDescriptor(U32 n, U32 index, U8* p_TDBuffer)
{
    U8 *pD;
    U32 i, size, lg;
    S32  rc;

    pD = p_BF;
//------ String Descriptor ----------------
    if (n == 0) {
	i = n;
	lg = 0;
    } else {
	i = index;
	lg = 0; //0x0409;    // American English
    }
    size = 0;

    while(1) {
	rc = Host_CtrlRecv(USB_DEVICE_TO_HOST | REQUEST_TO_DEVICE, GET_DESCRIPTOR,    \
             (USB_STRING_DESCRIPTOR_TYPE << 8) | (i), lg, 128, (p_TDBuffer+size));		 //	//(descType << 8)|(descIndex)
	if (rc != OK) {
	    PRINT_Err(rc);
	    return (FALSE);
	}
	size += p_TDBuffer[size];          // bLength

	if (i == index) {
	    break;
	}

	p_usbD->p_USB_StringDescriptor  = pD;
	i = index;
    } // END of while (1);

    //--- copying .....
    for (i=0; i< size; i++) {
	pD[i] = p_TDBuffer[i];
    }
    p_BF += size;

    return (TRUE);
}


/*
**************************************************************************************************************
*                                        RECEIVE THE CONTROL INFORMATION
*
* Description: This function is used to receive the control information
*
* Arguments  : bm_request_type
*              b_request
*              w_value
*              w_index
*              w_length
*              buffer
*
* Returns    : OK       if Success
*              ERROR    if Failed
*
**************************************************************************************************************
*/
   
S32  Host_CtrlRecv (         U8   bm_request_type,
                                    U8   b_request,
                                    U16   w_value,
                                    U16   w_index,
                                    U16   w_length,
                          volatile  U8  *buffer)
{
    S32  rc;


    Host_FillSetup(bm_request_type, b_request, w_value, w_index, w_length);
    rc = Host_ProcessTD(p_EDCtrl, TD_SETUP, p_TDBuffer, 8);
    if (rc == OK) {
        if (w_length) {
            rc = Host_ProcessTD(p_EDCtrl, TD_IN, p_TDBuffer, w_length);
        }
        if (rc == OK) {
            rc = Host_ProcessTD(p_EDCtrl, TD_OUT, NULL, 0);
        }
    }
    return (rc);
}

/*
**************************************************************************************************************
*                                         SEND THE CONTROL INFORMATION
*
* Description: This function is used to send the control information
*
* Arguments  : None
*
* Returns    : OK		              if Success
*              ERR_INVALID_BOOTSIG    if Failed
*
**************************************************************************************************************
*/

S32  Host_CtrlSend (          U8   bm_request_type,
                                     U8   b_request,
                                     U16   w_value,
                                     U16   w_index,
                                     U16   w_length,
                           volatile  U8  *buffer)
{
    S32  rc;


    Host_FillSetup(bm_request_type, b_request, w_value, w_index, w_length);

    rc = Host_ProcessTD(p_EDCtrl, TD_SETUP, p_TDBuffer, 8);
    if (rc == OK) {
        if (w_length) {
            rc = Host_ProcessTD(p_EDCtrl, TD_OUT, p_TDBuffer, w_length);
        }
        if (rc == OK) {
            rc = Host_ProcessTD(p_EDCtrl, TD_IN, NULL, 0);
        }
    }
    return (rc);
}

/*
**************************************************************************************************************
*                                          FILL SETUP PACKET
*
* Description: This function is used to fill the setup packet
*
* Arguments  : None
*
* Returns    : OK		              if Success
*              ERR_INVALID_BOOTSIG    if Failed
*
**************************************************************************************************************
*/

void  Host_FillSetup (U8   bm_request_type,
                      U8   b_request,
                      U16   w_value,
                      U16   w_index,
                      U16   w_length)
{
	int i;
	for (i=0;i<w_length;i++)
		p_TDBuffer[i] = 0;
	
    p_TDBuffer[0] = bm_request_type;
    p_TDBuffer[1] = b_request;
    WriteLE16U(&p_TDBuffer[2], w_value);
    WriteLE16U(&p_TDBuffer[4], w_index);
    WriteLE16U(&p_TDBuffer[6], w_length);
}



/*
**************************************************************************************************************
*                                         INITIALIZE THE TRANSFER DESCRIPTOR
*
* Description: This function initializes transfer descriptor
*
* Arguments  : Pointer to TD structure
*
* Returns    : None
*
**************************************************************************************************************
*/

void  Host_TDInit (volatile  HCTD *td)
{

    td->Control    = 0;
    td->CurrBufPtr = 0;
    td->Next       = 0;
    td->BufEnd     = 0;
}

/*
**************************************************************************************************************
*                                         INITIALIZE THE ENDPOINT DESCRIPTOR
*
* Description: This function initializes endpoint descriptor
*
* Arguments  : Pointer to ED strcuture
*
* Returns    : None
*
**************************************************************************************************************
*/

void  Host_EDInit (volatile  HCED *ed)
{

    ed->Control = 0;
    ed->TailTd  = 0;
    ed->HeadTd  = 0;
    ed->Next    = 0;
}

/*
**************************************************************************************************************
*                                 INITIALIZE HOST CONTROLLER COMMUNICATIONS AREA
*
* Description: This function initializes host controller communications area
*
* Arguments  : Pointer to HCCA
*
* Returns    : 
*
**************************************************************************************************************
*/

void  Host_HCCAInit (volatile  HCCA  *hcca)
{
    U32  i;


    for (i = 0; i < 32; i++) {

        hcca->HccaIntTable[i] = 0;
        hcca->HccaFrameNumber = 0;
        hcca->HccaDoneHead    = 0;
    }

}

/*
**************************************************************************************************************
*                                         WAIT FOR WDH INTERRUPT
*
* Description: This function is infinite loop which breaks when ever a WDH interrupt rises
*
* Arguments  : None
*
* Returns    : None
*
**************************************************************************************************************
*/

void  Host_WDHWait (void)
{
  while (!HOST_WdhIntr) {
    ;
  }
  HOST_WdhIntr = 0;
}

/*
**************************************************************************************************************
*                                         READ LE 32U
*
* Description: This function is used to read an unsigned integer from a charecter buffer in the platform
*              containing little endian processor
*
* Arguments  : pmem    Pointer to the charecter buffer
*
* Returns    : val     Unsigned integer
*
**************************************************************************************************************
*/

U32  ReadLE32U (volatile  U8  *pmem)
{
    U32   val;

    ((U8 *)&val)[0] = pmem[0];
    ((U8 *)&val)[1] = pmem[1];
    ((U8 *)&val)[2] = pmem[2];
    ((U8 *)&val)[3] = pmem[3];

    return (val);
}

/*
**************************************************************************************************************
*                                        WRITE LE 32U
*
* Description: This function is used to write an unsigned integer into a charecter buffer in the platform 
*              containing little endian processor.
*
* Arguments  : pmem    Pointer to the charecter buffer
*              val     Integer value to be placed in the charecter buffer
*
* Returns    : None
*
**************************************************************************************************************
*/

void  WriteLE32U (volatile  U8  *pmem,
                            U32   val)
{
    pmem[0] = ((U8 *)&val)[0];
    pmem[1] = ((U8 *)&val)[1];
    pmem[2] = ((U8 *)&val)[2];
    pmem[3] = ((U8 *)&val)[3];
}

/*
**************************************************************************************************************
*                                          READ LE 16U
*
* Description: This function is used to read an unsigned short integer from a charecter buffer in the platform
*              containing little endian processor
*
* Arguments  : pmem    Pointer to the charecter buffer
*
* Returns    : val     Unsigned short integer
*
**************************************************************************************************************
*/

U16  ReadLE16U (volatile  U8  *pmem)
{
    U16   val;

    ((U8 *)&val)[0] = pmem[0];
    ((U8 *)&val)[1] = pmem[1];


    return (val);
}

/*
**************************************************************************************************************
*                                         WRITE LE 16U
*
* Description: This function is used to write an unsigned short integer into a charecter buffer in the
*              platform containing little endian processor
*
* Arguments  : pmem    Pointer to the charecter buffer
*              val     Value to be placed in the charecter buffer
*
* Returns    : None
*
**************************************************************************************************************
*/

void  WriteLE16U (volatile  U8  *pmem,
                            U16   val)
{
    pmem[0] = ((U8 *)&val)[0];
    pmem[1] = ((U8 *)&val)[1];
}

/*
**************************************************************************************************************
*                                         READ BE 32U
*
* Description: This function is used to read an unsigned integer from a charecter buffer in the platform
*              containing big endian processor
*
* Arguments  : pmem    Pointer to the charecter buffer
*
* Returns    : val     Unsigned integer
*
**************************************************************************************************************
*/

U32  ReadBE32U (volatile  U8  *pmem)
{
    U32   val;

    ((U8 *)&val)[0] = pmem[3];
    ((U8 *)&val)[1] = pmem[2];
    ((U8 *)&val)[2] = pmem[1];
    ((U8 *)&val)[3] = pmem[0];

    return (val);
}

/*
**************************************************************************************************************
*                                         WRITE BE 32U
*
* Description: This function is used to write an unsigned integer into a charecter buffer in the platform
*              containing big endian processor
*
* Arguments  : pmem    Pointer to the charecter buffer
*              val     Value to be placed in the charecter buffer
*
* Returns    : None
*
**************************************************************************************************************
*/

void  WriteBE32U (volatile  U8  *pmem,
                            U32   val)
{
    pmem[0] = ((U8 *)&val)[3];
    pmem[1] = ((U8 *)&val)[2];
    pmem[2] = ((U8 *)&val)[1];
    pmem[3] = ((U8 *)&val)[0];

}

/*
**************************************************************************************************************
*                                         READ BE 16U
*
* Description: This function is used to read an unsigned short integer from a charecter buffer in the platform
*              containing big endian processor
*
* Arguments  : pmem    Pointer to the charecter buffer
*
* Returns    : val     Unsigned short integer
*
**************************************************************************************************************
*/

U16  ReadBE16U (volatile  U8  *pmem)
{
    U16  val;


    ((U8 *)&val)[0] = pmem[1];
    ((U8 *)&val)[1] = pmem[0];

    return (val);
}

/*
**************************************************************************************************************
*                                         WRITE BE 16U
*
* Description: This function is used to write an unsigned short integer into the charecter buffer in the
*              platform containing big endian processor
*
* Arguments  : pmem    Pointer to the charecter buffer
*              val     Value to be placed in the charecter buffer
*
* Returns    : None
*
**************************************************************************************************************
*/

void  WriteBE16U (volatile  U8  *pmem,
                            U16   val)
{
    pmem[0] = ((U8 *)&val)[1];
    pmem[1] = ((U8 *)&val)[0];
}
