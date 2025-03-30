/**
  ******************************************************************************
  * @file    hw_config.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Hardware Configuration & Setup
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
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

#include "ausb.h"

// #include "aquadrenc.h"
// #include "usb_lib.h"
// #include "usb_prop.h"
// #include "usb_desc.h"
// #include "usb_pwr.h"
// #include "usb_istr.h"
// //#include "stm32f10x_exti.h"
// #include "stm32f10x_gpio.h"
// #include "stm32f10x_rcc.h"
// #include "stm32f10x_pwr.h"
//#include "misc.h"
#include "aleds.h"
#include "amagenc.h"
#include "apedals.h"
#include "usbd_hid.h"
#include "usb_device.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include "usbd_def.h"
#include "abuttons.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//ErrorStatus HSEStartUpStatus;
//EXTI_InitTypeDef EXTI_InitStructure;
static uint16_t magEncPos;
static uint16_t ausb_effectOnTimer;
static uint16_t ausb_effectRepeatCnt;
static uint8_t ausb_pidOperation; /*Bits: 	0-Effect Playing
											1-Device Paused
											2-Actuators Enabled
											3-SafetySwitch
											4-ActuatorOverrideSwitch
											5-Actuator Power
								  */
// static uint8_t phyIntDevState;
// enum ePhyIntDevStates
// {
// 	s01Wait4CreateNewEffectSetReportReq, 	/*setup packet with the set report request (report id == CreateNewEffect)*/
// 	s02Wait4CreateNewEffectSetReportData, 	/*data packet*/
// 	s03PrepareCreatingEffect,				/*prepare the effect: allocate memory*/
// 	s04Wait4PIDBlockLoadGetReportReq,		/*setup packet with the get report request (report id == PID Block Load report*/
// 	s05SendPIDBlockLoadGetReportData,		/*data packet*/
// 	s06Wait4ParametersDownloadSetReportReq, /*setup packet with the effect parameters (report id == SetEffect)*/
// 	s07Wait4ParametersDownloadSetReportData,/*data packet; effect block index (second byte) is the index sent on PID block load report*/
// 	s08StartEffectorSetConditionReport,		/*most probable SetConditionReport is not sent here*/
// 	s09TimeOut
// };
uint8_t ausb_pidEvent;

/* External variables ----------------------------------------------------------*/
volatile uint8_t PrevXferComplete;
volatile uint8_t ausb_flags;
volatile uint8_t ausb_pidStateFlags; /*Bits: 	0-Start Playing Effect, 1-Stop Playing Effect,
											2-Pause,				3-Continue,
											4-Enable actuators,		5-Disable actuators
								  */
uint8_t sendPidStatus; /*variable set when a status report (id 2) needs to be sent and cleared when the report is successfully transmited*/
volatile uint8_t ausb_effectDataLoopCnt;
volatile uint8_t ausb_deviceGain;
pidEffectDataType ausb_effectData;
static uint8_t pidbuf[20];
uint8_t ausb_OperationReportEffectIdx;
uint8_t userButtonStatus;
static uint16_t setupEffectOngoing;
static uint8_t pAccOld;
static uint8_t pBrkOld;

/*variables used to calculate the wheel pos based on the encoder data*/
static uint16_t wheelPosRawOld; /*no need to initialize this variable*/
static uint16_t wheelPosOffset;
static uint16_t wheelPosMin;
static uint16_t wheelPosMax;
uint16_t wheelPos; // = mePos << (16- WHEELPOS_RESOLUTION_BITS);

/*gear shift buttons*/
extern uint8_t btnShiftDw;
extern uint8_t btnShiftUp;


/* imported variables*/
extern PCD_HandleTypeDef hpcd_USB_FS;
extern uint8_t usbd_hid_EP0rxBuf[];
// extern uint8_t pDataChanged;
extern uint8_t pAcc;
extern uint8_t pBrk;

/* Private function prototypes -----------------------------------------------*/
//static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);
static void sendNewHIDSensorsData( void );
//static void sendNewHIDPidStateData( void );
//static void Set_System(void);
//static void USB_Interrupts_Config(void);
static uint8_t sensorsDataChanged(void);
//static void Set_USBClock(void);
static void onNewControlEvent( void );
static uint16_t processWheelPos( uint16_t mePos );

