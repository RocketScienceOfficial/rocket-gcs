#pragma once

struct MeasurementData
{
    float roll;
    float pitch;
    float yaw;
    float velocity;
    float batteryVoltage;
    int batteryPercentage;
    double latitude;
    double longitude;
    float altitude;
    int signalStrength;
    int packetLoss;
};

void SetMeasurementData(MeasurementData *data);
MeasurementData GetCurrentMeasurement();