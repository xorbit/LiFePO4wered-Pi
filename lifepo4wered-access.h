/* 
 * LiFePO4wered/Pi access module
 * Copyright (C) 2015 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#ifndef LIFEPO4WERED_ACCESS_H
#define LIFEPO4WERED_ACCESS_H

#include <stdint.h>
#include <stdbool.h>


/* Read LiFePO4wered/Pi data */

bool read_lifepo4wered_data(uint8_t reg, uint8_t count, uint8_t *data);

/* Write LiFePO4wered/Pi chip data */

bool write_lifepo4wered_data(uint8_t reg, uint8_t count, uint8_t *data,
                              bool unlock);


#endif
