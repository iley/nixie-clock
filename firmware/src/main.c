#include <stdbool.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "rtc.h"

// How many digits the clock has.
#define DIGITS_COUNT 4

// Blinking delay when setting time.
#define BLINK_MS 250

// Start timer at 65536-1000 for 1ms delay at 8MHz with prescaler 8.
#define TIMER_START 64536

// PB0, PB1, PB6, PB7 -> TUBE4
#define OUT_MASK_B ((1<<PB0) | (1<<PB1) | (1<<PB6) | (1<<PB7))

// PC0-PC3 -> TUBE3
#define OUT_MASK_C 0x0f

// PD0-PD3 -> TUBE1, PD4-PD7 -> TUBE2
#define OUT_MASK_D 0xff

// Maps from digit value to Nixie pin number.
const uint8_t digit_map[] = { 6, 4, 5, 1, 0, 9, 8, 2, 3, 7, 0xf };

uint32_t millis = 0;

void init_timer() {
    TCCR1A = 0x00;
    TIMSK1 = (1 << TOIE1) ; // Timer 1 overflow interrupt.
    TCNT1 = TIMER_START;
    TCCR1B = (1<<CS11);    // Prescaler 8 => 1MHz
    sei();
}

ISR(TIMER1_OVF_vect) {
    millis++;
    TCNT1 = TIMER_START;
}

void init_pins() {
    DDRB = OUT_MASK_B;
    DDRC = OUT_MASK_C;
    DDRD = OUT_MASK_D;
    PORTB |= (1<<PB2) | (1<<PB3);  // Enable pull-ups for buttons.
}

void write_tubes(uint8_t tube1, uint8_t tube2, uint8_t tube3, uint8_t tube4) {
    PORTB = (PORTB & ~OUT_MASK_B) | (tube4 & 0x3) << 6 | (tube4 & 0xc) >> 2;
    PORTC = (PORTC & ~OUT_MASK_C) |
        (tube3 & 8) >> 3 | (tube3 & 4) >> 1 | (tube3 & 2) << 1 | (tube3 & 1) << 3;
    PORTD = (PORTD & ~OUT_MASK_D) | (tube2 << 4) | tube1;
}

// Display digits on the tubes. 10 == off.
void write_digits(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    write_tubes(digit_map[digit1], digit_map[digit2], digit_map[digit3], digit_map[digit4]);
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

void get_datetime_digits(datetime_t *dt, uint8_t *digits) {
    digits[0] = dt->hours >> 4;
    digits[1] = dt->hours & 0xf;
    digits[2] = dt->minutes >> 4;
    digits[3] = dt->minutes & 0xf;
}

void set_datetime_digits(datetime_t *dt, uint8_t *digits) {
    dt->hours = (digits[0] << 4) | digits[1];
    dt->minutes = (digits[2] << 4) | digits[3];
    dt->seconds = 0;
}

int main() {
    init_timer();
    init_pins();
    rtc_init();

    bool running = true;

    datetime_t dt;
    rtc_get(&dt); // Read initial time.

    button_t left_button;
    button_init(&left_button);
    button_t right_button;
    button_init(&right_button);

    static uint8_t digits[DIGITS_COUNT];      // Digits to display.

    // State for setting the time.
    uint8_t current_digit;            // What digit we're setting right now.
    uint32_t setting_start_millis;    // When we started setting time, for blinking.

    while (true) {
        // Read the button status.
        button_update(&left_button, (PINB & (1<<PB3)) == 0);
        button_update(&right_button, (PINB & (1<<PB2)) == 0);

        // Whether the buttons were pushed up.
        bool left_button_up = (!left_button.pressed && left_button.was_pressed);
        bool right_button_up = (!right_button.pressed && right_button.was_pressed);

        if (running) {
            rtc_get(&dt);
            get_datetime_digits(&dt, digits);

            if (left_button_up || right_button_up) {
                // If any button is pressed, enter timesetting mode.
                running = false;
                // Start setting time from left-most digit.
                current_digit = 0;
                // Track when this mode was entered, for blinking.
                setting_start_millis = millis;
            }
        } else {
            get_datetime_digits(&dt, digits);

            // Left button switches between digits.
            if (left_button_up) {
                if (current_digit < DIGITS_COUNT-1) {
                    // Advance to next digit.
                    current_digit++;
                } else {
                    // We scrolled through all digits. Finish setting the time.
                    rtc_set(&dt);
                    running = true;
                }
            } else if (right_button_up) {
                // Determine the upper limit for current digit.
                uint8_t limit = 10;
                if (current_digit == 0) {
                    // First digit can be either 0, 1 or 2.
                    limit = 3;
                } else if (current_digit == 1 && digits[0] == 2) {
                    // If first digit is 2, second one can only go up to 4.
                    limit = 5;
                } else if (current_digit == 2) {
                    limit = 6;
                }
                // Cycle through possible digit values.
                digits[current_digit] = (digits[current_digit] + 1) % limit;
            }

            // Save the current digits in datetime.
            set_datetime_digits(&dt, digits);

            // Blink the current digit.
            if (((millis - setting_start_millis) / BLINK_MS) % 2 == 1) {
                digits[current_digit] = 10;
            }
        }

        write_digits(digits[0], digits[1], digits[2], digits[3]);
    }
}
