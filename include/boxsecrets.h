/// Header for secrets and API keys used for setup. 
#ifndef BOXSECRETS_H
#define BOXSECRETS_H

#include <Arduino.h>


//extern const char* backendUlr = "http://"; // Not yet used in config or setup
//extern const char* apikey = "32-m7nYGFH-mNr831hIGnlFcwOADdO302k0NBwxE-xP1kIJS9Qh3yHAqZDdz8igE9o2nFgj6ATbyoFrVBfC6qA";

// WiFi credentials used for direct connection without portal
extern const char* cssid ;
extern const char* cpassword ;

// Settings using web-hooks to publish MQTT messages securely via Nabu Casa cloud
extern const char* homeassistant_host; // Replace with your MQTT server
extern const char* homeassistant_url; // Replace with your Nabu Casa URL
extern const String access_token;
extern String local_mqtt_server;
extern int local_mqtt_port ;
extern char* local_mqtt_user;
extern char* local_mqtt_pw;
#endif // BOXSECRETS_H

