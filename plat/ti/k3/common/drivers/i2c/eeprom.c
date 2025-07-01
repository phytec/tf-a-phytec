/*
 * Copyright (c) 2025, PHYTEC Messtechnik GmbH
 * Author: Florijan Plohl <florijan.plohl@norik.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "eeprom.h"
#include "ti_i2c.h"
#include <common/debug.h>

int eeprom_write_byte(int bus_num, uint8_t dev_addr, uint16_t mem_addr, uint8_t *data)
{
	uint8_t buf[3];

	buf[0] = (mem_addr >> 8) & 0xFF;
	buf[1] = mem_addr & 0xFF;
	buf[2] = *data;

	return ti_i2c_write(bus_num, dev_addr, buf, sizeof(buf));
}

int eeprom_read_byte(int bus_num, uint8_t dev_addr, uint16_t mem_addr, uint8_t *data, size_t length)
{
	uint8_t mem_addr_buf[2];
	mem_addr_buf[0] = (mem_addr >> 8) & 0xFF;
	mem_addr_buf[1] = mem_addr & 0xFF;

	int ret = ti_i2c_write(bus_num, dev_addr, mem_addr_buf, sizeof(mem_addr_buf));
	if (ret < 0)
		return ret;

	return ti_i2c_read(bus_num, dev_addr, data, length);
}
