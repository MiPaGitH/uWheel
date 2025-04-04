/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

#define userLed_Pin GPIO_PIN_0
#define userLed_GPIO_Port GPIOA
#define userButton_Pin GPIO_PIN_1
#define userButton_GPIO_Port GPIOA
//magnetic encoder SimpleSerial pins
#ifdef SSI_WITH_TIMER
#define magEnc_SSIClk_Pin GPIO_PIN_6
#define magEnc_SSIClk_GPIO_Port GPIOB
#define magEnc_SSIDI_Pin GPIO_PIN_7
#define magEnc_SSIDI_GPIO_Port GPIOB
#else
/*SPI used (in simple receive only mode) for mag enc SSI interface*/
/*pins PB13-CLK and PB14-MISO are used - initialized in spi.c module */

/*SPI CLK feedback line needed for disabling SPI after first bit was sent - see datasheet ( spi master receive only)*/
#define spiClkFeedBack_Pin GPIO_PIN_15
#define spiClkFeedBack_GPIO_Port GPIOB
#define spiClkFeedBack_EXTI_IRQn EXTI15_10_IRQn
#endif
//Spike pins
#define PIN_LogicAnalyzerTriggerOutOrClassReport GPIO_PIN_2 /*PA2*/
#define PIN_LogicAnalyzerTriggerInReport GPIO_PIN_3 /*PA3*/
//FF motor
#define FFMotorLeft_EnPin GPIO_PIN_4 /*PA4*/
#define FFMotorRight_EnPin GPIO_PIN_5 /*PA5*/
#define FFMotorPWM_T3CH1 GPIO_PIN_6 /*PA6*/
#define FFMotorPort GPIOA
#define FFMotorPWMPeriod  (int16_t)3600 /*for 20kHz operation*/ //before was 10000
#define FFMotorPulseMaxVal 15 /*in percent [%]*/
#if ( FFMotorPulseMaxVal > 100 )
#error value is in percent [%] so range is [1 .. 100]
#endif
//buttons for Shifting Gears
#define pinShiftGearDw GPIO_PIN_0
#define pinShiftGearUp GPIO_PIN_1
#define portShiftGear GPIOB

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
