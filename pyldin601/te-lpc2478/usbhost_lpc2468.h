/*
**************************************************************************************************************
*                                                 NXP USB Host Stack
*
*
**************************************************************************************************************
*/

#ifndef USBHOST_LPC2468_H
#define USBHOST_LPC2468_H

/*
**************************************************************************************************************
*                                       INCLUDE HEADER FILES
**************************************************************************************************************
*/

/*
**************************************************************************************************************
*                                        PRINT CONFIGURATION
**************************************************************************************************************
*/

#define  PRINT_ENABLE         1

#if PRINT_ENABLE
#define  PRINT_Log(...)       uart0Printf(__VA_ARGS__)
#define  PRINT_Err(rc)        uart0Printf("ERROR: In %s at Line %u - rc = %d\n", __FUNCTION__, __LINE__, rc)

#else 
#define  PRINT_Log(...)       do {} while(0)
#define  PRINT_Err(rc)        do {} while(0)

#endif

/*
**************************************************************************************************************
*                                        GENERAL DEFINITIONS
**************************************************************************************************************
*/

#define  DESC_LENGTH(x)  x[0]
#define  DESC_TYPE(x)    x[1]


#define  HOST_GET_DESCRIPTOR(descType, descIndex, data, length)                      \
         Host_CtrlRecv(USB_DEVICE_TO_HOST | USB_RECIPIENT_DEVICE, GET_DESCRIPTOR,    \
         (descType << 8)|(descIndex), 0, length, data)

#define  HOST_SET_ADDRESS(new_addr)                                                  \
         Host_CtrlSend(USB_HOST_TO_DEVICE | USB_RECIPIENT_DEVICE, SET_ADDRESS,       \
         new_addr, 0, 0, NULL)

#define  USBH_SET_CONFIGURATION(configNum)                                           \
         Host_CtrlSend(USB_HOST_TO_DEVICE | USB_RECIPIENT_DEVICE, SET_CONFIGURATION, \
         configNum, 0, 0, NULL)

#define  USBH_SET_INTERFACE(ifNum, altNum)                                           \
         Host_CtrlSend(USB_HOST_TO_DEVICE | USB_RECIPIENT_INTERFACE, SET_INTERFACE,  \
         altNum, ifNum, 0, NULL)

/*
**************************************************************************************************************
*                                    OHCI OPERATIONAL REGISTER DEFINITIONS
**************************************************************************************************************
*/

//#define  HcControl            *((volatile U32*)((U32)0xFFE0C004))
//#define  HcCommandStatus      *((volatile U32*)((U32)0xFFE0C008))
//#define  HcInterruptStatus    *((volatile U32*)((U32)0xFFE0C00C))
//#define  HcInterruptEnable    *((volatile U32*)((U32)0xFFE0C010))
//#define  HcHCCA               *((volatile U32*)((U32)0xFFE0C018))
//#define  HcControlHeadED      *((volatile U32*)((U32)0xFFE0C020))
//#define  HcBulkHeadED         *((volatile U32*)((U32)0xFFE0C028))
//#define  HcFmInterval         *((volatile U32*)((U32)0xFFE0C034))
//#define  HcRhStatus           *((volatile U32*)((U32)0xFFE0C050))

/*
#define HcRevision         HC_REVISION
*/
 /*  Control and Status Partition */
/*
#define HcControl          HC_CONTROL  
#define HcCommandStatus    HC_CMD_STAT 
#define HcInterruptStatus  HC_INT_STAT 
#define HcInterruptEnable  HC_INT_EN  
#define HcInterruptDisable HC_INT_DIS        
*/
 /*  Memory Pointer Partition */
/*
#define HcHCCA             HC_HCCA   
#define HcPeriodCurrentED  HC_PERIOD_CUR_ED
#define HcControlHeadED    HC_CTRL_HEAD_ED
#define HcControlCurrentED HC_CTRL_CUR_ED 
#define HcBulkHeadED       HC_BULK_HEAD_ED 
#define HcBulkCurrentED    HC_BULK_CUR_ED 
#define HcDoneHead         HC_DONE_HEAD       
*/
 /*  Frame Counter Partition */
/*
#define HcFmInterval       HC_FM_INTERVAL
#define HcFmRemaining      HC_FM_REMAINING 
#define HcFmNumber         HC_FM_NUMBER
#define HcPeriodicStart    HC_PERIOD_START   
#define HcLSThreshold      HC_LS_THRHLD    
*/
 /*  Root Hub Partition */
