#pragma once

#include <datalink.h>

void LoRaInit();
void LoRaCheck();
int LoRaGetRssi();
int LoRaGetRX();
int LoRaGetTX();
const datalink_frame_telemetry_data_obc_t *LoRaGetCurrentFrame();