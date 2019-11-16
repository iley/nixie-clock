#include <stdbool.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "rtc.h"

// Maps from digit value to Nixie pin number.
const uint8_t digit_map[] = { 6, 4, 5, 1, 0, 9, 8, 2, 3, 7 };
const uint8_t out_mask_b = (1<<PB0) | (1<<PB1) | (1<<PB6) | (1<<PB7); // TUBE4
const uint8_t out_mask_c = 0x0f; // PC0-PC3 -> TUBE3
const uint8_t out_mask_d = 0xff; // PD0-PD3 -> TUBE1, PD4-PD7 -> TUBE2

const uint16_t timer_start = (65536 - 1000);  // 1 ms at 8MHz using prescaler 8.

uint32_t millis = 0;

void init_timer() {
    TCCR1A = 0x00;
    TIMSK1 = (1 << TOIE1) ; // Timer 1 overflow interrupt.
    TCNT1 = timer_start;
    TCCR1B = (1<<CS11);    // Prescaler 8 => 1MHz
    sei();
}

ISR(TIMER1_OVF_vect) {
    millis++;
    TCNT1 = timer_start;
}

void init_pins() {
    DDRB = out_mask_b;
    DDRC = out_mask_c;
    DDRD = out_mask_d;
    PORTB |= (1<<PB2) | (1<<PB3);  // Enable pull-ups for buttons.
}

void set_tubes(uint8_t tube1, uint8_t tube2, uint8_t tube3, uint8_t tube4) {
    PORTB = (PORTB & ~out_mask_b) | (tube4 & 0x3) << 6 | (tube4 & 0xc) >> 2;
    PORTC = (PORTC & ~out_mask_c) |
        (tube3 & 8) >> 3 | (tube3 & 4) >> 1 | (tube3 & 2) << 1 | (tube3 & 1) << 3;
    PORTD = (PORTD & ~out_mask_d) | (tube2 << 4) | tube1;
}

void set_digits(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    set_tubes(digit_map[digit1], digit_map[digit2], digit_map[digit3], digit_map[digit4]);
}

typedef struct {
    // Whether the button was pressed before last update.
    bool was_pressed;
    // Whether the button is pressed now.
    bool pressed;
    // Milliseconds since boot when last state change happened.
    uint32_t last_change_millis;
} button_t;

void button_init(button_t* btn) {
    btn->was_pressed = false;
    btn->pressed = false;
    btn->last_change_millis = 0;
}

void button_update(button_t* btn, bool input) {
    btn->was_pressed = btn->pressed;
    // Debouncing: if last state change happened within 100ms, do nothing.
    if (millis - btn->last_change_millis < 100) {
        return;
    }
    btn->pressed = input;
    if (btn->was_pressed != btn->pressed) {
        btn->last_change_millis = millis;
    }
}

int main() {
    init_timer();
    init_pins();
    rtc_init();

    bool running = true;

    datetime_t dt;
    rtc_get(&dt);

    button_t btn1;
    button_init(&btn1);

    button_t btn2;
    button_init(&btn2);

    while (true) {
        button_update(&btn1, (PINB & (1<<PB3)) == 0);
        button_update(&btn2, (PINB & (1<<PB2)) == 0);

        if (!btn1.pressed && btn1.was_pressed) {
            running = !running;
        }

        if (running) {
            rtc_get(&dt);
            set_digits(dt.minutes >> 4,
                       dt.minutes & 0xf,
                       dt.seconds >> 4,
                       dt.seconds & 0xf);
        } else {
            // TODO
        }
    }

    while (true) {
    }
}
