/*
 * apedals.h
 *
 *  Created on: Sep 23, 2015
 *      Author: mirsiralu
 */

#ifndef APPLICATION_AACCPEDAL_AACCPEDAL_H_
#define APPLICATION_AACCPEDAL_AACCPEDAL_H_

extern void apedals_init( void );
extern void apedals_StoreNewValues( uint8_t buf[] );
extern void apedalsAsyncTask( void );
extern void apedalsPerTask( uint8_t per);


#endif /* APPLICATION_AACCPEDAL_AACCPEDAL_H_ */
