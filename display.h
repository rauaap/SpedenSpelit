#ifndef DISPLAY_H
#define DISPLAY_H

#include <arduino.h>

// Note that these are not arduino pins
// These are the pin numbers in the register
// If using PORTD luckily they correspond to arduino pin numbers
#define DATA_PIN 1
#define LATCH_PIN 2
#define CLOCK_PIN 3

// Direction and port registers for the pins
#define DISPLAY_DDREG DDRC
#define DISPLAY_DPREG PORTC

void initDisplay(void);
void displayNumber(uint8_t number);
void writeByte(uint8_t byte, bool last);

#endif // DISPLAY_H
