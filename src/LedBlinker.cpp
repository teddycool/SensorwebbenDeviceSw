

#include <Arduino.h>
#include "LedBlinker.h"

#if defined(CHIPTYPE) && CHIPTYPE == ESP32
#include "Esp32Config.h"
#elif defined(CHIPTYPE) && CHIPTYPE == ESP8266
#include "Esp8266Config.h"
#else
#error "Unsupported CHIPTYPE. Please define CHIPTYPE as ESP8266 or ESP32."
#endif

LedBlinker::LedBlinker(int LedPin)
{
    // Constructor implementation (can be empty or initialize members if needed)
    pinMode(LedPin, OUTPUT); // Set the LED pin as output
    this->ledpin = LedPin;   // Store the LED pin number
}

void LedBlinker::ledBlink(int ontime, int offtime, int count)
{
    for (int i = 0; i < count; i++)
    {
        digitalWrite(ledpin, HIGH);
        delay(ontime);
        digitalWrite(ledpin, LOW);
        delay(offtime);
    }
}
