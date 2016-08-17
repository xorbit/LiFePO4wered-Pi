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


/* Structure to define variable read and write behavior */

struct sVarDef {
  uint8_t   reg;
  int32_t   scale_mul;
  int32_t   scale_div;
  uint8_t   read_bytes;
  uint8_t   write_bytes;
};

/* This table covers all different I2C register versions,
 * the set of registers available to each version is defined
 * in separate tables. */

static const struct sVarDef all_vars[] = {
  /*  0: I2C_REG_VER      */  { 0x00,      1,      1,   1,  0 },
  /*  1: I2C_ADDRESS      */  { 0x01,      1,      1,   1,  1 },
  /*  2: LED_STATE        */  { 0x02,      1,      1,   1,  1 },
  /*  3: TOUCH_CAP_CYCLES */  { 0x03,      1,      1,   1,  1 },
  /*  4: TOUCH_THRESHOLD  */  { 0x04,      1,      1,   1,  1 },
  /*  5: TOUCH_HYSTERESIS */  { 0x05,      1,      1,   1,  1 },
  /*  6: DCO_RSEL         */  { 0x06,      1,      1,   1,  1 },
  /*  7: DCO_DCOMOD       */  { 0x07,      1,      1,   1,  1 },
  /*  8: VIN_MIN          */  { 0x08,   5000,   1023,   2,  2 },
  /*  9: VIN_SHDN         */  { 0x0A,   5000,   1023,   2,  2 },
  /* 10: VIN_BOOT         */  { 0x0C,   5000,   1023,   2,  2 },
  /* 11: VOUT_MAX         */  { 0x0E, 554878, 102300,   2,  2 },
  /* 12: AUTO_BOOT (1)    */  { 0x10,      1,      1,   1,  1 },
  /* 13: CFG_WRITE (1)    */  { 0x11,      1,      1,   1,  1 },
  /* 14: WAKE_TIME (1)    */  { 0x12,      1,      1,   2,  2 },
  /* 15: PI_RUNNING (1)   */  { 0x14,      1,      1,   1,  1 },
  /* 16: VIN (1)          */  { 0x15,   5000,   1023,   2,  0 },
  /* 17: VOUT (1)         */  { 0x17, 554878, 102300,   2,  0 },
  /* 18: TOUCH_STATE (1)  */  { 0x19,      1,      1,   1,  0 },
  /* 19: SHDN_DELAY (2)   */  { 0x10,      1,      1,   2,  2 },
  /* 20: AUTO_BOOT (2)    */  { 0x12,      1,      1,   1,  1 },
  /* 21: CFG_WRITE (2)    */  { 0x13,      1,      1,   1,  1 },
  /* 22: WAKE_TIME (2)    */  { 0x14,      1,      1,   2,  2 },
  /* 23: PI_RUNNING (2)   */  { 0x16,      1,      1,   1,  1 },
  /* 24: VIN (2)          */  { 0x17,   5000,   1023,   2,  0 },
  /* 25: VOUT (2)         */  { 0x19, 554878, 102300,   2,  0 },
  /* 26: TOUCH_STATE (2)  */  { 0x1B,      1,      1,   1,  0 },
};

/* Number of I2C register versions defined */

#define I2C_REG_VER_COUNT     2

/* This table refers to the correct variable definition in the
 * table above for each register version (register version - 1
 * to index) */

static const struct sVarDef *var_table[I2C_REG_VER_COUNT]
                                      [LFP_VAR_COUNT] = {
  /* Register set version 1 */
  {
    /* I2C_REG_VER      */  &all_vars[0],
    /* I2C_ADDRESS      */  &all_vars[1],
    /* LED_STATE        */  &all_vars[2],
    /* TOUCH_STATE      */  &all_vars[18],
    /* TOUCH_CAP_CYCLES */  &all_vars[3],
    /* TOUCH_THRESHOLD  */  &all_vars[4],
    /* TOUCH_HYSTERESIS */  &all_vars[5],
    /* DCO_RSEL         */  &all_vars[6],
    /* DCO_DCOMOD       */  &all_vars[7],
    /* VIN              */  &all_vars[16],
    /* VOUT             */  &all_vars[17],
    /* VIN_MIN          */  &all_vars[8],
    /* VIN_SHDN         */  &all_vars[9],
    /* VIN_BOOT         */  &all_vars[10],
    /* VOUT_MAX         */  &all_vars[11],
    /* AUTO_BOOT        */  &all_vars[12],
    /* WAKE_TIME        */  &all_vars[14],
    /* SHDN_DELAY       */  NULL,
    /* PI_RUNNING       */  &all_vars[15],
    /* CFG_WRITE        */  &all_vars[13],
  },
  /* Register set version 2 */
  {
    /* I2C_REG_VER      */  &all_vars[0],
    /* I2C_ADDRESS      */  &all_vars[1],
    /* LED_STATE        */  &all_vars[2],
    /* TOUCH_STATE      */  &all_vars[26],
    /* TOUCH_CAP_CYCLES */  &all_vars[3],
    /* TOUCH_THRESHOLD  */  &all_vars[4],
    /* TOUCH_HYSTERESIS */  &all_vars[5],
    /* DCO_RSEL         */  &all_vars[6],
    /* DCO_DCOMOD       */  &all_vars[7],
    /* VIN              */  &all_vars[24],
    /* VOUT             */  &all_vars[25],
    /* VIN_MIN          */  &all_vars[8],
    /* VIN_SHDN         */  &all_vars[9],
    /* VIN_BOOT         */  &all_vars[10],
    /* VOUT_MAX         */  &all_vars[11],
    /* AUTO_BOOT        */  &all_vars[20],
    /* WAKE_TIME        */  &all_vars[22],
    /* SHDN_DELAY       */  &all_vars[19],
    /* PI_RUNNING       */  &all_vars[23],
    /* CFG_WRITE        */  &all_vars[21],
  },
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
    const struct sVarDef *var_def = var_table[i2c_reg_ver - 1][var];
    /* Save it to the provided pointer, if one is provided */
    if (vd) {
      *vd = var_def;
    }
    /* Is this variable defined? */
    if (var_def) {
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
  if (var == I2C_REG_VER && access_mask & ACCESS_READ) {
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
        if (read_lifepo4wered_data(var_def->reg, var_def->read_bytes,
                                    data.b)) {
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
      if (write_lifepo4wered_data(var_def->reg, var_def->write_bytes,
                                    data.b)) {
        return read_lifepo4wered(var);
      }
    }
    return -2;
  }
  return -1;
}