/* Private functions ---------------------------------------------------------*/

/*initialization of the USB peripheral*/
void usbmain_init(void)
{
	/*peripheral initialization is done STMCubeMx generated modules*/
	// Set_System();
	// USB_Interrupts_Config();
	// Set_USBClock();
	// USB_Init();

	magEncPos = 0u;
	ausb_flags = 0u;
	ausb_pidStateFlags = 0u;
	ausb_deviceGain = 128u; /*set to half; FixMe after testing with motor see if this default value is OK*/
	ausb_pidOperation = 0x24u; /*actuators are powered and enabled by default (see affmotor_init)*/
	ausb_pidEvent = evNone;
	sendPidStatus = 0u;
	userButtonStatus = 0u;

	pAccOld = 0u;
	pBrkOld = 0u;

	wheelPosMin = 0xFFFFu;
	wheelPosMax = 0u;
	wheelPos = 1u; /*dummy value until the wheel is calibrated*/

	ausb_initEffectsData();

}

/*manages sensor data*/
void ausb_manage10ms( void )
{

	if (sensorsDataChanged() != 0)
	{
		sendNewHIDSensorsData();
	}

	if ( setupEffectOngoing > 10u )
	{
		setupEffectOngoing-=10u;
	}
	else
	{
		setupEffectOngoing = 0u;
	}
	
}

/*manages force feetback commands*/
void ausb_manage1ms( void )
{
	//static uint16_t ausb_effectOnTimerOld;

	// /*decrement timer and check timeOut*/
	// if ( (ausb_effectOnTimer > 0) && (0x00 == (ausb_pidOperation & 0x02u)) )
	// {/*effect still playing and pause flag is not set*/

	// 	ausb_effectOnTimer--;

	// 	if ( (0 == ausb_effectOnTimer) && (ausb_effectOnTimerOld > 0) )
	// 	{/*timeOutEvent*/
	// 		if ( ausb_effectRepeatCnt > 0 )
	// 		{/*effect can be played one more time*/
	// 			/*restart effect*/
	// 			ausb_effectOnTimer = ausb_effectData.duration;
	// 			ausb_effectRepeatCnt = ausb_effectDataLoopCnt;
	// 			/*TODO: set here a flag to restart the effect (this is valid if for example an envelope is used and the magnitude should be updated accordingly*/
	// 			ausb_effectRepeatCnt--;

	// 		}
	// 		else
	// 		{
	// 			/*stop effect*/
	// 			ausb_pidOperation &= 0xFE; /*clear flag*/

	// 			AUSB_STOP_ACTUATOR
	// 		}

	// 	}else{/*no actions*/}

	// 	ausb_effectOnTimerOld = ausb_effectOnTimer;

	// }else{/*no actions*/}

}

