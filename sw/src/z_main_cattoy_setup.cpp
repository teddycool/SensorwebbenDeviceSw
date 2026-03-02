#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "Adxl345Sensor.h"
#include "MqttPublisher.h"
#include "boxsecrets.h"

// ========= PIN DEFINES (ADAPT TO YOUR PCB) =========

// I2C to GY-291 / ADXL345
#define I2C_SDA   14      // your wiring: SDA on GPIO14
#define I2C_SCL   12      // your wiring: SCL on GPIO12
#define ADXL_ADDR 0x53    // GY-291 with SDO -> GND

// Optional LED + motor for feedback
#define LED_PIN    -1      // set to your LED pin (or -1 if no LED)
#define MOTOR_PIN  4     // set to your motor MOSFET pin (or -1 if no motor)

// ========= SENSOR INSTANCE =========
Adxl345Sensor adxlSensor(I2C_SDA, I2C_SCL, ADXL_ADDR);

// ========= WIFI & MQTT INSTANCES =========
WiFiClient wifiClient;
MqttPublisher mqttPublisher(wifiClient);

// ========= DATA STRUCTURES =========
struct AccelReading {
  float x, y, z;        // acceleration values in mg
  float activityLevel;  // activity level (0-5 scale)
  uint32_t timestamp;   // timestamp in milliseconds
};

// ========= CONFIG =========
static const float    ACTIVITY_THRESHOLD = 0.3f;       // Activity level threshold (0-5 scale)
static const uint8_t  READINGS_BUFFER_SIZE = 10;       // Store 10 readings per second

// ========= STATE =========
// Removed lastMotionMs since we're not tracking idle time anymore
static AccelReading readingsBuffer[READINGS_BUFFER_SIZE];
static uint8_t currentReadingIndex = 0;



// ========= LED / MOTOR HELPERS =========
static void ledOn() {
  if (LED_PIN >= 0) digitalWrite(LED_PIN, LOW);  // active-low assumption
}

static void ledOff() {
  if (LED_PIN >= 0) digitalWrite(LED_PIN, HIGH);
}

static void motorKick(uint16_t duty, uint16_t ms) {
  if (MOTOR_PIN < 0) return;
  analogWrite(MOTOR_PIN, duty);
  delay(ms);
  analogWrite(MOTOR_PIN, 0);
}


// ========= MOTION / IDLE LOGIC =========

static bool detectMotion() {
  // Use the new activity level (0-5 scale) for motion detection
  float activityLevel = adxlSensor.getActivityLevel();
  return (activityLevel > ACTIVITY_THRESHOLD);
}

static void printReadingsBuffer() {
  Serial.println("\n========== 1 SECOND OF READINGS (10 samples) ==========");
  for (uint8_t i = 0; i < READINGS_BUFFER_SIZE; i++) {
    const AccelReading& reading = readingsBuffer[i];
    Serial.printf("[%2d] mg: %6.0f %6.0f %6.0f | activity: %.2f | t: %lu\n",
                  i, reading.x, reading.y, reading.z, reading.activityLevel, 
                  (unsigned long)reading.timestamp);
  }
  Serial.println("======================================================\n");
}

static String createJsonFromBuffer() {
  // Create JSON document with buffer for 10 readings
  StaticJsonDocument<2048> doc;
  
  // Add metadata
  doc["device"] = "cattoy_accelerometer";
  doc["chip_id"] = String(ESP.getChipId());
  doc["timestamp"] = millis();
  doc["sample_count"] = READINGS_BUFFER_SIZE;
  doc["sample_rate_hz"] = 10;
  
  // Create readings array
  JsonArray readings = doc.createNestedArray("readings");
  
  for (uint8_t i = 0; i < READINGS_BUFFER_SIZE; i++) {
    const AccelReading& reading = readingsBuffer[i];
    
    JsonObject readingObj = readings.createNestedObject();
    readingObj["index"] = i;
    readingObj["x"] = reading.x;
    readingObj["y"] = reading.y;
    readingObj["z"] = reading.z;
    readingObj["activity_level"] = reading.activityLevel;
    readingObj["timestamp"] = reading.timestamp;
  }
  
  // Serialize to string
  String jsonString;
  serializeJson(doc, jsonString);
  
  return jsonString;
}

