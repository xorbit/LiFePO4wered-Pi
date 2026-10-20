/* 
 * LiFePO4wered/Pi data module
 * Copyright (C) 2015-2026 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#define _DEFAULT_SOURCE
#include <endian.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "lifepo4wered-data.h"
#include "lifepo4wered-access.h"
#include <stdio.h>


/* Number of register versions defined */

#define I2C_REG_VER_COUNT_LEGACY  7
#define REG_VER_NEW_START         10
#define REG_VER_COUNT             1

/* Minimum I2C register version that requires write unlock */

#define I2C_WRUNLOCK_REG_VER      5

/* Constant to use when a register is not available in a particular
 * register version */

#define R_NA                      0xFF

/* I2C access retries */

#define I2C_RETRIES               20

/* I2C identical reads requirement */

#define I2C_IDENTICAL_READS       3

/* I2C retry delay in us */

#define I2C_RETRY_DELAY           500

/* Generate strings for variable names */

#define GENERATE_LEGACY_STRING_1(ENUM)        #ENUM,
#define GENERATE_LEGACY_STRING_2(ENUM, LNAME) #LNAME,
#define GENERATE_LEGACY_STRING_X(x, A, B, FUNC, ...) FUNC
#define GENERATE_LEGACY_STRING(...)                       \
      GENERATE_LEGACY_STRING_X(,##__VA_ARGS__,            \
          GENERATE_LEGACY_STRING_2(__VA_ARGS__),          \
          GENERATE_LEGACY_STRING_1(__VA_ARGS__),          \
          GENERATE_LEGACY_STRING_0(__VA_ARGS__)           \
      )

const char *lifepo4wered_var_name_legacy[LFP_VAR_COUNT] = {
  FOREACH_LIFEPO4WERED_VAR(GENERATE_LEGACY_STRING)
};

#define GENERATE_STRING_1(ENUM)               #ENUM,
#define GENERATE_STRING_2(ENUM, LNAME)        #ENUM,
#define GENERATE_STRING_X(x, A, B, FUNC, ...) FUNC
#define GENERATE_STRING(...)                              \
      GENERATE_STRING_X(,##__VA_ARGS__,                   \
          GENERATE_STRING_2(__VA_ARGS__),                 \
          GENERATE_STRING_1(__VA_ARGS__),                 \
          GENERATE_STRING_0(__VA_ARGS__)                  \
      )

const char *lifepo4wered_var_name[LFP_VAR_COUNT] = {
  FOREACH_LIFEPO4WERED_VAR(GENERATE_STRING)
};

/* Structure to define variable scale factors */

struct sVarScale {
  int32_t       mul;
  int32_t       div;
};

/* Structure to define variable read and write behavior */

struct sVarDefLegacy {
  uint8_t       reg[I2C_REG_VER_COUNT_LEGACY];
  uint8_t       read_bytes;
  uint8_t       write_bytes;
  uint8_t       sign_extend;
  uint8_t       scale_idx;
};

struct sVarDef {
  uint8_t       reg[REG_VER_COUNT];
  uint8_t       read_bytes;
  uint8_t       write_bytes;
  uint8_t       sign_extend;
};

struct sVarDefGeneric {
  uint8_t       reg;
  uint8_t       read_bytes;
  uint8_t       write_bytes;
  uint8_t       sign_extend;
  int32_t       scale_mul;
  int32_t       scale_div;
};

/* This table defines scaling for legacy I2C registers, for different
 * scaling variants */

