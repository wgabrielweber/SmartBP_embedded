#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <ArduinoJson.h>

String createJsonPayload(unsigned int currentIndex, unsigned int* redValues, unsigned int* irValues);
String createStringPayload (String deviceId, short int sensor_param, unsigned long timestamp, unsigned int measureTime, unsigned int redArray[], unsigned int irArray[], int arraySize);
String createStringPayloadIrOnly(String deviceId, short int sensor_param, unsigned long timestamp, unsigned int measureTime, unsigned int irArray[], int arraySize);

#endif // PAYLOAD_H
