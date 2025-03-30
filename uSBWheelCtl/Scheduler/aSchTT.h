/*
 * aSchTT.h
 *
 *  Created on: Jul 24, 2013
 *      Author: mircea.patras
 */

#ifndef ASCHTT_H_
#define ASCHTT_H_

#include "aSchTT_types.h"

/**DEFINES
 */

/**TYPEDEFS
 */

/*task structure*/
typedef struct aSchTT_TaskType
{
	//TODO compress State with OnStack because uses only 2 bits + 1 bit
	ttTaskStateType State;		/*current task state*/
	BOOLEAN OnStack;			/*TRUE-if task context is saved on stack, FALSE otherwise*/

	UINT16 Period;				/*run period in tick's*/
	SINT16 Timer;				/*decreases at every OS tick -> 0 - change of State from SUSPENDED -> READY*/
	Routine TaskFunction;		/*pointer to task function*/

}ttTaskType;

/**Exported functions
 */
extern void aSchTTMng( void );
extern void aSchTTUpdate( void );

#endif /* ASCHTT_H_ */
