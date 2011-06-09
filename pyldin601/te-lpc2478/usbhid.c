/*----------------------------------------------------------------------------
**************************************************************************************************************
*               V1.2 (2010.07.22)
*
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
#include <stdlib.h>
 
#include "usbtype.h"
#include "usb.h"
#include "usb_create.h"

//#pragma diag_suppress 111,1441


/*       GLOBAL VARIABLES	*/
struct  usb_device_instance  *p_usbD = NULL;
U8 *p_BF;

#define SIZE_OF_DEVICE_BUFFER  (1024 * 1)

/*
 *  
 *    Parameters:      None
 *    Return Value:    None
 */
void  HID_BuildDeviceInstance (U8 *p_TDBuffer)
{
//  volatile  U8 *p_TDBuffer;
    U8  *pD;
    U32  n, size;

    p_usbD = (struct  usb_device_instance *) malloc( SIZE_OF_DEVICE_BUFFER );
    p_BF = (U8*) ((U32) p_usbD + sizeof(struct  usb_device_instance) );
//   p_usbD->next = NULL;				// no next device

//------ Device Descriptor ----------------
    p_usbD->bNumConfigurations = ((USB_DEVICE_DESCRIPTOR *)p_TDBuffer)->bNumConfigurations;
    p_usbD->ep0_bMaxPacketSize = ((USB_DEVICE_DESCRIPTOR *)p_TDBuffer)->bMaxPacketSize0;	

    size = ((USB_DEVICE_DESCRIPTOR *)p_TDBuffer)->bLength;
    pD = p_BF;
    p_usbD->p_USB_DeviceDescriptor = pD;
    for (n=0; n< size; n++) {
	pD[n] = p_TDBuffer[n];
    }
    p_BF =(U8*) ( ( ((U32) p_BF)+size+3) & ~0x03);	// align by 4
}

/*
 *  
 *    Parameters:      None
 *    Return Value:    None
 */
void HID_CopyConfigurationDescriptor(U32 n, U8* p_TDBuffer)
{
    U8 *pD; 
    U32 i, size;

    //------ Configuration Descriptor ----------------
    size = ((USB_CONFIGURATION_DESCRIPTOR *)p_TDBuffer)->wTotalLength;
//    if (!check_memsize(size ) ){
//  	   return FALSE;
//    }	

    pD = p_BF;
      //--- copying .....
    for (i=0; i< size; i++) {
	pD[i] = p_TDBuffer[i];
    }

    p_BF +=  size;
    if (n == 0) {					// the first configuration descriptor
	p_usbD->p_USB_ConfigDescriptor  = pD;
	// initial  configuration value is the one of the first configuration descriptor
	p_usbD->bUSB_Configuration = ((USB_CONFIGURATION_DESCRIPTOR *)p_TDBuffer)->bConfigurationValue; 
    }

    if ( (n+1) == p_usbD->bNumConfigurations) {
	// if got all configuration descriptors
	p_usbD->wSize_ConfigDescriptor = (p_BF - p_usbD->p_USB_ConfigDescriptor);
	p_BF =(U8*) ( ( ((U32) p_BF)+3) & ~0x03);	// align by 4
    }
}

/*
 *  
 *    Parameters:      None
 *    Return Value:    None
 */
BOOL  HID_ParseCurrentConfiguration(void)
{
    U8 *pD, *end_pD; 
    U32 n;

    pD = p_usbD->p_USB_ConfigDescriptor;
    for (n = 0; n < p_usbD->bNumConfigurations; n++) {   	
	if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bConfigurationValue != p_usbD->bUSB_Configuration) {
	    pD += ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;
	}
    } // END OF for (n = 0; n < p_usbD->bNumConfigurations; n++)
    //<<------------------------

    pD = p_usbD->p_USB_ConfigDescriptor;
    end_pD = pD + ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;
    //>>>---Find its interfaces ---------------
    while(1) {
	// local variables
	pD += ((USB_COMMON_DESCRIPTOR *)pD)->bLength;
	if (pD >= end_pD){
	    break;
	}

	//***** intereface descriptor ******
	if (pD[1] == USB_INTERFACE_DESCRIPTOR_TYPE){
	    if ( ((USB_INTERFACE_DESCRIPTOR *)pD)->bInterfaceClass != USB_DEVICE_CLASS_HUMAN_INTERFACE) {             /* check if the class is HID */
		return (FALSE);			  // NOT HID interface
	    }
	    continue;			// goto while(1)
	}   // END of interface descriptor *********
    }   // END OF while(1) 
    //<<-----------------------------------------------
    return (TRUE);
}

