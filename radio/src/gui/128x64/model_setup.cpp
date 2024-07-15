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

uint8_t g_moduleIdx;
void menuModelFailsafe(event_t event);

uint8_t getSwitchWarningsCount()
{
  int count = 0;
  for (int i=0; i<NUM_SWITCHES; ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      ++count;
    }
  }
  return count;
}

enum MenuModelSetupItems {
  ITEM_MODEL_NAME,
  ITEM_MODEL_TIMER1,
  ITEM_MODEL_TIMER1_NAME,
  ITEM_MODEL_TIMER1_PERSISTENT,
  ITEM_MODEL_TIMER1_MINUTE_BEEP,
  ITEM_MODEL_TIMER1_COUNTDOWN_BEEP,
  ITEM_MODEL_TIMER2,
  ITEM_MODEL_TIMER2_NAME,
  ITEM_MODEL_TIMER2_PERSISTENT,
  ITEM_MODEL_TIMER2_MINUTE_BEEP,
  ITEM_MODEL_TIMER2_COUNTDOWN_BEEP,
  ITEM_MODEL_TIMER3,
  ITEM_MODEL_TIMER3_NAME,
  ITEM_MODEL_TIMER3_PERSISTENT,
  ITEM_MODEL_TIMER3_MINUTE_BEEP,
  ITEM_MODEL_TIMER3_COUNTDOWN_BEEP,
  ITEM_MODEL_EXTENDED_LIMITS,
  ITEM_MODEL_EXTENDED_TRIMS,
  ITEM_MODEL_DISPLAY_TRIMS,
  ITEM_MODEL_TRIM_INC,
  ITEM_MODEL_THROTTLE_REVERSED,
  ITEM_MODEL_THROTTLE_TRACE,
  ITEM_MODEL_THROTTLE_TRIM,
  ITEM_MODEL_PREFLIGHT_LABEL,
#if !defined(PCBI6X)
  ITEM_MODEL_CHECKLIST_DISPLAY,
#endif
  ITEM_MODEL_THROTTLE_WARNING,
  ITEM_MODEL_SWITCHES_WARNING,
  ITEM_MODEL_BEEP_CENTER,
  ITEM_MODEL_USE_GLOBAL_FUNCTIONS,
  ITEM_MODEL_INTERNAL_MODULE_LABEL,
  ITEM_MODEL_INTERNAL_MODULE_MODE,
  ITEM_MODEL_INTERNAL_MODULE_SUBTYPE,
  ITEM_MODEL_INTERNAL_MODULE_SERVOFREQ,
  ITEM_MODEL_INTERNAL_MODULE_BIND,
  ITEM_MODEL_INTERNAL_MODULE_FAILSAFE,
  ITEM_MODEL_EXTERNAL_MODULE_LABEL,
  ITEM_MODEL_EXTERNAL_MODULE_MODE,
  ITEM_MODEL_EXTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_EXTERNAL_MODULE_BIND,
  ITEM_MODEL_EXTERNAL_MODULE_OPTIONS,
  ITEM_MODEL_EXTERNAL_MODULE_POWER,
  ITEM_MODEL_EXTERNAL_MODULE_FAILSAFE,
  ITEM_MODEL_TRAINER_LABEL,
  ITEM_MODEL_TRAINER_MODE,
  ITEM_MODEL_SETUP_MAX
};

#define MODEL_SETUP_2ND_COLUMN           (LCD_W-11*FW)
#define MODEL_SETUP_BIND_OFS             2*FW+1
#define MODEL_SETUP_RANGE_OFS            4*FW+3
#define MODEL_SETUP_SET_FAILSAFE_OFS     7*FW-2

#define CURRENT_MODULE_EDITED(k)       (k>=ITEM_MODEL_TRAINER_LABEL ? TRAINER_MODULE : (k>=ITEM_MODEL_EXTERNAL_MODULE_LABEL ? EXTERNAL_MODULE : INTERNAL_MODULE))

#define SW_WARN_ROWS                    uint8_t(NAVIGATION_LINE_BY_LINE|(getSwitchWarningsCount()-1)), uint8_t(getSwitchWarningsCount() > 5 ? TITLE_ROW : HIDDEN_ROW)
#if !defined(TARANIS_INTERNAL_PPM) && !defined(PCBI6X)
  #define INTERNAL_MODULE_MODE_ROWS       0 // (OFF / RF protocols)
#else
  #define INTERNAL_MODULE_MODE_ROWS       (isModuleXJT(INTERNAL_MODULE)||isModuleA7105(INTERNAL_MODULE) ? (uint8_t)1 : (uint8_t)0) // Module type + RF protocols
#endif
  #define IF_INTERNAL_MODULE_ON(x)       (IS_INTERNAL_MODULE_ENABLED()? (uint8_t)(x) : HIDDEN_ROW)
  #define IF_EXTERNAL_MODULE_ON(x)       (IS_EXTERNAL_MODULE_ENABLED()? (uint8_t)(x) : HIDDEN_ROW)
  #define INTERNAL_MODULE_CHANNELS_ROWS  IF_INTERNAL_MODULE_ON(1)
  #define EXTERNAL_MODULE_BIND_ROWS()    ((isModuleXJT(EXTERNAL_MODULE) && IS_D8_RX(EXTERNAL_MODULE)) || isModuleSBUS(EXTERNAL_MODULE)) ? (uint8_t)1 : (isModulePPM(EXTERNAL_MODULE) || isModulePXX(EXTERNAL_MODULE) || isModuleDSM2(EXTERNAL_MODULE) || isModuleMultimodule(EXTERNAL_MODULE)) ? (uint8_t)2 : (isModuleCrossfire(EXTERNAL_MODULE)) ? (uint8_t)0 : HIDDEN_ROW

#define OUTPUT_TYPE_ROWS()
#define PORT_CHANNELS_ROWS(x)          (x==EXTERNAL_MODULE ? EXTERNAL_MODULE_CHANNELS_ROWS : 0)

  #define EXTERNAL_MODULE_MODE_ROWS      (isModulePXX(EXTERNAL_MODULE) || isModuleDSM2(EXTERNAL_MODULE) || isModuleMultimodule(EXTERNAL_MODULE) || isModuleCrossfire(EXTERNAL_MODULE)) ? (uint8_t)1 : (uint8_t)0

  #define CURSOR_ON_CELL                 (true)
  #define MODEL_SETUP_MAX_LINES          (HEADER_LINE+ITEM_MODEL_SETUP_MAX)
  #define POT_WARN_ITEMS()               ((g_model.potsWarnMode) ? (uint8_t)(NUM_POTS+NUM_SLIDERS) : (uint8_t)0)
  #define TIMER_ROWS(x)                  2, 0, 0, 0, g_model.timers[x].countdownBeep != COUNTDOWN_SILENT ? (uint8_t)1 : (uint8_t)0
  #define TIMERS_ROWS                    TIMER_ROWS(0), TIMER_ROWS(1), TIMER_ROWS(2)
  #define EXTRA_MODULE_ROWS

