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

const unsigned char font_5x7[]  = {
#include "font_05x07.lbm"
};

#if defined(BOLD_SPECIFIC_FONT)
const unsigned char font_5x7_B[]  = {
#include "font_05x07_B_compressed.lbm"
};
#endif

const unsigned char font_10x14[]  = {
#include "font_10x14_compressed.lbm"
};

const unsigned char font_3x5[]  = {
#include "font_03x05.lbm"
};
const unsigned char font_4x6[]  = {
#include "font_04x06.lbm"
};

const unsigned char font_8x10[]  = {
#include "font_08x10.lbm"
};

// only lua uses those
// const unsigned char font_22x38_num[]  = {
// #include "font_22x38_num.lbm"
// };

const unsigned char font_4x6_extra[]  = {
#include "font_04x06_extra.lbm"
};

const unsigned char font_5x7_extra[]  = {
#include "font_05x07_extra.lbm"
};

const unsigned char font_10x14_extra[]  = {
#include "font_10x14_extra.lbm"
};
