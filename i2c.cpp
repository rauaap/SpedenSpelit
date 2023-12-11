#include "i2c.h"

#include <compat/twi.h>
#include <avr/interrupt.h>


static uint8_t _slaveAddr = 0;
static volatile uint8_t _data = 0;
static volatile uint8_t _dataSent = 1;


void i2c_init(uint8_t slaveAddr) {
    _slaveAddr = slaveAddr;
    // Both pre-scaler bits to 0 for pre-scaler value 1
    TWSR &= ~( _BV(TWPS0) | _BV(TWPS1) );
    // SCL frequency formula on page 180 in the ATmega328p datasheet
    TWBR = (F_CPU / TWI_FREQ - 16) / 2;
    sei();

    // I2C pins as outputs with pull-up resistors enabled
    DDRC &= ~(_BV(DDC4) | _BV(DDC5));
    PORTC |= _BV(PORTC4) | _BV(PORTC5);
}


void i2c_send(uint8_t data) {
    // Previous transaction in process
    // Wait for it to finish
    while ( !_dataSent || (TWCR & _BV(TWSTO)) );

    _data = data;
    _dataSent = 0;
    // Clear interrupt flag, enable TWI, enable interrupt, generate start condition
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
}


ISR(TWI_vect) {
    switch(TW_STATUS) {
    case TW_START:
        // Start condition, send slave address and write request
        TWDR = _slaveAddr << 1;
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        break;
    case TW_MT_SLA_ACK:
        // Slave ACK, send data
        TWDR = _data;
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        break;
   case TW_MT_DATA_ACK:
        // Data ACK, send stop 
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
        _dataSent = 1;
        break;
    default:
        break;
    }
}