/*
 *  
 *    Parameters:      None
 *    Return Value:    None
 */
void  HID_ExtractConfigurationDescriptor (void)
{
//  volatile  U8 *p_TDBuffer;
    U8  *pD;
    U32 n, size, maxNumIFCs, numRepD;

    //------ HID Report Descriptor  Information Blocks -------------
    pD = p_usbD->p_USB_ConfigDescriptor;
    maxNumIFCs = ((USB_CONFIGURATION_DESCRIPTOR *)pD)->bNumInterfaces ;
    p_usbD->bNumInterfaces = maxNumIFCs;    // the current  configuration is the first configuration,
    numRepD = 0;
    size = 0;

    p_usbD->p_subRepDescriptor = (struct HID_repDesc_info *) p_BF;

    for (n = 0; n < p_usbD->bNumConfigurations; n++) {   	
	// local variables
	U8 *end_pD;
	U32 info0;

	//---  Find p_usbD->bMaxNumInterfaces
	if (maxNumIFCs < ((USB_CONFIGURATION_DESCRIPTOR *)pD)->bNumInterfaces ) {
	    maxNumIFCs = ((USB_CONFIGURATION_DESCRIPTOR *)pD)->bNumInterfaces;
	}																 

        //---  information block of all descriptors of an alternate setting of an interface
	info0 = ((USB_CONFIGURATION_DESCRIPTOR *)pD)->bConfigurationValue;
	end_pD = pD + ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;

	while(1) {
	    pD += pD[0];					// pD[0] = bLength;
	    if (pD >= end_pD){
		break;						// end of configuration descriptor[i]
	    }

	    if (pD[1] == USB_INTERFACE_DESCRIPTOR_TYPE){	// pD[1] = (USB_COMMON_DESCRIPTOR *)pD->bDescriptorType
		info0 &= 0x00FF;	
		info0 |= ( pD[2]<< 8) | ( pD[3]<<16);	
		// pD[2]= ((USB_INTERFACE_DESCRIPTOR *)pD)->bInterfaceNumber
		// pD[3]= ((USB_INTERFACE_DESCRIPTOR *)pD)->bAlternateSetting
		continue;			// goto while(1)
	    }

	    if (pD[1] == HID_HID_DESCRIPTOR_TYPE){	// pD[1] = (USB_COMMON_DESCRIPTOR *)pD->bDescriptorType
		U8 *temp_pD;
		U32 len, i;

		temp_pD = pD;
		len = temp_pD[5];						// temp_pD[5]= bNumDescriptors 
		for (i=0; i< len; i++) {
		    if (temp_pD[6] == HID_REPORT_DESCRIPTOR_TYPE) {
			break;		 		// go out of for loop
		    }
		    temp_pD += 3;				// 1 (bDescriptorType)+ 2(wDescriptorLength)	
		} // END OF for (i=0; i< len; i++) 

		p_BF += sizeof(struct HID_repDesc_info);	   // size of struct will be multiple of 4-bytes after complilation

		p_usbD->p_subRepDescriptor[numRepD].ID= info0;
		p_usbD->p_subRepDescriptor[numRepD].wRepOffset= size;
		p_usbD->p_subRepDescriptor[numRepD].wRepLength= *(__packed U16*) (temp_pD+7);	// temp_pD([8]:[7])= wDescriptorLength
		//<<<<### NOTE: packed  U16, NOT U8 ########  
//---------------NOTE-----NOTE-----NOTE----------------
// ==>          value = (temp_pD[7] | (temp_pD[8]<<8));		// temp_pD([8]:[7])= wDescriptorLength
//<<---------------------------------------------------
		size += p_usbD->p_subRepDescriptor[numRepD].wRepLength;
		numRepD++;
	    }  // END OF  if (ifc_pD[1] == HID_HID_DESCRIPTOR_TYPE)

	}	   // END OF while(1)
    }	// END OF  for (n = 0; n < p_usbD->bNumConfigurations; n++) 
    p_usbD->bMaxNumInterfaces = maxNumIFCs;
    p_usbD->bNumReportDescriptors = numRepD;
    p_usbD->wSize_ReportDescriptor = size;
//    p_BF =(U8*) ( ( ((U32) p_BF)+3) & ~0x03);	// Already align by 4
}


