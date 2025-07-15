/******************************************************************************
 * By teddycool,
 * https://github.com/teddycool/
 * This project contains the software for a sensor-device built around ESP8266.
 * The software and the sensor-circuit-diagram are released under GPL-3.0
 * Pre-built hardware is sold by www.sensorwebben.se and www.biwebben.se
 * The sw makes heavy use of https://github.com/tzapu/WiFiManager for
 *  the wifi- and mqtt-configuration
 *
 * SW v2.2.2 2025-01-20
 * License: GPL-3.0
 ******************************************************************************
 * History:
 * ------------ *
 * v 3.0.0:
 * Added support for remote connection to Nabu Casa via web-hook
 * Still a work in progress.
 *
 *
 *
 * v 2.2.2:
 * Added cal-factor to the configfile at setup and used from the file
 * Added DHTTYPE to the configfile at setup and used from the file at runtime
 * Cleaned up the code and fixed some bugs
 * Changed behavior for failed MQTT at setup. Now it will hang with lit LED until reset
 * Updated documentation
 * Removed unused setting for configuration-topic (this might be added later when handled properly)
 *
 * v 2.2.1:
 * Added MQTT discovery message for auto-config in Homeassistant
 * Restructured to be easier to follow the 'flow' in the code
 * Added buffer for the mqtt messages to avoid send failure
 * Restructured with help of Copilot from Github
 *
 * v 2.2.0:
 * Completely rewritten and only local configuration via the ap web-gui
 * The connection to sensorwebben.se completely removed
 * Currently only works with mqtt
 * OTA is enabled but not yet working fully, sorry...
 */
#define CHIPTYPE ESP8266 // Define the chip type as ESP8266
#include <LittleFS.h> // Use LittleFS instead of SPIFFS
#include <ESP8266WiFi.h>
#include "Esp8266Config.h"
#include <DNSServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include "DiscoveryMsg.h"
#include "DhtSensor.h"
#include "MqttPublisher.h"
#include "HaRemoteClient.h"
#include "LedBlinker.h"
#include "boxsecrets.h" // WiFi credentials and MQTT settings for remote connection
#include "DiscoveryClient.h"

WiFiServer server(80);

int64_t sleeptimer;

// Parameters to measure

String ssid;
int wifitries;
String rssi;
String localIp;
int abat;
float vbat;
float calfactor;

char *mqtt_server;
char *mqtt_port;
char *mqtt_user;
char *mqtt_pw;
char *mqtt_ptopic;
char *mqtt_dtopic;
char *cchipid;

String chipid;
LedBlinker ledBlinker(LEDPIN); // LED blinker for status indication

// flag for saving data
bool shouldSaveConfig = false;

// callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void mqttcallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Setup starts..");
  Serial.println("Software version: " + String(SWVERSION));
  chipid = ESP.getChipId();
  Serial.println("CPUID: " + chipid);
  pinMode(PWRPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);

  if ((MQTT_LOCAL == true) && (digitalRead(MODEPIN) == HIGH))
  {
    Serial.println("MQTT_LOCAL is set to true and config-mode selected");

    Serial.println("Config mode selected");
    digitalWrite(PWRPIN, HIGH);
    digitalWrite(LEDPIN, HIGH);
    if (LittleFS.begin())
    {
      Serial.println("Mounted file system");
      if (LittleFS.exists("/config.json"))
      {
        // file exists, reading and loading
        Serial.println("Reading config file");
        File configFile = LittleFS.open("/config.json", "r");
        size_t size = configFile.size();
        Serial.println("Config file size: " + String(size));
        if (size > 0)
        {
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);
          configFile.readBytes(buf.get(), size);
          JsonDocument json;
          auto deserializeError = deserializeJson(json, buf.get());
          Serial.println("Deserialization done");
          Serial.println("Config-file json: " + json);

          if (!deserializeError)
          {
            Serial.println("Parsed json");
            strcpy(mqtt_server, json["mqtt_server"]);
            strcpy(mqtt_port, json["mqtt_port"]);
            strcpy(mqtt_user, json["mqtt_user"]);
            strcpy(mqtt_pw, json["mqtt_pw"]);
            strcpy(mqtt_ptopic, json["mqtt_ptopic"]);
            strcpy(mqtt_dtopic, json["mqtt_dtopic"]);
            calfactor = json["calfactor"].as<float>();
            sleeptimer = json["sleeptime"].as<int64_t>();
          }
          else
          {
            Serial.println("Failed to load json config. Using defult values.");
          }
        }
        else
        {
          Serial.println("Config-file was empty. Using defult values ");
          String ptopic = "home/sensor/sw_" + chipid;
          strcpy(mqtt_ptopic, ptopic.c_str());
        }
        configFile.close();
      }
    }
    else
    {
      Serial.println("Failed to mount FS");
    }
    // end read

    // Setting
    if (strlen(mqtt_ptopic) < 2)
    {
      Serial.println("Setting default mqtt topic");
      String topic = "home/sensor/sw_" + chipid;
      strcpy(mqtt_ptopic, topic.c_str());
    }

    WiFiManager wifiManager;

    Serial.println("Setting up wifimanager parameters..");
    // id/name, placeholder/prompt, default, length
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt server port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_user("user", "mqtt server user", mqtt_user, 20);
    WiFiManagerParameter custom_mqtt_pw("pw", "mqtt server pw", mqtt_pw, 20);

    char sleeptimerStr[8];
    snprintf(sleeptimerStr, sizeof(sleeptimerStr), "%lld", sleeptimer);    
    WiFiManagerParameter custom_sleeptime("sleeptimer", "sleeptime in min", sleeptimerStr, 5);
    WiFiManagerParameter advanced_set_hd("<h2>Advanced config</h2>");
    WiFiManagerParameter advanced_set_text("<p>Don't touch when using Homeassistant default values!</p>");
    WiFiManagerParameter custom_mqtt_ptopic("ptopic", "mqtt publish-topic", mqtt_ptopic, 50);
    //  WiFiManagerParameter custom_mqtt_dtopic("ctopic", "mqtt configuration-topic", mqtt_dtopic, 50);

    //
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pw);
    wifiManager.addParameter(&custom_sleeptime);

    wifiManager.addParameter(&advanced_set_hd);
    wifiManager.addParameter(&advanced_set_text);
    wifiManager.addParameter(&custom_mqtt_ptopic);
    //   wifiManager.addParameter(&custom_mqtt_dtopic);

    // set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    char apname[chipid.length() + 1];
    for (int x = 0; x < chipid.length(); x++)
    {
      apname[x] = chipid[x];
    }
    apname[chipid.length()] = '\0';

    // and go into a blocking loop awaiting configuration
    wifiManager.startConfigPortal(apname);

    Serial.println("Connected !");

    // read updated parameters from portal
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_pw, custom_mqtt_pw.getValue());
    strcpy(mqtt_ptopic, custom_mqtt_ptopic.getValue());
    //   strcpy(mqtt_dtopic, custom_mqtt_dtopic.getValue());
    sleeptimer = atoll(custom_sleeptime.getValue());

    Serial.println("The values in the file are: ");
    Serial.println("\tmqtt_server : " + String(mqtt_server));
    Serial.println("\tmqtt_port : " + String(mqtt_port));
    Serial.println("\tmqtt_user : " + String(mqtt_user));
    Serial.println("\tmqtt_pw : " + String(mqtt_pw));
    Serial.println("\tsleeptime : " + String(sleeptimer));

    Serial.println("\tmqtt_ptopic : " + String(mqtt_ptopic));
    Serial.println("\tmqtt_dtopic : " + String(mqtt_dtopic));

    Serial.println("Saving config");

    JsonDocument json;
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pw"] = mqtt_pw;
    json["mqtt_ptopic"] = mqtt_ptopic;
    json["mqtt_dtopic"] = mqtt_dtopic;
    json["sleeptime"] = sleeptimer;
    json["calfactor"] = calfactor;

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("Failed to open config file for writing");
    }
    else
    {
      Serial.println("Succeded to open config file for writing");
      //  serializeJson(json, Serial);
      serializeJson(json, configFile);
      Serial.println("Serialization of config to file done");
    }
    configFile.close();

    bool connected = wifiManager.autoConnect();
    if (!connected)
    {      Serial.println("Failed to connect to wifi and hit timeout");
      
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    wifitries = 1;
    int backoffDelay = 1000;
    while ((WiFi.status() != WL_CONNECTED) && (wifitries < WIFI_MAX_TRIES))
    {
      wifitries++;
      delay(backoffDelay);
      backoffDelay *= 2; // Exponential backoff
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi connected");
      Serial.println("IP address: " + WiFi.localIP().toString());
      Serial.println("Signal strength: " + String(WiFi.RSSI()) + " dBm");
      Serial.println("WiFi SSID: " + String(WiFi.SSID()));
      Serial.println("Starting to send config messages..");

      MqttPublisher pubClient;
      pubClient.initialize(chipid, mqtt_server, atoi(mqtt_port), mqtt_user, mqtt_pw);

      // Use DiscoverySession for all discovery/config messages
      DiscoveryClient dclient(pubClient, chipid);

      dclient.sendTemperature();
      dclient.sendHumidity();
      dclient.sendWifitries();
      dclient.sendBattery();
      dclient.sendSignalStrength();
      dclient.sendMqttTries();
      dclient.sendAbat();
    }
    else
    {
      Serial.println("WIFI connection FAILED at setup...");
      Serial.println("Will hang here until reset is pressed...");
      while (true)
      {
        delay(1000);
        Serial.print("*");
      }
    }

    // save the custom parameters to FS

    Serial.println("WiFi setup and config- parameters are ready!");
    Serial.println("Waiting for user to change mode-switch and press reset");
    while (true)
    {
      digitalWrite(LEDPIN, HIGH);
      delay(1000);
      digitalWrite(LEDPIN, LOW);
      delay(1000);
    }
  }
  else
  {
    Serial.println("Config mode was not selected, starting measurement loop...");
  }
}

