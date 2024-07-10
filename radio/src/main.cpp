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

#include "opentx.h"

uint8_t currentSpeakerVolume = 255;
uint8_t requiredSpeakerVolume = 255;
uint8_t currentBacklightBright = 0;
uint8_t requiredBacklightBright = 0;
uint8_t mainRequestFlags = 0;

#if defined(PCBI6X_ELRS)
extern void elrsStop();
#endif

#if defined(STM32)
void onUSBConnectMenu(const char *result)
{
#if !defined(PCBI6X) || defined(PCBI6X_USB_MSD)
  if (result == STR_USB_MASS_STORAGE) {
    setSelectedUsbMode(USB_MASS_STORAGE_MODE);
  }
  else
#endif
  if (result == STR_USB_JOYSTICK) {
    setSelectedUsbMode(USB_JOYSTICK_MODE);
  }
#if !defined(PCBI6X)
  else if (result == STR_USB_SERIAL) {
    setSelectedUsbMode(USB_SERIAL_MODE);
  }
#endif
}
#endif

void handleUsbConnection()
{
#if defined(STM32) && !defined(SIMU)
  if (!usbStarted() && usbPlugged()) {
    if (getSelectedUsbMode() == USB_UNSELECTED_MODE) {
      if (g_eeGeneral.USBMode == USB_UNSELECTED_MODE && popupMenuItemsCount == 0) {
        POPUP_MENU_ADD_ITEM(STR_USB_JOYSTICK);
  #if !defined(PCBI6X) || defined(PCBI6X_USB_MSD)
        POPUP_MENU_ADD_ITEM(STR_USB_MASS_STORAGE);
  #endif
  #if defined(DEBUG) && !defined(PCBI6X)
        POPUP_MENU_ADD_ITEM(STR_USB_SERIAL);
  #endif
        POPUP_MENU_START(onUSBConnectMenu);
      }
      else {
        setSelectedUsbMode(g_eeGeneral.USBMode);
      }
    }
    else {
      #if !defined(PCBI6X) || defined(PCBI6X_USB_MSD)
      if (getSelectedUsbMode() == USB_MASS_STORAGE_MODE) {
        #if defined(PCBI6X_ELRS)
        elrsStop();
        #endif
        opentxClose(false);
        usbPluggedIn();
      }
      #endif
      usbStart();
    }
  }

  if (usbStarted() && !usbPlugged()) {
    usbStop();
    #if !defined(PCBI6X) || defined(PCBI6X_USB_MSD)
    if (getSelectedUsbMode() == USB_MASS_STORAGE_MODE) {
      opentxResume();
    }
    #endif
    setSelectedUsbMode(USB_UNSELECTED_MODE);
  }
#endif // defined(STM32) && !defined(SIMU)
}

void checkSpeakerVolume()
{
  if (currentSpeakerVolume != requiredSpeakerVolume) {
    currentSpeakerVolume = requiredSpeakerVolume;
#if !defined(SOFTWARE_VOLUME)
    setScaledVolume(currentSpeakerVolume);
#endif
  }
}

#if defined(EEPROM)
void checkEeprom()
{
  if (eepromIsWriting())
    eepromWriteProcess();
  else if (TIME_TO_WRITE())
    storageCheck(false);
}
#else
void checkEeprom()
{
#if defined(RAMBACKUP)
  if (TIME_TO_RAMBACKUP()) {
    rambackupWrite();
    rambackupDirtyMsk = 0;
  }
#endif
  if (TIME_TO_WRITE()) {
    storageCheck(false);
  }
}
#endif

#define BAT_AVG_SAMPLES       8

void checkBatteryAlarms()
{
  // TRACE("checkBatteryAlarms()");
  if (IS_TXBATT_WARNING()) {
    AUDIO_TX_BATTERY_LOW();
    // TRACE("checkBatteryAlarms(): battery low");
  }
#if defined(PCBSKY9X)
  else if (g_eeGeneral.temperatureWarn && getTemperature() >= g_eeGeneral.temperatureWarn) {
    AUDIO_TX_TEMP_HIGH();
  }
  else if (g_eeGeneral.mAhWarn && (g_eeGeneral.mAhUsed + Current_used * (488 + g_eeGeneral.txCurrentCalibration)/8192/36) / 500 >= g_eeGeneral.mAhWarn) { // TODO move calculation into board file
    AUDIO_TX_MAH_HIGH();
  }
#endif
}

