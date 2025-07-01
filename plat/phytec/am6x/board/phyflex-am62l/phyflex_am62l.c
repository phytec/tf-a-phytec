/*
 * Copyright (c) 2025, PHYTEC Messtechnik GmbH
 * Author: Florijan Plohl <florijan.plohl@norik.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/libfdt/libfdt.h>

#include "eeprom.h"
#include "phyflex_am62l.h"
#include "ti_i2c.h"

#define EEPROM_ADDR	0x50
#define RAM_SIZE_OFFSET	0x09

extern unsigned char dtb_array[];

int ti_i2c_get_base_addr(enum I2C_BUS bus_num)
{
	switch (bus_num) {
	case I2C_BUS_0:
		return I2C0_BASE_ADDR;
	case I2C_BUS_1:
		return I2C1_BASE_ADDR;
	case I2C_BUS_2:
		return I2C2_BASE_ADDR;
	case WKUP_I2C_BUS:
		return WKUP_I2C_BASE_ADDR;
	default:
		ERROR("Invalid I2C bus number: %d\n", bus_num);
		return -1;
	}
}

static int patch_dtb_in_bl1(void)
{
	void *dtb = (void *)dtb_array;

	int mem_node = -1;
	int mc_node  = -1;
	int ret;
	fdt32_t *ctl_data, *pi_data;
	int len;

	if (!dtb) {
		ERROR("patch: dtb_array is NULL\n");
		return -1;
	}

	mem_node = fdt_node_offset_by_prop_value(dtb, -1,
						"device_type",
						"memory",
						sizeof("memory"));
	if (mem_node < 0) {
		WARN("patch: Unable to locate 'memory' node: %d\n", mem_node);
		return -1;
	}

	uint32_t reg_cells[4];
	reg_cells[0] = cpu_to_fdt32(0x00000000);
	reg_cells[1] = cpu_to_fdt32(0x80000000);
	reg_cells[2] = cpu_to_fdt32(0x00000000);
	reg_cells[3] = cpu_to_fdt32(0x80000000);

	ret = fdt_setprop(dtb, mem_node, "reg", reg_cells, sizeof(reg_cells));
	if (ret < 0) {
		ERROR("patch: fdt_setprop(reg) failed: %d\n", ret);
		return -1;
	}

	INFO("patch: memory node 'reg' changed to 2 GiB\n");

	mc_node = fdt_node_offset_by_compatible(dtb, -1, "ti,am62l-ddrss");
	if (mc_node < 0) {
		ERROR("patch: memorycontroller node not found\n");
		return -1;
	}

	ctl_data = (fdt32_t *)fdt_getprop(dtb, mc_node, "ti,ctl-data", &len);
	if (!ctl_data) {
		ERROR("patch: ti,ctl-data not found (%d)\n", len);
		return -1;
	}
	len /= sizeof(fdt32_t);

	ctl_data[7]  = cpu_to_fdt32(0x00089158);
	ctl_data[11] = cpu_to_fdt32(0x00089158);
	ctl_data[15] = cpu_to_fdt32(0x00089158);
	ctl_data[72] = cpu_to_fdt32(0x01B80803);
	ctl_data[74] = cpu_to_fdt32(0x000001B8);
	ctl_data[76] = cpu_to_fdt32(0x000001B8);
	ctl_data[96] = cpu_to_fdt32(0x025501C0);
	ctl_data[97] = cpu_to_fdt32(0x025501C0);
	ctl_data[98] = cpu_to_fdt32(0x01C001C0);
	ctl_data[99] = cpu_to_fdt32(0x01C001C0);
	ctl_data[317] = cpu_to_fdt32(0x00000000);
	ctl_data[318] = cpu_to_fdt32(0x3FFF0000);

	pi_data = (fdt32_t *)fdt_getprop(dtb, mc_node, "ti,pi-data", &len);
	if (!pi_data) {
		ERROR("patch: ti,pi-data not found (%d)\n", len);
		return -1;
	}
	len /= sizeof(fdt32_t);

	pi_data[77]  = cpu_to_fdt32(0x04000100);
	pi_data[175] = cpu_to_fdt32(0x000001B8);
	pi_data[177] = cpu_to_fdt32(0x000001B8);
	pi_data[179] = cpu_to_fdt32(0x000001B8);
	pi_data[259] = cpu_to_fdt32(0x00089158);
	pi_data[263] = cpu_to_fdt32(0x000001C0);
	pi_data[264] = cpu_to_fdt32(0x00089158);
	pi_data[268] = cpu_to_fdt32(0x000001C0);
	pi_data[269] = cpu_to_fdt32(0x00089158);
	pi_data[273] = cpu_to_fdt32(0x020001C0);

	INFO("patch: LPDDR4 ctl-data and pi-data registers patched\n");

	return 0;
}

int phyflex_am62l_get_ddr_size(void)
{
	int ret;
#if RAM_SIZE_FIXED == 0
	uint8_t eeprom_data;
	ret = eeprom_read_byte(WKUP_I2C_BUS, EEPROM_ADDR, RAM_SIZE_OFFSET, &eeprom_data, 1);
	if (ret < 0)
		return ret;

	switch (eeprom_data & 0x03) {
	case EEPROM_RAM_SIZE_2GB:
		ret = EEPROM_RAM_SIZE_2GB;
		break;
	case EEPROM_RAM_SIZE_1GB:
	default:
		ret = EEPROM_RAM_SIZE_1GB;
		break;
	}
#else
	ret = RAM_SIZE_FIXED;
#endif

	NOTICE("PHYTEC AM62L DDR size: %d GiB\n", ret);

	return ret;
}

int phyflex_am62l_som_detection(void)
{
	int ret;
	int ddr_size = 1;
	ret = phyflex_am62l_get_ddr_size();
	if (ret < 0)
		return ret;
	else
		ddr_size = ret;

	/* Patch memory node in DTB */
	if (ddr_size == EEPROM_RAM_SIZE_2GB) {
		ret = patch_dtb_in_bl1();
		if (ret < 0) {
			ERROR("%s: patch_dtb_in_bl1 failed\n", __func__);
			return ret;
		}
	}

	return 0;
}
