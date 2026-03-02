#ifndef WIFI_MANAGER_PARAMS_H
#define WIFI_MANAGER_PARAMS_H

#include <WiFiManager.h>

// Structure to hold pointers to WiFiManagerParameter objects
struct WifiManagerParams {
    WiFiManagerParameter* mqtt_server;
    WiFiManagerParameter* mqtt_port;
    WiFiManagerParameter* mqtt_user;
    WiFiManagerParameter* mqtt_pw;
    WiFiManagerParameter* sleeptime;
    WiFiManagerParameter* mqtt_ptopic;
    // Add more parameters as needed
};

// Function to create and return WifiManagerParams
WifiManagerParams createWifiManagerParams(
    char* mqtt_server,
    char* mqtt_port,
    char* mqtt_user,
    char* mqtt_pw,
    char* sleeptimeStr,
    char* mqtt_ptopic
);

void addParamsToManager(WiFiManager& wifiManager, WifiManagerParams& params);

#endif // WIFI_MANAGER_PARAMS_H