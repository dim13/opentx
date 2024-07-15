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

#if defined(__cplusplus)
extern "C" {
#endif
#include "usb_dcd_int.h"
#include "usb_bsp.h"
#if defined(__cplusplus)
}
#endif

#include "opentx.h"
#include "debug.h"
#if defined(ADC_JOYSTICK)
#include "board.h"
#endif

static bool usbDriverStarted = false;
#if defined(BOOT)
static usbMode selectedUsbMode = USB_MASS_STORAGE_MODE;
#else
static usbMode selectedUsbMode = USB_UNSELECTED_MODE;
#endif

int getSelectedUsbMode()
{
  return selectedUsbMode;
}

void setSelectedUsbMode(int mode)
{
  selectedUsbMode = usbMode (mode);
}

int usbPlugged()
{
#if defined(PCBI6X) && !defined(PCBI6X_USB_VBUS)
  if (globalData.usbConnect) {
    return 1;
  }
#endif

#if defined(USB_GPIO_PIN_VBUS)
  // debounce
  static uint8_t debounced_state = 0;
  static uint8_t last_state = 0;

  if (GPIO_ReadInputDataBit(USB_GPIO, USB_GPIO_PIN_VBUS)) {
    if (last_state) {
      debounced_state = 1;
    }
    last_state = 1;
  }
  else {
    if (!last_state) {
      debounced_state = 0;
    }
    last_state = 0;
  }
  return debounced_state;
#else
  return 0;
#endif // USB_GPIO_PIN_VBUS
}

USB_CORE_HANDLE USB_Device_dev;

extern "C" void USB_IRQHandler()
{
  USB_Istr();
}

void usbInit()
{
  // Initialize hardware
  USB_BSP_Init(&USB_Device_dev);
  usbDriverStarted = false;
}

void usbStart()
{
  switch (getSelectedUsbMode()) {
#if !defined(BOOT)
    case USB_JOYSTICK_MODE:
      // initialize USB as HID device
      USBD_Init(&USB_Device_dev, &USR_desc, &USBD_HID_cb, &USR_cb);
      break;
#endif
#if defined(USB_SERIAL)
    case USB_SERIAL_MODE:
      // initialize USB as CDC device (virtual serial port)
      USBD_Init(&USB_Device_dev, &USR_desc, &USBD_CDC_cb, &USR_cb);
      break;
#endif
    default:
    case USB_MASS_STORAGE_MODE:
      // initialize USB as MSC device
      USBD_Init(&USB_Device_dev, &USR_desc, &USBD_MSC_cb, &USR_cb);
      break;
  }
  usbDriverStarted = true;
}

void usbStop()
{
  usbDriverStarted = false;
  USBD_DeInit(&USB_Device_dev);
}

bool usbStarted()
{
  return usbDriverStarted;
}

#if !defined(BOOT)
/*
  Prepare and send new USB data packet

  The format of HID_Buffer is defined by
  USB endpoint description can be found in
  file usb_hid_joystick.c, variable HID_JOYSTICK_ReportDesc
*/
void usbJoystickUpdate()
{
  static uint8_t HID_Buffer[HID_IN_PACKET];

  // test to see if TX buffer is free
  if (USBD_HID_SendReport(&USB_Device_dev, 0, 0) == USBD_OK) {

    // 4 axes
    HID_Buffer[0] = uint8_t(adcValues[STICK1] >> 4) - 0x7f;
    HID_Buffer[1] = uint8_t(adcValues[STICK2] >> 4) - 0x7f;
    HID_Buffer[2] = uint8_t(adcValues[STICK3] >> 4) - 0x7f;
    HID_Buffer[3] = uint8_t(adcValues[STICK4] >> 4) - 0x7f;

    // 2 pots
    HID_Buffer[4] = uint8_t(adcValues[POT1] >> 4) - 0x7f;
    HID_Buffer[5] = uint8_t(adcValues[POT2] >> 4) - 0x7f;

    // 4 switches
    // up:  10
    // mid: 00
    // dn:  01
    HID_Buffer[6] = (~(uint8_t(adcValues[SW_A] >> 10) - 2) & 0x03)
		  | (~(uint8_t(adcValues[SW_B] >> 10) - 2) & 0x03) << 2
		  | (~(uint8_t(adcValues[SW_C] >> 10) - 2) & 0x03) << 4
		  | (~(uint8_t(adcValues[SW_D] >> 10) - 2) & 0x03) << 6;

    USBD_HID_SendReport(&USB_Device_dev, HID_Buffer, HID_IN_PACKET);
  }
}
#endif // BOOT
