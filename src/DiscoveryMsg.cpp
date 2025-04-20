/*********************************************************
* Creating the MQTT message needed for the Home Assistant
* discovery. One message for each sensor-entity.
* 
*/


#include "DiscoveryMsg.h"


// Create a serialized JSON string
 String DiscoveryMsg::createDiscoveryMsg(const String& deviceid, const String& dclass, const String& vname, const String& unit) {
    String json = "{";
    json += "\"deviceid\":\"" + deviceid + "\",";
    json += "\"dclass\":\"" + dclass + "\",";
    json += "\"vname\":\"" + vname + "\",";
    json += "\"unit\":\"" + unit + "\"";
    json += "}";
    return json;
}

// Create a discovery message topic
String DiscoveryMsg::createDiscoveryMsgTopic(const String& deviceid, const String& vname) {
    return deviceid + "/" + vname + "/config";
}