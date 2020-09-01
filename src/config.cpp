#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include "config.h"
#include "conversionHelpers.h"

#ifndef CONFIG_GLOBALS
#define CONFIG_GLOBALS

#define EEPROM_STATUS_ADDR 0
#define EEPROM_CONFIG_ADDR 4
boolean configNeedsSaving = false;
unsigned long configStatus = 0;

configDataStruct configData;
char deviceName[31] = {'W', 'i', 'F', 'i', '-', 'P', 'o', 't', 'i', '-', 'F', 'a', 'n', '-',
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#endif

void printConfig(configDataStruct configData) {
    Serial.println("--------- Configuration -----------");
    Serial.print("Device Name: "); Serial.println(getDeviceName());
    Serial.print("MQTT Server: "); Serial.println(configData.mqttServer);
    Serial.print("MQTT Port: "); Serial.println(configData.mqttPort);
    Serial.print("MQTT Username: "); Serial.println(configData.mqttUsername);
    Serial.print("MQTT Password: "); Serial.println(configData.mqttPassword);
    Serial.println("-----------------------------------");
}

void initDeviceName() {
    byte mac[6]; WiFi.macAddress(mac);
    char macStr[18];
    sprintf(&macStr[0], "%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    strncpy(&deviceName[14], macStr, 18);
    Serial.print("MAC:\t"); Serial.println(macStr);
    Serial.print("Device Name:\t"); Serial.println(deviceName);
}

void configModeCallback (WiFiManager *myWiFiManager) {
    Serial.println("Entered config mode.");
    configStatus = 0;
    EEPROM.put(EEPROM_STATUS_ADDR, configStatus);
    EEPROM.commit();
    strcpy(configData.mqttServer, "");
    configData.mqttPort = 1883;
    strcpy(configData.mqttUsername, "");
    strcpy(configData.mqttPassword, "");
    Serial.println("The config was reset:");
    printConfig(configData);
}

void saveConfigCallback() {
    configNeedsSaving = true;
}

void setUpConfig() {
    EEPROM.begin(CONFIG_LEN + 4);
    EEPROM.get(EEPROM_STATUS_ADDR, configStatus);
    Serial.print("configStatus: "); Serial.println(configStatus);
    if (configStatus == 1) {
        Serial.println("Configuration is ready to be loaded. Loading...");
        EEPROM.get(EEPROM_CONFIG_ADDR, configData);
        printConfig(configData);
    } else {
        Serial.println("Configuration is empty.");
        WiFi.disconnect();
    }

    WiFiManager wifiManager;
    initDeviceName();
    wifiManager.setAPCallback(configModeCallback);
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
    //WiFi.disconnect();

    if (configNeedsSaving) {
        strncpy(configData.mqttServer, custom_mqtt_server.getValue(), MQTT_SERVER_STRLEN);
        Serial.println(&mqttPortStr[0]);
        unsigned long convertedPort = strToInt(&mqttPortStr[0]);
        if (convertedPort >= 0) {
            configData.mqttPort = convertedPort;
        } else {
            Serial.print("Error converting MQTT Port number: "); Serial.println(mqttPortStr);
        }
        strncpy(configData.mqttUsername, custom_mqtt_username.getValue(), MQTT_USERNAME_STRLEN);
        strncpy(configData.mqttPassword, custom_mqtt_password.getValue(), MQTT_PASSWORD_STRLEN);
        saveConfig();
    }

    printConfig(configData);
}

void saveConfig() {
    EEPROM.put(EEPROM_CONFIG_ADDR, configData);
    EEPROM.commit();
    configStatus = 1;
    EEPROM.put(EEPROM_STATUS_ADDR, configStatus);
    EEPROM.commit();
    Serial.println("Successfully stored config in EEPROM.");
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