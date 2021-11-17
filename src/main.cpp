/**
 * @file main.cpp
 * @author Ollo
 * @brief
 * @version 0.1
 * @date 2021-11-05
 *
 * @copyright Copyright (c) 2021
 *
 */

/******************************************************************************
 *                                     INCLUDES
 ******************************************************************************/
#include <Homie.h>
#include <SoftwareSerial.h>
#include "HomieSettings.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

/******************************************************************************
 *                                     DEFINES
 ******************************************************************************/

#define GPIO_WS2812         D4 /**< GPIO2 */
#define SENSOR_PM1006_RX    D2 /**< GPIO4 */
#define SENSOR_PM1006_TX    -1 /**< Unused */
#define WITTY_RGB_R         D8 /**< GPIO15 */
#define WITTY_RGB_G         D6 /**< GPIO12 */
#define WITTY_RGB_B         D7 /**< GPIO13 */
#define PM1006_BIT_RATE     9600
#define PM1006_MQTT_UPDATE  5000 /**< Check the sensor every 10 seconds; New measurement is done every 20seconds by the sensor */
#define PIXEL_COUNT         3
#define GPIO_BUTTON   SENSOR_PM1006_RX /**< Button and software serial share one pin on Witty board */
#define SENSOR_I2C_SCK    D5 /**< GPIO14 - I2C clock pin */
#define SENSOR_I2C_SDI    D1 /**< GPIO5  - I2C data pin */

#define PM1006_BIT_RATE 9600
#define PM1006_MQTT_UPDATE 5000 /**< Check the sensor every 10 seconds; New measurement is done every 20seconds by the sensor */

#define SEALEVELPRESSURE_HPA (1013.25)

#define LOG_TOPIC "log\0"
#define MQTT_LOG_ERROR    1
#define MQTT_LOG_WARNING  10
#define MQTT_LOG_INFO     20
#define MQTT_LOG_DEBUG    90

#define TEMPBORDER        20

#define getTopic(test, topic)                                                                                                                 \
  char *topic = new char[strlen(Homie.getConfiguration().mqtt.baseTopic) + strlen(Homie.getConfiguration().deviceId) + 1 + strlen(test) + 1]; \
  strcpy(topic, Homie.getConfiguration().mqtt.baseTopic);                                                                                     \
  strcat(topic, Homie.getConfiguration().deviceId);                                                                                           \
  strcat(topic, "/");                                                                                                                         \
  strcat(topic, test);         

#define NUMBER_TYPE                     "Number"
#define NODE_PARTICE                    "particle"
#define NODE_TEMPERATUR                 "temp"
#define NODE_PRESSURE                   "pressure"
#define NODE_ALTITUDE                   "altitude"
/******************************************************************************
 *                                     TYPE DEFS
 ******************************************************************************/

/******************************************************************************
 *                            FUNCTION PROTOTYPES
 ******************************************************************************/

void log(int level, String message, int code);

/******************************************************************************
 *                            LOCAL VARIABLES
 ******************************************************************************/

bool mConfigured = false;
bool mConnected = false;

HomieNode particle(NODE_PARTICE, "particle", "Particle"); /**< Measuret in micro gram per quibik meter air volume */
HomieNode temperatureNode(NODE_TEMPERATUR, "Room Temperature", "Room Temperature");
HomieNode pressureNode(NODE_PRESSURE, "Pressure", "Room Pressure");
HomieNode altitudeNode(NODE_ALTITUDE, "Altitude", "Room altitude");

HomieSetting<bool> i2cEnable("i2c", "BMP280 sensor present");
HomieSetting<bool> rgbTemp("rgbTemp", "Show temperatur via red (>20 °C) and blue (< 20°C)");

static SoftwareSerial pmSerial(SENSOR_PM1006_RX, SENSOR_PM1006_TX);
Adafruit_BMP280 bmp; // connected via I2C

Adafruit_NeoPixel strip(PIXEL_COUNT, GPIO_WS2812, NEO_GRB + NEO_KHZ800);

