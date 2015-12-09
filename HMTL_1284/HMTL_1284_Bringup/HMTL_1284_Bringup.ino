/*******************************************************************************
 * Author: Adam Phelps
 * License: Creative Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Board bringup
 */

#include "Arduino.h"

#define DEBUG_LEVEL DEBUG_TRACE
#include "Debug.h"

#include "SerialCLI.h"

#include "Adafruit_WS2801.h"
#include "SPI.h" // Comment out this line if using Trinket or Gemma

SerialCLI serialcli(32, cliHandler);

int pin = 0;
int mode = 0;
int mode_period = 1000;

#define MODE_CYCLE      0
#define MODE_SINGLE_PIN 1

#define NUM_LEDS 22
const int leds[NUM_LEDS] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, // 19
    21, 22, 23 // 20
};

uint8_t dataPin  = 5;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 7;    // Green wire on Adafruit Pixels

Adafruit_WS2801 strip = Adafruit_WS2801(10, dataPin, clockPin);

void setup() {

  Serial.begin(9600);

  for (int led = 0; led < NUM_LEDS; led++) {
    pinMode(led, OUTPUT);
  }

  strip.begin();
  strip.show();
}

// the loop routine runs over and over again forever:
int value = 0;
int color = 0;
void loop() {
  serialcli.checkSerial();

  if (mode == MODE_CYCLE) {
    for (int i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(color++ % 256)); //Color(25, 0, 255));
    }  
    strip.show();
  
    digitalWrite(leds[value % NUM_LEDS], LOW);
    value = (value + 1) % NUM_LEDS;
    digitalWrite(leds[value % NUM_LEDS], HIGH);

    Serial.print("Pin:");
    Serial.println(value);
  
    delay(mode_period);
  }
}

void setAllPixels(byte r, byte g, byte b) {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(r, g, b));
  }
  strip.show();
}


// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void print_usage() {
Serial.print(F("\n"
               "Usage:\n"
               "  e <pin>: Set the pin to HIGH\n"
               "  d <pin>: Set the pin to LOW\n"
               "  c [period]: Put into cycle mode\n"
               "  r <r> <g> <b>: Set pixel color to r,g,b\n"
               "\n"
               "  h - Print this help text\n"
               "\n"));
}

/*
 *  CLI Handler to setup the geometry         
 */
void cliHandler(char **tokens, byte numtokens) {

  // TODO: Add actions to first set the face/vertex position of each LED, then
  //       assign each edge for all triangles (light the LEDs on that edge)

  switch (tokens[0][0]) {
    case 'h': {
      print_usage();
      break;
    }

    case 'c': {
      mode = MODE_CYCLE;
      DEBUG2_PRINT("Entering cycle mode");
      if (numtokens >= 2) {
        mode_period = atoi(tokens[1]);
        DEBUG2_VALUE(" period:", mode_period);
      }

      break;
    }

    case 'e': {
      if (numtokens < 2) return;
      int read_pin = atoi(tokens[1]);
      if ((read_pin < 0) || (read_pin > 100)) return;

      mode = MODE_SINGLE_PIN;
      digitalWrite(read_pin, HIGH);

      DEBUG2_VALUELN("Enabled:", read_pin);
      break;
    }

    case 'd': {
      if (numtokens < 2) return;
      int read_pin = atoi(tokens[1]);
      if ((read_pin < 0) || (read_pin > 100)) return;

      digitalWrite(read_pin, LOW);
      mode = MODE_SINGLE_PIN;

      DEBUG2_VALUELN("Disabled:", read_pin);
      break;
    }

    case 'r': {
      if (numtokens < 4) return;
      int red = atoi(tokens[1]);
      int green = atoi(tokens[2]);
      int blue = atoi(tokens[3]);

      setAllPixels(red, green, blue);
      mode = MODE_SINGLE_PIN;

      DEBUG2_VALUE("Pixels: ", red);
      DEBUG2_VALUE(",", green);
      DEBUG2_VALUELN(",", blue);
      break;
    }
}

DEBUG_PRINT_END()
}
