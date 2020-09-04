/* 
 * LiFePO4wered/Pi access module
 * Copyright (C) 2015-2020 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#define _DEFAULT_SOURCE
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/file.h>
#include <linux/i2c-dev.h>
#ifndef I2C_FUNC_I2C
#include <linux/i2c.h>
#define TOBUFTYPE(x) (x)
#else
#define TOBUFTYPE(x) ((char *)(x))
#endif
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "lifepo4wered-access.h"


/* LiFePO4wered/Pi access constants */

#define I2C_BUS             1
#define I2C_ADDRESS         0x43
#define I2C_WR_UNLOCK       0xC9


/* Open access to the specified I2C bus */

static bool open_i2c_bus(int bus, int *file) {
  /* Create the name of the device file */
  char filename[20];
  snprintf(filename, 19, "/dev/i2c-%d", bus);
  /* Open the device file */
  *file = open(filename, O_RDWR);
  if (*file < 0) return false;
  /* Lock access */
  if (flock(*file, LOCK_EX|LOCK_NB) != 0) {
    close (*file);
    return false;
  }
  /* Success */
  return true;
}

/* Close access to the specified I2C bus */

static bool close_i2c_bus(int file) {
  flock(file, LOCK_UN);
  close(file);
  return file >= 0;
}

/* Read LiFePO4wered/Pi data */

bool read_lifepo4wered_data(uint8_t reg, uint8_t count, uint8_t *data) {
  /* Open the I2C bus */
  int file;
  if (!open_i2c_bus(I2C_BUS, &file))
    return false;

  /* Declare I2C message structures */
  struct i2c_msg dread[2];
  struct i2c_rdwr_ioctl_data msgread = {
    dread,
    2
  };
  /* Write register message */
  dread[0].addr = I2C_ADDRESS;
  dread[0].flags = 0;
  dread[0].len = 1;
  dread[0].buf = TOBUFTYPE(&reg);
  /* Read data message */
  dread[1].addr = I2C_ADDRESS;
  dread[1].flags = I2C_M_RD;
  dread[1].len = count;
  dread[1].buf = TOBUFTYPE(data);

  /* Execute the command to send the register */
  bool result = ioctl(file, I2C_RDWR, &msgread) >= 0;

  /* Close the I2C bus */
  close_i2c_bus(file);

  /* Return the result */
  return result;
}

/* Write LiFePO4wered/Pi chip data */

bool write_lifepo4wered_data(uint8_t reg, uint8_t count, uint8_t *data,
                              bool unlock) {
  /* Open the I2C bus */
  int file;
  if (!open_i2c_bus(I2C_BUS, &file))
    return false;

  /* Declare I2C message structures */
  struct i2c_msg dwrite;
  struct i2c_rdwr_ioctl_data msgwrite = {
    &dwrite,
    1
  };
  /* Message payload */
  uint8_t payload[255];
  uint8_t header_len = unlock ? 2 : 1;
  payload[0] = reg;
  payload[1] = (I2C_ADDRESS << 1) ^ I2C_WR_UNLOCK ^ reg;
  memcpy(&payload[header_len], data, count);
  /* Write data message */
  dwrite.addr = I2C_ADDRESS;
  dwrite.flags = 0;
  dwrite.len = header_len + count;
  dwrite.buf = TOBUFTYPE(payload);

  /* Execute the command */
  bool result = ioctl(file, I2C_RDWR, &msgwrite) >= 0;

  /* Close the I2C bus */
  close_i2c_bus(file);

  /* Return the result */
  return result;
}
