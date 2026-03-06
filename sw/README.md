# Sensorwebben Device Software

This directory contains the firmware for Sensorwebben IoT sensor devices. The software is built using PlatformIO and Arduino framework, designed to run on ESP8266 and ESP32 microcontrollers.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Software Architecture](#software-architecture)
- [Project Structure](#project-structure)
- [PlatformIO Build System](#platformio-build-system)
- [Usage](#usage)

## Prerequisites

### For Developers

1. **Development Environment**: PlatformIO with ESP32/ESP8266 platform support
2. **Hardware**: ESP development board or custom PCB with auto-reset capability
3. **Testing Setup**: MQTT broker for local development and testing

For more details on Home Assistant, visit: [Home Assistant](https://www.home-assistant.io/).

## Software Architecture

### Core Interfaces

The firmware uses clean abstraction layers for extensibility:

**Sensor Interface** (`Sensor.h`)
```cpp
class Sensor {
public:
    virtual bool begin() = 0;
    virtual bool readSensor() = 0;
    virtual String getDiscoveryMsg() = 0;
    virtual String getPublishMsg() = 0;
};
```

**Publisher Interface** (`Publisher.h`)
```cpp
class Publisher {
public:
    virtual bool connect() = 0;
    virtual bool publish(String topic, String payload, bool retain = false) = 0;
    virtual bool isConnected() = 0;
};
```

### Implemented Components

**Sensor Drivers**:

* `DhtSensor` - DHT11/DHT22 via Adafruit DHT library
* `Hx711Sensor` - Precision weight measurement
* `Adxl345Sensor` - 3-axis accelerometer (I2C)
* `Ds18bSensor` - 1-Wire temperature sensors

**Publishers**:

* `MqttPublisher` - Local MQTT broker communication
* `HaRemoteClient` - Nabu Casa webhook integration

### Project Structure

```
sw/                      # Software (PlatformIO project)
├── src/                 # Main application code
├── include/             # Header files and interfaces
├── lib/                 # External dependencies
├── platformio.ini       # Build configuration
└── test/                # Test files
```

## PlatformIO Build System

Multiple build environments support different hardware platforms and use cases:

### Platform Configurations

**ESP8266 Targets**:
```ini
[env:esp8266_*]
platform = espressif8266
board = esp07  ; or d1_mini
framework = arduino
board_build.flash_mode = dout
board_build.f_cpu = 80000000L
upload_speed = 115200
```

**ESP32 Targets**:
```ini
[env:esp32_*]
platform = espressif32
board = esp32dev
framework = arduino
upload_protocol = esptool
; Auto-reset timing optimized for custom PCBs
```

### Device-Specific Environments

Each hardware variant has dedicated build configurations:

* `misto_*` - Multi-sensor environmental monitoring
* `apiscale_*` - Precision weight measurement platform  
* `cattoy_*` - Pet activity detection and monitoring
* `*_test` - Development and hardware validation

## Usage

### Building and Uploading

```bash
# Build specific environment
pio run -e misto_setup

# Upload to target device
pio run -e esp32_test --target upload

# Monitor serial output
pio device monitor -e esp32_test
```

### Development Workflow

1. **Configure Hardware**: Set up your ESP32/ESP8266 development board or custom PCB
2. **Install Dependencies**: PlatformIO will automatically handle library dependencies
3. **Select Environment**: Choose the appropriate build environment for your hardware
4. **Build & Upload**: Use PlatformIO commands to build and flash firmware
5. **Monitor Output**: Use serial monitor to debug and verify functionality

### Supported Build Targets

| Environment | Platform | Purpose | Hardware |
|-------------|----------|---------|----------|
| `misto_setup` | ESP8266 | Misto setup/calibration | ESP8266-07 |
| `misto_real` | ESP8266 | Misto production | ESP8266-07 |
| `esp32_test` | ESP32 | Development/testing | ESP32-DEV |
| `cat_toy` | ESP8266 | Cat activity detector | ESP8266-07 |
| `apiscale_real` | ESP32 | Precision scale | ESP32-DEV |

### Configuration

Device configuration is handled through:
* **Compile-time**: Build flags and environment settings in `platformio.ini`
* **Runtime**: Web-based captive portal for WiFi and MQTT settings
* **Persistent**: Configuration stored in flash memory (LittleFS/SPIFFS)

For more details about the overall project and hardware designs, see the [main project documentation](../readme.md).