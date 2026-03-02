#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <Arduino.h>

class Publisher {
public:
    virtual ~Publisher() {}
    virtual void initialize(const String& chipId, const String& mqtt_server, int mqtt_port, const String& mqtt_user, const String& mqtt_pw) = 0;
    virtual bool publish(const String &topic, const String &payload, bool retain = false) = 0;
};

#endif // PUBLISHER_H