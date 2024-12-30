#ifndef configmsg_h
#include <WString.h>
#include "ArduinoJson.h"

//Return a serialized json string
String createConfigMsg(String deviceid, String dclass, String vname, String unit );

String createConfigMsgTopic(String deviceid, String vname);

#endif