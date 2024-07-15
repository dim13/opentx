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

void frskyDProcessPacket(const uint8_t *packet);

#if defined(TELEMETRY_FRSKY)
TEST(FrSky, TelemetryValueWithMinAveraging)
{
  /*
    The following expected[] array is filled
    with values that correspond to 4 elements
    long averaging buffer.
    If length of averaging buffer is changed, this
    values must be adjusted
  */
  uint8_t expected[] = { 10, 12, 17, 25, 35, 45, 55, 65, 75, 85, 92, 97, 100, 100, 100, 100, 100};
  int testPos = 0;
  //test of averaging
  TelemetryValueWithMin testVal;
  testVal.value = 0;
  testVal.set(10);
  EXPECT_EQ(RAW_FRSKY_MINMAX(testVal), 10);
  EXPECT_EQ(testVal.value, expected[testPos++]);
  for(int n=2; n<10; ++n) {
    testVal.set(n*10);
    EXPECT_EQ(RAW_FRSKY_MINMAX(testVal), n*10);
    EXPECT_EQ(testVal.value, expected[testPos++]);
  }
  for(int n=2; n<10; ++n) {
    testVal.set(100);
    EXPECT_EQ(RAW_FRSKY_MINMAX(testVal), 100);
    EXPECT_EQ(testVal.value, expected[testPos++]);
  }
}

TEST(FrSky, Vfas_0x39_HiPrecision)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  EXPECT_EQ(telemetryItems[0].value, 0);

  allowNewSensors = true;

  // normal precision, resolution 0.1V
  processHubPacket(VFAS_ID, 1234);  // set value of 123.4V
  EXPECT_EQ(telemetryItems[0].value, 12340);      // stored value has resolution of 0.01V

  // now high precision, resolution 0.01V
  processHubPacket(VFAS_ID, VFAS_D_HIPREC_OFFSET);  // set value of 0V
  EXPECT_EQ(telemetryItems[0].value, 0);
  processHubPacket(VFAS_ID, VFAS_D_HIPREC_OFFSET + 12345);  // set value of 123.45V
  EXPECT_EQ(telemetryItems[0].value, 12345);
  processHubPacket(VFAS_ID, VFAS_D_HIPREC_OFFSET + 30012);  // set value of 300.12V
  EXPECT_EQ(telemetryItems[0].value, 30012);
}

TEST(FrSky, HubAltNegative)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  EXPECT_EQ(telemetryItems[0].value, 0);

  allowNewSensors = true;

  // altimeter auto offset
  processHubPacket(BARO_ALT_BP_ID, 0);
  processHubPacket(BARO_ALT_AP_ID, 0);
  EXPECT_EQ(telemetryItems[0].value, 0);

  // low precision altimeter, bp always less than 10
  processHubPacket(BARO_ALT_BP_ID, 12);  // set value of 12.3m
  processHubPacket(BARO_ALT_AP_ID, 3);
  EXPECT_EQ(telemetryItems[0].value, 123);      // altitude stored has resolution of 0.1m

  processHubPacket(BARO_ALT_BP_ID, -12);  // set value of -12.3m
  processHubPacket(BARO_ALT_AP_ID, 3);
  EXPECT_EQ(telemetryItems[0].value, -123);

  // hi precision altimeter, bp can be two decimals
  MODEL_RESET();
  TELEMETRY_RESET();

  // altimeter auto offset
  processHubPacket(BARO_ALT_BP_ID, 0);
  processHubPacket(BARO_ALT_AP_ID, 0);
  EXPECT_EQ(telemetryItems[0].value, 0);

  // first trigger hi precision, by setting AP above 9
  processHubPacket(BARO_ALT_BP_ID, -1);  // set value of -1.35m
  processHubPacket(BARO_ALT_AP_ID, 35);
  EXPECT_EQ(telemetryItems[0].value, -13);

  processHubPacket(BARO_ALT_BP_ID, 12);  // set value of 12.35m
  processHubPacket(BARO_ALT_AP_ID, 35);
  EXPECT_EQ(telemetryItems[0].value, 123);

  // now test with the AP less than 10 to check if hiprecision is still active
  processHubPacket(BARO_ALT_BP_ID, 12);  // set value of 12.05m
  processHubPacket(BARO_ALT_AP_ID, 05);
  EXPECT_EQ(telemetryItems[0].value, 120);
}

TEST(FrSky, Gps)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  allowNewSensors = true;

  EXPECT_EQ(telemetryItems[0].value, 0);

  // latitude 15 degrees north, 30.5000 minutes = 15.508333333333333 degrees
  processHubPacket(GPS_LAT_BP_ID, 1530);  // DDDMM.
  processHubPacket(GPS_LAT_AP_ID, 5000);  // .MMMM
  processHubPacket(GPS_LAT_NS_ID, 'N');

  // longitude 45 degrees west, 20.5000 minutes = 45.34166666666667 degrees
  processHubPacket(GPS_LONG_BP_ID, 4520);
  processHubPacket(GPS_LONG_AP_ID, 5000);
  processHubPacket(GPS_LONG_EW_ID, 'E');

  EXPECT_EQ(telemetryItems[0].gps.latitude, 15508333);
  EXPECT_EQ(telemetryItems[0].gps.longitude, 45341666);
}

#endif // defined(TELEMETRY_FRSKY)
