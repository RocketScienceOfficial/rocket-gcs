#include "radio.h"
#include "config.h"
#include "router.h"
#include "maths.h"
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#define RADIO_TLM_DATA_SEND_DELAY 250
#define RADIO_MAGIC 0x7B

typedef struct __attribute__((__packed__)) radio_obc_frame
{
    uint8_t magic;
    float qw;
    float qx;
    float qy;
    float qz;
    uint16_t velocity;
    uint8_t batteryVoltage10;
    uint8_t batteryPercentage;
    double lat;
    double lon;
    uint16_t alt;
    uint8_t state;
    uint8_t controlFlags;
    uint8_t seq;
    uint16_t crc;
} radio_obc_frame_t;

typedef enum radio_tlm_flags
{
    RADIO_TLM_FLAG_ARM_ENABLE = 1 << 0,
    RADIO_TLM_FLAG_ARM_DISABLE = 1 << 1,
    RADIO_TLM_FLAG_3V3_ENABLE = 1 << 2,
    RADIO_TLM_FLAG_3V3_DISABLE = 1 << 3,
    RADIO_TLM_FLAG_5V_ENABLE = 1 << 4,
    RADIO_TLM_FLAG_5V_DISABLE = 1 << 5,
    RADIO_TLM_FLAG_VBAT_ENABLE = 1 << 6,
    RADIO_TLM_FLAG_VBAT_DISABLE = 1 << 7,
} radio_tlm_flags_t;

typedef struct __attribute__((__packed__)) radio_tlm_frame
{
    uint8_t magic;
    uint8_t flags;
    uint16_t crc;
} radio_tlm_frame_t;

static RadioOBCData s_CurrentOBCData;
static RadioTLMData s_CurrentTLMData;
static int s_Rssi;
static int s_RX;
static int s_TX;
static uint8_t s_CurrentSeq;
static int s_PacketsLost;
static unsigned long s_SendTLMDataTimeOffset;

static void TryParsePacket(uint8_t *buffer, size_t len);
static bool ValidatePacket(const radio_obc_frame_t *packet);
static void SendTLMPacket();

void LoRaInit()
{
    Serial.println("Starting LoRa...");

    SPI.begin(LORA_SCLK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN);

    delay(1000);

    LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    if (!LoRa.begin(LORA_FREQ))
    {
        Serial.println("Starting LoRa failed!");

        while (1)
            ;
    }

    LoRa.setFrequency(LORA_FREQ);
    LoRa.setSignalBandwidth(LORA_BAND);
    LoRa.setSpreadingFactor(8);
    LoRa.receive();

    Serial.println("Starting LoRa success!");
}

void LoRaCheck()
{
    int packetSize = LoRa.parsePacket();

    if (packetSize)
    {
        Serial.print("Received packet with size: ");
        Serial.print(packetSize);
        Serial.println(" bytes");

        uint8_t buffer[512];
        size_t i = 0;

        while (LoRa.available())
        {
            if (i == packetSize)
            {
                Serial.println("Something went wrong while parsing packet!");

                return;
            }

            if (i < sizeof(buffer))
            {
                buffer[i++] = (uint8_t)LoRa.read();
            }
            else
            {
                Serial.println("Buffer overflow while parsing packet!");

                return;
            }
        }

        TryParsePacket(buffer, i);
    }

    if (s_SendTLMDataTimeOffset != 0 && millis() - s_SendTLMDataTimeOffset >= RADIO_TLM_DATA_SEND_DELAY)
    {
        s_SendTLMDataTimeOffset = 0;

        SendTLMPacket();
    }
}

int LoRaGetRssi()
{
    return s_Rssi;
}

int LoRaGetRX()
{
    return s_RX;
}

int LoRaGetTX()
{
    return s_TX;
}

RadioOBCData LoRaGetCurrentOBCData()
{
    return s_CurrentOBCData;
}

RadioTLMData *LoRaGetCurrentTLMData()
{
    return &s_CurrentTLMData;
}

