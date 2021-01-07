#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <IotWebConf.h>
#include "debugLogger.h"
#include "mqtt.h"
#include "configHelpers.h"
#include "conversionHelpers.h"

#define TACHO_PIN 5
#define FAN_PWM_PIN 4
#define CONFIG_VERSION "v0.2.1"

DeviceName devName;
DebugLogger logger;
unsigned long lastTrigger;
int mode = MODE_MANUAL;
int setSpeed = 25;
int lastManualValue = 0;
unsigned long reportTimer = 0;
unsigned long lastSpeedReport = 0;
float fanSpeed = 0;
const char wifiInitialApPassword[] = "loving_ct";
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(devName.get(), &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

ICACHE_RAM_ATTR void onFanRotation() {
  unsigned long now = millis();
  //logger.printf("onFanRotation: %ul %ul\n", lastTrigger, now);
  reportTimer += now-lastTrigger;
  if (reportTimer > 5000) {
    reportTimer = 0;
    fanSpeed = 60000/(now-lastTrigger);
  }
  lastTrigger = now;
}

float voltageToAngle(float voltage) {
  return 3.16306348e-86 * pow(voltage - 4.62751482e+02, 33) + 5.33460164e-03 * voltage + 1.81394601e+02;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(TACHO_PIN, INPUT_PULLUP);
  pinMode(FAN_PWM_PIN, OUTPUT);
  analogWriteRange(PWMRANGE);
  analogWriteFreq(25000);
  lastTrigger = millis();
  attachInterrupt(digitalPinToInterrupt(TACHO_PIN), onFanRotation, RISING);
  analogWrite(FAN_PWM_PIN, constrain(map(setSpeed, 0, 100, 0, PWMRANGE), 0, PWMRANGE));
  while(!Serial) {}  // Wait for Serial to start
  initWifiAP();
  reportTimer = 0;
  devName.print();
  Serial.println("\n\nSetup complete.");
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  //Serial.println("Loop iteration.");
  unsigned long now = millis();
  logger.printAllWithSerial();
  iotWebConf.doLoop();
  loopWifiChecks();
  int a = analogRead(A0);
  if (mode == MODE_MANUAL) {
    lastManualValue = constrain(map(a, 5, 1022, 0, PWMRANGE), 0, PWMRANGE);
  }
  if (mode == MODE_SET) {
    int newManualValue = constrain(map(a, 5, 1022, 0, PWMRANGE), 0, PWMRANGE);
    //Serial.printf("Checking if abs(%d - %d)=%d is more than %f.\n", 
    //  newManualValue, lastManualValue, abs(newManualValue - lastManualValue), PWMRANGE/3.0);
    if (abs(newManualValue - lastManualValue) > PWMRANGE/3.0) {
      mode = MODE_MANUAL;
      lastManualValue = newManualValue;
      Serial.printf("Switched to manual at %f%%.\n", ((float) newManualValue)/PWMRANGE*100.0);
    }
  }
  //Serial.printf("%d Manual value: %d\n", mode, lastManualValue);
  switch (mode) {
    case MODE_SET:
      analogWrite(FAN_PWM_PIN, constrain(map(setSpeed, 0, 100, 0, PWMRANGE), 0, PWMRANGE));
      break;
    case MODE_MANUAL:
      analogWrite(FAN_PWM_PIN, lastManualValue);
      break;
  }
  if ((now-lastSpeedReport) > 5000) {
    char* fanSpeedStr = (char*) malloc(50);
    sprintf(fanSpeedStr, "%.1f", fanSpeed);
    publishToMqtt("currentFanSpeed", fanSpeedStr);
    Serial.printf("Fan speed: %s rpm.\n", fanSpeedStr);
    free(fanSpeedStr);
    lastSpeedReport = now;
  }
}
