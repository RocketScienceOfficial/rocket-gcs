#pragma once

struct RadioOBCData
{
    float qw;
    float qx;
    float qy;
    float qz;
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
    bool arm_enable;
    bool arm_disable;
    bool v3v3_enable;
    bool v3v3_disable;
    bool v5_enable;
    bool v5_disable;
    bool vbat_enable;
    bool vbat_disable;
};

void LoRaInit();
void LoRaCheck();
int LoRaGetRssi();
int LoRaGetRX();
int LoRaGetTX();
RadioOBCData LoRaGetCurrentOBCData();
RadioTLMData *LoRaGetCurrentTLMData();