/**
  ******************************************************************************
  * @file    hw_config.h
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _AUSB_H
#define _AUSB_H

/* Includes ------------------------------------------------------------------*/
#include "affmotor.h"
#include <stdint.h>

/*uncomment the line below to enable debug pins*/
//#define DEBUGENABLE_USB

/* Exported types ------------------------------------------------------------*/
typedef struct _pidEffectData
{
	uint16_t duration;
	uint8_t gain;
	int8_t direction;
	uint16_t startDelay;
	uint8_t envelopeAttackLevel;
	uint8_t envelopeFadeLevel;
	uint16_t envelopeAttackTime;
	uint16_t envelopeFadeTime;
	int16_t constantForceMagnitude; /*range is -10000 / + 10000*/
}pidEffectDataType;

/* Exported constants --------------------------------------------------------*/
#define AUSB_NEW_CONTROL_EVENT		0x01u
#define AUSB_WAIT_CREATE_NEW_EFFECT_REPORT 0x02
#define AUSB_CREATE_NEW_EFFECT_REPORT_RECEIVED	0x04u
#define AUSB_WAIT_SET_EFFECT_REPORT 0x08u
#define AUSB_SET_EFFECT_REPORT_RECEIVED 0x10u
#define AUSB_CONSTANT_FORCE_UPDATED 0x20u

#define AUSB_MAX_MAGNITUDE			(int16_t)10000

/* Exported macro ------------------------------------------------------------*/
#define AUSB_START_ACTUATOR 		affmotor_start();
#define AUSB_STOP_ACTUATOR			affmotor_stop();
#define AUSB_ENABLE_ACTUATOR		affmotor_enable();
#define AUSB_DISABLE_ACTUATOR   affmotor_disable();
/* Exported define -----------------------------------------------------------*/
#define         ID1          (0x1FFFF7E8)
#define         ID2          (0x1FFFF7EC)
#define         ID3          (0x1FFFF7F0)

enum ePidEvents
{
  evNone,
	evCreateNewEffectSetReport,
  evCreateNewEffectDataStage,
	evSetEffectSetReport,
  evSetEffectSetReportDataStage,
  evPidBlockLoadGetReport,
  evPidPoolReport,
  evFreeEffectSetReport,
  evFreeEffectSetReportDataStage
};

/* Exported functions ------------------------------------------------------- */
extern void usbmain_init(void);
extern void ausb_manage10ms( void );
extern void ausb_manage1ms( void );
extern void Leave_LowPowerMode(void);
extern void Get_SerialNum(void);
extern void ausb_initEffectsData( void );
extern void ausb_physicalInterfaceSM( void );


/* External variables --------------------------------------------------------*/
extern volatile uint8_t PrevXferComplete;
extern volatile uint8_t ausb_flags;
extern volatile uint8_t ausb_deviceGain;
extern pidEffectDataType ausb_effectData;
extern volatile uint8_t ausb_pidStateFlags; /*Bits: 0-Effect Playing, 1-Device paused, 2-ActuatorsEnabled, 3-SafetySwitch, 4-ActuatorOverrideSwitch,  5-Actuator Power */
extern volatile uint8_t ausb_effectDataLoopCnt;
extern uint8_t ausb_pidEvent;
extern uint8_t ausb_OperationReportEffectIdx;

#endif  /*_AUSB_H*/
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
