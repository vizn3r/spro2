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

#define H0_RANGE 2
#define H1_RANGE 9
#define M0_RANGE 5
#define M1_RANGE 9

#define FLAPS_N 30
#define DEG_PER_FLAP (uint8_t)(360 / FLAPS_N)

volatile unsigned int HOUR = 0;
volatile unsigned int MINUTE = 0;

uint8_t dec_range[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

uint8_t get_current_flap(motor_t motor) {
  uint8_t ang_buff[4];
  mot_get_angle(&motor, ang_buff);
  float curr_ang = (*(float *)ang_buff);

  return curr_ang / DEG_PER_FLAP;
}

void move_to_flap(motor_t *motor, uint8_t flap_n) {
  float angle = 0.0f;

  angle = (float)(flap_n * DEG_PER_FLAP);

  mot_angle(motor, MOT_DIR_CCW, angle);
}

void app_main() {
  mot_init();
  nx_init(0, 115200, 17, 16);

  motor_t h0 = mot_new(0x08); // Hours first decimal
  motor_t h1 = mot_new(0x09); // Hours second decimal
  motor_t m0 = mot_new(0x0A); // Minutes first decimal
  motor_t m1 = mot_new(0x0B); // Minutes second decimal

  float angle;
  uint8_t buff[4];
  for (;;) {

    // angle = 1000.0f;
    // memcpy(buff, &angle, sizeof(float));
    // i2c_write(m1.address, MOT_REG_ANGLE, buff, 4);
    // vTaskDelay(1000 / portTICK_PERIOD_MS);

    // angle = 100.0f;
    // memcpy(buff, &angle, sizeof(float));
    // i2c_write(m1.address, MOT_REG_ANGLE, buff, 4);
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
