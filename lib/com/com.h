#pragma once

#include <stdint.h>

// Code for the high level commumication protocol

typedef struct {
  // Control registers (read/write)
  uint8_t CONTROL; // Set the control register bytes
  // Step mode
  uint8_t STEP_ANGLE; // Set angle of one step in step mode
  uint8_t STEPS;      // Set the steps the motor should make
  // Angle mode
  uint8_t ANGLE; // Set the angle motor should hold

  // State registers (read)
  uint8_t ANG_CURR;   // Current angle
  uint8_t STEPS_CURR; // Current number of steps from home
  uint8_t STAT;       // State of the motor
  uint8_t STAT_ERROR; // Error code, when no error, it will be MOT_ERR_NONE
} mot_reg_t;

volatile uint8_t mot_registers[sizeof(mot_reg_t)];

#define MOT_REG_SIZE sizeof(mot_registers)

#define _MOT_REGS ((volatile mot_reg_t *)mot_registers)

// Motor Control Register
#define MOT_CR _MOT_REGS->CONTROL

// Motor Step Angle Register
#define MOT_STP_ANG _MOT_REGS->STEP_ANGLE

// Motor Steps Register
#define MOT_STP _MOT_REGS->STEPS

// Motor Angle Register
#define MOT_ANG _MOT_REGS->ANGLE

// Motor Current Angle Register
#define MOT_CANG _MOT_REGS->ANG_CURR

// Motor Current Steps Register
#define MOT_CSTP _MOT_REGS->STEPS_CURR

// Motor Status Register
#define MOT_SR _MOT_REGS->STAT

// Motor Status Error Register
#define MOT_SER _MOT_REGS->STAT_ERROR

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

#define MOT_DIR_CW 0
#define MOT_DIR_CCW 1

#define MOT_MODE_STEP 0
#define MOT_MODE_ANGLE 1

#define MOT_DO_HOME 1
#define MOT_DO_SAVE 1
#define MOT_DO_RESET 1

// Motor states stored in the MOT_REG_STAT register
typedef enum {
  MOT_STAT_ERROR,  // Motor has encountered error, stored in MOT_SER
  MOT_STAT_OFF,    // Motor is off
  MOT_STAT_READY,  // Motor is on and ready
  MOT_STAT_MOVING, // Motor is moving
} mot_stat_t;

// Error state stored in MOT_SER
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

void com_init();
