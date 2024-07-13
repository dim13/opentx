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

#ifndef _BUZZER_H_
#define _BUZZER_H_

enum FragmentTypes {
  FRAGMENT_EMPTY,
  FRAGMENT_TONE,
  FRAGMENT_FILE,
};

extern bool isPlaying(uint8_t id = 0);

void audioEvent(unsigned int index);

extern void pushPrompt(uint16_t prompt);
extern void pushUnit(uint8_t unit, uint8_t idx, uint8_t id);

#define AUDIO_ERROR_MESSAGE(e)   audioEvent(e)
#define AUDIO_TIMER_MINUTE(t)    

#define AUDIO_KEY_PRESS()        audioKeyPress()
#define AUDIO_KEY_ERROR()        audioKeyError()

#define AUDIO_HELLO()            PUSH_SYSTEM_PROMPT(AUDIO_HELLO)
#define AUDIO_BYE()              
#define AUDIO_WARNING1()         audioEvent(AU_WARNING1)
#define AUDIO_WARNING2()         audioEvent(AU_WARNING2)
#define AUDIO_TX_BATTERY_LOW()   audioEvent(AU_TX_BATTERY_LOW)
#define AUDIO_ERROR()            audioEvent(AU_ERROR)
#define AUDIO_TIMER_COUNTDOWN(idx, val) audioTimerCountdown(idx, val)
#define AUDIO_TIMER_ELAPSED(idx) audioEvent(AU_TIMER1_ELAPSED+idx)
#define AUDIO_INACTIVITY()       audioEvent(AU_INACTIVITY)
#define AUDIO_MIX_WARNING(x)     audioEvent(AU_MIX_WARNING_1+x-1)
#define AUDIO_POT_MIDDLE(x)      audioEvent(AU_STICK1_MIDDLE+x)
#define AUDIO_TRIM_MIDDLE()      audioEvent(AU_TRIM_MIDDLE)
#define AUDIO_TRIM_MIN()         audioEvent(AU_TRIM_MIN)
#define AUDIO_TRIM_MAX()         audioEvent(AU_TRIM_MAX)
#define AUDIO_TRIM_PRESS(val)    audioTrimPress(val)
#define AUDIO_PLAY(p)            audioEvent(p)
#define AUDIO_VARIO(fq, t, p, f) playTone(fq, t, p, f)
#define AUDIO_RSSI_ORANGE()      audioEvent(AU_RSSI_ORANGE)
#define AUDIO_RSSI_RED()         audioEvent(AU_RSSI_RED)
#define AUDIO_RAS_RED()          audioEvent(AU_RAS_RED)
#define AUDIO_TELEMETRY_LOST()   audioEvent(AU_TELEMETRY_LOST)
#define AUDIO_TELEMETRY_BACK()   audioEvent(AU_TELEMETRY_BACK)
#define AUDIO_TRAINER_LOST()     //audioEvent(AU_TRAINER_LOST)
#define AUDIO_TRAINER_BACK()     //audioEvent(AU_TRAINER_BACK)

#define AUDIO_RESET()
#define AUDIO_FLUSH()

#define PLAY_PHASE_OFF(phase)
#define PLAY_PHASE_ON(phase)
#define PLAY_SWITCH_MOVED(sw)
#define PLAY_LOGICAL_SWITCH_OFF(sw)
#define PLAY_LOGICAL_SWITCH_ON(sw)
#define PLAY_MODEL_NAME()
#define START_SILENCE_PERIOD()
#define IS_SILENCE_PERIOD_ELAPSED()   (true)

#define PROMPT_CUSTOM_BASE      0
#define PROMPT_I18N_BASE        256
#define PROMPT_SYSTEM_BASE      480

#define I18N_PLAY_FUNCTION(lng, x, ...) void lng ## _ ## x(__VA_ARGS__, uint8_t id)
#define PLAY_FUNCTION(x, ...)           void x(__VA_ARGS__, uint8_t id)

#define PUSH_CUSTOM_PROMPT(p, id)       pushPrompt(PROMPT_CUSTOM_BASE+(p))
#define PUSH_NUMBER_PROMPT(p)           pushPrompt(PROMPT_I18N_BASE+(p))
#define PUSH_SYSTEM_PROMPT(p)           pushPrompt(PROMPT_SYSTEM_BASE+(p))
#define PLAY_NUMBER(n, u, a)            
#define PUSH_UNIT_PROMPT(p, i)   		
#define PLAY_DURATION(d, att)    		{}
#define PLAY_TIME
#define IS_PLAY_TIME()                  (0)
#define IS_PLAYING(id)                  isPlaying((id))
#define PLAY_VALUE(v, id)        		playValue((v), (id))

#define setScaledVolume(v)

#endif // _BUZZER_H_