static const struct sVarScale var_scale[][3] = {
  /* No scaling       */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* VIN              */  { { 966667, 102300 }, { 120833, 102300 }, { 317154, 102300 } },
  /* VBAT             */  { {   5000,   1023 }, {    625,   1023 }, {    625,   1023 } },
  /* VOUT             */  { { 554878, 102300 }, {  69360, 102300 }, {  65705, 102300 } },
  /* IOUT             */  { { 581395, 102300 }, {  72674, 102300 }, {  72674, 102300 } },
  /* VBAT_MIN         */  { {   5000,   1023 }, {    625,   1023 }, {    625,   1023 } },
  /* VBAT_SHDN        */  { {   5000,   1023 }, {    625,   1023 }, {    625,   1023 } },
  /* VBAT_BOOT        */  { {   5000,   1023 }, {    625,   1023 }, {    625,   1023 } },
  /* VOUT_MAX         */  { { 554878, 102300 }, {  69360, 102300 }, {  65705, 102300 } },
  /* VIN_THRESHOLD    */  { { 966667, 102300 }, { 120833, 102300 }, { 317154, 102300 } },
  /* IOUT_SHDN_THRESH */  { { 581395, 102300 }, {  72674, 102300 }, {  72674, 102300 } },
  /* VOFFSET_ADC      */  { {   5000,   1023 }, {   5000,   1023 }, {   5000,   1023 } },
  /* VBAT_OFFSET      */  { {   5000,   1023 }, {   5000,   1023 }, {   5000,   1023 } },
  /* VOUT_OFFSET      */  { { 554878, 102300 }, { 554878, 102300 }, { 525641, 102300 } },
  /* VIN_OFFSET       */  { { 966667, 102300 }, { 966667, 102300 }, { 253723,  10230 } },
  /* IOUT_OFFSET      */  { { 581395, 102300 }, { 581395, 102300 }, { 581395, 102300 } },
  /* PI_BOOT_TO       */  { {     10,      1 }, {     10,      1 }, {     10,      1 } },
  /* PI_SHDN_TO       */  { {     10,      1 }, {     10,      1 }, {     10,      1 } },
  /* WATCHDOG_GRACE   */  { {     10,      1 }, {     10,      1 }, {     10,      1 } },
  /* WATCHDOG_TIMER   */  { {     10,      1 }, {     10,      1 }, {     10,      1 } },
};

/* This table covers I2C register scale variants used by each register
 * version */

static const uint8_t var_scale_variant[I2C_REG_VER_COUNT_LEGACY] = {
  0, 0, 0, 0, 1, 1, 2
};

/* This table covers definitions of all I2C registers */