static void onNewControlEvent( void )
{
	/*check flags*/
	if ( 0 != (0x01u & ausb_pidStateFlags) )
	{/*play effect*/

		AUSB_START_ACTUATOR

		/*update PID operation flag*/
		ausb_pidOperation |= 0x01u;

		/*start timer*/
		ausb_effectOnTimer = ausb_effectData.duration;
		ausb_effectRepeatCnt = ausb_effectDataLoopCnt;

		/*play command is sent at the end of the effect setup so enable back the in report 1*/
		setupEffectOngoing = 0u;
	}else{/*no actions*/}
	if ( 0 != (0x02u & ausb_pidStateFlags) )
	{/*stop effect*/

		affmotor_updatePwmDC(/*0,*/ 0);
		AUSB_STOP_ACTUATOR

		/*update PID operation flag*/
		ausb_pidOperation &= 0xFEu;

		ausb_effectOnTimer = 0;
		ausb_effectRepeatCnt = 0;
	}else{/*no actions*/}
	if ( 0 != (0x04u & ausb_pidStateFlags) )
	{/*pause effects*/

		affmotor_updatePwmDC(/*0,*/ 0);
		AUSB_STOP_ACTUATOR

		/*update PID operation flag*/
		ausb_pidOperation |= 0x02u;

	}else{/*no actions*/}
	if ( 0 != (0x08u & ausb_pidStateFlags) )
	{/*continue playing effects*/

		if ( ausb_effectOnTimer > 0 )
		{
			AUSB_START_ACTUATOR
		}else{/*effect was not playing*/}

		/*clear PAUSE flag*/
		ausb_pidOperation &= (0xFFu^0x02u);

	}else{/*no actions*/}
	if ( 0 != (0x10u & ausb_pidStateFlags) )
	{/*enable actuators*/

		affmotor_updatePwmDC(/*0,*/ 0);
		AUSB_STOP_ACTUATOR

		ausb_effectOnTimer = 0;
		ausb_effectRepeatCnt = 0;

		/*update PID operation flag*/
		ausb_pidOperation &= 0xFEu;

		AUSB_ENABLE_ACTUATOR

		/*update PID operation flag*/
		ausb_pidOperation |= 0x04u;

	}else{/*no actions*/}
	if ( 0 != (0x20u & ausb_pidStateFlags) )
	{/*disable actuators*/

		AUSB_STOP_ACTUATOR

		ausb_effectOnTimer = 0;
		ausb_effectRepeatCnt = 0;

		/*update PID operation flag*/
		ausb_pidOperation &= 0xFEu;

		AUSB_DISABLE_ACTUATOR

		/*update PID operation flag*/
		ausb_pidOperation &= (0xFFu^0x04u);

	}else{/*no actions*/}

	sendPidStatus = 1u;

	ausb_pidStateFlags = 0x00u;

	/*turn on led to indicate a control event happened*/
	//aleds_ledGreenStatus = ALEDS_LED_STATUS_BLINK;
	//aleds_ledGreenBlinkTimeMs = 1000; /*ms*/
}



/*******************************************************************************
* Function Name : JoyState.
* Description   : Returns 1u if any sensor data was changed / 0u if not.
* Input         : None.
* Output        : None.
* Return value  : The direction value.
*******************************************************************************/
static uint8_t sensorsDataChanged(void)
{
	static uint8_t userButtonStatusOld = 0u;
	uint8_t retVal = 0;

	/*check if magnetic encoder value changed*/
	if ( (magEncPos != amagenc_ssiSample) /*(WheelState() != 0) ||*/ )
	{
		magEncPos = amagenc_ssiSample;
		retVal = 1u;
	}else{/*no actions: retVal already initialized with 0*/}

	if ( pAccOld != pAcc )
	{/*pedal data changed*/
		pAccOld = pAcc;
		retVal = 1u;
	}else{/*no actions: retVal already initialized with 0*/}

	if ( pBrkOld != pBrk )
	{/*pedal data changed*/
		pBrkOld = pBrk;
		retVal = 1u;
	}else{/*no actions: retVal already initialized with 0*/}

	/*check buttons*/
//User button
	if ( 0u != abuttons_getBStartPressedTime() )
	{/*button is pressed*/ /*set bit*/userButtonStatus |= 1u;}
	else
	{/*clear bit*/userButtonStatus &= (0xFFu ^ 1u);}
//Shift gears buttons
	if ( btnShiftDw != 0u )
	{/*button pressed*/ /*set bit*/userButtonStatus |= 2u;}
	else
	{/*clear bit*/userButtonStatus &= (0xFFu ^ 2u);}
	if ( btnShiftUp != 0u )
	{/*button pressed*/ /*set bit*/userButtonStatus |= 4u;}
	else
	{/*clear bit*/userButtonStatus &= (0xFFu ^ 4u);}
	
	if ( userButtonStatus != userButtonStatusOld )
	{
		retVal = 1u;	
	}


	userButtonStatusOld = userButtonStatus;

	return retVal;
}

