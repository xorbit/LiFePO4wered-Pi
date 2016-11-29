/* 
 * LiFePO4wered/Pi data module
 * Copyright (C) 2015 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#define _BSD_SOURCE
#include <endian.h>
#include <unistd.h>
#include "lifepo4wered-data.h"
#include "lifepo4wered-access.h"


/* Generate strings for variable names */

#define GENERATE_STRING(STRING) #STRING,

const char *lifepo4wered_var_name[LFP_VAR_COUNT] = {
  FOREACH_LIFEPO4WERED_VAR(GENERATE_STRING)
};


/* Number of I2C register versions defined */

#define I2C_REG_VER_COUNT     4

/* Constant to use when a register is not availabled in a particular
 * register version */

#define R_NA                  0xFF

/* Structure to define variable read and write behavior */

struct sVarDef {
  uint8_t   reg[I2C_REG_VER_COUNT];
  int32_t   scale_mul;
  int32_t   scale_div;
  uint8_t   read_bytes;
  uint8_t   write_bytes;
};

/* This table covers all different I2C register versions,
 * the set of registers available to each version is defined
 * in separate tables. */

static const struct sVarDef var_table[] = {
  /* I2C_REG_VER      */  { { 0x00, 0x00, 0x00, 0x00 },      1,      1,   1,  0 },
  /* I2C_ADDRESS      */  { { 0x01, 0x01, 0x01, 0x01 },      1,      1,   1,  1 },
  /* LED_STATE        */  { { 0x02, 0x02, 0x02, 0x02 },      1,      1,   1,  1 },
  /* TOUCH_STATE      */  { { 0x19, 0x1B, 0x1D, 0x23 },      1,      1,   1,  0 },
  /* TOUCH_CAP_CYCLES */  { { 0x03, 0x03, 0x03, 0x03 },      1,      1,   1,  1 },
  /* TOUCH_THRESHOLD  */  { { 0x04, 0x04, 0x04, 0x04 },      1,      1,   1,  1 },
  /* TOUCH_HYSTERESIS */  { { 0x05, 0x05, 0x05, 0x05 },      1,      1,   1,  1 },
  /* DCO_RSEL         */  { { 0x06, 0x06, 0x06, 0x06 },      1,      1,   1,  1 },
  /* DCO_DCOMOD       */  { { 0x07, 0x07, 0x07, 0x07 },      1,      1,   1,  1 },
  /* VIN              */  { { R_NA, R_NA, R_NA, 0x21 }, 966667, 102300,   2,  0 },
  /* VBAT             */  { { 0x15, 0x17, 0x19, 0x1D },   5000,   1023,   2,  0 },
  /* VOUT             */  { { 0x17, 0x19, 0x1B, 0x1F }, 554878, 102300,   2,  0 },
  /* VBAT_MIN         */  { { 0x08, 0x08, 0x08, 0x08 },   5000,   1023,   2,  2 },
  /* VBAT_SHDN        */  { { 0x0A, 0x0A, 0x0A, 0x0A },   5000,   1023,   2,  2 },
  /* VBAT_BOOT        */  { { 0x0C, 0x0C, 0x0C, 0x0C },   5000,   1023,   2,  2 },
  /* VOUT_MAX         */  { { 0x0E, 0x0E, 0x0E, 0x0E }, 554878, 102300,   2,  2 },
  /* VIN_THRESHOLD    */  { { R_NA, R_NA, R_NA, 0x10 }, 966667, 102300,   2,  2 },
  /* VOFFSET_ADC      */  { { R_NA, R_NA, 0x10, 0x12 },   5000,   1023,   2,  2 },
  /* AUTO_BOOT        */  { { 0x10, 0x12, 0x14, 0x18 },      1,      1,   1,  1 },
  /* WAKE_TIME        */  { { 0x12, 0x14, 0x16, 0x1A },      1,      1,   2,  2 },
  /* SHDN_DELAY       */  { { R_NA, 0x10, 0x12, 0x14 },      1,      1,   2,  2 },
  /* AUTO_SHDN_TIME   */  { { R_NA, R_NA, R_NA, 0x16 },      1,      1,   2,  2 },
  /* PI_RUNNING       */  { { 0x14, 0x16, 0x18, 0x1C },      1,      1,   1,  1 },
  /* CFG_WRITE        */  { { 0x11, 0x13, 0x15, 0x19 },      1,      1,   1,  1 },
};

/* I2C register version detected */

static int32_t i2c_reg_ver = 0;

/* I2C access retries */

#define I2C_RETRIES     5


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

/* Read data from LiFePO4wered/Pi */

int32_t read_lifepo4wered(enum eLiFePO4weredVar var) {
  const struct sVarDef *var_def;
  if (var == I2C_REG_VER ||
      can_access_lifepo4wered(var, ACCESS_READ, &var_def)) {
    union {
      uint8_t   b[4];
      int32_t   i;
    } data;
    data.i = 0;
    for (uint8_t retries = I2C_RETRIES; retries; retries--) {
      if (var == I2C_REG_VER) {
        if (read_lifepo4wered_data(I2C_REG_VER, 1, data.b)) {
          return le32toh(data.i);
        }
      } else {
        if (read_lifepo4wered_data(var_def->reg[i2c_reg_ver - 1],
                                  var_def->read_bytes, data.b)) {
          return (le32toh(data.i) * var_def->scale_mul
                  + var_def->scale_div / 2) / var_def->scale_div;
        }
      }
      usleep(2000);
    }
    return -2;
  }
  return -1;
}

/* Write data to LiFePO4wered/Pi */

int32_t write_lifepo4wered(enum eLiFePO4weredVar var, int32_t value) {
  const struct sVarDef *var_def;
  if (can_access_lifepo4wered(var, ACCESS_WRITE, &var_def)) {
    union {
      uint8_t   b[4];
      int32_t   i;
    } data;
    data.i = htole32((value * var_def->scale_div + var_def->scale_mul / 2)
                      / var_def->scale_mul);
    for (uint8_t retries = I2C_RETRIES; retries; retries--) {
      if (write_lifepo4wered_data(var_def->reg[i2c_reg_ver - 1],
                                  var_def->write_bytes, data.b)) {
        return read_lifepo4wered(var);
      }
    }
    return -2;
  }
  return -1;
}