#define TRAINER_ROWS                     LABEL(Trainer), 0, HIDDEN_ROW, HIDDEN_ROW

void editTimerCountdown(int timerIdx, coord_t y, LcdFlags attr, event_t event)
{
  TimerData & timer = g_model.timers[timerIdx];
  lcdDrawTextAlignedLeft(y, STR_BEEPCOUNTDOWN);
  lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VBEEPCOUNTDOWN, timer.countdownBeep, (menuHorizontalPosition == 0 ? attr : 0));
  if (timer.countdownBeep != COUNTDOWN_SILENT) {
    lcdDrawNumber(MODEL_SETUP_2ND_COLUMN + 6 * FW, y, TIMER_COUNTDOWN_START(timerIdx), (menuHorizontalPosition == 1 ? attr : 0) | LEFT);
    lcdDrawChar(lcdLastRightPos, y, 's');
  }
  if (attr && s_editMode > 0) {
    switch (menuHorizontalPosition) {
      case 0:
        CHECK_INCDEC_MODELVAR(event, timer.countdownBeep, COUNTDOWN_SILENT, COUNTDOWN_COUNT - 1);
        break;
      case 1:
        timer.countdownStart = -checkIncDecModel(event, -timer.countdownStart, -1, +2);
        break;
    }
  }
}

void onBindMenu(const char * result)
{
  uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition);

  if (result == STR_BINDING_1_8_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = false;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  }
  else if (result == STR_BINDING_1_8_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  }
  else if (result == STR_BINDING_9_16_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = false;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = true;
  }
  else if (result == STR_BINDING_9_16_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = true;
  }
  else {
    return;
  }

  moduleState[moduleIdx].mode = MODULE_MODE_BIND;
}


