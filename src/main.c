#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "nvs.h"

#include "../lib/api/api.h"
#include "../lib/motor/motor.h"
#include "../lib/nextion/nx.h"
#include "hal/gpio_types.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// Used for debug
#define DEBUG 0

#define CONFIG_FREERTOS_HZ 80000000 // 80MHz

#define UPDATE_PERIOD 1000 * 60 * 30 // Every 30 minutes

#define BUZZER_PIN 20 // For the alarm function

#define FLAPS_N 30
#define DEG_PER_FLAP (uint8_t)(360 / FLAPS_N)

volatile unsigned int HOUR = 0;
volatile unsigned int MINUTE = 0;

volatile unsigned int ALARM_HOUR = 0;
volatile unsigned int ALARM_MINUTE = 0;
volatile bool ALARM_ENABLED = false;

volatile motor_t motors[4];
#define H0 motors[0]
#define H1 motors[1]
#define M0 motors[2]
#define M1 motors[3]

char *WIFI_PASS = 0;
size_t WIFI_PASS_LEN = 0;
char *LOCATION = 0;
size_t LOCATION_LEN = 0;

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

// xTimer handles

void update_weather(TimerHandle_t xTimer) {
  get_weather_now("Sonderborg DK", "en");
}

// NVS funcs

void nvs_read(const char *key, char *dest, size_t *length) {
  nvs_handle_t nvs;
  esp_err_t err;

  err = nvs_open("storage", NVS_READONLY, &nvs);
  // handle err

  err = nvs_get_str(nvs, key, NULL, length);
  if (err == ESP_OK && *length != 0) {
    free(dest);
    dest = malloc(*length);
    err = nvs_get_str(nvs, key, dest, length);
    // handle err
  }

  nvs_close(nvs);
}

void nvs_write(const char *key, char *value) {
  nvs_handle_t nvs;
  esp_err_t err;

  err = nvs_open("storage", NVS_READWRITE, &nvs);
  // hnadle err

  err = nvs_set_str(nvs, key, value);
  // handle err

  nvs_commit(nvs);
  nvs_close(nvs);
}

// Nextion display

// void display_error(const char *message) {}

void config_wifi_pass() {
  // Check NVS
  nvs_read("wifi_pass", WIFI_PASS, &WIFI_PASS_LEN);
  if (WIFI_PASS_LEN != 0)
    return;

  // Get password from user
}

void config_location() {
  // Check NVS
  nvs_read("location", LOCATION, &LOCATION_LEN);
  if (LOCATION_LEN != -1)
    return;

  // Get password from user
}

void app_main() {
  if (DEBUG) {
    mot_init();
    motor_t motor = mot_new(0x08);

    for (;;) {
      mot_angle(&motor, 0, 0);
      vTaskDelay(portTICK_PERIOD_MS * 5000);
      mot_angle(&motor, 0, 180);
      vTaskDelay(portTICK_PERIOD_MS * 5000);
    }
    return;
  }

  mot_init();
  nx_init(0, 115200, 17, 16);

  TimerHandle_t weather_timer =
      xTimerCreate("updt_weather", pdMS_TO_TICKS(UPDATE_PERIOD), pdTRUE, NULL,
                   update_weather);

  H0 = mot_new(0x08); // Hours first decimal
  H1 = mot_new(0x09); // Hours second decimal
  M0 = mot_new(0x0A); // Minutes first decimal
  M1 = mot_new(0x0B); // Minutes second decimal

  xTimerStart(weather_timer, 0);

  // Startup

  // Setup BUZZER_PIN
  gpio_config_t buzzer = {.pin_bit_mask = (1ULL << BUZZER_PIN),
                          .mode = GPIO_MODE_OUTPUT,
                          .pull_up_en = GPIO_PULLUP_DISABLE,
                          .pull_down_en = GPIO_PULLDOWN_ENABLE,
                          .intr_type = GPIO_INTR_DISABLE};

  gpio_config(&buzzer);

  // Initial config from user

  // Updating flaps

  // Main loop
  for (;;) {
    // Updating time / weather
    update_time();

    // Updating flaps
    update_flap(M1, 9, get_digit(MINUTE, 0));
    update_flap(M0, 5, get_digit(MINUTE, 1));
    update_flap(H1, 9, get_digit(HOUR, 0));
    update_flap(H0, 2, get_digit(HOUR, 1));

    // Check alarm
    if (ALARM_ENABLED && (ALARM_HOUR == HOUR) && (ALARM_MINUTE == MINUTE)) {
      // Start alarm
      gpio_set_level(BUZZER_PIN, 1);

      // Pop a stop button on the display and wait for the button press
    }

    // Check for user input
  }
}
