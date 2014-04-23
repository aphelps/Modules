/*
 * This code powers an HMTL Module connected to a 4-channel trigger board.
 *
 * This assumes that the HMTL configuration has already been written (see
 * HMTL_Bringup).
 */

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>

#include "SPI.h"
#include "Adafruit_WS2801.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "PixelUtil.h"
#include "Wire.h"
#include "MPR121.h"
#include "SerialCLI.h"
#include "RS485Utils.h"

#define MAX_OUTPUTS 8
config_hdr_t config;
output_hdr_t *outputs[MAX_OUTPUTS];
config_max_t readoutputs[MAX_OUTPUTS];

#define MAX_TRIGGERS 4
config_value_t *triggers[MAX_TRIGGERS];
uint8_t num_triggers;

RS485Socket rs485;
config_rs485_t *rs485_output = NULL;

config_rgb_t *rgb_output = NULL; // RGB LED for debugging

SerialCLI serialcli(32, cliHandler);

void setup() {
  Serial.begin(9600);

  /* Read the configuration */
  config.address = HMTL_NO_ADDRESS;
  int configOffset = hmtl_read_config(&config, readoutputs, MAX_OUTPUTS);
  if (configOffset < 0) {
    DEBUG_ERR("Failed to read configuration!!!");
    DEBUG_ERR_STATE(0);
  }
  DEBUG_VALUELN(DEBUG_LOW, "Read config.  offset=", configOffset);

  /* Setup the output array */
  for (int i = 0; i < config.num_outputs; i++) {
    if (i >= MAX_OUTPUTS) {
      DEBUG_VALUELN(0, "Too many outputs:", config.num_outputs);
      return;
    }
    outputs[i] = (output_hdr_t *)&readoutputs[i];
  }

  DEBUG_VALUE(DEBUG_HIGH, "Config size:", configOffset - HMTL_CONFIG_ADDR);
  DEBUG_VALUELN(DEBUG_HIGH, " end:", configOffset);
  DEBUG_COMMAND(DEBUG_HIGH, hmtl_print_config(&config, outputs));

  if (config.address == HMTL_NO_ADDRESS) {
    DEBUG_ERR("No address was configured");
    DEBUG_ERR_STATE(1);
  }


  /* Initialize the outputs */
  num_triggers = 0;
  for (int i = 0; i < config.num_outputs; i++) {
    void *data = NULL;
    switch (outputs[i]->type) {
    case HMTL_OUTPUT_VALUE: {
      if (num_triggers >= MAX_TRIGGERS) {
	DEBUG_ERR("Too many triggers configured!");
	DEBUG_ERR_STATE(2);
      }
      triggers[num_triggers] = (config_value_t *)outputs[i];
      num_triggers++;
      break;
    }
    case HMTL_OUTPUT_RGB: {
      rgb_output = (config_rgb_t *)outputs[i];
      break;
    }
    case HMTL_OUTPUT_RS485: {
      data = &rs485;
      rs485_output = (config_rs485_t *)outputs[i];
      break;
    }
    default: {
      DEBUG_ERR("Found unused config type");
      continue;
    }
    }
    
    hmtl_setup_output((output_hdr_t *)outputs[i], data);
  }
}

void loop() {
  serialcli.checkSerial();

  for (int i = 0; i < num_triggers; i++) {
    hmtl_update_output((output_hdr_t *)triggers[i], NULL);
  }

  delay(10);
}


/*
 * t <num> <value> - Set trigger <num> to <value>
 */
void cliHandler(char **tokens, byte numtokens) {
  
  switch (tokens[0][0]) {
  case 't': {
    if (numtokens < 3) return;
    int trig = atoi(tokens[1]);
    int value = atoi(tokens[2]);

    if (trig >= num_triggers) {
      DEBUG_VALUELN(DEBUG_ERROR, "Invalid trigger: ", trig);
      return;
    }

    if (value > (int)((uint8_t)-1)) {
      DEBUG_VALUELN(DEBUG_ERROR, "Invalid trigger value: ", value);
      return;
    }

    triggers[trig]->value = value;

    DEBUG_VALUE(DEBUG_LOW, "Set trigger ", trig);
    DEBUG_VALUELN(DEBUG_LOW, " to ", value);
    break;
  }

  default:{
    break;
  }
  }
}