static const struct sVarDefLegacy var_table_legacy[LFP_VAR_COUNT] = {
  /* I2C_REG_VER        */  { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 1, 0, 0,  0 },
  /* FW_VERSION         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I2C_ADDRESS        */  { { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 }, 1, 1, 0,  0 },
  /* LED_STATE          */  { { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 }, 1, 1, 0,  0 },
  /* TOUCH_STATE        */  { { 0x19, 0x1B, 0x1D, 0x23, 0x22, 0x28, 0x3A }, 1, 0, 0,  0 },
  /* TOUCH_CAP_CYCLES   */  { { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 }, 1, 1, 0,  0 },
  /* TOUCH_THRESHOLD    */  { { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }, 1, 1, 0,  0 },
  /* TOUCH_HYSTERESIS   */  { { 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05 }, 1, 1, 0,  0 },
  /* DCO_RSEL           */  { { 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06 }, 1, 1, 0,  0 },
  /* DCO_DCOMOD         */  { { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 }, 1, 1, 0,  0 },
  /* V_REF              */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* VIN                */  { { R_NA, R_NA, R_NA, 0x21, R_NA, 0x26, 0x36 }, 2, 0, 1,  1 },
  /* V_CELL_BOT         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_CELL_BOT         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* VBAT               */  { { 0x15, 0x17, 0x19, 0x1D, 0x1E, 0x22, 0x32 }, 2, 0, 1,  2 },
  /* V_SUP              */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* VOUT               */  { { 0x17, 0x19, 0x1B, 0x1F, 0x20, 0x24, 0x34 }, 2, 0, 1,  3 },
  /* I_IN               */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_BAT              */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* IOUT               */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x38 }, 2, 0, 1,  4 },
  /* TEMP_INT           */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* R_THRM             */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* TEMP_THRM          */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* R25_THRM           */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* B_THRM             */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* TEMP_SEL           */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* TEMP               */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_IN_OC            */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_CHRG_MAX         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_CHRG_BAL         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_CHRG_UNDERTEMP   */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_PRECHRG_PCT      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_CHRG_TERM_PCT    */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_CELL_MIN         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_CELL_SHDN        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_CELL_BOOT        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_CELL_MAX         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* VBAT_MIN           */  { { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 }, 2, 2, 1,  5 },
  /* VBAT_SHDN          */  { { 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A }, 2, 2, 1,  6 },
  /* VBAT_BOOT          */  { { 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C }, 2, 2, 1,  7 },
  /* V_BAT_MAX          */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_BAT_CV_PCT       */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_BAT_CHRG_PCT     */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_BAT_PRECHRG_PCT  */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_IN_DROPOUT       */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_IN_MIN_CHRG      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* P_IN_MAX           */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_IN_MAX           */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_IN_LIMIT         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_IN_OC_PCT        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_IN_MPP           */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_IN_MPP_STEP      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_MIN_TRUST        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_OK_TRUST         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CHRG_PID_KP        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CHRG_PID_KI        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CHRG_PID_KD        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CHRG_TEMP_MIN      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CHRG_TEMP_COOL     */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CHRG_TEMP_WARM     */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CHRG_TEMP_MAX      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_CHRG_COOL_PCT    */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* I_CHRG_WARM_PCT    */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* V_BAT_WARM_PCT     */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CHRG_STATE         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* HEATER_SEL         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* EXT_HEAT_MAX_PWM   */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* LOAD_USB_PD        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* INPUT_USB_PD       */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* VOUT_MAX           */  { { 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E }, 2, 2, 1,  8 },
  /* VIN_THRESHOLD      */  { { R_NA, R_NA, R_NA, 0x10, R_NA, 0x10, 0x10 }, 2, 2, 1,  9 },
  /* I_OUT_ON_BOOT      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* IOUT_SHDN_THRESHOLD*/  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x1A }, 2, 2, 1, 10 },
  /* VOFFSET_ADC        */  { { R_NA, R_NA, 0x10, 0x12, 0x10, 0x12, R_NA }, 2, 2, 1, 11 },
  /* VBAT_OFFSET        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x12 }, 2, 2, 1, 12 },
  /* VOUT_OFFSET        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x14 }, 2, 2, 1, 13 },
  /* VIN_OFFSET         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x16 }, 2, 2, 1, 14 },
  /* IOUT_OFFSET        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x18 }, 2, 2, 1, 15 },
  /* PI_BOOT_TO         */  { { R_NA, R_NA, R_NA, R_NA, 0x15, 0x19, 0x21 }, 1, 1, 0, 16 },
  /* PI_SHDN_TO         */  { { R_NA, R_NA, R_NA, R_NA, 0x16, 0x1A, 0x22 }, 1, 1, 0, 17 },
  /* RTC_TIME           */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x28 }, 4, 4, 0,  0 },
  /* RTC_WAKE_TIME      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x2C }, 4, 4, 0,  0 },
  /* AUTO_BOOT          */  { { 0x10, 0x12, 0x14, 0x18, 0x14, 0x18, 0x20 }, 1, 1, 0,  0 },
  /* WAKE_TIME          */  { { 0x12, 0x14, 0x16, 0x1A, 0x1A, 0x1E, 0x26 }, 2, 2, 0,  0 },
  /* SHDN_DELAY         */  { { R_NA, 0x10, 0x12, 0x14, 0x12, 0x14, 0x1C }, 2, 2, 0,  0 },
  /* AUTO_SHDN_TIME     */  { { R_NA, R_NA, R_NA, 0x16, R_NA, 0x16, 0x1E }, 2, 2, 0,  0 },
  /* WATCHDOG_CFG       */  { { R_NA, R_NA, R_NA, R_NA, 0x17, 0x1B, 0x23 }, 1, 1, 0,  0 },
  /* WATCHDOG_GRACE     */  { { R_NA, R_NA, R_NA, R_NA, 0x18, 0x1C, 0x24 }, 1, 1, 0, 18 },
  /* WATCHDOG_TIMER     */  { { R_NA, R_NA, R_NA, R_NA, 0x1C, 0x20, 0x30 }, 1, 1, 0, 19 },
  /* PI_RUNNING         */  { { 0x14, 0x16, 0x18, 0x1C, 0x1D, 0x21, 0x31 }, 1, 1, 0,  0 },
  /* STBY_WAKE_PERIOD   */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* BAT_SOC_PCT        */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* BAT_CAPACITY       */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* BAT_CAPACITY_DET   */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* BAT_LOAD_PRESET    */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, R_NA }, 0, 0, 0,  0 },
  /* CFG_WRITE          */  { { 0x11, 0x13, 0x15, 0x19, 0x19, 0x1D, 0x25 }, 1, 1, 0,  0 },
};

