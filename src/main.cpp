/*********
 * By teddycool,
 * https://github.com/teddycool/SensorwebbenDeviceSw
 * This project contains the software for a sensor-device built around ESP8266.
 * The hardware is sold by www.sensorwebben.se
 *
 * License: GPL-3.0
 **********/

#include <ESP8266WiFi.h>     
#include <DNSServer.h>        
#include <ESP8266WebServer.h> 
#include <WiFiManager.h>      //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <PubSubClient.h>

#define DHTPIN 5


// Box and users settings:
String chipid; // The unique hw id for each box, actually arduino cpu-id
const char *apname = "AP_SENSORWEBBEN"; 

// Settings, read from Sensorwebben each start or reset
float calfactor;   
int64_t sleepTimeS = 3600; //default sleep-time
int maxtrieswifi = 20;     
int maxtriesmqtt = 5;
bool send_mqtt = false;
bool send_url = false;
bool send_blink = false;
uint8_t dhttype;

const char *postserver; 
const char *postresource ;  

const char *mqtt_host;
uint64_t mqtt_port;
const char *mqtt_user;
const char *mqtt_pw;
const char *mqtt_topic;

const char *server = "www.sensorwebben.se";   // Backend server for reading config 
const String sresource = "/post_dstatus.php"; // Receiving script in the backend for posting status-info and get the settingsfile

// Program variables:
WiFiClient boxclient;
PubSubClient mqttClient(boxclient);
ADC_MODE(ADC_TOUT);
bool wificonfig = false;

DynamicJsonDocument payload(1024);

//*******************************
// Helper functions
String getPostMsg(String pmeasurement, String pvalue, String punit)
{
  String ppost = "GET " + String(postresource) + "?chipid=" + chipid + "&measm=" + pmeasurement + "&value=" + pvalue +
                 "&unit=" + punit + " HTTP/1.1\r\n" + "Host: " + String(postserver) + "\r\n" + "Connection: close\r\n\r\n";
  Serial.println("Created measurement http request:");
  Serial.println(ppost);
  return ppost;
}

String getStatusMsg(String ssid, String boxuid)
{
  String spost = String(sresource) + "?chipid=" + boxuid + "&ssid=" + ssid;
  Serial.println("Created status url:");
  Serial.println(spost);
  return spost;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void mqttreconnect()
{
  int tries = 0;
  // Loop until we're reconnected
  while ((!mqttClient.connected())  && (tries < maxtriesmqtt))
  {

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("EspMqttClient", mqtt_user, mqtt_pw))
    {
      Serial.println("mqtt connected");
    }
    else
    {
      Serial.print("mqtt connection failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(2000);
      tries++;
    }
  }
}

//*******************************
// Main program setup and loop

void setup()
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);
  Serial.begin(9600);

  if (digitalRead(4) == HIGH)
  { // Config mode selected with the switch...
    digitalWrite(13, HIGH);
    digitalWrite(15, HIGH);
    Serial.println("Config selected!");
    WiFiManager wifiManager;
    wifiManager.startConfigPortal(apname);

    // If you get here you have connected to the WiFi
    Serial.println("WiFi setup ready and connected!");
    while (true)
    { // Stuck here until config-switch is changed to normal and reset is pressed...
      digitalWrite(15, HIGH);
      delay(1000);
      digitalWrite(15, LOW);
      delay(1000);
    }
  }
}

