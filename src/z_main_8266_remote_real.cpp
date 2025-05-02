/******************************************************************************
 * By teddycool,
 * https://github.com/teddycool/
 * This project contains the software for a sensor-device built around ESP8266.
 * The software and the sensor-circuit-diagram are released under GPL-3.0
 * Pre-built hardware is sold by www.sensorwebben.se and www.biwebben.se
 * The sw makes heavy use of https://github.com/tzapu/WiFiManager for
 *  the wifi- and mqtt-configuration
 *
 * This is the version for 'remote' connection to Nabu Casa via web-hook
 * All settings in boxsecrets.cpp with definitions in boxsecrets.h
 * No setup with webportal needed. The setup-mode connects to the predefined
 * wifi and sends the required discovery messages to the mqtt broker via the
 * webhook to Nabu Casa. The device will then be available in Home Assistant.
 *
 * SW v3.0.0 2025-05-01
 * License: GPL-3.0
 ******************************************************************************
 * History:
 * ------------ *
 * v 3.0.0:
 *
 *

**********/

#include <LittleFS.h> // Use LittleFS instead of SPIFFS
#include "boxsecrets.h"
#include "deviceconfig.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DiscoveryMsg.h"
#include "DhtSensor.h"
#include "HaRemoteClient.h"
#include "ConfigFile.h"
#include "ledblink.h"

// Box and users settings:
String chipid; // The unique hw id for each box, actually arduino cpu-id

float calfactor; // Default calibration factor for the battery voltage measurement
String mqtt_ptopic;
String mqtt_dtopic;
String ssid;
int rssi;
String localip;
int wifitries;
u_int32_t sleeptime=60; //Default sleep time in minutes
// Program variables:

ADC_MODE(ADC_TOUT);

//*******************************
// Main program setup and loop

void setup()
{
  Serial.begin(9600);
  pinMode(PWRPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(MODEPIN, INPUT);
  chipid = ESP.getChipId();
  Serial.println("CHIPID: " + chipid);

  WiFi.begin(cssid, cpassword);
  Serial.println("Connecting");
  wifitries = 1;
  Serial.println("WiFi connect not ready!");
  while ((WiFi.status() != WL_CONNECTED) && (wifitries < WIFI_MAX_TRIES))
  {
    Serial.print("*");
    wifitries = wifitries + 1;
    delay(1000);
  }

  if ((WiFi.status() == WL_CONNECTED))
  {
    if (digitalRead(MODEPIN) == HIGH)
    {
      digitalWrite(PWRPIN, HIGH);
      digitalWrite(LEDPIN, HIGH);
      Serial.println("Config-mode selected");
      Serial.println("Will send config-messages to mqtt broker");

      DiscoveryMsg discoveryMsg;
      String configmsg;
      String configmsgtopic;

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for temperature");
      configmsg = discoveryMsg.createDiscoveryMsg(chipid, "temperature", "temperature", "°C");
      configmsgtopic = discoveryMsg.createDiscoveryMsgTopic(chipid, "temperature");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for humidity");
      configmsg = discoveryMsg.createDiscoveryMsg(chipid, "humidity", "humidity", "%");
      configmsgtopic = discoveryMsg.createDiscoveryMsgTopic(chipid, "humidity");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for wifitries");
      configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "none", "wifitries", "");
      configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "wifitries");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for Voltage");
      configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "voltage", "battery", "");
      configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "battery");
      Serial.println(configmsgtopic);
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for signal_strength");
      configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "signal_strength", "rssi", "dB");
      configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "rssi");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for raw battery reading");
      configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "none", "abat", "");
      configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "abat");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for local ip");
      configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "none", "localip", "none");
      configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "localip");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for ssid");
      configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "none", "ssid", "none");
      configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "ssid");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for chipid");
      configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "none", "chipid", "none");
      configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "chipid");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");

      Serial.println("-------------------------------------");
      Serial.println("Creating config msg for calfactor");
      configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "none", "calfactor", "");
      configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "calfactor");
      publish(configmsgtopic, configmsg, true);
      Serial.println("Discovery message: " + configmsg);
      Serial.println("Discovery topic: " + configmsgtopic);
      Serial.println("Discovery message sent");
      Serial.println("Will hang here until swithced to normal mode and rested");
      while (1)
      {
        digitalWrite(LEDPIN, HIGH);
        delay(1000);
        digitalWrite(LEDPIN, LOW);
        delay(1000);
      }
    }
    else
    {
      Serial.println("Normal mode selected");
      Serial.println("Starting measuring cycle...");
    }
  }
  else
  {
    Serial.println("Not connected to wifi!");
  }
}