/*
 * 
 *    Parameters:      None
 *    Return Value:    None
 */

void Build_interface_instances (void) {
    U8 *pD;
    U32 size, i;

//==>  p_usbD->p_bAltSetting_perIFC = USB_AltSetting;		/* Array is a pointer */
    size = (U32) p_usbD->bMaxNumInterfaces;

    pD = p_BF;
    p_usbD->p_bAltSetting_perIFC = pD;		/* Array is a pointer */
    for (i = 0; i<size; i++){
	pD[i] = 0;						// set  value 0 to p_usbD->p_bAltSetting_perIFC[i]
    }

    size = (size +3) & ~0x03;					// align by 4
    pD += size;                           	/* U8 p_usbD->p_bAltSetting_perIFC[p_usbD->bMaxNumInterfaces]	 */
//==>	p_usbD->p_bHID_Protocol = HID_Protocol;				/* Array is a pointer */
    p_usbD->p_bHID_Protocol = pD;		/* Array is a pointer */
    for (i = 0; i<size; i++){
	pD[i] = 1;						// set  value 1 to p_usbD->p_bHID_Protocol[i]
    }
    pD += size;	                        /* U8 p_usbD->p_bHID_Protocol[p_usbD->bMaxNumInterfaces]	 */

//==>  p_usbD->p_IFCs = USB_IFCs;		/* Array is a pointer */
    size = sizeof(struct usb_IFC_instance) * p_usbD->bMaxNumInterfaces;    // size of struct will be multiple of 4-bytes after complilation

    p_usbD->p_IFCs = (struct  usb_IFC_instance*)pD;		/* Array is a pointer */
    pD += size;
    p_usbD->p_head_RIDinfo = pD;
    pD += 4;							// 4 bytes for  p_usbD->p_head_RIDinfo
    p_BF = pD;
}

//#######################################
/*
 *  check_memsize
 *    Parameters:      sz: required size of bytes
 *    Return Value:    TRUE/FALSE
 */
static	// A function declared "static" when we do not want this function to be accessed in other files.
BOOL check_memsize(U32 sz) {
    U32	used_size;

    used_size = sz+ ((U32) p_BF - (U32) p_usbD);
    if (used_size > SIZE_OF_DEVICE_BUFFER) {
	return FALSE;
    }
    return TRUE;
}

//############################################################
#define ReportID    0x84
#define ReportSize  0x74
#define ReportCount 0x94
#define MainInput   0x80
#define MainOutput  0x90
#define MainFeature 0xB0
#define Push        0xA4
#define Pop         0xB4

/* Last-----1
 *  Find_EPAddress
 *    Parameters:      pD: pointer of configuration descriptor of specifed configuration value
 *                     p_ifc:   pointer of the interface instance
 *                     info0: (bAlternateSetting: bInterfaceNumber: configuration order)
 *    Return Value:    None
*/
//=== Find the endpoint descriptor for the specified interface
static void	Find_EPAddress(U8* pD, struct  usb_IFC_instance *p_ifc, U32 ifcNo) { 
    U8 *end_pD, nn;

    end_pD = pD + ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;
    nn = p_usbD->p_bAltSetting_perIFC[ifcNo];
    while(1) {
	pD += pD[0];					// pD[0] = bLength;
	if (pD >= end_pD){
	    break;
	}

	if ( (pD[1] == USB_INTERFACE_DESCRIPTOR_TYPE) ){ // pD[1] = (USB_COMMON_DESCRIPTOR *)pD->bDescriptorType
              //---- if matched 
	    if ( (ifcNo == ((USB_INTERFACE_DESCRIPTOR *) pD)->bInterfaceNumber)  &&	 
	       (nn == ((USB_INTERFACE_DESCRIPTOR *)pD)->bAlternateSetting) ) {
		while(1) {
		    pD += pD[0];
		    if ( (pD[1] == USB_INTERFACE_DESCRIPTOR_TYPE) ||	// pD[1] = (USB_COMMON_DESCRIPTOR *)pD->bDescriptorType
		       (pD >= end_pD) ){
			break;
		    }

		    if ( pD[1] == USB_ENDPOINT_DESCRIPTOR_TYPE){	// pD[1] = (USB_COMMON_DESCRIPTOR *)pD->bDescriptorType
			nn = ((USB_ENDPOINT_DESCRIPTOR *)pD)->bEndpointAddress & 0x8F; 
			if ( nn & 0x80) {
			    p_ifc->bInEPaddress = nn;
			    nn = ((USB_ENDPOINT_DESCRIPTOR *)pD)->wMaxPacketSize; 
			    p_ifc->wInMaxPacketSize = nn;
			} else {
			    p_ifc->bOutEPaddress = nn;
			    nn = ((USB_ENDPOINT_DESCRIPTOR *)pD)->wMaxPacketSize; 
			    p_ifc->wOutMaxPacketSize = nn;
			}
		    }
		}	 // END of inner while(1)
	    return;
	    }	//END OF (nn == (info0>>8))
	}
           // local variable for this "if"  case
    }   // END OF while(1) 
}

