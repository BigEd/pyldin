/*----------------------------------------------------------------------------
 *      U S B  -  M E M O R Y
 *----------------------------------------------------------------------------
 *      Name:    .h
 *      Purpose: 
 *      Version: V1.00
 *----------------------------------------------------------------------------*/
#ifndef USB_HID
#define USB_HID  1
#endif

 
#ifndef __USBCREATE_H__
#define __USBCREATE_H__


/* USB Received Data Structure for OUTPUT and FEATURE reports*/
struct RX_DATA_INFO {
  U8  bConfigurationValue;
  U8  bInterfaceNumber;
  U8  bAlternateSetting;
  U8  bRID;
  U8  bType;        /* HID_REPORT_OUTPUT or HID_REPORT_FEATURE */
  U16 wDataSize;    /* total length of received bytes */
  U8  *pData;
};


/* Callback Function for OUTPUT and FEATURE Report of HID */
typedef void 
 CB_USB_Rx_Handler(struct RX_DATA_INFO  rx_info);

/* Callback Function for getting physical address of an HID report according to RID */ 
typedef void 
 CB_USB_BufMapping (U32 requestType, U32 ifcNo);

extern	BOOL USB_device_create (U8*, U8*, U8*, U8*);
extern	BOOL USB_parse_reportDescriptor (void);

/*  information block of report descriptors */
struct HID_repDesc_info{
U32	ID;			 /* bits[7:0]: configuration value; bits[15:8]: interface number; bits[23:16]: alternate setting */
U16	wRepOffset;	 /* offset from p_HID_ReportDescriptor to this Report subdescriptor */
U16	wRepLength;	 /* length of this Report subdescriptor */
// size  = 8 bytes;
};

/* struct definition */
struct  HID_RID_info{
U8	bRID;
U16	wSize;			/*  length of bytes of this report ID in non-boot mode */
U16	wLeftSize;		/* IN: left size of bytes to be sent; OUT: total length of received bytes */
U8	*pData;	  
// size  = 12 bytes;
U8 bDummy;  // to reduce code size by aligning struct size to 2**n bytes, although increase RW size
           //  size of struct will be multiple of 4-bytes after complilation
};

	  /* USB Interface Instance */
struct  usb_IFC_instance{
U8	        bInEPaddress;
U8	        bInNumRIDs;				 /* number of RIDs for INPUT reports  */
U16			wInMaxPacketSize;
U8	        bOutEPaddress;
U8	        bOutNumRIDs;			 /* number of RIDs for OUTPUT reports  */
U16			wOutMaxPacketSize;
U32			fg_queue_RIDs;           /* bits 31-16: "RID order" of IN transfer requests (Input Report); bits 15-0: "RID order" of OUT transfer requests (Output Report); */  		   
U8	        bFeatureNumRIDs;		 /* number of RIDs for FEATURE reports  */
U8          bMaxTotalRIDs;           /* maximum number of total RIDs of this interface for different alternates  */
U16			fg_wFeatureQueue_RIDs;	  /* bits 15-0: "RID order" of OUT transfer requests (Feature Report); */  		   
struct HID_RID_info	 *p_inRIDinfo;
struct HID_RID_info	 *p_outRIDinfo;
struct HID_RID_info	 *p_featureRIDinfo;
U32         maxDataBuffer;           /* maximum number of data buffer in Byte for this interface instance */
// size  = 32 bytes;
//U8  bDummy; // to reduce code size by aligning struct size to 2**n bytes, although increase RW size
            // size of struct will be multiple of 4-bytes after complilation
};

	  /* USB Device Instance */
struct  usb_device_instance	{
U8	*p_USB_DeviceDescriptor;
U8	*p_USB_ConfigDescriptor;
U8	*p_USB_StringDescriptor;
U8	*p_HID_ReportDescriptor;

U8	bUSB_DeviceAddress;		/* USB bus address */
U8	bUSB_Configuration;		/* current configuration number */
//U16	wUSB_DeviceStatus;		/* bits 15-3 = 0; bit(1) = Remote Wakeup; bit(0) = Self Powered */

			/* bits 31-16: input endpoints; bits 15-0: output endpoints */
//U32	fg_USB_EndPointMask;	/*  working endpoints for current configuration */
//U32	fg_USB_EndPointHalt;	/*  HALTED working endpoints for current configuration */

U8	ep0_bMaxPacketSize;		/* maximum packet size declared in Device Descriptor */
U8	bNumConfigurations;
U8	bNumInterfaces;     	/*  number of interfaces for current configuration */
U8	bMaxNumInterfaces;     	/* Maximum number of interfaces for all possible configurations */
U8	bNumStrings;				/* number of strings */
U8	bNumReportDescriptors;	/* number of report descriptors */

//U8	*p_bEP0Buf;				/*  working buffer for Control Transfer ep0 */
U8	*p_bAltSetting_perIFC;	/*  Alternate Setting No. per Interfaces for current configuration */

U8 	*p_bHID_Protocol;
//U8 	*p_bHID_IdleTime;
//U8 	*p_bHID_IdleCount;

U8	bIFCs_ready;
struct usb_IFC_instance	 *p_IFCs;
U32	fg_queue_IFCs;           /* bits 31-16: IFCs of IN transfer requests; bits 15-0: IFCs of Out (or Feature) transfer requests; */  

//CB_USB_Rx_Handler *p_callback_Rx_handler;	/* User's handler Function for output and feature reports */
//CB_USB_BufMapping *p_callback_bufMapping;
struct HID_repDesc_info *p_subRepDescriptor;  /*  information block of report descriptors */

U8	*p_head_RIDinfo;
U8 wSize_ConfigDescriptor;
U8 wSize_StringDescriptor;
U8 wSize_ReportDescriptor;
};



/*       GLOBAL VARIABLES	*/
extern struct  usb_device_instance  *p_usbD;

#endif  /* __USBCREATE__ */

