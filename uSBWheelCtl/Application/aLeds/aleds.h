/*
 * aleds.h
 *
 *  Created on: FEB 02, 2014
 *      Author: mircea.patras
 *
 *  Description
 *  	Implements LED's application module
 *
 *  History
 *  	FEB 02, 2014 file creation
 */
 
#include "aSchTT_types.h"

/**
* \brief 'definition of LED states
*/
enum aledsStates {
			ALEDS_LED_STATUS_OFF = 0,
			ALEDS_LED_STATUS_ON,
			ALEDS_LED_STATUS_BLINK	
};

/*Exported data*/
extern UINT8 aleds_ledGreenStatus;
extern UINT16 aleds_ledGreenBlinkTimeMs;
extern UINT8 aleds_ledRedStatus;

/*Exported functions*/
extern void aleds_init( void );
extern void aleds_manage( void );
