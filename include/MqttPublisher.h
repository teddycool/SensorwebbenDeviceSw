#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include <PubSubClient.h>
#include <WiFiClient.h>
#include "Publisher.h"

class MqttPublisher {
public:   
    MqttPublisher(WiFiClient &wifiClient);
    void initialize(const String& chipId, const String& mqtt_server, int mqtt_port, const String& mqtt_user, const String& mqtt_pw) ;
    bool publish(const String &topic, const String &payload, bool retain = false) ;

private:
    String chipId_;
    String mqtt_server_;
    int mqtt_port_;
    String mqtt_user_;
    String mqtt_pw_;
    PubSubClient mqttClient_;
    bool connect();
    void disconnect();
};

#endif // MQTT_PUBLISHER_H