#include "Hx711Sensor.h"
#include <HX711.h>

// Constructor
Hx711Sensor::Hx711Sensor(int dout_pin, int dsck_pin) : dout_pin(dout_pin), dsck_pin(dsck_pin), rweight(0.0), success(false)

{
    pinMode(dout_pin, INPUT);
    pinMode(dsck_pin, INPUT);
    success = false;
}

// Perform the DHT measurement
bool Hx711Sensor::performMeasurement()
{
    HX711 scale;
    Serial.println("Init scale");
    scale.begin(dout_pin, dsck_pin);
    Serial.println("Stabilizing sensors....");
    delay(1000);
    Serial.println("Start measuring");

    if (scale.wait_ready_timeout(20000))
    {
        Serial.println("Scale ready...");
        Serial.println("Reading weight...");
        rweight = scale.read_average(20);
        success = true;
        Serial.println("Raw weight: " + String(rweight));
        return success;
    }
    else
    {
        Serial.println("Scale not ready or timed out...");
        return false;
    }
}

bool Hx711Sensor::isSuccess() const
{
    return success;
}   

void Hx711Sensor::addToPayload(JsonDocument& payload)
{
    if (success)
    {
        payload["rweight"] = rweight;
    }
    else
    {
        payload["hx711"] = "Measurement failed";
    }
}

