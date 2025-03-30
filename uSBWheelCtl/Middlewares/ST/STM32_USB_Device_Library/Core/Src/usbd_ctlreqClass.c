/**
  ******************************************************************************
  * @file    usbd_ctlreqClass.c
  * @author  MiPa
  * @version V1.0
  * @date    28-December-2017 
  * @brief   This file provides the class USB requests implementation customized for PhisycalInterfaceDevice (PID)
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_ctlreqClass.h"
#include "usbd_ctlreq.h" /*needed for USBD_CtlError function*/
#include "usbd_ioreq.h"
#include "ausb.h"

#define USB_CLASSREQ_GET_REPORT    0x01
#define USB_CLASSREQ_GET_IDLE      0x02
#define USB_CLASSREQ_GET_PROTOCOL  0x03
#define USB_CLASSREQ_SET_REPORT    0x09
#define USB_CLASSREQ_SET_IDLE      0x0A
#define USB_CLASSREQ_SET_PROTOCOL  0x0B

extern uint16_t classReqHistory;
extern uint8_t usbd_hid_EP0rxBuf[];
extern uint8_t ausb_pidEvent;

void USBD_GetReport (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
void USBD_SetReport (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);


USBD_StatusTypeDef  USBD_ClassDevReq (USBD_HandleTypeDef  *pdev, USBD_SetupReqTypedef  *req)
{
/*this function is not needed because all device requests are standard (not class) requests*/

  return USBD_OK;
}

/**
* @brief  USBD_ClassItfReq
*         Handle class usb interface requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
USBD_StatusTypeDef  USBD_ClassItfReq (USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req)
{
  USBD_StatusTypeDef ret = USBD_OK;  
  
	/*TOGGLE THE trigger pin needed by logic analyzer*/
#ifdef DEBUGENABLE_USB
	HAL_GPIO_TogglePin(GPIOA, PIN_LogicAnalyzerTriggerOutOrClassReport);
  HAL_GPIO_TogglePin(GPIOA, PIN_LogicAnalyzerTriggerOutOrClassReport);
  HAL_GPIO_TogglePin(GPIOA, PIN_LogicAnalyzerTriggerOutOrClassReport);
#endif

  /*Class device specific requests:
    0x01 GET_REPORT 1 
    0x02 GET_IDLE 
    0x03 GET_PROTOCOL 2 
    0x04-0x08 Reserved 
    0x09 SET_REPORT 
    0x0A SET_IDLE 
    0x0B SET_PROTOCOL

  */
  switch (req->bRequest) 
  {
  case USB_CLASSREQ_GET_REPORT: 
    /*This request is mandatory and must be supported by all devices*/
    USBD_GetReport (pdev, req) ;
    break;
    
  case USB_CLASSREQ_GET_IDLE:                      

    break;
    
  case USB_CLASSREQ_GET_PROTOCOL:                    
    
    break;
    
  case USB_CLASSREQ_SET_REPORT:                 
    USBD_SetReport (pdev , req);
    break;
    
  case USB_CLASSREQ_SET_IDLE:                                  
    break;
    
    
  case USB_CLASSREQ_SET_PROTOCOL:   
    //USBD_SetFeature (pdev , req);    
    break;
    
  default:  
    USBD_CtlError(pdev , req);
    break;
  }
  
  return ret;
}

void USBD_GetReport (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{

  switch (req->wValue) //first byte report type (1-input, 3 - feature), second byte report id
  {
/*parse wValue according to:
 HID1_11.pdf (7.2 Class-Specific Requests; 7.2.1 Get_Report Request; wValue Report Type and Report ID )  
  and 
 pid1_01.pdf (page 42 (48 pdf): 3. Host then sends a Get Report request to device for the PID Block Load Report. The format of  the Get Report is shown below: )
  */
    case 0x0304:  /*4 - PID Block Load Report*/
    {
      ausb_pidEvent = evPidBlockLoadGetReport;
    }
    break;
    case 0x0301:  /*1 - PID Pool Report*/
      ausb_pidEvent = evPidPoolReport;
      classReqHistory = req->wValue;
    break;
    default:
      USBD_CtlError(pdev , req);
    break;
  }

}

void USBD_SetReport (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{

  //volatile uint32_t receivedDataSize = 0;
  switch (req->wValue) //first byte report type (2-output, 3-feature), second byte report id
  {
    case 0x0303:  //3 - Create New Effect Report
        ausb_pidEvent = evCreateNewEffectSetReport;
    break;
    case 0x0302:   //2 - device gain report
      //contains one byte with the overall gain
    break;
    case 0x0305:   //5 - block free report
      //contains one byte with index of efect block to be freed (should be 1-constant force in our case)
      ausb_pidEvent = evFreeEffectSetReport;
    break;
    default:
      USBD_CtlError(pdev , req);
    break;
  }
}

USBD_StatusTypeDef  USBD_ClassEPReq  (USBD_HandleTypeDef  *pdev, USBD_SetupReqTypedef  *req)
{
/*this function is not needed because all EndPoint requests are standard (not class) requests*/
  return USBD_OK;
}