/*
#define HcRhDescriptorA    HC_RH_DESCA  
#define HcRhDescriptorB    HC_RH_DESCB 
#define HcRhStatus         HC_RH_STAT        
*/
//#define HcRhPortStatus1    HC_RH_PORT_STAT1	 //  HC_RH_PORT_STAT1 =: *((volatile U32*)((U32)0xFFE0C050))
//#define HcRhPortStatus2    HC_RH_PORT_STAT2   
//#define  HcRhPortStatus       ((volatile U32*)((U32)0xFFE0C054 - 4) ) 	// NOTE: Only [1] and [2]; NO [0]
#define p_HcRhPortStatus     ((volatile U32*)((U32)&HC_RH_PORT_STAT1 -4) )	// NOTE: Only [1] and [2]; NO [0]

/*
**************************************************************************************************************
*                                  OHCI OPERATIONAL REGISTER FIELD DEFINITIONS
**************************************************************************************************************
*/

                                            /* ------------------ HcControl Register ---------------------  */
#define  OR_CONTROL_PLE                 0x00000004
#define  OR_CONTROL_IE                  0x00000008
#define  OR_CONTROL_CLE                 0x00000010
#define  OR_CONTROL_BLE                 0x00000020
#define  OR_CONTROL_HCFS                0x000000C0
#define  OR_CONTROL_HC_OPER             0x00000080
                                            /* ----------------- HcCommandStatus Register ----------------- */
#define  OR_CMD_STATUS_HCR              0x00000001
#define  OR_CMD_STATUS_CLF              0x00000002
#define  OR_CMD_STATUS_BLF              0x00000004
                                            /* --------------- HcInterruptStatus Register ----------------- */
#define  OR_INTR_STATUS_WDH             0x00000002
#define  OR_INTR_STATUS_RD              0x00000008
#define  OR_INTR_STATUS_RHSC            0x00000040
                                            /* --------------- HcInterruptEnable Register ----------------- */
#define  OR_INTR_ENABLE_WDH             Bit(1) // 0x00000002
#define  OR_INTR_ENABLE_RD              Bit(3) // 0x00000008
#define  OR_INTR_ENABLE_RHSC            Bit(6) // 0x00000040
#define  OR_INTR_ENABLE_MIE             Bit(31) // 0x80000000
                                            /* ---------------- HcRhDescriptorA Register ------------------ */
#define  OR_RH_STATUS_LPSC              0x00010000
#define  OR_RH_STATUS_DRWE              0x00008000
                                            /* -------------- HcRhPortStatus[1:NDP] Register -------------- */
#define  OR_RH_PORT_CCS                 Bit(0) // 0x00000001
#define  OR_RH_PORT_PRS                 Bit(4) //0x00000010
#define  OR_RH_PORT_LSDA                Bit(9) // 0x00000200
#define  OR_RH_PORT_CSC                 Bit(16) // 0x00010000
#define  OR_RH_PORT_PRSC                Bit(20) // 0x00100000


/*
**************************************************************************************************************
*                                               FRAME INTERVAL
*                   see Ch. 5.4 in OpenHCI
**************************************************************************************************************
*/

#define  FI                     0x2EDF           /* 12000 bits per frame (-1)                               */
#define  DEFAULT_FMINTERVAL     (( ( (6 * (FI - 210)) / 7) << 16) | FI)

/*
**************************************************************************************************************
*                                       TRANSFER DESCRIPTOR CONTROL FIELDS
**************************************************************************************************************
*/

#define  TD_ROUNDING        (U32) (0x00040000)        /* Buffer Rounding                             */
#define  TD_SETUP           (U32)(0)                  /* Direction of Setup Packet                   */
#define  TD_IN              (U32)(0x00100000)         /* Direction In                                */
#define  TD_OUT             (U32)(0x00080000)         /* Direction Out                               */
#define  TD_DELAY_INT(x)    (U32)((x) << 21)          /* Delay Interrupt                             */
#define  TD_TOGGLE_0        (U32)(0x02000000)         /* Toggle 0                                    */
#define  TD_TOGGLE_1        (U32)(0x03000000)         /* Toggle 1                                    */
#define  TD_CC              (U32)(0xF0000000)         /* Completion Code                             */

/*
**************************************************************************************************************
*                                       USB STANDARD REQUEST DEFINITIONS
**************************************************************************************************************
*/

#define  USB_RECIPIENT_DEVICE       0x00
#define  USB_RECIPIENT_INTERFACE    0x01
                                                    /* -------------- USB Standard Requests  -------------- */
