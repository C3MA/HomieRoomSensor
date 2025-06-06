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
#ifdef BME680
#include "Adafruit_BME680.h"
#else
#ifdef BMP280
#include "Adafruit_BMP280.h"
#else
#error "Decition, which BMx??? is used missing"
#endif
#endif

/******************************************************************************
 *                                     DEFINES
 ******************************************************************************/

#define GPIO_WS2812         D4 /**< GPIO2 */
#define SENSOR_PM1006_RX    D2 /**< GPIO4  */
#define SENSOR_PM1006_TX    -1 /**< Unused */
#define WITTY_RGB_R         D8 /**< GPIO15 */
#define WITTY_RGB_G         D6 /**< GPIO12 Used as 3.3V Power supply for the I2C Sensor */
#define WITTY_RGB_B         D7 /**< GPIO13 */
#define PM1006_BIT_RATE     9600
#define PM1006_MQTT_UPDATE  30000 /**< Check the sensor every 30 seconds; New measurement is done every 20seconds by the PM1006 sensor */
#define PIXEL_COUNT         3
#define GPIO_BUTTON   SENSOR_PM1006_RX /**< Button and software serial share one pin on Witty board */
#define SENSOR_I2C_SCK    D5 /**< GPIO14 - I2C clock pin */
#define SENSOR_I2C_SDI    D1 /**< GPIO5  - I2C data pin */

#define SEALEVELPRESSURE_HPA (1013.25)

#define BUTTON_MAX_CYCLE        10000U  /**< Action: Reset configuration */
#define BUTTON_MIN_ACTION_CYCLE 55U     /**< Minimum cycle to react on the button (e.g. 5 second) */
#define BUTTON_CHECK_INTERVALL  100U     /**< Check every 100 ms the button state */

#define MIN_MEASURED_CYCLES     2
#define PM_MAX                  1001    /**< According datasheet https://en.gassensor.com.cn/ParticulateMatterSensor/info_itemid_105.html 1000 is the maximum */

#define LOG_TOPIC "log\0"
#define MQTT_LEVEL_ERROR    1
#define MQTT_LEVEL_WARNING  10
#define MQTT_LEVEL_INFO     20
#define MQTT_LEVEL_DEBUG    90

#define MQTT_LOG_PM1006     10
#define MQTT_LOG_I2CINIT    100
#define MQTT_LOG_I2READ     101
#define MQTT_LOG_RGB        200

#define TEMPBORDER        20

#define getTopic(test, topic)                                                                                                                 \
  char *topic = new char[strlen(Homie.getConfiguration().mqtt.baseTopic) + strlen(Homie.getConfiguration().deviceId) + 1 + strlen(test) + 1]; \
  strcpy(topic, Homie.getConfiguration().mqtt.baseTopic);                                                                                     \
  strcat(topic, Homie.getConfiguration().deviceId);                                                                                           \
  strcat(topic, "/");                                                                                                                         \
  strcat(topic, test);         

#define PERCENT2FACTOR(b, a)               ((b * a.get()) / 100)

#define NUMBER_TYPE                     "Number"
#define NODE_PARTICLE                   "particle"
#define NODE_TEMPERATUR                 "temp"
#define NODE_PRESSURE                   "pressure"
#define NODE_ALTITUDE                   "altitude"
#define NODE_GAS                        "gas"
#define NODE_HUMIDITY                   "humidity"
#define NODE_AMBIENT                    "ambient"
#define NODE_BUTTON                     "button"
#define SERIAL_RCEVBUF_MAX                80      /**< Maximum 80 characters can be received from the PM1006 sensor */
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
bool mOTAactive = false;        /**< Stop sleeping, if OTA is running */
bool mFailedI2Cinitialization = false;
long mLastButtonAction = 0;

/******************************* Sensor data **************************/
HomieNode particle(NODE_PARTICLE, "particle", "number"); /**< Measuret in micro gram per quibik meter air volume */
HomieNode temperaturNode(NODE_TEMPERATUR, "Room Temperature", "number");
HomieNode pressureNode(NODE_PRESSURE, "Pressure", "number");
HomieNode altitudeNode(NODE_ALTITUDE, "Altitude", "number");
#ifdef BME680
HomieNode gasNode(NODE_GAS, "Gas", "number");
HomieNode humidityNode(NODE_HUMIDITY, "Humidity", "number");
#endif
HomieNode buttonNode(NODE_BUTTON, "Button", "number");

