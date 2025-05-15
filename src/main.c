#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

#include "../lib/motor/motor.h"
#include "../lib/nextion/nx.h"
#include "i2c.h"
#include "portmacro.h"
#include <stdint.h>
#include <string.h>

#define FLAPS_N 30
#define DEG_PER_FLAP (uint8_t)(360 / FLAPS_N)

volatile unsigned int HOUR = 0;
volatile unsigned int MINUTE = 0;

volatile motor_t motors[4];

#define H0 motors[0]
#define H1 motors[1]
#define M0 motors[2]
#define M1 motors[3]

uint8_t dec_range[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

uint8_t get_current_flap(motor_t motor) {
  uint8_t ang_buff[4];
  mot_get_angle(&motor, ang_buff);
  float curr_ang = (*(float *)ang_buff);

  return curr_ang / DEG_PER_FLAP;
}

void move_to_flap(motor_t motor, uint8_t flap_n) {
  float angle = 0.0f;

  angle = (float)(flap_n * DEG_PER_FLAP);

  mot_angle(&motor, MOT_DIR_CCW, angle);
}

void update_time() {
  // Uupdate HOUR and MINUTE through NTP
}

// place is MSD
uint8_t get_digit(unsigned int num, uint8_t place) {
  while (place--) {
    num /= 10;
  }
  return num % 10;
}

void update_flap(motor_t motor, uint8_t flap_range, uint8_t req_flap_d) {
  uint8_t curr_flap_n = get_current_flap(motor);
  uint8_t curr_flap_d = 0;

  // Get current flap number
  for (uint8_t j = 0; j < flap_range; j++) {
    if (curr_flap_n % j == 0) {
      curr_flap_d = j;
      break;
    }
  }

  if (curr_flap_d != req_flap_d) {
    uint8_t req_flap_n = curr_flap_n + (req_flap_d - curr_flap_d);
    move_to_flap(motor, req_flap_n);
  }
}

void app_main() {
  mot_init();
  nx_init(0, 115200, 17, 16);

  H0 = mot_new(0x08); // Hours first decimal
  H1 = mot_new(0x09); // Hours second decimal
  M0 = mot_new(0x0A); // Minutes first decimal
  M1 = mot_new(0x0B); // Minutes second decimal

  for (;;) {
    update_time();

    update_flap(M1, 9, get_digit(MINUTE, 0));
    update_flap(M0, 5, get_digit(MINUTE, 1));
    update_flap(H1, 9, get_digit(HOUR, 0));
    update_flap(H0, 2, get_digit(HOUR, 1));
  }
}
