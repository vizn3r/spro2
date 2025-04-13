#pragma once

#include <stdbool.h>
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
  // Control registers (read/write)
  MOT_REG_CONTROL, // Set the control register bytes
  // Step mode
  MOT_REG_STEP_ANGLE, // Set angle of one step in step mode
  MOT_REG_STEPS,      // Set the steps the motor should make
  // Angle mode
  MOT_REG_ANGLE, // Set the angle motor should hold

  // State registers (read)
  MOT_REG_ANG_CURR,   // Current angle
  MOT_REG_STEPS_CURR, // Current number of steps from home
  MOT_REG_STAT,       // State of the motor
  MOT_REG_STAT_ERROR, // Error code, when no error, it will be MOT_ERR_NONE
} mot_reg_t;

enum mot_control_reg_t {
  // Toggles
  MOT_CR_POWER, // On/Off
  MOT_CR_BREAK, // Enable break
  MOT_CR_DIR,   // Spin direction
  MOT_CR_LED,   // On/Off LED indicators
  MOT_CR_MODE,  // Step/Angle mode

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
} motor_t;

// Motor high level control functions

// Initialize the library I2C and stuff
void mot_init();

// Create new motor object, returns address pointer
motor_t *mot_new(uint8_t i2c_address);

// General functions for communication

// Write `data` to `motor` `reg` register
void mot_write_reg(motor_t *motor, mot_reg_t reg, uint8_t data);

// Read `data` from `reg` register, returns uint8_t[]
uint8_t *mot_read_reg(motor_t *motor, mot_reg_t reg);

// Setting of the motor settings

// Do n `steps` on `motor` in `direction`
void mot_step(motor_t *motor, mot_dir_t direction, unsigned int steps);

// Rotate `angle` degrees on `motor` in `direction`
void mot_angle(motor_t *motor, mot_dir_t direction, float angle);

// Getting current values of the motor

// Get current position of the `motor`
uint8_t mot_get_pos(motor_t *motor);

// Get current status of the `motor`
mot_stat_t mot_get_stat(motor_t *motor);

// Get current error of the `motor`
mot_error_t mot_get_err(motor_t *motor);
