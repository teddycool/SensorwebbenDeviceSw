/*********************************************************
 * Creating the MQTT message needed for the Home Assistant
 * discovery. One message for each sensor-entity.
 *
 */

#include "DiscoveryMsg.h"
#include "ArduinoJson.h"

//#include "Esp32Config.h"
//#include "Esp8266Config.h"


#if defined(CHIPTYPE) && CHIPTYPE == ESP32
#include "Esp32Config.h"
#elif defined(CHIPTYPE) && CHIPTYPE == ESP8266
#include "Esp8266Config.h"
#else
#error "Unsupported CHIPTYPE. Please define CHIPTYPE as ESP8266 or ESP32."
#endif




// Create a serialized JSON string
String DiscoveryMsg::createDiscoveryMsg(const String &deviceid, const String &dclass, const String &vname, const String &unit, int prec)
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
    
    // Set the number of decimals (precision) for Home Assistant
    if (prec > 0)
        mqttconfigmsg["suggested_display_precision"] = prec;

    mqttconfigmsg["value_template"] = "{{ value_json." + vname + " }}";
    mqttconfigmsg["unique_id"] = "sw_" + deviceid + "_" + vname;

    JsonArray identifiers = mqttconfigmsg["device"]["identifiers"].to<JsonArray>();
    identifiers.add("sw_" + deviceid);
    identifiers.end();

    mqttconfigmsg["device"]["manufacturer"] = "www.sensorwebben.se";
    mqttconfigmsg["device"]["model"] = SENSORMODEL;
    mqttconfigmsg["device"]["sw_version"] = firmware;
    mqttconfigmsg["device"]["hw_version"] = HWVERSION;
    mqttconfigmsg["device"]["name"] = deviceid + " (sensorwebben)";

    String outputmsg;
    serializeJson(mqttconfigmsg, outputmsg);

    return outputmsg;
}

// Create a discovery message topic
String DiscoveryMsg::createDiscoveryMsgTopic(const String &deviceid, const String &vname)
{
    return "homeassistant/sensor/" + deviceid + "/sw_" + deviceid + "_" + vname + "/config";
}