/* 
 * LiFePO4wered/Pi data module
 * Copyright (C) 2015 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#define _BSD_SOURCE
#include <endian.h>
#include "lifepo4wered-data.h"
#include "lifepo4wered-access.h"


/* Generate strings for variable names */

#define GENERATE_STRING(STRING) #STRING,

const char *lifepo4wered_var_name[LFP_VAR_COUNT] = {
  FOREACH_LIFEPO4WERED_VAR(GENERATE_STRING)
};


/* Structure to define variable read and write behavior */

struct {
  uint8_t   reg;
  int32_t   scale_mul;
  int32_t   scale_div;
  uint8_t   read_bytes;
  uint8_t   write_bytes;
} static const var_table[LFP_VAR_COUNT] = {
  /* I2C_ADDRESS      */  { 0x00,      1,      1,   1,  1 },
  /* TOUCH_STATE      */  { 0x15,      1,      1,   1,  0 },
  /* TOUCH_CAP_CYCLES */  { 0x01,      1,      1,   1,  1 },
  /* TOUCH_THRESHOLD  */  { 0x02,      1,      1,   1,  1 },
  /* TOUCH_HYSTERESIS */  { 0x03,      1,      1,   1,  1 },
  /* DCO_RSEL         */  { 0x04,      1,      1,   1,  1 },
  /* DCO_DCOMOD       */  { 0x05,      1,      1,   1,  1 },
  /* VIN              */  { 0x11,   5000,   1023,   2,  0 },
  /* VOUT             */  { 0x13, 554878, 102300,   2,  0 },
  /* VIN_MIN          */  { 0x06,   5000,   1023,   2,  2 },
  /* VIN_SHDN         */  { 0x08,   5000,   1023,   2,  2 },
  /* VIN_BOOT         */  { 0x0A,   5000,   1023,   2,  2 },
  /* VOUT_MAX         */  { 0x0C, 554878, 102300,   2,  2 },
  /* AUTO_BOOT        */  { 0x0E,      1,      1,   1,  1 },
  /* PI_RUNNING       */  { 0x10,      1,      1,   1,  1 },
  /* CFG_WRITE        */  { 0x0F,      1,      1,   1,  1 },
};


/* Read data from LiFePO4wered/Pi */

int32_t read_lifepo4wered(enum eLiFePO4weredVar var) {
  if (var < LFP_VAR_COUNT && var_table[var].read_bytes > 0) {
    union {
      uint8_t   b[4];
      int32_t   i;
    } data;
    data.i = 0;
    if (read_lifepo4wered_data(var_table[var].reg,
                              var_table[var].read_bytes, &data.b[0])) {
      return le32toh(data.i) * var_table[var].scale_mul
              / var_table[var].scale_div;
    }
  }
  return -1;
}

/* Write data to LiFePO4wered/Pi */

void write_lifepo4wered(enum eLiFePO4weredVar var, int32_t value) {
  if (var < LFP_VAR_COUNT && var_table[var].write_bytes > 0) {
    union {
      uint8_t   b[4];
      int32_t   i;
    } data;
    data.i = htole32(value * var_table[var].scale_div
              / var_table[var].scale_mul);
    write_lifepo4wered_data(var_table[var].reg,
                            var_table[var].write_bytes, &data.b[0]);
  }
}
