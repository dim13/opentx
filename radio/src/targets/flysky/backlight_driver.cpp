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

void backlightInit()
{
  GPIO_InitTypeDef gpio_init_c = {
    .GPIO_Pin = GPIO_Pin_9,
    .GPIO_Mode = GPIO_Mode_AF,
    .GPIO_Speed = GPIO_Speed_Level_1, // 2MHz
    .GPIO_OType = GPIO_OType_PP,      // Push/Pull
    .GPIO_PuPd = GPIO_PuPd_NOPULL,    // no pull-up
  };
  GPIO_Init(GPIOC, &gpio_init_c);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_0);

  TIM3->ARR = 100;
  TIM3->PSC = BACKLIGHT_TIMER_FREQ / 50000 - 1; // 20us * 100 = 2ms => 500Hz
  TIM3->CCMR2 = BACKLIGHT_CCMR2; // PWM
  TIM3->CCER = BACKLIGHT_CCER;
  TIM3->CCR4 = 100;
  TIM3->EGR = 0;
  TIM3->CR1 = TIM_CR1_CEN;  // Counter enable

  // std
  GPIO_InitTypeDef gpio_init_f = {
    .GPIO_Pin = GPIO_Pin_3,
    .GPIO_Mode = GPIO_Mode_OUT,
    .GPIO_Speed = GPIO_Speed_Level_1, // 2MHz
    .GPIO_OType = GPIO_OType_PP,      // Push/Pull
    .GPIO_PuPd = GPIO_PuPd_NOPULL,    // no pull-up
  };
  GPIO_Init(GPIOF, &gpio_init_f);
}

void backlightEnable(uint8_t level)
{
  BACKLIGHT_COUNTER_REGISTER = /*100 -*/ level;
  BACKLIGHT_TIMER->CR1 = TIM_CR1_CEN;

  // std
  if (level == 0) { // inverted
    GPIO_SetBits(BACKLIGHT_STD_GPIO, BACKLIGHT_STD_GPIO_PIN);
  } else {
    GPIO_ResetBits(BACKLIGHT_STD_GPIO, BACKLIGHT_STD_GPIO_PIN);
  }
}

void backlightDisable()
{
  BACKLIGHT_COUNTER_REGISTER = 100;
  BACKLIGHT_TIMER->CR1 &= ~TIM_CR1_CEN;          // solves very dim light with backlight off

  // std
  GPIO_ResetBits(BACKLIGHT_STD_GPIO, BACKLIGHT_STD_GPIO_PIN);
}

uint8_t isBacklightEnabled()
{
  return BACKLIGHT_COUNTER_REGISTER != 100 || GPIO_ReadInputDataBit(BACKLIGHT_STD_GPIO, BACKLIGHT_STD_GPIO_PIN) != 0;
}
