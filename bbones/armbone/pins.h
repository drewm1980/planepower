#ifndef PINS_H
#define PINS_H

// Both implementations will need these:
#define AZIMUTH_STATUS_PIN 115
#define ELEVATION_STATUS_PIN 116

// These will only be used if doing bitbanging
#define CS0_PIN 113
#define CS1_PIN 114
#define MISO_PIN  111
#define CLK_PIN 110

#define NUM(x) #x

#define PREFIX "sys/class/gpio/gpio"

#define CS0_VALUE_FILE PREFIX NUM(CS0_PIN) "/value"
#define CS1_VALUE_FILE PREFIX NUM(CS1_PIN) "/value"
#define MISO_VALUE_FILE PREFIX NUM(MISO_PIN) "/value"
#define CLK_VALUE_FILE PREFIX NUM(CLK_PIN) "/value"

#endif