#define  SET_ADDRESS                 5
#define  GET_DESCRIPTOR              6
#define  SET_CONFIGURATION           9
#define  SET_INTERFACE              11

/*
**************************************************************************************************************
*                                       TYPE DEFINITIONS
**************************************************************************************************************
*/

typedef struct hcED {                       /* ----------- HostController EndPoint Descriptor ------------- */
    volatile  U32  Control;              /* Endpoint descriptor control                              */
    volatile  U32  TailTd;               /* Physical address of tail in Transfer descriptor list     */
    volatile  U32  HeadTd;               /* Physcial address of head in Transfer descriptor list     */
    volatile  U32  Next;                 /* Physical address of next Endpoint descriptor             */
} HCED;

typedef struct hcTD {                       /* ------------ HostController Transfer Descriptor ------------ */
    volatile  U32  Control;              /* Transfer descriptor control                              */
    volatile  U32  CurrBufPtr;           /* Physical address of current buffer pointer               */
    volatile  U32  Next;                 /* Physical pointer to next Transfer Descriptor             */
    volatile  U32  BufEnd;               /* Physical address of end of buffer                        */
			   U32  HcEDPtr;			   // the pointer of HcED on (to) which this HcTD is hung (queued).
			   U32  InitBufPtr;            // initial pointer of the data buffer
			   U16  wLength;
			   U16  wRxCount;
			   U32  Reserved[1]; 
} HCTD;

typedef struct hcca {                       /* ----------- Host Controller Communication Area ------------  */
    volatile  U32  HccaIntTable[32];         /* Interrupt Table                                          */
    volatile  U32  HccaFrameNumber;          /* Frame Number                                             */
    volatile  U32  HccaDoneHead;             /* Done Head                                                */
    volatile  U8  Reserved[116];        /* Reserved for future use                                  */
    volatile  U8  Unknown[4];           /* Unused                                                   */
} HCCA;

typedef struct hcED_List {                       /* -- */
	U8   NextIndex;                           
    U16  Bandwidth;         
} HCED_LIST;
/*
**************************************************************************************************************
*                                     EXTERN DECLARATIONS
**************************************************************************************************************
*/

extern volatile  U32   HOST_RhscIntr;         /* Root Hub Status Change interrupt                       */
extern volatile  U32   HOST_WdhIntr  ;         /* Semaphore to wait until the TD is submitted            */
extern volatile  U8    HOST_TDControlStatus;

/*
**************************************************************************************************************
*                                       FUNCTION PROTOTYPES
**************************************************************************************************************
*/

void        Host_Init     (void);

void        Host_Isr      (void);

BOOL  Host_EnumDev  (void);

S32  Host_ProcessTD(volatile  HCED       *ed,
                           volatile  U32  token,
                           volatile  U8 *buffer,
                                     U32  buffer_len);

void        Host_DelayUS  (          U32    delay);
void        Host_DelayMS  (          U32    delay);


void        Host_TDInit   (volatile  HCTD *td);
void        Host_EDInit   (volatile  HCED *ed);
void        Host_HCCAInit (volatile  HCCA  *hcca);

S32  Host_CtrlRecv (          U8   bm_request_type,
                                     U8   b_request,
                                     U16   w_value,
                                     U16   w_index,
                                     U16   w_length,
                           volatile  U8  *buffer);

S32  Host_CtrlSend (          U8   bm_request_type,
                                     U8   b_request,
                                     U16   w_value,
                                     U16   w_index,
                                     U16   w_length,
                           volatile  U8  *buffer);

void        Host_FillSetup(          U8   bm_request_type,
                                     U8   b_request,
                                     U16   w_value,
                                     U16   w_index,
                                     U16   w_length);


void        Host_WDHWait  (void);


U32  ReadLE32U     (volatile  U8  *pmem);
void        WriteLE32U    (volatile  U8  *pmem,
                                     U32   val);
U16  ReadLE16U     (volatile  U8  *pmem);
void        WriteLE16U    (volatile  U8  *pmem,
                                     U16   val);
U32  ReadBE32U     (volatile  U8  *pmem);
void        WriteBE32U    (volatile  U8  *pmem,
                                     U32   val);
U16  ReadBE16U     (volatile  U8  *pmem);
void        WriteBE16U    (volatile  U8  *pmem,
                                     U16   val);
//===========================================
U8* Host_GPIO_configuration (void);
void Host_IRQ_Init (void);
volatile U32* HcRhPortStatus_Address (void);

void SystemInit (void);

