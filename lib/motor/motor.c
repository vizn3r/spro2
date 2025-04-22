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
  uint8_t float_uint8_arr[4];
  memcpy(float_uint8_arr, &angle, sizeof(float));
  i2c_write(motor->address, MOT_REG_ANGLE, float_uint8_arr, 4);
}

void mot_write_reg(motor_t *motor, mot_reg_t reg, uint8_t *data, size_t len) {
  i2c_write(motor->address, reg, data, len);
}

void mot_read_reg(motor_t *motor, mot_reg_t reg, uint8_t **dest) {
  i2c_read(motor->address, reg);
}

uint8_t mot_get_pos(motor_t *motor);

mot_stat_t mot_get_stat(motor_t *motor);

mot_error_t mot_get_err(motor_t *motor);