void loop()
{
  Serial.begin(9600);
  int tries = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  tries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (tries < maxtrieswifi))
  {
    Serial.println("WiFi connect not ready!");
    tries = tries + 1;
    delay(1000);
  }

  if (WiFi.status() == WL_CONNECTED)
  {

    //***********************
    // Connection and info state
    //***********************

    chipid = ESP.getChipId();
    Serial.println("CHIPID: " + chipid);
    Serial.println("WiFi status: " + String(WiFi.status()));
    Serial.println("WiFi connection ready!");
    Serial.println("Setting up power to sensors...");
    digitalWrite(13, HIGH);
    delay(2000);
    Serial.println("Reading info from wifi access-point...");
    String ssid = WiFi.SSID();
    int rssi = WiFi.RSSI();
    String signalstr = String(rssi);

    Serial.println("Connected to SSID: " + ssid);
    Serial.println("Signal strenght: " + signalstr);
    Serial.println("LOCAL IP adress: " + WiFi.localIP().toString());

    //***********************
    // Get settings from  sensorwebben backend
    //***********************

    HTTPClient http;
    String url = "http://" + String(server) + getStatusMsg(ssid, chipid);
    Serial.println(url);
    http.useHTTP10(true);
    http.begin(boxclient, url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      StaticJsonDocument<1024> json;
      Serial.println("JSON received from back-end");
      Serial.println("-----------------------");
      DeserializationError error = deserializeJson(json, http.getStream());
      // Test if parsing succeeds.

      
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }
      else{

   
      if (json["dht11"]){
        Serial.println("DHT11 specified in json");
        dhttype= 11;
      };

      if (json["dht22"]){
        Serial.println("DHT22 specified in json");
        dhttype= 22;
      };

      send_mqtt = json["send_mqtt"];
      send_blink = json["send_blink"];
      send_url = json["send_url"];
      sleepTimeS = json["sleeptime"].as<int64_t>();
      calfactor = json["calfactor"].as<float>();
      postserver = json["url_server"];
      postresource = json["url_resource"];
      mqtt_host = json["mqtt_host"];
      mqtt_port = json["mqtt_port"].as<uint64_t>();
      mqtt_user = json["mqtt_user"];
      mqtt_pw = json["mqtt_pw"];
      mqtt_topic = json["mqtt_topic"];
      }
    }
    else
    {
      Serial.println("An error occurred receiving data from server");
    }
    http.end();

    //***********************
    // Make the measurements
    //***********************

    if (send_blink)
    {
      Serial.println("Turning on led");
      digitalWrite(15, HIGH);
    }
    Serial.println("Reading battery voltage on A0...");
    int batterya = analogRead(A0);
    Serial.println(String(batterya));
    float batv = (float(batterya) / calfactor);
    Serial.println(String(batv) + " V");

    bool measurement = false;

    DHT dht(DHTPIN, dhttype);
    dht.begin();
    delay(200);
    Serial.println("Reading from DHT...");
    Serial.println(dhttype);
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    if (isnan(hum) || isnan(temp))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      Serial.println("Done reading measurement from DHT sensor!");
      measurement = true;
    }

    //***********************
    // URL send to backend
    //***********************
    if (send_url)
    {
      Serial.println("Sending data to url..");
      boxclient.connect(server, 80);
      boxclient.print(getPostMsg("bat", String(batv), "V"));
      delay(200);
      boxclient.connect(server, 80);
      boxclient.print(getPostMsg("wifi", String(tries), "Count"));
      delay(200);
      boxclient.connect(server, 80);
      boxclient.print(getPostMsg("rssi", signalstr, "raw"));
      delay(200);
      if (measurement)
      {
        boxclient.connect(server, 80);
        boxclient.print(getPostMsg("temp", String(temp), "C"));
        delay(200);
        boxclient.connect(server, 80);
        boxclient.print(getPostMsg("hum", String(hum), "%"));
        delay(200);
      }
    }

    //***********************
    // MQTT send to backend
    //***********************
    if (send_mqtt)
    {
      mqttClient.setServer(mqtt_host, mqtt_port);
      mqttClient.setCallback(callback);
      Serial.println("MQTT server settings");
      Serial.println(mqtt_host);
      Serial.println(mqtt_port);


      //***********************
      // MQTT connect
      //***********************
      if (measurement)
      {
        Serial.println("Sending data to mqtt..");
        if (!mqttClient.connected())
        {
          mqttreconnect();
        }
        mqttClient.loop();

        Serial.println("MQTT send...");
        // Prepare payload

        payload["temperature"] = temp;
        payload["humidity"] = hum;
        payload["battery"] = batv;
        payload["rssi"] = rssi;        
        payload["wifitries"] = tries;
        String output;
        serializeJson(payload, output);
        mqttClient.publish(chipid.c_str(), output.c_str());
      }
      mqttClient.disconnect();
    }

    //***********************
    // Turn off power
    //***********************
  }
  Serial.println("End measuring cycle");
  Serial.println("Turning off power to sensors...");
  digitalWrite(15, LOW);
  digitalWrite(13, LOW);

  //***********************
  // Go to deep-sleep again
  //***********************
  String secs = String(sleepTimeS).c_str();
  Serial.println("Going to sleep again....");
  uint32 micros = (uint32)sleepTimeS * 1000000;
  Serial.println("Will sleep for " + secs + " seconds...");
  ESP.deepSleep(micros); // uS!
}