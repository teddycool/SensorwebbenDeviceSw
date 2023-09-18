/*********
 * By teddycool,
 *
 *
 *
 *  *********/

#include <ESP8266WiFi.h>      //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>        //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>      //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <PubSubClient.h>

#define DHTPIN 5
#define DHTTYPE DHT11

// Box and users settings:
String chipid; // The unique hw id for each box, actually arduino cpu-id

// Settings, read from Sensorwebben each time
const char *apname = "AP_SENSORWEBBEN";

// Default values for data received from the server
float calfactor = 194.2;                                // Receives this from sebsorwebben
int64_t sleepTimeS = 3600;                      // default sleep-time in seconds between measurements
int maxtrieswifi = 20;                          // default maximum number of retries for wifi connection before giving up
const char *send_option = "mqtt";               // = "url";
const char *postserver = "www.sensorwebben.se"; // default receiving server
const char *postresource = "/post_dvalue.php";  // Receiving script in the backend for posting measurements

char *mqtt_host = "192.168.1.242";
uint64_t mqtt_port = 1883;
const char *mqtt_user = "mqtt";
const char *mqtt_pw = "raspberry";
char *mqtt_topic = "";

const char *server = "www.sensorwebben.se";   // Backend server for reading config
const String sresource = "/post_dstatus.php"; // Receiving script in the backend for posting status-info and get the settingsfile

// Program variables:
WiFiClient boxclient;
PubSubClient mqttClient(boxclient);
ADC_MODE(ADC_TOUT);
bool wificonfig = false;
DynamicJsonDocument json(1024);
DynamicJsonDocument payload(1024);

//*******************************
// Helper functions
String getPostMsg(String pmeasurement, String pvalue, String punit)
{
  String ppost = "GET " + String(postresource) + "?chipid=" + chipid + "&measm=" + pmeasurement + "&value=" + pvalue +
                 "&unit=" + punit + " HTTP/1.1\r\n" + "Host: " + String(server) + "\r\n" + "Connection: close\r\n\r\n";
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
  // Loop until we're reconnected
  while (!mqttClient.connected())
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
      // Wait 5 seconds before retrying
      delay(5000);
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
  mqttClient.setServer(mqtt_host, 1883);
  mqttClient.setCallback(callback);
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
    http.begin(boxclient, url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      String receivedjson = http.getString();
      Serial.println(receivedjson);
      deserializeJson(json, receivedjson);
      JsonObject obj = json.as<JsonObject>();
      // TODO: add some sanity check...
      //  send_option = obj[String("send_option")];
      sleepTimeS = obj[String("sleeptime")];
      calfactor = obj[String("calfactor")];

      postserver = obj[String("url_server")];
      postresource = obj[String("url_resource")];

      //  mqtt_host = obj[String("mqtt_host")];
      //  mqtt_port = obj[String("mqtt_port")];
      //  mqtt_user = obj[String("mqtt_user")];
      //  mqtt_pw = obj[String("mqtt_pw")];
      // mqtt_topic = obj[String("mqtt_topic")];
    }
    else
    {
      Serial.println("An error occurred receiving data from server");
    }
    http.end();

    //***********************
    // MQTT connect
    //***********************

    if (!mqttClient.connected())
    {
      mqttreconnect();
    }
    mqttClient.loop();

    //***********************
    // Make the measurements
    //***********************

    digitalWrite(15, HIGH);
    Serial.println("Reading battery voltage on A0...");
    int batterya = analogRead(A0);
    Serial.println(String(batterya));
    float batv = (float(batterya) / calfactor);
    Serial.println(String(batv) + " V");

    bool measurement = false;

    DHT dht(DHTPIN, DHTTYPE);
    dht.begin();
    Serial.println("Reading from DHT...");
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
    if (send_option == "url")
    {
      boxclient.connect(server, 80);
      boxclient.print(getPostMsg("bat", String(batv), "V"));
      delay(500);
      boxclient.connect(server, 80);
      boxclient.print(getPostMsg("wifi", String(tries), "Count"));
      delay(500);
      boxclient.connect(server, 80);
      boxclient.print(getPostMsg("rssi", signalstr, "raw"));
      delay(500);
      if (measurement)
      {
        boxclient.connect(server, 80);
        boxclient.print(getPostMsg("temp", String(temp), "C"));
        delay(500);
        boxclient.connect(server, 80);
        boxclient.print(getPostMsg("hum", String(hum), "%"));
        delay(500);
      }
    }

    //***********************
    // MQTT send to backend
    //***********************
    if (send_option == "mqtt")
    {
      if (measurement)
      {
        Serial.println("MQTT send...");
        // Prepare payload

        payload["temperature"] = temp;
        payload["humidity"] = hum;
        payload["rssi"] = rssi;
        payload["battery"] = batv;
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