void menuModelSetup(event_t event)
{
  int8_t old_editMode = s_editMode;
  int8_t old_posHorz = menuHorizontalPosition;

// -2 Don't show
// -1 Show label only
//  0 Show one input
//  1 Show two inputs in the same row
//  ... 
  MENU_TAB({ 
    HEADER_LINE_COLUMNS 
    0, 
    TIMERS_ROWS,
    0, // Extended limits
    1, // Extended trims
    0, // Show trims
    0, // Trims step
    0, // Throttle reverse
    0, // Throttle trace source
    0, // Throttle trim
    // 0, // Throttle trim switch
    LABEL(PreflightCheck), 
//    0, // Checklist
    0, 
    NUM_SWITCHES-1, 
    NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_ROTARY_ENCODERS-1, 
    0,
    LABEL(InternalModule),
    INTERNAL_MODULE_MODE_ROWS,
#if !defined(PCBI6X)
    INTERNAL_MODULE_CHANNELS_ROWS,
#endif    
    IF_INTERNAL_MODULE_ON(1), // Subtype
    IF_INTERNAL_MODULE_ON(1), // Servo Freq
    IF_INTERNAL_MODULE_ON(HAS_RF_PROTOCOL_MODELINDEX(g_model.moduleData[INTERNAL_MODULE].rfProtocol) ? (uint8_t)2 : (uint8_t)1),
    IF_INTERNAL_MODULE_ON(FAILSAFE_ROWS(INTERNAL_MODULE)),
    LABEL(ExternalModule),
    EXTERNAL_MODULE_MODE_ROWS,
    MULTIMODULE_SUBTYPE_ROWS(EXTERNAL_MODULE)
    MULTIMODULE_STATUS_ROWS
    EXTERNAL_MODULE_CHANNELS_ROWS,
    EXTERNAL_MODULE_BIND_ROWS(),
    OUTPUT_TYPE_ROWS()
    EXTERNAL_MODULE_OPTION_ROW,
    MULTIMODULE_MODULE_ROWS
    EXTERNAL_MODULE_POWER_ROW,
    EXTRA_MODULE_ROWS
    FAILSAFE_ROWS(EXTERNAL_MODULE),
    TRAINER_ROWS });

  MENU_CHECK(menuTabModel, MENU_MODEL_SETUP, HEADER_LINE+MODEL_SETUP_MAX_LINES);

#if (defined(DSM2) || defined(PXX))
  if (menuEvent) {
    moduleState[0].mode = 0;
#if NUM_MODULES > 1
    moduleState[1].mode = 0;
#endif
  }
#endif

  TITLE(STR_MENUSETUP);

  if (event == EVT_ENTRY) {
    reusableBuffer.modelsetup.r9mPower = g_model.moduleData[EXTERNAL_MODULE].pxx.power;
  }

  uint8_t sub = menuVerticalPosition - HEADER_LINE;
  int8_t editMode = s_editMode;

  for (uint8_t i=0; i<NUM_BODY_LINES; ++i) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i + menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j+HEADER_LINE] == HIDDEN_ROW) {
        if (++k >= (int)DIM(mstate_tab)) {
          return;
        }
      }
    }

    LcdFlags blink = ((editMode > 0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);
    //TRACE("k: %d max: %d", k, ITEM_MODEL_SETUP_MAX);
    switch (k) {
      case ITEM_MODEL_NAME:
        editSingleName(MODEL_SETUP_2ND_COLUMN, y, STR_MODELNAME, g_model.header.name, sizeof(g_model.header.name), event, attr);
        memcpy(modelHeaders[g_eeGeneral.currModel].name, g_model.header.name, sizeof(g_model.header.name));
        break;
      case ITEM_MODEL_TIMER1:
      case ITEM_MODEL_TIMER2:
      case ITEM_MODEL_TIMER3:
      {
        unsigned int timerIdx = (k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0));
        TimerData * timer = &g_model.timers[timerIdx];
        drawStringWithIndex(0*FW, y, STR_TIMER, timerIdx+1);
        drawTimerMode(MODEL_SETUP_2ND_COLUMN, y, timer->mode, menuHorizontalPosition==0 ? attr : 0);
        drawTimer(MODEL_SETUP_2ND_COLUMN+5*FW-2+5*FWNUM+1, y, timer->start, RIGHT | (menuHorizontalPosition==1 ? attr : 0), menuHorizontalPosition==2 ? attr : 0);
        if (attr && (editMode > 0 || p1valdiff)) {
          div_t qr = div(timer->start, 60);
          switch (menuHorizontalPosition) {
            case 0:
            {
              int8_t timerMode = timer->mode;
              if (timerMode < 0) timerMode -= TMRMODE_COUNT-1;
              CHECK_INCDEC_MODELVAR_CHECK(event, timerMode, -TMRMODE_COUNT-SWSRC_LAST+1, TMRMODE_COUNT+SWSRC_LAST-1, isSwitchAvailableInTimers);
              if (timerMode < 0) timerMode += TMRMODE_COUNT-1;
              timer->mode = timerMode;
#if defined(AUTOSWITCH)
              if (s_editMode > 0) {
                int8_t val = timer->mode - (TMRMODE_COUNT-1);
                int8_t switchVal = checkIncDecMovedSwitch(val);
                if (val != switchVal) {
                  timer->mode = switchVal + (TMRMODE_COUNT-1);
                  storageDirty(EE_MODEL);
                }
              }
#endif
              break;
            }
            case 1:
              CHECK_INCDEC_MODELVAR_ZERO(event, qr.quot, 539); // 8:59
              timer->start = qr.rem + qr.quot*60;
              break;
            case 2:
              qr.rem -= checkIncDecModel(event, qr.rem+2, 1, 62)-2;
              timer->start -= qr.rem;
              if ((int16_t)timer->start < 0) timer->start=0;
              if ((int16_t)timer->start > 5999) timer->start=32399; // 8:59:59
              break;
          }
        }
        break;
      }

      case ITEM_MODEL_TIMER1_NAME:
      case ITEM_MODEL_TIMER2_NAME:
      case ITEM_MODEL_TIMER3_NAME:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0)];
        editSingleName(MODEL_SETUP_2ND_COLUMN, y, STR_TIMER_NAME, timer->name, sizeof(timer->name), event, attr);
        break;
      }

      case ITEM_MODEL_TIMER1_MINUTE_BEEP:
      case ITEM_MODEL_TIMER2_MINUTE_BEEP:
      case ITEM_MODEL_TIMER3_MINUTE_BEEP:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0)];
        timer->minuteBeep = editCheckBox(timer->minuteBeep, MODEL_SETUP_2ND_COLUMN, y, STR_MINUTEBEEP, attr, event);
        break;
      }

      case ITEM_MODEL_TIMER1_COUNTDOWN_BEEP:
      case ITEM_MODEL_TIMER2_COUNTDOWN_BEEP:
      case ITEM_MODEL_TIMER3_COUNTDOWN_BEEP:
      {
        editTimerCountdown(k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0), y, attr, event);
        break;
      }

      case ITEM_MODEL_TIMER1_PERSISTENT:
      case ITEM_MODEL_TIMER2_PERSISTENT:
      case ITEM_MODEL_TIMER3_PERSISTENT:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0)];
        timer->persistent = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_PERSISTENT, STR_VPERSISTENT, timer->persistent, 0, 2, attr, event);
        break;
      }

      case ITEM_MODEL_EXTENDED_LIMITS:
        ON_OFF_MENU_ITEM(g_model.extendedLimits, MODEL_SETUP_2ND_COLUMN, y, STR_ELIMITS, attr, event);
        break;

      case ITEM_MODEL_EXTENDED_TRIMS:
        ON_OFF_MENU_ITEM(g_model.extendedTrims, MODEL_SETUP_2ND_COLUMN, y, STR_ETRIMS, menuHorizontalPosition<=0 ? attr : 0, event==EVT_KEY_BREAK(KEY_ENTER) ? event : 0);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+4*FW, y, STR_RESET_BTN, (menuHorizontalPosition>0  && !NO_HIGHLIGHT()) ? attr : 0);
        if (attr && menuHorizontalPosition>0) {
          s_editMode = 0;
          if (event==EVT_KEY_LONG(KEY_ENTER)) {
            START_NO_HIGHLIGHT();
            for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
              memclear(&g_model.flightModeData[i], TRIMS_ARRAY_SIZE);
            }
            storageDirty(EE_MODEL);
            AUDIO_WARNING1();
          }
        }
        break;

      case ITEM_MODEL_DISPLAY_TRIMS:
        g_model.displayTrims = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_DISPLAY_TRIMS, STR_VDISPLAYTRIMS, g_model.displayTrims, 0, 2, attr, event);
        break;

      case ITEM_MODEL_TRIM_INC:
        g_model.trimInc = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_TRIMINC, STR_VTRIMINC, g_model.trimInc, -2, 2, attr, event);
        break;

      case ITEM_MODEL_THROTTLE_REVERSED:
        ON_OFF_MENU_ITEM(g_model.throttleReversed, MODEL_SETUP_2ND_COLUMN, y, STR_THROTTLEREVERSE, attr, event ) ;
        break;

      case ITEM_MODEL_THROTTLE_TRACE:
      {
        lcdDrawTextAlignedLeft(y, STR_TTRACE);
        if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, g_model.thrTraceSrc, NUM_POTS+NUM_SLIDERS+MAX_OUTPUT_CHANNELS);
        uint8_t idx = g_model.thrTraceSrc + MIXSRC_Thr;
        if (idx > MIXSRC_Thr)
          idx += 1;
        if (idx >= MIXSRC_FIRST_POT+NUM_POTS+NUM_SLIDERS)
          idx += MIXSRC_CH1 - MIXSRC_FIRST_POT - NUM_POTS - NUM_SLIDERS;
        drawSource(MODEL_SETUP_2ND_COLUMN, y, idx, attr);
        break;
      }

      case ITEM_MODEL_THROTTLE_TRIM:
        ON_OFF_MENU_ITEM(g_model.thrTrim, MODEL_SETUP_2ND_COLUMN, y, STR_TTRIM, attr, event);
        break;

      case ITEM_MODEL_PREFLIGHT_LABEL:
        lcdDrawTextAlignedLeft(y, STR_PREFLIGHT);
        break;
#if !defined(PCBI6X)
      case ITEM_MODEL_CHECKLIST_DISPLAY:
        ON_OFF_MENU_ITEM(g_model.displayChecklist, MODEL_SETUP_2ND_COLUMN, y, STR_CHECKLIST, attr, event);
        break;
#endif
      case ITEM_MODEL_THROTTLE_WARNING:
        g_model.disableThrottleWarning = !editCheckBox(!g_model.disableThrottleWarning, MODEL_SETUP_2ND_COLUMN, y, STR_THROTTLEWARNING, attr, event);
        break;

      case ITEM_MODEL_SWITCHES_WARNING:
