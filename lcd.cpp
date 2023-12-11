#include "lcd.h"

#include <avr/delay.h>

#include "i2c.h"


void _lcd_write(uint8_t command, uint8_t registerSelect) {
    // Start read/write
    i2c_send((command & 0xF0) | BL | E | registerSelect);
    // Reset start read-write bit
    i2c_send((command & 0xF0) | BL | registerSelect);

    // Do the same for the low bits
    i2c_send((command << 4) | BL | E | registerSelect);
    i2c_send((command << 4) | BL | registerSelect);
}


void lcd_baseInit() {
    // Helper function for sending 8bit commands during initialization
    static auto write8bitCommand = [](uint8_t command) {
        i2c_send(command | E);
        i2c_send(command);
    };

    // Initialization sequence
    // See the HD44780 datasheet page 46 figure 24
    _delay_ms(POWER_ON_DELAY);

    write8bitCommand(INIT_BYTE);
    _delay_ms(INIT_DELAY1);

    write8bitCommand(INIT_BYTE);
    _delay_us(INIT_DELAY2);

    write8bitCommand(INIT_BYTE);
    _delay_us(COMMAND_DELAY);

    write8bitCommand(FUNCTION_SET);
    _delay_us(COMMAND_DELAY);

    // 4bit mode set, now we can start sending 4bit commands
    // Two row 5x8 character display
    lcd_writeCommand(FUNCTION_SET | N);
    // Display off, cursor off and no blink
    lcd_writeCommand(DISPLAY_CONTROL);
    lcd_writeCommand(CLEAR);
    // Entry mode to shift cursor to the right, do not shift display
    lcd_writeCommand(ENTRY_MODE | ID);
    // Display on
    lcd_writeCommand(DISPLAY_CONTROL | D);
}


void lcd_writeCharacter(uint8_t character) {
    _lcd_write(character, RS);
}


void lcd_writeCommand(uint8_t command) {
    _lcd_write(command, 0);

    // These two commands require much longer to execute than others
    if (command == CLEAR || command == HOME)
        _delay_ms(HOME_CLEAR_DELAY);
    else
        _delay_us(COMMAND_DELAY);
}


void lcd_writeString(char string[]) {
    while (*string)
        lcd_writeCharacter(*(string++));
}


void lcd_setCursor(uint8_t x, uint8_t y) {
    // There's only two rows
    // Roll over even numbers to the first row and odds to the second
    uint8_t row = y & 1;
    // Limit columns
    uint8_t col = x & 0x3f;
    // Second row addresses start from 0x40, so if y is 1 we add 0x40 to 
    // the columns which gives the us the address that corresponds 
    // to the xy position
    uint8_t address = x + (row * 0x40);
    lcd_writeCommand(SET_DDRAM_ADDR | address);
}


void lcd_writeLines(char lineOne[], char lineTwo[], bool clear = true) {
    if (clear)
        lcd_writeCommand(CLEAR);

    lcd_setCursor(0, 0);
    lcd_writeString(lineOne);
    lcd_setCursor(0, 1);
    lcd_writeString(lineTwo);
}
