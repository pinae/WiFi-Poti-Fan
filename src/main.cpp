#include <Arduino.h>

#define TACHO_PIN 4
#define FAN_PWM_PIN 5

unsigned long lastTrigger;

ICACHE_RAM_ATTR void onFanRotation() {
  unsigned long now = millis();
  //Serial.print("Fan speed: ");
  //Serial.println(60000/(now-lastTrigger));
  lastTrigger = now;
}

float voltageToAngle(float voltage) {
  return 3.16306348e-86 * pow(voltage - 4.62751482e+02, 33) + 5.33460164e-03 * voltage + 1.81394601e+02;
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
  while(!Serial) {}  // Wait for Serial to start
  Serial.println("\n\nSetup complete.");
}

void loop() {
  //Serial.println("Loop iteration.");
  int a = analogRead(A0);
  Serial.println(voltageToAngle(a));
  //Serial.println(constrain(map(a, 4, 910, 0, 255), 0, 255));
  analogWrite(FAN_PWM_PIN, constrain(map(voltageToAngle(a), 43, 204, 0, PWMRANGE), 0, PWMRANGE));
  //analogWrite(FAN_PWM_PIN, 375);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}