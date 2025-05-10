/**
 * @file MqttLog.h
 * @author Ollo
 * @brief Wrapper for Logger to Mqtt
 * @version 0.1
 *
 */

#ifndef MQTT_LOGGER
#define MQTT_LOGGER

#include <Homie.h>

#define LOG_TOPIC "log\0"
#define MQTT_LEVEL_ERROR    1
#define MQTT_LEVEL_WARNING  10
#define MQTT_LEVEL_INFO     20
#define MQTT_LEVEL_DEBUG    90

#define MQTT_LOG_PM1006     10
#define MQTT_LOG_I2CINIT    100
#define MQTT_LOG_I2READ     101
#define MQTT_LOG_RGB        200

#define MQTT_LOG_VICTRON    400

extern bool mConnected;

void log(int level, String message, int statusCode);

#endif /* end of MQTT_LOGGER */