#if defined(PCBTARANIS) || defined(PCBI6X)
        {
          #define FIRSTSW_STR   STR_VSRCRAW+(MIXSRC_FIRST_SWITCH-MIXSRC_Rud+1)*length
          uint8_t length = STR_VSRCRAW[0];
          horzpos_t l_posHorz = menuHorizontalPosition;

          if (i>=NUM_BODY_LINES-2 && getSwitchWarningsCount() > 5*(NUM_BODY_LINES-i)) {
            if (CURSOR_MOVED_LEFT(event))
              menuVerticalOffset--;
            else
              menuVerticalOffset++;
            break;
          }

          swarnstate_t states = g_model.switchWarningState;
          char c;

          lcdDrawTextAlignedLeft(y, STR_SWITCHWARNING);
          if (attr) {
            s_editMode = 0;
            if (!READ_ONLY()) {
              switch (event) {
                case EVT_KEY_BREAK(KEY_ENTER):
                  break;

                case EVT_KEY_LONG(KEY_ENTER):
#if !defined(PCBI6X)
                  if (menuHorizontalPosition < 0 || menuHorizontalPosition >= NUM_SWITCHES)
#endif
                  {
                    START_NO_HIGHLIGHT();
                    getMovedSwitch();
                    g_model.switchWarningState = switches_states;
                    AUDIO_WARNING1();
                    storageDirty(EE_MODEL);
                  }
                  killEvents(event);
                  break;
              }
            }
          }

          int current = 0;
          for (int i = 0; i < NUM_SWITCHES; i++) {
            if (SWITCH_WARNING_ALLOWED(i)) {
              div_t qr = div(current, 5);
              if (!READ_ONLY() && event==EVT_KEY_BREAK(KEY_ENTER) && attr && l_posHorz == current && old_posHorz >= 0) {
                g_model.switchWarningEnable ^= (1 << i);
                storageDirty(EE_MODEL);
              }
              uint8_t swactive = !(g_model.switchWarningEnable & (1<<i));
              c = "\300-\301"[states & 0x03];
              // lcdDrawChar(MODEL_SETUP_2ND_COLUMN+qr.rem*(2*FW+1), y+FH*qr.quot, 'A'+i, attr && (menuHorizontalPosition==current) ? INVERS : 0);
              lcdDrawSizedText(MODEL_SETUP_2ND_COLUMN + qr.rem*((2*FW)+1), y+FH*qr.quot, FIRSTSW_STR+(i*length)+3, 1, attr && (menuHorizontalPosition==current) ? INVERS : 0);
              if (swactive) lcdDrawChar(lcdNextPos, y+FH*qr.quot, c);
              ++current;
            }
            states >>= 2;
          }
          if (attr && (/*(menuHorizontalPosition < 0) ||*/ menuHorizontalPosition >= NUM_SWITCHES)) {
            lcdDrawFilledRect(MODEL_SETUP_2ND_COLUMN-1, y-1, 8*(2*FW+1), 1+FH*((current+4)/5));
          }
#else //PCBTARANIS
      {
        lcdDrawTextAlignedLeft(y, STR_SWITCHWARNING);
        swarnstate_t states = g_model.switchWarningState;
        char c;
        if (attr) {
          s_editMode = 0;
          if (!READ_ONLY()) {
            switch (event) {
              CASE_EVT_ROTARY_BREAK
              case EVT_KEY_BREAK(KEY_ENTER):
                if (menuHorizontalPosition < NUM_SWITCHES-1) {
                  g_model.switchWarningEnable ^= (1 << menuHorizontalPosition);
                  storageDirty(EE_MODEL);
                }
                break;

              case EVT_KEY_LONG(KEY_ENTER):
                if (menuHorizontalPosition == NUM_SWITCHES-1) {
                  START_NO_HIGHLIGHT();
                  getMovedSwitch();
                  g_model.switchWarningState = switches_states;
                  AUDIO_WARNING1();
                  storageDirty(EE_MODEL);
                }
                killEvents(event);
                break;
            }
          }
        }

        for (uint8_t i=0; i<NUM_SWITCHES-1/*not on TRN switch*/; i++) {
          uint8_t swactive = !(g_model.switchWarningEnable & 1 << i);
          attr = 0;

          if (IS_3POS(i)) {
            c = '0'+(states & 0x03);
            states >>= 2;
          }
          else {
            if ((states & 0x01) && swactive)
              attr = INVERS;
            c = *(STR_VSWITCHES - 2 + 9 + (3*(i+1)));
            states >>= 1;
          }
          if (attr && (menuHorizontalPosition == i)) {
            attr = BLINK | INVERS;
          }
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+i*FW, y, (swactive) ? c : '-', attr);
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+(NUM_SWITCHES*FW), y, "<]", (menuHorizontalPosition == NUM_SWITCHES-1 && !NO_HIGHLIGHT()) ? attr : 0);
        }
#endif
        break;
      }

      case ITEM_MODEL_BEEP_CENTER:
        lcdDrawTextAlignedLeft(y, STR_BEEPCTR);
        for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_ROTARY_ENCODERS; i++) {
          // TODO flash saving, \001 not needed in STR_RETA123
          coord_t x = MODEL_SETUP_2ND_COLUMN+i*FW;
          lcdDrawTextAtIndex(x, y, STR_RETA123, i, ((menuHorizontalPosition==i) && attr) ? BLINK|INVERS : (((g_model.beepANACenter & ((BeepANACenter)1<<i)) || (attr && CURSOR_ON_LINE())) ? INVERS : 0 ) );
        }
        if (attr && CURSOR_ON_CELL) {
          if (event==EVT_KEY_BREAK(KEY_ENTER) || p1valdiff) {
            if (READ_ONLY_UNLOCKED()) {
              s_editMode = 0;
              g_model.beepANACenter ^= ((BeepANACenter)1<<menuHorizontalPosition);
              storageDirty(EE_MODEL);
            }
          }
        }
        break;

      case ITEM_MODEL_USE_GLOBAL_FUNCTIONS:
        lcdDrawTextAlignedLeft(y, STR_USE_GLOBAL_FUNCS);
        drawCheckBox(MODEL_SETUP_2ND_COLUMN, y, !g_model.noGlobalFunctions, attr);
        if (attr) g_model.noGlobalFunctions = !checkIncDecModel(event, !g_model.noGlobalFunctions, 0, 1);
        break;

      case ITEM_MODEL_INTERNAL_MODULE_LABEL:
        lcdDrawTextAlignedLeft(y, TR_INTERNALRF);
        break;

      case ITEM_MODEL_INTERNAL_MODULE_MODE:
        lcdDrawTextAlignedLeft(y, INDENT TR_MODE);
        lcdDrawTextAtIndex(
          MODEL_SETUP_2ND_COLUMN, 
          y, 
          STR_I6X_PROTOCOLS, 
          1+g_model.moduleData[INTERNAL_MODULE].rfProtocol, 
          attr);
        if (attr) {
          g_model.moduleData[INTERNAL_MODULE].rfProtocol = 
          checkIncDec(event, 
                      g_model.moduleData[INTERNAL_MODULE].rfProtocol, 
                      RF_I6X_PROTO_OFF, 
                      RF_I6X_PROTO_LAST, 
                      EE_MODEL, 
                      isRfProtocolAvailable);
          if (checkIncDec_Ret) { // modified?
            g_model.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_AFHDS2A_SPI;
            if (g_model.moduleData[INTERNAL_MODULE].rfProtocol == RF_PROTO_OFF){
              g_model.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_NONE;
            }
          }
        }
        break;
        case ITEM_MODEL_INTERNAL_MODULE_SUBTYPE:
        lcdDrawTextAlignedLeft(y, INDENT "Subtype");
        lcdDrawTextAtIndex(
          MODEL_SETUP_2ND_COLUMN, 
          y, 
          STR_SUBTYPE_AFHDS2A, 
          g_model.moduleData[INTERNAL_MODULE].subType, 
          attr);
        if (attr) {
          g_model.moduleData[INTERNAL_MODULE].subType = 
          checkIncDec(event, 
                      g_model.moduleData[INTERNAL_MODULE].subType, 
                      AFHDS2A_SUBTYPE_FIRST, 
                      AFHDS2A_SUBTYPE_LAST, 
                      EE_MODEL, 
                      isSubtypeAvailable);
        }
        break;
        case ITEM_MODEL_INTERNAL_MODULE_SERVOFREQ:
        lcdDrawTextAlignedLeft(y, INDENT "Servo rate");
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.moduleData[INTERNAL_MODULE].afhds2a.servoFreq, attr|LEFT);
//        lcdDrawText(lcdLastRightPos, y, STR_MS, attr);
        if (attr) {
          CHECK_INCDEC_MODELVAR(event, g_model.moduleData[INTERNAL_MODULE].afhds2a.servoFreq, 50, 400);
        }
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_LABEL:
        lcdDrawTextAlignedLeft(y, TR_EXTERNALRF);
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_MODE:
        lcdDrawTextAlignedLeft(y, INDENT TR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_TARANIS_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].type, menuHorizontalPosition==0 ? attr : 0);