/* Last---2
 *  item_size
 *    Parameters:      
 *    Return Value:    
 */
static U8 item_size(U8 code) {
    U8 size;
    size = (code & 0x03) + 1;   // clear bits[2:7]
    if (size == 4) {
	size = 5;
    }
    return size;
}

/* Last---3
 *  item_data
 *    Parameters:      
 *    Return Value:    
 */
static U32 item_data(U8* pD, U32 size) {
    U32 data;
    if ((size <2) || (size > 5) ) {
	return (0);
    }
    data =  pD[1];
    if (size > 1+1)
	data |=  pD[2]<<8;
    if (size > 2+1)
	data |=  pD[3]<<16;
    if (size > 3+1)
	data |=  pD[4]<<24;
    return data;
}


/* Last-----4
 *  Find_RIDs
 *    Parameters:      pD: pointer of the report descriptor
 *                     end_pD: end pointer of the report descriptor
 *                
 *    Return Value:    (m_fe : m_out : m_in)
*/
     //--- Find report ID's for this interface ----
static U32 Find_RIDs(U8* pD, U8* end_pD) { 
 U32 fg_mainItem, fg_featureItem;
 U8 *p_PUSH;
 U32 m, max, sp_push;
 U8  code, size, m_in, m_out, m_fe;

// first Step
      p_PUSH = p_BF+1;				   // byte "odd no." for p_PUSH; while byte "even no." for p_BF
      sp_push = 0;
	  max = 0;
	  m = 0;						   // m = order of nonzero report ID from 1; 0 = no report ID	
	  fg_mainItem = 0;
	  fg_featureItem = 0;
      do {
	    U32 i;

         code = pD[0];
         size = item_size(code);
         code &= ~0x03;			 // (bTag:bType:0)
         switch(code){
           //====== MainInput ===========
         case MainInput:
            fg_mainItem |= Bit(16)<< m;		 //  i = bits 31-16 for order of RID m=(15-0)
            break;

           //====== MainOutput ===========
         case MainOutput:
            fg_mainItem |= Bit(0)<< m;  	 //  o = bits 15-0 for order of RID m=(15-0)
            break;

           //====== MainFeature ===========
         case MainFeature:
      	    fg_featureItem |= Bit(0)<< m;  	 //  f = bits 15-0 for order of RID m=(15-0)
            break;

           //====== ReportID ===========
         case ReportID:
            for (i=0; i<max; i++) {
			   if (pD[1] == p_BF[i<<1] ) {
			     break;
			   }
			}
			if ( i < max ) {
			  m = i+1;
			}else {
              p_BF[i<<1] = pD[1];          // save report ID (assume it is a byte)
			  max++;
			  m = max;          				   // m = order of nonzero report ID; 0 = no report ID
			}
			break;

           //====== Push ===========
        case Push:
            p_PUSH[sp_push<<1] = m;          //  push the order of ReportID
		    sp_push ++;
            break;

           //====== Pop ===========
         case Pop:
		    sp_push --;
            m = p_PUSH[sp_push<<1];           //  pop the order of ReportID
            break;

           //====== default
         default:
		    break;
         }	
         pD += size;
      } while(pD < end_pD);

// second Step
	  m_in = 0;			// number of IN report ID's
 	  m_out = 0;		// number of OUT report ID's
 	  m_fe = 0; 		// number of FEATURE report ID's

      for (m=0; m <= max; m++) {
          if ( fg_mainItem & (Bit(16)<<m) ) {
			  m_in ++;
          }          
          if ( fg_mainItem & (Bit(0)<<m) ) {
			  m_out ++;
          }          
          if ( fg_featureItem & (Bit(0)<<m)) {
			  m_fe ++;
          }          
      } // (k=0; k < m; k++)

//third Step
     return  (m_in | (m_out <<8) | (m_fe << 16) );
}


