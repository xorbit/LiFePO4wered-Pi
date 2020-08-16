/* 
 * LiFePO4wered/Pi data module
 * Copyright (C) 2015-2020 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#define _DEFAULT_SOURCE
#include <endian.h>
#include <unistd.h>
#include "lifepo4wered-data.h"
#include "lifepo4wered-access.h"


/* Number of I2C register versions defined */

#define I2C_REG_VER_COUNT     7

/* Minimum I2C register version that requires write unlock */

#define I2C_WRUNLOCK_REG_VER  5

/* Constant to use when a register is not availabled in a particular
 * register version */

#define R_NA                  0xFF

/* I2C access retries */

#define I2C_RETRIES           20

/* I2C identical reads requirement */

#define I2C_IDENTICAL_READS   3

/* I2C retry delay in us */

#define I2C_RETRY_DELAY       500

/* Generate strings for variable names */

#define GENERATE_STRING(STRING) #STRING,

const char *lifepo4wered_var_name[LFP_VAR_COUNT] = {
  FOREACH_LIFEPO4WERED_VAR(GENERATE_STRING)
};


/* Structure to define variable scale factors */

struct sVarScale {
  int32_t       mul;
  int32_t       div;
};

/* Structure to define variable read and write behavior */

struct sVarDef {
  uint8_t       reg[I2C_REG_VER_COUNT];
  uint8_t       read_bytes;
  uint8_t       write_bytes;
  uint8_t       sign_extend;
};

/* This table defines scaling for all I2C registers, for different
 * scaling variants */

static const struct sVarScale var_scale[LFP_VAR_COUNT][3] = {
  /* I2C_REG_VER      */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* I2C_ADDRESS      */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* LED_STATE        */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* TOUCH_STATE      */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* TOUCH_CAP_CYCLES */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* TOUCH_THRESHOLD  */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* TOUCH_HYSTERESIS */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* DCO_RSEL         */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* DCO_DCOMOD       */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
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
  /* AUTO_BOOT        */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* WAKE_TIME        */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* SHDN_DELAY       */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* AUTO_SHDN_TIME   */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* PI_BOOT_TO       */  { {     10,      1 }, {     10,      1 }, {     10,      1 } },
  /* PI_SHDN_TO       */  { {     10,      1 }, {     10,      1 }, {     10,      1 } },
  /* RTC_TIME         */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* RTC_WAKE_TIME    */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* WATCHDOG_CFG     */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* WATCHDOG_GRACE   */  { {     10,      1 }, {     10,      1 }, {     10,      1 } },
  /* WATCHDOG_TIMER   */  { {     10,      1 }, {     10,      1 }, {     10,      1 } },
  /* PI_RUNNING       */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
  /* CFG_WRITE        */  { {      1,      1 }, {      1,      1 }, {      1,      1 } },
};

/* This table covers I2C register scale variants used by each register
 * version */

static const uint8_t var_scale_variant[I2C_REG_VER_COUNT] = {
  0, 0, 0, 0, 1, 1, 2
};

/* This table covers definitions of all I2C registers */