#if !defined(PCBI6X)
        if (isModuleXJT(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN+5*FW, y, STR_XJT_PROTOCOLS, 1+g_model.moduleData[EXTERNAL_MODULE].rfProtocol, menuHorizontalPosition==1 ? attr : 0);
        else if (isModuleDSM2(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN+5*FW, y, STR_DSM_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, menuHorizontalPosition==1 ? attr : 0);
        else if (isModuleR9M(EXTERNAL_MODULE)){
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN+5*FW, y, STR_R9M_REGION, g_model.moduleData[EXTERNAL_MODULE].subType, (menuHorizontalPosition==1 ? attr : 0));
        }
#endif
        if (attr && (editMode>0 || p1valdiff)) {
          switch (menuHorizontalPosition) {
            case 0:
              g_model.moduleData[EXTERNAL_MODULE].type = checkIncDec(
                event, 
                g_model.moduleData[EXTERNAL_MODULE].type, 
                MODULE_TYPE_NONE, 
                IS_TRAINER_EXTERNAL_MODULE() ? MODULE_TYPE_NONE : MODULE_TYPE_COUNT-2, // exclude AFHDS2A 
                EE_MODEL, 
                isModuleAvailable);
              if (checkIncDec_Ret) {
                g_model.moduleData[EXTERNAL_MODULE].rfProtocol = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = defaultModuleChannels_M8(EXTERNAL_MODULE);
                if (isModuleSBUS(EXTERNAL_MODULE))
                  g_model.moduleData[EXTERNAL_MODULE].sbus.refreshRate = -31;
                if(isModulePPM(EXTERNAL_MODULE))
                  SET_DEFAULT_PPM_FRAME_LENGTH(EXTERNAL_MODULE);
              }
              break;
            case 1:
              if (isModuleDSM2(EXTERNAL_MODULE))
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, DSM2_PROTO_LP45, DSM2_PROTO_DSMX);
              else if (isModuleR9M(EXTERNAL_MODULE)) {
                uint8_t newR9MType = checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].subType, MODULE_SUBTYPE_R9M_FCC, MODULE_SUBTYPE_R9M_LAST, EE_MODEL, isR9MModeAvailable);
                if (newR9MType != g_model.moduleData[EXTERNAL_MODULE].subType && newR9MType > MODULE_SUBTYPE_R9M_EU) {
                  POPUP_WARNING(STR_R9MFLEXWARN1);
                  const char * w = STR_R9MFLEXWARN2;
                  SET_WARNING_INFO(w, strlen(w), 0);
                }
                g_model.moduleData[EXTERNAL_MODULE].subType = newR9MType;
              }

#if defined(MULTIMODULE)
              else if (isModuleMultimodule(EXTERNAL_MODULE)) {
                int multiRfProto = g_model.moduleData[EXTERNAL_MODULE].multi.customProto == 1 ? MM_RF_PROTO_CUSTOM : g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
                CHECK_INCDEC_MODELVAR(event, multiRfProto, MM_RF_PROTO_FIRST, MM_RF_PROTO_LAST);
                if (checkIncDec_Ret) {
                  g_model.moduleData[EXTERNAL_MODULE].multi.customProto = (multiRfProto == MM_RF_PROTO_CUSTOM);
                  if (!g_model.moduleData[EXTERNAL_MODULE].multi.customProto)
                    g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(multiRfProto);
                  g_model.moduleData[EXTERNAL_MODULE].subType = 0;
                  // Sensible default for DSM2 (same as for ppm): 7ch@22ms + Autodetect settings enabled
                  if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MM_RF_PROTO_DSM2) {
                    g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 1;
                  }
                  else {
                    g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 0;
                  }
                  g_model.moduleData[EXTERNAL_MODULE].multi.optionValue = 0;
                }
              }
#endif
              else {
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, RF_PROTO_X16, RF_PROTO_LAST);
              }
              if (checkIncDec_Ret) {
                g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = defaultModuleChannels_M8(EXTERNAL_MODULE);
              }
          }
        }
        break;
#if defined(MULTIMODULE)
      case ITEM_MODEL_EXTERNAL_MODULE_SUBTYPE:
      {
        lcdDrawTextAlignedLeft(y, STR_SUBTYPE);
        uint8_t multi_rfProto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true);
        const mm_protocol_definition * pdef = getMultiProtocolDefinition(multi_rfProto);

        if (multi_rfProto == MM_RF_CUSTOM_SELECTED) {
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN + 3 * FW, y, g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false), RIGHT | (menuHorizontalPosition == 0 ? attr : 0), 2);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN + 5 * FW, y, g_model.moduleData[EXTERNAL_MODULE].subType, RIGHT | (menuHorizontalPosition == 1 ? attr : 0), 2);
        }
        else {
          if (pdef->subTypeString != nullptr)
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, pdef->subTypeString, g_model.moduleData[EXTERNAL_MODULE].subType, attr);
        }
        if (attr && (editMode > 0 || p1valdiff)) {
          switch (menuHorizontalPosition) {
            case 0:
              if (multi_rfProto == MM_RF_CUSTOM_SELECTED)
                g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false), 0, 63, EE_MODEL));
              else if (pdef->maxSubtype > 0)
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, pdef->maxSubtype);
              break;
            case 1:
              // Custom protocol, third column is subtype
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, 7);
              break;
          }
        }
      }
      break;
