// This file contains the configuration for the ESP8266 cat-toy device.
// It includes the pin definitions, sensor types, and other device-specific settings.
#ifndef CatToyEsp8266CONFIG_H     
#define CatToyEsp8266CONFIG_H

#define SWVERSION "V1.0.0"
#define COMTYPE "Local MQTT"
#define HWVERSION "V1.0 ESP8266 cat-toy"
#define SENSORMODEL "Sensor Model CatToy" + String(DHTTYPE)

// In some hw versions of esp8266 these pin-no are reversed sw-hw pin ie io5 -> pin-io4!
#define MODEPIN 5
#define MOTOR_PIN 4     // motor MOSFET gate via resistor
#define I2C_SDA   14    // ADXL345 SDA
#define I2C_SCL   12    // ADXL345 SCL


#define TEST_VOLTAGE 3.5

#define MQTT_LOCAL true // true = local mqtt broker (default), false = remote mqtt connection via Nabu Casa web-hook
#define WIFI_MAX_TRIES 10 // Number of tries to connect to wifi
#define MQTT_MAX_TRIES 10 // Number of tries to connect to mqtt broker
#define MQTT_RETRY_DELAY 1000 // Delay between mqtt connection attempts 
#define DEFAULT_SLEEP_TIME 60 // Default sleep time in minutes

#endif