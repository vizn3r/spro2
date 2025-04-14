#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/uart_types.h"

#include "../lib/motor/motor.h"
#include "../lib/nextion/nx.h"

#define LED_PIN 2

#define UART UART_NUM_1
#define BUF_SIZE (1024)
#define RX 4
#define TX 5

void app_main() { mot_init(); }
