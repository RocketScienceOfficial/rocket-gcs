#include "measurements.h"
#include "config.h"
#include <Arduino.h>
#include <BluetoothSerial.h>

static BluetoothSerial SerialBT;
static MeasurementData s_CurrentMeasurement;

void MeasInit()
{
    SerialBT.begin(BT_NAME);
}

void SetMeasurementData(MeasurementData *data)
{
    SerialBT.print("/*");
    SerialBT.printf("%.1f,", data->roll);
    SerialBT.printf("%.1f,", data->pitch);
    SerialBT.printf("%.1f,", data->yaw);
    SerialBT.printf("%.1f,", data->velocity);
    SerialBT.printf("%.1f,", data->batteryVoltage);
    SerialBT.printf("%d,", data->batteryPercentage);
    SerialBT.printf("%.7f,", data->latitude);
    SerialBT.printf("%.7f,", data->longitude);
    SerialBT.printf("%d,", data->altitude);
    SerialBT.printf("%d,", data->state);
    SerialBT.printf("%d,", data->signalStrength);
    SerialBT.printf("%d,", data->packetLoss);
    SerialBT.println("*/");

    s_CurrentMeasurement = *data;
}

MeasurementData GetCurrentMeasurement()
{
    return s_CurrentMeasurement;
}