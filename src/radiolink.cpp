#include "radiolink.h"
#include "maths.h"
#include <string.h>
#include <Arduino.h>

void radiolink_serialize_sensor_frame(radiolink_frame_t *rlFrame, radiolink_sensor_frame_t *frame)
{
    rlFrame->magic = RADIOLINK_MAGIC;
    rlFrame->len = sizeof(*frame);
    rlFrame->msgId = RADIOLINK_MESSAGE_SENSORS;

    memcpy(rlFrame->payload, frame, rlFrame->len);

    rlFrame->checksum = crc16_mcrf4xx_calculate((uint8_t *)rlFrame, 3 * sizeof(uint8_t) + rlFrame->len);
}

bool radiolink_get_bytes(radiolink_frame_t *frame, uint8_t *data, size_t *len)
{
    if (frame->len > RADIOLINK_MAX_PAYLOAD_LENGTH)
    {
        Serial.printf("RadioLink frame length is too big: %d\n", frame->len);

        return false;
    }

    size_t offset = 3 * sizeof(uint8_t) + frame->len;
    size_t totalLength = 3 * sizeof(uint8_t) + frame->len + sizeof(uint16_t);

    if (*len < totalLength)
    {
        Serial.println("RadioLink buffer length is too small!");

        return false;
    }

    *len = totalLength;

    memcpy(data, frame, offset);
    memcpy(data + offset, &frame->checksum, sizeof(frame->checksum));

    return true;
}

bool radiolink_deserialize(radiolink_frame_t *frame, const uint8_t *data, size_t len)
{
    memset(frame, 0, sizeof(*frame));

    if (len > sizeof(radiolink_frame_t) || len < 5)
    {
        Serial.println("Invalid payload length!");

        return false;
    }

    size_t offset = 3 * sizeof(uint8_t);

    memcpy(frame, data, offset);

    if (frame->magic != RADIOLINK_MAGIC)
    {
        Serial.println("RadioLink magic is incorrect!");

        return false;
    }

    if (frame->len > RADIOLINK_MAX_PAYLOAD_LENGTH || len - offset != frame->len + sizeof(uint16_t))
    {
        Serial.println("RadioLink frame length is invalid!");

        return false;
    }

    if (frame->len > 0)
    {
        memcpy(&frame->payload, data + offset, frame->len);
    }

    memcpy(&frame->checksum, data + offset + frame->len, sizeof(frame->checksum));

    uint16_t checksum = crc16_mcrf4xx_calculate((uint8_t *)frame, 3 * sizeof(uint8_t) + frame->len);

    if (frame->checksum != checksum)
    {
        Serial.println("Checksum of RadioLink frame was incorrect!");

        return false;
    }

    return true;
}