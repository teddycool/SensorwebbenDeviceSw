//Only defines for the device in this file 
#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#define SWVERSION "V3.0.0"

// In some hw versions of esp8266 these pin-no are reversed sw-hw pin ie io5 -> pin-io4!
#define DHTPIN 5
#define MODEPIN 4
#define LEDPIN 15
#define PWRPIN 13

#define DHTTYPE 22 // DHT 22 (AM2302)

#define TEST_VOLTAGE 3.3

#define MQTT_LOCAL false // true = local mqtt broker (default), false = remote mqtt connection via Nabu Casa web-hook
#define WIFI_MAX_TRIES 10 // Number of tries to connect to wifi
#define MQTT_MAX_TRIES 10 // Number of tries to connect to mqtt broker
#define MQTT_RETRY_DELAY 1000 // Delay between mqtt connection attempts 
#define DEFAULT_SLEEP_TIME 3600 // Default sleep time in seconds

#endif