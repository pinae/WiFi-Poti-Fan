#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h>
#include "debugLogger.h"
#include "conversionHelpers.h"
#include "configHelpers.h"
#include "webHandlers.h"
#include "mqtt.h"
// Include Update server
#ifdef ESP8266
# include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
# include <IotWebConfESP32HTTPUpdateServer.h>
#endif

#ifndef CONFIG_GLOBALS
#define CONFIG_GLOBALS
#define STRING_LEN 128

extern int PWMRANGE;

char mqttServerValue[STRING_LEN];
char mqttPortValue[6] = {'1', '8', '8', '3', '\0', '\0'};
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];
extern WebServer server;

// Create Update Server
#ifdef ESP8266
ESP8266HTTPUpdateServer httpUpdater;
#elif defined(ESP32)
HTTPUpdateServer httpUpdater;
#endif

extern IotWebConf iotWebConf;
iotwebconf::ParameterGroup mqttgroup = iotwebconf::ParameterGroup(
    "mqttgroup", "MQTT");
iotwebconf::TextParameter mqttServerParam = iotwebconf::TextParameter(
    "MQTT server", "mqttServer", mqttServerValue, STRING_LEN);
iotwebconf::NumberParameter mqttPortParam = iotwebconf::NumberParameter(
    "MQTT port", "mqttPort", mqttPortValue, 6, "text", "1883", "1883");
iotwebconf::TextParameter mqttUserNameParam = iotwebconf::TextParameter(
    "MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN);
iotwebconf::PasswordParameter mqttUserPasswordParam = iotwebconf::PasswordParameter(
    "MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN, "password");
bool needReset = false;
bool wifiIsConnected = false;
extern int mode;
extern int setSpeed;
extern int lastManualValue;
extern DebugLogger logger;

#endif

DeviceName::DeviceName() {
    byte mac[6]; WiFi.macAddress(mac);
    char macStr[18];
    sprintf(&macStr[0], "%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    strncpy(&deviceNameStr[14], macStr, 18);
}

void DeviceName::print() {
    byte mac[6]; WiFi.macAddress(mac);
    char macStr[18];
    sprintf(&macStr[0], "%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    logger.printf("MAC:\t\t%s\n", macStr);
    logger.printf("Device Name:\t%s\n", deviceNameStr);
}

char* DeviceName::get() {
    return &deviceNameStr[0];
}

void wifiConnected() {
    wifiIsConnected = true;
}

void configSaved() {
    //Serial.println("Configuration was updated.");
    needReset = true;
}

void initWifiAP() {
    //Serial.println("Initializing WiFi...");
    mqttgroup.addItem(&mqttServerParam);
    mqttgroup.addItem(&mqttPortParam);
    mqttgroup.addItem(&mqttUserNameParam);
    mqttgroup.addItem(&mqttUserPasswordParam);
    iotWebConf.addParameterGroup(&mqttgroup);
    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setWifiConnectionCallback(&wifiConnected);
    iotWebConf.setupUpdateServer(
    [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });
    boolean validConfig = iotWebConf.init();
    if (!validConfig) {
        mqttServerValue[0] = '\0';
        mqttUserNameValue[0] = '\0';
        mqttUserPasswordValue[0] = '\0';
    }
    server.on("/", handleRoot);
    server.on("/config", []{ iotWebConf.handleConfig(); });
    server.onNotFound([](){ iotWebConf.handleNotFound(); });
}

char* getMqttServer() {
    return mqttServerValue;
}

unsigned long getMqttPort() {
    int portVal = strToInt(mqttPortValue);
    if (portVal < 0) return 1883;
    return portVal;
}

char* getMqttUsername() {
    return mqttUserNameValue;
}

char* getMqttPassword() {
    return mqttUserPasswordValue;
}

void setFanSpeed(char* payload) {
  int speedInPercent = strToInt(payload);
  if(speedInPercent >= 0) {
    logger.printf("MQTT Message: Switching to set mode.\nSetting fan speed to %d%%.\n", speedInPercent);
    setSpeed = speedInPercent;
    mode = MODE_SET;
    int a = analogRead(A0);
    lastManualValue = constrain(map(a, 5, 1022, 0, PWMRANGE), 0, PWMRANGE);
  } else {
    logger.printf("The over MQTT requested speed (%s) was not a number in percent.\n", payload);
  }
}

void mqttSubscribe() {
  subscribeToTopic("setFanSpeed", setFanSpeed);
}

void loopWifiChecks() {
    mqttSetup(mqttSubscribe);
    mqttLoop();
    if (needReset) {
        Serial.println("Rebooting after 1 second.");
        iotWebConf.delay(1000);
        ESP.restart();
    }
}

void iotWebConfDelay(unsigned long duration) {
    iotWebConf.delay(duration);
}
