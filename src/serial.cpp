#include "serial.h"
#include "radio.h"
#include "config.h"
#include <cobs.h>
#include <string.h>

static uint8_t s_RecvBuffer[512];
static size_t s_CurrentSize;
static datalink_frame_structure_serial_t s_CurrentFrame;
static uint8_t s_CurrentControlFlags;

static void ProcessNewFrame();

void SerialControlInit()
{
    Serial.begin(SERIAL_BAUD_RATE);

    SERIAL_DEBUG_PRINTF("Initialized serial port!\n");
}

void SerialControlUpdate()
{
    static uint8_t cobsDecodedBuffer[512];

    if (Serial.available())
    {
        int c = Serial.read();

        if (s_CurrentSize >= sizeof(s_RecvBuffer))
        {
            s_CurrentSize = 0;
        }

        s_RecvBuffer[s_CurrentSize++] = (uint8_t)c;

        if (c == 0x00)
        {
            int newLen = cobs_decode(s_RecvBuffer, s_CurrentSize, cobsDecodedBuffer) - 1;

            if (datalink_deserialize_frame_serial(&s_CurrentFrame, cobsDecodedBuffer, newLen))
            {
                ProcessNewFrame();
            }

            s_CurrentSize = 0;
        }
    }
}

void SerialControlSendFrame(const datalink_frame_structure_serial_t *frame)
{
    static uint8_t buffer[512];
    static uint8_t cobsEncodedBuffer[512];

    int len = sizeof(buffer);

    if (datalink_serialize_frame_serial(frame, buffer, &len))
    {
        int newLen = cobs_encode(buffer, len, cobsEncodedBuffer);
        cobsEncodedBuffer[newLen++] = 0x00;

        Serial.write(cobsEncodedBuffer, newLen);
        Serial.flush();
    }
}

uint8_t SerialControlGetCurrentControlFlags()
{
    return s_CurrentControlFlags;
}

static void ProcessNewFrame()
{
    if (s_CurrentFrame.msgId == DATALINK_MESSAGE_TELEMETRY_RESPONSE)
    {
        const datalink_frame_telemetry_response_t *payload = (const datalink_frame_telemetry_response_t *)s_CurrentFrame.payload;

        s_CurrentControlFlags = payload->flags;
    }
}