#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>

#define SLAVE_ADDR 0x08
#include "../lib/com/com.h"
#include "../lib/mot/control.h"
#include "../lib/mot/driver.h"
#include "../lib/mot/encoder.h"

// Delay for float valules (ignores floating point falue)
void delay_ms(float ms);

// Blink the LED when error is therekk
void blink_err(mot_error_t err);

// Check control register

int main(void) {
  com_init();

  // LED pin
  DDRC |= (1 << PC7);
  PORTC &= ~(1 << PC7);

  for (;;) {
    blink_err(MOT_SER);
    get_curr_ang();
    check_cr();
    mot_drive();
  }

  return 0;
}

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
    _delay_ms(100);
    PORTC ^= (1 << PORTC7);
    _delay_ms(100);
  }
  _delay_ms(1000);
}
