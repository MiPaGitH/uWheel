/*
 * amagencSpi.c
 *
 *  Created on: 7 May, 2020
 *      Author: mircea.patras
 *      Description: the SPI can be used in 16 bit mode to retreive data even when the encoder is configured in 10 bit mode
 */

#include "amagencSpi.h"
#include "stm32f1xx_ll_spi.h"

static uint16_t encVal;
static uint8_t firstTransfer;
/*exported variables*/
uint16_t amagenc_ssiSample;

void amagencSpi_init( void )
{ 
    encVal = 0u;
    firstTransfer = 1u;
    amagenc_ssiSample = 0u;
}

void amagencSpi_task( void )
{
    if ( 0u != LL_SPI_IsActiveFlag_RXNE( SPI2 ) )
    {/*data available in register*/
        encVal = LL_SPI_ReceiveData16( SPI2 );
        amagenc_ssiSample = encVal >> 5;
        /*clear flag and enable ext int for detecting first rising edge of clk*/
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);
        HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
        /*start new SPI transfer*/
        LL_SPI_Enable(SPI2);
        /*wait for one clock cycle (spi speed is 562.5 kBit/s) and then disable the spi: 
        easy way is just wait for the rising edge of the spi clk line 
        that is connected to input pin PB15
        check external interrupt configured for PB15*/
        
    }else{/*data is not available: this should happen only on the first call after start-up*/}

    if ( 0u != firstTransfer )
    {
        firstTransfer = 0u;
        /*start new SPI transfer*/
        LL_SPI_Enable(SPI2);
    }

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* GPIO_Pin must be spiClkFeedBack_Pin as only this one is configured with external interrupt*/
    /*rising edge on SPI clk: disable SPI to top the transaction when it ends (the current transaction will be finished even if only one bit was sent until now - see datasheet)*/
    LL_SPI_Disable(SPI2);
    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
}

