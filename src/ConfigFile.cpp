#include "ConfigFile.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <FS.h>

bool ConfigFile::writeToFile(const char *filename, const JsonDocument &json)
{
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount file system");
        return false;
    }

    File configFile = LittleFS.open(filename, "w");
    if (!configFile)
    {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    if (serializeJson(json, configFile) == 0)
    {
        Serial.println("Failed to write JSON to file");
        configFile.close();
        return false;
    }

    configFile.close();
    Serial.println("Configuration saved successfully");
    return true;
}

bool ConfigFile::readFromFile(const char *filename, JsonDocument &json)
{
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount file system");
        return false;
    }

    File configFile = LittleFS.open(filename, "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file for reading");
        return false;
    }

    DeserializationError error = deserializeJson(json, configFile);
    if (error)
    {
        Serial.println("Failed to parse JSON from file");
        configFile.close();
        return false;
    }

    configFile.close();
    Serial.println("Configuration loaded successfully");
    return true;
}