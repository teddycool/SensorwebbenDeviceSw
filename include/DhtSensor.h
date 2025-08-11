#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <ArduinoJson.h> 
#include <Sensor.h>

class DhtSensor: public Sensor {
public:
    // Constructor
    DhtSensor(int pin, int type);

    // Method to perform the DHT measurement
    bool performMeasurement();

    // Add temperature and humidity to the JSON payload
    void addToPayload(JsonDocument& payload);

    // Check if the last measurement was successful
    bool isSuccess() const;

private:
    int dhtpin;      // Pin number for the DHT sensor
    int dhttype;     // Type of the DHT sensor (DHT11, DHT22, etc.)
    float temperature; // Temperature value
    float humidity;    // Humidity value
    bool success;      // Measurement success flag
};

#endif // DHT_SENSOR_H