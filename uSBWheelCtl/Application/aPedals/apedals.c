/*
 * apedals.c
 *
 *  Created on: Sep 23, 2015
 *      Author: M
 *  Description:
 *  	Checks the values received from the BLE/UART modules for the pedals and if changed then updates them to be used by the USB module
 */

#include "stm32f1xx_hal.h"
#include "aBle2UARTApi.h"

uint8_t pAcc;
uint8_t pBrk;
// uint8_t pDataChanged;
uint8_t pData[2];

void apedals_init( void )
{
	pAcc = 0u;
	pBrk = 0u;
	pData[0] = 0u;
	pData[1] = 0u;
	// pDataChanged = 0u;

	aBle2UART_init();
}

void apedals_StoreNewValues( uint8_t buf[] )
{
	/*copy data from UART to pedals buffer*/
	/*first pedal*/
	/*add first character*/
	if ( (buf[0] >= '0') && (buf[0] <= '9') )
	{
		pAcc = (buf[0]-'0') * 16u;
	}
	else
	{/*must be between A and F*/
		pAcc = (10u + buf[0]-'A') * 16u;
	}
	/*add the second character*/
	if ( (buf[1] >= '0') && (buf[1] <= '9') )
	{
		pAcc += (buf[1]-'0');
	}
	else
	{/*must be between A and F*/
		pAcc += (10u + buf[1]-'A');
	}

	/*second pedal*/
	/*add first character*/
	if ( (buf[2] >= '0') && (buf[2] <= '9') )
	{
		pBrk = (buf[2]-'0') * 16u;
	}
	else
	{/*must be between A and F*/
		pBrk = (10u + buf[2]-'A') * 16u;
	}
	/*add the second character*/
	if ( (buf[3] >= '0') && (buf[3] <= '9') )
	{
		pBrk += (buf[3]-'0');
	}
	else
	{/*must be between A and F*/
		pBrk += (10u + buf[3]-'A');
	}
}

void apedalsAsyncTask( void )
{
	aBleAsyncTask();
}

// /*per - task period in ms*/
// void apedalsPerTask( uint8_t per )
// {

// 	if ( (pAcc != pData[0]) || (pBrk != pData[1]) )
// 	{
// 		pAcc = pData[0];
// 		pBrk = pData[1];
// 		pDataChanged = 1u;
// 	}

// }