#endif

      case ITEM_MODEL_TRAINER_LABEL:
        lcdDrawTextAlignedLeft(y, STR_TRAINER);
        break;

      case ITEM_MODEL_TRAINER_MODE:
        lcdDrawTextAlignedLeft(y, INDENT TR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VTRAINERMODES, g_model.trainerMode, attr);
        if (attr) {
          g_model.trainerMode = checkIncDec(event, g_model.trainerMode, 0, TRAINER_MODE_MAX(), EE_MODEL, isTrainerModeAvailable);
        }
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_CHANNELS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        lcdDrawTextAlignedLeft(y, STR_CHANNELRANGE);
        if ((int8_t)PORT_CHANNELS_ROWS(moduleIdx) >= 0) {
          lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_CH, menuHorizontalPosition==0 ? attr : 0);
          lcdDrawNumber(lcdLastRightPos, y, moduleData.channelsStart+1, LEFT | (menuHorizontalPosition==0 ? attr : 0));
          lcdDrawChar(lcdLastRightPos, y, '-');
          lcdDrawNumber(lcdLastRightPos + FW+1, y, moduleData.channelsStart+sentModuleChannels(moduleIdx), LEFT | (menuHorizontalPosition==1 ? attr : 0));
          if (attr && (editMode>0 || p1valdiff)) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.channelsStart, 32-8-moduleData.channelsCount);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.channelsCount, -4, min<int8_t>(maxModuleChannels_M8(moduleIdx), 32-8-moduleData.channelsStart));
                if ((k == ITEM_MODEL_EXTERNAL_MODULE_CHANNELS && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM)) {
                  SET_DEFAULT_PPM_FRAME_LENGTH(moduleIdx);
                }
                break;
            }
          }
        }
        break;
      }

      case ITEM_MODEL_INTERNAL_MODULE_BIND:
      case ITEM_MODEL_EXTERNAL_MODULE_BIND:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        if (isModulePPM(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_PPMFRAME);
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+3*FW, y, STR_MS);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength * PPM_STEP_SIZE + PPM_DEF_PERIOD, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT);
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, 'u');
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, (moduleData.ppm.delay*50)+300, RIGHT | ((CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0));
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+10*FW, y, moduleData.ppm.pulsePol ? '+' : '-', (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
          if (attr && (editMode>0 || p1valdiff)) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -20, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.delay, -4, 10);
                break;
              case 2:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.ppm.pulsePol, 1);
                break;
            }
          }
        }
        else if (isModuleSBUS(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_REFRESHRATE);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.sbus.refreshRate * SBUS_STEPSIZE + SBUS_DEF_PERIOD, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT);
          lcdDrawText(lcdLastRightPos, y, STR_MS);
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+5*FW+2, y, moduleData.sbus.noninverted ? "no inv" : "normal", (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0);

          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.sbus.refreshRate, (SBUS_MIN_PERIOD - SBUS_DEF_PERIOD) / SBUS_STEPSIZE, (SBUS_MAX_PERIOD - SBUS_DEF_PERIOD) / SBUS_STEPSIZE);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.sbus.noninverted, 1);
                break;
            }
          }
        } else {
          horzpos_t l_posHorz = menuHorizontalPosition;
          coord_t xOffsetBind = MODEL_SETUP_BIND_OFS;
          if (isModuleXJT(moduleIdx) && IS_D8_RX(moduleIdx)) {
            xOffsetBind = 0;
            lcdDrawTextAlignedLeft(y, STR_RECEIVER);
            if (attr) l_posHorz += 1;
//          } else if (isModuleCrossfire(moduleIdx)) {
//            lcdDrawTextAlignedLeft(y, STR_RECEIVER);
          } else {
            lcdDrawTextAlignedLeft(y, STR_RECEIVER);
          }
          if (isModulePXX(moduleIdx) || isModuleDSM2(moduleIdx) || isModuleMultimodule(moduleIdx) || isModuleCrossfire(moduleIdx) || isModuleA7105(moduleIdx)) {
            if (xOffsetBind)
              lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[moduleIdx], (l_posHorz==0 ? attr : 0) | LEADING0|LEFT, 2);
            if (attr && l_posHorz == 0) {
              if (editMode>0 || p1valdiff) {
                CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[moduleIdx], MAX_RX_NUM(moduleIdx));
                if (checkIncDec_Ret) {
                  if (isModuleCrossfire(moduleIdx))
                    moduleState[EXTERNAL_MODULE].counter = CRSF_FRAME_MODELID;
                  modelHeaders[g_eeGeneral.currModel].modelId[moduleIdx] = g_model.header.modelId[moduleIdx];
                }
                else if (event == EVT_KEY_LONG(KEY_ENTER)) {
                  killEvents(event);
                  uint8_t newVal = findNextUnusedModelId(g_eeGeneral.currModel, moduleIdx);
                  if (newVal != g_model.header.modelId[moduleIdx]) {
                    modelHeaders[g_eeGeneral.currModel].modelId[moduleIdx] = g_model.header.modelId[moduleIdx] = newVal;
                    storageDirty(EE_MODEL);
                  }
                }
              }
            }
          }
          if (isModuleA7105(moduleIdx)) { // isModuleBindRangeAvailable
            lcdDrawText(MODEL_SETUP_2ND_COLUMN+xOffsetBind, y, STR_MODULE_BIND, l_posHorz==1 ? attr : 0);
            lcdDrawText(MODEL_SETUP_2ND_COLUMN+MODEL_SETUP_RANGE_OFS+xOffsetBind, y, STR_MODULE_RANGE, l_posHorz==2 ? attr : 0);
            uint8_t newFlag = 0;
#if defined(MULTIMODULE)
            if (multiBindStatus == MULTI_BIND_FINISHED) {
              multiBindStatus = MULTI_NORMAL_OPERATION;
              s_editMode = 0;
            }
#endif

            if (attr && l_posHorz>0 && s_editMode>0) {
              if (l_posHorz == 1)
                newFlag = MODULE_MODE_BIND;
              else if (l_posHorz == 2)
                newFlag = MODULE_MODE_RANGECHECK;
            }

            moduleState[moduleIdx].mode = newFlag;

#if defined(MULTIMODULE)
            if (newFlag == MODULE_MODE_BIND) {
              multiBindStatus = MULTI_BIND_INITIATED;
            }
#endif
          }
        }
        break;
      }

      case ITEM_MODEL_INTERNAL_MODULE_FAILSAFE:
      case ITEM_MODEL_EXTERNAL_MODULE_FAILSAFE: {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData &moduleData = g_model.moduleData[moduleIdx];
        lcdDrawTextAlignedLeft(y, STR_FAILSAFE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VFAILSAFE, moduleData.failsafeMode, menuHorizontalPosition == 0 ? attr : 0);
        if (moduleData.failsafeMode == FAILSAFE_CUSTOM)
          lcdDrawText(MODEL_SETUP_2ND_COLUMN + MODEL_SETUP_SET_FAILSAFE_OFS, y, STR_SET, menuHorizontalPosition == 1 ? attr : 0);
        if (attr) {
          if (moduleData.failsafeMode != FAILSAFE_CUSTOM)
            menuHorizontalPosition = 0;
          if (menuHorizontalPosition == 0) {
            if (editMode > 0 || p1valdiff) {
              CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.failsafeMode, FAILSAFE_LAST);
              if (checkIncDec_Ret) SEND_FAILSAFE_NOW(moduleIdx);
            }
          } else if (menuHorizontalPosition == 1) {
            s_editMode = 0;
            if (moduleData.failsafeMode == FAILSAFE_CUSTOM) {
              if (event == EVT_KEY_LONG(KEY_ENTER)) {
                killEvents(event);
                setCustomFailsafe(moduleIdx);
                storageDirty(EE_MODEL);
                AUDIO_WARNING1();
                SEND_FAILSAFE_NOW(moduleIdx);
              }
              else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
                g_moduleIdx = moduleIdx;
                pushMenu(menuModelFailsafe);
              }
            }
          } else {
            lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN, y, LCD_W - MODEL_SETUP_2ND_COLUMN, 8);
          }
        }
      }
      break;

      case ITEM_MODEL_EXTERNAL_MODULE_OPTIONS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
