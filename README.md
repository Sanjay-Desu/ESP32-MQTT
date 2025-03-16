---

# ESP32-S3-EYE Camera System with MQTT Connectivity

This project implements an ESP32-S3-EYE camera system with MQTT connectivity for remote image capture and streaming. It provides a flexible foundation for IoT and computer vision applications.

## Table of Contents
1. [Camera Configuration](#camera-configuration)
2. [Wi-Fi/MQTT Integration](#wi-fi-mqtt-integration)
3. [Command Processing](#command-processing)
4. [Image Capture and Streaming](#image-capture-and-streaming)
5. [Optimization and Performance Considerations](#optimization-and-performance-considerations)
6. [Implementation Guide](#implementation-guide)
7. [Customization](#customization)
8. [Troubleshooting](#troubleshooting)

## Camera Configuration
### Hardware Setup
- **Camera Model**: ESP32-S3-EYE with OV2640 camera (2MP, 1600x1200 max resolution)
- **PSRAM Requirements**: 8MB Octal PSRAM configuration in menuconfig
- **Pin Assignments**: Defined in `camera_pins.h` for ESP32-S3-EYE model

### Image Capture Optimization
- **JPEG Compression**: Quality setting of 10 when PSRAM is detected
- **Frame Size**: SVGA (800x600) with PSRAM; falls back to VGA (640x480) without PSRAM

## Wi-Fi/MQTT Integration
- **Dual Connectivity**: Wi-Fi and MQTT
- **MQTT Library**: PubSubClient
- **Broker Details**: Connects to `test.mosquitto.org`
- **Topics**:
  - Subscribes to: `esp32cam/command`
  - Publishes to: `esp32cam/image` and `esp32cam/stream`
- **Reconnection Logic**: Implemented for MQTT

## Command Processing
- **Supported Commands**:
  - "Live Stream"
  - "Capture Image"
  - "Capture"
  - "Start Stream"
  - "Stop Stream"
- **Response**:
  - Base64 encoded images or starts/stops streaming based on commands

## Image Capture and Streaming
### Single Image Capture
- Triggered by MQTT command or button press
- Captures image, encodes to Base64, and publishes to MQTT
- Uses LED indicator (ws2812) for capture status

### Live Streaming
- Implements a simple streaming mechanism at ~20 fps
- Publishes Base64 encoded frames to `esp32cam/stream` topic
- Controlled by "Start Stream" and "Stop Stream" commands

## Optimization and Performance Considerations
### MQTT Buffer Size
- Increased to 60KB to handle large Base64 encoded images:
  ```cpp
  client.setBufferSize(60000);
  ```

### Frame Rate Control
- Uses a 50ms interval between frames for ~20 fps streaming:
  ```cpp
  const unsigned long streamInterval = 50;
  ```

### Memory Management
- Utilizes PSRAM when available for better performance
- Adjusts frame size and buffer count based on available memory

## Implementation Guide
### Setup
1. **Flashing**:
   - Flash the ESP32-S3-EYE with this code using Arduino IDE or PlatformIO
2. **Libraries**:
   - Ensure all required libraries are installed (`esp32-camera`, `PubSubClient`, `base64`)
3. **Configuration**:
   - Configure WiFi credentials and MQTT broker details

### Usage
1. **MQTT Connection**:
   - Connect to the MQTT broker using a client (e.g., MQTT Explorer)
2. **Commands**:
   - Send commands to the `esp32cam/command` topic:
     - "Capture Image|requestID" for single capture
     - "Start Stream" to begin streaming
     - "Stop Stream" to end streaming
3. **Image Reception**:
   - Receive images on `esp32cam/image` topic (single captures)
   - Receive stream frames on `esp32cam/stream` topic

## Customization
- **Resolution Adjustment**:
  - Change `FRAMESIZE_SVGA` to alter resolution (impacts performance)
- **Frame Rate Adjustment**:
  - Modify `streamInterval` to change frame rate
- **Additional Processing**:
  - Implement additional image processing or AI inference before sending

## Troubleshooting
- **Serial Output**:
  - Monitor Serial output for debugging information
- **LED Status**:
  - Check LED color codes for camera and connection status
- **Power Supply**:
  - Ensure sufficient power supply for stable operation

---

### References
1. [ESP32-Camera GitHub](https://github.com/espressif/esp32-camera)
2. [ESP32-S3-EYE Getting Started Guide](https://github.com/espressif/esp-who/blob/master/docs/en/get-started/ESP32-S3-EYE_Getting_Started_Guide.md)
3. [ESP32-CAM Video Streaming Tutorial](https://randomnerdtutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/)
4. [Publishing Images to MQTT](https://arduino.stackexchange.com/questions/72429/esp32-cam-publish-image-to-mqtt)
5. [ESP32 Forum Discussions](https://www.esp32.com/viewtopic.php?t=42397)

---

This project is designed to be adaptable and scalable for various IoT applications. Contributions and feedback are welcome!
