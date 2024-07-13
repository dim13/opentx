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

#include <OsConfig.h>
#include "board.h"

#define SYSTEM_TICKS_1US    ((CFG_CPU_FREQ + 500000)  / 1000000)      // number of system ticks in 1us
#define SYSTEM_TICKS_01US   ((CFG_CPU_FREQ + 5000000) / 10000000)     // number of system ticks in 0.1us (rounding needed for sys frequencies that are not multiple of 10MHz)

void delay_01us(uint16_t nb)
{
    TIM6->SR = 0;
    TIM6->PSC = 0;
    //104,166666667ns
    TIM6->ARR = 4 * nb;
    TIM6->CR1 |= TIM_CR1_CEN;
    while (!(TIM6->SR & TIM_SR_UIF));
}
void delay_us(uint16_t nb)
{
    TIM6->SR = 0;
    TIM6->PSC = 0;
    TIM6->ARR = 47 * nb;
    TIM6->CR1 |= TIM_CR1_CEN;
    while (!(TIM6->SR & TIM_SR_UIF));
}

void delay_ms(uint32_t ms)
{
  while (ms--) {
    delay_us(1000);
  }
}