#if defined(MULTIMODULE)
        if (isModuleMultimodule(moduleIdx)) {
          int optionValue = g_model.moduleData[moduleIdx].multi.optionValue;

          const uint8_t multi_proto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true);
          const mm_protocol_definition * pdef = getMultiProtocolDefinition(multi_proto);
          if (pdef->optionsstr)
            lcdDrawTextAlignedLeft(y, pdef->optionsstr);

          if (multi_proto == MM_RF_PROTO_FS_AFHDS2A)
            optionValue = 50 + 5 * optionValue;

          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, optionValue, LEFT | attr);
          if (attr) {
            if (multi_proto == MM_RF_PROTO_FS_AFHDS2A) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, 0, 70);
            }
            else if (multi_proto == MM_RF_PROTO_OLRS) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -1, 7);
            }
            else {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -128, 127);
            }
          }
        }
#endif
        if (isModuleR9M(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_MODULE_TELEMETRY);
          if (IS_TELEMETRY_INTERNAL_MODULE()) {
            lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_DISABLE_INTERNAL);
          }
          else {
            lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_MODULE_TELEM_ON);
          }
        }
        else if (isModuleSBUS(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_WARN_BATTVOLTAGE);
          putsVolts(lcdLastRightPos, y, getBatteryVoltage(), attr | PREC2 | LEFT);
        }
        break;
      }
#if !defined(PCBI6X)
      case ITEM_MODEL_EXTERNAL_MODULE_POWER:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        if (isModuleR9M(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, TR_MULTI_RFPOWER);
          if (isModuleR9M_FCC_VARIANT(moduleIdx)) {
            g_model.moduleData[moduleIdx].pxx.power = min((uint8_t)g_model.moduleData[moduleIdx].pxx.power, (uint8_t)R9M_FCC_POWER_MAX); // Lite FCC has only one setting
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_FCC_POWER_VALUES, g_model.moduleData[moduleIdx].pxx.power, LEFT | attr);
            if (attr)
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.moduleData[moduleIdx].pxx.power, R9M_FCC_POWER_MAX);
          }
          else {
            g_model.moduleData[moduleIdx].pxx.power = min((uint8_t)g_model.moduleData[moduleIdx].pxx.power, (uint8_t)R9M_LBT_POWER_MAX);
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_LBT_POWER_VALUES, g_model.moduleData[moduleIdx].pxx.power, LEFT | attr);
            if (attr) {
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.moduleData[moduleIdx].pxx.power, R9M_LBT_POWER_MAX);
            }
            if (attr && editMode == 0 && reusableBuffer.modelsetup.r9mPower != g_model.moduleData[moduleIdx].pxx.power) {
              if((reusableBuffer.modelsetup.r9mPower + g_model.moduleData[moduleIdx].pxx.power) < 5) { //switching between mode 2 and 3 does not require rebind
                POPUP_WARNING(STR_REBIND);
              }
              reusableBuffer.modelsetup.r9mPower = g_model.moduleData[moduleIdx].pxx.power;
            }
          }
        }
#if defined(MULTIMODULE)
        else if (isModuleMultimodule(moduleIdx)) {
          g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode = editCheckBox(g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode, MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_LOWPOWER, attr, event);
        }
#endif
      }
      break;
#endif // PCBI6X
#if defined(MULTIMODULE)
      case ITEM_MODEL_EXTERNAL_MODULE_AUTOBIND:
        if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MM_RF_PROTO_DSM2)
          g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = editCheckBox(g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode, MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_DSM_AUTODTECT, attr, event);
        else
          g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = editCheckBox(g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode, MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_AUTOBIND, attr, event);
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_STATUS: {
        lcdDrawTextAlignedLeft(y, STR_MODULE_STATUS);

        char statusText[64];
        multiModuleStatus.getStatusString(statusText);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, statusText);
        break;
      }

      case ITEM_MODEL_EXTERNAL_MODULE_SYNCSTATUS: {
        lcdDrawTextAlignedLeft(y, STR_MODULE_SYNC);

        char statusText[64];
        multiSyncStatus.getRefreshString(statusText);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, statusText);
        break;
      }
#endif


#if 0
      case ITEM_MODEL_PPM2_PROTOCOL:
        lcdDrawTextAlignedLeft(y, "Port2");
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VPROTOS, 0, 0);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+4*FW+3, y, STR_CH, menuHorizontalPosition<=0 ? attr : 0);
        lcdDrawNumber(lcdLastRightPos, y, g_model.moduleData[1].channelsStart+1, LEFT | (menuHorizontalPosition<=0 ? attr : 0));
        lcdDrawChar(lcdLastRightPos, y, '-');
        lcdDrawNumber(lcdLastRightPos + FW+1, y, g_model.moduleData[1].channelsStart+8+g_model.moduleData[1].channelsCount, LEFT | (menuHorizontalPosition!=0 ? attr : 0));
        if (attr && (editMode>0 || p1valdiff)) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.moduleData[1].channelsStart, 32-8-g_model.moduleData[1].channelsCount);
              SET_DEFAULT_PPM_FRAME_LENGTH(1);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].channelsCount, -4, min<int8_t>(8, 32-8-g_model.moduleData[1].channelsStart));
              SET_DEFAULT_PPM_FRAME_LENGTH(1);
              break;
          }
        }
        break;

      case ITEM_MODEL_PPM2_PARAMS:
        lcdDrawTextAlignedLeft(y, STR_PPMFRAME);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+3*FW, y, STR_MS);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)g_model.moduleData[1].ppmFrameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1 | LEFT);
        lcdDrawChar(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, 'u');
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, (g_model.moduleData[1].ppmDelay*50)+300, RIGHT | ((menuHorizontalPosition < 0 || menuHorizontalPosition==1) ? attr : 0));
        lcdDrawChar(MODEL_SETUP_2ND_COLUMN+10*FW, y, g_model.moduleData[1].ppmPulsePol ? '+' : '-', (menuHorizontalPosition < 0 || menuHorizontalPosition==2) ? attr : 0);
        if (attr && (editMode>0 || p1valdiff)) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].ppmFrameLength, -20, 35);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].ppmDelay, -4, 10);
              break;
            case 2:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.moduleData[1].ppmPulsePol, 1);
              break;
          }
        }
        break;
