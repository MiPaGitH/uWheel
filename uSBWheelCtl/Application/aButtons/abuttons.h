/*
 * abuttons.h
 *
 *  Created on: FEB 06, 2014
 *      Author: mircea.patras
 *
 *  Description
 *  	Implements Buttons manager application module
 *
 *  History
 *  	FEB 06, 2014 file creation
 */
#ifndef _ABUTTONS_H
#define _ABUTTONS_H

#include "aSchTT_types.h"

/**
* \brief 'definition of BUTTON states
*/
enum abuttonsStates {
			ABUTTONS_STATUS_OFF = 0,
			ABUTTONS_STATUS_ON	
};

/*Exported data*/
extern UINT8 abuttons_bStartDrillStatus;

/*Exported functions*/
extern void abuttons_init( void );
extern void abuttons_manage( void );
extern UINT16 abuttons_getBStartPressedTime( void );

#endif
