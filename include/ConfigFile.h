#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Arduino.h>

class ConfigFile {
public:
    static bool writeToFile(const char* filename, const JsonDocument& json);
    static bool readFromFile(const char* filename, JsonDocument& json);
};

#endif // CONFIGFILE_H