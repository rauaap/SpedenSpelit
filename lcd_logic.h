#ifndef LCD_LOGIC_H_
#define LCD_LOGIC_H_

#include "SpedenSpelit.h"

#define DEFAULT_NAME_INPUT "AAA"
#define DEFAULT_NAME_EMPTY "---"

#define TXT_gameover        "  - Game over -"
#define TXT_1st             " New high score!"
#define TXT_2nd             "   2nd place!"
#define TXT_3rd             "   3rd place!"
#define TXT_no_placement    "   Try again!"
#define TXT_nameinput       "Name:"



enum class Direction
{
    LEFT = 0,
    RIGHT,
    UP,
    DOWN
};


void lcd_init(uint8_t addr);
void lcd_newScore(int score);

void lcd_saveEntries();
void lcd_loadEntries();
void lcd_resetEntries();

// game over
State lcd_displayGameover();

// ask name
void lcd_displayAskname();
State lcd_moveCursor(Direction dir);

// show score
void lcd_displayScoreboard();


#endif /* LCD_LOGIC_H_ */
