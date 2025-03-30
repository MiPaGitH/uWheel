/*
 * application_init.c
 *
 *  Created on: FEB 01, 2014
 *      Author: mircea.patras
 *
 *  Description
 *  	Implements application specific initializations
 *
 *  History
 *  	FEB 01, 2014 file creation
 *  	MAR 16, 2014 added initialization routines for stepper motor
 *  	SEP 27, 2014 added initialization routine for drill motor RPM measurement with PhotoDetector (TC0 Channel 0 used; Stepper Motor now uses TC0 Channel 1)
 */

#include "application_init.h"

/*private prototypes*/
//static void application_perclkInit( void ); /*Note: periferal clocks are initialized by STMCubeMx generated modules*/

/**
* \brief Initializes application modules
*/
void application_init(void)
{
	//application_perclkInit(); /*Note: periferal clocks are initialized by STMCubeMx generated modules*/

	/*application modules*/
	usbmain_init();
	aleds_init();
	abuttons_init();
	//amagenc_init();
	amagencSpi_init();
	apedals_init();
	affmotor_init();

}

/*Note: periferal clocks are initialized by STMCubeMx generated modules*/
// void application_perclkInit( void )
// {
// 	/*configure preScaler of Peripheral Clock 1: PCLK1*/
// 	  /* PCLK1 = HCLK/4 */
// 	RCC_PCLK1Config(RCC_HCLK_Div1);
// 	/*enable peripheral clock's*/
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
// 	/*Timers*/
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	/*used by FFMOTOR_PWM_TIMER_NB which temporary uses the user green LED (on pin PA0) PWM*/
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	/*used by AQUADRENC_TIM_NB*/
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	/*used by AMAGENC_TIM_NB*/

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

// 	/*AccPedal module (uses ADC)*/
// 	RCC_ADCCLKConfig(RCC_PCLK2_Div6); //ADCCLK = PCLK22/6 = 72/6=12MHz
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //Enable ADC1 Clock

// }
