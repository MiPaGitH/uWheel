/*
 * events.h
 *
 *  Created on: MAR 24, 2014
 *      Author: mircea.patras
 *
 *  Description
 *  	Application Events definition
 *
 *  History
 *  	MAR 24, 2014 file creation
 */

#ifndef _MY_EVENTS_H
#define _MY_EVENTS_H

#include "aleds.h"
#include "ausb.h"

/*Start Button pressed event*/
#define MACRO_EV_BSTART_PRESSED	\
	aleds_ledGreenStatus = ALEDS_LED_STATUS_ON;
	/*TODO add clients to this event*/


/*Start Button released event*/
#define MACRO_EV_BSTART_RELEASED	\
	aleds_ledGreenStatus = ALEDS_LED_STATUS_OFF;
	/*TODO add clients to this event*/

#endif

