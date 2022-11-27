#ifndef _CLOCK_DS1302_H
#define _CLOCK_DS1302_H

#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/types.h"

typedef struct {
  uint ce_pin;
  uint sclk_pin;
  uint io_pin;
} ds1302_t;

void ds1302_init(ds1302_t* device, uint ce, uint sclk, uint io);
void ds1302_set_time(ds1302_t* device, datetime_t* input);
void ds1302_get_time(ds1302_t* device, datetime_t* output);

#endif // _CLOCK_DS1302_H
