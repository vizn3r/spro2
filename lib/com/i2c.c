#include "i2c.h"
#include "com.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <compat/twi.h>
#include <stdint.h>
#include <string.h>
#include <util/atomic.h>
#include <util/twi.h>

// Clear TWCR byte
#define ACK ((1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE))
// Set NACK and reset in TWCR
#define NACK ((1 << TWINT) | (1 << TWEN) | (1 << TWIE))
// Max I2C buff len
#define DATA_BUFF_LEN 32

// Initialize the I2C communication
void i2c_slave_init(uint8_t addr) {
  TWAR = (addr << 1);

  // TWSR &= !((1 << TWPS0) | (1 << TWPS1));
  // TWBR = 12; // 400kHz

  TWCR = ACK;

  PORTD |= (1 << PORTD0) | (1 << PORTD1);
  DDRD &= ~((1 << PORTD0) | (1 << PORTD1));
}

volatile uint8_t i2c_index = 0;           // Current data index
volatile uint8_t i2c_reg_addr = 0x00;     // Register address
volatile uint8_t i2c_data[DATA_BUFF_LEN]; // Data

ISR(TWI_vect) {
  switch (TW_STATUS) {
  /* TW SR*/
  case TW_SR_SLA_ACK:
    TWCR = ACK;
    break;

  case TW_SR_DATA_ACK:
    if (i2c_index == 0)
      i2c_reg_addr = TWDR;
    else if (i2c_reg_addr < MOT_REG_SIZE) {
      mot_registers[i2c_reg_addr + i2c_index - 1] = TWDR;
    }
    i2c_index++;
    TWCR = ACK;
    break;

  /* TW ST */
  case TW_ST_SLA_ACK:
    i2c_index = 0;
    if ((i2c_reg_addr + i2c_index) < MOT_REG_SIZE)
      TWDR = mot_registers[i2c_reg_addr + i2c_index];
    else
      TWDR = 0x55;
    TWCR = ACK;
    break;

  case TW_ST_DATA_ACK:
    i2c_index++;
    if ((i2c_reg_addr + i2c_index) < MOT_REG_SIZE)
      TWDR = mot_registers[i2c_reg_addr + i2c_index];
    else
      TWDR = 0x55;
    TWCR = ACK;
    break;

  case TW_ST_DATA_NACK:
  case TW_ST_LAST_DATA:
  case TW_SR_STOP:
    i2c_index = 0;
    TWCR = ACK;
    break;

  default:
    TWCR = ACK;
    break;
  }
}
