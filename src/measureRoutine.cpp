#include "measureRoutine.h"
#include "MAX30105.h"
#include "globalObjects.h"
#include "payload.h"
#include "commTimeManager.h"
#include "displayFunctions.h"

// Perform complete measurement and send data via mqtt
void performMeasurementIrOnly(unsigned short int num_samples) {   
    // Oled New Measure
    displayNewMeasure();

    // Turn the led on
    digitalWrite(LED_PIN, HIGH);
    
    // Wake the sensor
    ppgSensor.wakeUp();

    // Define constants
    const int MAX_SAMPLES = num_samples;   // Maximum number of samples
    const int EXTRA_SAMPLES = 20;          // Extra samples to exclude invalid readings in the sensor initialization 
    unsigned int irValues[MAX_SAMPLES];    // Array to store IR values
    unsigned int irReading; 
    unsigned int arraySamples = 0;
    unsigned int startMeasure = 0;
    unsigned int measureTime = 0;

    // Phase 1: Discard initial invalid samples
    int warmUpSamples = 0;
    while (warmUpSamples < EXTRA_SAMPLES) {
        ppgSensor.check(); // Check the sensor, read up to 3 samples

        while (ppgSensor.available()) {
            // Discard initial readings
            irReading = ppgSensor.getFIFOIR();
            warmUpSamples++;
            ppgSensor.nextSample(); // Move to the next sample
        }
    }

    // Auxiliary variable to calculate the measure time
    startMeasure = millis();

    // Phase 2: Collect valid samples
    while (arraySamples < MAX_SAMPLES) {
        ppgSensor.check(); // Check the sensor, read up to 3 samples

        while (ppgSensor.available()) {
            // Read and store valid samples
            irValues[arraySamples] = ppgSensor.getFIFOIR();
            arraySamples++;
            ppgSensor.nextSample(); // Move to the next sample
        }
    }

    // Calculate the measure time
    measureTime = millis() - startMeasure;

    // Shut the sensor down
    ppgSensor.shutDown();

    // Once the loop ends, array is full
    Serial.println("Data collection complete. Performing optimization...");

    // Turn the led off
    digitalWrite(LED_PIN, LOW);

    // Find the minimum values in the arrays
    unsigned int redMin = 262144;
    unsigned int irMin  = 262144;

    for (unsigned int i = 0; i < arraySamples; i++) {
        irMin = (irValues[i] < irMin) ? irValues[i] : irMin;
    }

    // Subtract the minimum value from all elements
    for (unsigned int i = 0; i < arraySamples; i++) {
        irValues[i] -= irMin;
    }

    //String payload = createJsonPayload(arraySamples, redValues, irValues);    // Send payload in json format
    String payload = createStringPayloadIrOnly("ESP32", sensor_param, getEpochTime(), measureTime, irValues, MAX_SAMPLES);   // Send payload in string format
    client.publish("prototype_esp/data", payload.c_str());
        
    Serial.print("Data published successfully.");

    // Oled Data Sent
    displayDataSent();
}

// Perform complete measurement and send data via mqtt
void performMeasurement(unsigned short int num_samples) {   
    // Oled New Measure
    displayNewMeasure();
    
    // Wake the sensor
    ppgSensor.wakeUp();

    // Define constants
    const int MAX_SAMPLES = num_samples;   // Maximum number of samples
    const int EXTRA_SAMPLES = 20;          // Extra samples to exclude invalid readings in the sensor initialization 
    unsigned int redValues[MAX_SAMPLES];   // Array to store red values
    unsigned int irValues[MAX_SAMPLES];    // Array to store IR values
    unsigned int redReading;
    unsigned int irReading; 
    unsigned int arraySamples = 0;
    unsigned int startMeasure = 0;
    unsigned int measureTime = 0;

    // Phase 1: Discard initial invalid samples
    int warmUpSamples = 0;
    while (warmUpSamples < EXTRA_SAMPLES) {
        ppgSensor.check(); // Check the sensor, read up to 3 samples

        while (ppgSensor.available()) {
            // Discard initial readings
            redReading = ppgSensor.getFIFORed();
            irReading = ppgSensor.getFIFOIR();
            warmUpSamples++;
            ppgSensor.nextSample(); // Move to the next sample
        }
    }

    // Auxiliary variable to calculate the measure time
    startMeasure = millis();

    // Phase 2: Collect valid samples
    while (arraySamples < MAX_SAMPLES) {
        ppgSensor.check(); // Check the sensor, read up to 3 samples

        while (ppgSensor.available()) {
            // Read and store valid samples
            redValues[arraySamples] = ppgSensor.getFIFORed();
            irValues[arraySamples] = ppgSensor.getFIFOIR();
            arraySamples++;
            ppgSensor.nextSample(); // Move to the next sample
        }
    }

    // Calculate the measure time
    measureTime = millis() - startMeasure;

    // Shut the sensor down
    ppgSensor.shutDown();

    // Once the loop ends, array is full
    Serial.println("Data collection complete. Performing optimization...");

    // Find the minimum values in the arrays
    unsigned int redMin = 262144;
    unsigned int irMin  = 262144;

    for (unsigned int i = 0; i < arraySamples; i++) {
        redMin = (redValues[i] < redMin) ? redValues[i] : redMin;
        irMin = (irValues[i] < irMin) ? irValues[i] : irMin;
    }

    // Subtract the minimum value from all elements
    for (unsigned int i = 0; i < arraySamples; i++) {
        redValues[i] -= redMin;
        irValues[i] -= irMin;
    }

    //String payload = createJsonPayload(arraySamples, redValues, irValues);    // Send payload in json format
    String payload = createStringPayload("ESP32", sensor_param, getEpochTime(), measureTime, redValues, irValues, MAX_SAMPLES);   // Send payload in string format
    client.publish("prototype_esp/data", payload.c_str());
        
    Serial.println("Data published successfully.");

    // Oled Data Sent
    displayDataSent();
}

// Function to set up the sensor parameters
void sensorSetup(short int parameter) {
    // Default Sensor Configuration
    byte ledBrightness = 0x1F;    // Options: 0=Off to 255=50mA
    byte sampleAverage = 8;       // Options: 1, 2, 4, 8, 16, 32
    byte ledMode = 2;             // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    int sampleRate = 1000;        // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulseWidth = 118;         // Options: 69, 118, 215, 411
    int adcRange = 16;            // Options: 15, 16, 17, 18 [bits]

    // Configure sensor parameters based on the input parameter
    switch (parameter) {
        case 1:
            sampleRate = 800;
            pulseWidth = 215;
            adcRange = 17;
            sampleAverage = 4;  // 200Hz with 4 samples
            break;
        case 2:
            sampleRate = 1000;
            pulseWidth = 118;
            adcRange = 16;
            sampleAverage = 8;  // 125Hz with 8 samples
            break;
        case 3:
            sampleRate = 1600;
            pulseWidth = 69;
            adcRange = 15;
            sampleAverage = 8;  // 200Hz with 8 samples
            break;
        case 4:
            sampleRate = 1600;
            pulseWidth = 69;
            adcRange = 15;
            sampleAverage = 16; // 100Hz with 16 samples
            break;
        default:
            Serial.println("Invalid parameter. Using default configuration.");
            break;
    }

    // Apply the configuration to the sensor
    ppgSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); // Configure sensor

    ppgSensor.enableAFULL();          // Enable the almost full interrupt (default is 32 samples)
    ppgSensor.setFIFOAlmostFull(3);   // Set almost full interrupt to fire at 29 samples
    ppgSensor.shutDown();             // Shut down the sensor, wake up only when a measure is made
}