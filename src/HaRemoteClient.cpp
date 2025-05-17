/*********************************************************
 * Publishing  messages to a remotely accessed MQTT broker
 * in a Home Assistant instance via NabuCasa webhooks.
 * The webhook needs to be setup in the Home Assistant
 * testautomation.yaml file.
 */
#include "HaRemoteClient.h"
#include "boxsecrets.h"
#include <WiFiClientSecure.h>

#include <Arduino.h>

WiFiClientSecure client;

bool publish(const String &topic, const String &payload, bool retain)
{    
    client.setInsecure(); // Disable certificate validation for simplicity
    String message = createMessage(topic, payload, retain);
    Serial.println("Message: " + message);
    Serial.print("[HTTPS] begin...\n");
    Serial.print("Connecting to: ");
    Serial.println(homeassistant_host);
    Serial.print("Connecting to: ");
    Serial.println(homeassistant_uri);
    Serial.print("Connecting to: ");
    Serial.println(homeassistant_full_url);

    Serial.print("DNS Server: ");
    Serial.println(WiFi.dnsIP());

    // Resolve host
    IPAddress ip;
    if (WiFi.hostByName(homeassistant_host, ip))
    {
        Serial.print("Resolved IP: ");
        Serial.println(ip);
    }
    else
    {
        Serial.println("Failed to resolve host");
    }

    if (!client.connect(homeassistant_host, 443))
    {
        Serial.println("Connection to Home Assistant failed!");
        return false;
    }

    String request = String("POST ") + homeassistant_uri + " HTTP/1.1\r\n" +
                     "Host: " + homeassistant_host + "\r\n" +
                     "Authorization: Bearer " + access_token + "\r\n" +
                     "Content-Type: application/json\r\n" +
                     "Content-Length: " + message.length() + "\r\n" +
                     "\r\n" +
                     message;

    client.print(request);

    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            break;
        }
    }
    Serial.println("Connection to Home Assistant SUCCEDED!");
    String response = client.readString();
    Serial.println("Response:");
    Serial.println(response);
    client.stop();
    return true;
}

String createMessage(const String &topic, const String &payload, bool retain)
{
    String message = "{";
    message += "\"topic\":\"" + topic + "\"" + ",";
    message += "\"payload\":" + payload + ",";
    message += "\"retain\":" + String(retain);
    message += "}";
    return message;
}
