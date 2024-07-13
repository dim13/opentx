/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x 
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* Includes ------------------------------------------------------------------*/

#include "usb_bsp.h"

#include "board.h"
#include "usbd_conf.h"

extern uint32_t SystemCoreClock;

#if defined USB_CLOCK_SOURCE_CRS
static void CRS_Config(void);
#endif

void USB_BSP_Init(USB_CORE_HANDLE *pdev) {

  RCC_AHBPeriphClockCmd(USB_RCC_AHBPeriph_GPIO, ENABLE);

#if defined USB_CLOCK_SOURCE_CRS
  RCC_USBCLKConfig(RCC_USBCLK_HSI48);

  CRS_Config();  
#else 
  RCC_HSEConfig(RCC_HSE_ON);
  
  /* Wait till HSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) {}
  
  /* Enable PLL */
  RCC_PLLCmd(ENABLE);
  
  /* Wait till PLL is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
  
  /* Configure USBCLK from PLL clock */
  RCC_USBCLKConfig(RCC_USBCLK_PLLCLK); 
#endif /*USB_CLOCK_SOURCE_CRS */ 

// included in RCC_APB1_LIST
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

#if defined(USB_GPIO_PIN_VBUS)
  /* Configure VBUS Pin */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = USB_GPIO_PIN_VBUS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(USB_GPIO, &GPIO_InitStructure);
#endif
}

void USB_BSP_Deinit(USB_CORE_HANDLE *pdev) {
  //nothing to do
}

void USB_BSP_EnableInterrupt(USB_CORE_HANDLE *pdev) {
  NVIC_SetPriority(USB_IRQn, 11);
  NVIC_EnableIRQ(USB_IRQn);
}

void USB_BSP_DisableInterrupt(USB_CORE_HANDLE *pdev) {
  NVIC_DisableIRQ(USB_IRQn);
}

void USB_BSP_DevConnect(USB_CORE_HANDLE *pdev) {

}

void USB_BSP_uDelay(const uint32_t usec) {
  delay_us(usec);
}

void USB_BSP_mDelay(const uint32_t msec) {
  delay_ms(msec);
}

#if defined USB_CLOCK_SOURCE_CRS
/**
  * @brief  Configure CRS peripheral to automatically trim the HSI 
  *         oscillator according to USB SOF
  * @param  None
  * @retval None
  */
static void CRS_Config(void)
{
  /*Enable CRS Clock*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CRS, ENABLE);
  
  /* Select USB SOF as synchronization source */
  CRS_SynchronizationSourceConfig(CRS_SYNCSource_USB);
  
  /*Enables the automatic hardware adjustment of TRIM bits: AUTOTRIMEN:*/
  CRS_AutomaticCalibrationCmd(ENABLE);
  
  /*Enables the oscillator clock for frequency error counter CEN*/
  CRS_FrequencyErrorCounterCmd(ENABLE);
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
