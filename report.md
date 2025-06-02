<div align="center">
  <h1>Report</h1>
  <p>Semester Project 2 - BEng in Mechatronics</p>
  <p>Group 1: Simon Vizner</p>
  <p>30/05/2025</p>
</div>

# Introduction

This report covers the software and electronics part of the project, divided into:
- initial ideas and thought process while choosing our approach,
- our implementation,
- challenges or problems we stumbled upon.

# Initial ideas

## Our task

Our task was to choose the proper electronics and components for this project, as well as develop software for control of the motor and communication between the various electrical components.

## Researching servo motors

So, before choosing the components, we decided to choose our platform for the motor, then we started to brainstorm about the implementation of our servo motor and overall electronics and software systems.

We started researching on servo motors, how they work and what types of servo motors there are. We found out there are many types of servo motors used for different purposes and applications, generally divided into three main categories:
- angle control,
- velocity (RPM) control,
- combination of both.

Servo motors are by definition a closed-loop system, meaning that they manage the motor driving and feedback in one control loop, and get instructions from an external controller via a communication protocol to control the angle, velocity or both.

They can be categorized by the internal motor type they are using:
- direct current (DC) motors,
- alternating current (AC) motors.

We can categorize DC motors into sub-categories:
1. Brushed
- uses brushes and a commutator for switching the magnetic field

2. Brushless
- electronic commutation using sensors and sensorless techniques

Same with AC motors:
1. Synchornous
- rotor rotates at the same speed as the stator's magnetic field
- requires additional excitation - permanent magnets or DC supply

2. Asynchronous
- most commonly used AC motor type
- rotor speed is slightly less than the stator's magnetic field - it slips
- sub-categories: 1 phase induction motor, 3 phase induction motor

The most used motor types are brushless DC motors, AC synchronous motors and brushed DC motors.

Servo motors use encoders for real-time feedback to control the motor precisely. Common encoders for the servo motor systems are:
- optical encoders,
- resistive encoders (potentiometers),
- Hall effect encoders.

Each of the encoders is better has their pros and cons, so we had to decide wisely.

## The servo motor requirements

Our project requires angle precise servo motor driven by a motor with small form factor since we aimed for the servo motor system to be as compact as possible, while being appropriately dimensioned for the requirements.

For controlling our servo motor, we had to choose an appropriate MCU, that would be able to get the data from the encoder, process them, then drive the motor appropriately so. Another requirement for our servo motor MCU was that it had to support some kind of commonly used communication protocol, such as I2C(TWI), SPI or UART.

We aimed for angular precision that would be sufficient enough for the clock to move precisely enough.

## Nextion display

We had to use the Nextion display, so we decided to use it to show the current weather, temperature and use it for configuration and calibration of the clock.

## Alarm clock functionality

We wanted to add an alarm clock functionality, for that we need a component that can generate sound. For that we had two options:
- speaker,
- piezo buzzer.

Since speaker would require some kind of driver and possibly an external FLASH memory for storing the sounds, we crossed that option out.

That left us with a piezo buzzer, which can be easily controlled with a PWM signal.

## The main controller requirements

For the main controller we needed WiFi capability for fetching real-time weather data and real-time clock from internet.

Also, the main controller MCU had to support the same communication protocol as the servo motor MCU, and UART protocol for the Nextion display.

Then lastly, it had to have PWM capable GPIO pins for driving the piezo buzzer.

This left us with two approaches:
- a microcontroller with external WiFi module,
- a microcontroller with built-in WiFi module.


# Our approach to implementation

## Servo motor

### Motor

We chose a DC motor with pre-installed gear reduction, that reduces the high RPM of the motor to a more manageable, lower RPM, so we could get more precise angle without worrying that much about precise timing and control algorithm tuning.

### Encoder

For the feedback encoder we have chosen the Hall effect encoders, since they can be precise when done right, reasonably simple to implement and the most important of all, really compact and require only three components:
- two Hall effect sensors with offset of 90 degrees,
- a magnet on the shaft.

With the combination of some simple trigonometrics and embedded code, we managed to get really decent angle measurements.

**HALL EFFECT SENSORS IN MAGNUS'S REPORT**

### Motor driving

Driving the motor was pretty straight forward, using a dual H-bridge integrated circuit, connecting it to the motor and our microcontroller.

We are using a PID controller to control the required PWM for the driver signal.

There are three main ways of tuning a PID controller:
- manually,
- the Cohen-Coon method,
- the Zieger-Nichols method.

We tried all three of them, starting with the manual method. We sat in the lab, tuning the three PID gains, trying to get it to be as smooth and precise as possible, but this approach was very time consuming and experiment driven, so we looked at other options.

Then we tried the Cohen-Coon method. It introduces three new variables, the process gain, time constant and dead time. We measure these three variables, then use them in the Cohen-Coon formulas and calculate the PID gains. This method works the best for position control and systems with delay and inertia. Our system didn't have that much delay or inertia, so after testing and experimenting with this approach, we decided to use the final approach.

The Zieger-Nichols method. It uses two new variables, the ultimate gain and the ultimate period. This method uses experimental way of getting values for these variables. So, first we need to find the Ku - the ultimate gain. We disable all PID, then make Kp equal the Ku and increase Ku until the system oscillates periodically. That will give us the final Ku. The ultimate period - Tu, is just the period of the oscillation, so we can also easily measure that.

With the Ku and Tu, we can choose from bunch of different formulas, we went with the one that has no overshoot:

`Kp = 0.2 * Ku`
`Ki = (0.4 * Ku) / Tu`
`Kd = 0.66 * Ku * Tu`

These parameters give us the smoothest PID gains and worked the best for our use case.

