/*
 * Copyright (c) 2025, PHYTEC Messtechnik GmbH
 * Author: Florijan Plohl <florijan.plohl@norik.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>
#include <stddef.h>

int eeprom_write_byte(int bus_num, uint8_t dev_addr, uint16_t mem_addr, uint8_t *data);
int eeprom_read_byte(int bus_num, uint8_t dev_addr, uint16_t mem_addr, uint8_t *data, size_t length);

#endif /* EEPROM_H */
