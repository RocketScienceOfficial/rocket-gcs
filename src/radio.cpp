#include "radio.h"
#include "radiolink.h"
#include "config.h"
#include "measurements.h"
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

static int s_Rssi;
static int s_RX;
static int s_TX;
static uint8_t s_CurrentSeq;
static int s_PacketsLost;

static void TryRunTest();
static void TryParsePacket(uint8_t *buffer, size_t len);

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

    LoRa.setSignalBandwidth(LORA_BANDWIDTH);

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

    TryRunTest();
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

static void TryRunTest()
{
    if (LORA_TEST)
    {
        static unsigned long timer;
        static uint8_t sequence;

        if (millis() - timer >= 2000)
        {
            timer = millis();

            radiolink_sensor_frame_t sens = {
                .pos = {0},
                .gyro = {60, 50, 0},
                .lat = 51.5287398,
                .lon = -0.2664034,
                .alt = 100.0f,
                .velocity = 50.0f,
                .batteryVoltage = 9.3f,
                .batteryPercentage = 87,
                .pressure = 99900.0f,
                .temperature = 290.0f,
            };
            radiolink_frame_t frame;
            radiolink_serialize_sensor_frame(&frame, &sequence, &sens);

            uint8_t buff[512];
            size_t len = sizeof(buff);
            radiolink_get_bytes(&frame, buff, &len);

            TryParsePacket(buff, len);
        }
    }
}

static void TryParsePacket(uint8_t *buffer, size_t len)
{
    radiolink_frame_t frame = {0};

    if (!radiolink_deserialize(&frame, buffer, len))
    {
        Serial.println("Couldn't deserialize packet!");

        return;
    }

    s_Rssi = LoRa.packetRssi();
    s_RX++;

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

    radiolink_sensor_frame_t *newFrame = (radiolink_sensor_frame_t *)frame.payload;

    MeasurementData measurement = {
        .positionX = newFrame->pos.x,
        .positionY = newFrame->pos.y,
        .positionZ = newFrame->pos.z,
        .roll = newFrame->gyro.x,
        .pitch = newFrame->gyro.y,
        .yaw = newFrame->gyro.z,
        .latitude = newFrame->lat,
        .longitude = newFrame->lon,
        .altitude = newFrame->alt,
        .velocity = newFrame->velocity,
        .batteryVoltage = newFrame->batteryVoltage,
        .batteryPercentage = newFrame->batteryPercentage,
        .pressure = newFrame->pressure,
        .temperature = newFrame->temperature,
        .signalStrength = s_Rssi,
        .packetLoss = (int)((float)s_PacketsLost / s_RX * 100),
    };

    SetMeasurementData(&measurement);
}