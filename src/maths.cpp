#include "maths.h"

float CalculateGeoDistance(float lat1, float lon1, float lat2, float lon2)
{
    lat1 = DEG_2_RAD(lat1);
    lat2 = DEG_2_RAD(lat2);

    double d_lat = lat2 - lat1;
    double d_lon = DEG_2_RAD(lon2 - lon1);

    double sin_d_lat_2 = sin(d_lat / 2.0);
    double sin_d_lon_2 = sin(d_lon / 2.0);

    double a = sin_d_lat_2 * sin_d_lat_2 + sin_d_lon_2 * sin_d_lon_2 * cos(lat1) * cos(lat2);
    double c = 2 * atan2(sqrt(a), sqrt(1.0 - a));

    return (float)c * EARTH_RADIUS;
}

uint16_t CalculateCRC16_MCRF4XX(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    uint8_t t;
    uint8_t L;

    while (length--)
    {
        crc ^= *data++;
        L = crc ^ (crc << 4);
        t = (L << 3) | (L >> 5);
        L ^= (t & 0x07);
        t = (t & 0xF8) ^ (((t << 1) | (t >> 7)) & 0x0F) ^ (uint8_t)(crc >> 8);
        crc = (L << 8) | t;
    }

    return crc;
}