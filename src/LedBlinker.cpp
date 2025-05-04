

#include <Arduino.h>
#include "LedBlinker.h"


#if defined(CHIPTYPE) && CHIPTYPE == ESP32
#include "Esp32Config.h"
#elif defined(CHIPTYPE) && CHIPTYPE == ESP8266
#include "Esp8266Config.h"
#else
#error "Unsupported CHIPTYPE. Please define CHIPTYPE as ESP8266 or ESP32."
#endif



    void LedBlinker::ledBlink(int ledPin, int ontime, int offtime, int count) {
        for (int i = 0; i < count; i++) {
            digitalWrite(ledPin, HIGH);
            delay(ontime);
            digitalWrite(ledPin, LOW);
            delay(offtime);
        }
    }
