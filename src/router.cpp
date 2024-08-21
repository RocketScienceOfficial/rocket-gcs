#include "router.h"
#include "radio.h"
#include "config.h"
#include <Arduino.h>
#include <string.h>

#ifdef BT_NAME
#include <BluetoothSerial.h>

static BluetoothSerial SerialBT;

#define LOGGER SerialBT
#else
#define LOGGER Serial
#endif

static char s_CMD[128];
static size_t s_CurrentSize;

static void SubmitCMD();
static void Reset();

void RouterInit()
{
#ifdef BT_NAME
    SerialBT.begin(BT_NAME);
#endif
}

void RouterUpdate()
{
    if (LOGGER.available())
    {
        int c = LOGGER.read();

        if (c == (int)'\r')
        {
            if (s_CurrentSize >= sizeof(s_CMD))
            {
                Reset();
            }
            else
            {
                s_CMD[s_CurrentSize] = '\0';

                SubmitCMD();
                Reset();
            }
        }
        else
        {
            if ((c >= (int)'0' && c <= (int)'9') || (c >= (int)'A' && c <= (int)'Z') || (c >= (int)'a' && c <= (int)'z') || (c == (int)'-') || (c == (int)'_') || (c == (int)'\\'))
            {
                if (s_CurrentSize >= sizeof(s_CMD))
                {
                    Reset();
                }
                else
                {
                    s_CMD[s_CurrentSize++] = (char)c;
                }
            }
        }
    }
}

void RouterSendData()
{
    RadioOBCData data = LoRaGetCurrentOBCData();

    LOGGER.print("/*");
    LOGGER.printf("%.1f,", data.qw);
    LOGGER.printf("%.1f,", data.qx);
    LOGGER.printf("%.1f,", data.qy);
    LOGGER.printf("%.1f,", data.qz);
    LOGGER.printf("%.1f,", data.velocity);
    LOGGER.printf("%.1f,", data.batteryVoltage);
    LOGGER.printf("%d,", data.batteryPercentage);
    LOGGER.printf("%.7f,", data.latitude);
    LOGGER.printf("%.7f,", data.longitude);
    LOGGER.printf("%d,", data.altitude);
    LOGGER.printf("%d,", data.state);
    LOGGER.printf("%d,", data.controlFlags);
    LOGGER.printf("%d,", data.signalStrength);
    LOGGER.printf("%d,", data.packetLoss);
    LOGGER.println("*/");
}

static void SubmitCMD()
{
    Serial.println("Submiting command...");

    if (strcmp(s_CMD, "\\arm-enable") == 0)
    {
        LoRaGetCurrentTLMData()->arm_enable = true;
    }
    else if (strcmp(s_CMD, "\\arm-disable") == 0)
    {
        LoRaGetCurrentTLMData()->arm_disable = true;
    }
    else if (strcmp(s_CMD, "\\voltage-3v3-enable") == 0)
    {
        LoRaGetCurrentTLMData()->v3v3_enable = true;
    }
    else if (strcmp(s_CMD, "\\voltage-3v3-disable") == 0)
    {
        LoRaGetCurrentTLMData()->v3v3_disable = true;
    }
    else if (strcmp(s_CMD, "\\voltage-5v-enable") == 0)
    {
        LoRaGetCurrentTLMData()->v5_enable = true;
    }
    else if (strcmp(s_CMD, "\\voltage-5v-disable") == 0)
    {
        LoRaGetCurrentTLMData()->v5_disable = true;
    }
    else if (strcmp(s_CMD, "\\voltage-vbat-enable") == 0)
    {
        LoRaGetCurrentTLMData()->vbat_enable = true;
    }
    else if (strcmp(s_CMD, "\\voltage-vbat-disable") == 0)
    {
        LoRaGetCurrentTLMData()->vbat_disable = true;
    }
}

static void Reset()
{
    memset(s_CMD, 0, sizeof(s_CMD));

    s_CurrentSize = 0;
}