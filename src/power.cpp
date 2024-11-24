#include "power.h"
#include "config.h"
#include "serial.h"
#include "XPowersLib.h"
#include <Arduino.h>
#include <Wire.h>

static PMUData s_CurrentData;
static XPowersLibInterface *s_PMU;

// REF: https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/examples/PMU/boards.h
void PMUInit()
{
    if (!s_PMU)
    {
        s_PMU = new XPowersAXP2101(Wire);

        if (!s_PMU->init())
        {
            SERIAL_DEBUG_PRINTF("Warning: Failed to find AXP2101 power management\n");

            delete s_PMU;

            s_PMU = NULL;
        }
        else
        {
            SERIAL_DEBUG_PRINTF("AXP2101 PMU init succeeded, using AXP2101 PMU\n");
        }
    }

    if (!s_PMU)
    {
        return;
    }

    s_PMU->setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);

    s_PMU->disablePowerOutput(XPOWERS_DCDC2);
    s_PMU->disablePowerOutput(XPOWERS_DCDC3);
    s_PMU->disablePowerOutput(XPOWERS_DCDC4);
    s_PMU->disablePowerOutput(XPOWERS_DCDC5);
    s_PMU->disablePowerOutput(XPOWERS_ALDO1);
    s_PMU->disablePowerOutput(XPOWERS_ALDO4);
    s_PMU->disablePowerOutput(XPOWERS_BLDO1);
    s_PMU->disablePowerOutput(XPOWERS_BLDO2);
    s_PMU->disablePowerOutput(XPOWERS_DLDO1);
    s_PMU->disablePowerOutput(XPOWERS_DLDO2);

    s_PMU->setPowerChannelVoltage(XPOWERS_VBACKUP, 3300);
    s_PMU->enablePowerOutput(XPOWERS_VBACKUP);

    s_PMU->setProtectedChannel(XPOWERS_DCDC1);

    s_PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
    s_PMU->enablePowerOutput(XPOWERS_ALDO2);

    s_PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
    s_PMU->enablePowerOutput(XPOWERS_ALDO3);

    s_PMU->enableSystemVoltageMeasure();
    s_PMU->enableVbusVoltageMeasure();
    s_PMU->enableBattVoltageMeasure();

    s_PMU->setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);
}

void PMURead()
{
    if (!s_PMU)
    {
        return;
    }

    s_CurrentData.isCharging = s_PMU->isCharging();
    s_CurrentData.isDischarge = s_PMU->isDischarge();
    s_CurrentData.isVBusIn = s_PMU->isVbusIn();
    s_CurrentData.isBatteryConnected = s_PMU->isBatteryConnect();
    s_CurrentData.batteryVoltage = s_PMU->getBattVoltage() / 1000.0f;
    s_CurrentData.VBusVoltage = s_PMU->getVbusVoltage() / 1000.0f;
    s_CurrentData.systemVoltage = s_PMU->getSystemVoltage() / 1000.0f;
    s_CurrentData.batteryPercentage = s_CurrentData.isBatteryConnected ? s_PMU->getBatteryPercent() : 0;
}

const PMUData &PMUGetCurrentData()
{
    return s_CurrentData;
}