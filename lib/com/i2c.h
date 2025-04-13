#pragma once

#ifndef SLAVE_ADDR
#define SLAVE_ADDR 0x08
#endif

#include <stdint.h>
void i2c_slave_init(uint8_t addr);
