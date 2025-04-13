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
// uart_config_t uart_config = {.baud_rate = 9600,
//                              .data_bits = UART_DATA_8_BITS,
//                              .parity = UART_PARITY_DISABLE,
//                              .stop_bits = UART_STOP_BITS_1,
//                              .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

// gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
// uart_driver_install(UART, BUF_SIZE * 2, 0, 0, NULL, 0);
// uart_param_config(UART, &uart_config);

// xTaskCreate(uart_task, "uart_task", 2048, NULL, 10, NULL);