static const struct sVarDef var_table[LFP_VAR_COUNT] = {
  /* REG_VERSION        */  { { 0x00 }, 1, 0, 0 },
  /* FW_VERSION         */  { { 0x01 }, 1, 0, 0 },
  /* I2C_ADDRESS        */  { { 0x02 }, 1, 1, 0 },
  /* LED_STATE_LOAD_ON  */  { { 0x03 }, 1, 1, 0 },
  /* BTN_STATE          */  { { 0x04 }, 1, 0, 0 },
  /* TOUCH_CAP_CYCLES   */  { { R_NA }, 0, 0, 0 },
  /* TOUCH_THRESHOLD    */  { { R_NA }, 0, 0, 0 },
  /* TOUCH_HYSTERESIS   */  { { R_NA }, 0, 0, 0 },
  /* DCO_RSEL           */  { { R_NA }, 0, 0, 0 },
  /* DCO_DCOMOD         */  { { R_NA }, 0, 0, 0 },
  /* V_REF              */  { { 0x05 }, 2, 2, 1 },
  /* V_IN               */  { { 0x07 }, 2, 0, 1 },
  /* V_CELL_BOT         */  { { 0x09 }, 2, 0, 1 },
  /* V_CELL_TOP         */  { { 0x0B }, 2, 0, 1 },
  /* V_BAT              */  { { 0x0D }, 2, 0, 1 },
  /* V_SUP              */  { { 0x0F }, 2, 0, 1 },
  /* V_OUT              */  { { 0x11 }, 2, 0, 1 },
  /* I_IN               */  { { 0x13 }, 2, 0, 1 },
  /* I_BAT              */  { { 0x15 }, 2, 0, 1 },
  /* I_OUT              */  { { 0x17 }, 2, 0, 1 },
  /* TEMP_INT           */  { { 0x19 }, 2, 0, 1 },
  /* R_THRM             */  { { 0x1B }, 4, 0, 1 },
  /* TEMP_THRM          */  { { 0x1F }, 2, 0, 1 },
  /* R25_THRM           */  { { 0x21 }, 4, 4, 1 },
  /* B_THRM             */  { { 0x25 }, 2, 2, 0 },
  /* TEMP_SEL           */  { { 0x27 }, 1, 1, 0 },
  /* TEMP               */  { { 0x28 }, 2, 0, 1 },
  /* V_IN_OC            */  { { 0x2A }, 2, 0, 1 },
  /* I_CHRG_MAX         */  { { 0x2C }, 2, 2, 1 },
  /* I_CHRG_BAL         */  { { 0x2E }, 2, 0, 1 },
  /* I_CHRG_UNDERTEMP   */  { { 0x30 }, 2, 2, 1 },
  /* I_PRECHRG_PCT      */  { { 0x32 }, 1, 1, 0 },
  /* I_CHRG_TERM_PCT    */  { { 0x33 }, 1, 1, 0 },
  /* V_CELL_MIN         */  { { 0x34 }, 2, 2, 1 },
  /* V_CELL_SHDN        */  { { 0x36 }, 2, 2, 1 },
  /* V_CELL_BOOT        */  { { 0x38 }, 2, 2, 1 },
  /* V_CELL_MAX         */  { { 0x3A }, 2, 2, 1 },
  /* V_BAT_MIN          */  { { 0x3C }, 2, 2, 1 },
  /* V_BAT_SHDN         */  { { 0x3E }, 2, 2, 1 },
  /* V_BAT_BOOT         */  { { 0x40 }, 2, 2, 1 },
  /* V_BAT_MAX          */  { { 0x42 }, 2, 2, 1 },
  /* V_BAT_CV_PCT       */  { { 0x44 }, 1, 1, 0 },
  /* V_BAT_CHRG_PCT     */  { { 0x45 }, 1, 1, 0 },
  /* V_BAT_PRECHRG_PCT  */  { { 0x46 }, 1, 1, 0 },
  /* V_IN_DROPOUT       */  { { 0x47 }, 2, 2, 1 },
  /* V_IN_MIN_CHRG      */  { { 0x49 }, 2, 0, 1 },
  /* P_IN_MAX           */  { { 0x4B }, 2, 2, 0 },
  /* I_IN_MAX           */  { { 0x4D }, 2, 2, 1 },
  /* I_IN_LIMIT         */  { { 0x4F }, 2, 0, 1 },
  /* V_IN_OC_PCT        */  { { 0x51 }, 1, 1, 0 },
  /* V_IN_MPP           */  { { 0x52 }, 2, 0, 1 },
  /* V_IN_MPP_STEP      */  { { 0x54 }, 2, 2, 1 },
  /* I_MIN_TRUST        */  { { 0x56 }, 2, 2, 1 },
  /* I_OK_TRUST         */  { { 0x58 }, 2, 2, 1 },
  /* CHRG_PID_KP        */  { { 0x5A }, 2, 2, 1 },
  /* CHRG_PID_KI        */  { { 0x5C }, 2, 2, 1 },
  /* CHRG_PID_KD        */  { { 0x5E }, 2, 2, 1 },
  /* CHRG_TEMP_MIN      */  { { 0x60 }, 2, 2, 1 },
  /* CHRG_TEMP_COOL     */  { { 0x62 }, 2, 2, 1 },
  /* CHRG_TEMP_WARM     */  { { 0x64 }, 2, 2, 1 },
  /* CHRG_TEMP_MAX      */  { { 0x66 }, 2, 2, 1 },
  /* I_CHRG_COOL_PCT    */  { { 0x68 }, 1, 1, 0 },
  /* I_CHRG_WARM_PCT    */  { { 0x69 }, 1, 1, 0 },
  /* V_BAT_WARM_PCT     */  { { 0x6A }, 1, 1, 0 },
  /* CHRG_STATE         */  { { 0x6B }, 1, 0, 0 },
  /* HEATER_SEL         */  { { 0x6C }, 1, 1, 0 },
  /* EXT_HEAT_MAX_PWM   */  { { 0x6D }, 1, 1, 0 },
  /* LOAD_USB_PD        */  { { 0x6E }, 1, 1, 0 },
  /* INPUT_USB_PD       */  { { 0x6F }, 1, 1, 0 },
  /* V_OUT_MAX          */  { { 0x70 }, 2, 2, 1 },
  /* V_IN_THRESHOLD     */  { { 0x72 }, 2, 2, 1 },
  /* I_OUT_ON_BOOT      */  { { 0x74 }, 2, 0, 1 },
  /* I_OUT_SHDN_DONE    */  { { 0x76 }, 2, 2, 1 },
  /* VOFFSET_ADC        */  { { R_NA }, 0, 0, 0 },
  /* VBAT_OFFSET        */  { { R_NA }, 0, 0, 0 },
  /* VOUT_OFFSET        */  { { R_NA }, 0, 0, 0 },
  /* VIN_OFFSET         */  { { R_NA }, 0, 0, 0 },
  /* IOUT_OFFSET        */  { { R_NA }, 0, 0, 0 },
  /* BOOT_TIMEOUT       */  { { 0x78 }, 2, 2, 0 },
  /* SHDN_TIMEOUT       */  { { 0x7A }, 2, 2, 0 },
  /* RTC_TIME           */  { { 0x7C }, 4, 4, 0 },
  /* RTC_WAKE_TIME      */  { { 0x80 }, 4, 4, 0 },
  /* AUTO_BOOT          */  { { 0x84 }, 1, 1, 0 },
  /* WAKE_TIME          */  { { R_NA }, 0, 0, 0 },
  /* SHDN_DELAY         */  { { 0x85 }, 1, 1, 0 },
  /* AUTO_SHDN_TIME     */  { { 0x86 }, 4, 4, 0 },
  /* WATCHDOG_CFG       */  { { 0x8A }, 1, 1, 0 },
  /* WATCHDOG_GRACE     */  { { 0x8B }, 1, 1, 0 },
  /* WATCHDOG_TIMER     */  { { 0x8C }, 1, 1, 0 },
  /* LOAD_RUNNING       */  { { 0x8D }, 1, 1, 0 },
  /* STBY_WAKE_PERIOD   */  { { 0x8E }, 1, 1, 0 },
  /* BAT_SOC_PCT        */  { { 0x8F }, 1, 0, 1 },
  /* BAT_CAPACITY       */  { { 0x90 }, 4, 4, 1 },
  /* BAT_CAPACITY_DET   */  { { 0x94 }, 4, 0, 1 },
  /* BAT_LOAD_PRESET    */  { { 0x98 }, 1, 1, 0 },
  /* CFG_WRITE          */  { { 0x99 }, 1, 1, 0 },
};

