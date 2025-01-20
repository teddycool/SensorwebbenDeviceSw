#ifndef configmsg_h
#include <WString.h>
#include "ArduinoJson.h"

#define SWVERSION "V2.2.2"

//Return a serialized json string
String createConfigMsg(String deviceid, String dclass, String vname, String unit );

String createConfigMsgTopic(String deviceid, String vname);

#endif