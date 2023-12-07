#ifndef DISPLAY_H
#define DISPLAY_H

#include <arduino.h>

// Note that these are not arduino pins
// These are the pin numbers in the register
// If using PORTD luckily they correspond to arduino pin numbers
#define DATA_PIN 0
#define LATCH_PIN 1
#define CLOCK_PIN 2

// Direction and port registers for the pins
#define DISPLAY_DDREG DDRB
#define DISPLAY_DPREG PORTB

void initDisplay(void);
void displayNumber(uint8_t number);
void writeByte(uint8_t byte, bool last);

#endif // DISPLAY_H
