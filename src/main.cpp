#include <Arduino.h>
#include "config.h"
#include "mqtt.h"
#include "conversionHelpers.h"

#define TACHO_PIN 4
#define FAN_PWM_PIN 5
#define MODE_SET 0
#define MODE_MANUAL 1

unsigned long lastTrigger;
int mode = MODE_MANUAL;
int setSpeed = 25;
int lastManualValue = 0;
unsigned long reportTimer = 0;

ICACHE_RAM_ATTR void onFanRotation() {
  unsigned long now = millis();
  reportTimer += now-lastTrigger;
  if (reportTimer > 5000) {
    reportTimer = 0;
    char* fanSpeedStr = (char*) malloc(50);
    sprintf(fanSpeedStr, "%lu", 60000/(now-lastTrigger));
    publishToMqtt("currentFanSpeed", fanSpeedStr);
    Serial.printf("Fan speed: %s rpm.\n", fanSpeedStr);
    free(fanSpeedStr);
  }
  lastTrigger = now;
}

float voltageToAngle(float voltage) {
  return 3.16306348e-86 * pow(voltage - 4.62751482e+02, 33) + 5.33460164e-03 * voltage + 1.81394601e+02;
}

void setFanSpeed(char* payload) {
  int speedInPercent = strToInt(payload);
  if(speedInPercent >= 0) {
    Serial.printf("MQTT Message: Switching to set mode.\nSetting fan speed to %d%%.\n", speedInPercent);
    setSpeed = speedInPercent;
    mode = MODE_SET;
    int a = analogRead(A0);
    lastManualValue = constrain(map(voltageToAngle(a), 43, 204, 0, PWMRANGE), 0, PWMRANGE);
  } else {
    Serial.printf("The over MQTT requested speed (%s) was not a number in percent.\n", payload);
  }
}

void mqttSubscribe() {
  subscribeToTopic("setFanSpeed", setFanSpeed);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TACHO_PIN, INPUT_PULLUP);
  pinMode(FAN_PWM_PIN, OUTPUT);
  analogWriteRange(PWMRANGE);
  analogWriteFreq(25000);
  lastTrigger = millis();
  attachInterrupt(digitalPinToInterrupt(TACHO_PIN), onFanRotation, RISING);
  analogWrite(FAN_PWM_PIN, constrain(map(setSpeed, 0, 100, 0, PWMRANGE), 0, PWMRANGE));
  while(!Serial) {}  // Wait for Serial to start
  setUpConfig();
  mqttSetup(mqttSubscribe);
  reportTimer = 0;
  Serial.println("\n\nSetup complete.");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  //Serial.println("Loop iteration.");
  int a = analogRead(A0);
  if (mode == MODE_MANUAL) {
    lastManualValue = constrain(map(voltageToAngle(a), 43, 204, 0, PWMRANGE), 0, PWMRANGE);
  }
  if (mode == MODE_SET) {
    int newManualValue = constrain(map(voltageToAngle(a), 43, 204, 0, PWMRANGE), 0, PWMRANGE);
    //Serial.printf("Checking if abs(%d - %d)=%d is more than %f.\n", 
    //  newManualValue, lastManualValue, abs(newManualValue - lastManualValue), PWMRANGE/3.0);
    if (abs(newManualValue - lastManualValue) > PWMRANGE/3.0) {
      mode = MODE_MANUAL;
      lastManualValue = newManualValue;
      Serial.printf("Switched to manual at %f%%.\n", ((float) newManualValue)/PWMRANGE*100.0);
    }
  }
  mqttLoop();
  switch (mode) {
    case MODE_SET:
      analogWrite(FAN_PWM_PIN, constrain(map(setSpeed, 0, 100, 0, PWMRANGE), 0, PWMRANGE));
      break;
    case MODE_MANUAL:
      analogWrite(FAN_PWM_PIN, lastManualValue);
      break;
  }
  delay(1);
}