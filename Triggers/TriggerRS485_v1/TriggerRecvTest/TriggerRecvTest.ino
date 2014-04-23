`#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>

//#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "RS485Utils.h"

#define PIN_RS485_1    1
#define PIN_RS485_2    2
#define PIN_RS485_3    0

#define RCV_LED  7
#define TRIG_PIN 6
 
RS485Socket rs485(PIN_RS485_1, PIN_RS485_2, PIN_RS485_3, false);

void setup() {
  pinMode(RCV_LED, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);

  /* Setup the RS485 connection */  
  rs485.setup();
}

#define MY_ADDR RS485_ADDR_ANY
void loop() {
  unsigned int msglen;

  const byte *data = rs485.getMsg(MY_ADDR, &msglen);
  if (data != NULL) {
    char letter = data[0];
    byte count = data[1];
    
    if (count % 2 == 0) digitalWrite(TRIG_PIN, HIGH);
    else digitalWrite(TRIG_PIN, LOW);
    digitalWrite(RCV_LED, HIGH);
  } else {
    delay(10);
    digitalWrite(RCV_LED, LOW);
  }
}