static const struct sVarDef var_table[LFP_VAR_COUNT] = {
  /* I2C_REG_VER      */  { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 1, 0, 0 },
  /* I2C_ADDRESS      */  { { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 }, 1, 1, 0 },
  /* LED_STATE        */  { { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 }, 1, 1, 0 },
  /* TOUCH_STATE      */  { { 0x19, 0x1B, 0x1D, 0x23, 0x22, 0x28, 0x3A }, 1, 0, 0 },
  /* TOUCH_CAP_CYCLES */  { { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 }, 1, 1, 0 },
  /* TOUCH_THRESHOLD  */  { { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }, 1, 1, 0 },
  /* TOUCH_HYSTERESIS */  { { 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05 }, 1, 1, 0 },
  /* DCO_RSEL         */  { { 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06 }, 1, 1, 0 },
  /* DCO_DCOMOD       */  { { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 }, 1, 1, 0 },
  /* VIN              */  { { R_NA, R_NA, R_NA, 0x21, R_NA, 0x26, 0x36 }, 2, 0, 1 },
  /* VBAT             */  { { 0x15, 0x17, 0x19, 0x1D, 0x1E, 0x22, 0x32 }, 2, 0, 1 },
  /* VOUT             */  { { 0x17, 0x19, 0x1B, 0x1F, 0x20, 0x24, 0x34 }, 2, 0, 1 },
  /* IOUT             */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x38 }, 2, 0, 1 },
  /* VBAT_MIN         */  { { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 }, 2, 2, 1 },
  /* VBAT_SHDN        */  { { 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A }, 2, 2, 1 },
  /* VBAT_BOOT        */  { { 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C }, 2, 2, 1 },
  /* VOUT_MAX         */  { { 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E }, 2, 2, 1 },
  /* VIN_THRESHOLD    */  { { R_NA, R_NA, R_NA, 0x10, R_NA, 0x10, 0x10 }, 2, 2, 1 },
  /* IOUT_SHDN_THRESH */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x1A }, 2, 2, 1 },
  /* VOFFSET_ADC      */  { { R_NA, R_NA, 0x10, 0x12, 0x10, 0x12, R_NA }, 2, 2, 1 },
  /* VBAT_OFFSET      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x12 }, 2, 2, 1 },
  /* VOUT_OFFSET      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x14 }, 2, 2, 1 },
  /* VIN_OFFSET       */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x16 }, 2, 2, 1 },
  /* IOUT_OFFSET      */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x18 }, 2, 2, 1 },
  /* AUTO_BOOT        */  { { 0x10, 0x12, 0x14, 0x18, 0x14, 0x18, 0x20 }, 1, 1, 0 },
  /* WAKE_TIME        */  { { 0x12, 0x14, 0x16, 0x1A, 0x1A, 0x1E, 0x26 }, 2, 2, 0 },
  /* SHDN_DELAY       */  { { R_NA, 0x10, 0x12, 0x14, 0x12, 0x14, 0x1C }, 2, 2, 0 },
  /* AUTO_SHDN_TIME   */  { { R_NA, R_NA, R_NA, 0x16, R_NA, 0x16, 0x1E }, 2, 2, 0 },
  /* PI_BOOT_TO       */  { { R_NA, R_NA, R_NA, R_NA, 0x15, 0x19, 0x21 }, 1, 1, 0 },
  /* PI_SHDN_TO       */  { { R_NA, R_NA, R_NA, R_NA, 0x16, 0x1A, 0x22 }, 1, 1, 0 },
  /* RTC_TIME         */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x28 }, 4, 4, 0 },
  /* RTC_WAKE_TIME    */  { { R_NA, R_NA, R_NA, R_NA, R_NA, R_NA, 0x2C }, 4, 4, 0 },
  /* WATCHDOG_CFG     */  { { R_NA, R_NA, R_NA, R_NA, 0x17, 0x1B, 0x23 }, 1, 1, 0 },
  /* WATCHDOG_GRACE   */  { { R_NA, R_NA, R_NA, R_NA, 0x18, 0x1C, 0x24 }, 1, 1, 0 },
  /* WATCHDOG_TIMER   */  { { R_NA, R_NA, R_NA, R_NA, 0x1C, 0x20, 0x30 }, 1, 1, 0 },
  /* PI_RUNNING       */  { { 0x14, 0x16, 0x18, 0x1C, 0x1D, 0x21, 0x31 }, 1, 1, 0 },
  /* CFG_WRITE        */  { { 0x11, 0x13, 0x15, 0x19, 0x19, 0x1D, 0x25 }, 1, 1, 0 },
};

/* I2C register version detected */

static int32_t i2c_reg_ver = 0;


/* Determine if the specified variable can be accessed in the specified
 * manner (read, write or both) and return a pointer to the variable
 * definition (internal function) */

