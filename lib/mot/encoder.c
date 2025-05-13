#include "encoder.h"
#include "com.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>
#include <stdint.h>
#include <util/atomic.h>

#define SAMPLE_SIZE 10
#define ADC_MID 512.0f
#define ADC_MAX 1024.0f

void encoder_init() {
  // ADC_init
  // setting the refferance voltage to Vcc.
  ADMUX = (1 << REFS0);
  // Setting the prescaler to 128 which gives a frequency of 125KHz
  ADCSRA = (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);

  // TIMER0_init
  // Set OCR0A to 250 which gives a 1ms timer with 64 prescaler at 16MHz main
  // clock
  OCR0A = 250;
  // TIMER0 CTC with OCR0 as Top
  TCCR0A = (1 << WGM01);
  // TIMER0 - 64 prescaler
  TCCR0B = (1 << CS01) | (1 << CS00);
  // Enables Interrupts for TIMER0_COMPA_vect every time the timer overflows
  // meaning every 1ms
  TIMSK0 = (1 << OCIE0A);
}

volatile float abs_angle = 0;

volatile float a0_buff[SAMPLE_SIZE];
volatile uint8_t a0_head;
volatile float a0_sum;

volatile float a1_buff[SAMPLE_SIZE];
volatile uint8_t a1_head;
volatile float a1_sum;

float adc_avg(uint16_t sample, volatile float *buff, volatile uint8_t *head,
              volatile float *sum) {
  // removes the oldest sample value from sum
  *sum -= buff[*head];

  // Store new sample
  buff[*head] = sample;

  // Add new sample to sum
  *sum += sample;

  // Update head
  *head = (*head + 1) % SAMPLE_SIZE;

  return *sum / SAMPLE_SIZE;
}

void get_curr_ang() {
  // IMPLEMMENT ANGLE MEASUREMENT
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { MOT_CANG = abs_angle; }
}

volatile uint16_t adc_read(uint8_t channel) {
  // Set the port to be read
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
  // Start the read conversion
  ADCSRA |= (1 << ADSC);
  // Wait for conversion to finish
  while (ADCSRA & (1 << ADSC)) {
  }
  return ADC;
}

ISR(TIMER0_COMPA_vect) {
  int A0 = adc_read(0);
  int A1 = adc_read(1);

  // Compute averages
  float avg0 = adc_avg(A0, a0_buff, &a0_head, &a0_sum);
  float avg1 = adc_avg(A1, a1_buff, &a1_head, &a1_sum);

  // Subtract DC offset and normalize to [-1, 1]
  float sin_val = (avg0 - ADC_MID) / ADC_MID;
  float cos_val = (avg1 - ADC_MID) / ADC_MID;

  // Absolute angle calculation
  abs_angle = atan2(sin_val, cos_val) * (180.0f / (float)M_PI);
  if (abs_angle < 0) {
    abs_angle += 360;
  }
}
