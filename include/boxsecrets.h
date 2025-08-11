/// Header for secrets and API keys used for setup. 
#ifndef BOXSECRETS_H
#define BOXSECRETS_H

#include <Arduino.h>


// WiFi credentials used for direct connection without portal (in setup)
extern const char* cssid ;
extern const char* cpassword ;

// Settings using web-hooks to publish MQTT messages securely via Nabu Casa cloud
extern const char* homeassistant_host; // Replace with your MQTT server
extern const char* homeassistant_uri; // Replace with your Nabu Casa URL
extern const char* homeassistant_full_url;


extern const String access_token;
extern String local_mqtt_server;
extern int local_mqtt_port ;
extern char* local_mqtt_user;
extern char* local_mqtt_pw;
#endif // BOXSECRETS_H

