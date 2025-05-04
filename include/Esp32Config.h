// This file contains the configuration for the ESP32 device.
// It includes the pin definitions, sensor types, and other device-specific settings.
#ifndef ESP32CONFIG_H
#define ESP32CONFIG_H

#define SWVERSION "V3.0.1"
#define COMTYPE "Remote Client"
#define HWVERSION "V4.2 ESP32"
#define SENSORMODEL "Sensor Model BWS"

#define DHTPIN 27
#define MODEPIN 5
#define LED_PIN 15  //renamed to not interfere from LED_BUILTIN definition
#define PWRPIN 18
#define ABATPIN 34
#define LOADCELL_DOUT_PIN 14
#define LOADCELL_SCK_PIN 12

#define DHTTYPE 22 // DHT 11 or 22 (AM2302)

#define TEST_VOLTAGE 4.5

#define MQTT_LOCAL false // true = local mqtt broker (default), false = remote mqtt connection via Nabu Casa web-hook
#define WIFI_MAX_TRIES 10 // Number of tries to connect to wifi
#define MQTT_MAX_TRIES 10 // Number of tries to connect to mqtt broker
#define MQTT_RETRY_DELAY 1000 // Delay between mqtt connection attempts 
#define DEFAULT_SLEEP_TIME 5 // Default sleep time in minutes

#endif