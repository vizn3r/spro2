#pragma once

#include <stdint.h>

void encoder_init();

uint16_t adc_read(uint8_t channel);

void get_curr_ang();
