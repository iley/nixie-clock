#include "i2c.h"
#include "rtc.h"

#include <avr/io.h>

const uint8_t DS1307_READ_MODE   = 0xd1;
const uint8_t DS1307_WRITE_MODE  = 0xd0;

const uint8_t DS1307_SEC_REG     = 0x00;
const uint8_t DS1307_DATE_REG    = 0x04;
const uint8_t DS1307_CONTROL_REG = 0x07;

void rtc_init() {
    i2c_init();
    i2c_start();

    i2c_write(DS1307_WRITE_MODE);
    i2c_write(DS1307_CONTROL_REG);

    i2c_write(0x00); // Disable SQW output.

    i2c_stop();
}

void rtc_set(datetime_t *dt) {
    i2c_start();

    i2c_write(DS1307_WRITE_MODE);
    i2c_write(DS1307_SEC_REG);

    i2c_write(dt->seconds);
    i2c_write(dt->minutes);
    i2c_write(dt->hours);
    i2c_write(dt->weekDay);
    i2c_write(dt->day);
    i2c_write(dt->month);
    i2c_write(dt->year);

    i2c_stop();
}

void rtc_get(datetime_t* dt) {
    i2c_start();

    i2c_write(DS1307_WRITE_MODE);
    i2c_write(DS1307_SEC_REG);

    i2c_stop();

    i2c_start();
    i2c_write(DS1307_READ_MODE);

    dt->seconds = i2c_read(1);
    dt->minutes = i2c_read(1);
    dt->hours = i2c_read(1);
    dt->weekDay = i2c_read(1);
    dt->day = i2c_read(1);
    dt->month = i2c_read(1);
    dt->year = i2c_read(0);

    i2c_stop();
}
