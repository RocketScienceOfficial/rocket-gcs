#include "measurements.h"
#include <Arduino.h>

static MeasurementData s_CurrentMeasurement;

void SetMeasurementData(MeasurementData *data)
{
    Serial.print("/*");
    Serial.printf("%.1f,", data->roll);
    Serial.printf("%.1f,", data->pitch);
    Serial.printf("%.1f,", data->yaw);
    Serial.printf("%.1f,", data->velocity);
    Serial.printf("%.1f,", data->batteryVoltage);
    Serial.printf("%d,", data->batteryPercentage);
    Serial.printf("%.7f,", data->latitude);
    Serial.printf("%.7f,", data->longitude);
    Serial.printf("%d,", data->altitude);
    Serial.printf("%d,", data->state);
    Serial.printf("%d,", data->signalStrength);
    Serial.printf("%d,", data->packetLoss);
    Serial.println("*/");

    s_CurrentMeasurement = *data;
}

MeasurementData GetCurrentMeasurement()
{
    return s_CurrentMeasurement;
}