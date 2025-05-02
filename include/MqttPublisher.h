#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include <PubSubClient.h>
#include <WiFiClient.h>
#include "Publisher.h" // Ensure the base class is included

class MqttPublisher: public Publisher  
{
private:
    // Static variables for MQTT configuration
    static String chipId;
    static String mqtt_server;
    static int mqtt_port;
    static char* mqtt_user;
    static char* mqtt_pw;
    static PubSubClient mqttClient;
    static WiFiClient wifiClient;

public:
    // Static methods for MQTT operations
    void initialize(String chipId, String mqtt_server, int mqtt_port, char* mqtt_user, char* mqtt_pw);
    bool publish(const String &topic, const String &payload, bool retain);
     bool connect();
     void disconnect();
};

#endif // MQTT_PUBLISHER_H