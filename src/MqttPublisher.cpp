/*********************************************************
 * Publishing MQTT messages to a broker access directly
 * using the PubSubClient library but don't care about the
 * message or topic structure
 */

#include "MqttPublisher.h"


MqttPublisher::MqttPublisher() {
    mqttClient_ = PubSubClient(wifiClient_);
}

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
    mqttClient_.setServer(mqtt_server.c_str(), mqtt_port);    
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
{
        if (!connect())
    {
        Serial.println("MQTT connection failed!");
        return false;
    }
    bool publishSuccess = mqttClient_.publish(topic.c_str(), payload.c_str(), retain);
    mqttClient_.loop();
    delay(500); // Give time for message to be sent
    disconnect();
    return publishSuccess;
}
