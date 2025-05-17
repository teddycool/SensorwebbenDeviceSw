
#ifndef DISCOVERYMSG_H
#define DISCOVERYMSG_H

#include <Arduino.h> // For Arduino String type
//#include <WString.h>

#if defined(CHIPTYPE) && CHIPTYPE == ESP32
#include <WiFi.h>
#elif defined(CHIPTYPE) && CHIPTYPE == ESP8266
#include <ESP8266WiFi.h>
#else
#error "Unsupported CHIPTYPE. Please define CHIPTYPE as ESP8266 or ESP32."
#endif

class DiscoveryMsg {
public:
    
    // Method to create a serialized JSON string
   static  String createDiscoveryMsg(const String& deviceid, const String& dclass, 
        const String& vname, const String& unit);

    // Method to create a discovery message topic
    static String createDiscoveryMsgTopic(const String& deviceid, const String& vname);
};

#endif // DISCOVERYMSG_H