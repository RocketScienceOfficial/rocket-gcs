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

  if (s_Now - s_PMULastUpdate >= PMU_UPDATE_INTERVAL)
  {
    s_PMULastUpdate = s_Now;

    PMURead();
  }

  if (s_Now - s_OLEDLastUpdate >= OLED_UPDATE_INTERVAL || StateChanged())
  {
    s_OLEDLastUpdate = s_Now;

    OLEDUpdateScreen();
  }
}