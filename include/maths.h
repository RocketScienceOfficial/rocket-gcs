#pragma once

#include <Arduino.h>

#define RAD_2_DEG(x) ((x) * 180.0 / PI)
#define DEG_2_RAD(x) ((x) * PI / 180.0)
#define EARTH_RADIUS 6371000.0

typedef struct vec3
{
    float x, y, z;
} vec3_t;

float CalculateGeoDistance(float lat1, float lon1, float lat2, float lon2);
uint16_t CalculateCRC16_MCRF4XX(const uint8_t *data, size_t length);