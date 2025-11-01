#include "radio.h"
#include "config.h"
#include "serial.h"
#include "gps.h"
#include "maths.h"
#include "serial.h"
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <string.h>

#define DEVICE_ID 0xDF
#define OBC_ID 0x11
#define RADIO_TLM_DATA_SEND_DELAY 100
#define TEMP_RX_RESET_TIME 5000

static datalink_frame_telemetry_data_obc_t s_CurrentFrame;
static int s_Rssi;
static int s_RX;
static int s_TX;
static uint8_t s_CurrentSeq;
static int s_PacketsLost;
static int s_TempRX;
static unsigned long s_SendTLMDataTimeOffset;
static unsigned long s_TempRXTimeOffset;

static void TryParsePacket(uint8_t *buffer, size_t len);
static void SendTLMPacket();

void LoRaInit()
{
    SERIAL_DEBUG_PRINTF("Starting LoRa...\n");

    SPI.begin(LORA_SCLK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN);
    LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    if (!LoRa.begin(LORA_FREQ))
    {
        SERIAL_DEBUG_PRINTF("Starting LoRa failed!\n");

        while (true)
            ;
    }

    LoRa.setFrequency(LORA_FREQ);
    LoRa.setSignalBandwidth(LORA_BAND);
    LoRa.setSpreadingFactor(LORA_SF);
    LoRa.receive();

    SERIAL_DEBUG_PRINTF("Starting LoRa success!\n");
}

void LoRaCheck()
{
    int packetSize = LoRa.parsePacket();

    if (packetSize)
    {
        SERIAL_DEBUG_PRINTF("Received packet with size: %d bytes\n", packetSize);

        uint8_t buffer[512];
        size_t i = 0;

        while (LoRa.available())
        {
            if (i == packetSize)
            {
                SERIAL_DEBUG_PRINTF("Something went wrong while parsing packet!\n");

                return;
            }

            if (i < sizeof(buffer))
            {
                buffer[i++] = (uint8_t)LoRa.read();
            }
            else
            {
                SERIAL_DEBUG_PRINTF("Buffer overflow while parsing packet!\n");

                return;
            }
        }

        TryParsePacket(buffer, i);
    }

    if (s_SendTLMDataTimeOffset != 0 && millis() - s_SendTLMDataTimeOffset >= RADIO_TLM_DATA_SEND_DELAY)
    {
        SendTLMPacket();

        s_SendTLMDataTimeOffset = 0;
    }

    if (millis() - s_TempRXTimeOffset >= TEMP_RX_RESET_TIME)
    {
        s_TempRX = 0;
        s_PacketsLost = 0;
        s_TempRXTimeOffset = millis();
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

const datalink_frame_telemetry_data_obc_t *LoRaGetCurrentFrame()
{
    return &s_CurrentFrame;
}

static void TryParsePacket(uint8_t *buffer, size_t len)
{
    datalink_frame_structure_radio_t frame;

    if (!datalink_deserialize_frame_radio(&frame, buffer, len))
    {
        SERIAL_DEBUG_PRINTF("Couldn't deserialize packet!\n");

        return;
    }

    if (frame.srcId != OBC_ID || frame.destId != DEVICE_ID)
    {
        SERIAL_DEBUG_PRINTF("Packet source or destination is invalid!\n");

        return;
    }

    s_Rssi = LoRa.packetRssi();
    s_RX++;
    s_TempRX++;

    if (frame.seq != s_CurrentSeq)
    {
        s_PacketsLost += frame.seq > s_CurrentSeq ? frame.seq - s_CurrentSeq : 256 + frame.seq - s_CurrentSeq;
        s_CurrentSeq = frame.seq + 1;
    }
    else if (s_CurrentSeq == 255)
    {
        s_CurrentSeq = 0;
    }
    else
    {
        s_CurrentSeq++;
    }

    if (frame.msgId != DATALINK_MESSAGE_TELEMETRY_DATA_OBC && frame.msgId != DATALINK_MESSAGE_TELEMETRY_DATA_OBC_WITH_RESPONSE)
    {
        SERIAL_DEBUG_PRINTF("Invalid message ID!\n");

        return;
    }

    const datalink_frame_telemetry_data_obc_t *payload = (const datalink_frame_telemetry_data_obc_t *)frame.payload;

    s_CurrentFrame = *payload;

    datalink_frame_telemetry_data_gcs_t newPayload = {
        .qw = payload->qw,
        .qx = payload->qx,
        .qy = payload->qy,
        .qz = payload->qz,
        .velocity_kmh = payload->velocity_kmh,
        .batteryVoltage100 = payload->batteryVoltage100,
        .batteryPercentage = payload->batteryPercentage,
        .lat = payload->lat,
        .lon = payload->lon,
        .alt = payload->alt,
        .gcsLat = (int)(GPSGetLatitude() * 10000000),
        .gcsLon = (int)(GPSGetLongitude() * 10000000),
        .gpsData = payload->gpsData,
        .state = payload->state,
        .ignFlags = payload->ignFlags,
        .controlFlags = payload->controlFlags,
        .signalStrengthNeg = (uint8_t)-s_Rssi,
        .packetLossPercentage = (uint8_t)((float)s_PacketsLost / (s_TempRX + s_PacketsLost) * 100),
        .packetsReceived = (uint16_t)s_RX,
        .packetsTransmitted = (uint16_t)s_TX,
    };
    datalink_frame_structure_serial_t newFrame = {
        .msgId = DATALINK_MESSAGE_TELEMETRY_DATA_GCS,
        .len = sizeof(newPayload),
    };
    memcpy(newFrame.payload, &newPayload, sizeof(newPayload));

    SerialControlSendFrame(&newFrame);

    if (frame.msgId == DATALINK_MESSAGE_TELEMETRY_DATA_OBC_WITH_RESPONSE)
    {
        s_SendTLMDataTimeOffset = millis();
    }

    SERIAL_DEBUG_PRINTF("Successfully parsed packet!\n");
}

static void SendTLMPacket()
{
    static uint8_t sequence = 0;

    datalink_frame_telemetry_response_t payload = {
        .flags = SerialControlGetCurrentControlFlags(),
    };
    datalink_frame_structure_radio_t frame = {
        .seq = sequence,
        .srcId = DEVICE_ID,
        .destId = OBC_ID,
        .msgId = DATALINK_MESSAGE_TELEMETRY_RESPONSE,
        .len = sizeof(payload),
    };
    memcpy(frame.payload, &payload, sizeof(payload));

    uint8_t buffer[512];
    int len = sizeof(buffer);

    if (datalink_serialize_frame_radio(&frame, buffer, &len))
    {
        LoRa.beginPacket();
        LoRa.write(buffer, len);
        LoRa.endPacket();

        s_TX++;

        SERIAL_DEBUG_PRINTF("Successfully sent %d bytes\n", len);

        LoRa.receive();
    }
    else
    {
        SERIAL_DEBUG_PRINTF("Couldn't serialize packet to send!\n");
    }

    sequence = sequence == 255 ? 0 : sequence + 1;
}