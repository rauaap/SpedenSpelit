#ifndef LEDS_H_
#define LEDS_H_

#include <arduino.h>


void setLedEx(uint8_t ledNumber);


/** 
 * @brief Initializes provided pins as outputs
 * 
 * @param p0 pin for led 0
 * @param p1 pin for led 1
 * @param p2 pin for led 2
 * @param p3 pin for led 3
 * @note pins default to A2, A3, A4, and A5
 */
void initializeLeds(void);


/**
 * @brief Set any led
 * 
 * @param ledNumber led index, 0-3
 * @note led number 0 -> arduino pin A2
 * @note led number 1 -> arduino pin A3
 * @note led number 2 -> arduino pin A4
 * @note led number 3 -> arduino pin A5
 */
void setLed(uint8_t ledNumber);


/**
 * @brief Clears all leds
 * 
 */
void clearAllLeds(void);


/**
 * @brief Sets all leds
 * 
 */
void setAllLeds(void);


/**
 * @brief Test led functions
 * 
 * @param timeBetween time between led changes
 */
void testLeds(int timeBetween = 500);


#endif /* LEDS_H_ */
