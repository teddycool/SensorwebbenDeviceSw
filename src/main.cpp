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
 * Changes in this version:
 * Added cal-factor to the configfile at setup and used from the file
 * Added DHTTYPE to the configfile at setup and used from the file at runtime
 * Cleaned up the code and fixed some bugs
 * Changed behavior for failed MQTT at setup. Now it will hang with lit LED until reset
 * Updated documentation
 * Removed unused setting for configuration-topic (this might be added later when handled properly)
 *
 * History:
 * ------------ *
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

#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include "DHT.h"
#include <PubSubClient.h>
#include "configmsg.h"
#include "led_blink.h"

// Fixed settings
#define DHTPIN 5               // In some hw versions of esp8266 these pin-no are reversed sw-hw pin ie io5 -> pin-io4!
//#define DHTTYPE DHT22          // DHT 22 (AM2302)
float calfactor = 182;         // Default calibration factor for the battery voltage measurement
const int MAX_WIFI_TRIES = 10; // Maximum number of WiFi connection attempts

WiFiServer server(80);

// Web-portal default configuration parameters:
char mqtt_server[40] = "homeassistant";
char mqtt_port[6] = "1883";
char mqtt_user[20] = "user";
char mqtt_pw[20] = "password";
char sleeptime[5] = "60"; // minutes, 0 means max-time for the chip

char mqtt_ptopic[50] = "";
char mqtt_ctopic[50] = "";
int dht_type = 22;

int64_t sleeptimer;

// Parameters to measure

String ssid;
int wifitries;
String rssi;
String localIp;
int abat;
float vbat;
float temp;
float hum;

