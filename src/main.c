#define F_CPU 16000000UL

#include <avr/interrupt.h>
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
  encoder_init();

  sei();

  // LED pin
  DDRC |= (1 << PC7);
  PORTC &= ~(1 << PC7);

  // PWM Setup - for D9(PB5), D10(PB6)
  DDRB |= (1 << PB5) | (1 << PB6);
  // Sets OCR1A and OCR1B to compare match
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
  // Sets WGM12 which in addition to the WGM10 sets mode to 5 which is 8Bit Fast
  // PWM Sets the prescaler 1
  TCCR1B = (1 << WGM12) | (1 << CS10);
  OCR1A = 0;
  OCR1B = 0;

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
    _delay_ms(1.0);
    ms -= 1.0;
  }
}

void blink_err(mot_error_t err) {
  if (err == MOT_ERR_NONE && !MOT_CR_READ(MOT_CR_LED))
    return;
  for (mot_error_t i = 0; i <= err; i++) {
    PORTC ^= (1 << PORTC7);
    _delay_ms(500);
    PORTC ^= (1 << PORTC7);
    _delay_ms(500);
  }
  _delay_ms(2500);
}
