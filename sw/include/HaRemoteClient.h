#ifndef HAREMOTECLIENT_H
#define HAREMOTECLIENT_H

#include <Arduino.h>
#include "Publisher.h"

class HaRemoteClient : public Publisher {
public:
    void initialize(const String& chipId, const String& mqtt_server, int mqtt_port, const String& mqtt_user, const String& mqtt_pw) override;
    bool publish(const String &topic, const String &payload, bool retain = false) override;

private:
    String chipId_;
    String mqtt_server_;
    int mqtt_port_;
    String mqtt_user_;
    String mqtt_pw_;
    String createMessage(const String &topic, const String &payload, bool retain);
};

#endif // HAREMOTECLIENT_H