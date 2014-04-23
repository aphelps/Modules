#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"
#include "GeneralUtils.h"


#define NUM_OUTPUTS 4
byte outputs[NUM_OUTPUTS] = { 2, 3, 6, 9 };

#define DEBUG_LED 13

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < NUM_OUTPUTS; i++) {
    pinMode(outputs[i], OUTPUT);
  }

  pinMode(DEBUG_LED, OUTPUT);
}

#define PERIOD_MS 1000
byte phase = 0;

void loop() {

#if 0
  digitalWrite(outputs[phase % NUM_OUTPUTS], LOW);
  phase++;
  digitalWrite(outputs[phase % NUM_OUTPUTS], HIGH);

  if (phase % 2) digitalWrite(DEBUG_LED, HIGH);
  else digitalWrite(DEBUG_LED, LOW);
  delay(PERIOD_MS);
#else

  static unsigned long next_period = 0;
  unsigned long now = millis();

  if (next_period == 0) next_period = now + PERIOD_MS;
  if (now >= next_period) {
    analogWrite(outputs[phase % NUM_OUTPUTS], 0);

    phase++;
    next_period = now + PERIOD_MS;
  }

  unsigned long remaining = next_period - now;

  // Output currently fading down
  byte current = phase % NUM_OUTPUTS;
  if (pin_is_PWM(outputs[current])) {
    analogWrite(outputs[current],
		 map(remaining, PERIOD_MS, 0, 255, 0));
  } else {
    digitalWrite(outputs[current], LOW);
  }

  // Output currently fading down
  byte next = (phase + 1) % NUM_OUTPUTS;
  if (pin_is_PWM(outputs[next])) {
    analogWrite(outputs[next],
		 map(remaining, PERIOD_MS, 0, 0, 255));
  } else {
    digitalWrite(outputs[next], HIGH);
  }
#endif
}