/* Register version detected */

static int32_t reg_ver = 0;


/* Capitalize a string */

static void capitalize(char *s) {
  for (; *s; s++) {
    *s = toupper(*s);
  }
}

/* Get the variable from a string representation */

enum eLiFePO4weredVar get_lifepo4wered_var_from_string(char *var) {
  /* Capitalize the string */
  capitalize(var);
  /* Check if we have a I2C register version */
  if (reg_ver <= 0) {
    /* If not, read it */
    reg_ver = read_lifepo4wered(REG_VERSION);
  }
  /* New or legacy device register version? */
  if (reg_ver < REG_VER_NEW_START) {
    /* Legacy device registers */
    for (int i = 0; i < LFP_VAR_COUNT; i++) {
      if (strcmp(var, lifepo4wered_var_name_legacy[i]) == 0) {
        return (enum eLiFePO4weredVar)i;
      }
    }
  } else {
    /* New device registers */
    for (int i = 0; i < LFP_VAR_COUNT; i++) {
      if (strcmp(var, lifepo4wered_var_name[i]) == 0) {
        return (enum eLiFePO4weredVar)i;
      }
    }
  }
  return LFP_VAR_INVALID;
}

/* Get the string representation of a variable */

const char *get_lifepo4wered_var_name(enum eLiFePO4weredVar var) {
  static const char unknown[] = "UNKNOWN";

  /* Check if we have a I2C register version */
  if (reg_ver <= 0) {
    /* If not, read it */
    reg_ver = read_lifepo4wered(REG_VERSION);
  }
  /* Enum value in range? */
  if (var < LFP_VAR_COUNT) {
    /* New or legacy device register version? */
    if (reg_ver < REG_VER_NEW_START) {
      /* Legacy device register names */
      return lifepo4wered_var_name_legacy[var];
    }
    else {
      /* New device register names */
      return lifepo4wered_var_name[var];
    }
  }

  return unknown;
}

