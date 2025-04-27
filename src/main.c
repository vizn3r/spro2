#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>

#define SLAVE_ADDR 0x08
#include "../lib/com/com.h"

void delay_ms(float ms) {
  while (ms >= 1.0) {
    _delay_ms(1.0); // this is fine because it's a constant
    ms -= 1.0;
  }
}

void blink_err(mot_error_t err) {
  if (err == MOT_ERR_NONE)
    return;
  for (mot_error_t i = 0; i <= err; i++) {
    PORTC ^= (1 << PORTC7);
    delay_ms(100);
  }
  delay_ms(500);
}

int main(void) {
  com_init();

  DDRC |= (1 << PC7);

  MOT_ANG = 500.0;
  // MOT_SER = MOT_ERR_UNDEFINED;

  for (;;) {
    float curr_angle = 0.0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { curr_angle = MOT_ANG; }
    PORTC ^= (1 << PORTC7);
    delay_ms(curr_angle);

    // blink_err(MOT_SER);
  }

  return 0;
}
