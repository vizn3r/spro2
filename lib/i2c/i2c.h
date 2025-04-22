#pragma once

#include <stdio.h>
#include <stdlib.h>

void i2c_init();

void i2c_read(uint8_t addr, uint8_t reg_addr, uint8_t *dest, size_t len);
void i2c_write(uint8_t addr, uint8_t reg_addr, uint8_t *data, size_t len);
