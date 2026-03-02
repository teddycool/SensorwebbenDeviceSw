#ifndef ADXL345_SENSOR_H
#define ADXL345_SENSOR_H

#include "Sensor.h"
#include <ArduinoJson.h>
#include <Wire.h>

struct AdxlVector {
    float x;
    float y;
    float z;
};

class Adxl345Sensor : public Sensor {
public:
    Adxl345Sensor(int sda_pin, int scl_pin, uint8_t address = 0x53);
    ~Adxl345Sensor() override;
    
    bool performMeasurement() override;
    void addToPayload(JsonDocument& payload) override;
    bool isSuccess() const override;
    
    // Get the measured acceleration vector
    AdxlVector getAcceleration() const { return accel_vector_; }
    
    // Calculate activity level (0-5 scale)
    float getActivityLevel() const;
    
private:
    // I2C configuration
    int sda_pin_;
    int scl_pin_;
    uint8_t address_;
    
    // Sensor state
    bool initialized_;
    bool last_measurement_success_;
    AdxlVector accel_vector_;
    AdxlVector prev_accel_vector_;  // Previous sample for change detection
    bool prev_sample_valid_;
    
    // Activity level constants
    static constexpr float MAX_ACTIVITY_THRESHOLD_MG = 200.0f;  // Threshold for activity level 5 (change rate)
    
    // ADXL345 register addresses
    static const uint8_t REG_DEVID         = 0x00;
    static const uint8_t REG_DATA_FORMAT   = 0x31;
    static const uint8_t REG_BW_RATE       = 0x2C;
    static const uint8_t REG_POWER_CTL     = 0x2D;
    static const uint8_t REG_INT_ENABLE    = 0x2E;
    static const uint8_t REG_INT_MAP       = 0x2F;
    static const uint8_t REG_DATAX0        = 0x32;
    
    // Private methods
    bool initializeSensor();
    void i2cWrite8(uint8_t reg, uint8_t val);
    uint8_t i2cRead8(uint8_t reg);
    void i2cReadMulti(uint8_t reg, uint8_t *buf, uint8_t len);
    float lsb_to_mg(int16_t v);
    AdxlVector readAccelerationSample();
    AdxlVector calculateMeanAcceleration();
};

#endif // ADXL345_SENSOR_H