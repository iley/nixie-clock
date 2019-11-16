#ifndef _NIXIE_CLOCK_I2C_H
#define _NIXIE_CLOCK_I2C_H

#include <stdint.h>

void i2c_init();
void i2c_start();
void i2c_write(uint8_t);
uint8_t i2c_read(uint8_t);

#endif // _NIXIE_CLOCK_I2C_H
