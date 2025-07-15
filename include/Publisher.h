#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <Arduino.h>

class Publisher {
public:
    virtual ~Publisher() {}
    virtual bool publish(const String &topic, const String &payload, bool retain = false) = 0;
    virtual void initialize(String chipId, String mqtt_server, int mqtt_port, char* mqtt_user, char* mqtt_pw);
};

#endif // PUBLISHER_H