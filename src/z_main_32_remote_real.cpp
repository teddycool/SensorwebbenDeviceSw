
#include <LittleFS.h> // Use LittleFS instead of SPIFFS
#include "boxsecrets.h"
#include "Esp32Config.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include "DiscoveryMsg.h"
#include "DhtSensor.h"
#include "Hx711Sensor.h"
#include "HaRemoteClient.h"
#include "ConfigFile.h"
#include <ArduinoUniqueID.h>
#include "LedBlinker.h"

int rssi;
String localip;
int wifitries;
u_int32_t sleeptime = 60;

// Box and users settings:
String chipid; // The unique hw id for each box, actually arduino cpu-id

float calfactor; // Default calibration factor for the battery voltage measurement
String mqtt_ptopic;
String mqtt_dtopic;

String ssid;

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

void setup()
{
    Serial.begin(9600);
    pinMode(PWRPIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(MODEPIN, INPUT);
    chipid = uids();
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
            digitalWrite(LED_PIN, HIGH);
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

            Serial.println("-------------------------------------");
            Serial.println("Creating config msg for raw weight");
            
            configmsg = DiscoveryMsg::createDiscoveryMsg(chipid, "none", "rweight", "");
            configmsgtopic = DiscoveryMsg::createDiscoveryMsgTopic(chipid, "rweight");
            publish(configmsgtopic, configmsg, true);
            Serial.println("Discovery message: " + configmsg);
            Serial.println("Discovery topic: " + configmsgtopic);
            Serial.println("Discovery message sent");

            Serial.println("Will hang here until swithced to normal mode and rested");

            while (1)
            {
                digitalWrite(LED_PIN, HIGH);
                delay(1000);
                digitalWrite(LED_PIN, LOW);
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
            Serial.println("Reading battery voltage...");
            int batterya = analogRead(ABATPIN);
            Serial.println(String(batterya));

            mqttpayload["calfactor"] = calfactor;
            mqttpayload["abat"] = batterya;
            mqttpayload["battery"] = (batterya / calfactor);

            DhtSensor dhtSensor(DHTPIN, DHTTYPE);
            dhtSensor.performMeasurement();
            dhtSensor.addToPayload(mqttpayload);

            Hx711Sensor hx711Sensor(LOADCELL_DOUT_PIN , LOADCELL_SCK_PIN);
            hx711Sensor.performMeasurement();
            hx711Sensor.addToPayload(mqttpayload);

            if(!hx711Sensor.isSuccess() || !dhtSensor.isSuccess())
            {
                Serial.println("Failed to read from sensors");
                Serial.println("Blink sensor-error (6)");
                LedBlinker::ledBlink(LED_PIN, 500, 500, 6);
            }
            else
            {
                Serial.println("Sensors read successfully");
            }

        }
        else
        {
            Serial.println("Failed to deserialize json file, aborting..");
            Serial.println("Blink config-file-error (5)");
            LedBlinker::ledBlink(LED_PIN, 500, 500, 5);
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
        LedBlinker::ledBlink(LED_PIN, 500, 500, 2);
    }

    Serial.println("Turning off power to sensors...");
    digitalWrite(LED_PIN, LOW);
    digitalWrite(PWRPIN, LOW);

    //FULFIX... remove
    sleeptime = 5;
   
    Serial.println("Going to sleep again....");
    int64_t micros = sleeptime * 60 * 1000000;
    esp_sleep_enable_timer_wakeup(micros);
    Serial.print("Will sleep for ");
    Serial.print(sleeptime);
    Serial.print(" minutes");
    delay(1000);
    Serial.flush();
    esp_deep_sleep_start();
}