static void TryParsePacket(uint8_t *buffer, size_t len)
{
    if (len != sizeof(radio_obc_frame_t))
    {
        Serial.println("Invalid packet length!");

        return;
    }

    radio_obc_frame_t *frame = (radio_obc_frame_t *)buffer;

    if (!ValidatePacket(frame))
    {
        Serial.println("Couldn't deserialize packet!");

        return;
    }

    s_Rssi = LoRa.packetRssi();
    s_RX++;

    if (frame->seq != s_CurrentSeq)
    {
        s_PacketsLost += frame->seq > s_CurrentSeq ? frame->seq - s_CurrentSeq : 256 + frame->seq - s_CurrentSeq;
        s_CurrentSeq = frame->seq + 1;
    }
    else if (s_CurrentSeq == 255)
    {
        s_CurrentSeq = 0;
    }
    else
    {
        s_CurrentSeq++;
    }

    s_CurrentOBCData = {
        .qw = (float)frame->qw,
        .qx = (float)frame->qx,
        .qy = (float)frame->qy,
        .qz = (float)frame->qz,
        .velocity = (float)frame->velocity,
        .batteryVoltage = frame->batteryVoltage10 / 10.0f,
        .batteryPercentage = frame->batteryPercentage,
        .latitude = frame->lat,
        .longitude = frame->lon,
        .altitude = (int)frame->alt,
        .state = (int)frame->state,
        .controlFlags = (int)frame->controlFlags,
        .signalStrength = s_Rssi,
        .packetLoss = (int)((float)s_PacketsLost / s_RX * 100),
    };

    RouterSendData();

    s_SendTLMDataTimeOffset = millis();
}

static bool ValidatePacket(const radio_obc_frame_t *packet)
{
    if (packet->magic != RADIO_MAGIC)
    {
        return false;
    }

    uint16_t crc = CalculateCRC16_MCRF4XX((const uint8_t *)packet, sizeof(radio_obc_frame_t) - 2);

    return crc == packet->crc;
}

static void SendTLMPacket()
{
    radio_tlm_frame_t frame = {
        .magic = RADIO_MAGIC,
    };

    if (s_CurrentTLMData.arm_enable)
    {
        frame.flags |= RADIO_TLM_FLAG_ARM_ENABLE;
    }
    else if (s_CurrentTLMData.arm_disable)
    {
        frame.flags |= RADIO_TLM_FLAG_ARM_DISABLE;
    }
    else if (s_CurrentTLMData.v3v3_enable)
    {
        frame.flags |= RADIO_TLM_FLAG_3V3_ENABLE;
    }
    else if (s_CurrentTLMData.v3v3_disable)
    {
        frame.flags |= RADIO_TLM_FLAG_3V3_DISABLE;
    }
    else if (s_CurrentTLMData.v5_enable)
    {
        frame.flags |= RADIO_TLM_FLAG_5V_ENABLE;
    }
    else if (s_CurrentTLMData.v5_disable)
    {
        frame.flags |= RADIO_TLM_FLAG_5V_DISABLE;
    }
    else if (s_CurrentTLMData.vbat_enable)
    {
        frame.flags |= RADIO_TLM_FLAG_VBAT_ENABLE;
    }
    else if (s_CurrentTLMData.vbat_disable)
    {
        frame.flags |= RADIO_TLM_FLAG_VBAT_DISABLE;
    }

    frame.crc = CalculateCRC16_MCRF4XX((const uint8_t *)&frame, sizeof(frame) - 2);

    const uint8_t *buffer = (const uint8_t *)&frame;
    size_t size = sizeof(frame);

    LoRa.beginPacket();

    for (size_t i = 0; i < size; i++)
    {
        LoRa.write(buffer[i]);
    }

    LoRa.endPacket();

    memset(&s_CurrentTLMData, 0, sizeof(s_CurrentTLMData));

    s_TX++;

    Serial.printf("Successfully sent %f bytes\n", size);
}