/* Determine if the specified variable can be accessed in the specified
 * manner (read, write or both) and return a pointer to the variable
 * definition (internal function) */

static bool can_access_lifepo4wered(enum eLiFePO4weredVar var,
                    uint8_t access_mask, struct sVarDefGeneric *vd) {
  /* Check if we have a I2C register version */
  if (reg_ver <= 0) {
    /* If not, read it */
    reg_ver = read_lifepo4wered(REG_VERSION);
  }
  /* New or legacy device register version? */
  if (reg_ver < REG_VER_NEW_START) {
    /* Legacy device registers */
    /* Are the variable and I2C register version in defined range? */
    if (var >= 0  && var < LFP_VAR_COUNT &&
        reg_ver > 0 && reg_ver <= I2C_REG_VER_COUNT_LEGACY) {
      /* Get a pointer to the variable definition */
      const struct sVarDefLegacy *var_def = &var_table_legacy[var];
      /* Save it to the provided pointer, if one is provided */
      if (vd) {
        vd->reg = var_def->reg[reg_ver - 1];
        vd->read_bytes = var_def->read_bytes;
        vd->write_bytes = var_def->write_bytes;
        vd->sign_extend = var_def->sign_extend;
        const struct sVarScale *scale =
            &var_scale[var_def->scale_idx][var_scale_variant[reg_ver - 1]];
        vd->scale_mul = scale->mul;
        vd->scale_div = scale->div;
      }
      /* Is this variable defined for the register version? */
      if (var_def->reg[reg_ver - 1] != R_NA) {
        /* Then check the access */
        return ((access_mask & ACCESS_READ) && var_def->read_bytes) ||
                ((access_mask & ACCESS_WRITE) && var_def->write_bytes);
      }
    }
  } else {
    /* New device registers */
    /* Are the variable and register version in defined range? */
    if (var >= 0 && var < LFP_VAR_COUNT &&
        reg_ver < (REG_VER_NEW_START + REG_VER_COUNT)) {
      /* Get a pointer to the variable definition */
      const struct sVarDef *var_def = &var_table[var];
      /* Save it to the provided pointer, if one is provided */
      if (vd) {
        vd->reg = var_def->reg[reg_ver - REG_VER_NEW_START];
        vd->read_bytes = var_def->read_bytes;
        vd->write_bytes = var_def->write_bytes;
        vd->sign_extend = var_def->sign_extend;
        vd->scale_mul = 1;
        vd->scale_div = 1;
      }
      /* Is this variable defined for the register version? */
      if (var_def->reg[reg_ver - REG_VER_NEW_START] != R_NA) {
        /* Then check the access */
        return ((access_mask & ACCESS_READ) && var_def->read_bytes) ||
                ((access_mask & ACCESS_WRITE) && var_def->write_bytes);
      }
    }
  }
  /* Access not available */
  return false;
}

