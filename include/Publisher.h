#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <Arduino.h>

class Publisher {
public:
    virtual ~Publisher() {}
    virtual bool publish(const String &topic, const String &payload, bool retain = false) = 0;
};

#endif // PUBLISHER_H