static bool hasSignificantActivity() {
  // Check if any reading in the buffer has activity level > 1.0
  for (uint8_t i = 0; i < READINGS_BUFFER_SIZE; i++) {
    if (readingsBuffer[i].activityLevel > 1.0f) {
      return true;
    }
  }
  return false;
}

static void setupWifiAndMqtt() {
  // Connect to WiFi using stored credentials
  WiFi.begin(cssid, cpassword);
  
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Initialize MQTT publisher
    String chipId = String(ESP.getChipId());
    mqttPublisher.initialize(chipId, local_mqtt_server, local_mqtt_port, local_mqtt_user, local_mqtt_pw);
  } else {
    Serial.println();
    Serial.println("WiFi connection failed!");
  }
}

// Deep sleep functionality removed - now continuously monitoring accelerometer

// ========= SETUP / LOOP =========

void setup() {
  if (LED_PIN >= 0) {
    pinMode(LED_PIN, OUTPUT);
    ledOff();
  }
  if (MOTOR_PIN >= 0) {
    pinMode(MOTOR_PIN, OUTPUT);
    digitalWrite(MOTOR_PIN, HIGH);
   // analogWriteRange(1023);
   // analogWriteFreq(1000);
   // analogWrite(MOTOR_PIN, 0);
   // motorKick(80, 1000);
   delay(1000);
   digitalWrite(MOTOR_PIN, LOW);
  }

  Serial.begin(115200);
  delay(500);
  Serial.println("\nCat Toy Core v2 — With Activity Level Detection");

  // Initialize the ADXL345 sensor
  bool sensorInit = adxlSensor.performMeasurement();
  if (!sensorInit) {
    Serial.println("ADXL345 sensor initialization failed!");
  } else {
    Serial.println("ADXL345 sensor initialized successfully.");
  }

  // Small feedback that we just booted
  ledOn();
  motorKick(700, 100);
  ledOff();

  // Setup WiFi and MQTT
  setupWifiAndMqtt();

  Serial.println("Running in continuous monitoring mode - no sleep.");
}

void loop() {
  static uint32_t lastSampleMs = 0;
  uint32_t now = millis();

  if (now - lastSampleMs >= 100) { // ~10 Hz
    lastSampleMs = now;



    // Perform measurement
    bool measurementOk = adxlSensor.performMeasurement();
    if (!measurementOk) {
      Serial.println("Sensor measurement failed!");
      delay(10);
      return;
    }

    // Check for motion using activity level
    bool active = detectMotion();
    if (active) {
      ledOn();
    } else {
      ledOff();
    }

    // Get sensor data
    AdxlVector accel = adxlSensor.getAcceleration();
    float activityLevel = adxlSensor.getActivityLevel();

    // Store reading in buffer
    readingsBuffer[currentReadingIndex].x = accel.x;
    readingsBuffer[currentReadingIndex].y = accel.y;
    readingsBuffer[currentReadingIndex].z = accel.z;
    readingsBuffer[currentReadingIndex].activityLevel = activityLevel;
    readingsBuffer[currentReadingIndex].timestamp = now;

      // Increment buffer index
    currentReadingIndex++;

    // Check if buffer is full (10 readings = 1 second)
    if (currentReadingIndex >= READINGS_BUFFER_SIZE) {
      // Print all readings from the past second
      printReadingsBuffer();
      
      // Check if any reading has significant activity (> 1.0)
      bool hasActivity = hasSignificantActivity();
      
      if (hasActivity) {
        // Create JSON from buffer
        String jsonPayload = createJsonFromBuffer();
        Serial.println("📈 Significant activity detected! JSON payload created:");
        Serial.println(jsonPayload);
        
        // Publish to MQTT if WiFi is connected
        if (WiFi.status() == WL_CONNECTED) {
          String topic = "cattoy/" + String(ESP.getChipId()) + "/accelerometer";
          bool published = mqttPublisher.publish(topic, jsonPayload, false);
          if (published) {
            Serial.println("✓ Data published to MQTT successfully");
          } else {
            Serial.println("✗ Failed to publish to MQTT");
          }
        } else {
          Serial.println("⚠ WiFi not connected - skipping MQTT publish");
        }
      } else {
        Serial.println("😴 No significant activity detected (all readings ≤ 1.0) - skipping MQTT publish");
      }
      
      // Reset buffer index for next second
      currentReadingIndex = 0;
      Serial.println("Buffer reset - starting new 1-second cycle\n");
    }
  }

  // Simple cooperative loop
  delay(10);
}
