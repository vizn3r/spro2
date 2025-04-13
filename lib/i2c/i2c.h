#pragma once

#include <stdio.h>
#include <stdlib.h>

void i2c_init();

uint8_t i2c_read(uint8_t addr, uint8_t reg_addr);
void i2c_write(uint8_t addr, uint8_t reg_addr, uint8_t data);