String chipid;

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
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);

  if (digitalRead(4) == HIGH)
  { // Config mode selected with the switch...

    Serial.println("Config mode selected");
    pinMode(13, OUTPUT);
    pinMode(15, OUTPUT);
    digitalWrite(13, HIGH);
    digitalWrite(15, HIGH);
    if (SPIFFS.begin())
    {
      Serial.println("Mounted file system");
      if (SPIFFS.exists("/config.json"))
      {
        // file exists, reading and loading
        Serial.println("Reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        size_t size = configFile.size();
        Serial.println("Config file size: " + String(size));
        if (size > 0)
        {
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);
          configFile.readBytes(buf.get(), size);
          DynamicJsonDocument json(2048);
          auto deserializeError = deserializeJson(json, buf.get());
          serializeJson(json, Serial);
          if (!deserializeError)
          {
            Serial.println("Parsed json");
            strcpy(mqtt_server, json["mqtt_server"]);
            strcpy(mqtt_port, json["mqtt_port"]);
            strcpy(mqtt_user, json["mqtt_user"]);
            strcpy(mqtt_pw, json["mqtt_pw"]);
            strcpy(sleeptime, json["sleeptime"]);
            strcpy(mqtt_ptopic, json["mqtt_ptopic"]);
            strcpy(mqtt_ctopic, json["mqtt_ctopic"]);

            dht_type = json["dht_type"].as<int>();
            calfactor = json["calfactor"].as<float>();
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
    WiFiManagerParameter custom_sleeptime("sleeptime", "sleeptime (min)", sleeptime, 5);
    WiFiManagerParameter advanced_set_hd("<h2>Advanced config</h2>");
    WiFiManagerParameter advanced_set_text("<p>Don't touch when using Homeassistant default values!</p>");
    WiFiManagerParameter custom_mqtt_ptopic("ptopic", "mqtt publish-topic", mqtt_ptopic, 50);
  //  WiFiManagerParameter custom_mqtt_ctopic("ctopic", "mqtt configuration-topic", mqtt_ctopic, 50);

    //
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pw);
    wifiManager.addParameter(&custom_sleeptime);

    wifiManager.addParameter(&advanced_set_hd);
    wifiManager.addParameter(&advanced_set_text);
    wifiManager.addParameter(&custom_mqtt_ptopic);
 //   wifiManager.addParameter(&custom_mqtt_ctopic);

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
 //   strcpy(mqtt_ctopic, custom_mqtt_ctopic.getValue());
    strcpy(sleeptime, custom_sleeptime.getValue());

    Serial.println("The values in the file are: ");
    Serial.println("\tmqtt_server : " + String(mqtt_server));
    Serial.println("\tmqtt_port : " + String(mqtt_port));
    Serial.println("\tmqtt_user : " + String(mqtt_user));
    Serial.println("\tmqtt_pw : " + String(mqtt_pw));
    Serial.println("\tsleeptime : " + String(sleeptime));

    Serial.println("\tmqtt_ptopic : " + String(mqtt_ptopic));
    Serial.println("\tmqtt_ctopic : " + String(mqtt_ctopic));

    Serial.println("Saving config");

    DynamicJsonDocument json(2048);
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pw"] = mqtt_pw;
    json["mqtt_ptopic"] = mqtt_ptopic;
    json["mqtt_ctopic"] = mqtt_ctopic;
    json["sleeptime"] = sleeptime;
    json["dht_type"] = dht_type;
    json["calfactor"] = calfactor;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("Failed to open config file for writing");
    }
    else
    {
      Serial.println("Succeded to open config file for writing");
      serializeJson(json, Serial);
      serializeJson(json, configFile);
      Serial.println("Serialization of config file done");
    }
    configFile.close();

    bool connected = wifiManager.autoConnect();
    if (!connected)
    {
      Serial.println("Failed to connect to wifi and hit timeout");
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    wifitries = 1;
    int backoffDelay = 1000;
    while ((WiFi.status() != WL_CONNECTED) && (wifitries < MAX_WIFI_TRIES))
    {
      wifitries++;
      delay(backoffDelay);
      backoffDelay *= 2; // Exponential backoff
    }

    WiFiClient mclient;
    PubSubClient mqttClient(mclient);

    if (WiFi.status() == WL_CONNECTED)
    {

      uint64_t port = strtoull(mqtt_port, nullptr, 10);
      mqttClient.setServer(mqtt_server, port);
      Serial.println("host: " + String(mqtt_server));
      Serial.println("port: " + String(port));

      mqttClient.setCallback(mqttcallback);
      Serial.println("ID:" + String(chipid));
      Serial.println("User:" + String(mqtt_user));
      Serial.println("PW:" + String(mqtt_pw));

      Serial.println("About to send config data to MQTT..");
      int mqtttries = 1;
      while ((!mqttClient.connected()) && (mqtttries < 5))
      {

        Serial.print("Attempting MQTT connection...");

        // Attempt to connect
        if (mqttClient.connect(chipid.c_str(), mqtt_user, mqtt_pw))
        {
          Serial.println("MQTT connected!");
        }
        else
        {
          Serial.print("MQTT connection failed, rc=");
          Serial.print(mqttClient.state());
          delay(2000);
          mqtttries++;
        }
      }

      if (mqttClient.connected())
      {
        mqttClient.setBufferSize(2048);
        mqttClient.loop();

        //----------------------------------
        // MQTT configuration for Homeassistant
        // Create and send config messages

        String configmsg;
        String configmsgtopic;
        Serial.println("-------------------------------------");
        Serial.println("Creating config msg for temperature");
        configmsg = createConfigMsg(chipid, "temperature", "temperature", "°C");
        Serial.println();
        Serial.println("Creating config message topic for temperature");
        configmsgtopic = createConfigMsgTopic(chipid, "temperature");
        Serial.println(configmsgtopic);
        if (!mqttClient.publish(configmsgtopic.c_str(), configmsg.c_str(), true))
        {
          Serial.println("Could not publish config message for temperature!");
          Serial.println("MQTT state: " + String(mqttClient.state()));
          Serial.println("Topic: " + configmsgtopic);
          Serial.println("Message: " + configmsg);
        }

        Serial.println("-------------------------------------");
        Serial.println("Creating config msg for humidity");
        configmsg = createConfigMsg(chipid, "humidity", "humidity", "%");
        Serial.println();
        Serial.println("Creating config message topic for wifitries");
        configmsgtopic = createConfigMsgTopic(chipid, "humidity");
        Serial.println(configmsgtopic);
        if (!mqttClient.publish(configmsgtopic.c_str(), configmsg.c_str(), true))
        {
          Serial.println("Could not publish config message for humidity!");
          Serial.println("MQTT state: " + String(mqttClient.state()));
        }

        Serial.println("-------------------------------------");
        Serial.println("Creating config msg for wifitries");
        configmsg = createConfigMsg(chipid, "none", "wifitries", "");
        Serial.println();
        Serial.println("Creating config message topic for humidity");
        configmsgtopic = createConfigMsgTopic(chipid, "wifitries");
        Serial.println(configmsgtopic);
        if (!mqttClient.publish(configmsgtopic.c_str(), configmsg.c_str(), true))
        {
          Serial.println("Could not publish config message for wifitries!");
          Serial.println("MQTT state: " + String(mqttClient.state()));
        }

        Serial.println("-------------------------------------");
        Serial.println("Creating config msg for Voltage");
        configmsg = createConfigMsg(chipid, "voltage", "battery", "");
        Serial.println();
        Serial.println("Creating config message topic for voltage");
        configmsgtopic = createConfigMsgTopic(chipid, "voltage");
        Serial.println(configmsgtopic);
        if (!mqttClient.publish(configmsgtopic.c_str(), configmsg.c_str(), true))
        {
          Serial.println("Could not publish config message for voltage!");
          Serial.println("MQTT state: " + String(mqttClient.state()));
        }

        Serial.println("-------------------------------------");
        Serial.println("Creating config msg for signal_strength");
        configmsg = createConfigMsg(chipid, "signal_strength", "rssi", "dB");
        Serial.println();
        Serial.println("Creating config message topic for signal_strength");
        configmsgtopic = createConfigMsgTopic(chipid, "rssi");
        Serial.println(configmsgtopic);
        if (!mqttClient.publish(configmsgtopic.c_str(), configmsg.c_str(), true))
        {
          Serial.println("Could not publish config message for signal_strength!");
          Serial.println("MQTT state: " + String(mqttClient.state()));
        }
        Serial.println("-------------------------------------");
        Serial.println("Creating config msg for mqtt-count");
        configmsg = createConfigMsg(chipid, "none", "mqtttries", "");
        Serial.println();
        Serial.println("Creating config message topic for  mqtt-count");
        configmsgtopic = createConfigMsgTopic(chipid, "mqtttries");
        Serial.println(configmsgtopic);
        if (!mqttClient.publish(configmsgtopic.c_str(), configmsg.c_str(), true))
        {
          Serial.println("Could not publish config message for  mqtt-count!");
          Serial.println("MQTT state: " + String(mqttClient.state()));
        }

        Serial.println("-------------------------------------");
        Serial.println("Creating config msg for abat");
        configmsg = createConfigMsg(chipid, "none", "abat", "");
        Serial.println();
        Serial.println("Creating config message topic for  abat");
        configmsgtopic = createConfigMsgTopic(chipid, "abat");
        Serial.println(configmsgtopic);
        if (!mqttClient.publish(configmsgtopic.c_str(), configmsg.c_str(), true))
        {
          Serial.println("Could not publish config message for  abat!");
          Serial.println("MQTT state: " + String(mqttClient.state()));
        }
        mqttClient.disconnect();
      }
      else
      {
        Serial.println("MQTT connection FAILED at setup...");
        Serial.println("Will hang here until reset is pressed...");
        while (true)
        { // Stuck here until config-switch is changed to normal and reset is pressed...
        delay(1000);
        Serial.print("*");
        }
      }
    }
    else
    {
      Serial.println("WIFI connection FAILED at setup...");
      Serial.println("Will hang here until reset is pressed...");
      while (true)
      { // Stuck here until config-switch is changed to normal and reset is pressed...
        delay(1000);
        Serial.print("*");
      }
    }

    // save the custom parameters to FS

    Serial.println("WiFi setup and config- parameters are ready!");
    Serial.println("Waiting for user to change mode-switch and press reset");
    while (true)
    { // Stuck here until config-switch is changed to normal and reset is pressed...
      digitalWrite(15, HIGH);
      delay(1000);
      digitalWrite(15, LOW);
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
  WiFi.begin();
  wifitries = 1;
  Serial.println("Setting up power to sensors...");
  digitalWrite(13, HIGH);
  delay(1000);
  while ((WiFi.status() != WL_CONNECTED) && (wifitries < 10))
  {
    Serial.println("WiFi connect not ready!");
    wifitries = wifitries + 1;
    delay(1000);
  }

  // Measure cycle starts here

  if (WiFi.status() == WL_CONNECTED)
  {
    //***********************
    // Connection and info state
    //***********************
    Serial.println("WiFi connection ready!");
    Serial.println("Starting measuring cycle");
    Serial.println("CHIPID: " + chipid);
    Serial.println("WiFi status: " + String(WiFi.status()));

    Serial.println("Reading info from wifi access-point...");
    ssid = WiFi.SSID();
    rssi = String(WiFi.RSSI());
    localIp = WiFi.localIP().toString();
    Serial.println("Local IP address: " + localIp);
    Serial.println("Connected to SSID: " + ssid);
    Serial.println("Signal strength: " + rssi);

    if (SPIFFS.begin())
    {
      Serial.println("Filesystem mounted");
      Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      size_t size = configFile.size();
      Serial.println("Config file size: " + String(size));
      if (size > 0)
      {
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(2048);
        auto deserializeError = deserializeJson(json, buf.get());

        if (!deserializeError)
        {
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_pw, json["mqtt_pw"]);
          strcpy(mqtt_ptopic, json["mqtt_ptopic"]);
          strcpy(mqtt_ctopic, json["mqtt_ctopic"]);
          strcpy(sleeptime, json["sleeptime"]);
          uint64_t port = json["mqtt_port"].as<uint64_t>();
          sleeptimer = json["sleeptime"].as<int64_t>();
          calfactor = json["calfactor"].as<float>();
          dht_type = json["dht_type"].as<uint8>();

          Serial.println("Parsed json:");
          Serial.println("The values in the file are: ");
          Serial.println("\tmqtt_server : " + String(mqtt_server));
          Serial.println("\tmqtt_port : " + String(mqtt_port));
          Serial.println("\tmqtt_user : " + String(mqtt_user));
          Serial.println("\tmqtt_pw : " + String(mqtt_pw));
          Serial.println("\tmqtt_ptopic : " + String(mqtt_ptopic));
          Serial.println("\tmqtt_ctopic : " + String(mqtt_ctopic));
          Serial.println("\tsleeptime : " + String(sleeptime));
          Serial.println("\tcalfactor : " + String(calfactor));
          Serial.println("\tdht_type : " + String(dht_type));

          Serial.println("Reading battery voltage on A0...");
          abat = analogRead(A0);
          Serial.println(String(abat));
          vbat = (float(abat) / calfactor);
          Serial.println(String(vbat) + " V");

          bool measurement = false;

          if(dht_type == 22){
           Serial.println("DHTTYPE: DHT22");
          }
          else{
           Serial.println("DHTTYPE: DHT11");
          }

          Serial.println("Setting up DHT sensor: " + String(DHTPIN) + " " + String(dht_type));
          DHT dht(DHTPIN,dht_type);
          dht.begin();
          delay(200);
          Serial.println("Reading from DHT...");
          hum = dht.readHumidity();
          temp = dht.readTemperature();
          if (isnan(hum) || isnan(temp))
          {
            Serial.println("Failed to read from DHT sensor!");
            Serial.println("Measuring cycle will be aborted");
          }
          else
          {
            Serial.println("Done reading measurement from DHT sensor!");
            Serial.println("Temperature: " + String(temp) + " C");
            Serial.println("Humidity: " + String(hum) + " %");
            measurement = true;
          }

          if (measurement)
          {
            Serial.println("Setting up for sending mqtt-data..");
            WiFiClient mclient;
            PubSubClient mqttClient(mclient);
            const char *host = mqtt_server;
            mqttClient.setServer(mqtt_server, port);

            mqttClient.setCallback(mqttcallback);
            Serial.println(mqtt_server);
            Serial.println(mqtt_port);
            Serial.println(mqtt_user);

            DynamicJsonDocument mqttpayload(1024);
            Serial.println("About to send data to MQTT..");
            int mqtttries = 1;
            while ((!mqttClient.connected()) && (mqtttries < 5))
            {

              Serial.print("Attempting MQTT connection...");
              // Attempt to connect
              if (mqttClient.connect(chipid.c_str(), mqtt_user, mqtt_pw))
              {
                Serial.println("MQTT connected!");
              }
              else
              {
                Serial.print("MQTT connection failed, rc=");
                Serial.print(mqttClient.state());
                delay(2000);
                mqtttries++;
              }
            }

            if (mqttClient.connected())
            {
              mqttClient.setBufferSize(2048);
              mqttClient.loop();

              Serial.println("MQTT send...");
              // Prepare payload

              mqttpayload["temperature"] = temp;
              mqttpayload["humidity"] = hum;
              mqttpayload["battery"] = vbat;
              mqttpayload["abat"] = abat;
              mqttpayload["rssi"] = rssi;
              mqttpayload["wifitries"] = wifitries;
              mqttpayload["mqtttries"] = mqtttries;
              mqttpayload["localip"] = localIp;
              mqttpayload["ssid"] = ssid;
              String output;
              ArduinoJson::serializeJson(mqttpayload, output);
              if (!mqttClient.publish(mqtt_ptopic, output.c_str()))
              {
                Serial.println("Could not publish MQTT message!");
                Serial.println("MQTT state: " + String(mqttClient.state()));
              }
              Serial.println("MQTT published...");
              Serial.println(output.c_str());
              mqttClient.disconnect();
            }
            else
            {
              Serial.print("MQTT connection FAILED...");
              Serial.println("Blink MQTT-connection-error (3)");
              ledBlink(500, 500, 3);
            }
          }
          else{
            Serial.println("Failed to measure, aborting..");
            Serial.println("Blink measurement-error (6)");
            ledBlink(500, 500, 6);
          }

          Serial.println("End measuring cycle");
        }
      }
      else
      {
        Serial.println("Failed to load json config, aborting..");
        Serial.println("Blink config-file-error (5)");
        ledBlink(500, 500, 5);
      }
    }
    else
    {
      Serial.println("Filesystem failed to mount");
      Serial.println("Blink file-system-error (4)");
      ledBlink(500, 500, 4);
    }
    SPIFFS.end();
  }
  else
  {
    Serial.println("Failed to connect to wifi, aborting..");
    Serial.println("Blink wifi-error (2)");
    ledBlink(500, 500, 2);
  }
  Serial.println("Turning off power to sensors...");
  digitalWrite(15, LOW);
  digitalWrite(13, LOW);

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
