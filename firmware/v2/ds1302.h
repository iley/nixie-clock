#ifndef _CLOCK_DS1302_H
#define _CLOCK_DS1302_H

#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/types.h"

#define DS1302_CHARGER_DIODE_ONE 1
#define DS1302_CHARGER_DIODE_TWO 2
#define DS1302_CHARGER_RESISTOR_2K 1
#define DS1302_CHARGER_RESISTOR_4K 2
#define DS1302_CHARGER_RESISTOR_8K 3

typedef struct {
  uint ce_pin;
  uint sclk_pin;
  uint io_pin;
} ds1302_t;

void ds1302_init(ds1302_t* device, uint ce, uint sclk, uint io);
void ds1302_set_time(ds1302_t* device, datetime_t* input);
void ds1302_get_time(ds1302_t* device, datetime_t* output);
void ds1302_enable_charger(ds1302_t* device, uint8_t diode_select, uint8_t resistor_select);
void ds1302_disable_charger(ds1302_t* device);

#endif // _CLOCK_DS1302_H
