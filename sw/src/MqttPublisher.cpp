/*********************************************************
 * Publishing MQTT messages to a broker access directly
 * using the PubSubClient library but don't care about the
 * message or topic structure
 */

#include "MqttPublisher.h"
#include <WiFiClient.h>



MqttPublisher::MqttPublisher(WiFiClient &wifiClient)
    : mqttClient_(wifiClient) {}

void MqttPublisher::initialize(const String &chipId, const String &mqtt_server, int mqtt_port, const String &mqtt_user, const String &mqtt_pw)
{
    Serial.println("Initializing MQTT Publisher with:");
    Serial.println("Server: " + String(mqtt_server));
    Serial.println("Port: " + String(mqtt_port));
    Serial.println("User: " + String(mqtt_user));
    Serial.println("Password: " + String(mqtt_pw));
    Serial.println("Client ID: " + chipId);
    chipId_ = chipId;
    mqtt_server_ = mqtt_server;
    mqtt_port_ = mqtt_port;
    mqtt_user_ = mqtt_user;
    mqtt_pw_ = mqtt_pw;
    mqttClient_.setServer(mqtt_server_.c_str(), mqtt_port_);
    mqttClient_.setBufferSize(2048);
}

bool MqttPublisher::connect()
{
    if (!mqttClient_.connected())
    {
        bool result = mqttClient_.connect(chipId_.c_str(), mqtt_user_.c_str(), mqtt_pw_.c_str());
        Serial.println("Connect result: " + String(result));
        Serial.println("MQTT state: " + String(mqttClient_.state()));
        return result;
    }
    return true;
}

void MqttPublisher::disconnect()
{
    mqttClient_.disconnect();
}

bool MqttPublisher::publish(const String &topic, const String &payload, bool retain)
{   connect();
    bool status = mqttClient_.publish(topic.c_str(), payload.c_str(), retain);
    Serial.println("Publish status: " + String(status));
    delay(100);
    disconnect();
    Serial.println("MQTT disconnected after publish.");
    return status;
}
