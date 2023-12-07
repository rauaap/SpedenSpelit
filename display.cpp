#include "display.h"

void initDisplay() {
    DISPLAY_DDREG |= (1 << DATA_PIN) |
                     (1 << LATCH_PIN) |
                     (1 << CLOCK_PIN);
}

void writeByte(uint8_t byte, bool last) {
    for (int i = 0; i < 8; i++) {
        // Clear data and clock pins
        DISPLAY_DPREG &= ~(
            (1 << DATA_PIN) |
            (1 << CLOCK_PIN)
        );

        // Set current bit to data pin
        DISPLAY_DPREG |= (byte & 1) << DATA_PIN;
        // Clock high, bit is shifted to register
        DISPLAY_DPREG |= (1 << CLOCK_PIN);
        // Shift to the next bit
        byte >>= 1;
    }

    if (last) {
        // Latch pin high to set register to output pins
        DISPLAY_DPREG |= (1 << LATCH_PIN);
        // Reset latch pin
        DISPLAY_DPREG &= ~(1 << LATCH_PIN);
    }
}

void displayNumber(uint8_t number) {
    constexpr uint8_t digits[] = {
        0x11, 0xd7, 0x32, 0x92, 0xd4,
        0x98, 0x18, 0xd3, 0x10, 0x90
    };

    static constexpr uint8_t FIXED_POINT_MAGIC_NUMBER = 103;
    static constexpr uint8_t FIXED_POINT_SHIFT_AMOUNT = 10;

    uint8_t tens = number * FIXED_POINT_MAGIC_NUMBER >> FIXED_POINT_SHIFT_AMOUNT;
    uint8_t ones = number - tens * 10;
    writeByte(digits[tens], false);
    writeByte(digits[ones], true);
}
