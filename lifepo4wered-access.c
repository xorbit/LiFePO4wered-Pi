/* 
 * LiFePO4wered/Pi access module
 * Copyright (C) 2015 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "lifepo4wered-access.h"


/* LiFePO4wered/Pi access constants */

#define I2C_ADDRESS         0x43


/* Open access to the specified I2C bus */

static bool open_i2c_bus(int bus, int *file) {
  /* Create the name of the device file */
  char filename[20];
  snprintf(filename, 19, "/dev/i2c-%d", bus);
  /* Open the device file */
  *file = open(filename, O_RDWR);
  /* Report result */
  return *file >= 0;
}

/* Close access to the specified I2C bus */

static bool close_i2c_bus(int file) {
  close(file);
  return file >= 0;
}

/* Read LiFePO4wered/Pi data */

bool read_lifepo4wered_data(uint8_t reg, uint8_t count, uint8_t *data) {
  /* Open the I2C bus */
  int file;
  if (!open_i2c_bus(1, &file))
    return false;

  /* Declare I2C message structures */
  struct i2c_msg dread[2];
  struct i2c_rdwr_ioctl_data msgset = {
    dread,
    2
  };
  /* Write register message */
  dread[0].addr = I2C_ADDRESS;
  dread[0].flags = 0;
  dread[0].len = 1;
  dread[0].buf = &reg;
  /* Read data message */
  dread[1].addr = I2C_ADDRESS;
  dread[1].flags = I2C_M_RD;
  dread[1].len = count;
  dread[1].buf = data;

  /* Execute the command */
  bool result = ioctl(file, I2C_RDWR, &msgset) >= 0;

  /* Close the I2C bus */
  close_i2c_bus(file);

  /* Return the result */
  return result;
}

/* Write LiFePO4wered/Pi chip data */

bool write_lifepo4wered_data(uint8_t reg, uint8_t count, uint8_t *data) {
  /* Open the I2C bus */
  int file;
  if (!open_i2c_bus(1, &file))
    return false;

  /* Declare I2C message structures */
  struct i2c_msg dwrite;
  struct i2c_rdwr_ioctl_data msgset = {
    &dwrite,
    1
  };
  /* Message payload */
  uint8_t payload[256];
  payload[0] = reg;
  memcpy(&payload[1], data, count);
  /* Write data message */
  dwrite.addr = I2C_ADDRESS;
  dwrite.flags = 0;
  dwrite.len = 1 + count;
  dwrite.buf = payload;

  /* Execute the command */
  bool result = ioctl(file, I2C_RDWR, &msgset) >= 0;

  /* Close the I2C bus */
  close_i2c_bus(file);

  /* Return the result */
  return result;
}
