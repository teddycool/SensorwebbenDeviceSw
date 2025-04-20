#ifndef HAREMOTECLIENT_H
#define HAREMOTECLIENT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <Publisher.h>

class HaRemoteClient : public Publisher 
{
public:
    HaRemoteClient();
    ~HaRemoteClient();

    bool publish(const String &topic, const String &payload, bool retain);

private:
      
    String createMessage(const String &topic, const String &payload, bool retain);
};

#endif