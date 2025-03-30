/*
 * aplication_tasks.c
 *
 *  Created on: July 24, 2013
 *      Author: MP
 *		Description: here the dispatch table and the periodic tasks are defined
 *		History
 *			July 24, 2013 file creation
 *			February 2, 2014 rename from aschtttasks.c to application_tasks.c and update the dispatch table
 */

#include "main.h"
#include "aSchTT_types.h"
#include "aSchTT.h"
#include "application_tasks.h"
#include "aleds.h"
#include "abuttons.h"
#include "ausb.h"
#ifdef SSI_WITH_TIMER
#include "amagenc.h"
#else
#include "amagencSpi.h"
#endif
#include "apedals.h"
#include "aBle2UARTApi.h"


/**defines
 */

/**variables
 */
extern UINT8 osTickExecuted;

/**function prototypes
 */
UINT8 dispTableCheck(UINT16 *pMin, UINT16 *pMax);

/**GENERATED DISPATCH TABLE
 */
ttTaskType aSchTTDispTbl_Tasks[ASCHTTTASKS_NBOF_TASKS] = {
					{0u, 	0u, 	1u, 	0, 		task_1ms},
					{0u, 	0u, 	10u, 	0, 		task_10ms},
					{0u, 	0u, 	10u, 	4,		task_10ms_Offset},
					{0u, 	0u, 	100u, 	6, 		task_100ms},
					{0u, 	0u, 	500u, 	1, 		task_500ms},
					{0u, 	0u, 	1000u, 	1000u, 	task_1000ms}

				};


/*THE IDLE TASK*/
void task_idle( void )
{
#ifdef SSI_WITH_TIMER
	amagenc_perTask();
#endif
	ausb_physicalInterfaceSM();
	apedalsAsyncTask();
}

/**GENERATED TASK definitions
 */
void task_1ms( void )
{
	ausb_manage1ms();
	affmotor_task();
}

void task_10ms( void )
{
  ausb_manage10ms();
  aBlePerTask( 10u );
}

void task_10ms_Offset( void )
{
#ifdef SSI_WITH_TIMER
  	/*magnetic encoder: get new sample: starts SSI CLK and activates SSI interrupt where Wheel data from the magnetic encoder is sampled*/
	amagenc_startSSIClk();
#else
	amagencSpi_task();
#endif
	/*leds control*/
	aleds_manage();

}

void task_100ms( void )
{

	abuttons_manage();
	// apedalsPerTask(100u);
	// affmotor_moveMotorsToKeepWireTension();

}

void task_500ms( void )
{

}
void task_1000ms( void )
{

}
