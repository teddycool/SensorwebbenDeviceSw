/*********************************************************
 * Publishing MQTT messages to a broker access directly
 * using the PubSubClient library but don't care about the
 * message or topic structure
 */

#include "MqttPublisher.h"
#include <WiFiClient.h>
#include "ledblink.h"

// Define static variables
String MqttPublisher::chipId;
String MqttPublisher::mqtt_server;
int MqttPublisher::mqtt_port;
char *MqttPublisher::mqtt_user;
char *MqttPublisher::mqtt_pw;
PubSubClient MqttPublisher::mqttClient;
WiFiClient MqttPublisher::wifiClient;

// Initialize the static class
void MqttPublisher::initialize(String chipId, String mqtt_server, int mqtt_port, char *mqtt_user, char *mqtt_pw)
{
    MqttPublisher::chipId = chipId;
    MqttPublisher::mqtt_server = mqtt_server;
    MqttPublisher::mqtt_port = mqtt_port;
    MqttPublisher::mqtt_user = mqtt_user;
    MqttPublisher::mqtt_pw = mqtt_pw;
   
}

// Publish a message
bool MqttPublisher::publish(const String &topic, const String &payload, bool retain)
{
    if (!MqttPublisher::connect())
    {
        Serial.println("MQTT connection failed!");
        return false;
    }

    if (mqttClient.publish(topic.c_str(), payload.c_str(), retain))
    {
        Serial.println("Message published successfully");
    }
    else
    {
        Serial.println("Failed to publish message");
    }

    mqttClient.loop();
    MqttPublisher::disconnect();
    return true;
}

// Connect to the MQTT broker
bool MqttPublisher::connect()
{
    mqttClient.setClient(wifiClient);
    mqttClient.setServer(mqtt_server.c_str(), mqtt_port);
    mqttClient.connect(chipId.c_str(), mqtt_user, mqtt_pw);
    if (mqttClient.connected())
    {
        Serial.println("MQTT connected!");
        return true;
    }    
    else
    {
        Serial.println("MQTT connection failed!");
        return false;
    }
}

// Disconnect from the MQTT broker
void MqttPublisher::disconnect()
{
    if (mqttClient.connected())
    {
        mqttClient.disconnect();
        Serial.println("MQTT disconnected");
    }
}
