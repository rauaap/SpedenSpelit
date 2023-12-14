#include "lcd_logic.h"

#include <stdio.h>
#include <string.h>

#include "eeprom.h"

#include "i2c.h"
#include "lcd.h"


static int __scores[3] = {0, 0, 0};
static char __names[][4] = {DEFAULT_NAME_EMPTY,
                            DEFAULT_NAME_EMPTY,
                            DEFAULT_NAME_EMPTY};

static char __tmp_name[] = DEFAULT_NAME_INPUT;
static int __tmp_name_idx = 0;
static int __tmp_score = 0;


void lcd_saveEntries()
{
    for (int i = 0; i < 3; i++)
    {
        // score
        eeprom_write(i * 4, __scores[i]);
        delay(4);

        // name
        eeprom_write(i * 4 + 1, __names[i][0]);
        delay(4);
        eeprom_write(i * 4 + 2, __names[i][1]);
        delay(4);
        eeprom_write(i * 4 + 3, __names[i][2]);
        delay(4);
    }
}


void lcd_loadEntries()
{
    for (int i = 0; i < 3; i++)
    {
        char name[4];
        name[0] = eeprom_read(i * 4 + 1);
        name[1] = eeprom_read(i * 4 + 2);
        name[2] = eeprom_read(i * 4 + 3);
        name[3] = '\0';
        strcpy(__names[i], name);
        __scores[i] = eeprom_read(i * 4);
    }
}


void lcd_resetEntries()
{
    for (int i = 0; i < 3; i++)
    {
        strcpy(__names[i], DEFAULT_NAME_EMPTY);
        __scores[i] = 0;
    }
    lcd_saveEntries();
}


static char *_score_to_string(int score)
{
    static char tmp[3];
    sprintf(tmp, "%02d", score);
    return tmp;
}


static int _get_placement(int score)
{
    for (int i = 0; i < 3; i++)
    {
        if (score > __scores[i])
        {
            return i;
        }
    }
    return -1;
}


static void _new_entry()
{
    // shift scores to the right
    const int placement = _get_placement(__tmp_score);
    for (int i = 2; i > placement; i--)
    {
        __scores[i] = __scores[i - 1];
        strcpy(__names[i], __names[i - 1]);
    }

    // insert new score
    __scores[placement] = __tmp_score;
    strcpy(__names[placement], __tmp_name);

    lcd_saveEntries();
}


void lcd_init(uint8_t addr)
{
    i2c_init(addr);
    lcd_baseInit();
    lcd_writeCommand(CLEAR);
    lcd_loadEntries();
}


void lcd_newScore(int score)
{
    __tmp_score = score;
}


State lcd_displayGameover()
{
    lcd_writeCommand(CLEAR);
    lcd_setCursor(0, 0);
    lcd_writeString(TXT_gameover);

    lcd_setCursor(0, 1);
    State state =  LCD_ASKNAME;
    const int placement = _get_placement(__tmp_score);
    if (placement == -1) 
    {
        lcd_writeString(TXT_no_placement);
        state = LCD_SCORES;
    }
    else if (placement == 0) lcd_writeString(TXT_1st);
    else if (placement == 1) lcd_writeString(TXT_2nd);
    else if (placement == 2) lcd_writeString(TXT_3rd);

    // reset name input
    strcpy(__tmp_name, DEFAULT_NAME_INPUT);
    __tmp_name_idx = 0;

    return state;
}


void lcd_displayAskname()
{
    lcd_writeCommand(CLEAR);
    lcd_setCursor(0, 0);
    lcd_writeString(TXT_nameinput);

    lcd_setCursor(6, 0);
    lcd_writeString(__tmp_name);

    lcd_setCursor(6 + __tmp_name_idx, 1);
    lcd_writeString("^");
}


State lcd_moveCursor(Direction dir)
{
    State state = NONE;
    switch (dir)
    {
        case Direction::LEFT:
            // if at leftmost position, show score without saving name
            if (--__tmp_name_idx < 0)
            {
                state = LCD_SCORES;
            }
            break;
        
        case Direction::RIGHT:
            // if at rightmost position, show score and save name
            if (++__tmp_name_idx > 2)
            {
                _new_entry();
                state = LCD_SCORES;
            }
            break;
        
        case Direction::DOWN:
            if (__tmp_name[__tmp_name_idx] == 'Z')
                __tmp_name[__tmp_name_idx] = 'A';
            else
                __tmp_name[__tmp_name_idx]++;
            break;
        
        case Direction::UP:
            if (__tmp_name[__tmp_name_idx] == 'A')
                __tmp_name[__tmp_name_idx] = 'Z';
            else
                __tmp_name[__tmp_name_idx]--;
            break;
        
        default:
            break;
    }

    return state;
}


void lcd_displayScoreboard()
{
    lcd_writeCommand(CLEAR);

    // #1
    lcd_setCursor(2, 0);
    lcd_writeString(__names[0]);
    lcd_setCursor(3, 1);
    lcd_writeString(_score_to_string(__scores[0]));

    // #2
    ;lcd_setCursor(7, 0);
    lcd_writeString(__names[1]);
    lcd_setCursor(8, 1);
    lcd_writeString(_score_to_string(__scores[1]));

    // #3
    lcd_setCursor(12, 0);
    lcd_writeString(__names[2]);
    lcd_setCursor(13, 1);
    lcd_writeString(_score_to_string(__scores[2]));
}
