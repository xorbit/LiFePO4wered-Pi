/* 
 * LiFePO4wered/Pi data module
 * Copyright (C) 2015 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#ifndef LIFEPO4WERED_DATA_H
#define LIFEPO4WERED_DATA_H

#include <stdint.h>
#include <stdbool.h>


/* Generate enumeration and corresponding strings of available
 * LiFePO4wered/Pi variables */

#define FOREACH_LIFEPO4WERED_VAR(LIFEPO4WERED_VAR)  \
  LIFEPO4WERED_VAR(I2C_REG_VER)                     \
  LIFEPO4WERED_VAR(I2C_ADDRESS)                     \
  LIFEPO4WERED_VAR(LED_STATE)                       \
  LIFEPO4WERED_VAR(TOUCH_STATE)                     \
  LIFEPO4WERED_VAR(TOUCH_CAP_CYCLES)                \
  LIFEPO4WERED_VAR(TOUCH_THRESHOLD)                 \
  LIFEPO4WERED_VAR(TOUCH_HYSTERESIS)                \
  LIFEPO4WERED_VAR(DCO_RSEL)                        \
  LIFEPO4WERED_VAR(DCO_DCOMOD)                      \
  LIFEPO4WERED_VAR(VIN)                             \
  LIFEPO4WERED_VAR(VOUT)                            \
  LIFEPO4WERED_VAR(VIN_MIN)                         \
  LIFEPO4WERED_VAR(VIN_SHDN)                        \
  LIFEPO4WERED_VAR(VIN_BOOT)                        \
  LIFEPO4WERED_VAR(VOUT_MAX)                        \
  LIFEPO4WERED_VAR(AUTO_BOOT)                       \
  LIFEPO4WERED_VAR(WAKE_TIME)                       \
  LIFEPO4WERED_VAR(PI_RUNNING)                      \
  LIFEPO4WERED_VAR(CFG_WRITE)                       \

#define GENERATE_ENUM(ENUM) ENUM,

enum eLiFePO4weredVar {
  FOREACH_LIFEPO4WERED_VAR(GENERATE_ENUM)
  LFP_VAR_COUNT,
  LFP_VAR_INVALID = LFP_VAR_COUNT
};

extern const char *lifepo4wered_var_name[LFP_VAR_COUNT];

/* Touch states and masks */

#define TOUCH_INACTIVE      0x00
#define TOUCH_START         0x03
#define TOUCH_STOP          0x0C
#define TOUCH_HELD          0x0F
#define TOUCH_ACTIVE_MASK   0x03
#define TOUCH_MASK          0x0F

/* LED states when Pi on */

#define LED_STATE_OFF       0x00
#define LED_STATE_ON        0x01
#define LED_STATE_PULSING   0x02
#define LED_STATE_FLASHING  0x03


/* Read data from LiFePO4wered/Pi */

int32_t read_lifepo4wered(enum eLiFePO4weredVar);

/* Write data to LiFePO4wered/Pi */

void write_lifepo4wered(enum eLiFePO4weredVar, int32_t value);


#endif
