#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/types.h"
#include "ds1302.h"

#define PIN_CS   18
#define PIN_SCLK 16
#define PIN_IO   17

#define PIN_BUTTON 11

int main() {
  stdio_init_all();

  ds1302_t ds1302;
  ds1302_init(&ds1302, PIN_CS, PIN_SCLK, PIN_IO);
  ds1302_enable_charger(&ds1302, DS1302_CHARGER_DIODE_ONE, DS1302_CHARGER_RESISTOR_2K);

  gpio_init(PIN_BUTTON);
  gpio_set_dir(PIN_BUTTON, GPIO_IN);
  gpio_pull_up(PIN_BUTTON);

  datetime_t dt;
  datetime_t prev_dt;

  while (true) {
    /*
    if (!gpio_get(PIN_BUTTON)) {
      // Reset clock.
      datetime_t initial_time = {
        .year  = 2022,
        .month = 1,
        .day   = 1,
        .dotw  = 7,
        .hour  = 0,
        .min   = 0,
        .sec   = 0,
      };

      ds1302_set_time(&ds1302, &initial_time);
    }

    ds1302_get_time(&ds1302, &dt);

    if (dt.sec != prev_dt.sec) {
      printf("%d-%d-%d %d:%d:%d\n", dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec);
    }
    prev_dt = dt;
    */

    ds1302_disable_charger(&ds1302);
  }

  return 0;
}