void loop()
{
  WiFi.mode(WIFI_STA);
  if (!MQTT_LOCAL)
  {
    WiFi.begin(cssid, cpassword);
  }
  else
  {
    WiFi.begin();
  }
  Serial.println("Connecting");

  wifitries = 1;

  while ((WiFi.status() != WL_CONNECTED) && (wifitries < 10))
  {
    Serial.println("WiFi connect not ready!");
    wifitries = wifitries + 1;
    delay(1000);
  }
  Serial.println("Setting up power to sensors...");
  digitalWrite(PWRPIN, HIGH);
  delay(1000);
  // Measure cycle starts here

  if (WiFi.status() == WL_CONNECTED)
  {
    //***********************
    // Connection and info state
    //***********************
    Serial.println("Setting up power to sensors...");
    digitalWrite(PWRPIN, HIGH);

    Serial.println("Reading info from wifi access-point...");
    String ssid = WiFi.SSID();
    int rssi = WiFi.RSSI();
    localIp = WiFi.localIP().toString();
    Serial.println("Local IP address: " + localIp);
    Serial.println("Connected to SSID: " + ssid);
    Serial.println("Signal strength: " + rssi);

    // Allocate a static JsonDocument and start adding data to mqttpayload
    StaticJsonDocument<1024> mqttpayload;
    mqttpayload["chipid"] = chipid;
    mqttpayload["rssi"] = rssi;
    mqttpayload["localip"] = localIp;
    mqttpayload["ssid"] = ssid;
    mqttpayload["wifitries"] = wifitries;

    Serial.println("WiFi connection ready!");
    Serial.println("Starting measuring cycle");
    Serial.println("WiFi status: " + String(WiFi.status()));

    if (LittleFS.begin())
    {
      cchipid = new char[10];
      mqtt_server = new char[40];
      mqtt_user = new char[20];
      mqtt_pw = new char[20];
      mqtt_ptopic = new char[50];
      mqtt_dtopic = new char[50];
      Serial.println("Filesystem mounted");
      Serial.println("Reading config file");
      File configFile = LittleFS.open("/config.json", "r");
      if (!configFile)
      {
        Serial.println("Failed to load json config, aborting..");
        Serial.println("Blink config-file-error (5)");
        ledBlinker.ledBlink(500, 500, 5);
      }
      else
      {
        size_t size = configFile.size();
        Serial.println("Config file size: " + String(size));
        if (size > 0)
        {
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);
          configFile.readBytes(buf.get(), size);
          DynamicJsonDocument json(2048);
          Serial.println("Bytes read: " + String(size));
          auto deserializeError = deserializeJson(json, buf.get());
          serializeJson(json, Serial);

          if (!deserializeError)
          {
            Serial.println("JSON parsed successfully");
            Serial.println("Parsed json:");
            Serial.println("The values in the file are: ");

            strcpy(cchipid, json["chipid"]);
            Serial.println("\tchipid from file : " + String(cchipid));
            strcpy(mqtt_server, json["mqtt_server"]);
            Serial.println("\tmqtt_server : " + String(mqtt_server));
            strcpy(mqtt_user, json["mqtt_user"]);
            Serial.println("\tmqtt_user : " + String(mqtt_user));
            strcpy(mqtt_pw, json["mqtt_pw"]);
            Serial.println("\tmqtt_pw : " + String(mqtt_pw));
            strcpy(mqtt_ptopic, json["mqtt_ptopic"]);
            Serial.println("\tmqtt_ptopic : " + String(mqtt_ptopic));
            strcpy(mqtt_dtopic, json["mqtt_dtopic"]);
            Serial.println("\tmqtt_dtopic : " + String(mqtt_dtopic));
            uint64_t port = json["mqtt_port"].as<uint64_t>();
            sleeptimer = json["sleeptime"].as<int64_t>();
            Serial.println("\tsleeptime : " + String(sleeptimer));
            calfactor = json["calfactor"].as<float>();
            Serial.println("\tcalfactor : " + String(calfactor));

            Serial.println("Reading battery voltage on A0...");
            abat = analogRead(A0);
            Serial.println(String(abat));
            vbat = (float(abat) / calfactor);
            Serial.println(String(vbat) + " V");
            mqttpayload["battery"] = vbat;
            mqttpayload["abat"] = abat;
            mqttpayload["calfactor"] = calfactor;

            DhtSensor dhtSensor(DHTPIN, DHTTYPE);
            dhtSensor.performMeasurement();
            dhtSensor.addToPayload(mqttpayload);
            Serial.println("DHT sensor data added to payload");
            Serial.println("MQTT message payload:");
            serializeJson(mqttpayload, Serial);
            Serial.println();
            Serial.println("MQTT message topic: " + String(mqtt_ptopic));
            Publisher *pubClient = nullptr;
            Serial.println("Publisher created!");
            Serial.println("MQTT_LOCAL is set to false");
            Serial.println("MQTT connection will be done via Nabu Casa web-hook");
            pubClient = new HaRemoteClient();
            bool mqttsuccess = pubClient->publish(mqtt_ptopic, mqttpayload.as<String>(), false);
            if (mqttsuccess)
            {
              Serial.println("MQTT message published with remote client!");
            }
            else
            {
              Serial.println("MQTT message published failed!");
              Serial.println("Blink mqtt-error (3)");
              ledBlinker.ledBlink(500, 500, 3);
            }
            delete pubClient;
            Serial.println("Publisher deleted!");
          }
        }
        else
        {
          Serial.println("Failed to deserialize json file, aborting..");
          Serial.println("Blink config-file-error (5)");
          ledBlinker.ledBlink(500, 500, 5);
        }

        configFile.close();
      }
    }
    else
    {
      Serial.println("Filesystem failed to mount");
      Serial.println("Blink file-system-error (4)");
      ledBlinker.ledBlink(500, 500, 4);
    }

    LittleFS.end();
  }

  else
  {
    Serial.println("WiFi connection failed!");
    Serial.println("Blink wifi-connection-error (3)");
    ledBlinker.ledBlink(500, 500, 3);
  }

  Serial.println("Turning off power to sensors...");
  digitalWrite(LEDPIN, LOW);
  digitalWrite(PWRPIN, LOW);

  int64_t micros;
  if (sleeptimer == 0)
  {
    micros = ESP.deepSleepMax();
    String minutes = String(micros / 1000000 / 60);
    Serial.println("Will sleep for the maximum time: " + minutes + " minutes...");
  }
  else
  {
    micros = sleeptimer * 1000000 * 60;
    Serial.println("Will sleep for " + String(sleeptimer) + " minutes...");
  }
  Serial.println("Going to sleep again....");
  ESP.deepSleep(micros); // uS!
}
