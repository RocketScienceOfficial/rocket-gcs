#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct vec3
{
    float x, y, z;
} vec3_t;

float CalculateGeoDistance(float lat1, float lon1, float lat2, float lon2);
float CalulateGeoBearing(float lat1, float lon1, float lat2, float lon2);

uint16_t crc16_mcrf4xx_calculate(const uint8_t *data, size_t length);