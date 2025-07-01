/*
 * Copyright (c) 2025, PHYTEC Messtechnik GmbH
 * Author: Florijan Plohl <florijan.plohl@norik.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PHYFLEX_AM62L_H
#define PHYFLEX_AM62L_H

#include <stdint.h>
#include <stddef.h>

/* I2C register base addresses */
#define I2C0_BASE_ADDR		0x20000000
#define I2C1_BASE_ADDR		0x20010000
#define I2C2_BASE_ADDR		0x20020000
#define WKUP_I2C_BASE_ADDR 	0x2B200000

enum I2C_BUS {
	I2C_BUS_0,
	I2C_BUS_1,
	I2C_BUS_2,
	WKUP_I2C_BUS
};

enum {
	EEPROM_RAM_SIZE_512MB = 0,
	EEPROM_RAM_SIZE_1GB = 1,
	EEPROM_RAM_SIZE_2GB = 2,
	EEPROM_RAM_SIZE_4GB = 4
};

int phyflex_am62l_som_detection(void);

#endif /* PHYFLEX_AM62L_H */