/* Determine if the specified variable can be accessed in the specified
 * manner (read, write or both, external function) */

bool access_lifepo4wered(enum eLiFePO4weredVar var, uint8_t access_mask) {
  return can_access_lifepo4wered(var, access_mask, NULL);
}

/* Read data from LiFePO4wered device
 * Because the MSP430G micro I2C peripheral relies heavily on software
 * support, it seems not possible to make reads work 100% reliable at
 * 100kHz, because other interrupts that are running may cause too much
 * latency and the first bit comes out wrong.  We fix that by requiring
 * a number of identical reads.  This also takes care of rejecting
 * multi-byte values that change in the middle of a read.
 * We don't do this for new devices because some readings like voltage
 * can change too fast (5000 times per second) and the newer micros do
 * not have weak drivers and slow I2C performance and have atomic caching
 * for multi-byte values. */

int32_t read_lifepo4wered(enum eLiFePO4weredVar var) {
  struct sVarDefGeneric var_def;
  if (var == REG_VERSION ||
      (can_access_lifepo4wered(var, ACCESS_READ, &var_def) && reg_ver)) {
    uint8_t match_tries = 0;
    union {
      uint8_t   b[4];
      int16_t   h[2];
      int32_t   i;
    } data, match_data;
    data.i = 0;
    match_data.i = 0;
    uint8_t reg = (var == REG_VERSION) ? 0 : var_def.reg;
    uint8_t read_bytes = (var == REG_VERSION) ? 1 : var_def.read_bytes;
    uint8_t sign_extend = (var == REG_VERSION) ? 0 : var_def.sign_extend;
    for (uint8_t retries = 0; retries < I2C_RETRIES; retries++) {
      if (read_lifepo4wered_data(reg, read_bytes, data.b)) {
        if (!match_tries || data.i == match_data.i) {
          if (reg_ver >= REG_VER_NEW_START ||
              match_tries >= I2C_IDENTICAL_READS - 1) {
            if (sign_extend) {
              if (read_bytes == 2) data.i = data.h[0];
              if (read_bytes == 1) data.i = data.b[0];
            }
            if (var == REG_VERSION || reg_ver >= REG_VER_NEW_START) {
              return le32toh(data.i);
            }
            return (le32toh(data.i) * var_def.scale_mul + var_def.scale_div
                    / 2) / var_def.scale_div;
          }
          match_tries++;
        } else {
          match_tries = 0;
        }
        match_data.i = data.i;
      }
      usleep(500);
    }
    return -2;
  }
  return -1;
}

/* Write data to LiFePO4wered device */

int32_t write_lifepo4wered(enum eLiFePO4weredVar var, int32_t value) {
  struct sVarDefGeneric var_def;
  if (can_access_lifepo4wered(var, ACCESS_WRITE, &var_def) && reg_ver) {
    union {
      uint8_t   b[4];
      int32_t   i;
    } data;
    if (reg_ver >= REG_VER_NEW_START) {
      data.i = htole32(value);
    } else {
      data.i = htole32((value * var_def.scale_div + var_def.scale_mul / 2)
                / var_def.scale_mul);
    }
    for (uint8_t retries = 0; retries < I2C_RETRIES; retries++) {
      if (write_lifepo4wered_data(var_def.reg,
                                  var_def.write_bytes, data.b,
                                  reg_ver >= I2C_WRUNLOCK_REG_VER)) {
        return read_lifepo4wered(var);
      }
    }
    return -2;
  }
  return -1;
}
