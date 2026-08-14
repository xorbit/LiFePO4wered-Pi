/* 
 * LiFePO4wered/Pi data module
 * Copyright (C) 2015-2026 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#ifndef LIFEPO4WERED_DATA_H
#define LIFEPO4WERED_DATA_H

#include <stdint.h>
#include <stdbool.h>


/* Generate enumeration and corresponding strings of available
 * LiFePO4wered/Pi variables - Legacy definitions first */

#define FOREACH_LIFEPO4WERED_VAR_LEGACY(LIFEPO4WERED_VAR) \
  LIFEPO4WERED_VAR(I2C_REG_VER)                           \
  LIFEPO4WERED_VAR(I2C_ADDRESS)                           \
  LIFEPO4WERED_VAR(LED_STATE)                             \
  LIFEPO4WERED_VAR(TOUCH_STATE)                           \
  LIFEPO4WERED_VAR(TOUCH_CAP_CYCLES)                      \
  LIFEPO4WERED_VAR(TOUCH_THRESHOLD)                       \
  LIFEPO4WERED_VAR(TOUCH_HYSTERESIS)                      \
  LIFEPO4WERED_VAR(DCO_RSEL)                              \
  LIFEPO4WERED_VAR(DCO_DCOMOD)                            \
  LIFEPO4WERED_VAR(VIN)                                   \
  LIFEPO4WERED_VAR(VBAT)                                  \
  LIFEPO4WERED_VAR(VOUT)                                  \
  LIFEPO4WERED_VAR(IOUT)                                  \
  LIFEPO4WERED_VAR(VBAT_MIN)                              \
  LIFEPO4WERED_VAR(VBAT_SHDN)                             \
  LIFEPO4WERED_VAR(VBAT_BOOT)                             \
  LIFEPO4WERED_VAR(VOUT_MAX)                              \
  LIFEPO4WERED_VAR(VIN_THRESHOLD)                         \
  LIFEPO4WERED_VAR(IOUT_SHDN_THRESHOLD)                   \
  LIFEPO4WERED_VAR(VOFFSET_ADC)                           \
  LIFEPO4WERED_VAR(VBAT_OFFSET)                           \
  LIFEPO4WERED_VAR(VOUT_OFFSET)                           \
  LIFEPO4WERED_VAR(VIN_OFFSET)                            \
  LIFEPO4WERED_VAR(IOUT_OFFSET)                           \
  LIFEPO4WERED_VAR(AUTO_BOOT)                             \
  LIFEPO4WERED_VAR(WAKE_TIME)                             \
  LIFEPO4WERED_VAR(SHDN_DELAY)                            \
  LIFEPO4WERED_VAR(AUTO_SHDN_TIME)                        \
  LIFEPO4WERED_VAR(PI_BOOT_TO)                            \
  LIFEPO4WERED_VAR(PI_SHDN_TO)                            \
  LIFEPO4WERED_VAR(RTC_TIME)                              \
  LIFEPO4WERED_VAR(RTC_WAKE_TIME)                         \
  LIFEPO4WERED_VAR(WATCHDOG_CFG)                          \
  LIFEPO4WERED_VAR(WATCHDOG_GRACE)                        \
  LIFEPO4WERED_VAR(WATCHDOG_TIMER)                        \
  LIFEPO4WERED_VAR(PI_RUNNING)                            \
  LIFEPO4WERED_VAR(CFG_WRITE)                             \

/* Generate enumeration and corresponding strings of available
 * LiFePO4wered variables - Modern devices */

