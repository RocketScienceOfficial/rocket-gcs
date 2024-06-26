#pragma once

struct RadioOBCData
{
    float roll;
    float pitch;
    float yaw;
    float velocity;
    float batteryVoltage;
    int batteryPercentage;
    double latitude;
    double longitude;
    int altitude;
    int state;
    int controlFlags;
    int signalStrength;
    int packetLoss;
};

struct RadioTLMData
{
    bool armed;
    bool v3v3;
    bool v5;
    bool vbat;
};

void LoRaInit();
void LoRaCheck();
int LoRaGetRssi();
int LoRaGetRX();
int LoRaGetTX();
RadioOBCData LoRaGetCurrentOBCData();
RadioTLMData *LoRaGetCurrentTLMData();