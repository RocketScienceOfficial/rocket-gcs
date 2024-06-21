#include "measurements.h"
#include "config.h"
#include <Arduino.h>

#ifdef BT_NAME
#include <BluetoothSerial.h>

static BluetoothSerial SerialBT;

#define LOGGER SerialBT
#else
#define LOGGER Serial
#endif

static MeasurementData s_CurrentMeasurement;

void MeasInit()
{
#ifdef BT_NAME
    SerialBT.begin(BT_NAME);
#endif
}

void SetMeasurementData(MeasurementData *data)
{
    LOGGER.print("/*");
    LOGGER.printf("%.1f,", data->roll);
    LOGGER.printf("%.1f,", data->pitch);
    LOGGER.printf("%.1f,", data->yaw);
    LOGGER.printf("%.1f,", data->velocity);
    LOGGER.printf("%.1f,", data->batteryVoltage);
    LOGGER.printf("%d,", data->batteryPercentage);
    LOGGER.printf("%.7f,", data->latitude);
    LOGGER.printf("%.7f,", data->longitude);
    LOGGER.printf("%d,", data->altitude);
    LOGGER.printf("%d,", data->state);
    LOGGER.printf("%d,", data->signalStrength);
    LOGGER.printf("%d,", data->packetLoss);
    LOGGER.println("*/");

    s_CurrentMeasurement = *data;
}

MeasurementData GetCurrentMeasurement()
{
    return s_CurrentMeasurement;
}