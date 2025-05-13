#pragma once

#include <stdint.h>

// Code for the high level commumication protocol

// Register struct
// Do not change the order of these, or im gonna find you
typedef struct {
  // Angle mode
  uint8_t ANGLE[4]; // Set the angle motor should hold

  // State registers (read)
  uint8_t ANG_CURR[4];   // Current angle
  uint8_t STAT[1];       // State of the motor
  uint8_t STAT_ERROR[1]; // Error code, when no error, it will be MOT_ERR_NONE

  // Control register
  uint8_t CONTROL[1];
} mot_reg_t;

// EEPROM addresses
typedef enum {
  MOT_ADDR_ANG = 0x00,
  MOT_ADDR_CANG = 0x04,
  MOT_ADDR_SR = 0x08,
  MOT_ADDR_SER = 0x09,
  MOT_ADDR_CR = 0x0a,

  // Home angle
  MOT_ADDR_HANG = 0x0b,

} mot_eeprom_addr_t;

volatile uint8_t mot_registers[sizeof(mot_reg_t)];

#define MOT_REG_SIZE sizeof(mot_registers)

#define _MOT_REGS ((volatile mot_reg_t *)mot_registers)

#define UINT8_TO_FLOAT(b) (*(volatile float *)(b))
#define FLOAT_TO_UINT8(f) ((uint8_t *)(&(f)))
#define MOT_TO_UINT8(m) ((uint8_t *)&(m))

// Motor Angle Register
#define MOT_ANG UINT8_TO_FLOAT(_MOT_REGS->ANGLE)

// Motor Current Angle Register
#define MOT_CANG UINT8_TO_FLOAT(_MOT_REGS->ANG_CURR)

// Motor Status Register
#define MOT_SR _MOT_REGS->STAT[0]

// Motor Status Error Register
#define MOT_SER _MOT_REGS->STAT_ERROR[0]

// Motor Control Register
#define MOT_CR _MOT_REGS->CONTROL[0]

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

#define MOT_CR_READ(bit) ((MOT_CR >> (bit)) & 1)
#define MOT_CR_SET(bit) (MOT_CR |= (1 << (bit)))
#define MOT_CR_CLEAR(bit) (MOT_CR &= ~(1 << (bit)))

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
void eeprom_or_def(mot_eeprom_addr_t addr, uint8_t *reg);
