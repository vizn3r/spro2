<div align="center">
  <h1>Software documentation - Split Flap Display Clock</h1>
  <p>Semester Project 2 - BEng in Mechatronics</p>
  <p>Group 1: Simon Vizner, Magnus Dubbeling Trip, Luke Huntington Piotti, Kshitiz Dev Adhikari, Joan Quiñonero Vizuete, Filip-Hannibal Dragos, Daniela Sanz Hernandez, Alessandra-Andreea Pricop</p>
  <p>27/04/2025</p>
</div>

## 1. Requirements


The high-level requirements for the software are straightforward:

- Firmware for motor control
- Firmware for the main controller
- Some kind of protocol for communication between the motor(s) and the main controller
- Nextion display integration
- External weather API integration

These are the main goals of the software part. This document will go through every part, explaining the high- and low-level implementation of every component.

## 2. The motor firmware and control

In development

## 3. The main controller firmware and control

In development

## 4. Internal communication protocol

### 4.1 The high-level functionality

#### Choosing the right protocol

This was not that hard of a task to choose, since our microcontrollers only support these protocols:

- UART
- I2C (TWI)
- SPI

Since we need to have a way to communicate with more than one microcontroller, we automatically crossed out UART, since it only supports duplex communication between two devices.

Then, we had to choose between I2C or SPI. Here, it also was easy to cross out SPI, since we need a way to communicate with the devices with as few wires as possible.

So, our obvious choice is I2C. I2C (or TWI in AVR documentation) requires only two wires (if we don’t count in the ground wire) for communication between multiple devices, by use of device addresses (something like IP addresses) and master-slave configuration.

In our use case, the main controller is the master, and the motors are the slaves.

#### Data transfer

We had to come up with some way to standardize the data transfer between the main controller and the motors, so we developed a register system, where we have software registers for writing the data (angle, mode of the motor, etc.) and reading the data (current angle, status of the motor, etc.).

So, if we want to write to, let’s say, `ANGLE` register, the data flow goes as following:

- Send the 7 bit address
- Send the W (write) bit
- Send the 8 bit register address we want to write to
- Send the data
- End the data transfer

If we want to read from, for example `ANG_CURR` (current angle) register, we follow a very similar process:

- Send the 7 bit address
- Send the R (read) bit
- Send the 8 bit register address we want to read from
- Wait for the data
- Save the incoming data
- End the data transfer

This process is partially handled by the ESP-IDF framework on the main controller side, but on the motor side, we have written this all from scratch. We are using the TWI interrupt for the data handling.

### 4.2 Motor (slave) register system

#### Available registers

We have these registers available:

- `ANGLE`- the angle motor should hold
- `ANG_CURR` - current angle measured from the feedback
- `STAT` - state of the motor (status codes listed later)
- `STAT_ERROR` - if the motor status flag indicates an error, it will be stored here
- `CONTROL` - the control register of the motor

The `ANGLE` (Read/Write) register stores a `float32` value of an angle the motor should hold at all times.

The `ANG_CURR` (Read only) register stores the value of current angle the motor holds. Value is determined from the angular encoder on the motor shaft.

The `STAT` (Read only) register stores the current state of the motor. State can have these values:

- `MOT_STAT_ERROR` - motor encountered an error, and stored the value into the `STAT_ERROR` register
- `MOT_STAT_OFF` - motor driver is not active
- `MOT_STAT_READY` - motor is ready for operation
- `MOT_STAT_MOVING` - motor is currently moving into the position

The `STAT_ERROR` (Read only) register store the error code if the `STAT` register has `MOT_STAT_ERROR` status code. Possible error values:

- `MOT_ERR_NONE` - no error
- `MOT_ERR_UNDEFINED` - error not defined in this list
- `MOT_ERR_POWER` - insufficient/low power
- `MOT_ERR_PID` - PID motor control error
- `MOT_ERR_DRIVER` - motor driver error
- `MOT_ERR_I2C` - I2C protocol error
- `MOT_ERR_I2C_INVALID_ADDR` - master tried to read/write to/from an invalid register address

Lastly, the `CONTROL` (Read/Write) register sets the control of the motor, using these bits (ON - 1/OFF - 0):

- `MOT_CR_POWER` - power the driver ON(1)/OFF(0)
- `MOT_CR_BREAK` - enable the motor driver active breaking
- `MOT_CR_DIR` - spin direction (counter-clockwise - 1/clockwise - 0)
- `MOT_CR_LED` - status LED indication
- `MOT_CR_UNSET` - unused bit

Bits that trigger some functionality (DEFAULT - 0/TRIGGER - 1):

- `MOT_CR_HOME` - set the current position as the home position (after the motor powers on, it will move to this position)
- `MOT_CR_CLEAR_ERR` - clear the `STAT_ERROR` register
- `MOT_CR_RESET` - reset the motor registers to default values

#### Low-level explanation

The register structure definition on the motor looks like this:

```c
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
```

It defines all of the required registers. But, how do we store the data in the memory? Instead of initializing the structure itself, we initialize an array with `sizeof(mot_reg_t_` and then do AVR-style definitions for the registers like so:

```c
volatile uint8_t mot_registers[sizeof(mot_reg_t)];

#define MOT_REG_SIZE sizeof(mot_registers)

#define _MOT_REGS ((volatile mot_reg_t *)mot_registers)

// Motor Angle Register
#define MOT_ANG (*(volatile float *)(_MOT_REGS->ANGLE))

// Motor Current Angle Register
#define MOT_CANG (*(volatile float *)(_MOT_REGS->ANG_CURR))

// Motor Status Register
#define MOT_SR _MOT_REGS->STAT[0]

// Motor Status Error Register
#define MOT_SER _MOT_REGS->STAT_ERROR[0]

// Motor Control Register
#define MOT_CR _MOT_REGS->CONTROL[0]
```


This might seem a bit confusing, like, "Why would we want this? Is it necessary?", well we just liked the idea of having our own AVR-styled registers, and a way to read/write to them just like you would with AVR library registers. And the best part is, it works! And it is really satisfying to look at the code afterwards.
