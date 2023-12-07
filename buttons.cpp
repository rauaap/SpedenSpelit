#include "Buttons.h"
#include <Arduino.h>

void initButtonsAndButtonInterrupts(){
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);

    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);

    PCICR |= (1 << PCIE2);
    PCMSK2 |= ((1 << PCINT19) | (1 << PCINT20) | (1 << PCINT21) | (1 << PCINT22));
}


