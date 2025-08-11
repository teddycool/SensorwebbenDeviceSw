#ifndef LEDBLINKER_H
#define LEDBLINKER_H

#include <Arduino.h>

class LedBlinker
{
private:
    int ledpin;

public:
    // Constructor to initialize the LED pin
    LedBlinker(int Ledpin);

    void ledBlink(int ontime, int offtime, int count);
};

#endif // LEDBLINKER_H