/**
  ******************************************************************************
  * File Name          : TIM.c
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
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

/* Includes ------------------------------------------------------------------*/
#include "tim.h"
#include "amagenc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim3;
TIM_OC_InitTypeDef sConfigTim3;
TIM_HandleTypeDef htim4;

//timer used for FF motor PWM
void MX_TIM3_Init(void)
{
  htim3.Instance = TIM3;
  //71u 12V - the power source stopped (entered protection) immediately (only one channel used)
  //0u 12V - and max duty cycle of 12% works ok but high pitch sound is heard (2 channels of Pololu MD03A are used)
  //71u 5v - max duty cycle 100% - motor does not spin - only makes noise; current is at about 1 - 2 Amps max
  //71u 12v - max duty cycle 100% - power supply stops (enters protection)
  //71u 12v, max duty cycle 20% - ok - current is under 5A, sound is loud and annoying, 
  //71u 12v, max duty cycle 38% - ok - current is under 14A but at some time the MD03A chips overheat (maybe with heatsink is better), sound is loud and annoying, 
  //35u 12v, max duty cycle 32% - ok - current is at max 14A but at some time the MD03A chips overheat (maybe with heatsink is better), sound is loud and annoying, (pitch is higler)
  //10u 12v - max duty cycle 32% - ok - current is at max 24A but also takes some time for MD03A to overheat, sound is annoying with high pitch
  //2 12v, 32% - ok current reaches 28A, MD03A overheats at some point, sound is very very annoying
  //0 12V, 32% - not ok current reaches 40A and wires are MELTING, MD03A overheats at some point, sound is still annoying but much silent and higher pitch
  //71u 12v, max duty cycle 30% - ok but very noisy
  //0, 12v, 20% : OK no noise (MD03A both channels used in parallel for the wheel motor 
                  // ( Pwm period is 3600 for 20kHz frequency ) 
                  // max current from PS is 10 A and measured on the motor wires is 30 A; 
                  // MD03A overheats but PS does not go to overprotection
  //same as above but with radiator - ok for one lap but then the radiator gets very hot and the MD03A chip enters overtemperature protection
  //same as above but only 10% max duty cycle: OK no overheating no noise all ok (force feedback feels could be a little stronger)
  //same as above but with 15% max duty cycle: after 2-3 laps the chip enters overtemperature protection
  //same as above but with 12% max duty cycle: ok, after a few laps chips ok no overheat protection, force feedback is strong enough

  htim3.Init.Prescaler = 0u;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = FFMotorPWMPeriod; //with prescaler 71 (+1) this will give a 10 ms period; with 2 (+1) about 0.4 ms; with 0 - 0.1 ms
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  htim3.Init.RepetitionCounter = 0u;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigTim3.OCMode       = TIM_OCMODE_PWM1;
  sConfigTim3.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfigTim3.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfigTim3.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  sConfigTim3.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  sConfigTim3.OCIdleState  = TIM_OCIDLESTATE_RESET;

  /* Set the pulse value for channel 1 */
  sConfigTim3.Pulse = FFMotorPWMPeriod/2u;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigTim3, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Configuration Error */
    Error_Handler();
  }

}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
  GPIO_InitTypeDef   GPIO_InitStruct;
  if( htim->Instance == TIM3 )
  {
    __HAL_RCC_TIM3_CLK_ENABLE();

    /*GPIO Channels Clock is already enabled in gpio.c */

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = FFMotorPWM_T3CH1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  //    /*Configure GPIO pin Output Level */
  // HAL_GPIO_WritePin(GPIOA, FFMotorPWM_T3CH1, GPIO_PIN_RESET);
  }

}

#ifdef SSI_WITH_TIMER 
//Timer used for magnetic encoder SS comunication
void MX_TIM4_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 29; // 72MHz : 30 = 2.4 MHz
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 11u; //2.4 MHz : 12 = 0.2 MHz (and /2 because 2 interrupts per clock cycle are needed: 100KHz)
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  htim4.Init.RepetitionCounter = 0u;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim4);

}
#endif

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_ocHandle)
{
#ifdef SSI_WITH_TIMER 
  if(tim_ocHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspInit 0 */

  /* USER CODE END TIM4_MspInit 0 */
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* TIM4 interrupt Init */
    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspInit 1 */

  /* USER CODE END TIM4_MspInit 1 */
  }
#endif
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{
#ifdef SSI_WITH_TIMER 
  if(timHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspPostInit 0 */
  //no actions needed
  /* USER CODE END TIM4_MspPostInit 0 */
  

  /* USER CODE BEGIN TIM4_MspPostInit 1 */

  /* USER CODE END TIM4_MspPostInit 1 */
  }
#endif
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_ocHandle)
{
#ifdef SSI_WITH_TIMER 
  if(tim_ocHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspDeInit 0 */

  /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /* TIM4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspDeInit 1 */

  /* USER CODE END TIM4_MspDeInit 1 */
  }
#endif
} 

/* USER CODE BEGIN 1 */
/*TIM OC callback*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
#ifdef SSI_WITH_TIMER
  if( htim->Instance == TIM4 )
	{/*this is MagEnc Timer used for SSI*/  
    HAL_TIM_Clbk_MagEnc(htim);
  }
#endif
}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
