#include "configmsg.h"



String createConfigMsg(String deviceid, String dclass, String vname, String unit)
{
    DynamicJsonDocument mqttconfigmsg(1024);
    mqttconfigmsg["name"] = "sw_" + deviceid + "_" + vname;
    mqttconfigmsg["state_topic"] = "home/sensor/sw_" + deviceid;
    mqttconfigmsg["unit_of_measurement"] = unit;
    if (dclass != "none")
    {
        mqttconfigmsg["device_class"] = dclass;
    }

    mqttconfigmsg["value_template"] = "{{value_json." + vname + "}}";
    mqttconfigmsg["unique_id"] = "sw_" + deviceid + "_" + vname;

    JsonArray identifiers = mqttconfigmsg["device"]["identifiers"].to<JsonArray>();
    identifiers.add("sw_" + deviceid);

    mqttconfigmsg["device"]["manufacturer"] = "www.sensorwebben.se";
    mqttconfigmsg["device"]["model"] = "Sensor Model GDC";
    mqttconfigmsg["device"]["sw_version"] = SWVERSION;
    mqttconfigmsg["device"]["name"] = deviceid;

    String outputmsg;
    serializeJson(mqttconfigmsg, outputmsg);
    serializeJsonPretty(mqttconfigmsg, Serial);

    return outputmsg;
}

String createConfigMsgTopic(String deviceid, String vname)
{
    return "homeassistant/sensor/sw_" + deviceid + "_" + vname + "/config";
}
