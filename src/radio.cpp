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

void LoRaInit()
{
    SPI.begin(LORA_SCLK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN);

    delay(2000);

    Serial.println("Starting LoRa...");

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

        s_Rssi = LoRa.packetRssi();
        s_RX++;

        radiolink_frame_t frame = {0};

        if (!radiolink_deserialize(&frame, buffer, i))
        {
            return;
        }

        radiolink_sensor_frame_t *newFrame = (radiolink_sensor_frame_t *)frame.payload;

        MeasurementData measurement = {
            .pos_x = 0,
            .pos_y = 0,
            .pos_z = 0,
            .roll = 0,
            .pitch = 0,
            .yaw = 0,
            .latitude = 0,
            .longitude = 0,
            .altitude = 0,
            .velocity = 0,
        };

        SetMeasurementData(&measurement);
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