/*
 * aBle2Uart.c
 *
 *  Created on: Sep 23, 2015
 *      Author: M
 *  Description:
 *  	handles the Ble - UART transceiver to communicate with the pedals unit 
 */

#include "aBle2UART.h"
#include "aleds.h"
#include "apedals.h"

UART_HandleTypeDef UartHandle;
/*commands:
  a. $$$ - enter command mode
  b. c - try to connect to the last bonded device
  c. ci - start client operation on the RN4870/71
  d. chw - writes the content of the client service characteristic from the remote device by addressing its handle (parameters values are found by connecting first with a phone and getting the characteristic number of the notification service - 28 in this case; when writing 1 to the notification characteristic the BLE module starts sending notification packets to our module with the pedal position data)
*/
const uint8_t bleSetupScript[] = "03$$$02c\r03ci\r14chw,0028,0100\r";
/* Buffer used for transmission*/
static uint8_t aTxBuffer[14]; /*last command has 14 bytes and the rest are shorter*/

/* rx Buffer - Length calculation:
    a. reboot message: %REBOOT%% - 9 fixed bytes
    b. unknown device message: Unknown Device% - 15 fixed bytes
    c. response to $$$ command: CMD>  - 5 fixed bytes (4 plus last char is space)
    d. response to c\r command: 
        Trying\r\n - 8 fixed bytes
        %CONNECT,0,adr12Bytes% - 24 bytes
        %SECURED% - 9 bytes
        ERR  - 3 bytes
    e. response to ci\r command: AOK\r\nCMD> - 10 bytes ( including a space at the end)
    f. response to chw,****,0100\r: AOK\r\nCMD> - 10 bytes ( including a space at the end)
    g. notification message: %NOTI,0027,BY1BY2...BYn% - 12 fixed bytes (including last %) + 2*BYn */
static uint8_t aRxBuffer[24];
volatile static uint8_t cmdLength;
uint8_t sIdx; /*index to parse the bleSetupScript*/
uint8_t evBleRx;
uint8_t evTimeout;
uint16_t bleTim;
typedef enum
{
  bleRST,
  bleWaitInit,
  bleInitWaitRx,
  bleWaitRx,
  bleTimeOut
}bleStates;
bleStates aBleState;

static void aBle2UART_Setup( void );

void aBle2UART_init( void )
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  ABLEMODULE_RST_GPIO_CLK_ENABLE();

  /*reset the BLE module until the power is stable*/
  HAL_GPIO_WritePin(ABLEMODULE_RST_GPIO_PORT,ABLEMODULE_RST_PIN,GPIO_PIN_RESET);

  /*BLE module RST pin (0 == reset)*/
  GPIO_InitStruct.Pin       = ABLEMODULE_RST_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull      = GPIO_NOPULL; /*RST pin is pulled up by the BLE module*/
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(ABLEMODULE_RST_GPIO_PORT, &GPIO_InitStruct);


  /*##-1- Configure the UART peripheral ######################################*/
  UartHandle.Instance          = USARTx;

  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;

  if (HAL_UART_Init(&UartHandle) != HAL_OK) /*calls the function HAL_UART_MspInit that is defined below*/
  {
      /* Initialization Error */
      Error_Handler();
  }

  sIdx = 0u;
  bleTim = 300u; /*300 ms wait before releasing the rst pin of BLE external module*/
  aBleState = bleRST;
  evBleRx = 0u;
  evTimeout = 0u;
  cmdLength = 0u;

}

/*prepare the BLE module for data reception*/
void aBle2UART_Setup( void )
{
  uint8_t i;
  /*cmd is on 2 digits*/
  cmdLength = ((bleSetupScript[sIdx]-'0') * 10u) + (bleSetupScript[sIdx+1]-'0');

  sIdx+=2u;

  /*get the current command*/
  for (i = 0u; i< cmdLength; i++)
  {
    aTxBuffer[i] = bleSetupScript[sIdx+i];
  }

  /*execute the next command from the script buffer*/
  if (HAL_UART_Transmit_DMA(&UartHandle, aTxBuffer, cmdLength) != HAL_OK)
  {
      /* Transfer error in transmission process */
      Error_Handler();
  }
  sIdx+=cmdLength;

}


