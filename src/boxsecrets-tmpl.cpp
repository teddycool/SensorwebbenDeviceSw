// Add your secrets and rename to boxsectrets.cpp    
// bosxsecrets.cpp is and must be IGNORED by git

#include "boxsecrets.h"


// WiFi credentials
 const char* ssid = "";
 const char* password = "";

// Settings using web-hooks to publish MQTT messages securely via Nabu Casa cloud
 const char* homeassistant_host = ""; // Replace with your MQTT server
 const char* homeassistant_url; // Replace with your Nabu Casa URL
 const char* access_token = "";
 String local_mqtt_server = "";
 int local_mqtt_port = 1883;
 char* local_mqtt_user = "user";
 char* local_mqtt_pw = "password";