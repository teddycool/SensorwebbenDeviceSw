
#include "deviceconfig.h"
#include <Arduino.h>
#include "ledblink.h"

void ledBlink(int ontime, int offtime, int count)
{
    for (int i = 0; i < count; i++)
    {
        digitalWrite(LEDPIN, HIGH);
        delay(ontime);
        digitalWrite(LEDPIN, LOW);
        delay(offtime);
    }
}