#pragma once

#include <Arduino.h>

struct OLEDInputData
{
    int rssi;
    int rx;
    int tx;
    float batteryVoltage;
    int batteryPercentage;
    float lat;
    float lon;
    float alt;
    float targetLat;
    float targetLon;
    float targetAlt;
};

void OLEDInit();
void OLEDUpdateScreen(const OLEDInputData &data);