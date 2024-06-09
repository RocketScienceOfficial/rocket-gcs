#include "radio.h"
#include "config.h"
#include "measurements.h"
#include "maths.h"
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#define RADIO_MAGIC 0x7B

typedef struct __attribute__((__packed__)) radio_frame
{
    uint8_t magic;
    uint8_t roll;
    uint8_t pitch;
    uint8_t yaw;
    uint16_t velocity;
    uint8_t batteryVoltage10;
    uint8_t batteryPercentage;
    double lat;
    double lon;
    uint16_t alt;
    uint8_t state;
    uint8_t seq;
    uint16_t crc;
} radio_frame_t;

static int s_Rssi;
static int s_RX;
static int s_TX;
static uint8_t s_CurrentSeq;
static int s_PacketsLost;

static void TryParsePacket(uint8_t *buffer, size_t len);
static bool ValidatePacket(const radio_frame_t *packet);

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

static void TryParsePacket(uint8_t *buffer, size_t len)
{
    if (len != sizeof(radio_frame_t))
    {
        Serial.println("Invalid packet length!");

        return;
    }

    radio_frame_t *frame = (radio_frame_t *)buffer;

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

    MeasurementData measurement = {
        .roll = (float)frame->roll,
        .pitch = (float)frame->pitch,
        .yaw = (float)frame->yaw,
        .velocity = (float)frame->velocity,
        .batteryVoltage = frame->batteryVoltage10 / 10.0f,
        .batteryPercentage = frame->batteryPercentage,
        .latitude = frame->lat,
        .longitude = frame->lon,
        .altitude = (int)frame->alt,
        .state = (int)frame->state,
        .signalStrength = s_Rssi,
        .packetLoss = (int)((float)s_PacketsLost / s_RX * 100),
    };

    SetMeasurementData(&measurement);
}

static bool ValidatePacket(const radio_frame_t *packet)
{
    if (packet->magic != RADIO_MAGIC)
    {
        return false;
    }

    uint16_t crc = CalculateCRC16_MCRF4XX((const uint8_t *)packet, sizeof(radio_frame_t) - 2);

    return crc == packet->crc;
}

// static void TryRunTest()
// {
//     static unsigned long timer;
//     static uint8_t sequence;

//     if (millis() - timer >= 2000)
//     {
//         timer = millis();

//         radiolink_sensor_frame_t sens = {
//             .pos = {0},
//             .gyro = {60, 50, 0},
//             .lat = 51.5287398,
//             .lon = -0.2664034,
//             .alt = 100.0f,
//             .velocity = 50.0f,
//             .batteryVoltage = 9.3f,
//             .batteryPercentage = 87,
//             .pressure = 99900,
//             .temperature = 290.0f,
//         };
//         radiolink_frame_t frame;
//         radiolink_serialize_sensor_frame(&frame, &sequence, &sens);

//         uint8_t buff[512];
//         size_t len = sizeof(buff);
//         radiolink_get_bytes(&frame, buff, &len);

//         TryParsePacket(buff, len);
//     }
// }