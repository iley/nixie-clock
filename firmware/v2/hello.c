#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "blink.pio.h"

void blink_pin(PIO pio, uint state_machine, uint offset, uint pin, uint freq) {
  blink_program_init(pio, state_machine, offset, pin);
  pio_sm_set_enabled(pio, state_machine, true);
  pio->txf[state_machine] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}

int main() {
  PIO pio = pio0;

  uint offset = pio_add_program(pio, &blink_program);
  blink_pin(pio, 0, offset, PICO_DEFAULT_LED_PIN, 10);

  return 0;
}
