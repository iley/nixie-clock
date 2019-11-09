#include <stdbool.h>
#include <stdint.h>

#include <avr/io.h>
#include <util/delay.h>

// Maps from digit value to Nixie pin number.
uint8_t digit_map[] = { 6, 4, 5, 1, 0, 9, 8, 2, 3, 7 };

void set_digits(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    set_tubes(digit_map[digit1], digit_map[digit2], digit_map[digit3], digit_map[digit4]);
}

void set_tubes(uint8_t tube1, uint8_t tube2, uint8_t tube3, uint8_t tube4) {
    PORTD = (tube2 << 4) | tube1;
    PORTC = (tube3 & 8) >> 3 | (tube3 & 4) >> 1 | (tube3 & 2) << 1 | (tube3 & 1) << 3;
    PORTB = (tube4 & 0x3) << 6 | (tube4 & 0xc) >> 2;
}

int main() {
    DDRD = 0xff; // PD0-PD3 -> TUBE1, PD4-PD7 -> TUBE2
    DDRC = 0x0f; // PC0-PC3 -> TUBE3
    DDRB = (1<<PB0) | (1<<PB1) | (1<<PB6) | (1<<PB7); // TUBE4

    uint8_t digit = 0;
    while (true) {
        set_digits(digit, digit, digit, digit);
        digit = (digit + 1) % 10;
        _delay_ms(500);
    }
}
