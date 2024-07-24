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

#define LCD_CMD_RESET            0xE2
#define LCD_CMD_BIAS_1_9         0xA2  // 1/9 bias
#define LCD_CMD_BIAS_1_7         0xA3  // 1/7 bias

#define LCD_CMD_SEG_NORMAL       0xA0
#define LCD_CMD_SEG_INVERSE      0xA1

#define LCD_CMD_COM_NORMAL       0xC0
#define LCD_CMD_COM_INVERSE      0xC8

#define LCD_CMD_REG_RATIO_000    0x20
#define LCD_CMD_REG_RATIO_001    0x21
#define LCD_CMD_REG_RATIO_010    0x22
#define LCD_CMD_REG_RATIO_011    0x23
#define LCD_CMD_REG_RATIO_100    0x24
#define LCD_CMD_REG_RATIO_101    0x25
#define LCD_CMD_REG_RATIO_110    0x26
#define LCD_CMD_REG_RATIO_111    0x27

#define LCD_CMD_EV               0x81
#define LCD_CMD_POWERCTRL_ALL_ON 0x2F

#define LCD_CMD_SET_STARTLINE    0x40
#define LCD_CMD_SET_PAGESTART    0xB0

#define LCD_CMD_SET_COL_LO       0x00
#define LCD_CMD_SET_COL_HI       0x10

#define LCD_CMD_DISPLAY_OFF      0xAE
#define LCD_CMD_DISPLAY_ON       0xAF

#define LCD_CMD_MODE_RAM         0xA4
#define LCD_CMD_MODE_ALLBLACK 	 0xA5


// LCD
// data lines
#define LCD_DATA_GPIO        GPIOE
#define LCD_DATA_GPIO_CLK    RCC_AHBPeriph_GPIOE
// RW
#define LCD_RW_GPIO          GPIOB
#define LCD_RW_GPIO_CLK      RCC_AHBPeriph_GPIOB
#define LCD_RW_PIN           GPIO_Pin_5
// RST
#define LCD_RST_GPIO         GPIOB
#define LCD_RST_GPIO_CLK     RCC_AHBPeriph_GPIOB
#define LCD_RST_PIN          GPIO_Pin_4
// RS
#define LCD_RS_GPIO          GPIOB
#define LCD_RS_GPIO_CLK      RCC_AHBPeriph_GPIOB
#define LCD_RS_PIN           GPIO_Pin_3
// RD
#define LCD_RD_GPIO          GPIOD
#define LCD_RD_GPIO_CLK      RCC_AHBPeriph_GPIOD
#define LCD_RD_PIN           GPIO_Pin_7
// CS
#define LCD_CS_GPIO          GPIOD
#define LCD_CS_GPIO_CLK      RCC_AHBPeriph_GPIOD
#define LCD_CS_PIN           GPIO_Pin_2

#define LCD_RW_HI()   { LCD_RW_GPIO->BSRR = (LCD_RW_PIN);  }
#define LCD_RW_LO()   { LCD_RW_GPIO->BRR  = (LCD_RW_PIN); }

#define LCD_RST_HI()  { LCD_RST_GPIO->BSRR = (LCD_RST_PIN); }
#define LCD_RST_LO()  { LCD_RST_GPIO->BRR  = (LCD_RST_PIN); }

#define LCD_RS_HI()   { LCD_RS_GPIO->BSRR = (LCD_RS_PIN); }
#define LCD_RS_LO()   { LCD_RS_GPIO->BRR  = (LCD_RS_PIN); }

#define LCD_RD_HI()   { LCD_RD_GPIO->BSRR = (LCD_RD_PIN); }
#define LCD_RD_LO()   { LCD_RD_GPIO->BRR  = (LCD_RD_PIN); }

#define LCD_CS_HI()   { LCD_CS_GPIO->BSRR = (LCD_CS_PIN); }
#define LCD_CS_LO()   { LCD_CS_GPIO->BRR  = (LCD_CS_PIN); }

#define LCD_DATA_SET(data) { *(volatile uint8_t *)&LCD_DATA_GPIO->ODR = (data); }

static void lcdReset(void);