// Variablen
uint8_t serialRxBuf[80];
uint8_t rxBufIdx = 0;
int spm25 = 0;
int last = 0;
unsigned int mButtonPressed = 0;

/******************************************************************************
 *                            LOCAL FUNCTIONS
 *****************************************************************************/

/**
 * @brief Get the Sensor Data from software serial
 * 
 * @return int PM25 value
 */
int getSensorData() {
  uint8_t rxBufIdx = 0;
  uint8_t checksum = 0;

  // Sensor Serial aushorchen
  while ((pmSerial.available() && rxBufIdx < 127) || rxBufIdx < 20) 
  {
    serialRxBuf[rxBufIdx++] = pmSerial.read();
    delay(15);
  }

  // calculate checksum
  for (uint8_t i = 0; i < 20; i++) 
  {
    checksum += serialRxBuf[i];
  }
  
  /* Debug Print of received bytes */
  String dbgBuffer = String("PM1006: ");
  for (uint8_t i = 0; i < 20; i++) 
  {
    dbgBuffer += String(serialRxBuf[i], 16);
  }
  dbgBuffer += String("check: " + String(checksum, 16));
  log(MQTT_LOG_DEBUG, String(dbgBuffer), 1);

  // Header und Prüfsumme checken
  if (serialRxBuf[0] == 0x16 && serialRxBuf[1] == 0x11 && serialRxBuf[2] == 0x0B /* && checksum == 0 */)
  {
    return (serialRxBuf[5] << 8 | serialRxBuf[6]);
  }
  else
  {
    return (-1);
  }
}

/**
 * @brief Handle events of the Homie platform
 * @param event
 */
void onHomieEvent(const HomieEvent &event)
{
  switch (event.type)
  {
  case HomieEventType::MQTT_READY:
    mConnected=true;
    digitalWrite(WITTY_RGB_R, LOW);
    if (!i2cEnable.get()) { /** keep green LED activated to power I2C sensor */
      digitalWrite(WITTY_RGB_G, LOW);
    }
    digitalWrite(WITTY_RGB_B, LOW);
    strip.fill(strip.Color(0,0,128));
    strip.show();
    break;
  case HomieEventType::READY_TO_SLEEP:
    break;
  case HomieEventType::OTA_STARTED:
    break;
  case HomieEventType::OTA_SUCCESSFUL:
    ESP.restart();
    break;
  default:
    break;
  }
}

void bmpPublishValues() {
  temperatureNode.setProperty(NODE_TEMPERATUR).send(String(bmp.readTemperature()));
  pressureNode.setProperty(NODE_PRESSURE).send(String(bmp.readPressure() / 100.0F));
  altitudeNode.setProperty(NODE_ALTITUDE).send(String(bmp.readAltitude(SEALEVELPRESSURE_HPA)));
  if (rgbTemp.get()) {
      if (bmp.readTemperature() < TEMPBORDER) {
        strip.setPixelColor(0, strip.Color(0,0,255));
      } else {
        strip.setPixelColor(0, strip.Color(255,0,0));
      }
      strip.show();
  }
}

/**
 * @brief Main loop, triggered by the Homie API
 * All logic needs to be done here.
 * 
 * The ranges are defined as followed:
 * - green: 0-35 (good+low)
 * - orange: 36-85 (OK+medicore)
 * - red: 86-... (bad+high)
 * 
 * source @link{https://github.com/Hypfer/esp8266-vindriktning-particle-sensor/issues/16#issuecomment-903116056}
 */
void loopHandler()
{
  static long lastRead = 0;
  if ((millis() - lastRead) > PM1006_MQTT_UPDATE) {
    int pM25 = getSensorData();
    if (pM25 >= 0) {
      particle.setProperty("particle").send(String(pM25));
      if (pM25 < 35) {
        strip.fill(strip.Color(0, 255, 0)); /* green */
      } else if (pM25 < 85) {
        strip.fill(strip.Color(255, 127, 0)); /* orange */
      } else {
        strip.fill(strip.Color(255, 0, 0)); /* red */
      }
    }
    strip.show();

    /* Read BOSCH sensor */
    bmpPublishValues();
  
    lastRead = millis();
  }
  // Feed the dog -> ESP stay alive
  ESP.wdtFeed();
}

