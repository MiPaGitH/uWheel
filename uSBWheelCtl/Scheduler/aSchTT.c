/*
 * aSchTT.c
 *
 *  Created on: Jul 24, 2013
 *      Author: mircea.patras
 */

#include "aSchTT.h"
#include "application_tasks.h"
#include "main.h"

/*defines*/

/*external variables*/

/*variables*/

/* Scheduler task manager / dispatcher
 * Description:
 * 		scheduler main function
 * 		determines the task to be executed and starts it
 * 		3 cases:
 * 			no preemption: tasks run to completion
 * 			preemption: the function runs the pending function even if it means to stop the current running task
 * 			restore: if no more task's need to run searches for preempted tasks and runs them
 */
void aSchTTMng( void )
{
	UINT8 TaskIndex = 0u;

	for (TaskIndex = 0u; TaskIndex < ASCHTTTASKS_NBOF_TASKS; TaskIndex++)
	{

		if ( aSchTTDispTbl_Tasks[TaskIndex].Timer <= 0 )
		{

			/*check if TASK is suspended*/
			if ( aSchTTDispTbl_Tasks[TaskIndex].State == ttSUSPENDED )
			{
				/*set running flag*/
				aSchTTDispTbl_Tasks[TaskIndex].State = ttRUNNING;

				/*reload Timer*/
				aSchTTDispTbl_Tasks[TaskIndex].Timer = aSchTTDispTbl_Tasks[TaskIndex].Period;

				/*run the task*/
				(*aSchTTDispTbl_Tasks[TaskIndex].TaskFunction)();

				/*task finished*/
				aSchTTDispTbl_Tasks[TaskIndex].State = ttSUSPENDED;

				/*remove task if is not periodic (Period > 0)*/
				if ( 0 == aSchTTDispTbl_Tasks[TaskIndex].Period )
				{/*task is of type OneShot*/
					/*remove the task from the dispatch table*/
					aSchTTDispTbl_Tasks[TaskIndex].TaskFunction = 0;
					aSchTTDispTbl_Tasks[TaskIndex].Timer = 1;
				}else{/*n2do: task is periodic*/}

			}
			else /*task is already running: OSEK doesn't allow a task to preempt itself*/
			{
				/*report error*/

			}
		}
		else
		{
			/*nothing to do: timer not elapsed yet*/
		}
	}
	/*run the idle task*/
	task_idle();
}


/*update task timers*/
void aSchTTUpdate( void )
{
	/*index to parse the task's array*/
	UINT8 TaskIndex = 0u;


	for (TaskIndex = 0u; TaskIndex < ASCHTTTASKS_NBOF_TASKS; TaskIndex++)
	{
		if (aSchTTDispTbl_Tasks[TaskIndex].TaskFunction != 0)
		{/*decrement the timer if the task is active (OneShot tasks are removed after the first call: TaskFunction= 0)*/
			aSchTTDispTbl_Tasks[TaskIndex].Timer--;
		}

		/*has the task timer elapsed?*/
		if ( aSchTTDispTbl_Tasks[TaskIndex].Timer == 0 ) //XXX: was <=0
		{

			/*Check if Task was not stopped*/
			if ( ttSUSPENDED != aSchTTDispTbl_Tasks[TaskIndex].State )
			{
				/*task overrun condition: it is still running/preempted from last call*/
				/*report error*/

			}//if task not suspended
			else /*normal operation: task is suspended*/
			{
				/*check and report task drift: timer < 0 (task drift happens when a task runs over other task's start time*/
				if (aSchTTDispTbl_Tasks[TaskIndex].Timer < 0)
				{
					/*print OS message with drift report*/
					/*report error*/

				}

			}
		}//if Timer <= 0
		else
		{
			/*is the task running or preempted?*/
			if ( aSchTTDispTbl_Tasks[TaskIndex].State == ttRUNNING ) //was: != ttSUSPENDED )
			{
			}
		}
	}//for

}
