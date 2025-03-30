/*
 * aFFMotor.h
 *
 *  Created on: Jan 23, 2016
 *      Author: mirsiralu
 */

#ifndef APPLICATION_AFFMOTOR_AFFMOTOR_H_
#define APPLICATION_AFFMOTOR_AFFMOTOR_H_

#include "stm32f1xx_hal.h"

extern void affmotor_init( void );
extern void affmotor_enable( void );
extern void affmotor_disable( void );
extern void affmotor_start( void );
extern void affmotor_stop( void );
extern void affmotor_updatePwmDC( /*uint8_t direction,*/ int16_t cfMagnitude );

extern void affmotor_task( void );

#endif /* APPLICATION_AFFMOTOR_AFFMOTOR_H_ */
