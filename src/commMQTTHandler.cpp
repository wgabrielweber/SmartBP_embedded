#include "commMQTTHandler.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "defines.h"
#include "globalObjects.h"
#include "commTimeManager.h"
#include "measureRoutine.h"

// MQTT Broker credentials and topics
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_data_topic = MQTT_DATA_TOPIC;

// Define global instances
WiFiClient espClient;     // Definition for the global instance of WiFiClientSecure
PubSubClient client(espClient); // Definition for the global instance of PubSubClient

void connectToMQTT() {
    client.setServer(mqtt_server, mqtt_port);
    while (!client.connected()) {
        Serial.print("Connecting to MQTT broker...");
        if (client.connect("ESP32Client")) {
            Serial.println("connected");
            client.subscribe(mqtt_data_topic); // Subscribe to a topic
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" trying again in 5 seconds");
            delay(5000);
        }
    }
}

void processCommand(String topic, short int payloadValue) {
    if (topic == "prototype_esp/command/setup") {
        if (payloadValue >= 1 && payloadValue <= 4) {
            Serial.print("Setting sensor parameter to: ");
            Serial.println(payloadValue);
            sensor_param = payloadValue;
            sensorSetup(payloadValue);  // Call sensorSetup with the parameter
            String sensorParam = "Sensor parameter set to: " + String(sensor_param);
            client.publish("prototype_esp/ack/command", sensorParam.c_str());
        } else {
            Serial.println("Error: Invalid setup parameter. Expected a value between 1 and 4.");
        }
    } else if (topic == "prototype_esp/command/requestIrMeasure") {
        if (payloadValue > 0) {
            Serial.print("Received IR measurement value: ");
            Serial.println(payloadValue);
            performMeasurementIrOnly(payloadValue);  // Call performMeasurementIrOnly
        } else {
            Serial.println("Error: Invalid IR measurement value. Expected a positive number.");
        }
    } else if (topic == "prototype_esp/command/requestMeasure") {
        if (payloadValue > 0) {
            Serial.print("Received measurement value: ");
            Serial.println(payloadValue);
            performMeasurement(payloadValue);  // Call performMeasurement (Red and IR)
        } else {
            Serial.println("Error: Invalid measurement value. Expected a positive number.");
        }
    } else {
        Serial.println("Error: Unknown MQTT command topic.");
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to a string
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    // Convert topic to String
    String topicStr = String(topic);

    // Check if the message is a valid number
    short int payloadValue = message.toInt();

    if (payloadValue > 0) {
        // Delegate command processing
        processCommand(topicStr, payloadValue);
    } else {
        Serial.println("Error: Invalid MQTT command payload. Expected a positive number.");
    }
}