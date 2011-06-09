/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    USB.H
 *      Purpose: USB Definitions
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *
 *---------------------------------------------------------------------------*/

#ifndef __USB_H__
#define __USB_H__

#define __packed  __attribute__((packed))

typedef union {
  U16 W;
  struct {
    U8 L;
    U8 H;
  } __packed WB;
} __packed U16_U8;


/* bmRequestType.Dir */
#define REQUEST_HOST_TO_DEVICE     0
#define REQUEST_DEVICE_TO_HOST     1

#define  USB_DEVICE_TO_HOST         0x80
#define  USB_HOST_TO_DEVICE         0x00
#define  USB_REQUEST_TYPE_STANDARD  0x00
#define  USB_REQUEST_TYPE_CLASS     0x20
#define  USB_REQUEST_TYPE_VENDOR    0x40

/* bmRequestType.Type */
#define REQUEST_STANDARD           0
#define REQUEST_CLASS              1
#define REQUEST_VENDOR             2
#define REQUEST_RESERVED           3

/* bmRequestType.Recipient */
#define REQUEST_TO_DEVICE          0
#define REQUEST_TO_INTERFACE       1
#define REQUEST_TO_ENDPOINT        2
#define REQUEST_TO_OTHER           3

/* bmRequestType Definition */
typedef union _REQUEST_TYPE {
  struct _BM {
    U8 Recipient : 5;
    U8 Type      : 2;
    U8 Dir       : 1;
  } __packed BM;
  U8 B;
} __packed REQUEST_TYPE;

/* USB Standard Request Codes */
#define USB_REQUEST_GET_STATUS                 0
#define USB_REQUEST_CLEAR_FEATURE              1
#define USB_REQUEST_SET_FEATURE                3
#define USB_REQUEST_SET_ADDRESS                5
#define USB_REQUEST_GET_DESCRIPTOR             6
#define USB_REQUEST_SET_DESCRIPTOR             7
#define USB_REQUEST_GET_CONFIGURATION          8
#define USB_REQUEST_SET_CONFIGURATION          9
#define USB_REQUEST_GET_INTERFACE              10
#define USB_REQUEST_SET_INTERFACE              11
#define USB_REQUEST_SYNC_FRAME                 12

/* USB GET_STATUS Bit Values */
#define USB_GETSTATUS_SELF_POWERED             0x01
#define USB_GETSTATUS_REMOTE_WAKEUP            0x02
#define USB_GETSTATUS_ENDPOINT_STALL           0x01

/* USB Standard Feature selectors */
#define USB_FEATURE_ENDPOINT_STALL             0
#define USB_FEATURE_REMOTE_WAKEUP              1

/* USB Default Control Pipe Setup Packet */
typedef struct _USB_SETUP_PACKET {
  REQUEST_TYPE bmRequestType;
  U8         bRequest;
  U16_U8    wValue;
  U16_U8    wIndex;
  U16         wLength;
} __packed USB_SETUP_PACKET;


/* USB Descriptor Types */
#define USB_DEVICE_DESCRIPTOR_TYPE             1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE      2
#define USB_STRING_DESCRIPTOR_TYPE             3
#define USB_INTERFACE_DESCRIPTOR_TYPE          4
#define USB_ENDPOINT_DESCRIPTOR_TYPE           5
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE   6
#define USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE 7
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE    8

/* USB Device Classes */
#define USB_DEVICE_CLASS_RESERVED              0x00
#define USB_DEVICE_CLASS_AUDIO                 0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS        0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE       0x03
#define USB_DEVICE_CLASS_MONITOR               0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE    0x05
#define USB_DEVICE_CLASS_POWER                 0x06
#define USB_DEVICE_CLASS_PRINTER               0x07
#define USB_DEVICE_CLASS_STORAGE               0x08
#define USB_DEVICE_CLASS_HUB                   0x09
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC       0xFF

/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                0xC0
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

/* bEndpointAddress in Endpoint Descriptor */
#define USB_ENDPOINT_DIRECTION_MASK            0x80
#define USB_ENDPOINT_OUT(addr)                 ((addr) | 0x00)
#define USB_ENDPOINT_IN(addr)                  ((addr) | 0x80)