/*******************************************************************************
* Function Name : sendNewHIDSensorsData
* Description   : prepares buffer with sensors data to be sent.
* Input         : None
* Output        : None.
* Return value  : None.
*******************************************************************************/
static void sendNewHIDSensorsData( void )
{
  if ( 0u == setupEffectOngoing )
  {/*no effect setup is taking place*/
	uint8_t wheelBuf[AUSB_EP1InMaxPacketSize];
	uint16_t bufferSize = 0u;
	int16_t tWheelPos = 0;
	uint16_t tPedal = 0u;

	tWheelPos = processWheelPos(magEncPos);
	/*convert to range [-0xFFFFu/2, +0xFFFFu/2]*/
	tWheelPos = 0x7FFF - tWheelPos;

	/*Report ID*/
	wheelBuf[bufferSize++] = 0x01;
	/*X axis - Wheel angle*/
	wheelBuf[bufferSize++] = (uint8_t)(tWheelPos);				/*LSB - Little EnDian format: LSB is in the lower memory address*/;
	wheelBuf[bufferSize++] = (uint8_t)(tWheelPos >> 8);	/*MSB*/ //XXX: for 8 bit resolution update usb_desc.c and here assign 0u
	/*Acceleration PEDAL - Y axis*/
	tPedal = ((uint16_t)pAcc - 25u) * 255u / (225u-25u); /*convert from 25 - 225 to 0-255*/
	wheelBuf[bufferSize++] = (uint8_t)tPedal;
	/*Brake PEDAL - rZ axis*/
	tPedal = ((uint16_t)pBrk -25u) * 255u / (225u-25u); /*convert from 25 - 225 to 0-255*/
	wheelBuf[bufferSize++] = (uint8_t)tPedal;
	/*buttons and hat switch*/
	wheelBuf[bufferSize++] = userButtonStatus;

	USBD_HID_SendReport(&hUsbDeviceFS,wheelBuf,bufferSize);
  }else{/*setup stage ongoing so do not send this report*/}

}

void ausb_initEffectsData( void )
{
	ausb_effectData.duration = 100; 	/* 100 = 1 mSec*/
	ausb_effectData.gain = 50;			/* 50%*/
	ausb_effectData.direction = 1;				/* 1 - towards right FixMe is this right or left?*/
	ausb_effectData.envelopeAttackLevel = 0;
	ausb_effectData.envelopeFadeLevel = 0;
	ausb_effectData.envelopeAttackTime = 0;
	ausb_effectData.envelopeFadeTime = 0;
	ausb_effectData.constantForceMagnitude = AUSB_MAX_MAGNITUDE / 2;

	ausb_effectDataLoopCnt = 0u;

	ausb_effectOnTimer = 0;
	ausb_effectRepeatCnt = 0;

	ausb_OperationReportEffectIdx = 0u;
	setupEffectOngoing = 0u;
	//constantForceEffectCreated = 0u;
}

