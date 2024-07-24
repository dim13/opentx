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

CustomFunctionsContext modelFunctionsContext = { 0 };

CustomFunctionsContext globalFunctionsContext = { 0 };

#if defined(DEBUG)
/*
 * This is a test function for debugging purpose, you may insert there your code and compile with the option DEBUG=YES
 */
void testFunc()
{
}
#endif

bool isRepeatDelayElapsed(const CustomFunctionData * functions, CustomFunctionsContext & functionsContext, uint8_t index)
{
  const CustomFunctionData * cfn = &functions[index];
  tmr10ms_t tmr10ms = get_tmr10ms();
  uint8_t repeatParam = CFN_PLAY_REPEAT(cfn);
  if (!IS_SILENCE_PERIOD_ELAPSED() && repeatParam == CFN_PLAY_REPEAT_NOSTART) {
    functionsContext.lastFunctionTime[index] = tmr10ms;
  }
  if (!functionsContext.lastFunctionTime[index] || (repeatParam && repeatParam!=CFN_PLAY_REPEAT_NOSTART && (signed)(tmr10ms-functionsContext.lastFunctionTime[index])>=100*repeatParam)) {
    functionsContext.lastFunctionTime[index] = tmr10ms;
    return true;
  }
  else {
    return false;
  }
}

#define VOLUME_HYSTERESIS 10            // how much must a input value change to actually be considered for new volume setting
getvalue_t requiredSpeakerVolumeRawLast = 1024 + 1; //initial value must be outside normal range

