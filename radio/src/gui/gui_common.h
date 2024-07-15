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

#ifndef _GUI_COMMON_H_
#define _GUI_COMMON_H_

#include "lcd.h"
#include "keys.h"

#if defined(ROTARY_ENCODER_NAVIGATION)
  #define CASE_EVT_ROTARY_LEFT         case EVT_ROTARY_LEFT:
  #define CASE_EVT_ROTARY_RIGHT        case EVT_ROTARY_RIGHT:
#else
  #define CASE_EVT_ROTARY_LEFT
  #define CASE_EVT_ROTARY_RIGHT
#endif

typedef bool (*IsValueAvailable)(int);

int circularIncDec(int current, int inc, int min, int max, IsValueAvailable isValueAvailable=NULL);
int getFirstAvailable(int min, int max, IsValueAvailable isValueAvailable);

bool isTrimModeAvailable(int mode);
bool isInputAvailable(int input);
bool isSourceAvailableInInputs(int source);
bool isThrottleSourceAvailable(int source);
bool isLogicalSwitchFunctionAvailable(int function);
bool isLogicalSwitchAvailable(int index);
bool isAssignableFunctionAvailable(int function);
bool isSourceAvailable(int source);
bool isSourceAvailableInGlobalFunctions(int source);
bool isSourceAvailableInCustomSwitches(int source);
bool isSourceAvailableInResetSpecialFunction(int index);
bool isSourceAvailableInGlobalResetSpecialFunction(int index);
bool isSwitchAvailableInLogicalSwitches(int swtch);
bool isSwitchAvailableInCustomFunctions(int swtch);
bool isSwitchAvailableInMixes(int swtch);
bool isSwitchAvailableInTimers(int swtch);
bool isR9MModeAvailable(int mode);
bool isModuleAvailable(int module);
bool isRfProtocolAvailable(int protocol);
bool isTelemetryProtocolAvailable(int protocol);
bool isTrainerModeAvailable(int mode);
bool isSubtypeAvailable(int i);

bool isSensorUnit(int sensor, uint8_t unit);
bool isCellsSensor(int sensor);
bool isGPSSensor(int sensor);
bool isAltSensor(int sensor);
bool isVoltsSensor(int sensor);
bool isCurrentSensor(int sensor);
bool isTelemetryFieldAvailable(int index);
bool isTelemetryFieldComparisonAvailable(int index);
bool isSensorAvailable(int sensor);

bool modelHasNotes();

#if defined(GUI)
#define IS_INSTANT_TRIM_ALLOWED()      (IS_MAIN_VIEW_DISPLAYED() || IS_TELEMETRY_VIEW_DISPLAYED() || IS_OTHER_VIEW_DISPLAYED())
#else
#define IS_INSTANT_TRIM_ALLOWED()      true
#endif

#if defined(FLIGHT_MODES)
void drawFlightMode(coord_t x, coord_t y, int8_t idx, LcdFlags att=0);
#endif

swsrc_t checkIncDecMovedSwitch(swsrc_t val);

#include "telemetry/telemetry_sensors.h"
void drawValueWithUnit(coord_t x, coord_t y, int val, uint8_t unit, LcdFlags flags);
void drawCurveRef(coord_t x, coord_t y, CurveRef & curve, LcdFlags flags=0);
void drawDate(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags flags=0);
void drawTelemScreenDate(coord_t x, coord_t y, source_t sensor, LcdFlags flags=0);
void drawGPSPosition(coord_t x, coord_t y, int32_t longitude, int32_t latitude, LcdFlags flags=0);
void drawGPSSensorValue(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags flags=0);
void drawSensorCustomValue(coord_t x, coord_t y, uint8_t sensor, int32_t value, LcdFlags flags=0);
void drawSourceCustomValue(coord_t x, coord_t y, source_t channel, int32_t val, LcdFlags flags=0);
void drawSourceValue(coord_t x, coord_t y, source_t channel, LcdFlags flags=0);

void drawCurve(coord_t offset=0);

void lcdDrawMMM(coord_t x, coord_t y, LcdFlags flags=0);

// model_setup Defines that are used in all uis in the same way
#define EXTERNAL_MODULE_CHANNELS_ROWS   IF_EXTERNAL_MODULE_ON((isModuleDSM2(EXTERNAL_MODULE) || isModuleCrossfire(EXTERNAL_MODULE) || isModuleSBUS(EXTERNAL_MODULE) || (isModuleMultimodule(EXTERNAL_MODULE) && g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) != MM_RF_PROTO_DSM2)) ? (uint8_t)0 : (uint8_t)1)


#define MULTIMODULE_STATUS_ROWS
#define MULTIMODULE_MODULE_ROWS
#define MULTIMODULE_HASFAILSAFE(x)      false
#define MULTIMODULE_SUBTYPE_ROWS(x)
#define MULTIMODULE_MODE_ROWS(x)        (uint8_t)0
#define MULTI_MAX_RX_NUM(x)             15
#define MULTIMODULE_OPTIONS_ROW         HIDDEN_ROW

#if defined(PCBI6X)
#define MAX_RX_NUM(x)                  (isModuleA7105(x) ? 15 : 63)
#else
#define MAX_RX_NUM(x)                  (isModuleDSM2(x) ? 20 : isModuleMultimodule(x) ? MULTI_MAX_RX_NUM(x) : 63)
#endif
#define IS_D8_RX(x)                    (g_model.moduleData[x].rfProtocol == RF_PROTO_D8)
#define IS_R9M_OR_XJTD16(x)            ((isModuleXJT(x) && g_model.moduleData[x].rfProtocol== RF_PROTO_X16) || isModuleR9M(x))

#define FAILSAFE_ROWS(x)               ((isModuleXJT(x) && HAS_RF_PROTOCOL_FAILSAFE(g_model.moduleData[x].rfProtocol)) || MULTIMODULE_HASFAILSAFE(x) || isModuleR9M(x) || isModuleA7105(x))  ? (g_model.moduleData[x].failsafeMode==FAILSAFE_CUSTOM ? (uint8_t)1 : (uint8_t)0) : HIDDEN_ROW

#define EXTERNAL_MODULE_OPTION_ROW     (isModuleR9M(EXTERNAL_MODULE) || isModuleSBUS(EXTERNAL_MODULE)  ? TITLE_ROW : MULTIMODULE_OPTIONS_ROW)

#define EXTERNAL_MODULE_POWER_ROW      (isModuleMultimodule(EXTERNAL_MODULE) || isModuleR9M(EXTERNAL_MODULE)) ? (uint8_t) 0 : HIDDEN_ROW

void editStickHardwareSettings(coord_t x, coord_t y, int idx, event_t event, LcdFlags flags);

#endif // _GUI_COMMON_H_
