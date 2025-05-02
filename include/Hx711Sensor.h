
#ifndef HX711_SENSOR_H  // HX711_SENSOR_H
#define HX711_SENSOR_H

#include <ArduinoJson.h> 
#include <Sensor.h>

class Hx711Sensor: public Sensor {
public:
    // Constructor
    Hx711Sensor(int dout_pin, int dsck_pin);

    // Method to perform the DHT measurement
    bool performMeasurement();

    // Add temperature and humidity to the JSON payload
    void addToPayload(JsonDocument& payload);

    // Check if the last measurement was successful
    bool isSuccess() const;

private:
    int dout_pin;      // LOADCELL_DOUT_PIN
    int dsck_pin;     // LOADCELL_SCK_PIN
    int rweight;   // Raw value from ad converter
    bool success;      // Measurement success flag
};

#endif //HX711_SENSOR_H

