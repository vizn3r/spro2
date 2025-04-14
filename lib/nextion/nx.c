/*

Created by Simon Vizner 2024

*/

#include "nx.h"
#include "driver/uart.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include "soc/clk_tree_defs.h"
#include "soc/soc.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uart_port_t uart_num;

size_t nx_read(uint8_t *buffer) {
  size_t term_counter = 0;
  size_t index = 0;

  while (1) {
    uint8_t c;
    int len = uart_read_bytes(uart_num, &c, 1, portMAX_DELAY);
    if (len != 1)
      break;

    if (c == 0xFF)
      term_counter++;
    else
      term_counter = 0;

    if (term_counter == 0 && c == 0xFF)
      break;
    buffer[index++] = c;

    if (term_counter == 3)
      break;
  }
  return index;
}

size_t nx_read_multiple(uint8_t *buffer) {
  size_t term_counter = 0;
  size_t index = 0;
  while (1) {
    uint8_t c;
    int len = uart_read_bytes(uart_num, &c, 1, portMAX_DELAY);
    if (len != 1)
      break;

    if (term_counter == 3 && c != 0xFF)
      break;
    if (c == 0xFF)
      term_counter++;
    else
      term_counter = 0;
    buffer[index++] = c;
  }
  return index;
}

uint8_t *nx_alloc_buff() {
  return calloc(NX_RETURN_BUFF_SIZE, sizeof(uint8_t));
}

void nx_clean_buff(uint8_t *buff, size_t size) {
  for (size_t i = 0; i < size; i++)
    buff[i] = 0x00;
}

void nx_write(uint8_t *bytes, size_t size) {
  uart_write_bytes(uart_num, (const char *)bytes, size);
}

void nx_send(const char *str, ...) {
  va_list args;
  va_start(args, str);
  size_t size = vsnprintf(NULL, 0, str, args);
  char *formatted = malloc(size + 1);
  if (formatted != NULL) {
    vsnprintf(formatted, size, str, args);
    uart_write_bytes(uart_num, formatted, size);
    free(formatted);
    formatted = NULL;
  }
  va_end(args);

  nx_send_term();
}

size_t nx_send_read(uint8_t *buff, const char *str, ...) {
  va_list args;
  va_start(args, str);
  size_t size = vsnprintf(NULL, 0, str, args) + 1;
  char *formatted = malloc(size + 1);
  if (formatted != NULL) {
    vsnprintf(formatted, size, str, args);
    uart_write_bytes(uart_num, formatted, size);
    free(formatted);
    formatted = NULL;
  }
  va_end(args);
  nx_send_term();

  return nx_read(buff);
}

OnTouchFunc *on_touch_buff = NULL;
size_t on_touch_buff_size = 0;

void nx_on_touch(uint8_t id, uint8_t page_n, void (*trigger)(void)) {
  on_touch_buff_size++;
  OnTouchFunc *temp_otfb =
      realloc(on_touch_buff, sizeof(OnTouchFunc) * on_touch_buff_size);
  if (temp_otfb == NULL)
    return;
  on_touch_buff = temp_otfb;

  on_touch_buff[on_touch_buff_size - 1].id = id;
  on_touch_buff[on_touch_buff_size - 1].page_n = page_n;
  on_touch_buff[on_touch_buff_size - 1].on_release = 0;
  on_touch_buff[on_touch_buff_size - 1].trigger = trigger;
}

void nx_on_release(uint8_t id, uint8_t page_n, void (*trigger)(void)) {
  on_touch_buff_size++;
  OnTouchFunc *temp_otfb =
      realloc(on_touch_buff, sizeof(OnTouchFunc) * on_touch_buff_size);
  if (temp_otfb == NULL)
    return;
  on_touch_buff = temp_otfb;

  on_touch_buff[on_touch_buff_size - 1].id = id;
  on_touch_buff[on_touch_buff_size - 1].page_n = page_n;
  on_touch_buff[on_touch_buff_size - 1].on_release = 1;
  on_touch_buff[on_touch_buff_size - 1].trigger = trigger;
}

