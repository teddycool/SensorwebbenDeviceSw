# Misto2 - Environmental Sensor Device

## Table of Contents
- [Overview](#overview)
- [Hardware Specifications](#hardware-specifications)
- [Key Features](#key-features)
- [Technical Specifications](#technical-specifications)
- [Use Cases](#use-cases)
- [Programming Interface](#programming-interface)
- [References and Design Files](#references-and-design-files)

---

The Misto2 is a compact, multi-power environmental monitoring device designed for long-term indoor deployment. Built around the ESP8266 microcontroller, it provides wireless connectivity for home automation and environmental monitoring applications through MQTT integration with Home Assistant.

**Key Characteristics**: Ultra-low power design, USB-C convenience, wireless configuration, automatic discovery, and flexible power options with 6+ month battery lifespan.

All hardware and software designs are open-source and available in this repository for building custom Misto2 devices or as inspiration for other IoT projects.

## Overview

Misto2 is a battery-powered IoT sensor platform optimized for long-term environmental monitoring. The device measures temperature, humidity, and battery status, with intelligent power management enabling months of autonomous operation.

**Primary Features**:
* Environmental sensing (temperature, humidity)
* Battery voltage and RSSI monitoring  
* MQTT communication with Home Assistant auto-discovery
* Web-based configuration portal
* Deep sleep power management
* USB-C programming, charging, and power supply

**Target Applications**: Indoor climate monitoring, smart home integration, energy-efficient data logging, and remote environmental sensing.

![Misto2 PCB without enclosure](misto2_hardware.jpg)

## Hardware Specifications

### Microcontroller Platform
* **MCU**: ESP8266-07
* **Flash**: 4MB SPI flash memory
* **RAM**: 80KB user data RAM
* **Connectivity**: WiFi 802.11 b/g/n (2.4GHz)
* **Power Modes**: Active, light sleep, deep sleep 

### Sensor Subsystem
* **Environmental**: DHT22 (temperature ±0.5°C, humidity ±2% RH)
* **Power Monitoring**: ADC-based battery voltage measurement
* **Signal Quality**: WiFi RSSI reporting
* **Update Rate**: Configurable 

### Power Management
* **Supply Options**: 
  - 3x AAA batteries (4.5V nominal)
  - Rechargeable LiPo battery with USB-C charging
  - Direct USB-C power supply (5V)
* **Regulation**: Onboard 3.3V LDO with low dropout
* **USB-C Charging**: Integrated charging circuit for LiPo batteries
* **Sleep Current**: <20µA in deep sleep mode
* **Active Current**: ~80mA during WiFi transmission
* **Battery Life**: 6-12 months (depends on reporting interval and power source)

### Physical Interface
* **Programming & Power**: USB-C connector (programming, charging, power supply)
* **Indicators**: Status LED for configuration and error states
* **Enclosure**: 3D-printed protective housing

## Key Features

### Flexible Power Options
The Misto2 offers three convenient power options:

* **3x AAA batteries** - Traditional battery operation for maximum portability
* **Rechargeable LiPo** - USB-C rechargeable option for permanent installations
* **Direct USB-C** - Continuous power supply via USB-C cable

### USB-C Convenience
* Single USB-C connector for programming, charging, and power
* No need for separate programming headers
* Standard USB-C cables for easy connectivity
* Integrated charging circuit for LiPo batteries

### Smart Power Management
The device implements intelligent power management through:

* Configurable sleep intervals (minutes to hours)
* Automatic sensor power control
* Battery voltage monitoring with low-battery alerts
* Deep sleep mode between measurements
* Automatic power source detection

### Easy Configuration
* WiFi credentials and setup through web-portal, accessible via WiFi
* No programming required for basic operation
* Configurable MQTT broker settings
* USB-C programming without special cables or adapters

### Home Assistant Integration
* Automatic device discovery
* Real-time sensor data reporting
* Battery status monitoring
* Signal strength reporting
* Customizable measurement intervals

### Robust Operation
* Automatic WiFi reconnection
* MQTT connection retry logic
* Error indication through LED feedback

## Technical Specifications

| Parameter | Specification |
|-----------|---------------|
| Operating Voltage | 3.3V (regulated from various power sources) |
| Supply Range | 2.7V - 5.5V (battery voltage or USB-C) |
| USB-C Input | 5V (standard USB power) |
| Temperature Range | -40°C to +80°C (sensor), 0°C to +50°C (operation) |
| Humidity Range | 0-100% RH (±2% accuracy) |
| Temperature Accuracy | ±0.5°C (DHT22) |
| WiFi Standards | 802.11 b/g/n (2.4GHz only) |
| Communication | MQTT over WiFi |
| Battery Type | 3x AAA (alkaline, NiMH, lithium) or rechargeable LiPo |
| Charging | USB-C charging for LiPo batteries |
| Battery Life | 6-12 months (dependent on reporting interval and power source) |
| Sleep Current | <20µA (deep sleep) |
| Active Current | ~80mA (WiFi active) |
| Dimensions | TBD (with enclosure) |
| Operating Temperature | -20°C to +50°C |
| Storage Temperature | -20°C to +70°C |

## Use Cases

* Indoor climate monitoring
* Home automation sensor networks
* Energy-efficient environmental logging
* Remote monitoring applications
* Smart home integration projects
* Permanent installations with USB-C power
* Portable monitoring with rechargeable batteries

## Programming Interface

Misto2 features a convenient USB-C connector for all programming, charging, and power supply needs. This eliminates the need for separate programming headers and special cables.

### USB-C Interface Features

| Feature | Description |
|---------|-------------|
| Programming | Firmware updates via standard USB-C cable |
| Power Supply | Direct 5V power for continuous operation |
| Charging | Automatic LiPo battery charging when connected |
| Data Communication | UART communication for monitoring and debugging |

### Programming Procedure

1. **Connect USB-C cable** from computer to Misto2
2. **For flashing**: Device automatically enters programming mode when detected
3. **For monitoring**: Use serial monitor at 9600 baud, 8N1
4. **Power options**: 
   - USB-C only (remove batteries)
   - USB-C + LiPo charging (batteries can remain)
   - Battery only (disconnect USB-C)

**Convenience**: No special programming cables, headers, or mode pins required. Standard USB-C cable provides all functionality.

### Power Source Priority
* USB-C power takes priority when connected
* Automatic switching between USB-C and battery power
* LiPo charging occurs automatically when USB-C is connected
* Battery voltage monitoring works with all power sources

![Misto2 USB-C Interface](../../doc/misto2-usb-interface.jpg)

## References and Design Files

This section provides access to hardware design files and documentation for the Misto2 environmental sensor device.

### Open Source Design Files

#### Electrical Design
* [Misto2 Schematic (PDF)](misto2_circuit.pdf) - Complete electrical schematic diagram
* **PCB Files**: KiCad design files (coming soon)
* **PCB Ordering**: Will be available through open PCB services

#### Mechanical Design  
* [Enclosure Design Files (ZIP)](misto2-box-FreeCad.zip) - FreeCAD source files for 3D-printed enclosure

### Key Design Improvements
* Integrated USB-C connector for programming and power
* Multiple power source options (AAA, LiPo, USB-C)
* Simplified programming interface
* Enhanced charging capabilities

### Manufacturing Information

The provided design files enable:

* 3D printing of protective enclosures
* Understanding of electrical design for custom variations
* Reference for developing compatible devices
* USB-C integration examples

**Licensing**: All design files are provided under open-source licensing terms.

---

**Note**: Misto2 represents the evolution of the original Misto design, adding USB-C convenience and flexible power options while maintaining the same environmental sensing capabilities and Home Assistant integration.
