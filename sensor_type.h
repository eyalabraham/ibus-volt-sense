/*****************************************************************************
* sensor_type.h
*
* Header file defining sensor ID codes.
* Based on:
* https://github.com/cleanflight/cleanflight/blob/7cd417959b3cb605aa574fc8c0f16759943527ef/src/main/telemetry/ibus_shared.h
*
* Created: September 2022
*
*****************************************************************************/

#ifndef __SENSOR_TYPE_H__
#define __SENSOR_TYPE_H__

/****************************************************************************
  Definitions
****************************************************************************/

/*  Sensor type byte value --------------------    Returned value in sensor data packet
 */
typedef enum {
    IBUS_SENSOR_TYPE_INT_VOLT         = 0x00,   // [Do not use] Internal RC receiver voltage in 0.01 volt increments
    IBUS_SENSOR_TYPE_TEMPERATURE      = 0x01,   // 2-bytes Temperature in 0.1 degrees, where 0=-40'C
    IBUS_SENSOR_TYPE_RPM_FLYSKY       = 0x02,   // 2-bytes RPM
    IBUS_SENSOR_TYPE_EXTERNAL_VOLTAGE = 0x03,   // 2-bytes External voltage measurement in 0.01 volt increments
    IBUS_SENSOR_TYPE_CELL             = 0x04,   // 2-bytes Average Cell voltage
    IBUS_SENSOR_TYPE_BAT_CURR         = 0x05,   // 2-bytes Battery current A * 100
    IBUS_SENSOR_TYPE_FUEL             = 0x06,   // 2-bytes Remaining battery percentage / mAh drawn otherwise or fuel level no unit!
    IBUS_SENSOR_TYPE_RPM              = 0x07,   // 2-bytes Throttle value / battery capacity
    IBUS_SENSOR_TYPE_CMP_HEAD         = 0x08,   // 2-bytes Heading  0..360 degrees, 0=north
    IBUS_SENSOR_TYPE_CLIMB_RATE       = 0x09,   // 2-bytes m/s *100
    IBUS_SENSOR_TYPE_COG              = 0x0a,   // 2-bytes  Course over ground (NOT heading, but direction of movement) in degrees * 100, 0.0..359.99 degrees. unknown max uint
    IBUS_SENSOR_TYPE_GPS_STATUS       = 0x0b,   // 2-bytes ??
    IBUS_SENSOR_TYPE_ACC_X            = 0x0c,   // 2-bytes m/s *100 signed
    IBUS_SENSOR_TYPE_ACC_Y            = 0x0d,   // 2-bytes m/s *100 signed
    IBUS_SENSOR_TYPE_ACC_Z            = 0x0e,   // 2-bytes m/s *100 signed
    IBUS_SENSOR_TYPE_ROLL             = 0x0f,   // 2-bytes degrees *100 signed
    IBUS_SENSOR_TYPE_PITCH            = 0x10,   // 2-bytes degrees *100 signed
    IBUS_SENSOR_TYPE_YAW              = 0x11,   // 2-bytes degrees *100 signed
    IBUS_SENSOR_TYPE_VERTICAL_SPEED   = 0x12,   // 2-bytes m/s *100
    IBUS_SENSOR_TYPE_GROUND_SPEED     = 0x13,   // 2-bytes m/s *100 different unit than build-in sensor
    IBUS_SENSOR_TYPE_GPS_DIST         = 0x14,   // 2-bytes distance from home m unsigned
    IBUS_SENSOR_TYPE_ARMED            = 0x15,   // 2-bytes
    IBUS_SENSOR_TYPE_FLIGHT_MODE      = 0x16,   // 2-bytes
    IBUS_SENSOR_TYPE_PRES             = 0x41,   // 4-bytes Pressure
    IBUS_SENSOR_TYPE_ODO1             = 0x7c,   // 2-bytes Odometer1
    IBUS_SENSOR_TYPE_ODO2             = 0x7d,   // 2-bytes Odometer2
    IBUS_SENSOR_TYPE_SPE              = 0x7e,   // 2-bytes Speed Km/h

    IBUS_SENSOR_TYPE_GPS_LAT          = 0x80,   // 4-bytes signed WGS84 in degrees * 1E7
    IBUS_SENSOR_TYPE_GPS_LON          = 0x81,   // 4-bytes signed WGS84 in degrees * 1E7
    IBUS_SENSOR_TYPE_GPS_ALT          = 0x82,   // 4-bytes signed GPS altitude m*100
    IBUS_SENSOR_TYPE_ALT              = 0x83,   // 4-bytes signed altitude m*100
    IBUS_SENSOR_TYPE_ALT_MAX          = 0x84,   // 4-bytes signed maximum altitude m*100

    IBUS_SENSOR_TYPE_ALT_FLYSKY       = 0xf9,   // Altitude 2 bytes signed in m

    IBUS_SENSOR_TYPE_UNKNOWN          = 0xff
} ibus_sensor_type_t;

#endif  /* __SENSOR_TYPE_H__ */
