#include "com.h"
#include "i2c.h"
#include <avr/eeprom.h>
#include <stdint.h>

void com_init() {
  // Init registers

  // eeprom_or_def(MOT_ADDR_ANG, FLOAT_TO_UINT8(MOT_ANG));
  // eeprom_or_def(MOT_ADDR_CANG, FLOAT_TO_UINT8(MOT_CANG));
  // eeprom_or_def(MOT_ADDR_SR, MOT_TO_UINT8(MOT_SR));
  // eeprom_or_def(MOT_ADDR_SER, MOT_TO_UINT8(MOT_SER));
  // eeprom_or_def(MOT_ADDR_CR, MOT_TO_UINT8(MOT_CR));

  // Init i2c
  i2c_slave_init(SLAVE_ADDR);
}

void eeprom_or_def(mot_eeprom_addr_t addr, uint8_t *buff) {
  switch (addr) {
  case MOT_ADDR_ANG:
    for (uint8_t i = 0; i < 4; i++)
      buff[i] = eeprom_read_byte((uint8_t *)(addr + i));
    break;
  case MOT_ADDR_CANG:
    for (uint8_t i = 0; i < 4; i++)
      buff[i] = eeprom_read_byte((uint8_t *)(addr + i));
    break;
  case MOT_ADDR_HANG:
    // No register for Home Angle, just EEPROM data
    break;

  case MOT_ADDR_SR:
    for (uint8_t i = 0; i < 1; i++)
      buff[i] = eeprom_read_byte((uint8_t *)(addr + i));
    if (buff[0] == 0) {
      buff[0] = MOT_STAT_OFF;
    }
    break;
  case MOT_ADDR_SER:
    for (uint8_t i = 0; i < 1; i++)
      buff[i] = eeprom_read_byte((uint8_t *)(addr + i));
    if (buff[0] == 0) {
      buff[0] = MOT_ERR_NONE;
    }
    break;
  case MOT_ADDR_CR:
    for (uint8_t i = 0; i < 1; i++)
      buff[i] = eeprom_read_byte((uint8_t *)(addr + i));
    if (buff[0] == 0) {
      buff[0] = (1 << MOT_CR_POWER) | (1 << MOT_CR_LED);
    }
    break;
  }
}
