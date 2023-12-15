#include "leds.h"

static constexpr uint8_t __pins[4] = {9, 10, 11, 12};

void initializeLeds(void)
{
    for (int i = 0; i < 4; i++) pinMode(__pins[i], OUTPUT);
}

void setLed(uint8_t ledNumber)
{
    if (ledNumber < 0 || ledNumber > 3) return;

    clearAllLeds();
    digitalWrite(__pins[ledNumber], HIGH);
}


void setLedEx(uint8_t ledNumber)
{
    if (ledNumber < 0 || ledNumber > 3) return;
    digitalWrite(__pins[ledNumber], HIGH);
}


void clearAllLeds()
{
    for (int i = 0; i < 4; i++) digitalWrite(__pins[i], LOW);
}


void setAllLeds()
{
    for (int i = 0; i < 4; i++) digitalWrite(__pins[i], HIGH);
}


void testLeds(int timeBetween)
{
    // blink each led in order
    for (uint8_t i = 0; i < 4; i++)
    {
        setLed(i);
        delay(timeBetween);
    }

    // blink all leds twice
    for (int i = 0; i < 2; i++)
    {
        clearAllLeds();
        delay(timeBetween);
        
        setAllLeds();
        delay(timeBetween);
    }
    
    clearAllLeds();
    delay(timeBetween);
}
