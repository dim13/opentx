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

#include "gtests.h"

void setLogicalSwitch(int index, uint16_t _func, int16_t _v1, int16_t _v2, int16_t _v3 = 0, uint8_t _delay = 0, uint8_t _duration = 0, int8_t _andsw = 0)
{
  g_model.logicalSw[index].func = _func;
  g_model.logicalSw[index].v1 = _v1;
  g_model.logicalSw[index].v2 = _v2;
  g_model.logicalSw[index].v3 = _v3;
  g_model.logicalSw[index].delay = _delay;
  g_model.logicalSw[index].duration = _duration;
  g_model.logicalSw[index].andsw = _andsw;
}

#if defined(PCBTARANIS)
TEST(getSwitch, OldTypeStickyCSW)
{
  RADIO_RESET();
  MODEL_RESET();
  MIXER_RESET();

  setLogicalSwitch(0, LS_FUNC_AND, SWSRC_SA0, SWSRC_NONE);
  setLogicalSwitch(1, LS_FUNC_OR, SWSRC_SW1, SWSRC_SW2);

  simuSetSwitch(0, 0);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SA0, both switches should become true
  simuSetSwitch(0, -1);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(0, 0);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now reset logical switches
  logicalSwitchesReset();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);
}
#endif

TEST(getSwitch, nullSW)
{
  MODEL_RESET();
  EXPECT_EQ(getSwitch(0), true);
}


#if defined(PCBTARANIS)
TEST(getSwitch, inputWithTrim)
{
  MODEL_RESET();
  modelDefault(0);
  MIXER_RESET();

  // g_model.logicalSw[0] = { LS_FUNC_VPOS, MIXSRC_FIRST_INPUT, 0, 0 };
  setLogicalSwitch(0, LS_FUNC_VPOS, MIXSRC_FIRST_INPUT, 0, 0);

  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  setTrimValue(0, 0, 32);
  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
}
#endif
