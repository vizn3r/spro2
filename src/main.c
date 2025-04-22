#include <avr/io.h>
#include <util/delay.h>

#define SLAVE_ADDR 0x80
#include "../lib/com/com.h"

int main(void) {
  com_init();

  DDRC |= (1 << PC7);

  MOT_ANG = 100;

  for (;;) {
    PORTC |= (1 << PORTC7);
    _delay_ms(MOT_ANG);
    PORTC &= ~(1 << PORTC7);
    _delay_ms(MOT_ANG);
  }

  return 0;
}