//============================================
#ifdef __cplusplus
#define     __I     volatile                  /*!< defines 'read only' permissions      */
#else
#define     __I     volatile const            /*!< defines 'read only' permissions      */
#endif
#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */

/*------------- Universal Serial Bus (USB) -----------------------------------*/
typedef struct
{
  __I  U32 HcRevision;             /* USB Host Registers                 */
  __IO U32 HcControl;
  __IO U32 HcCommandStatus;
  __IO U32 HcInterruptStatus;
  __IO U32 HcInterruptEnable;
  __IO U32 HcInterruptDisable;
  __IO U32 HcHCCA;
  __I  U32 HcPeriodCurrentED;
  __IO U32 HcControlHeadED;
  __IO U32 HcControlCurrentED;
  __IO U32 HcBulkHeadED;
  __IO U32 HcBulkCurrentED;
  __I  U32 HcDoneHead;
  __IO U32 HcFmInterval;
  __I  U32 HcFmRemaining;
  __I  U32 HcFmNumber;
  __IO U32 HcPeriodicStart;
  __IO U32 HcLSTreshold;
  __IO U32 HcRhDescriptorA;
  __IO U32 HcRhDescriptorB;
  __IO U32 HcRhStatus;
  __IO U32 HcRhPortStatus1;
  __IO U32 HcRhPortStatus2;
       U32 RESERVED0[40];
  __I  U32 Module_ID;

  __I  U32 OTGIntSt;               /* USB On-The-Go Registers            */
  __IO U32 OTGIntEn;
  __O  U32 OTGIntSet;
  __O  U32 OTGIntClr;
  __IO U32 OTGStCtrl;
  __IO U32 OTGTmr;
       U32 RESERVED1[58];

  __I  U32 USBDevIntSt;            /* USB Device Interrupt Registers     */
  __IO U32 USBDevIntEn;
  __O  U32 USBDevIntClr;
  __O  U32 USBDevIntSet;

  __O  U32 USBCmdCode;             /* USB Device SIE Command Registers   */
  __I  U32 USBCmdData;

  __I  U32 USBRxData;              /* USB Device Transfer Registers      */
  __O  U32 USBTxData;
  __I  U32 USBRxPLen;
  __O  U32 USBTxPLen;
  __IO U32 USBCtrl;
  __O  U32 USBDevIntPri;

  __I  U32 USBEpIntSt;             /* USB Device Endpoint Interrupt Regs */
  __IO U32 USBEpIntEn;
  __O  U32 USBEpIntClr;
  __O  U32 USBEpIntSet;
  __O  U32 USBEpIntPri;

  __IO U32 USBReEp;                /* USB Device Endpoint Realization Reg*/
  __O  U32 USBEpInd;
  __IO U32 USBMaxPSize;

  __I  U32 USBDMARSt;              /* USB Device DMA Registers           */
  __O  U32 USBDMARClr;
  __O  U32 USBDMARSet;
       U32 RESERVED2[9];
  __IO U32 USBUDCAH;
  __I  U32 USBEpDMASt;
  __O  U32 USBEpDMAEn;
  __O  U32 USBEpDMADis;
  __I  U32 USBDMAIntSt;
  __IO U32 USBDMAIntEn;
       U32 RESERVED3[2];
  __I  U32 USBEoTIntSt;
  __O  U32 USBEoTIntClr;
  __O  U32 USBEoTIntSet;
  __I  U32 USBNDDRIntSt;
  __O  U32 USBNDDRIntClr;
  __O  U32 USBNDDRIntSet;
  __I  U32 USBSysErrIntSt;
  __O  U32 USBSysErrIntClr;
  __O  U32 USBSysErrIntSet;
       U32 RESERVED4[15];

  union {
  __I  U32 I2C_RX;                 /* USB OTG I2C Registers              */
  __O  U32 I2C_TX;
  } cx1;
  __I  U32 I2C_STS;
  __IO U32 I2C_CTL;
  __IO U32 I2C_CLKHI;
  __O  U32 I2C_CLKLO;
       U32 RESERVED5[824];

  union {
  __IO U32 xUSBClkCtrl;             /* USB Clock Control Registers        */
  __IO U32 xOTGClkCtrl;
  } cx2;
  union {
  __I  U32 xUSBClkSt;
  __I  U32 xOTGClkSt;
  } cx3;
} LPC_USB_TypeDef;

#define LPC_USB               ((LPC_USB_TypeDef       *) USBHC_BASE_ADDR     )

#endif