#endif

    }
  }

  if (IS_RANGECHECK_ENABLE()) {
    showMessageBox("RQly ");
    lcdDrawNumber(16+4*FW, 5*FH, TELEMETRY_RSSI(), BOLD);
  }

  // some field just finished being edited
  if (old_editMode > 0 && s_editMode == 0) {
    switch(menuVerticalPosition) {
    case ITEM_MODEL_INTERNAL_MODULE_BIND:
      if (menuHorizontalPosition == 0)
        checkModelIdUnique(g_eeGeneral.currModel, INTERNAL_MODULE);
      break;
      case ITEM_MODEL_EXTERNAL_MODULE_BIND:
      if (menuHorizontalPosition == 0)
        checkModelIdUnique(g_eeGeneral.currModel, EXTERNAL_MODULE);
      break;
    }
  }
}

void menuModelFailsafe(event_t event)
{
  const uint8_t channelStart = g_model.moduleData[g_moduleIdx].channelsStart;
  const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;
  uint8_t wbar = LCD_W - FW * 4 - FWNUM * 4;
#if defined(PPM_UNIT_PERCENT_PREC1)
  wbar -= 6;
#endif

  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    killEvents(event);
    event = 0;

    if (menuVerticalPosition < sentModuleChannels(g_moduleIdx)) {
      if (s_editMode) {
        g_model.moduleData[g_moduleIdx].failsafeChannels[menuVerticalPosition] = channelOutputs[menuVerticalPosition+channelStart];
        s_editMode = 0;
      }
      else {
        int16_t * failsafe = &g_model.moduleData[g_moduleIdx].failsafeChannels[menuVerticalPosition];
        if (*failsafe < FAILSAFE_CHANNEL_HOLD)
          *failsafe = FAILSAFE_CHANNEL_HOLD;
        else if (*failsafe == FAILSAFE_CHANNEL_HOLD)
          *failsafe = FAILSAFE_CHANNEL_NOPULSE;
        else
          *failsafe = 0;
      }
    }
    else {
      // "Outputs => Failsafe" menu item
      setCustomFailsafe(g_moduleIdx);
    }

    storageDirty(EE_MODEL);
    AUDIO_WARNING1();
    SEND_FAILSAFE_NOW(g_moduleIdx);
  }

  SIMPLE_SUBMENU_NOTITLE(sentModuleChannels(g_moduleIdx) + 1);

  lcdDrawText(LCD_W / 2, 0, STR_FAILSAFESET, CENTERED);
  lcdInvertLine(0);

  const coord_t x = 1;
  coord_t y = FH + 1;
  uint8_t line = (menuVerticalPosition >= sentModuleChannels(g_moduleIdx) ? 2 : 0);
  uint8_t ch = (menuVerticalPosition >= 8 ? 8 : 0) + line;

  // Channels
  for (; line < 8; line++) {
    const int32_t channelValue = channelOutputs[ch+channelStart];
    int32_t failsafeValue = g_model.moduleData[g_moduleIdx].failsafeChannels[ch];

    //Channel
    putsChn(x+1, y, ch+1, SMLSIZE);

    // Value
    LcdFlags flags = TINSIZE;
    if (menuVerticalPosition == ch) {
      flags |= INVERS;
      if (s_editMode) {
        if (failsafeValue == FAILSAFE_CHANNEL_HOLD || failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
          s_editMode = 0;
        }
        else {
          flags |= BLINK;
          CHECK_INCDEC_MODELVAR(event, g_model.moduleData[g_moduleIdx].failsafeChannels[ch], -lim, +lim);
        }
      }
    }

    uint8_t xValue = x+LCD_W-4-wbar;
    if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
      lcdDrawText(xValue, y, STR_HOLD, RIGHT|flags);
      failsafeValue = 0;
    }
    else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
      lcdDrawText(xValue, y, STR_NONE, RIGHT|flags);
      failsafeValue = 0;
    }
    else {
#if defined(PPM_UNIT_US)
      lcdDrawNumber(xValue, y, PPM_CH_CENTER(ch)+failsafeValue/2, RIGHT|flags);
#elif defined(PPM_UNIT_PERCENT_PREC1)
      lcdDrawNumber(xValue, y, calcRESXto1000(failsafeValue), RIGHT|PREC1|flags);
#else
      lcdDrawNumber(xValue, y, calcRESXto1000(failsafeValue)/10, RIGHT|flags);
#endif
    }

    // Gauge
#if !defined(PCBX7) && !defined(PCBI6X) // X7  & i6X LCD doesn't like too many horizontal lines
    lcdDrawRect(x+LCD_W-3-wbar, y, wbar+1, 6);
#endif
    const uint8_t lenChannel = limit<uint8_t>(1, (abs(channelValue) * wbar/2 + lim/2) / lim, wbar/2);
    const uint8_t lenFailsafe = limit<uint8_t>(1, (abs(failsafeValue) * wbar/2 + lim/2) / lim, wbar/2);
    const coord_t xChannel = (channelValue>0) ? x+LCD_W-3-wbar/2 : x+LCD_W-2-wbar/2-lenChannel;
    const coord_t xFailsafe = (failsafeValue>0) ? x+LCD_W-3-wbar/2 : x+LCD_W-2-wbar/2-lenFailsafe;
    lcdDrawHorizontalLine(xChannel, y+1, lenChannel, DOTTED, 0);
    lcdDrawHorizontalLine(xChannel, y+2, lenChannel, DOTTED, 0);
    lcdDrawSolidHorizontalLine(xFailsafe, y+3, lenFailsafe);
    lcdDrawSolidHorizontalLine(xFailsafe, y+4, lenFailsafe);

    y += FH - 1;

    if (++ch >= sentModuleChannels(g_moduleIdx))
      break;
  }

  if (menuVerticalPosition >= sentModuleChannels(g_moduleIdx)) {
    // Outputs => Failsafe
    lcdDrawText(CENTER_OFS, LCD_H - (FH + 1), STR_OUTPUTS2FAILSAFE, INVERS);
  }
}