/* Last-----5.1
 * 		Find_index_RID
*/
U32  Find_index_RID(U8 bRID, U32 numRIDs, struct  HID_RID_info *p_info) {
   U32 i;
   
   for (i=0; i<numRIDs; i++) {
      if (p_info[i].bRID == 0 ) {
	     p_info[i].bRID = bRID;
		 break;
	  } else  if (	p_info[i].bRID == bRID ){
	   	 break;
	  }
   }
   return i;
}

/* Last-----5
 *  Find_values_RIDinfo
 *    Parameters:      pD: pointer of the report descriptor
 *                     end_pD: end pointer of the report descriptor
 *                     p_ifc: pointer of the specified interface instance
 *                
 *    Return Value:    (m_fe : m_out : m_in)
*/
     //------- assign values for RIDinfo   
static void Find_values_RIDinfo(U8* pD, U8* end_pD, struct usb_IFC_instance *p_ifc) { 
     U32   i, k, m, size, *p_PUSH; 
	 U32  size_in, size_out, size_fe;
     U8   code, bRID, sp_push; 
	 struct  HID_RID_info *p_in, *p_out, *p_fe;

      p_PUSH = (U32*) p_BF;
	  sp_push =0;
	  p_in = p_ifc->p_inRIDinfo;
	  p_out = p_ifc->p_outRIDinfo;
	  p_fe = p_ifc->p_featureRIDinfo;

      size_in = 0;
      size_out = 0;
      size_fe = 0;
      bRID = 0;
      do {
	    U8 new_RID;

         code = pD[0];
         size = item_size(code);
         code &= ~0x03;			 // (bTag:bType:0)
         switch(code){
           //====== ReportID ===========
         case ReportID:
		    new_RID = pD[1];
lb_sum:
            if (size_in  ) {
               i = Find_index_RID(bRID, p_ifc->bInNumRIDs, p_in);
               p_in[i].wSize += size_in;
               size_in = 0;
            }	
            if (size_out  ) {
               i = Find_index_RID(bRID, p_ifc->bOutNumRIDs, p_out);
               p_out[i].wSize += size_out;
               size_out = 0;
            }	
            if (size_fe  ) {
               i = Find_index_RID(bRID, p_ifc->bFeatureNumRIDs, p_fe);
               p_fe[i].wSize += size_fe;
               size_fe = 0;
            }					 

            bRID = new_RID;             // update RID value
            break;

           //====== ReportSize ===========
         case ReportSize:
            k = item_data(pD, size); 
            break;

           //====== ReportCount ===========
         case ReportCount:
            m = item_data(pD, size); 
            break;

           //====== MainInput ===========
         case MainInput:
            size_in += m*k;
            break;

           //====== MainOutput ===========
         case MainOutput:
            size_out += m*k;
            break;

           //====== MainFeature ===========
         case MainFeature:
            size_fe += m*k;
            break;

          //====== Push ===========
         case Push:
            p_PUSH[sp_push<<2] = k;            // report size 
            p_PUSH[1+sp_push<<2] = m;          // report count
            p_PUSH[2+sp_push<<2] = bRID;        
		    sp_push ++;
            break;

           //====== Pop ===========
         case Pop:
		    sp_push --;
            k = p_PUSH[sp_push<<2];           // report size
            m = p_PUSH[1+sp_push<<2];         // report count
            new_RID = p_PUSH[2+sp_push<<2];
			if (bRID != new_RID) {
			   goto lb_sum ;
			}          
            break;
 
           //====== default
          default:
		    break;
        }	
         pD += size;
      } while(pD < end_pD);
 
            if (size_in  ) {
               i = Find_index_RID(bRID, p_ifc->bInNumRIDs, p_in);
               p_in[i].wSize += size_in;
            }	
            if (size_out  ) {
               i = Find_index_RID(bRID, p_ifc->bOutNumRIDs, p_out);
               p_out[i].wSize += size_out;
            }	
            if (size_fe  ) {
               i = Find_index_RID(bRID, p_ifc->bFeatureNumRIDs, p_fe);
               p_fe[i].wSize += size_fe;
            }	
}

