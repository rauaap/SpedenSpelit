#ifndef EEPROM_H
#define EEPROM_H 

#include <stdint.h>

uint8_t eeprom_read(uint8_t addr);
void eeprom_write(uint8_t addr, uint8_t data);

#endif // EEPROM_H
