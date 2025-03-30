/*
 * amagenc.c
 *
 *  Created on: Oct 9, 2015
 *      Author: mircea.patras
 */
#include "amagenc.h"
#include "stm32f1xx_hal.h"
#include "tim.h"

#define MACRO_SSI_STOP 1u
/*static variables*/
static uint8_t ssiFlags;
static uint8_t	ssiBitCnt;
static uint16_t ssiSample;
/*exported variables*/
uint16_t amagenc_ssiSample;

/*static functions*/
static void amagenc_stopSSIClk( void );

void amagenc_init( void )
{
	/*Note: Peripheral Clock, GPIO and TIM is initialized by STMCubeMx generated modules: gpio.c and tim.c*/

	// TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	// TIM_OCInitTypeDef TIM_OCStructure;
	// NVIC_InitTypeDef NVIC_InitStructure;

	/*iniT global variables*/
	ssiBitCnt = SSI_BITS_PER_SAMPLE + 1; /*one is added so we get one extra negative edge for the last bit*/
	ssiSample = 0u;
	amagenc_ssiSample = 0u;
	ssiFlags = 0u;
}

/*periodic task to check the end of transmission*/
void amagenc_perTask( void )
{
	if ( 0u != ( ssiFlags & MACRO_SSI_STOP ) )
	{/*SSI stop requested*/
		amagenc_stopSSIClk();
		/*clear flag*/
		ssiFlags &= ( 0xFFu ^ MACRO_SSI_STOP );
	}
}

void amagenc_startSSIClk( void )
{
	ssiBitCnt = SSI_BITS_PER_SAMPLE + 1u; /*one is added so we get one extra negative edge for the last bit*/
	ssiSample = 0u;

	HAL_TIM_Base_Start_IT(&htim4);

}

static void amagenc_stopSSIClk( void )
{
	HAL_TIM_Base_Stop_IT(&htim4);
}

void HAL_TIM_Clbk_MagEnc(TIM_HandleTypeDef *htim)
{

	if (GPIO_PIN_SET == HAL_GPIO_ReadPin(magEnc_SSIClk_GPIO_Port, magEnc_SSIClk_Pin) )
	{/*clk line is HIGH*/
		/*clear clk pin*/
		HAL_GPIO_WritePin(magEnc_SSIClk_GPIO_Port, magEnc_SSIClk_Pin, GPIO_PIN_RESET);
		/**/
		if (ssiBitCnt > 0)
		{/*new bit received*/
			/*read data on negative edge of the CLK*/
			ssiBitCnt--;
			if ( ssiBitCnt < SSI_BITS_PER_SAMPLE )
			{
				//amagenc_ssiSample |= ( GPIO_ReadInputDataBit(AMAGENC_TIM_PINS_PORT, AMAGENC_SSIDI_PIN) << amagenc_ssiBitCnt );
				ssiSample |= ( HAL_GPIO_ReadPin(magEnc_SSIDI_GPIO_Port, magEnc_SSIDI_Pin) << ssiBitCnt );
			}else{/*first falling edge: ignore it because no data is available until first rising edge is detected by the MagEnc IC*/}
		}else{/*no actions: all bits received*/}
	}
	else
	{/*clk line is low*/
		/*set the CLK pin: the magnetic encoder updates the data line on positive edge of the clock*/
		HAL_GPIO_WritePin(magEnc_SSIClk_GPIO_Port, magEnc_SSIClk_Pin, GPIO_PIN_SET);
		if ( 0u == ssiBitCnt )
		{/*data was received and CLK line is High*/
			/*save received value to external variable*/
			amagenc_ssiSample = ssiSample;
			ssiSample = 0u;
			/*stop SSI CLK */
			//ssiFlags |= MACRO_SSI_STOP;
			HAL_TIM_Base_Stop_IT(&htim4);
		}
	}

}