#define FOREACH_LIFEPO4WERED_VAR(LIFEPO4WERED_VAR)        \
  LIFEPO4WERED_VAR(REG_VERSION)                           \
  LIFEPO4WERED_VAR(FW_VERSION)                            \
  LIFEPO4WERED_VAR(I2C_ADDRESS)                           \
  LIFEPO4WERED_VAR(LED_STATE_LOAD_ON)                     \
  LIFEPO4WERED_VAR(BTN_STATE)                             \
  LIFEPO4WERED_VAR(V_REF)                                 \
  LIFEPO4WERED_VAR(V_IN)                                  \
  LIFEPO4WERED_VAR(V_CELL_BOT)                            \
  LIFEPO4WERED_VAR(V_CELL_TOP)                            \
  LIFEPO4WERED_VAR(V_BAT)                                 \
  LIFEPO4WERED_VAR(V_SUP)                                 \
  LIFEPO4WERED_VAR(V_OUT)                                 \
  LIFEPO4WERED_VAR(I_IN)                                  \
  LIFEPO4WERED_VAR(I_BAT)                                 \
  LIFEPO4WERED_VAR(I_OUT)                                 \
  LIFEPO4WERED_VAR(TEMP_INT)                              \
  LIFEPO4WERED_VAR(R_THRM)                                \
  LIFEPO4WERED_VAR(TEMP_THRM)                             \
  LIFEPO4WERED_VAR(R25_THRM)                              \
  LIFEPO4WERED_VAR(B_THRM)                                \
  LIFEPO4WERED_VAR(TEMP_SEL)                              \
  LIFEPO4WERED_VAR(TEMP)                                  \
  LIFEPO4WERED_VAR(V_IN_OC)                               \
  LIFEPO4WERED_VAR(I_CHRG_MAX)                            \
  LIFEPO4WERED_VAR(I_CHRG_BAL)                            \
  LIFEPO4WERED_VAR(I_CHRG_UNDERTEMP)                      \
  LIFEPO4WERED_VAR(I_PRECHRG_PCT)                         \
  LIFEPO4WERED_VAR(I_CHRG_TERM_PCT)                       \
  LIFEPO4WERED_VAR(V_CELL_MIN)                            \
  LIFEPO4WERED_VAR(V_CELL_SHDN)                           \
  LIFEPO4WERED_VAR(V_CELL_BOOT)                           \
  LIFEPO4WERED_VAR(V_CELL_MAX)                            \
  LIFEPO4WERED_VAR(V_BAT_MIN)                             \
  LIFEPO4WERED_VAR(V_BAT_SHDN)                            \
  LIFEPO4WERED_VAR(V_BAT_BOOT)                            \
  LIFEPO4WERED_VAR(V_BAT_MAX)                             \
  LIFEPO4WERED_VAR(V_BAT_CV_PCT)                          \
  LIFEPO4WERED_VAR(V_BAT_CHRG_PCT)                        \
  LIFEPO4WERED_VAR(V_BAT_PRECHRG_PCT)                     \
  LIFEPO4WERED_VAR(V_IN_DROPOUT)                          \
  LIFEPO4WERED_VAR(V_IN_MIN_CHRG)                         \
  LIFEPO4WERED_VAR(P_IN_MAX)                              \
  LIFEPO4WERED_VAR(I_IN_MAX)                              \
  LIFEPO4WERED_VAR(I_IN_LIMIT)                            \
  LIFEPO4WERED_VAR(V_IN_OC_PCT)                           \
  LIFEPO4WERED_VAR(V_IN_MPP)                              \
  LIFEPO4WERED_VAR(V_IN_MPP_STEP)                         \
  LIFEPO4WERED_VAR(I_MIN_TRUST)                           \
  LIFEPO4WERED_VAR(I_OK_TRUST)                            \
  LIFEPO4WERED_VAR(CHRG_PID_KP)                           \
  LIFEPO4WERED_VAR(CHRG_PID_KI)                           \
  LIFEPO4WERED_VAR(CHRG_PID_KD)                           \
  LIFEPO4WERED_VAR(CHRG_TEMP_MIN)                         \
  LIFEPO4WERED_VAR(CHRG_TEMP_COOL)                        \
  LIFEPO4WERED_VAR(CHRG_TEMP_WARM)                        \
  LIFEPO4WERED_VAR(CHRG_TEMP_MAX)                         \
  LIFEPO4WERED_VAR(I_CHRG_COOL_PCT)                       \
  LIFEPO4WERED_VAR(I_CHRG_WARM_PCT)                       \
  LIFEPO4WERED_VAR(V_BAT_WARM_PCT)                        \
  LIFEPO4WERED_VAR(CHRG_STATE)                            \
  LIFEPO4WERED_VAR(HEATER_SEL)                            \
  LIFEPO4WERED_VAR(EXT_HEAT_MAX_PWM)                      \
  LIFEPO4WERED_VAR(LOAD_USB_PD)                           \
  LIFEPO4WERED_VAR(INPUT_USB_PD)                          \
  LIFEPO4WERED_VAR(V_OUT_MAX)                             \
  LIFEPO4WERED_VAR(V_IN_THRESHOLD)                        \
  LIFEPO4WERED_VAR(I_OUT_ON_BOOT)                         \
  LIFEPO4WERED_VAR(I_OUT_SHDN_DONE)                       \
  LIFEPO4WERED_VAR(BOOT_TIMEOUT)                          \
  LIFEPO4WERED_VAR(SHDN_TIMEOUT)                          \
  LIFEPO4WERED_VAR(RTC_TIME)                              \
  LIFEPO4WERED_VAR(RTC_WAKE_TIME)                         \
  LIFEPO4WERED_VAR(AUTO_BOOT)                             \
  LIFEPO4WERED_VAR(SHDN_DELAY)                            \
  LIFEPO4WERED_VAR(AUTO_SHDN_TIME)                        \
  LIFEPO4WERED_VAR(WATCHDOG_CFG)                          \
  LIFEPO4WERED_VAR(WATCHDOG_GRACE)                        \
  LIFEPO4WERED_VAR(WATCHDOG_TIMER)                        \
  LIFEPO4WERED_VAR(LOAD_RUNNING)                          \
  LIFEPO4WERED_VAR(STBY_WAKE_PERIOD)                      \
  LIFEPO4WERED_VAR(BAT_SOC_PCT)                           \
  LIFEPO4WERED_VAR(BAT_CAPACITY)                          \
  LIFEPO4WERED_VAR(BAT_CAPACITY_DET)                      \
  LIFEPO4WERED_VAR(BAT_LOAD_PRESET)                       \
  LIFEPO4WERED_VAR(CFG_WRITE)                             \

