#include "encoder.h"
#include "com.h"
#include "driver.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <util/delay.h>

#define SAMPLE_SIZE 10
#define ADC_MID 512.0f
#define ADC_MAX 1024.0f
#define ADC_SETTLE 50
#define REDUCTION 36.0f

volatile float a0_buff[SAMPLE_SIZE];
volatile uint8_t a0_head = 0;
volatile float a0_sum = 0;

volatile float a1_buff[SAMPLE_SIZE];
volatile uint8_t a1_head = 0;
volatile float a1_sum = 0;

void encoder_init() {
  // ADC_init
  // setting the reference voltage to Vcc.
  ADMUX = (1 << REFS0) | (0 << ADLAR);

  ADCSRB = 0;
  ADCSRA = (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
  DIDR0 = (1 << ADC0D) | (1 << ADC1D);

  _delay_ms(2);

  // Initialize buffers to zero
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    a0_buff[i] = 0.0f;
    a1_buff[i] = 0.0f;
  }
  a0_sum = 0.0f;
  a1_sum = 0.0f;
  a0_head = 0;
  a1_head = 0;

  // TIMER0_init
  // Set OCR0A to 249 (not 250) for exact 1ms with 64 prescaler at 16MHz
  // 16MHz / 64 / 250 = 1000Hz = 1ms
  OCR0A = 249;

  // TIMER0 CTC with OCR0A as Top
  TCCR0A = (1 << WGM01);

  // TIMER0 - 64 prescaler
  TCCR0B = (1 << CS01) | (1 << CS00);

  // Enables Interrupts for TIMER0_COMPA_vect every time the timer overflows
  TIMSK0 = (1 << OCIE0A);
}

float adc_avg(uint16_t sample, volatile float *buff, volatile uint8_t *head,
              volatile float *sum) {
  // removes the oldest sample value from sum
  *sum -= buff[*head];

  // Store new sample
  buff[*head] = (float)sample; // Explicit cast

  // Add new sample to sum
  *sum += (float)sample;

  // Update head
  *head = (*head + 1) % SAMPLE_SIZE;

  return *sum / SAMPLE_SIZE;
}

uint16_t adc_read(uint8_t channel) {
  ADMUX = (ADMUX & 0xF0) | channel;
  ADCSRA |= (1 << ADSC);

  _delay_us(ADC_SETTLE);

  while (ADCSRA & (1 << ADSC))
    ;

  _delay_us(ADC_SETTLE);
  return ADC;
}

volatile float mot_tot_angle = 0.0f;
static float prev_abs_angle = 0.0f;

ISR(TIMER0_COMPA_vect) {
  // Read ADC values
  uint16_t A0 = adc_read(6);
  uint16_t A1 = adc_read(7);

  // Compute averages
  float avg0 = adc_avg(A0, a0_buff, &a0_head, &a0_sum);
  float avg1 = adc_avg(A1, a1_buff, &a1_head, &a1_sum);

  // Subtract DC offset and normalize to [-1, 1]
  float sin_val = (avg0 - ADC_MID) / ADC_MID;
  float cos_val = (avg1 - ADC_MID) / ADC_MID;

  float abs_angle = atan2f(sin_val, cos_val) * (180.0f / M_PI);
  if (abs_angle < 0.0f)
    abs_angle += 360.0f;

  // Absolute angle calculation
  float delta = abs_angle - prev_abs_angle;
  if (delta > 180.0f)
    delta -= 360.0f;
  if (delta < -180.0f)
    delta += 360.0f;

  mot_tot_angle += delta;
  prev_abs_angle = abs_angle;

  float out_tot_angle = mot_tot_angle / REDUCTION;
  float angle = fmodf(out_tot_angle, 360.0f);
  if (angle < 0.0f)
    angle += 360.0f;

  // Update global angle variable atomically
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { MOT_CANG = angle; }
}
