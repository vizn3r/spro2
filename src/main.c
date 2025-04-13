#include <avr/io.h>
#include <util/delay.h>

#define SLAVE_ADDR 0x80
#include "../lib/com/com.h"

int main(void) {
  com_init();

  DDRC |= (1 << PC7);

  for (;;) {
    PORTC |= (1 << PORTC7);
    _delay_ms(100);
    PORTC &= ~(1 << PORTC7);
    _delay_ms(100);
  }

  return 0;
}
