/*********
 * By teddycool, 
 *
 * Add capability to set sleep-time from return-value of http request
 * Add status message for wifi, ssid-name and signal strenght
 * Merged BDC ans BWS software, now they are the same but the type is a part of the settings
 * Future ToDos: BoxType as enum. Settings to move into a file/list for all boxes, use http request and json for values, add api-key or similar
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
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 5      
#define DHTTYPE DHT11 

// Box and users settings:
String boxid;             // The unique hw id for each box, actually arduino cpu-id

// Settings, read from Sensorwebben each time
const char *apname = "AP_SENSORWEBBEN";

//Default values for data received from the server
float calfactor = 200;             
int64_t sleepTimeS = 3600;    // default sleep-time in seconds between measurements
int maxtrieswifi = 20;        // default maximum number of retries for wifi connection before giving up


const char *server = "www.sensorwebben.se";    // Backend server
const char *mresource = "/post_value.php";     // Receiving script in the backend for measurements
String sresource = "/post_status.php";         // Receiving script in the backend for status

// Program variables:
WiFiClient boxclient;
ADC_MODE(ADC_TOUT);
bool wificonfig = false;
DynamicJsonDocument json(1024);


//*******************************
// Helper functions
String getPostMsg(String pmeasurement, String pvalue, String punit)
{
  String ppost = "GET " + String(mresource) + "?devid=" + boxid + "&measm=" + pmeasurement + "&value=" + pvalue +
                 "&unit=" + punit + " HTTP/1.1\r\n" + "Host: " + String(server) + "\r\n" + "Connection: close\r\n\r\n";
  Serial.println("Created measurement http request:");
  Serial.println(ppost);
  return ppost;
}

String getStatusMsg(String ssid, String boxuid)
{
  String spost = String(sresource) + "?devid=" + boxuid + "&ssid=" + ssid;
  Serial.println("Created status url:");
  Serial.println(spost);
  return spost;
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
    
    boxid = ESP.getChipId();
    Serial.println("CHIPID: " + boxid);
    Serial.println("WiFi status: " + String(WiFi.status()));
    Serial.println("WiFi connection ready!");
    Serial.println("Setting up power to sensors...");
    digitalWrite(13, HIGH);
    delay(2000);
    Serial.println("Reading info from wifi access-point...");
    String ssid = WiFi.SSID();
    String signalstr = String(WiFi.RSSI());
    Serial.println("LOCAL IP adress: " + WiFi.localIP().toString());
    Serial.println("Connected to SSID: " + ssid);
    Serial.println("Signal strenght: " + signalstr);

    Serial.println("Reading battery voltage on A0...");
    int batterya = analogRead(A0);
    Serial.println(String(batterya));
    float batv = (float(batterya) / calfactor);
    Serial.println(String(batv) + "V");

    // First send status msg and get sleeptime etc...
    HTTPClient http;
    String url = "http://" + String(server) + getStatusMsg(ssid, boxid);
    Serial.println(url);
    http.begin(boxclient, url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      // TODO: add some sanity check...
      deserializeJson(json, http.getString());
      JsonObject obj = json.as<JsonObject>();
      int32 tempsleep = obj[String("sleeptime")];
      if (tempsleep > 4)
      {
        if (tempsleep < 3601)
        {
          sleepTimeS = tempsleep;
          Serial.println("New sleeptime from server: " + String(sleepTimeS));
        }
      }
    }
    else
    {
      Serial.println("An error occurred receiving data from server");
    }
    http.end();

    // Send common values for all box-types
    boxclient.connect(server, 80);
    boxclient.print(getPostMsg("bat", String(batv), "V"));
    delay(500);
    boxclient.connect(server, 80);
    boxclient.print(getPostMsg("wifi", String(tries), "Count"));
    delay(500);
    boxclient.connect(server, 80);
    boxclient.print(getPostMsg("rssi", signalstr, "raw"));
    delay(500);

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
      boxclient.connect(server, 80);
      boxclient.print(getPostMsg("temp", String(temp), "C"));
      delay(500);
      boxclient.connect(server, 80);
      boxclient.print(getPostMsg("hum", String(hum), "%"));
      delay(500);
    }
  }
  Serial.println("End measuring cycle");

  Serial.println("Turning off power to sensors...");
  digitalWrite(13, LOW);

  // Go to deep-sleep
  String secs = String(sleepTimeS).c_str();
  Serial.println("Going to sleep again....");
  uint32 micros = (uint32)sleepTimeS * 1000000;
  Serial.println("Will sleep for " + secs + " seconds...");
  ESP.deepSleep(micros); // uS!
}