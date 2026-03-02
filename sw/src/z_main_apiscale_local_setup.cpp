/*********
 * By teddycool,
 * https://github.com/teddycool/SensorwebbenDeviceSw
 * This project contains the setup-software for a sensor-device built around ESP8266.
 * The hardware is sold by www.sensorwebben.se
 * A project to use when setting up a new ESP8266-device

** Purpose:
** 1: test hardware
** 2: calculate calibration-factors
** 3: find device-id
** 4: create the settingsfile for the device
** 5: save the settingsfile to the device (LittleFS)

**********/
#define CHIPTYPE ESP32 // Define the chip type as ESP32
#include <LittleFS.h> // Use LittleFS instead of SPIFFS
#include "boxsecrets.h"
#include "Esp32Config.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include "DiscoveryMsg.h"
#include "DhtSensor.h"
#include "Hx711Sensor.h"
#include "MqttPublisher.h"
#include "ConfigFile.h"
#include <ArduinoUniqueID.h>


// Box and users settings:
String chipid; // The unique hw id for each box, actually arduino cpu-id

float calfactor; // Default calibration factor for the battery voltage measurement
String mqtt_ptopic;
String mqtt_dtopic;

// Unique ID as a string
String uids()
{
  String uidds;
  for (size_t i = 0; i < UniqueIDsize; i++)
  {
    if (UniqueID[i] < 0x10)
    {
      uidds = uidds + "0";
    }
    uidds = uidds + String(UniqueID[i], HEX);
  }
  return uidds;
}

//*******************************
// Main program setup and loop

void setup()
{
  Serial.begin(9600);
  pinMode(PWRPIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  chipid = uids();
  Serial.println("CHIPID: " + chipid);

  mqtt_ptopic = "home/sensor/sw_" + chipid;

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
  LittleFS.begin();
  Serial.println("LittleFS format");
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
  digitalWrite(LED_PIN, HIGH);
  delay(1000);

  Serial.println("Reading battery voltage on A0...");
  int batterya = analogRead(ABATPIN);
  Serial.println(String(batterya));
  Serial.println("Calculating calibration values and present device-id");
  calfactor = batterya / TEST_VOLTAGE;
  Serial.println("Calibration-factor for this box: " + String(calfactor));

  mqttpayload["calfactor"] = calfactor;
  mqttpayload["abat"] = analogRead(ABATPIN);
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

  Hx711Sensor hx711Sensor(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  hx711Sensor.performMeasurement(); 
  if (hx711Sensor.isSuccess())
  {
    Serial.println("HX711 sensor measurement successful");
    hx711Sensor.addToPayload(mqttpayload);
  }
  else
  {
    Serial.println("HX711 sensor measurement failed");
  }
  

  // Create JSON document for settings
  DynamicJsonDocument json(2048);
  json["chipid"] = chipid;
  json["mqtt_server"] = local_mqtt_server;
  json["mqtt_port"] = local_mqtt_port;
  json["mqtt_user"] = local_mqtt_user;
  json["mqtt_pw"] = local_mqtt_pw;
  json["mqtt_ptopic"] = mqtt_ptopic;
  json["mqtt_dtopic"] = mqtt_dtopic;
  json["sleeptime"] = DEFAULT_SLEEP_TIME_SEC;
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
    serializeJson(readJson, Serial);
    Serial.println("Configuration read from LittleFS");
  }
  else
  {
    Serial.println("Failed to read configuration");
  }

  // Publisher *pubClient = nullptr;
  Serial.println("MQTT_LOCAL is set to false");
  Serial.println("Sending to remote mqtt broker");
  String payload;
  serializeJson(mqttpayload, payload);
  Serial.println("Payload: " + payload);
  Serial.println("Topic: " + mqtt_ptopic);
  publish(mqtt_ptopic, payload, false);
  Serial.println("MQTT publish done");
  Serial.println("End measuring cycle");
  Serial.println("Turning off power to sensors...");
  digitalWrite(LED_PIN, LOW);
  digitalWrite(PWRPIN, LOW);

  Serial.println("Will now sleep for 10 seconds before next cycle ");
  // ESP.deepSleep(10000000); // uS!
  delay(10000); // 10 seconds
}