void checkBattery()
{
  static uint32_t batSum;
  static uint8_t sampleCount;
  // filter battery voltage by averaging it
  if (g_vbat100mV == 0) {
    g_vbat100mV = (getBatteryVoltage() + 5) / 10;
    batSum = 0;
    sampleCount = 0;
  }
  else {
    batSum += getBatteryVoltage();
    //TRACE("checkBattery(): sampled = %d", getBatteryVoltage());
    if (++sampleCount >= BAT_AVG_SAMPLES) {
      g_vbat100mV = (batSum + BAT_AVG_SAMPLES * 5 ) / (BAT_AVG_SAMPLES * 10);
      batSum = 0;
      sampleCount = 0;
      //TRACE("checkBattery(): g_vbat100mV = %d", g_vbat100mV);
    }
  }
}

void periodicTick_1s()
{
  checkBattery();
}

void periodicTick_10s()
{
  checkBatteryAlarms();
}

void periodicTick()
{
  static uint8_t count10s;
  static uint32_t lastTime;
  if ( (get_tmr10ms() - lastTime) >= 100 ) {
    lastTime += 100;
    periodicTick_1s();
    if (++count10s >= 10) {
      count10s = 0;
      periodicTick_10s();
    }
  }
}

#if defined(GUI)
void handleGui(event_t event) {
  // if Lua standalone, run it and don't clear the screen (Lua will do it)
  // else if Lua telemetry view, run it and don't clear the screen
  // else clear scren and show normal menus
#if defined(PCBI6X) && defined(RADIO_TOOLS)
  if (globalData.cToolRunning == 1) {
    // standalone c script is active
    menuHandlers[menuLevel](event);
  }
  else
#endif
  {
    lcdClear();
    menuHandlers[menuLevel](event);
    drawStatusLine();
  }
}

bool inPopupMenu = false;

void guiMain(event_t evt)
{
  // wait for LCD DMA to finish before continuing, because code from this point
  // is allowed to change the contents of LCD buffer
  //
  // WARNING: make sure no code above this line does any change to the LCD display buffer!
  //
  lcdRefreshWait();

  if (menuEvent) {
    // we have a popupMenuActive entry or exit event
    menuVerticalPosition = (menuEvent == EVT_ENTRY_UP) ? menuVerticalPositions[menuLevel] : 0;
    menuHorizontalPosition = 0;
    evt = menuEvent;
    menuEvent = 0;
  }

  if (warningText) {
    // show warning on top of the normal menus
    handleGui(0); // suppress events, they are handled by the warning
    DISPLAY_WARNING(evt);
  }
  else if (popupMenuItemsCount > 0) {
    // popup menu is active display it on top of normal menus
    handleGui(0); // suppress events, they are handled by the popup
    if (!inPopupMenu) {
      TRACE("Popup Menu started");
      inPopupMenu = true;
    }
    const char * result = runPopupMenu(evt);
    if (result) {
      TRACE("popupMenuHandler(%s)", result);
      popupMenuHandler(result);
    }
  }
  else {
    // normal menus
    if (inPopupMenu) {
      TRACE("Popup Menu ended");
      inPopupMenu = false;
    }
    handleGui(evt);
  }

  lcdRefresh();
}
#endif

void perMain()
{
  DEBUG_TIMER_START(debugTimerPerMain1);

#if defined(PCBSKY9X) && !defined(REVA)
  calcConsumption();
#endif
#if !defined(PCBI6X)
  checkSpeakerVolume();
#endif

  if (!usbPlugged()) {
    checkEeprom();
    #if defined(SDCARD)
    logsWrite();
    #endif
  }

  handleUsbConnection();

  checkTrainerSettings();
  periodicTick();
  DEBUG_TIMER_STOP(debugTimerPerMain1);

  if (mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)) {
    TRACE("Executing requested Flight Reset");
    flightReset();
    mainRequestFlags &= ~(1 << REQUEST_FLIGHT_RESET);
  }

  checkBacklight();

  event_t evt = getEvent(false);

#if defined(RAMBACKUP)
  if (globalData.unexpectedShutdown) {
    drawFatalErrorScreen(STR_EMERGENCY_MODE);
    return;
  }
#endif

#if defined(STM32) && defined(SDCARD)
  if (!usbPlugged() && SD_CARD_PRESENT() && !sdMounted()) {
    sdMount();
  }
#endif

#if !defined(EEPROM)
  // In case the SD card is removed during the session
  if (!usbPlugged() && !SD_CARD_PRESENT() && !globalData.unexpectedShutdown) {
    drawFatalErrorScreen(STR_NO_SDCARD);
    return;
  }
#endif

#if defined(STM32)
  if (usbPlugged() && getSelectedUsbMode() == USB_MASS_STORAGE_MODE) {
    // disable access to menus
    lcdClear();
    menuMainView(0);
    lcdRefresh();
    return;
  }
#endif

#if defined(GUI)
  DEBUG_TIMER_START(debugTimerGuiMain);
  guiMain(evt);
  DEBUG_TIMER_STOP(debugTimerGuiMain);
#endif

#if defined(INTERNAL_GPS)
  gpsWakeup();
#endif
}
