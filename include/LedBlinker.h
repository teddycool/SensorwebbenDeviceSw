#ifndef LEDBLINKER_H
#define LEDBLINKER_H

#include <Arduino.h>

class LedBlinker {
private:
    int LEDPIN;

public:
    // Constructor to initialize the LED pin
    LedBlinker(int LEDPIN);

    // Static method to blink the LED
    static void ledBlink(int ledPin, int ontime, int offtime, int count);
};

#endif // LEDBLINKER_H