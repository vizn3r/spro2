#pragma once

#include <stdint.h>

void encoder_init();

volatile uint16_t adc_read(uint8_t channel);

void get_curr_ang();
