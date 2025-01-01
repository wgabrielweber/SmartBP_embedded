#include <Arduino.h>
#include <Wire.h>
#include "globalObjects.h"
#include "defines.h"
#include "commWiFiHandler.h"
#include "commMQTTHandler.h"
#include "commTimeManager.h"
#include "payload.h"
#include "measureRoutine.h"
#include "displayFunctions.h"
#include "MAX30105.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 15000; // 15 seconds for publishing
unsigned long lastRequestedPublishTime = 0;

// Define Global instances
MAX30105 ppgSensor;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

long startTime;
byte interruptPin = 2; //Connect INT pin on breakout board to pin 3

void setup() {
    Serial.begin(115200);   // Serial terminal for Debugging
    Wire.begin(); // Initialize I2C bus

    // Initialize WiFi and synchronize time
    setupWiFiAndTime();

    // Connect to MQTT and set callback
    client.setCallback(mqttCallback); 
    connectToMQTT();

    client.subscribe("prototype_esp/command");

    // Initialize sensor
    if (!ppgSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
    {
        Serial.println("MAX30105 was not found. Please check wiring/power. ");
        while (1);
    }

    // Sensor Setup
    sensorSetup(2);

    // Initialize OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("OLED initialization failed. Please check wiring.");
        while (1);
    }
    display.display();
    delay(2000); // Pause for 2 seconds
    display.clearDisplay();

}

void loop() {
    if (!client.connected()) {
        Serial.println("MQTT not connected. Reconnecting...");
        displayNotConnected();
        connectToMQTT(); // Reconnect if the client is disconnected
    }

    // Handle MQTT and ensure it is active
    client.loop();

    displayLoop();
}