#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include "globalObjects.h"      // Include the global espClient declaration

void connectToMQTT();
void processCommand(String topic, short int payloadValue);
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif
