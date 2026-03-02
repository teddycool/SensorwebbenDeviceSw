#include "Adxl345Sensor.h"

Adxl345Sensor::Adxl345Sensor(int sda_pin, int scl_pin, uint8_t address)
    : sda_pin_(sda_pin), scl_pin_(scl_pin), address_(address),
      initialized_(false), last_measurement_success_(false),
      accel_vector_{0.0, 0.0, 0.0}, prev_accel_vector_{0.0, 0.0, 0.0},
      prev_sample_valid_(false) {
}

Adxl345Sensor::~Adxl345Sensor() {
    // Clean up if needed
}

bool Adxl345Sensor::performMeasurement() {
    if (!initialized_) {
        initialized_ = initializeSensor();
        if (!initialized_) {
            last_measurement_success_ = false;
            return false;
        }
    }
    
    // Store previous sample for change detection
    if (last_measurement_success_) {
        prev_accel_vector_ = accel_vector_;
        prev_sample_valid_ = true;
    }
    
    // Calculate mean acceleration over 1/10 second (100ms)
    accel_vector_ = calculateMeanAcceleration();
    last_measurement_success_ = true;
    
    return true;
}

void Adxl345Sensor::addToPayload(JsonDocument& payload) {
    if (last_measurement_success_) {
        // Add acceleration vector as individual components
        payload["accel_x"] = accel_vector_.x;
        payload["accel_y"] = accel_vector_.y;
        payload["accel_z"] = accel_vector_.z;
        
        // Calculate and add vector magnitude representing total force
        float magnitude = sqrt(accel_vector_.x * accel_vector_.x + 
                              accel_vector_.y * accel_vector_.y + 
                              accel_vector_.z * accel_vector_.z);
        payload["accel_magnitude"] = magnitude;
        
        // Calculate 2D force in X-Y plane (horizontal force)
        float xy_force = sqrt(accel_vector_.x * accel_vector_.x + 
                             accel_vector_.y * accel_vector_.y);
        payload["accel_xy_force"] = xy_force;
        
        // Add activity level (0-5 scale)
        payload["activity_level"] = getActivityLevel();
    } else {
        payload["accel_x"] = nullptr;
        payload["accel_y"] = nullptr;
        payload["accel_z"] = nullptr;
        payload["accel_magnitude"] = nullptr;
        payload["accel_xy_force"] = nullptr;
        payload["activity_level"] = nullptr;
        payload["accel_error"] = "measurement_failed";
    }
}

bool Adxl345Sensor::isSuccess() const {
    return last_measurement_success_;
}

bool Adxl345Sensor::initializeSensor() {
    // Initialize I2C
    Wire.begin(sda_pin_, scl_pin_);
    Wire.setClock(100000);  // 100kHz
    
    // Check device ID
    uint8_t devid = i2cRead8(REG_DEVID);
    if (devid != 0xE5) {
        Serial.printf("ADXL345 not found! Expected 0xE5, got 0x%02X\n", devid);
        return false;
    }
    
    Serial.printf("ADXL345 found with DEVID = 0x%02X\n", devid);
    
    // Configure data format: FULL_RES=1, +/-4g range
    i2cWrite8(REG_DATA_FORMAT, 0b00001001);
    
    // Configure bandwidth rate: 50 Hz for responsive readings
    i2cWrite8(REG_BW_RATE, 0x0C);
    
    // Disable interrupts for polled mode
    i2cWrite8(REG_INT_ENABLE, 0x00);
    i2cWrite8(REG_INT_MAP, 0xFF);
    
    // Enable measurement mode
    i2cWrite8(REG_POWER_CTL, 0b00001000);
    
    // Give sensor time to stabilize
    delay(50);
    
    Serial.println("ADXL345 initialized successfully");
    return true;
}

void Adxl345Sensor::i2cWrite8(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(address_);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

uint8_t Adxl345Sensor::i2cRead8(uint8_t reg) {
    Wire.beginTransmission(address_);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(address_, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0;
}

void Adxl345Sensor::i2cReadMulti(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(address_);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(address_, len);
    for (uint8_t i = 0; i < len && Wire.available(); ++i) {
        buf[i] = Wire.read();
    }
}

float Adxl345Sensor::lsb_to_mg(int16_t v) {
    // Conversion factor: ~3.9 mg/LSB in full-res mode
    return (float)v * 3.9f;
}

AdxlVector Adxl345Sensor::readAccelerationSample() {
    uint8_t buf[6];
    i2cReadMulti(REG_DATAX0, buf, 6);
    
    int16_t x = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t y = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t z = (int16_t)((buf[5] << 8) | buf[4]);
    
    AdxlVector vector;
    vector.x = lsb_to_mg(x);
    vector.y = lsb_to_mg(y);
    vector.z = lsb_to_mg(z);
    
    return vector;
}

AdxlVector Adxl345Sensor::calculateMeanAcceleration() {
    const int num_samples = 10;  // Take 10 samples over 100ms
    const int sample_delay = 10; // 10ms between samples
    
    AdxlVector sum = {0.0, 0.0, 0.0};
    int valid_samples = 0;
    
    for (int i = 0; i < num_samples; i++) {
        AdxlVector sample = readAccelerationSample();
        
        // Simple validation - check if values are reasonable
        if (abs(sample.x) < 5000 && abs(sample.y) < 5000 && abs(sample.z) < 5000) {
            sum.x += sample.x;
            sum.y += sample.y;
            sum.z += sample.z;
            valid_samples++;
        }
        
        if (i < num_samples - 1) {
            delay(sample_delay);
        }
    }
    
    AdxlVector mean = {0.0, 0.0, 0.0};
    if (valid_samples > 0) {
        mean.x = sum.x / valid_samples;
        mean.y = sum.y / valid_samples;
        mean.z = sum.z / valid_samples;
    }
    
   // Serial.printf("ADXL345: Mean acceleration over %dms: X=%.1f, Y=%.1f, Z=%.1f mg (from %d samples)\n",
   //               num_samples * sample_delay, mean.x, mean.y, mean.z, valid_samples);
    
    return mean;
}

float Adxl345Sensor::getActivityLevel() const {
    if (!last_measurement_success_ || !prev_sample_valid_) {
        return 0.0f;
    }
    
    // Calculate change in acceleration from previous sample
    float change_x = abs(accel_vector_.x - prev_accel_vector_.x);
    float change_y = abs(accel_vector_.y - prev_accel_vector_.y);
    float change_z = abs(accel_vector_.z - prev_accel_vector_.z);
    
    // Total change magnitude (how much acceleration changed)
    float total_change = change_x + change_y + change_z;
    
    // Scale to 0-5 range
    float activity_level = (total_change / MAX_ACTIVITY_THRESHOLD_MG) * 5.0f;
    
    // Clamp to 0-5 range
    if (activity_level > 5.0f) {
        activity_level = 5.0f;
    }
    
    return activity_level;
}