enum eLiFePO4weredVar {
#define GENERATE_ENUM(ENUM) ENUM##_LEGACY,
  FOREACH_LIFEPO4WERED_VAR_LEGACY(GENERATE_ENUM)
#undef GENERATE_ENUM
#define GENERATE_ENUM(ENUM) ENUM,
  FOREACH_LIFEPO4WERED_VAR(GENERATE_ENUM)
  LFP_VAR_COUNT,
  LFP_VAR_INVALID = LFP_VAR_COUNT,
  LFP_VAR_COUNT_LEGACY = REG_VERSION,
  LFP_VAR_FIRST_NEW = REG_VERSION
};

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
#define LED_STATE_TOUCH         0x04
#define LED_STATE_PULSE_UP      0x05

/* Auto boot settings */

#define AUTO_BOOT_OFF           0x00
#define AUTO_BOOT_VBAT          0x01
#define AUTO_BOOT_VBAT_SMART    0x02
#define AUTO_BOOT_VIN           0x03
#define AUTO_BOOT_VIN_SMART     0x04
#define AUTO_BOOT_NO_VIN        0x05
#define AUTO_BOOT_NO_VIN_SMART  0x06
#define AUTO_BOOT_VIN_SMART2    0x07

/* Watchdog settings */

#define WATCHDOG_OFF            0x00
#define WATCHDOG_ALERT          0x01
#define WATCHDOG_SHDN           0x02

/* Register access masks */

#define ACCESS_READ             0x01
#define ACCESS_WRITE            0x02


/* Get the variable from a string representation */

enum eLiFePO4weredVar get_lifepo4wered_var_from_string(char *var);

/* Get the string representation of a variable */

const char *get_lifepo4wered_var_name(enum eLiFePO4weredVar var);

/* Determine if the specified variable can be accessed in the specified
 * manner (read, write or both) */

bool access_lifepo4wered(enum eLiFePO4weredVar var, uint8_t access_mask);

/* Read data from LiFePO4wered/Pi */

int32_t read_lifepo4wered(enum eLiFePO4weredVar);

/* Write data to LiFePO4wered/Pi */

int32_t write_lifepo4wered(enum eLiFePO4weredVar, int32_t value);


#endif
