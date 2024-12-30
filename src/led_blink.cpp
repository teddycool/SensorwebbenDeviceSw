#include <Arduino.h>
#include "led_blink.h"

void ledBlink(int ontime, int offtime, int count)
{
    for (int i = 0; i < count; i++)
    {
        digitalWrite(15, HIGH);
        delay(ontime);
        digitalWrite(15, LOW);
        delay(offtime);
    }
}