### Motor microcontroller

# *Magnus wrote about this in his report*

### Internal communication protocol

This was not that hard of a task to choose, since our microcontrollers only support these protocols:

- UART
- I2C (TWI)
- SPI

Since we need to have a way to communicate with more than one microcontroller, we automatically crossed out UART, since it only supports duplex communication between two devices.

Then, we had to choose between I2C or SPI. Here, it also was easy to cross out SPI, since we need a way to communicate with the devices with as few wires as possible.

So, our obvious choice is I2C. I2C (or TWI in AVR documentation) requires only two wires (if we don’t count in the ground wire) for communication between multiple devices, by use of device addresses (something like IP addresses) and master-slave configuration.

In our use case, the main controller is the master, and the motors are the slaves.

### Data transfer

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

### Motor (slave) register system

We have these registers available:

- `ANGLE` - the angle motor should hold
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

### Low-level explanation

The register structure definition of the motor looks like this:

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

This might seem a bit confusing, like, "Why would we want this? Is it necessary?", well we just liked the idea of having our own AVR-styled registers, and a way to read/write to them just like you would 
with AVR library registers. And the best part is, it works! And it is really satisfying to look at the code afterwards.

## Main controller

### Microcontroller

We went with previously stated 2nd option, a microcontroller with built-in WiFi module, specifically the ESP32C6 MCU on the Seeed XIAO ESP32C6 development board.

The ESP32C6 also has a feature called "Pin Multiplexing", which means we can assign the I2C / UART / PWM pins to any GPIO with this capability.

### I2C protocol

The master's I2C protocol was programmed so it matches the slave communication definition and functionality. We can fully control the servo motor using high-level functions in our code, thanks to a library we wrote for that purpose.

### Real-time weather and time

For real-time data, we needed the WiFi capability, which our main microcontroller supports. Our approach is to scan the WiFi signals around, list them out for the user on the Nextion display, the user selects the desired WiFi network, enters a password and the main controller automatically connects and stores the network's credentials in EEPROM.

We are using the `weatherapi.com` API for fetching the real-time weather in the area which user also defines using the Nextion display, in the same fashion as the WiFi credentials. Based on this weather data, we display an appropriate weather icon on the Nextion display, together with additional weather data.

For the real-time clock, we are using the SNTP (Simple Network Time Protocol) for precise time and automatic synchronization. We get the time zone based on the user's previous location input and display the correct time.

### The software flow

To put everything together, here's how everything works together.

1. Calibration and configuration

When the user starts the clock for the first time, it will ask for WiFi credentials and location for correct data fetching.
Then it will ask the user to calibrate the flaps on the display using controls on the display. The display should initially show all zeroes like this: 00:00
When this step is done, the clock will automatically determine the time it has to display and order the motors to spin appropriately so.

2. Clock is running

The main controller will periodically fetch the real-time data from external APIs, update the data on the Nextion display and the flaps, to make sure the time is always correct.
User can also reconfigure or recalibrate the clock any time using the Nextion display.
User can set an alarm that will set off when the current time reaches the set time, setting off the piezo buzzer and displaying a snooze button on the Nextion display.

3. Power off

When the clock powers off, it loses track of the flap positions, so after power-up, the workflow will reset to the first step, where the user has to recalibrate the flap positions back to 00:00.

# Challenges and problems

## Insufficient DC motor power output

After constructing the gearboxes and flapping mechanisms, we found out our motors are not properly suited for precise operation of our clock. We mitigated some of the issues programmatically and by tuning down the PIDs.

## Building the binaries for main controller

When we fist got our ESP32C6 development board, we tested it out with simple LED blink program, but it didn't work. We are using PlatformIO, so we tried to use different board definitions, but we had no success. Then after investigating further, reading through long verbose logs, we found out that the ESP-IDF tool was building for 8MB flash, but our MCU has a 4MB flash, so the resulting binary was not compatible. After this discovery, we made our own board definition, so it builds the binary for 4MB.

## I2C testing

Testing our custom application level, register protocol took us a lot of time, since debugging I2C interfaces without proper logic analyzers or tools is painstaking. First, we thought the I2C speeds were just off, wrongly configured, but after checking, that was not the issue. The next issue could be an address mismatch, so we made sure that everything in our slave's and master's I2C code calculated the address properly. We found out that the calculations were proper, but the addresses we assigned to the slave were not. Since the addresses have to be 7 bit in the I2C communication, we had limited range of addresses we could use, and that was our mistake. We just blindly assigned some random value to the slave address, and it turned out it was out of the 7 bit range. After changing the address, the communication worked properly and we didn't have any other issues.

## Tuning PIDs

We mentioned that we tried using the Cohen-Coon method. That was one of our mistakes, because after reading more about this method, we realized it might not be the proper approach for our PID tuning, so we just used the Zieger-Nichols method instead.

## Initial main controller not working

Initially we went with a ESP32C3 development board, but after testing we thought it was faulty, so we ordered the new ESP32C6 development board were are using now. But after encountering the same issue with the new board, we realized that the ESP32C3 board had the same FLASH size issue, hence rendering the fault as our user error.

## Power delivery to the components

We have a complete 3.3V system, excluding the Nextion display. We thought that the on-board regulator on the ESP32C6 development board would be sufficient, but after calculating the peak current, we assumed we had a need for a more proper solution, so we went with a 3.3V step-down buck converter, that is sufficient enough for our peak currents.

## Buzzer was too loud

When we unpacked the buzzer and tested it with the 3.3V, it was really loud. It sounded more like a fire alarm than a alarm clock, so we just put a piece of tape over the sound hole that dampened the sound sufficiently, resulting in pretty pleasant beeping sound.
