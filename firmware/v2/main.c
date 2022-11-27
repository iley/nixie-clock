#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/types.h"
#include "ds1302.h"

#define PIN_CS   18
#define PIN_SCLK 16
#define PIN_IO   17

int main() {
  stdio_init_all();

  ds1302_t ds1302;
  ds1302_init(&ds1302, PIN_CS, PIN_SCLK, PIN_IO);

  datetime_t initial_time = {
    .year  = 2022,
    .month = 11,
    .day   = 27,
    .dotw  = 6,
    .hour  = 15,
    .min   = 32,
    .sec   = 0,
  };

  ds1302_set_time(&ds1302, &initial_time);

  datetime_t dt;
  while (true) {
    ds1302_get_time(&ds1302, &dt);
    sleep_ms(1000);
    printf("%d-%d-%d %d:%d:%d\n", dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec);
  }

  return 0;
}
