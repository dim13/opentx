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

// FIXME(ds) STICK1, STICK2, STICK3, STICK4, POT1, POT2, TX_VOLTAGE
const int8_t ana_direction[NUM_ANALOGS] = {1,-1,1,-1,  1,-1,0};

#if NUM_PWMSTICKS > 0
  #define FIRST_ANALOG_ADC             (STICKS_PWM_ENABLED() ? NUM_PWMSTICKS : 0)
  #define NUM_ANALOGS_ADC              (STICKS_PWM_ENABLED() ? (NUM_ANALOGS - NUM_PWMSTICKS) : NUM_ANALOGS)
#else
  #define FIRST_ANALOG_ADC             0
  #define NUM_ANALOGS_ADC              NUM_ANALOGS
#endif

uint16_t adcValues[NUM_ANALOGS] __DMA;

#if defined(STM32F0)
void adcInit()
{

}

void adcSingleRead()
{


}
#else
void adcInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

#if defined(ADC_GPIOA_PINS)
  GPIO_InitStructure.GPIO_Pin = ADC_GPIOA_PINS;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

#if defined(ADC_GPIOB_PINS)
  GPIO_InitStructure.GPIO_Pin = ADC_GPIOB_PINS;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

#if defined(ADC_GPIOC_PINS)
  GPIO_InitStructure.GPIO_Pin = ADC_GPIOC_PINS;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

#if defined(ADC_GPIOF_PINS)
  GPIO_InitStructure.GPIO_Pin = ADC_GPIOF_PINS;
  GPIO_Init(GPIOF, &GPIO_InitStructure);
#endif

  ADC_MAIN->CR1 = ADC_CR1_SCAN;
  ADC_MAIN->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC_MAIN->SQR1 = (NUM_ANALOGS_ADC-1) << 20; // bits 23:20 = number of conversions

  ADC_MAIN->SQR2 = (ADC_CHANNEL_POT3<<0) + (ADC_CHANNEL_SLIDER1<<5) + (ADC_CHANNEL_SLIDER2<<10) + (ADC_CHANNEL_BATT<<15); // conversions 7 and more
  ADC_MAIN->SQR3 = (ADC_CHANNEL_STICK_LH<<0) + (ADC_CHANNEL_STICK_LV<<5) + (ADC_CHANNEL_STICK_RV<<10) + (ADC_CHANNEL_STICK_RH<<15) + (ADC_CHANNEL_POT1<<20) + (ADC_CHANNEL_POT2<<25); // conversions 1 to 6

  ADC_MAIN->SMPR1 = ADC_SAMPTIME + (ADC_SAMPTIME<<3) + (ADC_SAMPTIME<<6) + (ADC_SAMPTIME<<9) + (ADC_SAMPTIME<<12) + (ADC_SAMPTIME<<15) + (ADC_SAMPTIME<<18) + (ADC_SAMPTIME<<21) + (ADC_SAMPTIME<<24);
  ADC_MAIN->SMPR2 = ADC_SAMPTIME + (ADC_SAMPTIME<<3) + (ADC_SAMPTIME<<6) + (ADC_SAMPTIME<<9) + (ADC_SAMPTIME<<12) + (ADC_SAMPTIME<<15) + (ADC_SAMPTIME<<18) + (ADC_SAMPTIME<<21) + (ADC_SAMPTIME<<24) + (ADC_SAMPTIME<<27);

  ADC->CCR = 0;

  ADC_DMA_Stream->CR = DMA_SxCR_PL | ADC_DMA_SxCR_CHSEL | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC;
  ADC_DMA_Stream->PAR = CONVERT_PTR_UINT(&ADC_MAIN->DR);
  ADC_DMA_Stream->M0AR = CONVERT_PTR_UINT(&adcValues[FIRST_ANALOG_ADC]);
  ADC_DMA_Stream->NDTR = NUM_ANALOGS_ADC;
  ADC_DMA_Stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmInit();
  }
#endif
}

void adcSingleRead()
{
  ADC_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  ADC_MAIN->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADC_SET_DMA_FLAGS();
  ADC_DMA_Stream->CR |= DMA_SxCR_EN; // Enable DMA
  ADC_MAIN->CR2 |= (uint32_t) ADC_CR2_SWSTART;

  for (unsigned int i = 0; i < 10000; i++) {
    if (ADC_TRANSFER_COMPLETE()) {
      break;
    }
  }
  ADC_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
}
#endif
void adcRead()
{
  uint16_t temp[NUM_ANALOGS] = { 0 };

  for (int i=0; i<4; i++) {
    adcSingleRead();
    for (uint8_t x=FIRST_ANALOG_ADC; x<NUM_ANALOGS; x++) {
      uint16_t val = adcValues[x];
#if defined(JITTER_MEASURE)
      if (JITTER_MEASURE_ACTIVE()) {
        rawJitter[x].measure(val);
      }
#endif
      temp[x] += val;
    }
  }

  for (uint8_t x=FIRST_ANALOG_ADC; x<NUM_ANALOGS; x++) {
    adcValues[x] = temp[x] >> 2;
  }

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmRead(adcValues);
  }
#endif
}

// TODO
void adcStop()
{
}

#if !defined(SIMU)
uint16_t getAnalogValue(uint8_t index)
{
  if (IS_POT(index) && !IS_POT_SLIDER_AVAILABLE(index)) {
    // Use fixed analog value for non-existing and/or non-connected pots.
    // Non-connected analog inputs will slightly follow the adjacent connected analog inputs,
    // which produces ghost readings on these inputs.
    return 0;
  }
  if (ana_direction[index] < 0)
    return 4095 - adcValues[index];
  else
    return adcValues[index];
}
#endif // #if !defined(SIMU)
