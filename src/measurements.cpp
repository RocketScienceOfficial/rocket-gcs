#include "measurements.h"
#include <Arduino.h>

static MeasurementData s_CurrentMeasurement;

void SetMeasurementData(MeasurementData *data)
{
    Serial.print("/*");
    Serial.printf("%.2f,", data->roll);
    Serial.printf("%.2f,", data->pitch);
    Serial.printf("%.2f,", data->yaw);
    Serial.printf("%.2f,", data->velocity);
    Serial.printf("%.2f,", data->batteryVoltage);
    Serial.printf("%d,", data->batteryPercentage);
    Serial.printf("%.7f,", data->latitude);
    Serial.printf("%.7f,", data->longitude);
    Serial.printf("%.2f,", data->altitude);
    Serial.printf("%d,", data->signalStrength);
    Serial.printf("%d,", data->packetLoss);
    Serial.println("*/");

    s_CurrentMeasurement = *data;
}

MeasurementData GetCurrentMeasurement()
{
    return s_CurrentMeasurement;
}