#include "esp_camera.h"
#define CAMERA_MODEL_ESP32S3_EYE
#include "camera_pins.h"
#include "ws2812.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <base64.h>

#define BUTTON_PIN  0

// WiFi credentials
const char* ssid = "Rohith's S23";
const char* password = "12346789";

// MQTT Broker details
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32cam/command";
const char* mqtt_image_topic = "esp32cam/image";
const char* mqtt_stream_topic = "esp32cam/stream";

WiFiClient espClient;
PubSubClient client(espClient);

bool isStreaming = false;
unsigned long lastStreamTime = 0;
const unsigned long streamInterval = 50; // Stream every 50ms (20 fps)

void setup_wifi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println("Received message: " + message);
    
    int separatorIndex = message.indexOf("|");
    if (separatorIndex != -1) {
        String command = message.substring(0, separatorIndex);
        String requestId = message.substring(separatorIndex + 1);
        
        Serial.println("Received Command: " + command);
        
        if (command == "Live Stream" || command == "Capture Image" || command == "Capture") {
            String imageBase64 = captureImageAndEncodeToBase64();
            String response = imageBase64 + "|" + requestId;
            client.publish(mqtt_image_topic, response.c_str());
        } else if (command == "Start Stream") {
            isStreaming = true;
            Serial.println("Starting live stream");
        } else if (command == "Stop Stream") {
            isStreaming = false;
            Serial.println("Stopping live stream");
        }
    }
}

String captureImageAndEncodeToBase64() {
    ws2812SetColor(3);
    camera_fb_t * fb = esp_camera_fb_get();
    if (fb == NULL) {
        Serial.println("Camera capture failed");
        ws2812SetColor(1);
        return "";
    }
    
    String base64Image = base64::encode(fb->buf, fb->len);
    esp_camera_fb_return(fb);
    
    Serial.printf("Image captured. Size: %d bytes, Base64 size: %d bytes\n", fb->len, base64Image.length());
    ws2812SetColor(2);
    return base64Image;
}

void reconnect() {
    int attempts = 0;
    while (!client.connected() && attempts < 3) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32CAMClient")) {
            Serial.println("connected");
            client.subscribe(mqtt_topic);
            Serial.println("Subscribed to topic: " + String(mqtt_topic));
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
        attempts++;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    ws2812Init();
    
    if (cameraSetup() == 1) {
        ws2812SetColor(2);
    } else {
        ws2812SetColor(1);
        return;
    }
  
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    if (client.setBufferSize(60000)) {
        Serial.println("MQTT buffer size set to 60 KB");
    } else {
        Serial.println("Failed to set MQTT buffer size");
    }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(20);
        if (digitalRead(BUTTON_PIN) == LOW) {
            while (digitalRead(BUTTON_PIN) == LOW);
            String imageBase64 = captureImageAndEncodeToBase64();
            String response = imageBase64 + "|button_pressed";
            client.publish(mqtt_image_topic, response.c_str());
        }
    }

    if (isStreaming && (millis() - lastStreamTime > streamInterval)) {
        String imageBase64 = captureImageAndEncodeToBase64();
        client.publish(mqtt_stream_topic, imageBase64.c_str());
        lastStreamTime = millis();
    }
}

int cameraSetup(void) {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_SVGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  
    if(psramFound()){
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        config.frame_size = FRAMESIZE_VGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return 0;
    }

    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, 1);
    s->set_brightness(s, 4);
    s->set_saturation(s, 1);
    s->set_contrast(s, 3);

    Serial.println("Camera configuration complete!");
    return 1;
}
