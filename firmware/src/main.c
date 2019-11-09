#include <stdbool.h>
#include <stdint.h>

#include <avr/io.h>
#include <util/delay.h>

// Maps from digit value to Nixie pin number.
uint8_t digit_map[] = { 6, 4, 5, 1, 0, 9, 8, 2, 3, 7 };

void set_tubes(uint8_t tube1, uint8_t tube2, uint8_t tube3, uint8_t tube4) {
    PORTD = (tube2 << 4) | tube1;
    PORTC = (tube3 & 8) >> 3 | (tube3 & 4) >> 1 | (tube3 & 2) << 1 | (tube3 & 1) << 3;
    PORTB = (tube4 & 0x3) << 6 | (tube4 & 0xc) >> 2;
}

void set_digits(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    set_tubes(digit_map[digit1], digit_map[digit2], digit_map[digit3], digit_map[digit4]);
}

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

const uint8_t DS1307_READ_MODE   = 0xd1;
const uint8_t DS1307_WRITE_MODE  = 0xd0;

const uint8_t DS1307_SEC_REG     = 0x00;
const uint8_t DS1307_DATE_REG    = 0x04;
const uint8_t DS1307_CONTROL_REG = 0x07;

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekDay;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} datetime_t;

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

void demo() {
    uint8_t digit = 0;
    while (true) {
        set_digits(digit, digit, digit, digit);
        digit = (digit + 1) % 10;
        _delay_ms(500);
    }
}

static datetime_t initial_time = {
    .seconds = 0x34,
    .minutes = 0x12,
    .hours = 0,
    .weekDay = 0,
    .day = 0,
    .month = 0,
    .year = 0
};

int main() {
    DDRD = 0xff; // PD0-PD3 -> TUBE1, PD4-PD7 -> TUBE2
    DDRC = 0x0f; // PC0-PC3 -> TUBE3
    DDRB = (1<<PB0) | (1<<PB1) | (1<<PB6) | (1<<PB7); // TUBE4

    rtc_init();

    rtc_set(&initial_time);

    static datetime_t dt;

    while (true) {
        rtc_get(&dt);
        set_digits(dt.minutes >> 4,
                   dt.minutes & 0xf,
                   dt.seconds >> 4,
                   dt.seconds & 0xf);
        _delay_ms(100);
    }
}