/******************************************************************************
 *                            GLOBAL FUNCTIONS
 *****************************************************************************/

void setup()
{
  SPIFFS.begin();
  Serial.begin(115200);
  Serial.setTimeout(2000);
  pinMode(WITTY_RGB_R, OUTPUT);
  pinMode(WITTY_RGB_G, OUTPUT);
  pinMode(WITTY_RGB_B, OUTPUT);
  digitalWrite(WITTY_RGB_R, LOW);
  digitalWrite(WITTY_RGB_G, LOW);
  digitalWrite(WITTY_RGB_B, LOW);
    
  Homie_setFirmware(HOMIE_FIRMWARE_NAME, HOMIE_FIRMWARE_VERSION);
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(onHomieEvent);
  i2cEnable.setDefaultValue(false);
  rgbTemp.setDefaultValue(false);
  memset(serialRxBuf, 0, 80);

  pmSerial.begin(PM1006_BIT_RATE);
  Homie.setup();
  
  particle.advertise("particle").setName("Particle").setDatatype(NUMBER_TYPE).setUnit("micro gram per quibik");
  temperatureNode.advertise(NODE_TEMPERATUR).setName("Degrees")
                                      .setDatatype("float")
                                      .setUnit("ºC");
  pressureNode.advertise(NODE_PRESSURE).setName("Pressure")
                                      .setDatatype("float")
                                      .setUnit("hPa");
  altitudeNode.advertise(NODE_ALTITUDE).setName("Altitude")
                                      .setDatatype("float")
                                      .setUnit("m");


  strip.begin();
  /* activate I2C for BOSCH sensor */
  Wire.begin(SENSOR_I2C_SDI, SENSOR_I2C_SCK);

  mConfigured = Homie.isConfigured();
  digitalWrite(WITTY_RGB_G, HIGH);
  if (mConfigured)
  {
    if (i2cEnable.get()) {
      strip.fill(strip.Color(0,128,0));
      strip.show();
      /* Extracted from library's example */
      if (!bmp.begin()) {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                          "try a different address!"));
        while (1) delay(10);
      }

      /* Default settings from datasheet. */
      bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                      Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                      Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                      Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                      Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    }
    strip.fill(strip.Color(0,0,0));
    for (int i=0;i < (PIXEL_COUNT / 2); i++) {
      strip.setPixelColor(0, strip.Color(0,0,128));
    }
    strip.show();
    digitalWrite(WITTY_RGB_B, HIGH);
  } else {
    strip.fill(strip.Color(128,0,0));
    for (int i=0;i < (PIXEL_COUNT / 2); i++) {
      strip.setPixelColor(0, strip.Color(0,0,128));
    }
    strip.show();
  }
}

void loop()
{
  Homie.loop();
  /* use the pin, receiving the soft serial additionally as button */
  if (digitalRead(GPIO_BUTTON) == LOW) {
    mButtonPressed++;
  } else {
    mButtonPressed=0U;
  }

  if (mButtonPressed > 10000U) {
    mButtonPressed=0U;
    if (SPIFFS.exists("/homie/config.json")) {
      printf("Resetting config\r\n");
      SPIFFS.remove("/homie/config.json");
      SPIFFS.end();
    } else {
      printf("No config present\r\n");
    }
  }
}


void log(int level, String message, int statusCode)
{
  String buffer;
  StaticJsonDocument<200> doc;
  doc["level"] = level;
  doc["message"] = message;
  doc["statusCode"] = statusCode;
  serializeJson(doc, buffer);
  if (mConnected)
  {
    getTopic(LOG_TOPIC, logTopic)

    Homie.getMqttClient().publish(logTopic, 2, false, buffer.c_str());
    delete logTopic;
  }
}
