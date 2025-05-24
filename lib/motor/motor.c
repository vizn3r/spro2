#include "motor.h"
#include "../i2c/i2c.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void mot_init() { i2c_init(); }

motor_t mot_new(uint8_t i2c_address) {
  motor_t motor = {.address = i2c_address,
                   .error = MOT_ERR_UNDEFINED,
                   .state = MOT_STAT_OFF};
  return motor;
}

void mot_angle(motor_t *motor, mot_dir_t direction, float angle) {
  uint8_t buff[4];
  uint8_t cr_buff[1];

  motor->con_reg = (direction << MOT_CR_DIR);
  cr_buff[0] = motor->con_reg;

  memcpy(buff, &angle, sizeof(float));

  mot_write_reg(motor, MOT_REG_CONTROL, cr_buff, 1);
  mot_write_reg(motor, MOT_REG_ANG, buff, 4);
}

void mot_write_reg(motor_t *motor, mot_reg_t reg, uint8_t *data, size_t len) {
  i2c_write(motor->address, reg, data, len);
}

void mot_read_reg(motor_t *motor, mot_reg_t reg, uint8_t *dest, size_t len) {
  i2c_read(motor->address, reg, dest, len);
}

void mot_get_angle(motor_t *motor, uint8_t *dest) {
  mot_read_reg(motor, MOT_REG_ANG_CURR, dest, 4);
}

mot_stat_t mot_get_stat(motor_t *motor) {
  uint8_t buff[1];
  mot_read_reg(motor, MOT_REG_STAT, buff, 1);
  return buff[0];
}

mot_error_t mot_get_err(motor_t *motor) {
  uint8_t buff[1];
  mot_read_reg(motor, MOT_REG_STAT_ERROR, buff, 1);
  return buff[0];
};
