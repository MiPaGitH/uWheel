/*
 * amagenc.h
 *
 *  Created on: Oct 9, 2015
 *      Author: mircea.patras
 */
#include "stm32f1xx_hal.h"

#ifndef APPLICATION_AMAGENC_AMAGENC_H_
#define APPLICATION_AMAGENC_AMAGENC_H_

/*Labels*/
#define SSI_BITS_PER_SAMPLE_DEFAULT 	10u
#define SSI_BITS_PER_SAMPLE_APP			14u
#define SSI_BITS_PER_SAMPLE 			SSI_BITS_PER_SAMPLE_DEFAULT

//extern uint8_t	amagenc_ssiBitCnt;
extern uint16_t amagenc_ssiSample;

extern void amagenc_init( void );
extern void amagenc_startSSIClk( void );
extern void amagenc_perTask( void );
extern void HAL_TIM_Clbk_MagEnc(TIM_HandleTypeDef *htim);

#endif /* APPLICATION_AMAGENC_AMAGENC_H_ */