int *nx_check(uint8_t *buff, size_t buff_size) {
  int allocated = 0;
  if (buff == NULL || buff_size == 0) {
    buff = nx_alloc_buff();
    buff_size = nx_read(buff);
    allocated = 1;
  }

  if (buff_size == 0 || !nx_has_termination(buff, buff_size)) {
    if (allocated == 1)
      free(buff);
    return NULL;
  }

  int *return_buff = NULL;

  // Check if the return code is error code
  if (buff[0] <= 0x24)
    nx_send_err(buff[0]);
  else {
    switch (buff[0]) {
    case NX_TOUCH_EVENT:
      for (size_t i = 0; i < on_touch_buff_size; i++) {
        if (buff[1] == on_touch_buff[i].page_n &&
            buff[2] == on_touch_buff[i].id && buff[3] == 0x01 &&
            on_touch_buff[i].on_release == 0) {
          on_touch_buff[i].trigger();
        }
        if (buff[1] == on_touch_buff[i].page_n &&
            buff[2] == on_touch_buff[i].id && buff[3] == 0x00 &&
            on_touch_buff[i].on_release == 1) {
          on_touch_buff[i].trigger();
        }
      }
      break;
    case NX_CURRENT_PAGE_NUMBER:
      return_buff = (int *)malloc(sizeof(int));
      return_buff[0] = buff[1];
      break;
    case NX_NUMERIC_DATA_ENCLOSED:
      return_buff = (int *)malloc(sizeof(int));
      return_buff[0] = buff[1] + buff[2] * (0xFF + 1) + buff[3] * (0xFFFF + 1) +
                       buff[4] * (0xFFFFFF + 1);
      break;
    }
  }
  if (allocated == 1)
    free(buff);
  buff = NULL;
  return return_buff;
}

uint8_t nx_current_page(void) {
  uint8_t *buff = nx_alloc_buff();
  size_t buff_size = nx_send_read(buff, "page");

  while (buff_size == 0)
    buff_size = nx_read(buff);

  if (buff[0] == NX_CURRENT_PAGE_NUMBER &&
      nx_has_termination(buff, buff_size)) {
    uint8_t page = buff[1];
    free(buff);
    buff = NULL;
    return page;
  } else {
    free(buff);
    buff = NULL;
    return 0x00;
  }
}

bool nx_has_termination(uint8_t *bytes, size_t size) {
  if (size < 3)
    return false;
  return (bytes[size - 3] == 0xFF && bytes[size - 2] == 0xFF &&
          bytes[size - 1] == 0xFF);
}

bool nx_is_startup(uint8_t *bytes, size_t size) {
  if (bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0x00 &&
      nx_has_termination(bytes, size))
    return true;
  return false;
}

void nx_send_term() {
  uint8_t term[] = {255, 255, 255};
  nx_write(term, 3);
}

void nx_send_err(uint8_t code) {
  nx_send("cls RED");
  nx_send(
      "xstr 10,10,300,220,1,RED,BLACK,1,1,1,\"ERROR CODE: %x EXITING PROGRAM\"",
      code);
  exit(0);
}

uint8_t **nx_separate_multiple(uint8_t *bytes, size_t size) {
  uint8_t **buffer = malloc(size);
  size_t term_counter = 0;
  size_t arr_index = 0;
  for (size_t i = 0; i < size; i++) {
    uint8_t c = bytes[i];
    if (c == 255)
      term_counter++;
    else
      term_counter = 0;
    buffer[arr_index][i++] = c;

    if (term_counter == 3)
      arr_index++;
  }
  return buffer;
}

size_t nx_buff_size(uint8_t *buff) { return sizeof(buff) / sizeof(uint8_t); }

void nx_init(uart_port_t port, int baud, int rx_pin, int tx_pin) {
  uart_num = port;
  uart_config_t conf = {.baud_rate = baud,
                        .data_bits = UART_DATA_8_BITS,
                        .parity = UART_PARITY_DISABLE,
                        .stop_bits = UART_STOP_BITS_1,
                        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                        .source_clk = UART_SCLK_DEFAULT};
  uart_param_config(uart_num, &conf);
  uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE);
  uart_driver_install(uart_num, 2048, 2048, 0, NULL, 0);

  uint8_t *buff = nx_alloc_buff();
  size_t buff_size;

  nx_send("rest");

  // Wait for nextion STARTUP and READY
  while (nx_is_startup(buff, nx_read_multiple(buff)))
    ;

  free(buff);
  buff = NULL;
}
