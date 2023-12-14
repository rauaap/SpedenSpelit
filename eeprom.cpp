#include <avr/eeprom.h>
#include "eeprom.h"

uint8_t eeprom_read(uint8_t addr) {
    // Wait for possible write operation to finish
    while (EECR & (1 << EEPE));
    EEAR = addr;
    EECR |= (1 << EERE);

    // Return data that was read into the eeprom data register
    return EEDR;
}

void eeprom_write(uint8_t addr, uint8_t data) {
    // Wait for previous write or any SPM operation to finish
    while ((EECR & (1 << EEPE)) | (SPMCSR & (1 << SELFPRGEN)));
    EEAR = addr;
    EEDR = data;
    EECR |= (1 << EEMPE);
    EECR |= (1 << EEPE);
}
