#pragma once

#include "config.h"
#include <Arduino.h>
#include <datalink.h>
#include <stdint.h>

#if IS_DEBUG
#define SERIAL_DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define SERIAL_DEBUG_PRINTF(fmt, ...)
#endif

void SerialControlInit();
void SerialControlUpdate();
void SerialControlSendFrame(const datalink_frame_structure_serial_t *frame);
uint8_t SerialControlGetCurrentControlFlags();