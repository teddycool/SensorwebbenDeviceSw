/*********************************************************
 * Creating the MQTT message needed for the Home Assistant
 * discovery. One message for each sensor-entity.
 *
 */

#include "DiscoveryMsg.h"
#include "ArduinoJson.h"
#include "deviceconfig.h"


// Create a serialized JSON string
String DiscoveryMsg::createDiscoveryMsg(const String &deviceid, const String &dclass, const String &vname, const String &unit)
{
    String firmware = String(SWVERSION) + " " + String(COMTYPE);

    DynamicJsonDocument mqttconfigmsg(2048);
    mqttconfigmsg["name"] = "sw_" + deviceid + "_" + vname;
    mqttconfigmsg["state_topic"] = "home/sensor/sw_" + deviceid;
    if (unit != "none")
    {
        mqttconfigmsg["unit_of_measurement"] = unit;
    }
    
    if (dclass != "none")
    {
        mqttconfigmsg["device_class"] = dclass;
    }

    mqttconfigmsg["value_template"] = "{{ value_json." + vname + " }}";
    mqttconfigmsg["unique_id"] = "sw_" + deviceid + "_" + vname;

    JsonArray identifiers = mqttconfigmsg["device"]["identifiers"].to<JsonArray>();
    identifiers.add("sw_" + deviceid);
    identifiers.end();

    mqttconfigmsg["device"]["manufacturer"] = "www.sensorwebben.se";
    mqttconfigmsg["device"]["model"] = SENSORMODEL;
    mqttconfigmsg["device"]["sw_version"] = firmware;
    mqttconfigmsg["device"]["hw_version"] = HWversion;
    mqttconfigmsg["device"]["name"] = deviceid + " (sensorwebben)";

    String outputmsg;
    serializeJson(mqttconfigmsg, outputmsg);

    return outputmsg;
}

// Create a discovery message topic
String DiscoveryMsg::createDiscoveryMsgTopic(const String &deviceid, const String &vname)
{
    return "homeassistant/sensor/sw_" + deviceid + "_" + vname + "/config";
}