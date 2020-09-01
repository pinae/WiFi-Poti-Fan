#ifndef MQTT_TYPES
#define MQTT_TYPES

struct subscribedMqttTopic {
    char* topic;
    void (*callback)(char*);
};

struct subscribedMqttTopicList {
    subscribedMqttTopic* entry;
    subscribedMqttTopicList* next;
};

#endif

void mqttSetup();
void mqttLoop();
void subscribeToTopic(char* topic);
void publishToMqtt(char* topic, char* payload);