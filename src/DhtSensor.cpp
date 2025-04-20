#include "DhtSensor.h"
#include <deviceconfig.h>
#include <Wire.h>
#include "DHT.h"
#include "ledblink.h"
// Constructor
DhtSensor::DhtSensor(int pin, int type) : dhtpin(pin), dhttype(type), temperature(0.0), humidity(0.0), success(false) {}

// Perform the DHT measurement
bool DhtSensor::performMeasurement()
{
    DHT dht(DHTPIN, DHTTYPE);
    dht.begin();
    delay(1000);

    Serial.println("Reading from " + String(DHTTYPE) + " sensor");
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature))
    {
        Serial.println("Failed to read from DHT sensor!");
        ledBlink(100, 100, 6);
        Serial.println("Blink measurement-error (6)");
        success = false;
    }
    else
    {
        success = true;
    }
    Serial.println("DHT hum: " + String(humidity) + "%");
    Serial.println("DHT temp: " + String(temperature) + "C");
    return success;
}

// Add temperature and humidity to the JSON payload
void DhtSensor::addToPayload(JsonDocument &payload)
{
    if (success)
    {
        payload["temperature"] = temperature;
        payload["humidity"] = humidity;
    }
    else
    {
        payload["error"] = "DHT measurement failed";
    }
}

// Check if the last measurement was successful
bool DhtSensor::isSuccess() const
{
    return success;
}