/****************************** Output control ***********************/
HomieNode ledStripNode /* to rule them all */("led", "RGB led", "color");

/************************** Settings ******************************/
HomieSetting<bool> i2cEnable("i2c", 
#ifdef BME680
"BME680 sensor present"
#else
#ifdef BMP280
"BMP280 sensor present"
#else
"No I2C sensor specified in the project"
#endif
#endif

);
HomieSetting<bool> rgbTemp("rgbTemp", "Show temperature via red (>20 °C) and blue (< 20°C)");
HomieSetting<long> rgbDim("rgbDim", "Factor (1 to 200%) of the status LEDs");
HomieSetting<long> deepsleep("deepsleep", "Amount of seconds to sleep (default 0 - always online, maximum 4294 - 71 minutes)");

static SoftwareSerial pmSerial(SENSOR_PM1006_RX, SENSOR_PM1006_TX);
#ifdef BME680
Adafruit_BME680 bmx(&Wire); // connected via I2C
#else
#ifdef BMP280
Adafruit_BMP280 bmx; // connected via I2C
#endif
#endif

Adafruit_NeoPixel strip(PIXEL_COUNT, GPIO_WS2812, NEO_GRB + NEO_KHZ800);

// Variablen
uint8_t serialRxBuf[SERIAL_RCEVBUF_MAX];
uint8_t rxBufIdx = 0;
int mParticle_pM25 = 0;
int last = 0;
unsigned int mButtonPressed = 0;
bool mSomethingReceived = false;

uint32_t      mMeasureIndex = 0;

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
  log(MQTT_LEVEL_DEBUG, String(dbgBuffer), MQTT_LOG_PM1006);

  // Header und Prüfsumme checken
  if (serialRxBuf[0] == 0x16 && serialRxBuf[1] == 0x11 && serialRxBuf[2] == 0x0B /* && checksum == 0 */)
  {
    int pmValue = (serialRxBuf[5] << 8 | serialRxBuf[6]);
    if (pmValue > PM_MAX) {
      return (-1);
    } else {
      return pmValue;
    }
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
    case HomieEventType::READY_TO_SLEEP:
      if (mOTAactive) {
        Homie.getLogger() << "Skip sleeping, as OTA was started" << endl;
        return;
      } else if (deepsleep.get() > 0) {
        long sleepInSeconds = deepsleep.get();
        Homie.doDeepSleep(sleepInSeconds * 1000000, RF_NO_CAL);
      }
    break;
  case HomieEventType::MQTT_READY:
    mConnected=true;
    digitalWrite(WITTY_RGB_R, LOW);
    if (!i2cEnable.get()) { /** keep green LED activated to power I2C sensor */
      digitalWrite(WITTY_RGB_G, LOW);
      log(MQTT_LEVEL_INFO, F("I2C powersupply deactivated"), MQTT_LOG_I2CINIT);
    }
    digitalWrite(WITTY_RGB_B, LOW);
    /* Update LED only, if not sleeping */
    if (deepsleep.get() <= 0) {
      strip.fill(strip.Color(0,0,PERCENT2FACTOR(127, rgbDim)));
      strip.show();
    }

    if (mFailedI2Cinitialization) {
      log(MQTT_LEVEL_DEBUG, 
#ifdef BME680
    "Could not find a valid BME680 sensor, check wiring or try a different address!"
#else
#ifdef BMP280
    "Could not find a valid BMP280 sensor, check wiring or try a different address!"
#else
  "no I2C sensor defined"
#endif
#endif
      , MQTT_LOG_I2CINIT);
    } else {
      log(MQTT_LEVEL_INFO, F("BME680 sensor found"), MQTT_LOG_I2CINIT);
    }
    break;
  case HomieEventType::OTA_STARTED:
    mOTAactive = true;
    break;
  case HomieEventType::OTA_SUCCESSFUL:
    ESP.restart();
    break;
  case HomieEventType::WIFI_CONNECTED:
    digitalWrite(WITTY_RGB_B, HIGH);
    break;
  default:
    break;
  }
}

