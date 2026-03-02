#include "WifiManagerParams.h"

WifiManagerParams createWifiManagerParams(
    char* mqtt_server,
    char* mqtt_port,
    char* mqtt_user,
    char* mqtt_pw,
    char* sleeptimeStr,
    char* mqtt_ptopic
) {
    WifiManagerParams params;
    params.mqtt_server = new WiFiManagerParameter("server", "mqtt server", mqtt_server, 40);
    params.mqtt_port   = new WiFiManagerParameter("port", "mqtt server port", mqtt_port, 6);
    params.mqtt_user   = new WiFiManagerParameter("user", "mqtt server user", mqtt_user, 20);
    params.mqtt_pw     = new WiFiManagerParameter("pw", "mqtt server pw", mqtt_pw, 20);
    params.sleeptime   = new WiFiManagerParameter("sleeptimer", "sleeptime in min", sleeptimeStr, 5);
    params.mqtt_ptopic = new WiFiManagerParameter("ptopic", "mqtt publish-topic", mqtt_ptopic, 50);
    return params;
}

void addParamsToManager(WiFiManager& wifiManager, WifiManagerParams& params) {
    wifiManager.addParameter(params.mqtt_server);
    wifiManager.addParameter(params.mqtt_port);
    wifiManager.addParameter(params.mqtt_user);
    wifiManager.addParameter(params.mqtt_pw);
    wifiManager.addParameter(params.sleeptime);
    wifiManager.addParameter(params.mqtt_ptopic);
    // Add more as needed
}#include "WifiManagerParams.h"
// ...existing code...

void setup() {
    // ...existing code...

    char sleeptimerStr[8];
    snprintf(sleeptimerStr, sizeof(sleeptimerStr), "%lld", sleeptimer);

    WiFiManager wifiManager;
    WifiManagerParams params = createWifiManagerParams(
        mqtt_server, mqtt_port, mqtt_user, mqtt_pw, sleeptimerStr, mqtt_ptopic
    );
    addParamsToManager(wifiManager, params);

    wifiManager.setSaveConfigCallback(saveConfigCallback);

    // ...rest of your setup code...

    // After config portal, retrieve values:
    strcpy(mqtt_server, params.mqtt_server->getValue());
    strcpy(mqtt_port,   params.mqtt_port->getValue());
    strcpy(mqtt_user,   params.mqtt_user->getValue());
    strcpy(mqtt_pw,     params.mqtt_pw->getValue());
    strcpy(mqtt_ptopic, params.mqtt_ptopic->getValue());
    sleeptimer = atoll(params.sleeptime->getValue());

    // Don't forget to delete allocated parameters to avoid memory leaks
    delete params.mqtt_server;
    delete params.mqtt_port;
    delete params.mqtt_user;
    delete params.mqtt_pw;
    delete params.sleeptime;
    delete params.mqtt_ptopic;
}