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

static void _SubmitCMD();
static void _Reset();

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
                _Reset();
            }
            else
            {
                s_CMD[s_CurrentSize] = '\0';

                _SubmitCMD();
                _Reset();
            }
        }
        else
        {
            if ((c >= (int)'0' && c <= (int)'9') || (c >= (int)'A' && c <= (int)'Z') || (c >= (int)'a' && c <= (int)'z') || (c == (int)'-') || (c == (int)'_') || (c == (int)'\\'))
            {
                if (s_CurrentSize >= sizeof(s_CMD))
                {
                    _Reset();
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
    LOGGER.printf("%.1f,", data.roll);
    LOGGER.printf("%.1f,", data.pitch);
    LOGGER.printf("%.1f,", data.yaw);
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

static void _SubmitCMD()
{
    if (strcmp(s_CMD, "\\arm") == 0)
    {
        LoRaGetCurrentTLMData()->armed = true;
    }
    else if (strcmp(s_CMD, "\\set-voltage-3v3") == 0)
    {
        LoRaGetCurrentTLMData()->v3v3 = true;
    }
    else if (strcmp(s_CMD, "\\set-voltage-5v") == 0)
    {
        LoRaGetCurrentTLMData()->v5 = true;
    }
    else if (strcmp(s_CMD, "\\set-voltage-vbat") == 0)
    {
        LoRaGetCurrentTLMData()->vbat = true;
    }
}

static void _Reset()
{
    memset(s_CMD, 0, sizeof(s_CMD));

    s_CurrentSize = 0;
}