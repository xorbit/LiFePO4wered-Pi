/* 
 * LiFePO4wered/Pi data module
 * Copyright (C) 2015-2018 Patrick Van Oosterwijck
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
  LIFEPO4WERED_VAR(VBAT)                            \
  LIFEPO4WERED_VAR(VOUT)                            \
  LIFEPO4WERED_VAR(IOUT)                            \
  LIFEPO4WERED_VAR(VBAT_MIN)                        \
  LIFEPO4WERED_VAR(VBAT_SHDN)                       \
  LIFEPO4WERED_VAR(VBAT_BOOT)                       \
  LIFEPO4WERED_VAR(VOUT_MAX)                        \
  LIFEPO4WERED_VAR(VIN_THRESHOLD)                   \
  LIFEPO4WERED_VAR(IOUT_SHDN_THRESHOLD)             \
  LIFEPO4WERED_VAR(VOFFSET_ADC)                     \
  LIFEPO4WERED_VAR(VBAT_OFFSET)                     \
  LIFEPO4WERED_VAR(VOUT_OFFSET)                     \
  LIFEPO4WERED_VAR(VIN_OFFSET)                      \
  LIFEPO4WERED_VAR(IOUT_OFFSET)                     \
  LIFEPO4WERED_VAR(AUTO_BOOT)                       \
  LIFEPO4WERED_VAR(WAKE_TIME)                       \
  LIFEPO4WERED_VAR(SHDN_DELAY)                      \
  LIFEPO4WERED_VAR(AUTO_SHDN_TIME)                  \
  LIFEPO4WERED_VAR(PI_BOOT_TO)                      \
  LIFEPO4WERED_VAR(PI_SHDN_TO)                      \
  LIFEPO4WERED_VAR(RTC_TIME)                        \
  LIFEPO4WERED_VAR(RTC_WAKE_TIME)                   \
  LIFEPO4WERED_VAR(WATCHDOG_CFG)                    \
  LIFEPO4WERED_VAR(WATCHDOG_GRACE)                  \
  LIFEPO4WERED_VAR(WATCHDOG_TIMER)                  \
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

#define TOUCH_INACTIVE          0x00
#define TOUCH_START             0x03
#define TOUCH_STOP              0x0C
#define TOUCH_HELD              0x0F
#define TOUCH_ACTIVE_MASK       0x03
#define TOUCH_MASK              0x0F

/* LED states when Pi on */

#define LED_STATE_OFF           0x00
#define LED_STATE_ON            0x01
#define LED_STATE_PULSING       0x02
#define LED_STATE_FLASHING      0x03

/* Auto boot settings */

#define AUTO_BOOT_OFF           0x00
#define AUTO_BOOT_VBAT          0x01
#define AUTO_BOOT_VBAT_SMART    0x02
#define AUTO_BOOT_VIN           0x03
#define AUTO_BOOT_VIN_SMART     0x04
#define AUTO_BOOT_NO_VIN        0x05
#define AUTO_BOOT_NO_VIN_SMART  0x06

/* Watchdog settings */

#define WATCHDOG_OFF            0x00
#define WATCHDOG_ALERT          0x01
#define WATCHDOG_SHDN           0x02

/* Register access masks */

#define ACCESS_READ             0x01
#define ACCESS_WRITE            0x02


/* Determine if the specified variable can be accessed in the specified
 * manner (read, write or both) */

bool access_lifepo4wered(enum eLiFePO4weredVar var, uint8_t access_mask);

/* Read data from LiFePO4wered/Pi */

int32_t read_lifepo4wered(enum eLiFePO4weredVar);

/* Write data to LiFePO4wered/Pi */

int32_t write_lifepo4wered(enum eLiFePO4weredVar, int32_t value);


#endif
