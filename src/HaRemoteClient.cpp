/*********************************************************
 * Publishing  messages to a remotely accessed MQTT broker
 * in a Home Assistant instance via NabuCasa webhooks.
 * The webhook needs to be setup in the Home Assistant
 * testautomation.yaml file.
 */
#include "HaRemoteClient.h"
#include "boxsecrets.h"
#include <ESP8266HTTPClient.h>

// Define the access token

HaRemoteClient::HaRemoteClient()
{
}

bool HaRemoteClient::publish(const String &topic, const String &payload, bool retain)
{
    WiFiClientSecure *boxclient = new WiFiClientSecure;
    HTTPClient https;
    boxclient->setInsecure(); // Disable certificate validation for simplicity
    String message = createMessage(topic, payload, retain);
    Serial.println("Message: " + message);
    Serial.print("[HTTPS] begin...\n");
    bool success = false;
    if (https.begin(*boxclient, homeassistant_url)) // HTTPS
    {

        https.addHeader("Content-Type", "application/json");
        https.addHeader("Authorization", "Bearer " + access_token);

        int httpCode = https.POST(message);
        if (httpCode > 0)
        {
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                Serial.println("Payload received from backend");
                Serial.println(https.getString());
            }
            success = true;
        }
        else
        {
            Serial.printf("[HTTPS]... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        https.end();
        boxclient->stop();
        
    }
    return success;
}

String HaRemoteClient::createMessage(const String &topic, const String &payload, bool retain)
{
    String message = "{";
    message += "\"topic\":\"" + topic + "\"" + ",";
    message += "\"payload\":" + payload + ",";
    message += " \"retain\":" + String(retain);
    message += "}";
    return message;
}

HaRemoteClient::~HaRemoteClient()
{
    
}