void ausb_physicalInterfaceSM( void )
{
	volatile uint32_t receivedDataSize = 0;
	USBD_HandleTypeDef *pdev;

	pdev = (USBD_HandleTypeDef*)(hpcd_USB_FS.pData);
	/*perform actions for new constant force magnitude value*/
	if ( 0u != (ausb_flags & AUSB_CONSTANT_FORCE_UPDATED) )
	{/*new magnitude value received*/
		affmotor_updatePwmDC(/*ausb_effectData.direction,*/ausb_effectData.constantForceMagnitude);
		/*clear flag*/
		ausb_flags &= (0xFF^AUSB_CONSTANT_FORCE_UPDATED);
	}

	if ( AUSB_NEW_CONTROL_EVENT == (ausb_flags & AUSB_NEW_CONTROL_EVENT) )
	{/*new PID control event happened*/
		onNewControlEvent();
		ausb_flags &= (0xFF^AUSB_NEW_CONTROL_EVENT); //clear flag
	}else{/*no action*/}

	if ( 0u != sendPidStatus )
	{
		uint8_t stateReportBuf[AUSB_EP1InMaxPacketSize];
  		uint16_t bufferSize = 0u;
		if ( HID_IDLE == ((USBD_HID_HandleTypeDef *)hUsbDeviceFS.pClassData)->state )
		{
			/*send state report*/
			stateReportBuf[bufferSize++]=2u;/*report id 2 - pid state report*/
			stateReportBuf[bufferSize++]=ausb_OperationReportEffectIdx;/*effect block index (or parameter block) - 0u*/
			stateReportBuf[bufferSize++]=ausb_pidOperation;/*pid operation*/
			USBD_HID_SendReport(&hUsbDeviceFS,stateReportBuf,bufferSize);
			
			sendPidStatus = 0u;
		}
		else
		{/*HID is busy*/
			/*sendPidStatus flag is not cleared so we try again to send status on next call*/
		}
	}
	/*check and perform setup events actions*/
	if ( evPidPoolReport == ausb_pidEvent )
	{

		pidbuf[0] = 0x01;	//Report ID
		pidbuf[1] = 0x20; //RAM pool size /*XXX: was 0x20 - 32 but I think it doesn't matter because the Pool is device managed*/ //Ram pool size 32 (32 is defined as max size in the report descriptor)
		pidbuf[2] = 0x00; //RAM pool size
		pidbuf[3] = 0x01; //simultaneous effects max
		pidbuf[4] = 0x03; //bit 0 - device managed pool, bit 1 - shared parameters block, bit 2..7 - pad
	
		USBD_CtlSendData (pdev, pidbuf, 5);
		
		ausb_pidEvent = evNone;
	
	}

	if ( evCreateNewEffectSetReport == ausb_pidEvent )
	{
		setupEffectOngoing = 1000u;
		
		ausb_pidEvent = evNone;
		receivedDataSize = 3u; /*or get it from the wLength parameter of the report 0x0303 (Feature, Create New Effect)*/
		/*get the data*/
		USBD_CtlPrepareRx(pdev,usbd_hid_EP0rxBuf, receivedDataSize);

	}
	if ( evCreateNewEffectDataStage == ausb_pidEvent )
	{
		ausb_pidEvent = evNone;
		// /*check second byte: effect type*/
		if ( 0x01 == usbd_hid_EP0rxBuf[1] )
		{/*0x01 - constant force report*/
			/*this report id is supported*/
		}
	}
	if ( evPidBlockLoadGetReport == ausb_pidEvent )
	{
		ausb_pidEvent = evNone;
		//phyIntDevState = s06Wait4ParametersDownloadSetReportReq;

		pidbuf[0] = 0x04;	//Report ID
		// if ( 0u == constantForceEffectCreated )
		// {
			pidbuf[1] = 1; //Effect block index FixMe for now is always 1 ( see if a proper effect buffer is needed)
			pidbuf[2] = 1; //Block Load Status: 1 - Block load success; 2 - Block load full (out of memory); 3 - Block load error
			//constantForceEffectCreated = 1u;
		// }
		// else
		// {/*effect is created already*/
		// 	pidbuf[1] = 0;
		// 	pidbuf[2] = 2;
		// }
		
		/*RAM Pool Available: 1 bytes of data*/
		pidbuf[3] = 0x20; /*was 0x10 maybe by mistake 10b == 2*/ /*should be LSB FixMe if not*/
		// pidbuf[4] = 0x00; /*should be MSB*/

		USBD_CtlSendData (pdev, pidbuf, 4); //last byte was 6 but on pid1_01.pdf page 43 only 5 bytes are specified for the PID Block Load Report
	}
	if ( evFreeEffectSetReport == ausb_pidEvent )
	{
		affmotor_updatePwmDC(/*0,*/ 0);
		AUSB_STOP_ACTUATOR	
		
		ausb_pidEvent = evNone;

		receivedDataSize = 2u; /*or get it from the wLength parameter of the report 0x0303 (Feature, Create New Effect)*/
		/*get the data*/
		USBD_CtlPrepareRx(pdev,usbd_hid_EP0rxBuf, receivedDataSize);
		
	}
	if ( evFreeEffectSetReportDataStage == ausb_pidEvent )
	{
		//constantForceEffectCreated = 0u;

	}

}

#define WHEELPOS_RESOLUTION_BITS 10u
#if ( WHEELPOS_RESOLUTION_BITS > 14u )
#error Function processWheelPos might not work correctly on resolution > 14 bits as overflow of uint16 can happen
#endif
#define WHEELPOS_RAW_VAL_MASK (0xFFFFu >> ( 16u - WHEELPOS_RESOLUTION_BITS ) )
#define WHEELPOS_RAW_MAX_VAL ( ( (uint16_t)(1u) << WHEELPOS_RESOLUTION_BITS ) - 1u )
#define WHEELPOS_OVERFLOW_THRESHOLD 255u
#define WHEELPOS_RAW_MAX_MINUS_THRESHOLD (WHEELPOS_RAW_MAX_VAL - WHEELPOS_OVERFLOW_THRESHOLD)
#define WHEELPOS_ONE_WHEEL_ROTATION_IN_SENSOR_ROTATIONS	1367  /*1.367 calculated from the values monitored with debugger ( (Max-Min) / (2 ^ WHEELPOS_RESOLUTION_BITS) )*/
#define WHEELPOS_MAX_VAL (uint16_t)(( (uint32_t)(WHEELPOS_RAW_MAX_VAL) * WHEELPOS_ONE_WHEEL_ROTATION_IN_SENSOR_ROTATIONS ) / 1000u) 
#define WHEELPOS_TOLERANCE 25u

