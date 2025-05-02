#include "Hx711Sensor.h"
#include <deviceconfig.h>
#include <HX711.h>


// Constructor
Hx711Sensor::Hx711Sensor(int dout_pin, int dsck_pin) : dout_pin(dout_pin), dsck_pin(dsck_pin), rweight(0.0), success(false) 

{
    pinMode(dout_pin, INPUT);
    pinMode(dsck_pin, INPUT);
}



// Perform the DHT measurement
bool Hx711Sensor::performMeasurement()
{
    HX711 scale;
    scale.begin(dout_pin, dsck_pin);
    Serial.println("Stabilizing sensors....");

}