#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

#include "../lib/motor/motor.h"
// #include "../lib/nextion/nx.h"
#include "i2c.h"
#include "portmacro.h"
#include <stdint.h>
#include <string.h>

#define LED_PIN 2

#define UART UART_NUM_1
#define BUF_SIZE (1024)
#define RX 4
#define TX 5

static const char *TAG = "led_blink";

// The ESP32-C3 Super Nano typically has an onboard LED connected to GPIO pin 2
// Adjust this if your board has the LED on a different pin
#define LED_PIN 2

void app_main(void) {
  // Configure the LED pin as output
  gpio_config_t io_conf = {.pin_bit_mask = (1ULL << LED_PIN),
                           .mode = GPIO_MODE_OUTPUT,
                           .pull_up_en = GPIO_PULLUP_DISABLE,
                           .pull_down_en = GPIO_PULLDOWN_DISABLE,
                           .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);

  ESP_LOGI(TAG, "LED Blink Example Started");

  // Blink the LED in an infinite loop
  while (1) {
    ESP_LOGI(TAG, "LED ON");
    gpio_set_level(LED_PIN, 1);            // Turn LED on
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second

    ESP_LOGI(TAG, "LED OFF");
    gpio_set_level(LED_PIN, 0);            // Turn LED off
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
  }
}

// void app_main() {
//   mot_init();
//
//   motor_t m1 = mot_new(0x08);
//
//   float angle;
//   uint8_t buff[4];
//   for (;;) {
//     angle = 1000.0f;
//     memcpy(buff, &angle, sizeof(float));
//     i2c_write(m1.address, MOT_REG_ANGLE, buff, 4);
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//
//     angle = 100.0f;
//     memcpy(buff, &angle, sizeof(float));
//     i2c_write(m1.address, MOT_REG_ANGLE, buff, 4);
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//   }
// }
