#include "i2c.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "hal/gpio_types.h"
#include "hal/i2c_types.h"
#include "portmacro.h"
#include <stddef.h>
#include <stdint.h>

#ifndef I2C_SDA
#define I2C_SDA 8
#endif

#ifndef I2C_SCL
#define I2C_SCL 9
#endif

#define I2C_NUM 0
#define I2C_FREQ_HZ 100000
#define I2C_TIMEOUT 5000

void i2c_init() {
  i2c_config_t conf = {.mode = I2C_MODE_MASTER,
                       .sda_io_num = I2C_SDA,
                       .scl_io_num = I2C_SCL,
                       .sda_pullup_en = GPIO_PULLUP_ENABLE,
                       .scl_pullup_en = GPIO_PULLUP_ENABLE,
                       .master.clk_speed = I2C_FREQ_HZ};

  esp_err_t ret = i2c_param_config(I2C_NUM, &conf);
  if (ret != ESP_OK) {
    ESP_LOGE("I2C", "I2C config failed");
  }

  ret = i2c_driver_install(I2C_NUM, conf.mode, 0, 0, 0);
  if (ret != ESP_OK) {
    ESP_LOGE("I2C", "I2C driver isntall failed: %s", esp_err_to_name(ret));
  }
}

void i2c_write(uint8_t addr, uint8_t reg_addr, uint8_t *data, size_t len) {
  ESP_LOGD("I2C", "Write slave 0x%X reg 0x%X size %zu data[0] 0x%X",
           (addr << 1), reg_addr, len, data[0]);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg_addr, true);
  i2c_master_write(cmd, data, len, true);
  i2c_master_stop(cmd);

  esp_err_t ret =
      i2c_master_cmd_begin(I2C_NUM, cmd, I2C_TIMEOUT / portTICK_PERIOD_MS);
  if (ret != ESP_OK) {
    ESP_LOGE("I2C", "I2C write failed: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGD("I2C", "I2C write success");
  }

  i2c_cmd_link_delete(cmd);
}

void i2c_read(uint8_t addr, uint8_t reg_addr, uint8_t *dest, size_t len) {
  ESP_LOGD("I2C", "Read slave 0x%X reg 0x%X size %zu", (addr << 1), reg_addr,
           len);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg_addr, true);

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
  if (len > 1)
    i2c_master_read(cmd, dest, len - 1, I2C_MASTER_ACK);
  i2c_master_read_byte(cmd, dest + len - 1, I2C_MASTER_NACK);
  i2c_master_stop(cmd);

  esp_err_t err =
      i2c_master_cmd_begin(I2C_NUM, cmd, I2C_TIMEOUT / portTICK_PERIOD_MS);
  if (err != ESP_OK) {
    ESP_LOGE("I2C", "I2C read failed: %s", esp_err_to_name(err));
  } else {
    ESP_LOGD("I2C", "Read slave 0x%X reg 0x%X data[0] 0x%X", (addr << 1),
             reg_addr, *(dest + len - 1));
  }

  i2c_cmd_link_delete(cmd);
}
