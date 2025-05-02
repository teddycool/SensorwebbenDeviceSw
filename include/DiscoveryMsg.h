
#ifndef DISCOVERYMSG_H
#define DISCOVERYMSG_H

#include <Arduino.h> // For Arduino String type
//#include <WString.h>

class DiscoveryMsg {
public:
    
    // Method to create a serialized JSON string
   static  String createDiscoveryMsg(const String& deviceid, const String& dclass, 
        const String& vname, const String& unit);

    // Method to create a discovery message topic
    static String createDiscoveryMsgTopic(const String& deviceid, const String& vname);
};

#endif // DISCOVERYMSG_H