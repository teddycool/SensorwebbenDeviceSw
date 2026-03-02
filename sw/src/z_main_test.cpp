/*
 * Ultra minimal test - compatible with both ESP32 and ESP8266
 * Just serial output to verify basic functionality
 */
#include <Arduino.h>

void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    delay(1000);  // Give serial time to initialize
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("ULTRA MINIMAL TEST - DEVICE STARTED!");
    Serial.println("========================================");
    
#ifdef ESP8266
    Serial.println("Chip: ESP8266");
    Serial.printf("Chip ID: %08X\n", ESP.getChipId());
    Serial.printf("Flash ID: %08X\n", ESP.getFlashChipId());
    Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
#elif defined(ESP32)
    Serial.println("Chip: ESP32");
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
#else
    Serial.println("Chip: Unknown");
#endif
    
    Serial.println("Setup completed successfully!");
    Serial.println("----------------------------------------");
}

void loop() {
    static unsigned long lastPrint = 0;
    static int counter = 0;
    
    unsigned long currentTime = millis();
    
    if (currentTime - lastPrint >= 3000) {  // Print every 3 seconds
        counter++;
        Serial.printf("[%lu] Loop #%d - Device is alive and running!\n", currentTime / 1000, counter);
        Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.println();
        lastPrint = currentTime;
    }
    
    delay(100);  // Small delay to prevent watchdog issues
}