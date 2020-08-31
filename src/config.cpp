#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include "config.h"

#ifndef CONFIG_GLOBALS
#define CONFIG_GLOBALS

#define EEPROM_ADDR 0

configDataStruct configData;
char deviceName[31] = {'W', 'i', 'F', 'i', '-', 'P', 'o', 't', 'i', '-', 'F', 'a', 'n', '-' 
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#endif

void printConfig(configDataStruct configData) {
  Serial.println("--------- Configuration -----------");
  Serial.print("MQTT Server: "); Serial.println(configData.mqttServer);
  Serial.print("MQTT Port: "); Serial.println(configData.mqttPort);
  Serial.println("-----------------------------------");
}

void initDeviceName() {
    byte mac[6]; WiFi.macAddress(mac);
    char macStr[18];
    sprintf(&macStr[0], "%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    strncpy(&deviceName[14], macStr, 18);
    Serial.print("MAC:\t"); Serial.println(macStr);
    Serial.print("Device name:\t"); Serial.println(deviceName);
}

void setUpConfig() {
    WiFiManager wifiManager;
    initDeviceName();
    EEPROM.begin(CONFIG_LEN);
    EEPROM.get(EEPROM_ADDR, configData);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    WiFiManagerParameter custom_mqtt_server("mqttServer", "MQTT Server", 
                                            configData.mqttServer, MQTT_SERVER_STRLEN);
    wifiManager.addParameter(&custom_mqtt_server);
    char mqttPortStr[10] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
    sprintf(mqttPortStr, "%lu", configData.mqttPort);
    WiFiManagerParameter custom_mqtt_port("mqttPort", "MQTT Port", mqttPortStr, 10);
    wifiManager.addParameter(&custom_mqtt_port);
    WiFiManagerParameter custom_mqtt_username("mqttUsername", "MQTT Username", 
                                              configData.mqttUsername, MQTT_USERNAME_STRLEN);
    wifiManager.addParameter(&custom_mqtt_username);
    WiFiManagerParameter custom_mqtt_password("mqttPassword", "MQTT Password", 
                                              configData.mqttPassword, MQTT_PASSWORD_STRLEN);
    wifiManager.addParameter(&custom_mqtt_password);

    wifiManager.autoConnect(getDeviceName(), "loving_ct");

    strncpy(configData.mqttServer, custom_mqtt_server.getValue(), MQTT_SERVER_STRLEN);
    char** conversionErrorPos = 0;
    unsigned long convertedPort = strtoul(&mqttPortStr[0], conversionErrorPos, 10);
    if (conversionErrorPos == 0) {
        configData.mqttPort = convertedPort;
    } else {
        Serial.print("Error converting MQTT Port number: "); Serial.println(mqttPortStr);
    }
    strncpy(configData.mqttUsername, custom_mqtt_username.getValue(), MQTT_USERNAME_STRLEN);
    strncpy(configData.mqttPassword, custom_mqtt_password.getValue(), MQTT_PASSWORD_STRLEN);
    saveConfigCallback();

    printConfig(configData);
}

void saveConfigCallback() {
    EEPROM.put(EEPROM_ADDR, configData);
    EEPROM.commit();
}

char* getDeviceName() {
    return &deviceName[0];
}

char* getMqttServer() {
    return &(configData.mqttServer)[0];
}

unsigned long getMqttPort() {
    return configData.mqttPort;
}

char* getMqttUsername() {
    return &(configData.mqttUsername)[0];
}

char* getMqttPassword() {
    return &(configData.mqttPassword)[0];
}