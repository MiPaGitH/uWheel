#ifndef __H_ABLE2UART_H_
#define __H_ABLE2UART_H_

#include "stm32f1xx_hal.h"
#include "aBle2UARTApi.h"

/* User can use this section to tailor USARTx/UARTx instance used and associated
   resources */
/* Definition for USARTx clock resources */
#define USARTx                           USART1
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE();
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define ABLEMODULE_RST_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for BLE module RST pin ( 0 == reset )*/
#define ABLEMODULE_RST_PIN               GPIO_PIN_8
#define ABLEMODULE_RST_GPIO_PORT         GPIOA

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_9
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_RX_PIN                    GPIO_PIN_10
#define USARTx_RX_GPIO_PORT              GPIOA

/* Definition for USARTx's DMA */

#define USARTx_TX_DMA_CHANNEL             DMA1_Channel4
#define USARTx_RX_DMA_CHANNEL             DMA1_Channel5



/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn                DMA1_Channel4_IRQn
#define USARTx_DMA_RX_IRQn                DMA1_Channel5_IRQn

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      USART1_IRQn

#endif