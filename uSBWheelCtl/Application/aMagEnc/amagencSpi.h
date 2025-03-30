/*
 * amagencSpi.h
 *
 *  Created on: 7 May, 2020
 *      Author: mircea.patras
 */

#ifndef APPLICATION_AMAGENC_AMAGENC_SPI_H_
#define APPLICATION_AMAGENC_AMAGENC_SPI_H_

#include "stm32f1xx_hal.h"

extern uint16_t amagenc_ssiSample;

extern void amagencSpi_init( void );
extern void amagencSpi_task( void );

#endif /* APPLICATION_AMAGENC_AMAGENC_SPI_H_ */