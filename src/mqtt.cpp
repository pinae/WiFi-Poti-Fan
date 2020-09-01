#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "mqtt.h"
#include "config.h"

#ifndef MQTT_GLOBALS
#define MQTT_GLOBALS

WiFiClient espClient;
PubSubClient mqttClient(espClient);
subscribedMqttTopicList* subscribedTopicsList;

#endif

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    char* plStr = (char*) calloc(length+1, sizeof(char));
    strncpy(&plStr[0], (char*) payload, length);
    *(plStr + length) = 0;
    Serial.printf("MQTT: %s (len: %d) - Payload: %s\n", 
                  topic, length, plStr);
    Serial.println("-----------------");
    Serial.print("subscribedTopicsList: "); Serial.println((int) subscribedTopicsList);
    subscribedMqttTopicList* stl = subscribedTopicsList;
    Serial.print("before loop... stl: "); Serial.println((int) stl);
    while(stl != NULL) {
        Serial.print("stl: "); Serial.println((int) stl);
        Serial.print("stl.entry: "); Serial.println((int) stl->entry);
        Serial.print("topic: "); Serial.println(topic);
        Serial.print("stl.entry.topic: "); Serial.println((char*) stl->entry->topic);
        Serial.print("stl.entry.topic: "); Serial.println((*(*stl).entry).topic);
        Serial.print("stl.entry.callback: "); Serial.println((int) stl->entry->callback);
        if (strcmp(stl->entry->topic, topic) == 0) {
            Serial.printf("Found topic: %s\n", stl->entry->topic);
            stl->entry->callback(plStr);
            break;
        }
        stl = (*stl).next;
    }
    free(plStr);
}

void subscribeToTopic(char* topic, void (*callback)(char*)) {
    char* deviceName = getDeviceName();
    unsigned int topicLen = strlen(deviceName)+1+strlen(topic);
    char* fullTopicStr = (char*) calloc(topicLen+1, sizeof(char));
    strncpy(fullTopicStr, deviceName, strlen(deviceName));
    strncpy(fullTopicStr+strlen(deviceName), "/", 1);
    strncpy(fullTopicStr+strlen(deviceName)+1, topic, strlen(topic));
    *(fullTopicStr + topicLen+1) = 0;
    mqttClient.subscribe(fullTopicStr);
    subscribedMqttTopic* newTopic = (subscribedMqttTopic*) malloc(sizeof(subscribedMqttTopic));
    newTopic->topic = fullTopicStr;
    newTopic->callback = callback;
    subscribedMqttTopicList* newTopicListElem = (subscribedMqttTopicList*) malloc(sizeof(subscribedMqttTopicList));
    newTopicListElem->entry = newTopic;
    newTopicListElem->next = subscribedTopicsList;
    subscribedTopicsList = newTopicListElem;
    Serial.print("Subscribed to MQTT-topic: "); Serial.println(fullTopicStr);
    //free(fullTopicStr);
}

void printPayload(char* payload) {
    Serial.println(payload);
}

void connectToMqtt() {
    boolean successfullyConnected = false;
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection... ");
        if (strlen(getMqttUsername()) > 0 && strlen(getMqttPassword())) {
            Serial.printf("\nMQTT: Using credentials: %s PW: %s\n", 
                          getMqttUsername(), getMqttPassword());
            successfullyConnected = mqttClient.connect(
                getDeviceName(), getMqttUsername(), getMqttPassword());
        } else {
            successfullyConnected = mqttClient.connect(getDeviceName());
        }
        if (successfullyConnected) {
            Serial.println("connected!");
            subscribeToTopic((char*) "setFanSpeed", printPayload);
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 2 seconds");
            // Wait 2 seconds before retrying
            delay(2000);
        }
    }
}

void mqttSetup() {
    mqttClient.setServer(getMqttServer(), getMqttPort());
    mqttClient.setCallback(onMqttMessage);
    connectToMqtt();
}

void publishToMqtt(char* topic, char* payload) {
    mqttClient.publish(topic, payload);
}

void mqttLoop() {
    if (!mqttClient.connected()) {
        connectToMqtt();
    }
    mqttClient.loop();
}