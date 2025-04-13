#include "i2c.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "hal/i2c_types.h"
#include "portmacro.h"

#define I2C_SCL 22
#define I2C_SDA 21
#define I2C_NUM I2C_NUM_0
#define I2C_FREQ_HZ 100000

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
    ESP_LOGE("I2C", "I2C driver isntall failed");
  }
}

void i2c_write(uint8_t addr, uint8_t reg_addr, uint8_t data) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg_addr, true);
  i2c_master_write_byte(cmd, data, true);

  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  if (ret != ESP_OK) {
    ESP_LOGE("I2C", "I2C write failed");
  }

  i2c_cmd_link_delete(cmd);
}

uint8_t i2c_read(uint8_t addr, uint8_t reg_addr) {
  uint8_t data = 0;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg_addr, true);

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
  i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);

  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  if (ret != ESP_OK) {
    ESP_LOGE("I2C", "I2C read failed");
  }

  i2c_cmd_link_delete(cmd);

  return data;
}
