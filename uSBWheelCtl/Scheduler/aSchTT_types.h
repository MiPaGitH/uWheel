/*
 * osek_types.h
 *
 *  Created on: 18.07.2013
 *      Author: GrAnd
 */

#ifndef OSEK_TYPES_H
#define OSEK_TYPES_H

/*defines*/
#define PRIVATE_DATA static

/*define basic types*/
typedef unsigned char UINT8, BOOLEAN;
typedef signed char SINT8;
typedef unsigned short int UINT16;
typedef signed short int SINT16;
typedef signed long int SINT32;
typedef unsigned long int aUINT32;


/*! \brief
 * 	\details
 */
typedef void (*Routine)(void);

/*! \brief
 * 	\details
 */
#ifndef TASKTYPEDEFINED
#define TASKTYPEDEFINED
#if (TOTAL_USER_TASKS < 255u)
typedef UINT8	TaskType;
#else
typedef UINT16	TaskType;
#endif
typedef TaskType *TaskRefType;

#define INVALID_TASK	(TaskType)(~0u)
#endif /* TASKTYPEDEFINED */

/*! \brief
 * 	\details
 */
#ifndef PRIORITYTYPEDEFINED
#define PRIORITYTYPEDEFINED
#if(OSEK_HIGHEST_PRIORITY > 255u)
typedef const UINT16	PriorityType;
#else
typedef UINT8		PriorityType;
#endif
typedef PriorityType *PriorityRefType;
#endif /* PRIORITYTYPEDEFINED */

/*! \brief
 * 	\details
 */
#ifndef STATUSTYPEDEFINED
#define STATUSTYPEDEFINED
typedef UINT8 ttStatusType;

#define TT_E_OK 			0u
#define TT_E_OS_ACCESS		1u
#define TT_E_OS_CALLEVEL	2u
#define TT_E_OS_ID			3u
#define TT_E_OS_LIMIT		4u
#define TT_E_OS_NOFUNC		5u
#define TT_E_OS_RESOURCE	6u
#define TT_E_OS_STATE		7u
#define TT_E_OS_VALUE		8u

#define TT_E_OS_SYS_FATAL	16u
/*OS messages*/
#define OS_SHUTTING_DOWN 1u
#endif /* STATUSTYPEDEFINED */

/*! \brief
 * 	\details
 */
#ifndef TASKSTATETYPEDEFINED
#define TASKSTATETYPEDEFINED
typedef enum e_TaskState
{
	ttSUSPENDED = 0u,				/*Timer is > 0*/
	ttPREEMPTED,					/*Task was running and then preempted*/
	ttRUNNING						/*Task is running*/
}ttTaskStateType, *ttTaskStateRefType;
#endif /* TASKSTATETYPEDEFINED */

/*! \brief
 * 	\details
 */
#ifndef APPMODETYPEDEFINED
#define APPMODETYPEDEFINED
typedef enum ci_AppMode
{
	ttNormal,
	ttDebug,
	ttBootLoader,
	ttEndOfLine
}ttAppModeType;
#endif /* APPMODETYPEDEFINED */

/*! \brief
 * 	\details
 */
#ifndef EVENTTYPEDEFINED
#define EVENTTYPEDEFINED
typedef struct s_Event
{
	UINT16 Counter;
	UINT16 AlarmPeriod;
	UINT8 AlarmAction;
	Routine AlarmCallback;
}EventType, *EventRefType;
#endif /* EVENTTYPEDEFINED */

/*! \brief
 * 	\details
 */
#ifndef RESOURCETYPEDEFINED
#define RESOURCETYPEDEFINED
typedef struct s_Resource
{

}ResourceType, *ResourceRefType;
#endif /* RESOURCETYPEDEFINED */

/*! \brief
 * 	\details
 */
#ifndef EVENTMASKTYPEDEFINED
#define EVENTMASKTYPEDEFINED
typedef struct
{

}EventMaskType;
#endif /* EVENTMASKTYPEDEFINED */

/*! \brief
 * 	\details
 */
typedef enum e_Action
{
	ACTIVATETASK,
	SETEVENT,
	ALARMCALLBACK
}ActionType;

#endif /* OSEK_TYPES_H */