void evalFunctions(const CustomFunctionData * functions, CustomFunctionsContext & functionsContext)
{
  MASK_FUNC_TYPE newActiveFunctions  = 0;
  MASK_CFN_TYPE  newActiveSwitches = 0;

  #define PLAY_INDEX   0

#if defined(OVERRIDE_CHANNEL_FUNCTION)
  for (uint8_t i=0; i<MAX_OUTPUT_CHANNELS; i++) {
    safetyCh[i] = OVERRIDE_CHANNEL_UNDEFINED;
  }
#endif

#if defined(GVARS)
  for (uint8_t i=0; i<NUM_TRIMS; i++) {
    trimGvar[i] = -1;
  }
#endif

  for (uint8_t i=0; i<MAX_SPECIAL_FUNCTIONS; i++) {
    const CustomFunctionData * cfn = &functions[i];
    swsrc_t swtch = CFN_SWITCH(cfn);
    if (swtch) {
      MASK_CFN_TYPE switch_mask = ((MASK_CFN_TYPE)1 << i);

      bool active = getSwitch(swtch, IS_PLAY_FUNC(CFN_FUNC(cfn)) ? GETSWITCH_MIDPOS_DELAY : 0);

      if (HAS_ENABLE_PARAM(CFN_FUNC(cfn))) {
        active &= (bool)CFN_ACTIVE(cfn);
      }

      if (active || IS_PLAY_BOTH_FUNC(CFN_FUNC(cfn))) {

        switch (CFN_FUNC(cfn)) {

#if defined(OVERRIDE_CHANNEL_FUNCTION)
          case FUNC_OVERRIDE_CHANNEL:
            safetyCh[CFN_CH_INDEX(cfn)] = CFN_PARAM(cfn);
            break;
#endif

          case FUNC_TRAINER:
          {
            uint8_t mask = 0x0f;
            if (CFN_CH_INDEX(cfn) > 0) {
              mask = (1<<(CFN_CH_INDEX(cfn)-1));
            }
            newActiveFunctions |= mask;
            break;
          }

          case FUNC_INSTANT_TRIM:
            newActiveFunctions |= (1 << FUNCTION_INSTANT_TRIM);
            if (!isFunctionActive(FUNCTION_INSTANT_TRIM)) {
              if (IS_INSTANT_TRIM_ALLOWED()) {
                instantTrim();
              }
            }
            break;

          case FUNC_RESET:
            switch (CFN_PARAM(cfn)) {
              case FUNC_RESET_TIMER1:
              case FUNC_RESET_TIMER2:
              case FUNC_RESET_TIMER3:
                timerReset(CFN_PARAM(cfn));
                break;
              case FUNC_RESET_FLIGHT:
              	if (!(functionsContext.activeSwitches & switch_mask)) {
                  mainRequestFlags |= (1 << REQUEST_FLIGHT_RESET);     // on systems with threads flightReset() must not be called from the mixers thread!
                }
                break;
#if defined(TELEMETRY_FRSKY)
              case FUNC_RESET_TELEMETRY:
                telemetryReset();
                break;
#endif
            }
            if (CFN_PARAM(cfn)>=FUNC_RESET_PARAM_FIRST_TELEM) {
              uint8_t item = CFN_PARAM(cfn)-FUNC_RESET_PARAM_FIRST_TELEM;
              if (item < MAX_TELEMETRY_SENSORS) {
                telemetryItems[item].clear();
              }
            }
            break;

          case FUNC_SET_TIMER:
            timerSet(CFN_TIMER_INDEX(cfn), CFN_PARAM(cfn));
            break;

          case FUNC_SET_FAILSAFE:
            setCustomFailsafe(CFN_PARAM(cfn));
            break;

#if defined(DANGEROUS_MODULE_FUNCTIONS)
          case FUNC_RANGECHECK:
          case FUNC_BIND:
          {
            unsigned int moduleIndex = CFN_PARAM(cfn);
            if (moduleIndex < NUM_MODULES) {
              moduleState[moduleIndex].mode = 1 + CFN_FUNC(cfn) - FUNC_RANGECHECK;
            }
            break;
          }
#endif  

#if defined(GVARS)
          case FUNC_ADJUST_GVAR:
            if (CFN_GVAR_MODE(cfn) == FUNC_ADJUST_GVAR_CONSTANT) {
              SET_GVAR(CFN_GVAR_INDEX(cfn), CFN_PARAM(cfn), mixerCurrentFlightMode);
            }
            else if (CFN_GVAR_MODE(cfn) == FUNC_ADJUST_GVAR_GVAR) {
              SET_GVAR(CFN_GVAR_INDEX(cfn), GVAR_VALUE(CFN_PARAM(cfn), getGVarFlightMode(mixerCurrentFlightMode, CFN_PARAM(cfn))), mixerCurrentFlightMode);
            }
            else if (CFN_GVAR_MODE(cfn) == FUNC_ADJUST_GVAR_INCDEC) {
              if (!(functionsContext.activeSwitches & switch_mask)) {
                SET_GVAR(CFN_GVAR_INDEX(cfn), limit<int16_t>(MODEL_GVAR_MIN(CFN_GVAR_INDEX(cfn)), GVAR_VALUE(CFN_GVAR_INDEX(cfn), getGVarFlightMode(mixerCurrentFlightMode, CFN_GVAR_INDEX(cfn))) + CFN_PARAM(cfn), MODEL_GVAR_MAX(CFN_GVAR_INDEX(cfn))), mixerCurrentFlightMode);
              }
            }
            else if (CFN_PARAM(cfn) >= MIXSRC_FIRST_TRIM && CFN_PARAM(cfn) <= MIXSRC_LAST_TRIM) {
              trimGvar[CFN_PARAM(cfn)-MIXSRC_FIRST_TRIM] = CFN_GVAR_INDEX(cfn);
            }
            else {
              SET_GVAR(CFN_GVAR_INDEX(cfn), limit<int16_t>(MODEL_GVAR_MIN(CFN_GVAR_INDEX(cfn)), calcRESXto100(getValue(CFN_PARAM(cfn))), MODEL_GVAR_MAX(CFN_GVAR_INDEX(cfn))), mixerCurrentFlightMode);
            }
            break;
#endif
#if !defined(PCBI6X)
          case FUNC_VOLUME:
          {
            getvalue_t raw = getValue(CFN_PARAM(cfn));
            // only set volume if input changed more than hysteresis
            if (abs(requiredSpeakerVolumeRawLast - raw) > VOLUME_HYSTERESIS) {
              requiredSpeakerVolumeRawLast = raw;
            }
            requiredSpeakerVolume = ((1024 + requiredSpeakerVolumeRawLast) * VOLUME_LEVEL_MAX) / 2048;
            break;
          }
#endif
#if defined(SDCARD) || defined(PCBI6X)
          case FUNC_PLAY_SOUND:
          {
            if (isRepeatDelayElapsed(functions, functionsContext, i)) {
              if (!IS_PLAYING(PLAY_INDEX)) {
                if (CFN_FUNC(cfn) == FUNC_PLAY_SOUND) {
                  #if defined(PCBI6X)
                  if (true) {
                  #else
                  if (audioQueue.isEmpty()) {
                  #endif
                    AUDIO_PLAY(AU_SPECIAL_SOUND_FIRST + CFN_PARAM(cfn));
                  }
                }
                else {
                }
              }
            }
            break;
          }
#if !defined(PCBI6X)
          case FUNC_BACKGND_MUSIC:
            if (!(newActiveFunctions & (1 << FUNCTION_BACKGND_MUSIC))) {
              newActiveFunctions |= (1 << FUNCTION_BACKGND_MUSIC);
              if (!IS_PLAYING(PLAY_INDEX)) {
                playCustomFunctionFile(cfn, PLAY_INDEX);
              }
            }
            break;

          case FUNC_BACKGND_MUSIC_PAUSE:
            newActiveFunctions |= (1 << FUNCTION_BACKGND_MUSIC_PAUSE);
            break;
#endif
#else
          case FUNC_PLAY_SOUND:
          case FUNC_PLAY_TRACK:
          case FUNC_PLAY_VALUE:
          {
            tmr10ms_t tmr10ms = get_tmr10ms();
            uint8_t repeatParam = CFN_PLAY_REPEAT(cfn);
            if (!functionsContext.lastFunctionTime[i] || (active!=(bool)(functionsContext.activeSwitches&switch_mask)) || (repeatParam && (signed)(tmr10ms-functionsContext.lastFunctionTime[i])>=1000*repeatParam)) {
              functionsContext.lastFunctionTime[i] = tmr10ms;
              uint8_t param = CFN_PARAM(cfn);
              if (CFN_FUNC(cfn) == FUNC_PLAY_SOUND) {
                AUDIO_PLAY(AU_SPECIAL_SOUND_FIRST+param);
              }
              else if (CFN_FUNC(cfn) == FUNC_PLAY_VALUE) {
                PLAY_VALUE(param, PLAY_INDEX);
              }
              else {
#if defined(GVARS)
                if (CFN_FUNC(cfn) == FUNC_PLAY_TRACK && param > 250)
                  param = GVAR_VALUE(param-251, getGVarFlightMode(mixerCurrentFlightMode, param-251));
#endif
                PUSH_CUSTOM_PROMPT(active ? param : param+1, PLAY_INDEX);
              }
            }
            if (!active) {
              // PLAY_BOTH would change activeFnSwitches otherwise
              switch_mask = 0;
            }
            break;
          }
#endif // PCBI6X || SDCARD

#if defined(TELEMETRY_FRSKY) && defined(VARIO)
          case FUNC_VARIO:
            newActiveFunctions |= (1u << FUNCTION_VARIO);
            break;
#endif


#if defined(SDCARD)
          case FUNC_LOGS:
            if (CFN_PARAM(cfn)) {
              newActiveFunctions |= (1u << FUNCTION_LOGS);
              logDelay = CFN_PARAM(cfn);
            }
            break;
#endif

          case FUNC_BACKLIGHT:
          {
            newActiveFunctions |= (1u << FUNCTION_BACKLIGHT);
            if (!CFN_PARAM(cfn)) {  // When no source is set, backlight works like original backlight and turn on regardless of backlight settings
              requiredBacklightBright = BACKLIGHT_FORCED_ON;
              break;
            }

            getvalue_t raw = getValue(CFN_PARAM(cfn));
            requiredBacklightBright = (1024 - raw) * 100 / 2048;
            break;
          }

#if defined(PCBTARANIS)
          case FUNC_SCREENSHOT:
            if (!(functionsContext.activeSwitches & switch_mask)) {
              mainRequestFlags |= (1 << REQUEST_SCREENSHOT);
            }
            break;
#endif

#if defined(DEBUG)
          case FUNC_TEST:
            testFunc();
            break;
#endif
        }

        newActiveSwitches |= switch_mask;
      }
      else {
        functionsContext.lastFunctionTime[i] = 0;
#if defined(DANGEROUS_MODULE_FUNCTIONS)
        if (functionsContext.activeSwitches & switch_mask) {
          switch (CFN_FUNC(cfn)) {
            case FUNC_RANGECHECK:
            case FUNC_BIND:
            {
              unsigned int moduleIndex = CFN_PARAM(cfn);
              if (moduleIndex < NUM_MODULES) {
                moduleState[moduleIndex].mode = 0;
              }
              break;
            }
          }
        }
#endif
      }
    }
  }

  functionsContext.activeSwitches   = newActiveSwitches;
  functionsContext.activeFunctions  = newActiveFunctions;
}