/* bmAttributes in Endpoint Descriptor */
#define USB_ENDPOINT_TYPE_MASK                 0x03
#define USB_ENDPOINT_TYPE_CONTROL              0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS          0x01
#define USB_ENDPOINT_TYPE_BULK                 0x02
#define USB_ENDPOINT_TYPE_INTERRUPT            0x03
#define USB_ENDPOINT_SYNC_MASK                 0x0C
#define USB_ENDPOINT_SYNC_NO_SYNCHRONIZATION   0x00
#define USB_ENDPOINT_SYNC_ASYNCHRONOUS         0x04
#define USB_ENDPOINT_SYNC_ADAPTIVE             0x08
#define USB_ENDPOINT_SYNC_SYNCHRONOUS          0x0C
#define USB_ENDPOINT_USAGE_MASK                0x30
#define USB_ENDPOINT_USAGE_DATA                0x00
#define USB_ENDPOINT_USAGE_FEEDBACK            0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FEEDBACK   0x20
#define USB_ENDPOINT_USAGE_RESERVED            0x30

/* USB Standard Device Descriptor */
typedef struct _USB_DEVICE_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U16  bcdUSB;
  U8  bDeviceClass;
  U8  bDeviceSubClass;
  U8  bDeviceProtocol;
  U8  bMaxPacketSize0;
  U16  idVendor;
  U16  idProduct;
  U16  bcdDevice;
  U8  iManufacturer;
  U8  iProduct;
  U8  iSerialNumber;
  U8  bNumConfigurations;
} __packed USB_DEVICE_DESCRIPTOR;

/* USB 2.0 Device Qualifier Descriptor */
typedef struct _USB_DEVICE_QUALIFIER_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U16  bcdUSB;
  U8  bDeviceClass;
  U8  bDeviceSubClass;
  U8  bDeviceProtocol;
  U8  bMaxPacketSize0;
  U8  bNumConfigurations;
  U8  bReserved;
} __packed USB_DEVICE_QUALIFIER_DESCRIPTOR;

/* USB Standard Configuration Descriptor */
typedef struct _USB_CONFIGURATION_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U16  wTotalLength;
  U8  bNumInterfaces;
  U8  bConfigurationValue;
  U8  iConfiguration;
  U8  bmAttributes;
  U8  bMaxPower;
} __packed USB_CONFIGURATION_DESCRIPTOR;

/* USB Standard Interface Descriptor */
typedef struct _USB_INTERFACE_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U8  bInterfaceNumber;
  U8  bAlternateSetting;
  U8  bNumEndpoints;
  U8  bInterfaceClass;
  U8  bInterfaceSubClass;
  U8  bInterfaceProtocol;
  U8  iInterface;
} __packed USB_INTERFACE_DESCRIPTOR;

/* USB Standard Endpoint Descriptor */
typedef struct _USB_ENDPOINT_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U8  bEndpointAddress;
  U8  bmAttributes;
  U16  wMaxPacketSize;
  U8  bInterval;
} __packed USB_ENDPOINT_DESCRIPTOR;

/* USB String Descriptor */
typedef struct _USB_STRING_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U16  bString/*[]*/;
} __packed USB_STRING_DESCRIPTOR;

/* USB Common Descriptor */
typedef struct _USB_COMMON_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
} __packed USB_COMMON_DESCRIPTOR;


#endif  /* __USB_H__ */


#ifndef __USBDESC_H__
#define __USBDESC_H__

/* HID Descriptor */
typedef struct _HID_DESCRIPTOR {
  U8  bLength;
  U8  bDescriptorType;
  U16  bcdHID;
  U8  bCountryCode;
  U8  bNumDescriptors;
  /* Array of one or more descriptors */
  __packed struct _HID_DESCRIPTOR_LIST {
    U8  bDescriptorType;
    U16  wDescriptorLength;
  } DescriptorList[1];
} __packed HID_DESCRIPTOR;


/* HID Request Codes */

#define HID_REQUEST_GET_REPORT          0x01
#define HID_REQUEST_GET_IDLE            0x02
#define HID_REQUEST_GET_PROTOCOL        0x03
#define HID_REQUEST_SET_REPORT          0x09
#define HID_REQUEST_SET_IDLE            0x0A
#define HID_REQUEST_SET_PROTOCOL        0x0B

/* HID Report Types */
#define HID_REPORT_INPUT                0x01
#define HID_REPORT_OUTPUT               0x02
#define HID_REPORT_FEATURE              0x03

/* HID Descriptor Types */
//#ifndef __HID_Type__
//#define __HID_Type__

#define HID_HID_DESCRIPTOR_TYPE         0x21
#define HID_REPORT_DESCRIPTOR_TYPE      0x22
#define HID_PHYSICAL_DESCRIPTOR_TYPE    0x23

//#endif  /* ____HID_Type___ */



#endif  /* __USBDESC_H__ */
