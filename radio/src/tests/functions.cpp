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

class SpecialFunctionsTest : public OpenTxTest {};

TEST_F(SpecialFunctionsTest, SwitchFiledSize)
{
  // test the size of swtch member
  g_model.customFn[0].swtch = SWSRC_LAST;
  EXPECT_EQ(g_model.customFn[0].swtch, SWSRC_LAST) << "CustomFunctionData.swtch member is too small to hold all possible values";
  g_model.customFn[0].swtch = -SWSRC_LAST;
  EXPECT_EQ(g_model.customFn[0].swtch, -SWSRC_LAST) << "CustomFunctionData.swtch member is too small to hold all possible values";
}

