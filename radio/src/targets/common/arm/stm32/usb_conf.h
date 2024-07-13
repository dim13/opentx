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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USB_CONF_H_
#define _USB_CONF_H_

#include "STM32F0xx_StdPeriph_Lib_V1.5.0/Libraries/CMSIS/Device/ST/STM32F0xx/Include/stm32f0xx.h"

/* Select the CRS using HSI48 internal clock as USB clock source.
    If this define is commented, PLL will be used as USB clock source.
    User need to ensure that the PLL output clock to USB is 48MHz  for proper 
    USB functioning */
#define INTERNAL_PULLUP

#define USB_CLOCK_SOURCE_CRS

// #define USB_DEVICE_LOW_PWR_MGMT_SUPPORT

/* Endpoints used by the device */
// #define EP_NUM     (2)  /* EP0 + EP1 IN For HID */
#define EP_NUM    (3) /* EP0 + EP1 for MSC IN + EP2 for MSC OUT */
// #define EP_NUM     (4)  /* EP0 + EP1 For HID + EP2/EP3 for CDC  */

/* Buffer table base address */
#define BTABLE_ADDRESS    (0x00)

/* EP0, RX/TX buffers base address */
#define ENDP0_RX_ADDRESS    (0x18)
#define ENDP0_TX_ADDRESS    (0x58)

/* EP1, TX buffer base address */
#define MSC_IN_TX_ADDRESS     (0x98)
#define HID_IN_TX_ADDRESS     (0x98)
    
/* EP2, Rx buffer base address */
#define MSC_OUT_RX_ADDRESS    (0xD8)
#define HID_OUT_RX_ADDRESS    (0xD8)

/* EP2 Tx buffer base address */
#define BULK_IN_TX_ADDRESS  (0xC0) 

/* EP2 Rx buffer base address */
#define BULK_OUT_RX_ADDRESS (0x110)

/* EP3 Tx buffer base address */
#define INT_IN_TX_ADDRESS   (0x100)

/* Macro to get variable aligned on 4-bytes, for __ICCARM__ the directive "#pragma data_alignment=4" must be used instead */
#if defined   (__GNUC__)        /* GNU Compiler */
  #ifndef __ALIGN_END
    #define __ALIGN_END    __attribute__ ((aligned (4)))
  #endif /* __ALIGN_END */
  #ifndef __ALIGN_BEGIN
    #define __ALIGN_BEGIN
  #endif /* __ALIGN_BEGIN */
#else
  #ifndef __ALIGN_END
    #define __ALIGN_END
  #endif /* __ALIGN_END */
  #ifndef __ALIGN_BEGIN
    #if defined   (__CC_ARM)      /* ARM Compiler */
      #define __ALIGN_BEGIN    __align(4)
    #elif defined (__ICCARM__)    /* IAR Compiler */
      #define __ALIGN_BEGIN
    #endif /* __CC_ARM */
  #endif /* __ALIGN_BEGIN */
#endif /* __GNUC__ */

/* __packed keyword used to decrease the data type alignment to 1-byte */
#if !defined(__packed)
  #if defined (__CC_ARM)         /* ARM Compiler */
    #define __packed    __packed
  #elif defined (__ICCARM__)     /* IAR Compiler */
    #define __packed    __packed
  #elif defined   ( __GNUC__ )   /* GNU Compiler */                        
    #define __packed    __attribute__ ((__packed__))
  #elif defined   (__TASKING__)  /* TASKING Compiler */
    #define __packed    __unaligned
  #endif /* __CC_ARM */
#endif

#endif // _USB_CONF_H_

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

