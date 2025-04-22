#include "i2c.h"
#include "com.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <compat/twi.h>
#include <stdint.h>
#include <string.h>
#include <util/twi.h>

// Clear TWCR byte
#define CLR_TWCR ((1 << TWINT) | (1 << TWEN) | (1 << TWEA))
// Set NACK and reset in TWCR
#define NACK_TWCR ((1 << TWINT) | (1 << TWEN) | (0 << TWEA))
// Max I2C buff len
#define DATA_BUFF_LEN 32

// Initialize the I2C communication
void i2c_slave_init(uint8_t addr) {
  PORTD |= (1 << PORTD0) | (1 << PORTD1);

  TWAR = (addr << 1);
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE);

  sei();
}

volatile uint8_t i2c_data_index = 0;      // Current data index
volatile uint8_t i2c_reg_addr = 0xFF;     // Register address
volatile uint8_t i2c_data[DATA_BUFF_LEN]; // Data after the address, don't
// need it because reading only uin8_t after the address

ISR(TWI_vect) {
  switch (TWSR & 0xF8) {

  // Slave Read Acknowledgment
  // Reset the variables
  case TW_SR_SLA_ACK:
    i2c_data_index = 0;
    i2c_reg_addr = 0xFF;
    TWCR = CLR_TWCR;
    break;

  // Slave Read Data
  // Read the incoming data
  case TW_SR_DATA_ACK:
    if (i2c_data_index == 0) {
      i2c_reg_addr = TWDR;
    } else {
      if (i2c_reg_addr < MOT_REG_SIZE) {
        i2c_data[i2c_data_index - 1] = TWDR;
      }
    }
    i2c_data_index++;
    TWCR = CLR_TWCR;
    break;

  // Slave Read Stop
  // Stop the communication and reset
  case TW_SR_STOP:
    if (i2c_reg_addr < MOT_REG_SIZE) {
      memcpy((uint8_t *)&mot_registers[i2c_reg_addr], (uint8_t *)i2c_data,
             i2c_data_index);
    } else {
      MOT_SER = MOT_ERR_I2C_INVALID_ADDR;
      MOT_SR = MOT_STAT_ERROR;
    }
    i2c_data_index = 0;
    i2c_reg_addr = 0xFF;
    memset((void *)i2c_data, 0, sizeof(i2c_data));
    TWCR = CLR_TWCR;
    break;

  // Slave Transmission Acknowledgment
  // Sends the data back
  case TW_ST_SLA_ACK:
    if (i2c_reg_addr < MOT_REG_SIZE) {
      TWDR = mot_registers[i2c_reg_addr];
    } else {
      TWDR = MOT_ERR_NEW;
      MOT_SR = MOT_STAT_ERROR;
      MOT_SER = MOT_ERR_I2C_INVALID_ADDR;
    }
    TWCR = CLR_TWCR;
    break;

  // Slave Transmission Data
  // Send NACK - stopping sending data
  case TW_ST_DATA_ACK:
    i2c_reg_addr++;
    if (i2c_reg_addr < MOT_REG_SIZE) {
      TWDR = mot_registers[i2c_reg_addr];
      TWCR = CLR_TWCR;
    } else {
      TWCR = NACK_TWCR;
      i2c_reg_addr = 0xFF;
    }
    // No more data to send :(
    break;

  // Slave Transmission Last Data
  // Send NACK
  case TW_ST_DATA_NACK:
    TWCR = NACK_TWCR;
    break;

  // Bus Error
  // Just reset the TWCR
  case TW_BUS_ERROR:
    TWCR = CLR_TWCR;
    break;

  // Any other code - reset TWCR
  default:
    TWCR = CLR_TWCR;
    break;
  }
}
