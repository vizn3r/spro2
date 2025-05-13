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
#define CLR_TWCR ((1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE))
// Set NACK and reset in TWCR
#define NACK_TWCR ((1 << TWINT) | (1 << TWEN) | (1 << TWIE))
// Max I2C buff len
#define DATA_BUFF_LEN 32

// Initialize the I2C communication
void i2c_slave_init(uint8_t addr) {
  TWAR = (addr << 1);

  // TWSR &= !((1 << TWPS0) | (1 << TWPS1));
  // TWBR = 12; // 400kHz

  TWCR = CLR_TWCR;

  PORTD |= (1 << PORTD0) | (1 << PORTD1);
  DDRD &= ~((1 << PORTD0) | (1 << PORTD1));
}

volatile uint8_t i2c_data_index = 0;      // Current data index
volatile uint8_t i2c_reg_addr = 0xFF;     // Register address
volatile uint8_t i2c_data[DATA_BUFF_LEN]; // Data

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
      if (i2c_reg_addr < MOT_REG_SIZE && (i2c_data_index - 1) < DATA_BUFF_LEN) {
        i2c_data[i2c_data_index - 1] = TWDR;
      } else {
        // handle error
      }
    }
    i2c_data_index++;
    TWCR = CLR_TWCR;
    break;

  // Slave Read Stop
  // Stop the communication and reset
  case TW_SR_STOP:
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      if (i2c_reg_addr < MOT_REG_SIZE) {
        uint8_t data_len = i2c_data_index - 1;
        if (i2c_reg_addr + data_len <= MOT_REG_SIZE) {
          memcpy((uint8_t *)&mot_registers[i2c_reg_addr], (uint8_t *)i2c_data,
                 data_len);
          MOT_SER = MOT_ERR_NONE;
        } else {
          MOT_SER = MOT_ERR_I2C_INVALID_ADDR;
          MOT_SR = MOT_STAT_ERROR;
        }
      } else {
        MOT_SER = MOT_ERR_I2C_INVALID_ADDR;
        MOT_SR = MOT_STAT_ERROR;
      }
    }

    i2c_data_index = 0;
    i2c_reg_addr = 0xFF;
    for (volatile uint8_t i = 0; i < DATA_BUFF_LEN; i++)
      i2c_data[i] = 0;
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
    MOT_SER = MOT_ERR_I2C;
    MOT_SR = MOT_STAT_ERROR;
    TWCR = CLR_TWCR;
    break;

  // Any other code - reset TWCR
  default:
    TWCR = CLR_TWCR;
    break;
  }
}
