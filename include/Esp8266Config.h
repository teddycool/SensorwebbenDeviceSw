// This file contains the configuration for the ESP8266 device.
// It includes the pin definitions, sensor types, and other device-specific settings.
#ifndef ESP8266CONFIG_H     
#define ESP8266CONFIG_H

#define DHTTYPE 11 // DHT 11 or 22 (AM2302)

#define SWVERSION "V3.0.0"
#define COMTYPE "Local MQTT"
#define HWVERSION "V5.0 ESP8266"
#define SENSORMODEL "Sensor Model Misto DHT" + String(DHTTYPE)

// In some hw versions of esp8266 these pin-no are reversed sw-hw pin ie io5 -> pin-io4!
#define DHTPIN 5
#define MODEPIN 4
#define LEDPIN 15
#define PWRPIN 13



#define TEST_VOLTAGE 3.5

#define MQTT_LOCAL true // true = local mqtt broker (default), false = remote mqtt connection via Nabu Casa web-hook
#define WIFI_MAX_TRIES 10 // Number of tries to connect to wifi
#define MQTT_MAX_TRIES 10 // Number of tries to connect to mqtt broker
#define MQTT_RETRY_DELAY 1000 // Delay between mqtt connection attempts 
#define DEFAULT_SLEEP_TIME 60 // Default sleep time in minutes

#endif