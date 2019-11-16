#include "i2c.h"

#include <avr/io.h>
#include <util/delay.h>

void i2c_init() {
    TWSR = 0x00; // Set prescaler bits to zero
    TWBR = 0x46; // SCL frequency is 50K for 16Mhz
    TWCR = 0x04; // Enable TWI module.
}

void i2c_start() {
    TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
    while (!(TWCR & (1 << TWINT)));
}

void i2c_stop() {
    TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTO));
    _delay_us(100);
}

void i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = ((1 << TWINT) | (1 << TWEN));
    while (!(TWCR & (1 << TWINT)));
}

uint8_t i2c_read(uint8_t ack)
{
    TWCR = ((1 << TWINT) | (1 << TWEN) | (ack << TWEA));
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}
