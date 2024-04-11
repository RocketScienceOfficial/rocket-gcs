#pragma once

struct MeasurementData
{
    float positionX;
    float positionY;
    float positionZ;
    float roll;
    float pitch;
    float yaw;
    double latitude;
    double longitude;
    float altitude;
    float velocity;
    float batteryVoltage;
    int batteryPercentage;
    float pressure;
    float temperature;
    int signalStrength;
    int packetLoss;
};

void SetMeasurementData(MeasurementData *data);
MeasurementData GetCurrentMeasurement();