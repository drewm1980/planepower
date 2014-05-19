#ifndef PINS_H
#define PINS_H

// Both implementations will need these:
#define AZIMUTH_STATUS_PIN 115
#define ELEVATION_STATUS_PIN 116

// These will only be used if doing bitbanging
#define CS0_PIN 113
#define CS0_PIN_STRING "113"
#define CS1_PIN 114
#define CS1_PIN_STRING "114"
#define MISO_PIN  111
#define MISO_PIN_STRING  "111"
#define CLK_PIN 110
#define CLK_PIN_STRING "110"

#define PREFIX "/sys/class/gpio/gpio"

#define CS0_VALUE_FILE PREFIX CS0_PIN_STRING "/value"
#define CS1_VALUE_FILE PREFIX CS1_PIN_STRING "/value"
#define MISO_VALUE_FILE PREFIX MISO_PIN_STRING "/value"
#define CLK_VALUE_FILE PREFIX CLK_PIN_STRING "/value"

#endif
