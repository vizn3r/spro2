#include "i2c.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <compat/twi.h>
#include <util/twi.h>

#define CLR_TWCR (1 << TWINT) | (1 << TWEN) | (1 << TWEA)

void i2c_slave_init(uint8_t address) {
  PORTD |= (1 << PORTD0) | (1 << PORTD1);

  TWAR = (address << 1) | (0 << PORTD1);
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE);

  sei();
}

ISR(TWI_vect) {
  switch (TWSR & 0xF8) {
  case TW_SR_SLA_ACK:
    TWCR = CLR_TWCR;
    break;

  case TW_SR_DATA_ACK:
    // Read the data from TWDR (TWI Data Register)
    // uint8_t data = TWDR
    TWCR = CLR_TWCR;
    break;

  case TW_ST_SLA_ACK:
    // When your master requests data:
    // Load the data you want to transmit into TWDR.
    TWDR = 0x55; // Example data
    TWCR = CLR_TWCR;
    break;

  case TW_ST_DATA_ACK:
    // If you have more data, load the next byte into TWDR.
    // If no more data, you might choose to send a NACK or reset.
    // In this simple example we just re-enable with ACK:
    TWCR = CLR_TWCR;
    break;

  case TW_BUS_ERROR:
    TWCR = CLR_TWCR;
    break;

  default:
    TWCR = CLR_TWCR;
    break;
  }
}
