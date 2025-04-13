#include "com.h"
#include "i2c.h"

void com_init() {
  // Power ON and status LED ON
  MOT_CR = (1 << MOT_CR_POWER) | (1 << MOT_CR_LED);

  // Init i2c
  i2c_slave_init(SLAVE_ADDR);
}
