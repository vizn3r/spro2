#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Code for high level motor control
// and I2C communication with the motor controller
//
// Motor can be set to rotate certain degrees or steps

// Direction of the motor, use MOT_DIR_CW or MOT_DIR
typedef enum {
  MOT_DIR_CW,
  MOT_DIR_CCW,
} mot_dir_t;

// I2C registers on the motor
typedef enum {
  // Angle mode
  MOT_REG_ANG = 0x00, // Set the angle motor should hold

  // State registers (read)
  MOT_REG_ANG_CURR = 0x04,   // Current angle
  MOT_REG_STAT = 0x08,       // State of the motor
  MOT_REG_STAT_ERROR = 0x09, // Error code

  // Control register
  MOT_REG_CONTROL = 0x0a, // Set the control register bytes

  // Testing code
  MOT_REG_TEST = 0x0F,
} mot_reg_t;

enum mot_control_reg_t {
  // Toggles
  MOT_CR_POWER, // On/Off
  MOT_CR_BREAK, // Enable break
  MOT_CR_DIR,   // Spin direction
  MOT_CR_LED,   // On/Off LED indicators
  MOT_CR_UNSET, // UNSET for now

  // Triggers
  MOT_CR_HOME,      // Set current position as home
  MOT_CR_CLEAR_ERR, // Clear error message
  MOT_CR_RESET,     // Reset motor
};

// Motor states stored in the MOT_REG_STAT register
typedef enum {
  MOT_STAT_ERROR,  // Duh
  MOT_STAT_OFF,    // Motor is off
  MOT_STAT_READY,  // Motor is on and ready
  MOT_STAT_MOVING, // Motor is moving
} mot_stat_t;

typedef enum {
  MOT_ERR_NONE,
  MOT_ERR_UNDEFINED,
  MOT_ERR_POWER,
  MOT_ERR_PID,
  MOT_ERR_DRIVER,
  MOT_ERR_I2C,
  MOT_ERR_I2C_INVALID_ADDR,
  MOT_ERR_NEW = 0xFF,
} mot_error_t;

typedef struct {
  uint8_t address;
  mot_stat_t state;
  mot_error_t error;
  uint8_t con_reg;
} motor_t;

// Motor high level control functions

// Initialize the library I2C and stuff
void mot_init();

// Create new motor object, returns address pointer
motor_t mot_new(uint8_t i2c_address);

// General functions for communication

// Write `data` to `motor` `reg` register
void mot_write_reg(motor_t *motor, mot_reg_t reg, uint8_t *data, size_t len);

// Read `data` from `reg` register, returns uint8_t[]
void mot_read_reg(motor_t *motor, mot_reg_t reg, uint8_t *dest, size_t len);

// Setting of the motor settings

// Rotate `angle` degrees on `motor` in `direction`
void mot_angle(motor_t *motor, mot_dir_t direction, float angle);

// Getting current values of the motor

// Get current position of the `motor`
void mot_get_angle(motor_t *motor, uint8_t *dest);

// Get current status of the `motor`
mot_stat_t mot_get_stat(motor_t *motor);

// Get current error of the `motor`
mot_error_t mot_get_err(motor_t *motor);
