/*
 * aSchTTTasks.h
 *
 *  Created on: Jul 24, 2013
 *      Author: mircea.patras
 */

#ifndef ASCHTTTASKS_H_
#define ASCHTTTASKS_H_

#include "aSchTT.h"

/* tick period in milliseconds used by scheduler; calculated by GENERATOR*/
#define ASCHTT_OS_TICK_PERIOD_MS 	1ul
/* tick period is calculated as the greatest common divider of all task periods
 * example: Periods for tasks 1 and 2 are 3ms and 10ms -> the greatest common divider is 1ms
 * 										  6ms and 10ms -> tick time is 2ms */

/*number of tasks; value set by GENERATOR*/
#define ASCHTTTASKS_NBOF_TASKS		(UINT8)(6u)

/*exported variables*/
extern ttTaskType aSchTTDispTbl_Tasks[ASCHTTTASKS_NBOF_TASKS];

/*exported functions*/
/*this task is always present: runs in the time between the end of last periodic task function and the start of the new OS cycle*/
extern void task_idle( void ); 
/*periodic tasks*/
extern void task_1ms( void );
extern void task_10ms( void );
extern void task_10ms_Offset( void );
extern void task_100ms( void );
extern void task_500ms( void );
extern void task_1000ms( void );

#endif /* ASCHTTTASKS_H_ */
