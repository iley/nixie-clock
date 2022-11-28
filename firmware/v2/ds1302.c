#include "ds1302.h"

#define EPOCH_START_YEAR 2000

#define CMD_CLOCK_BURST_READ  0xbf
#define CMD_CLOCK_BURST_WRITE 0xbe

#define REG_SECONDS         0x0
#define REG_TRICKLE_CHARGER 0x8

static void spi_begin(ds1302_t* device);
static void spi_end(ds1302_t* device);
static void spi_write(ds1302_t* device, uint8_t data);
static void spi_write_before_read(ds1302_t* device, uint8_t data);
static uint8_t spi_read(ds1302_t* device);
static void ds1302_write_register(ds1302_t* device, uint8_t register_, uint8_t data);
static uint8_t ds1302_read_register(ds1302_t* device, uint8_t register_);
static uint8_t from_bcd(uint8_t value);
static uint8_t to_bcd(uint8_t value);
static uint8_t decode_hours(uint8_t value);

static inline void toggle_pin(uint pin) {
  gpio_put(pin, 1);
  sleep_us(1);
  gpio_put(pin, 0);
  sleep_us(1);
}

void ds1302_init(ds1302_t* device, uint ce_pin, uint sclk_pin, uint io_pin) {
  device->ce_pin = ce_pin;
  device->sclk_pin = sclk_pin;
  device->io_pin = io_pin;

  gpio_init(ce_pin);
  gpio_set_dir(ce_pin, GPIO_OUT);

  gpio_init(sclk_pin);
  gpio_set_dir(sclk_pin, GPIO_OUT);

  gpio_init(io_pin);
  gpio_set_dir(io_pin, GPIO_OUT);
}

void ds1302_set_time(ds1302_t* device, datetime_t* input) {
  uint8_t clock_halt_flag = ds1302_read_register(device, REG_SECONDS) & 0x80;

  spi_begin(device);

  spi_write(device, CMD_CLOCK_BURST_WRITE);
  spi_write(device, clock_halt_flag | to_bcd(input->sec));
  spi_write(device, to_bcd(input->min));
  spi_write(device, to_bcd(input->hour));
  spi_write(device, to_bcd(input->day));
  spi_write(device, to_bcd(input->month));
  spi_write(device, to_bcd(input->dotw));
  spi_write(device, to_bcd(input->year - EPOCH_START_YEAR));
  spi_write(device, 0); // write protection register

  spi_end(device);
}

void ds1302_get_time(ds1302_t* device, datetime_t* output) {
  spi_begin(device);

  spi_write_before_read(device, CMD_CLOCK_BURST_READ);
  output->sec = from_bcd(spi_read(device));
  output->min = from_bcd(spi_read(device));
  output->hour = decode_hours(spi_read(device));
  output->day = from_bcd(spi_read(device));
  output->month = from_bcd(spi_read(device));
  output->dotw = from_bcd(spi_read(device));
  output->year = EPOCH_START_YEAR + from_bcd(spi_read(device));

  spi_end(device);
}

void ds1302_enable_charger(ds1302_t* device, uint8_t diode_select, uint8_t resistor_select) {
  uint8_t value = 0xa0 | ((diode_select & 0x3) << 2) | (resistor_select & 0x3);
  ds1302_write_register(device, REG_TRICKLE_CHARGER, value);
}

void ds1302_disable_charger(ds1302_t* device) {
  ds1302_write_register(device, REG_TRICKLE_CHARGER, 0);
}

static void ds1302_write_register(ds1302_t* device, uint8_t register_, uint8_t data) {
  spi_begin(device);
  uint8_t command = (0x80 | (register_ << 1));
  spi_write(device, command);
  spi_write(device, data);
  spi_end(device);
}

static uint8_t ds1302_read_register(ds1302_t* device, uint8_t register_) {
  spi_begin(device);
  uint8_t command = (0x81 | (register_ << 1));
  spi_write(device, command);
  uint8_t data = spi_read(device);
  spi_end(device);
  return data;
}

static void spi_begin(ds1302_t* device) {
  gpio_put(device->sclk_pin, 0);
  gpio_put(device->ce_pin, 1);
  sleep_us(4);
}

static void spi_end(ds1302_t* device) {
  gpio_put(device->ce_pin, 0);
  sleep_us(4);
}

static void spi_write(ds1302_t* device, uint8_t data) {
  gpio_set_dir(device->io_pin, GPIO_OUT);

  for (int i = 0; i < 8; i++) {
    gpio_put(device->io_pin, (data >> i) & 1);
    sleep_us(1);
    toggle_pin(device->sclk_pin);
  }
}

static void spi_write_before_read(ds1302_t* device, uint8_t data) {
  gpio_set_dir(device->io_pin, GPIO_OUT);

  for (int i = 0; i < 7; i++) {
    gpio_put(device->io_pin, (data >> i) & 1);
    sleep_us(1);
    toggle_pin(device->sclk_pin);
  }

  gpio_put(device->io_pin, (data >> 7) & 1);
  sleep_us(1);
  gpio_put(device->sclk_pin, 1);
  sleep_us(1);
  // Leave SCLK high on the last cycle.
}

static uint8_t spi_read(ds1302_t* device) {
  gpio_set_dir(device->io_pin, GPIO_IN);

  uint8_t data;
  for (int i = 0; i < 8; i++) {
    toggle_pin(device->sclk_pin);

    uint8_t bit = gpio_get(device->io_pin) ? 1 : 0;
    data |= (bit << i);
  }

  return data;
}

static uint8_t from_bcd(uint8_t value) {
  return (10 * ((value & 0xf0) >> 4) + (value & 0x0f));
}

static uint8_t to_bcd(uint8_t value) {
  return ((value / 10) << 4) | (value % 10);
}

static uint8_t decode_hours(uint8_t value) {
  uint8_t offset;
  if (value & 0x80) {
    // 12-hour mode.
    return (value & 0xf) + (value & 0x20 ? 12 : 0);
  } else {
    // 24-hour mode.
    return (value & 0xf)  + 10 * ((value & 0x30) >> 4);
  }
}
