#include "driver.h"
#include "com.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <util/atomic.h>

static float Kp = 0.0f;
static float Ki = 0.0f;
static float Kd = 0.0f;

#define DT 0.001f // 1ms

static uint8_t PWM_MIN = 0;
#define PWM_MAX 255

#define INT_MAX 100.0f
#define INT_MIN -INT_MAX

static float pid_integral = 0.0f;
static float pid_prev_err = 0.0f;

#define PID_LOG_SIZE 500

typedef enum {
  PID_NOT_TUNED,
  PID_TUNE_ST,
  PID_TUNE_TEST,
  PID_TUNE_CALC,
  PID_TUNED,
} pid_status_t;

static pid_status_t pid_state = PID_NOT_TUNED;
static float pid_tune_angle = 0.0f;
static uint32_t pid_log_time[PID_LOG_SIZE];
static float pid_log_angle[PID_LOG_SIZE];
uint16_t pid_log_index = 0;

float average(float *data) {
  float total = 0.0f;
  for (int i = 0; i < PID_LOG_SIZE; i++) {
    total += data[i];
  }
  return total / (float)PID_LOG_SIZE;
}

void mot_drive() {
  float curr_angle = 0.0f;
  float dest_angle = 0.0f;
  uint8_t pwm_f = 0, pwm_r = 0;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    curr_angle = MOT_CANG;
    dest_angle = MOT_ANG;
  }

  switch (pid_state) {
  case PID_NOT_TUNED:
    // Check flash

    pid_tune_angle = curr_angle;
    pid_log_index = 0;
    pid_state = PID_TUNE_ST;
    break;
  case PID_TUNE_ST:
    if (fabsf(curr_angle - pid_tune_angle) > 1.0f) {
      PWM_MIN += 1;
      pwm_f = PWM_MIN;
      pwm_r = 0;
    } else {
      pwm_f = 0;
      pwm_r = 0;
      pid_state = PID_TUNE_TEST;
    }
    break;
  case PID_TUNE_TEST:
    pwm_f = PWM_MIN + 20;
    pwm_r = 0;
    pid_log_time[pid_log_index] = pid_log_index;
    pid_log_angle[pid_log_index] = curr_angle;
    pid_log_index++;
    if (pid_log_index > PID_LOG_SIZE) {
      pid_state = PID_TUNE_CALC;
    }
    break;
  case PID_TUNE_CALC: {
    // Differentiate
    float speed[PID_LOG_SIZE];
    speed[0] = 0;
    for (int i = 1; i < PID_LOG_SIZE; i++) {
      speed[i] = (pid_log_angle[i] - pid_log_angle[i - 1]) / DT;
    }

    float theta = 0.0f;
    for (int i = 0; i < PID_LOG_SIZE; i++) {
      if (speed[i] > 1.0f) {
        theta = pid_log_time[i] / 1000.0f;
        break;
      }
    }

    float avg_speed = average(speed);
    float target_speed = 0.632 * avg_speed;
    float tau = 0.0f;
    for (int i = 0; i < PID_LOG_SIZE; i++) {
      if (speed[i] >= target_speed) {
        tau = (pid_log_time[i] / 1000.0f) - theta;
        break;
      }
    }

    float K = avg_speed / (PWM_MIN + 20);
    float Ti = (2.5 * theta * (tau + 0.6 * theta)) / (tau + 2.0f * theta);
    float Td = (0.37 * theta * tau) / (tau + 0.6 * theta);
    Kp = (1.35 / K) * (tau / theta + 0.25);
    Ki = Kp / Ti;
    Kd = Kp * Td;
    pid_state = PID_TUNED;

    // Update flash
  } break;
  case PID_TUNED: {
    float error = dest_angle - curr_angle;
    error = fmodf(error + 180.0f, 360.0f) - 180.0f;

    pid_integral += error * DT;
    pid_integral = fminf(fmaxf(pid_integral, INT_MIN), INT_MAX);

    float derivative = (error - pid_prev_err) / DT;
    pid_prev_err = error;

    float u = Kp * error + Ki * pid_integral + Kd * derivative;
    u = fminf(fmaxf(u, -PWM_MAX), PWM_MAX);

    if (error != 0.0f && fabsf(u) < PWM_MIN) {
      u = (u > 0) ? PWM_MIN : -PWM_MIN;
    }

    if (u >= 0) {
      pwm_f = (uint8_t)u;
      pwm_r = 0;
    } else {
      pwm_f = 0;
      pwm_r = (uint8_t)(-u);
    }
  } break;
  };

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
