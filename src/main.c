#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/uart_types.h"

#include "../lib/motor/motor.h"

#define LED_PIN 2

#define UART UART_NUM_1
#define BUF_SIZE (1024)
#define RX 4
#define TX 5

void uart_task(void *arg) {
  uint8_t data[BUF_SIZE];

  for (;;) {
    int len = uart_read_bytes(UART, data, BUF_SIZE, pdMS_TO_TICKS(1000));
  }
}

void app_main() { mot_init(); }