const static unsigned char lcdInitSequence[] =
{
	0xE2, // LCD_CMD_RESET,
	0xAE, // LCD_CMD_DISPLAY_OFF,
  0xA4, // LCD_CMD_MODE_RAM,
	0xA3, // LCD_CMD_BIAS_1_7,
	0xC0, // LCD_CMD_COM_NORMAL,
	0xA1, // LCD_CMD_SEG_INVERSE,
	0x2F, // LCD_CMD_POWERCTRL_ALL_ON,
	0x23, // LCD_CMD_REG_RATIO_011,
	0x81, // LCD_CMD_EV,
	0x38, // LCD_CONTRAST_DEFAULT,
	0x40, // LCD_CMD_SET_STARTLINE,
  0xB0, // LCD_CMD_SET_PAGESTART,
	0x00, // LCD_CMD_SET_COL_LO,
  0x10, // LCD_CMD_SET_COL_HI,
	0xAF, // LCD_CMD_DISPLAY_ON
};

static void lcdSendCtl(uint8_t data) {
    LCD_RS_LO(); //command
    LCD_DATA_SET(data);
    LCD_RD_HI();
    LCD_RD_LO();
}

static void lcdSendGFX(uint8_t data) {
//    LCD_RS_HI(); //gfx, done on transaction start
    LCD_DATA_SET(data);
    LCD_RD_HI();
    LCD_RD_LO();
}

void lcdInit() {
  // data
  GPIO_InitTypeDef gpio_init_e = {
    .GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7,
    .GPIO_Mode = GPIO_Mode_OUT,
    .GPIO_Speed = GPIO_Speed_Level_3, // 50MHz
    .GPIO_OType = GPIO_OType_PP,      // Push/Pull
    .GPIO_PuPd = GPIO_PuPd_NOPULL,    // none
  };
  GPIO_Init(GPIOE, &gpio_init_e);

  // RS, RST, RW
  GPIO_InitTypeDef gpio_init_b = {
    .GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5,
    .GPIO_Mode = GPIO_Mode_OUT,
    .GPIO_Speed = GPIO_Speed_Level_3, // 50MHz
    .GPIO_OType = GPIO_OType_PP,      // Push/Pull
    .GPIO_PuPd = GPIO_PuPd_NOPULL,    // none
  };
  GPIO_Init(GPIOB, &gpio_init_b);

  // CS, RD
  GPIO_InitTypeDef gpio_init_d = {
    .GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_7,
    .GPIO_Mode = GPIO_Mode_OUT,
    .GPIO_Speed = GPIO_Speed_Level_3, // 50MHz
    .GPIO_OType = GPIO_OType_PP,      // Push/Pull
    .GPIO_PuPd = GPIO_PuPd_NOPULL,    // none
  };
  GPIO_Init(GPIOD, &gpio_init_d);

  LCD_RST_LO();
  LCD_CS_LO(); // Enable access to LCD
  LCD_RW_LO(); // Enable write mode
  LCD_RD_HI();
  LCD_RS_HI();
  lcdReset();
  lcdClear();
  lcdRefresh();
}

void lcdRefresh() {
  uint8_t *p = displayBuf;
  int page = 0xB0;
  do {
    uint8_t line = LCD_W;
    lcdSendCtl(page); // page selection
    lcdSendCtl(4);    // start at line 4 -> controller is 132 lines while lcd 128
    lcdSendCtl(0x10u);// column address 0
    LCD_RS_HI(); // start gfx data
    do {
      lcdSendGFX(*p++);
    } while (--line);
    ++page;
  } while (page < 0xB8);
}

void lcdOff() {
    // switch display off
    lcdSendCtl(LCD_CMD_DISPLAY_OFF);
    // all pixels on
    lcdSendCtl(LCD_CMD_MODE_ALLBLACK);
}

void lcdSetRefVolt(uint8_t val){
    limit<uint8_t>(LCD_CONTRAST_MIN, val, LCD_CONTRAST_MAX);
    lcdSendCtl(LCD_CMD_EV);
    lcdSendCtl(val);
}

void lcdReset() {
    // wait for voltages to be stable
    delay_ms(20);
    LCD_RST_LO();
    delay_us(20);  // at least 5us
    LCD_RST_HI();
    delay_us(20); // at least 5us?
    for (uint8_t i=0; i<sizeof(lcdInitSequence); i++) {
      lcdSendCtl(lcdInitSequence[i]);
    }
}
