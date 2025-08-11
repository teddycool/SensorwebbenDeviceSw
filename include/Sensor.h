//Sensor interface, should be implemented for each new sensor-capability

#ifndef SENSOR_H
#define SENSOR_H

#include <ArduinoJson.h>

class Sensor {
public:
    virtual ~Sensor() {}
    virtual bool performMeasurement(){return false;};
    virtual void addToPayload(JsonDocument& payload){};
    virtual bool isSuccess() const;
};

#endif // PUBLISHER_H