void loop()
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    Serial.println("");
    Serial.println("Reading info from wifi access-point...");
    ssid = WiFi.SSID();
    rssi = WiFi.RSSI();
    localip = WiFi.localIP().toString();
    Serial.println("Connected to SSID: " + ssid);
    Serial.println("Signal strenght: " + rssi);
    Serial.println("LOCAL IP adress: " + localip);
    String localIp = WiFi.localIP().toString();

    mqtt_ptopic = "home/sensor/sw_" + chipid;
    Serial.println("Setting up power to sensors...");
    digitalWrite(PWRPIN, HIGH);

    // Allocate a static JsonDocument
    DynamicJsonDocument mqttpayload(1024);
    mqttpayload["chipid"] = chipid;
    mqttpayload["rssi"] = rssi;
    mqttpayload["localip"] = localip;
    mqttpayload["wifitries"] = wifitries;
    mqttpayload["ssid"] = ssid;

    // Serialize the JSON document to a String
    String payload;

    // Read settingsfile from LittleFS

    Serial.println("Reading settings from LittleFS...");
    DynamicJsonDocument readJson(2048);
    if (ConfigFile::readFromFile("/config.json", readJson))
    {
      Serial.println("Configuration read from LittleFS");
      serializeJson(readJson, Serial);     

      calfactor = readJson["calfactor"];
      sleeptime = readJson["sleeptime"];
      mqtt_ptopic = readJson["mqtt_ptopic"].as<String>();
      Serial.println("Reading battery voltage on A0...");
      int batterya = analogRead(A0);
      Serial.println(String(batterya));

      mqttpayload["calfactor"] = calfactor;
      mqttpayload["abat"] = analogRead(A0);
      mqttpayload["battery"] = (batterya / calfactor);

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
        Serial.println("Failed to deserialize json file, aborting..");
        Serial.println("Blink config-file-error (6)");
        ledBlink(500, 500, 6);
      }
    }
    else
    {
      Serial.println("Failed to deserialize json file, aborting..");
      Serial.println("Blink config-file-error (5)");
      ledBlink(500, 500, 5);
    }

    serializeJson(mqttpayload, payload);

    // Use the serialized String for concatenation or printing
    Serial.println("Payload: " + payload);

    Serial.println("Sending to remote mqtt broker");
    Serial.println("Topic: " + mqtt_ptopic);
    publish(mqtt_ptopic, payload, false);
    Serial.println("MQTT publish done");
    Serial.println("End measuring cycle");
  }

  else
  {
    Serial.println("Not connected to wifi!");
    Serial.println("Blink wifi error (2)");
    ledBlink(500, 500, 2);
  }

  Serial.println("Turning off power to sensors...");
  digitalWrite(LEDPIN, LOW);
  digitalWrite(PWRPIN, LOW);

  Serial.println("Will now deep-sleep for " + String(sleeptime) + " minutes");
  uint32_t sleeptimeus = sleeptime * 60 * 1000000;
  Serial.println("Will now deep-sleep for " + String(sleeptimeus) + " micro-seconds");
  ESP.deepSleep(sleeptimeus); // uS!
}