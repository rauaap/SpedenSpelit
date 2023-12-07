#ifndef LEDS_H_
#define LEDS_H_

#include <arduino.h>


/**
 * @brief Subroutine intializes analog pins A2,A3,A4,A5 to be used as outputs.
 *        Speden Spelit leds are connected to those pins. 
 * 
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
