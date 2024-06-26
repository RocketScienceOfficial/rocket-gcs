#include <Arduino.h>

#include "config.h"
#include "radio.h"
#include "gps.h"
#include "oled.h"
#include "state.h"
#include "router.h"
#include "power.h"

static unsigned long s_PMULastUpdate;
static unsigned long s_OLEDLastUpdate;
static unsigned long s_Now;
static bool s_FirstUpdate;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initialized Board!");

  RouterInit();
  OLEDInit();
  StateInit();
  GPSInit();
  LoRaInit();
  PMUInit();

  delay(1000);
}

void loop()
{
  StateCheck();
  GPSCheck();
  LoRaCheck();
  RouterUpdate();

  s_Now = millis();

  if (s_Now - s_PMULastUpdate >= PMU_UPDATE_INTERVAL || s_FirstUpdate)
  {
    s_PMULastUpdate = s_Now;

    PMURead();
  }

  if (s_Now - s_OLEDLastUpdate >= OLED_UPDATE_INTERVAL || StateChanged() || s_FirstUpdate)
  {
    s_OLEDLastUpdate = s_Now;

    OLEDInputData data = {
        .rssi = LoRaGetRssi(),
        .rx = LoRaGetRX(),
        .tx = LoRaGetTX(),
        .batteryVoltage = PMUGetCurrentData().batteryVoltage,
        .batteryPercentage = PMUGetCurrentData().batteryPercentage,
        .targetBatteryVoltage = LoRaGetCurrentOBCData().batteryVoltage,
        .targetBatteryPercentage = LoRaGetCurrentOBCData().batteryPercentage,
        .lat = GPSGetLatitude(),
        .lon = GPSGetLongitude(),
        .alt = GPSGetAltitude(),
        .targetLat = LoRaGetCurrentOBCData().latitude,
        .targetLon = LoRaGetCurrentOBCData().longitude,
        .targetAlt = (float)LoRaGetCurrentOBCData().altitude,
    };
    OLEDUpdateScreen(data);
  }

  s_FirstUpdate = true;
}