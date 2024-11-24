#pragma once

#include <datalink.h>
#include <stdint.h>

void SerialControlInit();
void SerialControlUpdate();
void SerialControlSendFrame(const datalink_frame_structure_serial_t *frame);
uint8_t SerialControlGetCurrentControlFlags();