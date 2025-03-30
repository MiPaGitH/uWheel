/*
 * aFFMotor.c
 *
 *  Created on: Jan 23, 2016
 *      Author: mircea patras
 */
#include "affmotor.h"
#include "ausb.h"
#include "main.h"
#include "stm32f1xx_hal.h"

#define PWM_DEADZONE_DIVIDER (int16_t)5

static uint16_t pwmPulseVal;
static uint8_t directionChangeTimer;
static int16_t lastUnprocessedMagnitude;
enum
{
	dirLeft,
	dirRight
};

static uint8_t directionVal;

extern TIM_HandleTypeDef htim3;
extern TIM_OC_InitTypeDef sConfigTim3;

void affmotor_init( void )
{
	pwmPulseVal = 0u;
	directionVal = dirLeft;

	directionChangeTimer = 0u;

	lastUnprocessedMagnitude = 0;

	affmotor_enable();
}

/*turn on motor power*/
void affmotor_enable( void )
{
	// HAL_GPIO_WritePin(GPIOA, FFMotorRight_EnPin, GPIO_PIN_SET);
	// HAL_GPIO_WritePin(GPIOA, FFMotorLeft_EnPin, GPIO_PIN_SET);
}

/*turn off motor power*/
void affmotor_disable( void )
{
	HAL_GPIO_WritePin(GPIOA, FFMotorRight_EnPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, FFMotorLeft_EnPin, GPIO_PIN_RESET);
}

/*start motor*/
void affmotor_start( void )
{

  	if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK)
  	{
    	/* PWM Generation Error */
    	Error_Handler();
  	}
}

/*stop motor*/
void affmotor_stop( void )
{
	if (HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1) != HAL_OK)
  	{
    	/* PWM Generation Error */
    	Error_Handler();
  	}
}

/*
params:
	direction - negative or positive  
				LFS doesn't change the direction value (is always 2); but constant force magnitude goes from -10000 to 10000 (depending on the direction of the centrifugal force: left or right)
				TODO see if other games use the direction bit in set effect report

	cfMagnitude - value received from usb; range is -10000 - +10000 (but only 0-10000 should be received; TODO: test this )
*/
static int16_t cfMagnitudeOld;
void affmotor_updatePwmDC( /*uint8_t direction,*/ int16_t cfMagnitude )
{
	/*save value in case is needed later after a rotation direction change*/
	lastUnprocessedMagnitude = cfMagnitude;

	//int16_t pwmDeadZone = FFMotorPWMPeriod/PWM_DEADZONE_DIVIDER;
	/*limit value to max range [-10000 .. 10000] this is the range of the values received via usb*/
	if ( cfMagnitude > 10000 )
	{
		cfMagnitude = 10000;
	}
	else if ( cfMagnitude < ( -10000 ) )
	{
		cfMagnitude = -10000;
	}
	/*limit the min value*/
	// else if ( ( cfMagnitude < pwmDeadZone ) && ( cfMagnitude > ( (int16_t)0 - pwmDeadZone ) ) )
	// {/*value is in the range [-min:0:min]*/
	// 	cfMagnitude = 0u;
	// }
	else{/*value is in normal range: no limiting needed*/}

	/*scale magnitude from range [-10000 .. 0 .. +10000] to [-FFMotorPWMPeriod .. 0 .. +FFMotorPWMPeriod]*/
	cfMagnitude = (int16_t) ( ( (int32_t)cfMagnitude * FFMotorPWMPeriod ) / 10000 );	
	
	if ( ( (cfMagnitude > 0 ) && ( cfMagnitudeOld < 0 ) ) ||
		 ( (cfMagnitude < 0 ) && ( cfMagnitudeOld > 0 ) ) )
	{/*rotation direction change*/
		/*set pwm to 0*/
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0u);
		/*clear direction pins*/
		HAL_GPIO_WritePin(GPIOA, FFMotorRight_EnPin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, FFMotorLeft_EnPin, GPIO_PIN_RESET);
		/*start cool off timer*/
		directionChangeTimer = 9u; /*time in ms as the variable in decremented in 1ms task*/
	}

	cfMagnitudeOld = cfMagnitude;

	if ( 0u == directionChangeTimer )
	{/*no rotation direction change was just requested*/
		if ( cfMagnitude > 0 )
		{/*try to turn the wheel to the left*/
			HAL_GPIO_WritePin(GPIOA, FFMotorRight_EnPin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, FFMotorLeft_EnPin, GPIO_PIN_SET);
			pwmPulseVal = cfMagnitude;
		}
		else
		{/*try to turn the wheel to the right*/
			HAL_GPIO_WritePin(GPIOA, FFMotorLeft_EnPin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, FFMotorRight_EnPin, GPIO_PIN_SET);
			pwmPulseVal = (int16_t)0 - cfMagnitude;
		}
		/*scale to max pulse val*/
		pwmPulseVal = (uint16_t) ( ( (uint32_t)pwmPulseVal * FFMotorPulseMaxVal ) / 100 ) + 1;			
		/*update PWM pulse (duty ratio)*/
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwmPulseVal);
		//todo add a pin for right motor
	}
	
}

// int16_t testSpeed;
// int16_t testSpeedOld;
void affmotor_task( void )
{
	if ( directionChangeTimer > 0u )
	{
		directionChangeTimer--;
	}
	else
	{/*time out*/
		/*call update pwm as the last command was not processed as the direction was changed*/
		affmotor_updatePwmDC(lastUnprocessedMagnitude);
	}

	// /*TODO - comment this test code*/
	// if (  testSpeedOld != testSpeed )
	// {
	// 	if ( 0 == testSpeedOld )
	// 	{
	// 		affmotor_start();
	// 	}
	// 	else if ( 0 == testSpeed )
	// 	{
	// 		affmotor_stop();
	// 	}
	// 	if ( 0 != testSpeed  )
	// 	{
	// 		affmotor_updatePwmDC(testSpeed);
	// 	}
	// }
	// testSpeedOld = testSpeed;
}

