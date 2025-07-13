#pragma once

#include <Arduino.h>

struct OLEDInputData
{
    int rssi;
    int rx;
    int tx;
    float batteryVoltage;
    int batteryPercentage;
    float targetBatteryVoltage;
    int targetBatteryPercentage;
    double lat;
    double lon;
    float alt;
    double targetLat;
    double targetLon;
    float targetAlt;
    int velocity;
};

void OLEDInit();
void OLEDUpdateScreen(const OLEDInputData &data);