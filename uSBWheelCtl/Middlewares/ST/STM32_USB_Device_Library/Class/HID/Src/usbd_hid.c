/**
  ******************************************************************************
  * @file    usbd_hid.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application 
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "ausb.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 

uint16_t classReqHistory;
uint8_t usbd_hid_EP0rxBuf[32]; /*0x14 = 20DEC; buffer used to receive PID CLASS reports data on EP0*/
uint8_t usbd_hid_EP1rxBuf[AUSB_EP1OutMaxPacketSize]; /*0x14 = 20DEC; buffer used to receive Out data on EP1*/
static uint8_t newDeviceDataAvailable;
 
/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_HID_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_HID_EP0_RxReady(USBD_HandleTypeDef *pdev );

static uint8_t  USBD_HID_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_HID_DataOut(USBD_HandleTypeDef *pdev , uint8_t epnum);
/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Variables
  * @{
  */ 

USBD_ClassTypeDef  USBD_HID = 
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/  
  USBD_HID_EP0_RxReady, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  USBD_HID_DataOut, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,      
  USBD_HID_GetCfgDesc,
  USBD_HID_GetCfgDesc, 
  USBD_HID_GetCfgDesc,
  USBD_HID_GetDeviceQualifierDesc,
};

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[JOYSTICK_SIZ_CONFIG_DESC]  __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  JOYSTICK_SIZ_CONFIG_DESC,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xE0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  
  /************** Descriptor of Joystick Mouse interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  (JOYSTICK_SIZ_REPORT_DESC%0x100u), /*wItemLength: Total length of Report descriptor*/
  (JOYSTICK_SIZ_REPORT_DESC/0x100u),
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  AUSB_EP1InMaxPacketSize, /*wMaxPacketSize */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
  /* 34 */
  0x07,	/* bLength: EndpoinT Descriptor size */
	USB_DESC_TYPE_ENDPOINT,	/* bDescriptorType: */ /*	EndpoinT descriptor type */
	HID_EPOUT_ADDR,	/* bEndpointAddress: */ /*	EndpoinT Address (OUT) */
	0x03,	/* bmAttributes: Interrupt endpoinT */
	AUSB_EP1OutMaxPacketSize,	/* wMaxPacketSize: Now the report with maximum size is Set Effect Report (but is less than 16 bytes so it's OK)  */
	0x00,
	3,	/* bInterval: Polling Interval */
	/* 41 */
} ;

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ]  __ALIGN_END  =
{
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  (JOYSTICK_SIZ_REPORT_DESC%0x100u), /*wItemLength: Total length of Report descriptor*/
  (JOYSTICK_SIZ_REPORT_DESC/0x100u),
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

__ALIGN_BEGIN static uint8_t Joystick_ReportDescriptor[JOYSTICK_SIZ_REPORT_DESC]  __ALIGN_END =
{
 		  0x05, 0x01,         		/*  Usage Page (Generic Desktop),           [12] */
			   0x09, 0x04,         	/*  Usage (JoyStik),                        */
			   0xA1, 0x01,         	/*  Collection (Application),               */
			   0x85, 0x01,			/* REPORT_ID (1)*/
			   //0xA1, 0x02,        /* Collection (Logical) */
			   0x09, 0x01,			/*usage pointer*/
			    0xA1, 0x00,			/* Collection (Physical) */
			     /*X axis - the wheel axis										[21]*/
			   	 0x09, 0x30,        /* Usage (X),                      */
			     0x16, 0x01, 0x80,  /* Logical Minimum (-32767),       */
			     0x26, 0xFF, 0x7F, 	/* Logical Maximum (32767),          */
			     0x35, 0x00,        /* Physical Minimum (0),           */
			     0x47, 0xFF, 0xFF, 0x00, 0x00, 	/* Physical Maximum (65k),         */
				 0x75, 0x10,        /* Report Size (16),               */
				 0x95, 0x01,        /* Report Count (1),               */
			     0x81, 0x02,        /* Input (Variable),               */
				 /*Y axis - Acceleration PedaL									[17]*/
				 0x09, 0x31,         	/*          Usage (Y),                      */
				 0x15, 0x00, /*0x81,*/           	/*          Logical Minimum 0 (-127),        	*/
			     0x25, 0xFF, /*0x7F*/			/*      	Logical Maximum 255 (127),          */
			     0x34,                  /*      	Physical Minimum (0),           */
			     0x46, 0xFF, 0x00,   	/*       	Physical Maximum (255),         */
			     0x75, 0x08,         	/*          Report Size (8),                */
				 0x95, 0x01,         	/*          Report Count (1),               */
			     0x81, 0x02,         	/*          Input (Variable),               */
			    0xC0,				/* End collection (Physical)*/
				/*rZ axis - Brake PedaL											[16]*/
				0x09, 0x35,         	/*          Usage (RZ),                     */
				0x15, 0x00,             /*          Logical Minimum (0),            */
			    0x26, 0xFF, 0x00,   	/*       	Logical Maximum (255),          */
			    0x46, 0xFF, 0x00,   	/*       	Physical Maximum (255),         */
				0x95, 0x01,         	/*          Report Count (1),               */
			    0x75, 0x08,         	/*          Report Size (8),                */
			    0x81, 0x02,         	/*          Input (Variable),               */
				//0xA1, 0x02,        /* Collection (Logical) 						*/
				/*4 buttons (can be Console Style: [] /\ 0 X					[16]*/
				0x05, 0x09,         	/*          Usage Page (Button),            */
			    0x95, 0x04,         	/*          Report Count (4),               */
			    0x75, 0x01,         	/*          Report Size (1),                */
			    0x25, 0x01,         	/*          Logical Maximum (1),            */
			    0x45, 0x01,         	/*          Physical Maximum (1),           */
			    0x19, 0x01,         	/*          Usage Minimum (01h),            */
			    0x29, 0x04,         	/*          Usage Maximum (04h),            */
			    0x81, 0x02,         	/*          Input (Variable),               */
				/*Hat Switch													[20]*/
			    //0x05, 0x01,         	/*          Usage Page (Generic Desktop),   */
				0x09, 0x39,         	/*          Usage (Hat Switch),             */
				0x15, 0x01,             /*          Logical Minimum,            	*/
				0x25, 0x08,         	/*          Logical Maximum					*/
				0x35, 0x00,         	/*          Physical Minimum				*/
				0x46, 0x3B, 0x01, 		/*          Physical Maximum (315),         */
				0x66, 0x14, 0x00,     	/*          Unit (Degrees),                 */
				0x75, 0x04,         	/*          Report Size (4),                */
			    0x95, 0x01,         	/*          Report Count (1),               */
			    0x81, 0x02,         	/*          Input (Variable),               */
			   //0xC0,               		/*      End Collection,  (Logical),         */
			   /*start Force Feedback definitions*/
			   0x05, 0x0F,			/*Usage_Page (Physical Interface)				[2]*/
			   /*Set Effect report													[6]*/
			   0x09, 0x21,			/*USAGE (Set effect report)*/
			   0xA1, 0x02, 			/*COLLECTION (Logical)*/
			   0x85, 0x01,			/* REPORT_ID (1)*/
			    /*Effect Block Index												[16]*/
			    0x09, 0x22,			/*USAGE (Effect Block Index)*/
				0x15, 0x01,			/*LOGICAL_MINIMUM (1)*/
			    0x25, 0x20,			/*LOGICAL_MAXIMUM (32)*/
				0x35, 0x01,        	/*Physical Minimum*/
				0x45, 0x20, 		/*Physical Maximum*/
			    0x75, 0x08,			/*REPORT_SIZE (8)*/
			    0x95, 0x01,			/*REPORT_COUNT (1)*/
			    0x91, 0x02,			/*OUTPUT (Data,VaR, AbS)*/
			    /*Effect type														[21]*/
			    0x09, 0x25,			/*USAGE (Effect Type)*/
			    0xA1, 0x02,			/*COLLECTION (Logical)*/
			     0x09, 0x26,		/*USAGE (ET Constant Force)*/
				 0x09, 0x43,		/*USAGE (ET Friction)*/
			     0x15, 0x01,		/*LOGICAL_MINIMUM*/
				 0x25, 0x02,		/*LOGICAL_MAXIMUM*/
				 0x35, 0x01,        /*Physical Minimum*/
				 0x45, 0x02, 		/*Physical Maximum*/
				 0x75, 0x08,		/*REPORT_SIZE (8)*/
				 0x91, 0x00,		/*OUTPUT (Data,ArY,AbS)*/
				0xC0,				/*END COLLECTION*/
				/*Duration and 	Trigger Repeat Interval									[25]*/
				0x09, 0x50,			/*USAGE (Duration)*/
				0x09, 0x54,			/*USAGE (Trigger Repeat Interval)*/
				0x15, 0x00,			/*LOGICAL_MINIMUM (0)*/
				0x26, 0x10, 0x27,	/*LOGICAL_MAXIMUM (10000)*/
				0x35, 0x00,         /*Physical Minimum*/
				0x46, 0x10, 0x27,	/*PHYSICAL_MAXIMUM (10000)*/
				0x66, 0x03, 0x10,	/*UNIT (ENG Lin:Time)*/
				0x55, 0x0D,			/*UNIT_EXPONENT (-3)*/
				0x75, 0x10,			/*REPORT_SIZE (16)*/
				0x95, 0x02,			/*REPORT_COUNT (2) - XXX 2 is for Duration and Trigger Repeat Interval ? XXX*/
				0x91, 0x02,			/*OUTPUT (Data,VaR,AbS)*/
				/*Sample period													[6]*/
				0x09, 0x51,			/*USAGE (Sample Period)*/
				0x95, 0x01,			/*REPORT_COUNT (1) */
				0x91, 0x02,			/*OUTPUT (Data,VaR,AbS)*/
				/*GLOBAL Defines												[7]*/
				0x45, 0x00,			/*PHYSICAL_MAXIMUM (0)*/
				0x55, 0x00, 		/*UNIT_EXPONENT (0)*/
				0x66, 0x00, 0x00,	/*UNIT (None)*/
				/*Gain 															[18]*/
				0x09, 0x52,			/*USAGE (Gain)*/
				0x15, 0x00,			/*LOGICAL_MINIMUM*/
				0x26, 0x10, 0x27,	/*LOGICAL_MAXIMUM (10000)*/
				0x35, 0x00,         /*Physical Minimum*/
				0x46, 0x10, 0x27,	/*PHYSICAL_MAXIMUM (10000)*/
				0x75, 0x10,			/*REPORT_SIZE (16)*/
				0x95, 0x01,			/*REPORT_COUNT*/
				0x91, 0x02,			/*OUTPUT (Data,VaR,AbS)*/
				/*Trigger Button 												[16]*/
				0x09, 0x53,			/*USAGE (Trigger Button)*/
				0x15, 0x01,			/*LOGICAL_MINIMUM*/
				0x25, 0x08,			/*LOGICAL_MAXIMUM*/
				0x35, 0x01,         /*Physical Minimum*/
				0x45, 0x08,			/*PHYSICAL_MAXIMUM*/
				0x75, 0x08,			/*REPORT_SIZE*/
				0x95, 0x01,			/*REPORT_COUNT*/
				0x91, 0x02,			/*OUTPUT (Data,VaR,AbS)*/
				/*Axes enable 															[22]*/
				0x09, 0x55,			/*USAGE (Axes Enable) ; Tie these axes to the stick*/
				0xA1, 0x02,			/*COLLECTION (Logical)*/
				 0x05, 0x01,		/*USAGE_PAGE (Generic Desktop)*/
				 0xA1, 0x00,		/*COLLECTION (PHYSICAL)*/	
				  0x09, 0x30,		/*USAGE (X) - Wheel Axis*/
				  0x15, 0x00,		/*LOGICAL_MINIMUM (0)*/
				  0x25, 0x01,		/*LOGICAL_MAXIMUM (1)*/
				  0x75, 0x01,		/*REPORT_SIZE (1)*/
				  0x95, 0x01,		/*REPORT_COUNT (1)*/
				  0x91, 0x02,		/*OUTPUT (Data,VaR,AbS)*/
				 0xC0,				/*END_COLLECTION (physical)*/ 
				0xC0,				/*END_COLLECTION (logical)*/
				/*Direction enable - uses one more bit in the byte for axes enable		[12]*/
				0x05, 0x0F, 		/*USAGE_PAGE (Physical Interface)*/
				0x09, 0x56,			/*USAGE (Direction enable)*/
				0x95, 0x01, 		/* REPORT_COUNT (1) */
				0x91, 0x02,			/* OUTPUT (Data,VaR,AbS)*/
				0x95, 0x06, 		/* REPORT_COUNT (6 - one bit used by axes enable and one by direction enable) */
				0x91, 0x03,			/* OUTPUT (Constant,VaR,AbS); bit padding*/
				/*Direction 															[43]*/
				0x09, 0x57,			/*USAGE (Direction)*/
				0xA1, 0x02,			/*COLLECTION (Logical)*/
				  0x0B, 0x01, 0x00, 0x0A, 0x00,	/*USAGE (Ordinals:Instance 1)*/
				  0x0B, 0x02, 0x00, 0x0A, 0x00,	/*USAGE (Ordinals:Instance 2)*/
				  0x65, 0x14,		/*unit degrees*/
				  0x55, 0xFE,/*was 0x0E,*/	/*unit exponent ? (was -2)*/
				  0x15, 0x00,		/*LOGICAL_MINIMUM (0)*/
				  0x26, 0xFF, 0x00,	/*LOGICAL_MAXIMUM (255)*/
				  0x35, 0x00,		/*PHYSICAL_MINIMUM (0 - use logical limits)*/
				  0x47, 0xa0, 0x8c, 0x00, 0x00, /*was0x45, 0x00,*/	/*PHYSICAL_MAXIMUM (36000) (was 0 - use logical limits)*/
				  0x65, 0x00,		/*UNIT (0)*/
				  0x75, 0x08,		/*REPORT_SIZE (8)*/
				  0x95, 0x02,		/*REPORT_COUNT*/
				  0x91, 0x02,		/*OUTPUT (Data,VaR,AbS)*/
				  0x55, 0x00,	  	/*Unit exponent (0)*/
				  0x65, 0x00,		/*UNIT (None) */
				0xC0,				/*END_COLLECTION (Direction)*/
				// /*Start delay												[not used 21]*/
				// 0x09, 0xA7,			/*USAGE (Start Delay)*/
				// 0x15, 0x00,			/*LOGICAL_MINIMUM (0)*/
				// 0x26, 0x10, 0x27,	/*LOGICAL_MAXIMUM (10000)*/
				// 0x46, 0x10, 0x27,	/*PHYSICAL_MAXIMUM (10000)*/
				// 0x75, 0x10,			/*REPORT_SIZE (16)*/
				// 0x66, 0x03, 0x10,	/*UNIT (ENG Lin:Time)*/
				// 0x55, 0x0D,			/*UNIT_EXPONENT (-3)*/
				// 0x95, 0x01,			/*REPORT_COUNT (1)*/
				// 0x91, 0x02,			/*OUTPUT (Data,VaR,AbS)*/
				// /*GLOBAL Defines												[not used 6]*/
				// 0x45, 0x00,			/*PHYSICAL_MAXIMUM (0)*/
				// 0x55, 0x00, 		/*UNIT_EXPONENT (0)*/
				// 0x65, 0x00,			/*UNIT (None)*/
				// /*Type Specific Block Offset 								[not used 21]*/
				// 0x05, 0x0F, 		/*USAGE_PAGE (Physical Interface)*/
				// 0x09, 0x58,			/*USAGE (Type Specific Block Offset)*/
				// 0xA1, 0x02, 			/*COLLECTION (Logical)*/
				//  0x0B, 0x01, 0x00, 0x0A, 0x00,	/*USAGE (Ordinals:Instance 1)*/
				//  //0x0B, 0x02, 0x00, 0x0A, 0x00,	/*USAGE (Ordinals:Instance 2)*/
				//  0x26, 0x20, 0x00,		/*LOGICAL_MAXIMUM (32 XXX was 32765: 0xFD, 0x7F)*/
				//  0x75, 0x10,			/*REPORT_SIZE (16)*/
				//  0x95, 0x01,			/*REPORT_COUNT (1)*/
				//  0x91, 0x02,			/*OUTPUT (Data,VaR,AbS)*/
				// 0xC0,					/*END_COLLECTION*/
				/*END of Set Effect report									[1]*/
			   0xC0, 					/*END COLLECTION ( Effect report )*/
			   /*Constant Force Report Definition 							[41]*/
			   0x09, 0x73,			/*USAGE (Set Constant Force Report)*/
			   0xA1, 0x02,			/*COLLECTION (Logical) */
			    0x85, 0x03,				/*REPORT_ID (3) */
			    //0x09, 0x23,				/*USAGE (Parameter Block Offset); TODO if problems: use 22 (Effect Block Index): the linuX driver hid-pidff.c expects 0x22 (also the SideWinder example uses 0x22)*/
			    0x09, 0x22,				/*Effect Block Index */
				0x15, 0x01,				/*LOGICAL_MINIMUM (1)*/
				0x25, 0x20,			    /*LOGICAL_MAXIMUM (32)*/
				0x35, 0x01,				/*PHYSICAL_MINIMUM (1)*/
				0x45, 0x20,				/*PHYSICAL_MAXIMUM (32)*/
			    0x75, 0x08,				/*REPORT_SIZE (8)*/
			    0x95, 0x01,				/*REPORT_COUNT (1)*/
			    0x91, 0x02,				/*OUTPUT (Data,VaR,AbS)*/
			    0x09, 0x70,				/*USAGE (Magnitude) */
				0x16, 0xF0, 0xD8,		/*LOGICAL_MINIMUM (-10000)*/
			    0x26, 0x10, 0x27,		/*LOGICAL_MAXIMUM (10000) */
				0x36, 0xF0, 0xD8,		/*PHYSICAL_MINIMUM (-10000)*/
				0x46, 0x10, 0x27,		/*PHYSICAL_MAXIMUM (10000)*/
			    0x75, 0x10,				/*REPORT_SIZE (16)*/
			    0x91, 0x02,				/*OUTPUT (Data,VaR,AbS) */
			   0xC0,					/*END_COLLECTION */
			   /*Envelope Report Definition										[58] */
			   0x09, 0x5A,	/*USAGE (Set Envelope Report) */
			   0xA1, 0x02,	/*COLLECTION (Logical)*/
			    0x85, 0x02, 		/* REPORT_ID (2)*/
			    0x09, 0x22, 		/* USAGE (Effect Block Index) XXX was 0x23 USAGE (Parameter Block Offset) but the linuX driver hid-pidff.c expects 0x22*/
				0x15, 0x01,			/* LOGICAL_MINIMUM (1)*/
			    0x25, 0x20,			/* LOGICAL_MAXIMUM (32)*/
				0x35, 0x01,			/* PHYSICAL_MINIMUM (1)*/
				0x45, 0x20,			/* PHYSICAL_MAXIMUM (32)*/
			    0x75, 0x08, 		/* REPORT_SIZE (8)*/
			    0x95, 0x01, 		/* REPORT_COUNT (1)*/
			    0x91, 0x02, 		/* OUTPUT (Data,VaR,AbS) */
			    0x09, 0x5B, 		/* USAGE (Attack Level) */
			    0x09, 0x5D,	 		/* USAGE (Fade Level) */
				0x15, 0x00,			/* LOGICAL_MINIMUM (0)*/
			    0x26, 0xFF, 0x00,	/* LOGICAL_MAXIMUM (255) */
				0x46, 0x10, 0x27, 	/* PHYSICAL_MAXIMUM (10000)  */
			    0x75, 0x08, 		/* REPORT_SIZE (8)*/
			    0x95, 0x02, 		/* REPORT_COUNT (2)*/
			    0x91, 0x02,			/* OUTPUT (Data,VaR,AbS)*/
			    0x09, 0x5C, 		/* USAGE (Attack Time) */
			    0x09, 0x5E, 		/* USAGE (Fade Time)  */
			    0x66, 0x03, 0x10, 	/* UNIT (EnG Lin:Time) */
			    0x55, 0x0D, 		/* UNIT_EXPONENT (-3)*/
			    0x75, 0x08, 		/* REPORT_SIZE*/
			    0x91, 0x02, 		/* OUTPUT (Data,VaR,AbS) */
			    0x65, 0x00, 		/* UNIT (None) */
			    0x55, 0x00, 		/* UNIT_EXPONENT (0)*/
			   0xC0, 				/*END_COLLECTION */
			   /*Effect Operation Report Definition							[60]*/
			   0x09, 0x77,				/*USAGE (Effect Operation Report)*/
			   0xA1, 0x02,				/* COLLECTION (Logical) */
			    0x85, 0x04,				/* REPORT_ID (4) */
			    0x09, 0x22,				/* USAGE (Effect Block Index) */
				0x15, 0x01,				/*LOGICAL_MINIMUM (1)*/
			    0x25, 0x20,				/*LOGICAL_MAXIMUM (32)*/
				0x35, 0x01,				/* PHYSICAL_MINIMUM (1)*/
				0x45, 0x20,				/* PHYSICAL_MAXIMUM (32)*/
			    0x75, 0x08,				/* REPORT_SIZE (8) */
			    0x95, 0x01,				/* REPORT_COUNT (1) */
			    0x91, 0x02,				/* OUTPUT (Data,VaR,AbS) */
			    0x09, 0x78,				/* USAGE (Effect Operation) */
			    0xA1, 0x02,				/* COLLECTION (Logical) */
			     0x09, 0x79,			/* USAGE (OpEffect Start) */
			     0x09, 0x7A,			/* USAGE (OpEffect Start Solo) */
			     0x09, 0x7B,			/* USAGE (OpEffect Stop) */
			     0x15, 0x01,			/* LOGICAL_MINIMUM*/
			     0x25, 0x03,			/* LOGICAL_MAXIMUM*/
				 0x35, 0x01,			/* PHYSICAL_MINIMUM*/
			     0x45, 0x03,			/* PHYSICAL_MAXIMUM*/
			     0x75, 0x08,			/* REPORT_SIZE (8) */
			     0x91, 0x00,			/* OUTPUT (Data,ArY,AbS) */
			    0xC0,					/* END_COLLECTION */
				0x09, 0x7C,				/* USAGE (Loop Count) */
				0x15, 0x00,				/* LOGICAL_MINIMUM (0) */
				0x26, 0xFF, 0x00,		/* LOGICAL_MAXIMUM (255) */
				0x35, 0x00,				/* PHYSICAL_MINIMUM*/
			    0x46, 0xFF, 0x00,		/* PHYSICAL_MAXIMUM*/
				0x91, 0x02,				/* OUTPUT (Data,VaR,AbS) */
			   0xC0,					/* END_COLLECTION */
			   /* PID State report Definition 								[53]*/
			   0x09, 0x92,				/* USAGE (PID State Report) */
			   0xA1, 0x02,				/* COLLECTION (Logical) */
			    0x85, 0x02, 			/* REPORT_ID (2) */
				0x09, 0x22,				/* USAGE (Effect Block Index) */
				0x15, 0x01,				/* LOGICAL_MINIMUM (1)*/
			    0x25, 0x20,				/* LOGICAL_MAXIMUM (32)*/
				0x35, 0x01,				/* PHYSICAL_MINIMUM (1)*/
				0x45, 0x20, 			/* PHYSICAL_MAXIMUM (32)*/
				0x75, 0x08,				/* REPORT_SIZE (8) */
				0x95, 0x01,				/* REPORT_COUNT (1) */
				0x81, 0x02,				/* INPUT (Data,VaR,AbS) */
				0x09, 0x94, 			/* USAGE (Effect Playing) */
				0x09, 0x9F,				/* USAGE (Device Paused) */
				0x09, 0xA0, 			/* USAGE (Actuators Enabled) */
				0x09, 0xA4, 			/* USAGE (Safety Switch) */
				0x09, 0xA5, 			/* USAGE (Actuator Override Switch)*/
				0x09, 0xA6, 			/* USAGE (Actuator Power) */
				0x15, 0x00,				/* LOGICAL_MINIMUM (0)*/
				0x25, 0x01, 			/* LOGICAL_MAXIMUM (1) */
				0x35, 0x00,				/* PHYSICAL_MINIMUM (0)*/
				0x45, 0x01, 			/* PHYSICAL_MAXIMUM (1)*/
				0x75, 0x01,				/* REPORT_SIZE (1) */
				0x95, 0x06, 			/* REPORT_COUNT (6) */
				0x81, 0x02,				/* INPUT (Data,VaR,AbS) */
				0x95, 0x02, 			/* REPORT_COUNT (2) */
				0x81, 0x03,				/* INPUT (CnsT,VaR,AbS) ; 2-bit pad */
			   0xC0,					/* END_COLLECTION */
			   /* PID Device control Report Definition						[33]*/
//			   0x09, 0x95,				/* USAGE (PID Device Control Report)  XXX not used because in linuX is written that anyway the 0x96 (PID Device control is expected) and the SideWinder example also uses only 0x96)*/
//			   0xA1, 0x02,				/* COLLECTION (Logical) */
//			    0x85, 0x0B,				/* REPORT_ID (11) */
			   0x09, 0x96,				/* USAGE (PID Device Control) */
			   0xA1, 0x02,				/* COLLECTION (LOGICAL) */
			    0x85, 0x05,				/* REPORT_ID (5) */
			    0x09, 0x97,			/* USAGE (DC Enable Actuators) */
				0x09, 0x98,			/* USAGE (DC Disable Actuators) */
				0x09, 0x99,			/* USAGE (DC Stop All Effects) */
			    0x09, 0x9A,			/* USAGE (DC Reset) */
			    0x09, 0x9B,			/* USAGE (DC Pause) */
			    0x09, 0x9C,			/* USAGE (DC Continue) */
			    0x15, 0x01,			/* LOGICAL_MINIMUM (1) */
			    0x25, 0x06,			/* LOGICAL_MAXIMUM (6) */
				0x35, 0x00,			/* PHYSICAL_MINIMUM (0 - use logical limits)*/
				0x45, 0x00, 		/* PHYSICAL_MAXIMUM (0 - use logical limits)*/
//				0x75, 0x01,			/* REPORT_SIZE (1)  XXX in SideWinder report descriptor is not 1; see after commented lines how it is*/
//			    0x95, 0x08,			/* REPORT_COUNT (4 XXX Wrong:is 0x08 not 0x04 ) */
				 /*XXX FixMe use 2 bit padding or use Array as defined below if report is not correctly read:
				   0x75, 0x08,		 REPORT_SIZE (8)
				   0x95, 0x01,		 REPORT_COUNT (1)
				   0x91, 0x00,		 OUTPUT (Data,ArY,AbS)*/
				0x75, 0x08,			/* REPORT_SIZE (8) */
				0x95, 0x01,			/* REPORT_COUNT (1) */
			    // 0x91, 0x02,			/* OUTPUT(Data,Var,AbS) XXX: was commented because also in SideWinder report example is not like this*/
				0x91, 0x00,			/* OUTPUT(Data,ArY,AbS)*/
			   0xC0,					/* END_COLLECTION */
//			   0xC0,					/* END_COLLECTION  XXX from commented COLLECTION after USAGE (PID Device Control R ... */
			   /* Device Gain Report Definition								[19]*/
			   0x09, 0x7D,				/* USAGE (Device Gain Report) */
			   0xA1, 0x02,				/* COLLECTION (Logical) */
			    0x85, 0x02,				/* REPORT_ID (2) */
			    0x09, 0x7E,				/* USAGE (Device Gain) */
				0x15, 0x00,			 	/* LOGICAL_MINIMUM (0) */
			    0x25, 0xFF,				/* LOGICAL_MAXIMUM (255) */
			    0x75, 0x08,				/* REPORT_SIZE (8) */
			    0x95, 0x01, 			/* REPORT_COUNT (1) */
			    0xB1, 0x02, 			/* FEATURE (Data,VaR,AbS) */
			   0xC0,					/* END_COLLECTION */
			   /* PID Pool Report Definition								[59]*/
			   0x09, 0x7F,				/* USAGE (PID Pool Report) */
			   0xA1, 0x02,				/* COLLECTION (Logical) */
			    0x85, 0x01,				/* REPORT_ID (1) */
			    0x09, 0x80,				/* USAGE (RAM Pool Size) */
				0x16, 0x00, 0x00,		/* LOGICAL Minimum*/
				0x34,              		/* Physical Minimum (0),           */
				0x26, 0x00, 0x00,		/* LOGICAL_MAXIMUM was 0xFD, 0x7F*/
			    0x44, 					/* Physical Maximum (0),         */
			    0x95, 0x01,				/* REPORT_COUNT (1) */
			    0x75, 0x10,				/* REPORT_SIZE (16) */
			    0xB1, 0x02,				/* FEATURE (Data,VaR,AbS) */
				0x09, 0x83,				/* USAGE (Simultaneous effects max)*/
				0x25, 0xFF,				/* LOGICAL_MAXIMUM (255) */
				0x45, 0xFF,				/* Physical Maximum (255) */
				0x75, 0x08,				/* REPORT_SIZE - 8*/
				0x95, 0x01,				/* REPORT_COUNT - 1*/
				0xB1, 0x02,				/* FEATURE (Data,VaR,AbS) */
				0x09, 0xa9,				/* usage device managed pool*/
				0x09, 0xaa,				/* usage shared parameter blocks*/
				0x75, 0x01,				/* REPORT_SIZE (1) */
				0x95, 0x02,				/* REPORT_COUNT (2) */
				0x15, 0x00,				/* LOGICAL Minimum (0)*/
				0x25, 0x01,				/* LOGICAL_MAXIMUM (1)*/
				0x35, 0x00,				/* Physical Minimum (0) */
				0x45, 0x01,				/* Physical Maximum (1) */
				0xB1, 0x02,				/* FEATURE (Data,VaR,AbS) */
				0x75, 0x06,				/* REPORT_SIZE (6) */
				0x95, 0x01,				/* REPORT_COUNT (1) */
				0xB1, 0x03,				/* FEATURE (CnsT,VaR,AbS) ; 6-bit pad */
			   0xC0,					/* END_COLLECTION */
			   /* Create New Effect Report 									[48]*/
			   0x09, 0xAB,	   			/* USAGE (Create New Effect Report) */
			   0xA1, 0x02, 	   	   	    /* COLLECTION (Logical) */
			    0x85, 0x03, 	   	   	/* REPORT_ID (3) */
			    0x09, 0x25,   	   	   	/* USAGE (Effect Type) */
			    0xA1, 0x02, 	   	   	/* COLLECTION (Logical) */
				 0x09, 0x26,  	   	   	/* USAGE (ET Constant Force) */
//			   	   	   	   	   	   	    /* USAGE (ET Ramp) */
//			   	   	   	   	   	   	    /* USAGE (ET Square) */
//			   	   	   	   	   	   	    /* USAGE (ET Sine) */
//			   	   	   	   	   	   	    /* USAGE (ET Triangle) */
//			   	   	   	   	   	   	    /* USAGE (ET Sawtooth Up) */
//			   	   	   	   	   	   	    /* USAGE (ET Sawtooth Down) */
//			   	   	   	   	   	   	    /* USAGE (ET Spring) */
//			   	   	   	   	   	   	    /* USAGE (ET Damper) */
//			   	   	   	   	   	   	    /* USAGE (ET Inertia) */
//			   	   	   	   	   	   	    /* USAGE (ET Friction) */
//			   	   	   	   	   	   	    /* USAGE (ET Custom Force Data) */
				 0x15, 0x01,  	   	   	/* LOGICAL_MINIMUM (1) */
				 0x25, 0x01,   	   	    /* LOGICAL_MAXIMUM (1 XXX was 12 when all ET's where active) */
				 0x35, 0x00,			/* Physical Minimum (0) */
				 0x45, 0x01,			/* Physical Maximum (1) */
				 0x75, 0x08, 	   	   	/* REPORT_SIZE (8) */
				 0x95, 0x01,   	   	   	/* REPORT_COUNT (1) */
				 0xB1, 0x00,  	   	   	/* FEATURE (Data,ArY,AbS) */
				0xC0,  	   	   	   		/* END_COLLECTION */
				0x05, 0x01,   	   	   	/* USAGE_PAGE (Generic Desktop) */
				0x09, 0x3B,			    /* USAGE (Byte Count) */
				0x15, 0x00,	   	   	    /* LOGICAL_MINIMUM (0) */
				0x26, 0xFF, 0x00,   	/* LOGICAL_MAXIMUM (255) */
				0x35, 0x00,				/* Physical Minimum (0) */
				0x46, 0xFF, 0x00,		/* Physical Maximum (255) */
				0x75, 0x08,   	   	   	/* REPORT_SIZE (8) */
				0x95, 0x01,   	   	   	/* REPORT_COUNT (1) */
				0xB1, 0x02,	   	   	    /* FEATURE (Data,VaR,AbS) */
			   0xC0,   	   	   	    	/* END_COLLECTION */
			   /* PID Block Load Report 									[68]*/
			   0x05, 0x0F,				/* USAGE_PAGE (Physical Interface)*/
			   0x09, 0x89,				/* USAGE (PID Block Load Report) */
			   0xA1, 0x02,				/* COLLECTION (Logical) */
			    0x85, 0x04,				/* REPORT_ID (4) */
			    0x09, 0x22,				/* USAGE (Effect Block Index) */
				0x15, 0x01, 			/* LOGICAL_MINIMUM (1) */
				0x25, 0x20,				/* LOGICAL_MAXIMUM (32) */
				0x35, 0x01,				/* PHYSICAL_MINIMUM (1)*/
				0x45, 0x20,				/* PHYSICAL_MAXIMUM (32)*/
				0x75, 0x08,				/* REPORT_SIZE (8) */
				0x95, 0x01,				/* REPORT_COUNT (1) */
				0xB1, 0x02,				/* FEATURE (Data,VaR,AbS) */
				0x09, 0x8B,				/* USAGE (Block Load Status) */
				0xA1, 0x02,				/* COLLECTION (Logical) */
				 0x09, 0x8C,			/* USAGE (Block Load Success) */
				 0x09, 0x8D,			/* USAGE (Block Load Full) */
				 0x09, 0x8E,			/* USAGE (Block Load Error) */
				 0x25, 0x03,			/* LOGICAL_MAXIMUM (3) */
				 0x15, 0x01,			/* LOGICAL_MINIMUM (1) */
				 0x35, 0x01,			/* PHYSICAL_MINIMUM (1) */
				 0x45, 0x03,	 		/* PHYSICAL_MAXIMUM (3) */
				 0x75, 0x08,			/* REPORT_SIZE (8) */
				 0x95, 0x01,			/* REPORT_COUNT (1) */
				 0xB1, 0x00,			/* FEATURE (Data,ArY,AbS) */
				0xC0,					/* END_COLLECTION */
				0x09, 0xAC,				/* USAGE (RAM Pool Available) */
				0x15, 0x00,				/* LOGICAL_MINIMUM (0) */
				0x26, 0xFF, 0x00,		/* LOGICAL_MAXIMUM (255 ) */
				0x35, 0x00,				/* PHYSICAL_MINIMUM (0) */
				0x46, 0xFF, 0x00,		/* PHYSICAL_MAXIMUM (255) */
				0x75, 0x08,				/* REPORT_SIZE  */
				0x95, 0x01,				/* REPORT_COUNT (1) */
				//0xB1, 0x00,				/* FEATURE (Data,ArY,AbS) */
				0xB1, 0x02,				/* FEATURE (Data,VaR,AbS) */
			   0xC0,					/* END_COLLECTION */
			   /* PID Block Free Report 									[23]*/
			   0x09, 0x90,	   	   	   /* USAGE (PID Block Free Report) */
			   0xA1, 0x02,	   	   	   /* COLLECTION (Logical) */
			    0x85, 0x05,	   	   	   /* REPORT_ID (5) */
				0x09, 0x22,			   /* USAGE (Effect Block Index) */
				0x15, 0x01,			   /* LOGICAL_MINIMUM (01) */
				0x25, 0x20,			   /* LOGICAL_MAXIMUM (32) */
				0x35, 0x01,			   /* PHYSICAL_MINIMUM (01) */
				0x45, 0x20,	 		   /* PHYSICAL_MAXIMUM (32) */
				0x75, 0x08,			   /* REPORT_SIZE (8) */
				0x95, 0x01,			   /* REPORT_COUNT (1) */
				0xB1, 0x02,			   /* FEATURE (Data,VaR,AbS) */
			   0xC0,				   /* END COLLECTION () */
			  /* Collection Application END									[1]*/
			  0xC0                /*  End Collection  Application*/ 
}; 

/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  uint8_t ret = 0;
  
  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
                 HID_EPIN_ADDR,
                 USBD_EP_TYPE_INTR,
                 AUSB_EP1InMaxPacketSize);  

	/* Open EP OUT */
  USBD_LL_OpenEP(pdev,
                 HID_EPOUT_ADDR,
                 USBD_EP_TYPE_INTR,
                 AUSB_EP1OutMaxPacketSize); 

  /*prepare to receive data*/
  USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR, usbd_hid_EP1rxBuf, AUSB_EP1OutMaxPacketSize);

  pdev->pClassData = USBD_malloc(sizeof (USBD_HID_HandleTypeDef));
  
  if(pdev->pClassData == NULL)
  {
    ret = 1; 
  }
  else
  {
    ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  }
  return ret;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  USBD_LL_CloseEP(pdev,
                  HID_EPIN_ADDR);
  
  /* FRee allocated memory */
  if(pdev->pClassData != NULL)
  {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  } 
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*) pdev->pClassData;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
           
    case HID_REQ_SET_PROTOCOL:
      hhid->Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&hhid->Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      hhid->IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&hhid->IdleState,
                        1);        
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(JOYSTICK_SIZ_REPORT_DESC , req->wLength);
        pbuf = Joystick_ReportDescriptor;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_HID_Desc;   
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->AltSetting,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      hhid->AltSetting = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport 
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport     (USBD_HandleTypeDef  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
  USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;
  
  if (pdev->dev_state == USBD_STATE_CONFIGURED )
  {
    if(hhid->state == HID_IDLE)
    {
      hhid->state = HID_BUSY;
      USBD_LL_Transmit (pdev, 
                        HID_EPIN_ADDR,                                      
                        report,
                        len);
    }
  }

  newDeviceDataAvailable = 1u;

  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetPollingInterval 
  *         return polling interval from endpoint descriptor
  * @param  pdev: device instance
  * @retval polling interval
  */
uint32_t USBD_HID_GetPollingInterval (USBD_HandleTypeDef *pdev)
{
  uint32_t polling_interval = 0;

  /* HIGH-speed endpoints */
  if(pdev->dev_speed == USBD_SPEED_HIGH)
  {
   /* Sets the data transfer polling interval for high speed transfers. 
    Values between 1..16 are allowed. Values correspond to interval 
    of 2 ^ (bInterval-1). This option (8 ms, corresponds to HID_HS_BINTERVAL */
    polling_interval = (((1 <<(HID_HS_BINTERVAL - 1)))/8);
  }
  else   /* LOW and FULL-speed endpoints */
  {
    /* Sets the data transfer polling interval for low and full 
    speed transfers */
    polling_interval =  HID_FS_BINTERVAL;
  }
  
  return ((uint32_t)(polling_interval));
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint16_t *length)
{

  *length = sizeof (USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}


/**
  * @brief  USBD_HID_EP0_RxReady
  *         data reception finished on EP0
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_HID_EP0_RxReady(USBD_HandleTypeDef *pdev )
{
#ifdef DEBUGENABLE_USB
	// /*TOGGLE THE trigger pin needed by logic analyzer*/
	// HAL_GPIO_TogglePin(GPIOA, PIN_LogicAnalyzerTriggerOutOrClassReport);
	// HAL_GPIO_TogglePin(GPIOA, PIN_LogicAnalyzerTriggerOutOrClassReport);
	// HAL_GPIO_TogglePin(GPIOA, PIN_LogicAnalyzerTriggerOutOrClassReport);
#endif

	/*check first byte: report is (should be the same as MsB of wValue received in previous setup packet)*/
	if ( 0x03 == usbd_hid_EP0rxBuf[0] ) //&& (0x0303 == classReqHistory))
	{//This is the data stage for "Create new effect report"
		ausb_pidEvent = evCreateNewEffectDataStage;
	}

	if ( 0x05 ==  usbd_hid_EP0rxBuf[0] )
	{/*data stage for free efect*/
		if ( 1 == usbd_hid_EP0rxBuf[0] )
		{/*effect index is 1*/
			ausb_pidEvent = evFreeEffectSetReportDataStage;
		}/*only effect index 1 - constant force is supported*/
	}

	return USBD_OK;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn (USBD_HandleTypeDef *pdev, 
                              uint8_t epnum)
{
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;


  if ( 0u != newDeviceDataAvailable )
  {
	newDeviceDataAvailable = 0u; 
#ifdef DEBUGENABLE_USB
  	/*TOGGLE THE trigger pin needed by logic analyzer*/
  	HAL_GPIO_TogglePin(GPIOA, PIN_LogicAnalyzerTriggerInReport);
#endif
  }

  return USBD_OK;
}

/**
  * @brief  USBD_HID_DataOut
  *         handle data Out Stage (MiPa: from PC to UsbDevice)
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataOut(USBD_HandleTypeDef *pdev , uint8_t epnum)
{
	/*TOGGLE THE trigger pin needed by logic analyzer*/
#ifdef DEBUGENABLE_USB
	HAL_GPIO_TogglePin(GPIOA, PIN_LogicAnalyzerTriggerOutOrClassReport);
#endif

	/*data should be already available in usbd_hid_EP1rxBuf*/

	if ( 1 == epnum )
	{/*interrupt endpoint 1*/

		switch (usbd_hid_EP1rxBuf[0])		/*at index 0 is the Report ID*/
		{
		case 1: /* Set Effect  Report */
		{
			/*save effect index (should be 1: only one effect is stored - the constant force effect)*/
			ausb_OperationReportEffectIdx = usbd_hid_EP1rxBuf[1];

			/*check effect type: should be 1 - Constant Force*/
			if ( 1 == usbd_hid_EP1rxBuf[2] )
			{/*all OK*/
				/*update relevant fields in the effect structure*/
				ausb_effectData.duration = ((uint16_t)(usbd_hid_EP1rxBuf[4])<<8) | usbd_hid_EP1rxBuf[3]; /*16 bit; FixMe: check scaling (exponent is used*/
				/*[5,6] trigger repeat interval (not used)*/
				/*[7,8] sample period (not used)*/
				ausb_effectData.gain = usbd_hid_EP1rxBuf[9]; /*normalized value: 0-100%*/
				/*[10] trigger button (not used)*/
				/*[11] axes enable(must be X axis because other axis is not used for PID (we don't check it)*/
				ausb_effectData.direction = usbd_hid_EP1rxBuf[12];  /*should be normalized value: -100..0..100% (FixMe negative number means force from left to right? FixMe*/
				/*Receive_Buffer[13]; direction enable 1 byte - MSBit is a flag, the other bits are not used*/
				ausb_effectData.startDelay = ((uint16_t)(usbd_hid_EP1rxBuf[15])<<8) | usbd_hid_EP1rxBuf[14]; /*start delay 2 bytes - XXX FixMe see if it's used*/
				/*Receive_Buffer[16,17]; ignore this field because for constant force effect is clear that first parameter is Magnitude; XXX was ausb_setEffectData.typeSpecificBlockOffset1 = ((uint16_t)Receive_Buffer[14] << 8) | Receive_Buffer[13]; */
				/*Receive_Buffer[18,19]; ignore this field because for constant force effect is clear that second parameter is Envelope; XXX was ausb_setEffectData.typeSpecificBlockOffset2 = ((uint16_t)Receive_Buffer[16] << 8) | Receive_Buffer[15]; */
			}else{/*no actions: effect type not OK*/}

		}break;
		// case 2: /* Envelope report */
		// {
		// 	if ( 1 == Receive_Buffer[1] )
		// 	{/*Block Index OK (now is always 1)*/
		// 		/*save the envelope values*/
		// 		ausb_effectData.envelopeAttackLevel = Receive_Buffer[2];
		// 		ausb_effectData.envelopeFadeLevel = Receive_Buffer[3];
		// 		ausb_effectData.envelopeAttackTime = ( (uint16_t)Receive_Buffer[5] << 8 ) | Receive_Buffer[4];
		// 		ausb_effectData.envelopeFadeTime = ( (uint16_t)Receive_Buffer[7] << 8 ) | Receive_Buffer[6];
		// 	}else{/*unsupported index*/}

		// }break;
		case 3: /* Set Constant Force Report */
		{
			/*save the magnitude value*/
			ausb_effectData.constantForceMagnitude = (usbd_hid_EP1rxBuf[3] << 8) | usbd_hid_EP1rxBuf[2];
			ausb_flags |= AUSB_CONSTANT_FORCE_UPDATED;
		}break;
		case 4: /*Effect Operation Report */
		{
			ausb_flags |= AUSB_NEW_CONTROL_EVENT; //set flag

			/*save effect index*/
			ausb_OperationReportEffectIdx = usbd_hid_EP1rxBuf[1];

			/*check effect operation command byte; values are: 1-Start, 2-StartSolo, 3-Stop*/
			if ( (1 == usbd_hid_EP1rxBuf[2]) || (2 == usbd_hid_EP1rxBuf[2]) )
			{/*Start effect or Start effect Solo (for now is the same because only one effect is active at a time*/
				ausb_pidStateFlags |= 0x01u;
				/*save the loop count*/
				ausb_effectDataLoopCnt = usbd_hid_EP1rxBuf[3];
			}
			else
			{/*we stop the effect on any value different than 1 or 2 (normally: 3 - Stop effect)*/
				ausb_pidStateFlags |= 0x02u;
				ausb_effectDataLoopCnt = 0;
			}

		}break;
		case 5: /*PID device control */
		{
			ausb_flags |= AUSB_NEW_CONTROL_EVENT; //set flag

			/*check command byte*/
			switch ( usbd_hid_EP1rxBuf[1] )
			{
			case 1: //Enable actuators
			{
				ausb_pidStateFlags |= 0x10u;

			}break;
			case 2:	// Disable actuators
			{
				ausb_pidStateFlags |= 0x20u;
			}break;
			case 3:	//Stop all effects
			{
				ausb_pidStateFlags |= 0x02u;
				ausb_effectDataLoopCnt = 0;
			}break;
			case 4:	//Reset
			{/*Clears any device paused condition, enables all actuators and clears all effects from memory*/
				ausb_pidStateFlags |= (0x18u); /*set continue and enable actuators flags*/
				/*clear effects data*/
				ausb_initEffectsData();
			}break;
			case 5:	//Pause
			{
				ausb_pidStateFlags |= 0x04u;
			}break;
			case 6:	//Continue
			{
				ausb_pidStateFlags |= 0x08u;
			}break;

			default:
			{
				ausb_flags &= 0xFF^AUSB_NEW_CONTROL_EVENT; //clear flag because the event is undefined
			}break;
			}
		}break;

		default:
			///*TODO: remove this DEBUG TEST on LED*/		aleds_ledGreenStatus = ALEDS_LED_STATUS_BLINK; /*TODO: remove this DEBUG TEST on LED*/
		break;
		}
	}
	/*prepare for next RX data*/
	USBD_LL_PrepareReceive(pdev, epnum, usbd_hid_EP1rxBuf, AUSB_EP1OutMaxPacketSize);

	return USBD_OK;
} 

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_HID_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_HID_DeviceQualifierDesc);
  return USBD_HID_DeviceQualifierDesc;
}

/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
