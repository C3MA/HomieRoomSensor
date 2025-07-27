/**
 * @file HomieSettings.h
 * @author your name (you@domain.com)
 * @brief Air quality sensor
 * @version 0.1
 * @date 2021-11-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef HOMIE_SETTINGS
#define HOMIE_SETTINGS

#if BME680
#define FIRMWARE_FEATURE1   "_Bme680"
#else
#define FIRMWARE_FEATURE1   ""
#endif

#if VICTRON
#define FIRMWARE_FEATURE2   "_Victron"
#else
#define FIRMWARE_FEATURE2   ""
#endif

#define HOMIE_FIRMWARE_NAME     "RoomSensor" FIRMWARE_FEATURE1 FIRMWARE_FEATURE2
#define HOMIE_FIRMWARE_VERSION  "2.4.5"

#define SERIAL_BAUDRATE 19200

#endif
