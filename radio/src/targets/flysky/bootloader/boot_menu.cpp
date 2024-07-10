#include "../../common/arm/stm32/bootloader/boot.h"
#include "opentx.h"
#if defined(SDCARD)
#include "../../common/arm/stm32/bootloader/bin_files.h"

extern MemoryType memoryType;
#endif
void bootloaderInitScreen() {
  backlightEnable();
}

static void bootloaderDrawMsg(unsigned int x, const char* str, uint8_t line, bool inverted) {
  lcdDrawSizedText(x, (line + 2) * FH, str, DISPLAY_CHAR_WIDTH, inverted ? INVERS : 0);
}

void bootloaderDrawFilename(const char* str, uint8_t line, bool selected) {
  bootloaderDrawMsg(INDENT_WIDTH, str, line, selected);
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str) {
  lcdClear();
  lcdDrawText(0, 0, BOOTLOADER_TITLE);
  lcdInvertLine(0);

  if (st == ST_START) {
#if defined(SDCARD)
    lcdDrawText(FW, 2*FH, "Write Firmware", opt == 0 ? INVERS : 0);
    lcdDrawText(FW, 3*FH, "Restore EEPROM", opt == 0 ? INVERS : 0);
    lcdDrawText(FW, 4*FH, "Exit");
#else
    lcdDrawText(FW, 4*FH, "Exit", opt == 0 ? INVERS : 0); // only Exit option on PCBI6X
#endif

    lcdDrawText(FW, 5 * FH + FH / 2, STR_OR_PLUGIN_USB_CABLE);

    // Remove "opentx-" from string
    const char* vers = getOtherVersion();
    if (strstr(vers, "opentx-"))
      vers += 7;

    lcdDrawText(FW, 7 * FH, vers);
    lcdInvertLine(7);
  } else if (st == ST_USB) {
    lcdDrawText(0, 4 * FH, STR_USB_CONNECTED);
  } else if (st == ST_DIR_CHECK) {
#if defined(SDCARD)
    if (opt == FR_NO_PATH) {
      bootloaderDrawMsg(INDENT_WIDTH, "Directory is missing!", 1, false);
      bootloaderDrawMsg(INDENT_WIDTH, getBinaryPath(memoryType), 2, false);
    } else {
      bootloaderDrawMsg(INDENT_WIDTH, "Directory is empty!", 1, false);
    }
#endif
#if defined(SDCARD)
  } else if (st == ST_FLASH_CHECK) {
    if (opt == FC_ERROR) {
      if (memoryType == MEM_FLASH)
        bootloaderDrawMsg(0, STR_INVALID_FIRMWARE, 2, false);
      else
        bootloaderDrawMsg(0, STR_INVALID_EEPROM, 2, false);
    } else if (opt == FC_OK) {
      const char* vers = getOtherVersion((char*)Block_buffer);
      // Remove opentx- from string
      if (strstr(vers, "opentx-"))
        vers = vers + 7;
      bootloaderDrawMsg(INDENT_WIDTH, vers, 0, false);
    }
    bootloaderDrawMsg(0, STR_HOLD_ENTER_TO_START, 2, false);
#endif
  } else if (st == ST_FLASHING) {
    lcdDrawTextAlignedLeft(4 * FH, CENTER "\015Writing...");

    lcdDrawRect(3, 6 * FH + 4, (LCD_W - 8), 7);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 6, (LCD_W - 12) * opt / 100, FORCE);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 7, (LCD_W - 12) * opt / 100, FORCE);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 8, (LCD_W - 12) * opt / 100, FORCE);
  } else if (st == ST_FLASH_DONE) {
    lcdDrawTextAlignedLeft(4 * FH, CENTER "\007Writing complete");
  }
}
