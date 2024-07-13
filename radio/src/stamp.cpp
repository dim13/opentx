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
#include "stamp.h"

#define STR2(s) #s
#define DEFNUMSTR(s)  STR2(s)

#define EEPROM_STR DEFNUMSTR(EEPROM_VER);

#define TAB "\037\033"

#if defined(FRSKY_RELEASE)
#define DISPLAY_VERSION "-frsky"
#elif defined(FLYSKY_RELEASE)
#define DISPLAY_VERSION "-flysky"
#elif defined(JUMPER_RELEASE)
#define DISPLAY_VERSION "-jumper"
#elif defined(RADIOMASTER_RELEASE)
#define DISPLAY_VERSION "-RM"
#elif defined(TBS_RELEASE)
#define DISPLAY_VERSION "-tbs"
#elif defined(IMRC_RELEASE)
#define DISPLAY_VERSION "-imrc"
#else
#define DISPLAY_VERSION
#endif

const char vers_stamp[]  = "FW" TAB ": openi6x" "\036VERS" TAB ": " VERSION DISPLAY_VERSION " (" GIT_STR ")" "\036DATE" TAB ": " DATE " " TIME "\036EEPR" TAB ": " EEPROM_STR;