void bmpPublishValues() {
#ifdef BME680
  // Tell BME680 to begin measurement.
  unsigned long endTime = bmx.beginReading();
  if (endTime == 0) {
    log(MQTT_LEVEL_ERROR, "BMX not accessible", MQTT_LOG_I2READ);
    return;
  }
#endif
  //  Publish the values
  temperaturNode.setProperty(NODE_TEMPERATUR).send(String(bmx.readTemperature()));
  pressureNode.setProperty(NODE_PRESSURE).send(String(bmx.readPressure() / 100.0F));
  altitudeNode.setProperty(NODE_ALTITUDE).send(String(bmx.readAltitude(SEALEVELPRESSURE_HPA)));
#ifdef BME680
  gasNode.setProperty(NODE_GAS).send(String((bmx.gas_resistance / 1000.0)));
  humidityNode.setProperty(NODE_HUMIDITY).send(String(bmx.humidity));
#endif
  log(MQTT_LEVEL_DEBUG, String("Temp" + String(bmx.readTemperature()) + "\tPressure:" +
      String(bmx.readPressure() / 100.0F) + "\t Altitude:"+
      String(bmx.readAltitude(SEALEVELPRESSURE_HPA))), MQTT_LOG_I2READ);
  if ( (rgbTemp.get()) && (!mSomethingReceived) ) {
      if (bmx.readTemperature() < TEMPBORDER) {
        strip.setPixelColor(0, strip.Color(0,0,PERCENT2FACTOR(127, rgbDim)));
      } else {
        strip.setPixelColor(0, strip.Color(PERCENT2FACTOR(127, rgbDim),0,0));
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
    mParticle_pM25 = getSensorData();
    if (mParticle_pM25 >= 0) {
        particle.setProperty(NODE_PARTICLE).send(String(mParticle_pM25));
      if (!mSomethingReceived) {
        if (mParticle_pM25 < 35) {
          strip.fill(strip.Color(0, PERCENT2FACTOR(127, rgbDim), 0)); /* green */
        } else if (mParticle_pM25 < 85) {
          strip.fill(strip.Color(PERCENT2FACTOR(127, rgbDim), PERCENT2FACTOR(64, rgbDim), 0)); /* orange */
        } else {
          strip.fill(strip.Color(PERCENT2FACTOR(127, rgbDim), 0, 0)); /* red */
        }
        strip.show();
      }
    }

    /* Read BOSCH sensor */
    if (i2cEnable.get() && (!mFailedI2Cinitialization)) {
      bmpPublishValues();
    }

    mMeasureIndex++;

    /* Clean cycles buttons */
    if (mButtonPressed <= BUTTON_MIN_ACTION_CYCLE) {
      buttonNode.setProperty(NODE_BUTTON).send("0");
    }
    lastRead = millis();

    /* If nothing needs to be done, sleep and the time is ready for sleeping */
    if (mMeasureIndex > MIN_MEASURED_CYCLES && (deepsleep.get() > 0) ) {
      Homie.prepareToSleep();
      delay(100);
    }

  }

  /* if the user sees something via the LEDs, inform MQTT, too */
  if (mButtonPressed > BUTTON_MIN_ACTION_CYCLE) {
    buttonNode.setProperty(NODE_BUTTON).send(String(mButtonPressed));
  }

  // Feed the dog -> ESP stay alive
  ESP.wdtFeed();
}



bool ledHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present

  Homie.getLogger() << "Received: " << (value) << endl;
  if (value.equals("250,250,250")) {
    mSomethingReceived = false; // enable animation again
    ledStripNode.setProperty(NODE_AMBIENT).send(value);
    return true;
  } else {
    mSomethingReceived = true; // Stop animation

    int sep1 = value.indexOf(',');
    int sep2 = value.indexOf(',', sep1 + 1);
    if ((sep1 > 0) && (sep2 > 0)) {
      int red = value.substring(0,sep1).toInt(); 
      int green = value.substring(sep1 + 1, sep2).toInt(); 
      int blue = value.substring(sep2 + 1, value.length()).toInt(); 

      uint8_t r = (red * 255) / 250;
      uint8_t g = (green *255) / 250;
      uint8_t b = (blue *255) / 250;
      uint32_t c = strip.Color(r,g,b);
      strip.fill(c);
      strip.show();
      ledStripNode.setProperty(NODE_AMBIENT).send(value);
      return true;
    }    
  }
  return false;
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
  rgbDim.setDefaultValue(100).setValidator([] (long candidate) {
    return (candidate > 1) && (candidate <= 200);
  });
  deepsleep.setDefaultValue(0).setValidator([] (long candidate) {
      return ((candidate >= 0) && (candidate < 4294)); /* between 0 (deactivated) and 71 minutes */
  });
  memset(serialRxBuf, 0, SERIAL_RCEVBUF_MAX);

  pmSerial.begin(PM1006_BIT_RATE);
  Homie.setup();
  
  particle.advertise(NODE_PARTICLE).setName("Particle").setDatatype(NUMBER_TYPE).setUnit("micro gram per quibik");
  temperaturNode.advertise(NODE_TEMPERATUR).setName("Degrees")
                                      .setDatatype("float")
                                      .setUnit("ºC");
  pressureNode.advertise(NODE_PRESSURE).setName("Pressure")
                                      .setDatatype("float")
                                      .setUnit("hPa");
  altitudeNode.advertise(NODE_ALTITUDE).setName("Altitude")
                                      .setDatatype("float")
                                      .setUnit("m");
#ifdef BME680
  gasNode.advertise(NODE_GAS).setName("Gas")
                              .setDatatype("float")
                              .setUnit(" KOhms");
  humidityNode.advertise(NODE_HUMIDITY).setName("Humidity")
                              .setDatatype("float")
                              .setUnit("%");
#endif
  ledStripNode.advertise(NODE_AMBIENT).setName("All Leds")
                            .setDatatype("color").setFormat("rgb")
                            .settable(ledHandler);
  buttonNode.advertise(NODE_BUTTON).setName("Button pressed")
                            .setDatatype("integer");

  strip.begin();

  mConfigured = Homie.isConfigured();
  digitalWrite(WITTY_RGB_G, HIGH);
  if (mConfigured)
  {
    if (i2cEnable.get()) {
#ifdef BME680
    printf("Wait 1 second...\r\n");
    delay(1000);
#endif
      /* activate I2C for BOSCH sensor */
      Wire.begin(SENSOR_I2C_SDI, SENSOR_I2C_SCK);
      printf("Wait 50 milliseconds...\r\n");
      delay(50);
      /* Extracted from library's example */
      mFailedI2Cinitialization = !bmx.begin();
      if (!mFailedI2Cinitialization) {
        strip.fill(strip.Color(0,PERCENT2FACTOR(64, rgbDim),0));
        strip.show();
#ifdef BME680
        bmx.setTemperatureOversampling(BME680_OS_8X);
        bmx.setHumidityOversampling(BME680_OS_2X);
        bmx.setPressureOversampling(BME680_OS_4X);
        bmx.setIIRFilterSize(BME680_FILTER_SIZE_3);
        bmx.setGasHeater(320, 150); // 320*C for 150 ms
#endif
#ifdef BMP280
      /* Default settings from datasheet. */
      bmx.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                      Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                      Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                      Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                      Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
#endif
        printf("Sensor found on I2C bus\r\n");
      } else {
        printf("Failed to initialize I2C bus\r\n");
      }
    }
    /* Nothing when sleeping */
    if (deepsleep.get() <= 0) {
      strip.fill(strip.Color(0,0,0));
      for (int i=0;i < (PIXEL_COUNT / 2); i++) {
        strip.setPixelColor(0, strip.Color(0,0,128 * rgbDim.get()));
      }
      strip.show();
    }
  } else {
    digitalWrite(WITTY_RGB_R, HIGH);
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
    if ((millis() - mLastButtonAction) > BUTTON_CHECK_INTERVALL) {
      mButtonPressed++;
    }
    if (mButtonPressed > BUTTON_MIN_ACTION_CYCLE) {
      digitalWrite(WITTY_RGB_R, HIGH);
      digitalWrite(WITTY_RGB_B, LOW);
      strip.fill(strip.Color(0,0,0));
      strip.setPixelColor(0, strip.Color((mButtonPressed % 100),0,0));
      strip.setPixelColor(1, strip.Color((mButtonPressed / 100),0,0));
      strip.setPixelColor(2, strip.Color((mButtonPressed / 100),0,0));
      strip.show();
    }
  } else {
    mButtonPressed=0U;
    digitalWrite(WITTY_RGB_R, LOW);
  }

  if (mButtonPressed > BUTTON_MAX_CYCLE) {
    if (SPIFFS.exists("/homie/config.json")) {
      strip.fill(strip.Color(0,PERCENT2FACTOR(127, rgbDim),0));
      strip.show();
      printf("Resetting config\r\n");
      SPIFFS.remove("/homie/config.json");
      SPIFFS.end();
      delay(50);
      Homie.reboot();
    } else {
      printf("No config present\r\n");
      strip.fill(strip.Color(0,0,128));
      strip.show();
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
  Homie.getLogger() << (level) << "@" << (statusCode) << " " << (message) << endl;
}
