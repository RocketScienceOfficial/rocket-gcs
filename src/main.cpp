#include "config.h"
#include "radio.h"
#include "gps.h"
#include "oled.h"
#include "state.h"
#include "serial.h"
#include "power.h"

static unsigned long s_PMULastUpdate;
static unsigned long s_OLEDLastUpdate;
static unsigned long s_Now;
static bool s_FirstUpdate;

void setup()
{
  SerialControlInit();
  OLEDInit();
  StateInit();
  GPSInit();
  LoRaInit();
  PMUInit();

  delay(1000);
}

void loop()
{
  SerialControlUpdate();
  StateCheck();
  GPSCheck();
  LoRaCheck();

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
        .targetBatteryVoltage = (float)LoRaGetCurrentFrame()->batteryVoltage10 / 10,
        .targetBatteryPercentage = LoRaGetCurrentFrame()->batteryPercentage,
        .lat = GPSGetLatitude(),
        .lon = GPSGetLongitude(),
        .alt = GPSGetAltitude(),
        .targetLat = LoRaGetCurrentFrame()->lat,
        .targetLon = LoRaGetCurrentFrame()->lon,
        .targetAlt = (float)LoRaGetCurrentFrame()->alt,
    };
    OLEDUpdateScreen(data);
  }

  s_FirstUpdate = true;
}