void aBleAsyncTask( void )
{

  switch (aBleState)
  {
    case bleRST:
      if (evTimeout)
      {
        evTimeout = 0u;
        /*release the reset of the BLE module*/
        HAL_GPIO_WritePin(ABLEMODULE_RST_GPIO_PORT,ABLEMODULE_RST_PIN,GPIO_PIN_SET);
        bleTim = 200u; //init timeout
        aBleState = bleWaitInit;
      }
    break;
    case bleWaitInit:
      if (evTimeout)
      {//timeout occured after reset was released
        evTimeout = 0u;
        //aBleState = bleTimeOut;
        bleTim = 200u; //reload timer
        /*send first command*/
        sIdx = 0u;
        aBle2UART_Setup();
        aBleState = bleInitWaitRx;
      }
    break;
    case bleInitWaitRx:
      /*check rx event*/
      if ( 0u != evBleRx )
      {/*response received - but because I did not call the DMA receive this event will not happen*/
        evBleRx = 0u;
        bleTim = 0u; //stop timer
        if (bleSetupScript[sIdx] != '\0')
        { 
          bleTim = 800u; //reload timer
          /*continue setup script*/
          aBle2UART_Setup();
        }
        else
        {/*setup script completed*/
          aBleState = bleWaitRx;
          /*##-3- Put UART peripheral in reception process ###########################*/
          if (HAL_UART_Receive_DMA(&UartHandle, aRxBuffer, 14u/*todo-put 16u when 2 pedals are attached*/) != HAL_OK) /*%NOTI,0027,BY%*/
          {
            /* Transfer error in reception process */
            Error_Handler();
          }
        }
      }
      if (evTimeout)
      {
        evTimeout = 0u;

        bleTim = 0u; //stop timer
        if (bleSetupScript[sIdx] != '\0')
        { 
          bleTim = 800u; //reload timer
          /*continue setup script*/
          aBle2UART_Setup();
        }
        else
        {/*setup script completed*/
          aBleState = bleWaitRx;
          /*##-3- Put UART peripheral in reception process ###########################*/
          if (HAL_UART_Receive_DMA(&UartHandle, aRxBuffer, 16u/*todo-put 16u when 2 pedals are attached*/) != HAL_OK) /*%NOTI,0027,BY%*/
          {
            /* Transfer error in reception process */
            Error_Handler();
          }
        }

      }
    break;
    case bleWaitRx:
      if ( 0u != evBleRx )
      {
        uint8_t rxSize = 16u;
        evBleRx = 0u;
        apedals_StoreNewValues(&aRxBuffer[11]);
        if ((aRxBuffer[0] != '%') && (aRxBuffer[1] != 'N') )
        {/*buffer is not aligned ( one part is from current telegram and the other from the last one)*/
          /*read one extra byte untill the buffer is aligned*/
          rxSize += 1u;
        }
        /*##-3- Put UART peripheral in reception process ###########################*/
        if (HAL_UART_Receive_DMA(&UartHandle, &aRxBuffer[0], rxSize) != HAL_OK) /*%NOTI,0027,BY%*/
        {
          /* Transfer error in reception process */
          Error_Handler();
        }
      }
    break;
    case bleTimeOut:
      //todo: if button is pressed then
      //sIdx = 0u;
      //aBleState = bleRST;
    break;
    default:
      aBleState = bleRST;
    break;
  }
}

void aBlePerTask( uint8_t per)
{
  if ( bleTim >= per )
	{
		bleTim-=per;
		if ( bleTim < per )
		{
	    evTimeout = 1u;
		}
	}
}


/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle.
  * @note   This example shows a simple way to report end of DMA Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}

/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  evBleRx = 1u;
}

/**
  * @brief  UART error callbacks
  * @param  huart: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    aleds_ledGreenStatus = ALEDS_LED_STATUS_BLINK;
}


/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

/**
  * @brief UART MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - DMA configuration for transmission request by peripheral
  *           - NVIC configuration for DMA interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  static DMA_HandleTypeDef hdma_tx;
  static DMA_HandleTypeDef hdma_rx;

  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();



  /* Enable USARTx clock */
  USARTx_CLK_ENABLE();

  /* Enable DMA clock */
  DMAx_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

  /*##-3- Configure the DMA ##################################################*/
  /* Configure the DMA handler for Transmission process */
  hdma_tx.Instance                 = USARTx_TX_DMA_CHANNEL;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;

  HAL_DMA_Init(&hdma_tx);

  /* Associate the initialized DMA handle to the UART handle */
  __HAL_LINKDMA(huart, hdmatx, hdma_tx);

  /* Configure the DMA handler for reception process */
  hdma_rx.Instance                 = USARTx_RX_DMA_CHANNEL;
  hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_rx.Init.Mode                = DMA_NORMAL;
  hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;

  HAL_DMA_Init(&hdma_rx);

  /* Associate the initialized DMA handle to the the UART handle */
  __HAL_LINKDMA(huart, hdmarx, hdma_rx);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (USARTx_TX) */
  HAL_NVIC_SetPriority(USARTx_DMA_TX_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USARTx_DMA_TX_IRQn);

  /* NVIC configuration for DMA transfer complete interrupt (USARTx_RX) */
  HAL_NVIC_SetPriority(USARTx_DMA_RX_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USARTx_DMA_RX_IRQn);

  /* NVIC configuration for USART, to catch the TX complete */
  HAL_NVIC_SetPriority(USARTx_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{

  static DMA_HandleTypeDef hdma_tx;
  static DMA_HandleTypeDef hdma_rx;

  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
  
  /*##-3- Disable the DMA Channels ###########################################*/
  /* De-Initialize the DMA Channel associated to transmission process */
  HAL_DMA_DeInit(&hdma_tx);
  /* De-Initialize the DMA Channel associated to reception process */
  HAL_DMA_DeInit(&hdma_rx);

  /*##-4- Disable the NVIC for DMA ###########################################*/
  HAL_NVIC_DisableIRQ(USARTx_DMA_TX_IRQn);
  HAL_NVIC_DisableIRQ(USARTx_DMA_RX_IRQn);
}

