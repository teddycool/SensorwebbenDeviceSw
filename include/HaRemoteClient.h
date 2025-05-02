#ifndef HAREMOTECLIENT_H
#define HAREMOTECLIENT_H

#include <Arduino.h>



    bool publish(const String &topic, const String &payload, bool retain);


      
    String createMessage(const String &topic, const String &payload, bool retain);
#endif