static bool can_access_lifepo4wered(enum eLiFePO4weredVar var,
                    uint8_t access_mask, const struct sVarDef **vd) {
  /* Check if we have a I2C register version */
  if (i2c_reg_ver <= 0) {
    /* If not, read it */
    i2c_reg_ver = read_lifepo4wered(I2C_REG_VER);
  }
  /* Are the variable and I2C register version in defined range? */
  if (var > I2C_REG_VER && var < LFP_VAR_COUNT &&
      i2c_reg_ver > 0 && i2c_reg_ver <= I2C_REG_VER_COUNT) {
    /* Get a pointer to the variable definition */
    const struct sVarDef *var_def = &var_table[var];
    /* Save it to the provided pointer, if one is provided */
    if (vd) {
      *vd = var_def;
    }
    /* Is this variable defined for the register version? */
    if (var_def->reg[i2c_reg_ver - 1] != R_NA) {
      /* Then check the access */
      return ((access_mask & ACCESS_READ) && var_def->read_bytes) ||
              ((access_mask & ACCESS_WRITE) && var_def->write_bytes);
    }
  }
  /* Access not available */
  return false;
}

/* Determine if the specified variable can be accessed in the specified
 * manner (read, write or both, external function) */

bool access_lifepo4wered(enum eLiFePO4weredVar var, uint8_t access_mask) {
  if (var == I2C_REG_VER && (access_mask & ACCESS_READ)) {
    return true;
  } else {
    return can_access_lifepo4wered(var, access_mask, NULL);
  }
}

/* Read data from LiFePO4wered/Pi
 * Because the MSP430G micro I2C peripheral relies heavily on software
 * support, it seems not possible to make reads work 100% reliable at
 * 100kHz, because other interrupts that are running may cause too much
 * latency and the first bit comes out wrong.  We fix that by requiring
 * a number of identical reads.  This also takes care of rejecting
 * multi-byte values that change in the middle of a read, so shadow
 * buffering reads on the micro may not be needed anymore. */

int32_t read_lifepo4wered(enum eLiFePO4weredVar var) {
  const struct sVarDef *var_def;
  if (var == I2C_REG_VER ||
      can_access_lifepo4wered(var, ACCESS_READ, &var_def)) {
    uint8_t match_tries = 0;
    union {
      uint8_t   b[4];
      int16_t	h[2];
      int32_t   i;
    } data, match_data;
    data.i = 0;
    match_data.i = 0;
    uint8_t reg = var == I2C_REG_VER ?
                  I2C_REG_VER : var_def->reg[i2c_reg_ver - 1];
    uint8_t read_bytes = var == I2C_REG_VER ? 1 : var_def->read_bytes;
    uint8_t sign_extend = var == I2C_REG_VER ? 0 : var_def->sign_extend;
    for (uint8_t retries = 0; retries < I2C_RETRIES; retries++) {
      usleep(500);
      if (read_lifepo4wered_data(reg, read_bytes, data.b)) {
        if (!match_tries || data.i == match_data.i) {
          if (match_tries >= I2C_IDENTICAL_READS - 1) {
            if (var == I2C_REG_VER) {
              return le32toh(data.i);
            }
            const struct sVarScale *scale =
                    &var_scale[var][var_scale_variant[i2c_reg_ver - 1]];
	    if (sign_extend) {
              data.i = data.h[0];
            }
            return (le32toh(data.i) * scale->mul + scale->div / 2)
                    / scale->div;
          }
          match_tries++;
        } else {
          match_tries = 0;
        }
        match_data.i = data.i;
      }
    }
    return -2;
  }
  return -1;
}

/* Write data to LiFePO4wered/Pi */

int32_t write_lifepo4wered(enum eLiFePO4weredVar var, int32_t value) {
  const struct sVarDef *var_def;
  if (can_access_lifepo4wered(var, ACCESS_WRITE, &var_def) && i2c_reg_ver) {
    union {
      uint8_t   b[4];
      int32_t   i;
    } data;
    const struct sVarScale *scale =
          &var_scale[var][var_scale_variant[i2c_reg_ver - 1]];
    data.i = htole32((value * scale->div + scale->mul / 2) / scale->mul);
    for (uint8_t retries = 0; retries < I2C_RETRIES; retries++) {
      if (write_lifepo4wered_data(var_def->reg[i2c_reg_ver - 1],
                                  var_def->write_bytes, data.b,
                                  i2c_reg_ver >= I2C_WRUNLOCK_REG_VER)) {
        return read_lifepo4wered(var);
      }
    }
    return -2;
  }
  return -1;
}
