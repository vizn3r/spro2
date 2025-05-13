#include "control.h"
#include "com.h"
#include <stdbool.h>

// For toggling so we don't write to the registers constantly
volatile bool POWER = 0;
volatile bool BREAK = 0;
volatile bool DIR = MOT_DIR_CW;
volatile bool LED = 0;

void check_cr() {
  if (MOT_CR & (1 << MOT_CR_POWER) && !POWER) {
    // Enable PWM driver output pins

    POWER = 1;
  } else if (!(MOT_CR & (1 << MOT_CR_POWER)) && POWER) {
    // Disable PWM driver output pins

    POWER = 0;
  }

  if (MOT_CR & (1 << MOT_CR_BREAK) && !BREAK) {
    // Enable break

    BREAK = 1;
  } else if (!(MOT_CR & (1 << MOT_CR_BREAK)) && BREAK) {
    // Disable break

    BREAK = 0;
  }

  DIR = MOT_CR & (1 << MOT_CR_DIR);
  LED = MOT_CR & (1 << MOT_CR_LED);

  if (MOT_CR & (1 << MOT_CR_HOME)) {
    // Home motors

    MOT_CR &= ~(1 << MOT_CR_HOME);
  }

  if (MOT_CR & (1 << MOT_CR_CLEAR_ERR)) {
    // Clear error code
    MOT_SER = MOT_ERR_NONE;
    MOT_SR = MOT_STAT_READY;

    MOT_CR &= ~(1 << MOT_CR_CLEAR_ERR);
  }

  if (MOT_CR & (1 << MOT_CR_RESET)) {
    // Reset

    MOT_CR &= ~(1 << MOT_CR_RESET);
  }
}