/*wheel turns more that 1 time so overflow must be taken into account*/
static uint16_t processWheelPos( uint16_t mePos )
{
	/*magnetic encoder chip is configured for 10 bits by default but because
	the SPI peripheral is used to get the values 16 bits are retreived so first 6 MSbits are not used*/
	uint16_t wheepPosRaw = mePos & WHEELPOS_RAW_VAL_MASK;//1023u-(mePos << 6);
	uint32_t tWheelPos = 0x0u;

	if ( 0xFFFFu == wheelPosMin )
	{/*wheel is not calibrated yet - min value not set*/
		if ( 0u != btnShiftDw )
		{/*shift down button is pressed*/
			/*set min value to current wheel position but make sure the value is not 0xFFFFu*/
			if ( 0xFFFFu == wheepPosRaw ) wheepPosRaw--;
			wheelPosMin = wheepPosRaw + WHEELPOS_TOLERANCE; /*add also tolerance (overflow cannot happen here as the raw value is at most on 14 bits) so min pos is considered just before reaching the actual min - edge values are not always the same - they change a little bit usually with a value less then 10*/
			/*clear also the offset - it will be incremented on overflow from now on*/
			wheelPosOffset = 0u;
		}else{/*waiting for the button press: no actions needed as wheelPos calculation is disabled and is already initialized with some dummy value*/
		}
	}
	else
	{/*min value is set - so we can start calculating the position*/
		if ( ( wheepPosRaw <= WHEELPOS_OVERFLOW_THRESHOLD ) && ( wheelPosRawOld > WHEELPOS_RAW_MAX_MINUS_THRESHOLD ) )
		{/*owerflow detected*/
			/*add one encoder turn to the offset*/
			wheelPosOffset += WHEELPOS_RAW_MAX_VAL;
		}
		else if ( ( wheepPosRaw > WHEELPOS_RAW_MAX_MINUS_THRESHOLD ) && (wheelPosRawOld < WHEELPOS_OVERFLOW_THRESHOLD) )
		{/*underflow*/
			/*substract one encoder turn from the offset*/
			wheelPosOffset -= WHEELPOS_RAW_MAX_VAL;
		}

		wheelPos = wheepPosRaw + wheelPosOffset;

		if ( 0u == wheelPosMax )
		{/*wheel is not calibrated yet - max value not set*/
			if ( 0u != btnShiftUp )
			{/*shift up button is pressed*/
				/*set max value to current wheel position */
				wheelPosMax = wheelPos - WHEELPOS_TOLERANCE; /*substract tolerance to make sure that max value is always reached*/
			}else{/*wait for button press; no actions needed: the scaled value will not be calculated and the unscaled value will be transmitted via usb*/}
		}
	 	else
		{/*wheel is fully calibrated: min and max values are set*/
			/*scale weelPos to 0u-0xFFFFu*/
			/*normalize the value from [min..max] range to [0..(max-min)]; keep acount also that at edges because of tollerances the possition will be out of range*/
			if ( wheelPos <= wheelPosMin)
			{ 
				tWheelPos = 0;
			}
			else if ( wheelPos >= wheelPosMax) 
			{
				tWheelPos = (wheelPosMax - wheelPosMin);
			}	
			else
			{
				tWheelPos = wheelPos - wheelPosMin;
			}
			/*scale the value*/
			tWheelPos = (tWheelPos * 0xFFFFu) / (wheelPosMax - wheelPosMin);
			/*max value of tWheelPos is 0xFFFFu so the assignement below will have no loss of value*/
			wheelPos = (uint16_t)tWheelPos;
		}
	}
	wheelPosRawOld = wheepPosRaw;

	return wheelPos;
}