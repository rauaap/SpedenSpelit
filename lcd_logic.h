#ifndef LCD_LOGIC_H_
#define LCD_LOGIC_H_

#define DEFAULT_NAME_INPUT "AAA"
#define DEFAULT_NAME_EMPTY "---"

#define TXT_gameover        "  - Game over -"
#define TXT_1st             " New high score!"
#define TXT_2nd             "   2nd place!"
#define TXT_3rd             "   3rd place!"
#define TXT_no_placement    "   Try again!"
#define TXT_nameinput       "Name:"

#include <Arduino.h>


enum class Direction
{
    LEFT = 0,
    RIGHT,
    UP,
    DOWN
};


void lcd_init(uint8_t addr);
Mode lcd_getMode();
void lcd_newScore(int score);

// game over
void lcd_displayGameover();

// ask name
void lcd_displayAskname();
void lcd_moveCursor(Direction dir);

// show score
void lcd_displayScoreboard();


#endif /* LCD_LOGIC_H_ */