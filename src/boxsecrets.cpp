// boxsecrets.cpp is and must be ignored by git.
// This file contains sensitive information such as WiFi credentials and API keys.
// It is not recommended to share this file publicly or include it in version control.
#include "boxsecrets.h"

//extern const char* backendUlr = "http://"; // Not yet used in config or setup
//extern const char* apikey = "32-m7nYGFH-mNr831hIGnlFcwOADdO302k0NBwxE-xP1kIJS9Qh3yHAqZDdz8igE9o2nFgj6ATbyoFrVBfC6qA";

// WiFi credentials
 const char* cssid = "RUT240_B40A";
 const char* cpassword = "Ti9g3X0N";

 //const char* cssid = "dlink-4ECC";
 //const char* cpassword = "szcin99228";



// Settings using web-hooks to publish MQTT messages securely via Nabu Casa cloud
 const char* homeassistant_host = "https://cleo7lc8f9if7nhqthcr6dbgs3ftutqp.ui.nabu.casa"; // Replace with your nabu casa host
 const char* homeassistant_url= "https://cleo7lc8f9if7nhqthcr6dbgs3ftutqp.ui.nabu.casa/api/webhook/-ZJbTo2yKKJRR0lhUGdQn7KaS";
 const String access_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiIzNGYwZWQzYTk0MjM0N2Q4OTFjNzc4YzRlNWYwOGZhMiIsImlhdCI6MTc0MDMxNzg2MiwiZXhwIjoyMDU1Njc3ODYyfQ.cU2gtP4mc4LhAJ5KwHv9wIwLDJ2dnE6rLSt7UH6GSpI";

 String local_mqtt_server = "192.168.68.100";
 int local_mqtt_port = 1883;
 char* local_mqtt_user = "par";
 char* local_mqtt_pw = "Yeti1501";

 