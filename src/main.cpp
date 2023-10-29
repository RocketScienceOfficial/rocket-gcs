#include <Arduino.h>

#include "config.h"
#include "radio.h"
#include "gps.h"
#include "oled.h"
#include "state.h"
#include "measurements.h"
#include "power.h"

static unsigned long s_LastUpdate;
static unsigned long s_Now;

static void UpdateScreen();

void setup()
{
  Serial.begin(115200);
  Serial.println("Initialized Board!");

  GPSInit();
  LoRaInit();
  OLEDInit();
  StateInit();
  PMUInit();

  delay(1000);
}

void loop()
{
  GPSCheck();
  LoRaCheck();
  StateCheck();
  PMURead();

  s_Now = millis();

  if (s_Now - s_LastUpdate >= OLED_UPDATE_INTERVAL || StateChanged())
  {
    s_LastUpdate = s_Now;

    UpdateScreen();
  }
}

static void UpdateScreen()
{
  OLEDInputData data = {
      .rssi = LoRaGetRssi(),
      .rx = LoRaGetRX(),
      .tx = LoRaGetTX(),
      .batteryVoltage = PMUGetCurrentData().batteryVoltage,
      .batteryPercentage = PMUGetCurrentData().batteryPercentage,
      .lat = GPSGetLatitude(),
      .lon = GPSGetLongitude(),
      .alt = GPSGetAltitude(),
      .targetLat = GetCurrentMeasurement().latitude,
      .targetLon = GetCurrentMeasurement().longitude,
      .targetAlt = GetCurrentMeasurement().altitude,
  };

  OLEDUpdateScreen(data);
}