/*
 *  USB_parse_reportDescriptor
 *    Parameters:      None
 *    Return Value:    TRUE / FALSE
 */
BOOL USB_parse_reportDescriptor (void) {
 U8 *pD_conf;
 U32 n;

   p_BF = p_usbD->p_head_RIDinfo;  // free the old RIDinfo buffers
   p_usbD->fg_queue_IFCs = 0;    /* bits 31-16: IFCs of IN transfer requests; bits 15-0: IFCs of Out (or Feature) transfer requests; */  

//== STEP 0:
   pD_conf = p_usbD->p_USB_ConfigDescriptor;
    for (n = 0; n < p_usbD->bNumConfigurations; n++) {   	
      if (((USB_CONFIGURATION_DESCRIPTOR *)pD_conf)->bConfigurationValue != p_usbD->bUSB_Configuration) {
        pD_conf += ((USB_CONFIGURATION_DESCRIPTOR *)pD_conf)->wTotalLength;
      } 
	} // END OF for (n = 0; n < p_usbD->bNumConfigurations; n++)

   for (n = 0; n < p_usbD->bNumInterfaces; n++) {
        //-- local variables
	 struct  usb_IFC_instance *p_ifc;
	 S32 ix;
     U8 *pD, *end_pD, *t_pD;
     U32   i, m, size, temp_m, size_dataBuf;
     U8  m_in, m_out, m_fe;

      p_ifc = &p_usbD->p_IFCs[n];		// pointer of the start of "usb_IFC_instance[n]"
//== STEP 1:
      p_ifc->bInEPaddress = 0;
      p_ifc->bOutEPaddress = 0;
      p_ifc->wOutMaxPacketSize = p_usbD->ep0_bMaxPacketSize;
      p_ifc->fg_queue_RIDs = 0;
      p_ifc->fg_wFeatureQueue_RIDs = 0;

      Find_EPAddress(pD_conf, p_ifc, n);

//== STEP 2:
      temp_m = p_usbD->bUSB_Configuration;
      temp_m |= n<<8;
      //==> temp_m |= (p_usbD->p_bAltSetting_perIFC[n])<<16;     /* initial alternate settings are all 0 */

      for (ix =p_usbD->bNumReportDescriptors-1; ix >=0; ix--) {
          if (temp_m == (p_usbD->p_subRepDescriptor[ix].ID & 0xFFFF)) {	   // the largest alternate setting
             temp_m =  p_usbD->p_subRepDescriptor[ix].ID; 
             break;
          }          
      } // END OF for ( ; i < p_usbD->bNumReportDescriptors; i++)
      size = 0;
      size_dataBuf = 0;

lb_do_find_RIDs:
//== STEP 2.1:
      pD = p_usbD->p_HID_ReportDescriptor;
      pD += p_usbD->p_subRepDescriptor[ix].wRepOffset;
      end_pD = pD + p_usbD->p_subRepDescriptor[ix].wRepLength;
     //--- Find report ID's for this interface ----
      m = Find_RIDs(pD, end_pD);

	  m_in = m & 0xFF;
	  m_out = (m>>8) & 0xFF;
	  m_fe = (m>>16) & 0xFF;

	  m = m_in + m_out + m_fe;
	  if (size < m) {
	     size = m;
	  }
	  
     //>>>>=== From the largest to the default Setting, which is the one with (alternate setting == 0)
           //----  allocate space for "struct  HID_RID_info"      
        p_ifc->bInNumRIDs = m_in;				 // will be overwritten when alternate setting = 0
        p_ifc->bOutNumRIDs = m_out;				 // will be overwritten when alternate setting = 0
        p_ifc->bFeatureNumRIDs = m_fe;  		 // will be overwritten when alternate setting = 0

		t_pD = p_BF;
		m = size * sizeof(struct HID_RID_info);
        if (!check_memsize(m) ) {
             return FALSE;
        }
		p_BF +=m;
		 
		  // clear variables
 	    for (i =0; i< (m/4); i++) {			 // 4 bytes at one time
          ((U32*)t_pD)[i] = 0;					  // clear the assigned memory
	    }

        if ( m_in) {
            p_ifc->p_inRIDinfo = (struct HID_RID_info *) t_pD;
            t_pD += m_in * sizeof(struct HID_RID_info);	  		// size of struct will be multiple of 4-bytes after complilation
	    } else {
            p_ifc->p_inRIDinfo = NULL;
	    }

	    if ( m_out) {
            p_ifc->p_outRIDinfo = (struct HID_RID_info *) t_pD;
            t_pD += m_out * sizeof(struct HID_RID_info);	  		// size of struct will be multiple of 4-bytes after complilation
	    } else {
            p_ifc->p_outRIDinfo = NULL;
	    }

	    if ( m_fe) {
            p_ifc->p_featureRIDinfo = (struct HID_RID_info *) t_pD;	
	    } else {
            p_ifc->p_featureRIDinfo = NULL;
	    }

       //== STEP 2.2:
       //------- assign values for RIDinfo   
	     Find_values_RIDinfo(pD, end_pD, p_ifc);

		 //>>>--  correct wSize for each "struct  HID_RID_info"
		 m = 0;
         for (i=0; i < (m_in + m_out + m_fe); i++) {
		    U32 ns;
#define p_RID p_ifc->p_inRIDinfo

			 ns = (p_RID[i].wSize +7) /8;
			 if (p_RID[i].bRID)  ns = ns+1;   //  The first byte is report ID value
			 p_RID[i].wSize = ns;
//			   m += (ns + 3) & ~0x03;			// align to 4-bit boundary
			 m += ns;
		 }
		 //<<<<<--------------------

 	     if (size_dataBuf < m) {
	         size_dataBuf = m;
	     }

	  //<<<<<<<<<<<<<<<<<<<<<=====================  

      if ( ((temp_m >> 16) != 0) && (ix > 0) ) {
	     ix --; 
	     if (p_usbD->p_subRepDescriptor[ix].ID == (temp_m - Bit(16)) ) {
		     p_BF -= size * sizeof(struct HID_RID_info);
             goto lb_do_find_RIDs;
         }
	  }	           
	  //<<<<<<<<<<<<<<<<<<<<<=====================  
		  // continue if no more alternate setting (i.e., 0) for this interface
      p_ifc->bMaxTotalRIDs = size;           /* maximum number of total RIDs of this interface for different alternate settings  */
      p_ifc->maxDataBuffer = size_dataBuf;    /* maximum size of data buffer in Byte for this interface instance */
   } // END OF for (n = 0; n < p_usbD->bNumInterfaces; n++)


//== STEP 3:
     //#######  allocate space for RID data buffers
   for (n = 0; n < p_usbD->bNumInterfaces; n++) {
	 struct  usb_IFC_instance *p_ifc;
     U32   k, size;
     U8  *p_U8;

       p_ifc = &p_usbD->p_IFCs[n];		// pointer of the start of "usb_IFC_instance[n]"
       size = p_ifc->maxDataBuffer;    /* maximum size of data buffer in Byte for this interface instance */
       if (!check_memsize(size) ){
          return FALSE;
       }
	   p_U8 = p_BF;
	   p_BF += size;

       for (k=0; k< p_ifc->bInNumRIDs; k++) {
          p_ifc->p_inRIDinfo[k].wLeftSize = 0;       // clear  it
          p_ifc->p_inRIDinfo[k].pData = p_U8;

		  size = p_ifc->p_inRIDinfo[k].wSize;
//          p_U8 += (size+3) & ~0x03;	// align by 4
          p_U8 += size;	
       }	// END of for (i=0; i< p_ifc->bInNumRIDs; i++)

       for (k=0; k< p_ifc->bOutNumRIDs; k++) {
          p_ifc->p_outRIDinfo[k].wLeftSize = 0;       // clear  it
          p_ifc->p_outRIDinfo[k].pData = p_U8;

		  size = p_ifc->p_outRIDinfo[k].wSize;
//          p_U8 += (size+3) & ~0x03;	// align by 4
          p_U8 += size;	
       }	// END of for (i=0; i< p_ifc->bOutNumRIDs; i++)
	       
       for (k=0; k< p_ifc->bFeatureNumRIDs; k++) {
          p_ifc->p_featureRIDinfo[k].wLeftSize = 0;       // clear  it
          p_ifc->p_featureRIDinfo[k].pData = p_U8;

		  size = p_ifc->p_featureRIDinfo[k].wSize;
//          p_U8 += (size+3) & ~0x03;	// align by 4
          p_U8 += size;	
       }	// END of for (i=0; i< p_ifc->bFeatureNumRIDs; i++)

   } // END OF for (n = 0; n < p_usbD->bNumInterfaces; n++)
  		
   return TRUE;
}

