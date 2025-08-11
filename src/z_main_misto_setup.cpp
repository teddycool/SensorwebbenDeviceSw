/*********
 * By teddycool,
 * https://github.com/teddycool/SensorwebbenDeviceSw
 * This project contains the setup-software for sensor-device 'Misto', built around ESP8266.
 * The hardware is sold by www.sensorwebben.se

** Purpose:
** 1: test hardware
** 2: calculate calibration-factors
** 3: find device-id
** 4: create the settingsfile for the device
** 5: save the settingsfile to the device (LittleFS)

**********/
#define CHIPTYPE ESP8266 // Define the chip type as ESP8266

#include <LittleFS.h> // Use LittleFS instead of SPIFFS
#include "boxsecrets.h"
#include "Esp8266Config.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DhtSensor.h"
#include "ConfigFile.h"
#include "MqttPublisher.h"
#include <WiFiClient.h>

// Box and users settings:
String chipid; // The unique hw id for each box, actually arduino cpu-id

float calfactor; // Default calibration factor for the battery voltage measurement
String mqtt_ptopic;
String mqtt_dtopic;

// Program variables:

ADC_MODE(ADC_TOUT);

WiFiClient wifiClient; // Create a WiFiClient instance for MQTT communication

//*******************************
// Main program setup and loop

void setup()
{
    Serial.begin(9600);
    pinMode(PWRPIN, OUTPUT);
    pinMode(LEDPIN, OUTPUT);
    chipid = ESP.getChipId();
    Serial.println("CHIPID: " + chipid);

    mqtt_ptopic = String("home/sensor/sw_" + chipid).c_str();
    // TODO: fix correct topic for each entity
    mqtt_dtopic = String("homeassistant/sensor/sw_" + chipid + "/config").c_str();

    WiFi.begin(cssid, cpassword);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Reading info from wifi access-point...");
    String ssid = WiFi.SSID();
    int rssi = WiFi.RSSI();

    Serial.println("Connected to SSID: " + ssid);
    Serial.println("Signal strenght: " + rssi);
    Serial.println("LOCAL IP adress: " + WiFi.localIP().toString());
    String localIp = WiFi.localIP().toString();

    delay(2000);

    Serial.println("format file system for first use");
    LittleFS.format();
}

void loop()
{
    Serial.println("CHIPID: " + chipid);
    Serial.println("Setting up power to sensors...");
    digitalWrite(PWRPIN, HIGH);

    Serial.println("Reading info from wifi access-point...");
    String ssid = WiFi.SSID();
    int rssi = WiFi.RSSI();
    String signalstr = String(rssi);

    Serial.println("Connected to SSID: " + ssid);
    Serial.println("Signal strenght: " + signalstr);
    Serial.println("LOCAL IP adress: " + WiFi.localIP().toString());

    // Allocate a static JsonDocument
    DynamicJsonDocument mqttpayload(1024);
    mqttpayload["chipid"] = chipid;
    mqttpayload["rssi"] = rssi;
    mqttpayload["localip"] = WiFi.localIP().toString();
    mqttpayload["ssid"] = ssid;

    //***********************
    // Make the tests
    //***********************

    Serial.println("Turning on led");
    digitalWrite(LEDPIN, HIGH);
    delay(1000);

    Serial.println("Reading battery voltage on A0...");
    int batterya = analogRead(A0);
    Serial.println(String(batterya));
    Serial.println("Calculating calibration values and present device-id");
    calfactor = batterya / TEST_VOLTAGE;
    Serial.println("Calibration-factor for this box: " + String(calfactor));

    mqttpayload["calfactor"] = calfactor;
    mqttpayload["abat"] = batterya;
    mqttpayload["battery"] = (batterya / calfactor);

    if (digitalRead(MODEPIN) == HIGH)
    {
        Serial.println("Box in WiFi set-up mode");
    }
    else
    {
        Serial.println("Box in Normal mode");
    }

    DhtSensor dhtSensor(DHTPIN, DHTTYPE);
    dhtSensor.performMeasurement();
    if (dhtSensor.isSuccess())
    {
        Serial.println("DHT sensor measurement successful");
        dhtSensor.addToPayload(mqttpayload);
    }
    else
    {
        Serial.println("DHT sensor measurement failed");
    }

    // Create JSON document for settings
    DynamicJsonDocument json(2048);
    json["chipid"] = chipid;
    json["mqtt_server"] = local_mqtt_server;
    json["mqtt_port"] = local_mqtt_port;
    json["mqtt_user"] = local_mqtt_user;
    json["mqtt_pw"] = local_mqtt_pw;
    json["mqtt_ptopic"] = mqtt_ptopic;
    json["sleeptime"] = DEFAULT_SLEEP_TIME;
    json["calfactor"] = calfactor;
    json["dht_type"] = DHTTYPE;

    // Write settings to file
    if (ConfigFile::writeToFile("/config.json", json))
    {
        serializeJson(json, Serial);
        Serial.println("Configuration saved to LittleFS");
    }
    else
    {
        Serial.println("Failed to save configuration");
    }

    // Read settings back from file (optional, for testing)
    DynamicJsonDocument readJson(2048);
    if (ConfigFile::readFromFile("/config.json", readJson))
    {
        Serial.println("Configuration read from LittleFS");
        serializeJson(readJson, Serial);
    }
    else
    {
        Serial.println("Failed to read configuration");
    }

    String payload;
    serializeJson(mqttpayload, payload);
    Serial.println("MQTT-payload: " + payload);
    WiFiClient wifiClient;
    MqttPublisher pubClient = MqttPublisher(wifiClient);
    pubClient.initialize(chipid, local_mqtt_server, 1883, local_mqtt_user, local_mqtt_pw);
    pubClient.publish(mqtt_ptopic.c_str(), payload.c_str(), false);

    Serial.println("End measuring cycle");
    Serial.println("Turning off power to sensors...");
    digitalWrite(LEDPIN, LOW);
    digitalWrite(PWRPIN, LOW);

    Serial.println("Will now sleep for 100 seconds before next cycle ");
    // ESP.deepSleep(10000000); // uS!
    delay(100000); // 100 seconds
}