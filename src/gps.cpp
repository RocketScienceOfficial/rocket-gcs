#include "gps.h"
#include "config.h"
#include "serial.h"
#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <MicroNMEA.h>

static SFE_UBLOX_GNSS s_GNSS;
static char s_NMEABuffer[100];
static MicroNMEA s_NMEA(s_NMEABuffer, sizeof(s_NMEABuffer));
static double s_Latitude;
static double s_Longitude;
static float s_Altitude;

void GPSInit()
{
    Serial1.begin(9600, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);

    if (!s_GNSS.begin(Serial1))
    {
        SERIAL_DEBUG_PRINTF("GPS failed to start!\n");

        while (1)
            ;
    }

    s_GNSS.setUART1Output(COM_TYPE_NMEA);
    s_GNSS.saveConfiguration();

    SERIAL_DEBUG_PRINTF("GPS started!\n");
}

void GPSCheck()
{
    if (s_GNSS.checkUblox())
    {
        if (s_NMEA.isValid())
        {
            long lat = s_NMEA.getLatitude();
            long lon = s_NMEA.getLongitude();
            long alt = 0;

            s_Latitude = lat / 1000000.0;
            s_Longitude = lon / 1000000.0;

            s_NMEA.getAltitude(alt);

            s_Altitude = alt / 1000.0f;

            SERIAL_DEBUG_PRINTF("Updated GPS!\n");
        }
    }
}

double GPSGetLatitude()
{
    return s_Latitude;
}

double GPSGetLongitude()
{
    return s_Longitude;
}

float GPSGetAltitude()
{
    return s_Altitude;
}

void SFE_UBLOX_GNSS::processNMEA(char incoming)
{
    s_NMEA.process(incoming);
}