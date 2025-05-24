#include "driver.h"
#include "com.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>
#include <stdint.h>
#include <util/atomic.h>

#define Kp 25.0f
#define Ki 2.5f
#define Kd 7.5f

#define DT 0.001f // 1ms

#define PWM_MIN 100
#define PWM_MAX 255

#define INT_MIN -100.0f
#define INT_MAX 100.0f

static float pid_integral = 0.0f;
static float pid_prev_err = 0.0f;

void mot_drive() {
  float curr_angle = 0.0f;
  float dest_angle = 0.0f;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    curr_angle = MOT_CANG;
    dest_angle = MOT_ANG;
  }

  float error = dest_angle - curr_angle;
  if (error > 180.0f)
    error -= 360.0f;
  if (error < -180.0f)
    error += 360.0f;

  pid_integral += error * DT;
  pid_integral = fminf(fmaxf(pid_integral, INT_MIN), INT_MAX);

  float derivative = (error - pid_prev_err) / DT;
  pid_prev_err = error;

  float u = Kp * error + Ki * pid_integral + Kd * derivative;
  u = fminf(fmaxf(u, -PWM_MAX), PWM_MAX);

  if (error != 0.0f && fabsf(u) < PWM_MIN) {
    u = (u > 0) ? PWM_MIN : -PWM_MIN;
  }

  uint8_t pwm_f = 0, pwm_r = 0;
  if (u >= 0) {
    pwm_f = (uint8_t)u;
    pwm_r = 0;
  } else {
    pwm_f = 0;
    pwm_r = (uint8_t)(-u);
  }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    OCR1A = pwm_f;
    OCR1B = pwm_r;
  }
}

ISR(TIMER3_COMPA_vect) { mot_drive(); }

void driver_init(void) {
  // PWM Setup - for D9(PB5), D10(PB6)
  DDRB |= (1 << PB5) | (1 << PB6);
  // Sets OCR1A and OCR1B to compare match
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
  // Sets WGM12 which in addition to the WGM10 sets mode to 5 which is 8Bit Fast
  // PWM Sets the prescaler 1
  TCCR1B = (1 << WGM12) | (1 << CS10);
  OCR1A = 0;
  OCR1B = 0;

  // Timer setup
  TCCR3A = 0;
  TCCR3B = (1 << WGM32);
  TCCR3B |= (1 << CS31) | (1 << CS30);
  OCR3A = 249;
  TIMSK3 = (1 << OCIE3A);
}
